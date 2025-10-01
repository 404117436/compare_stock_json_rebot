#include "StockDataBatchReader.h"
#include <iostream>
#include <algorithm>

// 构造函数
StockDataBatchReader::StockDataBatchReader(const std::string& filePath, const std::string& indexKey, int64_t indexDecimal, const std::vector<std::string>& ignoreFields)
    : filePath_(filePath), maxMemorySize_(MEM_SIZE), container_decimal_(indexDecimal), indexKey_(indexKey), ignore_fields_(ignoreFields), hasPendingData_(false) {

    // 预计算忽略字段缓存（性能优化）
    rebuildIgnoreCache();

    try {
        lineReader_ = std::unique_ptr<LineReader>(new LineReader(filePath_));
    } catch (const std::exception& e) {
        std::cerr << "Failed to open file: " << filePath_ << ", error: " << e.what() << std::endl;
        lineReader_ = nullptr;
    }
}

// 读取下一批次
size_t StockDataBatchReader::readNextBatch() {
    if (!lineReader_ || !lineReader_->isOpen()) {
        return 0;
    }

    size_t recordsRead = 0;
    int64_t currentBatchValue = 0;
    bool batchStarted = false;

    // 1. 优先处理临界数据
    if (hasPendingData_) {
        pendingData_.setIndexKey(indexKey_);  // 确保使用正确的索引字段
        pendingData_.setIndexDecimal(container_decimal_);  // 确保使用正确的精度
        currentBatchValue = pendingData_.getIndexValue();
        dataQueue_.push_back(std::move(pendingData_));
        hasPendingData_ = false;
        batchStarted = true;
        recordsRead++;
    }

    // 2. 循环读取数据
    StockDataContainer newData;
    while (readSingleRecord(newData)) {
        int64_t newIndexValue = newData.getIndexValue();

        // 检查是否开始新批次
        if (batchStarted && newIndexValue != currentBatchValue) {
            // index_value_不同，检查内存是否超限
            if (getCurrentMemoryUsage() >= maxMemorySize_) {
                // 缓存临界数据，停止读取
                pendingData_ = std::move(newData);
                hasPendingData_ = true;
                break;
            } else {
                // 内存未超限，开始新批次
                currentBatchValue = newIndexValue;
            }
        } else if (!batchStarted) {
            // 第一条数据，初始化批次
            currentBatchValue = newIndexValue;
            batchStarted = true;
        }

        // 将数据添加到队列
        dataQueue_.push_back(std::move(newData));
        recordsRead++;
    }

    return recordsRead;
}

// 读取单条记录
bool StockDataBatchReader::readSingleRecord(StockDataContainer& container) {
    if (!lineReader_->hasNextLine()) {
        return false;
    }

    try {
        std::string jsonLine = lineReader_->output();

        // 跳过空行
        if (jsonLine.empty() || jsonLine.find_first_not_of(" \t\r\n") == std::string::npos) {
            return readSingleRecord(container);  // 递归处理下一行
        }

        // 创建新容器并设置索引字段
        container = StockDataContainer("BatchData", indexKey_, container_decimal_);

        // 解析JSON（使用预计算的缓存字符串，避免每次重建）
        bool parseSuccess = false;
        if (ignore_fields_.empty()) {
            parseSuccess = container.parseFromJsonString(jsonLine);
        } else {
            parseSuccess = container.parseFromJsonString(jsonLine, cached_ignore_string_);
        }

        if (parseSuccess) {
            return true;
        } else {
            // 解析失败，尝试下一行
            return readSingleRecord(container);
        }

    } catch (const std::exception& e) {
        std::cerr << "Error reading record: " << e.what() << std::endl;
        return false;
    }
}

// 计算当前内存使用
size_t StockDataBatchReader::getCurrentMemoryUsage() const {
    size_t totalSize = 0;

    for (const auto& container : dataQueue_) {
        // JSON字符串大小
        totalSize += container.getRawJson().size();
        // 对象本身大小（估算）
        totalSize += sizeof(StockDataContainer);
        // 字符串成员变量大小
        totalSize += container.getCode().size();
        totalSize += sizeof(int64_t);  // getIndexValue()现在是int64_t类型
        totalSize += container.getIndexKey().size();
    }

    // 临界数据大小
    if (hasPendingData_) {
        totalSize += pendingData_.getRawJson().size();
        totalSize += sizeof(StockDataContainer);
        totalSize += pendingData_.getCode().size();
        totalSize += sizeof(int64_t);  // getIndexValue()现在是int64_t类型
        totalSize += pendingData_.getIndexKey().size();
    }

    return totalSize;
}

// 取出当前批次
bool StockDataBatchReader::popBatch(std::vector<StockDataContainer>& result) {
    // 清空输出参数
    result.clear();

    // 1. 当deque为空时调用readNextBatch补充数据
    if (dataQueue_.empty()) {
        size_t readCount = readNextBatch();
        if (readCount == 0) {
            return false; // 没有更多数据可读
        }
    }

    // 2. 如果仍然为空，返回失败
    if (dataQueue_.empty()) {
        return false;
    }

    // 3. 执行原有的批次提取逻辑
    int64_t currentBatchValue = dataQueue_.front().getIndexValue();

    // 只取出index_value_相同的连续数据
    while (!dataQueue_.empty() &&
           dataQueue_.front().getIndexValue() == currentBatchValue) {
        result.push_back(std::move(dataQueue_.front()));
        dataQueue_.pop_front();
    }

    return true; // 成功获取批次
}

// 获取批次（拷贝）
std::vector<StockDataContainer> StockDataBatchReader::getBatch() const {
    if (dataQueue_.empty()) {
        return {};
    }

    std::vector<StockDataContainer> result;
    int64_t currentBatchValue = dataQueue_.front().getIndexValue();

    // 只复制index_value_相同的连续数据
    for (const auto& container : dataQueue_) {
        if (container.getIndexValue() == currentBatchValue) {
            result.push_back(container);
        } else {
            break;  // 遇到不同批次，停止复制
        }
    }

    return result;
}

// 检查是否有完整批次
bool StockDataBatchReader::hasCompleteBatch() const {
    if (dataQueue_.empty()) {
        return false;
    }

    // 检查队列中是否至少有一个完整的批次
    int64_t firstBatchValue = dataQueue_.front().getIndexValue();

    // 如果只有一个元素且没有更多数据，也算完整批次
    if (dataQueue_.size() == 1 && !lineReader_->hasNextLine() && !hasPendingData_) {
        return true;
    }

    // 检查是否有多个不同的批次值（表示至少有一个完整批次）
    for (const auto& container : dataQueue_) {
        if (container.getIndexValue() != firstBatchValue) {
            return true;
        }
    }

    // 如果有临界数据且与当前批次不同，也表示当前批次完整
    if (hasPendingData_ && pendingData_.getIndexValue() != firstBatchValue) {
        return true;
    }

    return false;
}

// 清空批次
void StockDataBatchReader::clearBatch() {
    dataQueue_.clear();
}


// 将忽略字段转换为逗号分隔字符串
std::string StockDataBatchReader::joinIgnoreFields() const {
    if (ignore_fields_.empty()) {
        return "";
    }

    std::string result;
    for (size_t i = 0; i < ignore_fields_.size(); ++i) {
        if (i > 0) {
            result += ",";
        }
        result += ignore_fields_[i];
    }
    return result;
}

// 重建忽略字段缓存（性能优化）
void StockDataBatchReader::rebuildIgnoreCache() {
    // 清空旧缓存
    cached_ignore_string_.clear();
    cached_ignore_set_.clear();

    if (ignore_fields_.empty()) {
        return;
    }

    // 1. 构建 set 缓存（用于快速查找）
    cached_ignore_set_ = std::set<std::string>(
        ignore_fields_.begin(),
        ignore_fields_.end()
    );

    // 2. 构建字符串缓存（用于传递给 parseFromJsonString）
    // 优化：预先计算总长度，避免多次重分配
    size_t totalLen = 0;
    for (const auto& field : ignore_fields_) {
        totalLen += field.size();
    }
    if (!ignore_fields_.empty()) {
        totalLen += (ignore_fields_.size() - 1);  // 逗号数量
    }

    cached_ignore_string_.reserve(totalLen);  // 预分配内存

    for (size_t i = 0; i < ignore_fields_.size(); ++i) {
        if (i > 0) {
            cached_ignore_string_ += ',';
        }
        cached_ignore_string_ += ignore_fields_[i];
    }
}

// 字段过滤控制方法
void StockDataBatchReader::setIgnoreFields(const std::vector<std::string>& fields) {
    ignore_fields_ = fields;
    rebuildIgnoreCache();  // 触发缓存重建
}

void StockDataBatchReader::addIgnoreField(const std::string& field) {
    // 检查字段是否已存在，避免重复添加
    for (const auto& existingField : ignore_fields_) {
        if (existingField == field) {
            return; // 字段已存在，不重复添加
        }
    }
    ignore_fields_.push_back(field);
    rebuildIgnoreCache();  // 触发缓存重建
}

void StockDataBatchReader::removeIgnoreField(const std::string& field) {
    auto it = std::find(ignore_fields_.begin(), ignore_fields_.end(), field);
    if (it != ignore_fields_.end()) {
        ignore_fields_.erase(it);
        rebuildIgnoreCache();  // 触发缓存重建
    }
}