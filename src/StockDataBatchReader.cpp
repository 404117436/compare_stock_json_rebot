#include "StockDataBatchReader.h"
#include <iostream>

// 构造函数
StockDataBatchReader::StockDataBatchReader(const std::string& filePath, size_t maxMemorySize, int64_t indexDecimal)
    : filePath_(filePath), maxMemorySize_(maxMemorySize), index_decimal_(indexDecimal), hasPendingData_(false) {

    try {
        lineReader_ = std::unique_ptr<LineReader>(new LineReader(filePath_));
    } catch (const std::exception& e) {
        std::cerr << "Failed to open file: " << filePath_ << ", error: " << e.what() << std::endl;
        lineReader_ = nullptr;
    }
}

// 读取下一批次
size_t StockDataBatchReader::readNextBatch(const std::string& indexKey) {
    if (!lineReader_ || !lineReader_->isOpen()) {
        return 0;
    }

    size_t recordsRead = 0;
    int64_t currentBatchValue = 0;
    bool batchStarted = false;

    // 1. 优先处理临界数据
    if (hasPendingData_) {
        pendingData_.setIndexKey(indexKey);  // 确保使用正确的索引字段
        currentBatchValue = convertIndexToComparableValue(pendingData_.getIndexValue());
        dataQueue_.push_back(std::move(pendingData_));
        hasPendingData_ = false;
        batchStarted = true;
        recordsRead++;
    }

    // 2. 循环读取数据
    StockDataContainer newData;
    while (readSingleRecord(newData, indexKey)) {
        int64_t newIndexValue = convertIndexToComparableValue(newData.getIndexValue());

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
bool StockDataBatchReader::readSingleRecord(StockDataContainer& container, const std::string& indexKey) {
    if (!lineReader_->hasNextLine()) {
        return false;
    }

    try {
        std::string jsonLine = lineReader_->output();

        // 跳过空行
        if (jsonLine.empty() || jsonLine.find_first_not_of(" \t\r\n") == std::string::npos) {
            return readSingleRecord(container, indexKey);  // 递归处理下一行
        }

        // 创建新容器并设置索引字段
        container = StockDataContainer("BatchData", indexKey);

        // 解析JSON
        if (container.parseFromJsonString(jsonLine)) {
            return true;
        } else {
            // 解析失败，尝试下一行
            return readSingleRecord(container, indexKey);
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
        totalSize += container.getIndexValue().size();
        totalSize += container.getIndexKey().size();
    }

    // 临界数据大小
    if (hasPendingData_) {
        totalSize += pendingData_.getRawJson().size();
        totalSize += sizeof(StockDataContainer);
        totalSize += pendingData_.getCode().size();
        totalSize += pendingData_.getIndexValue().size();
        totalSize += pendingData_.getIndexKey().size();
    }

    return totalSize;
}

// 取出当前批次
std::vector<StockDataContainer> StockDataBatchReader::popBatch() {
    if (dataQueue_.empty()) {
        return {};
    }

    std::vector<StockDataContainer> result;
    int64_t currentBatchValue = convertIndexToComparableValue(dataQueue_.front().getIndexValue());

    // 只取出index_value_相同的连续数据
    while (!dataQueue_.empty() &&
           convertIndexToComparableValue(dataQueue_.front().getIndexValue()) == currentBatchValue) {
        result.push_back(std::move(dataQueue_.front()));
        dataQueue_.pop_front();
    }

    return result;
}

// 获取批次（拷贝）
std::vector<StockDataContainer> StockDataBatchReader::getBatch() const {
    if (dataQueue_.empty()) {
        return {};
    }

    std::vector<StockDataContainer> result;
    int64_t currentBatchValue = convertIndexToComparableValue(dataQueue_.front().getIndexValue());

    // 只复制index_value_相同的连续数据
    for (const auto& container : dataQueue_) {
        if (convertIndexToComparableValue(container.getIndexValue()) == currentBatchValue) {
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
    int64_t firstBatchValue = convertIndexToComparableValue(dataQueue_.front().getIndexValue());

    // 如果只有一个元素且没有更多数据，也算完整批次
    if (dataQueue_.size() == 1 && !lineReader_->hasNextLine() && !hasPendingData_) {
        return true;
    }

    // 检查是否有多个不同的批次值（表示至少有一个完整批次）
    for (const auto& container : dataQueue_) {
        if (convertIndexToComparableValue(container.getIndexValue()) != firstBatchValue) {
            return true;
        }
    }

    // 如果有临界数据且与当前批次不同，也表示当前批次完整
    if (hasPendingData_ && convertIndexToComparableValue(pendingData_.getIndexValue()) != firstBatchValue) {
        return true;
    }

    return false;
}

// 清空批次
void StockDataBatchReader::clearBatch() {
    dataQueue_.clear();
}

// 设置索引精度
void StockDataBatchReader::setIndexDecimal(int64_t decimal) {
    if (decimal <= 0) {
        std::cerr << "警告：索引精度必须为正数，已设置为默认值1" << std::endl;
        index_decimal_ = 1;
    } else {
        index_decimal_ = decimal;
    }
}

// 将索引值转换为可比较的数值
int64_t StockDataBatchReader::convertIndexToComparableValue(const std::string& indexValue) const {
    try {
        if (indexValue.empty()) {
            return 0;
        }

        // 移除前导空格
        std::string trimmedValue = indexValue;
        size_t start = trimmedValue.find_first_not_of(" \t");
        if (start != std::string::npos) {
            trimmedValue = trimmedValue.substr(start);
        } else {
            return 0; // 全是空格
        }

        // 转换为int64_t
        int64_t value = std::stoll(trimmedValue);

        // 应用精度除法
        return value / index_decimal_;

    } catch (const std::invalid_argument& e) {
        std::cerr << "警告：无法将索引值 '" << indexValue << "' 转换为数字，使用默认值0" << std::endl;
        return 0;
    } catch (const std::out_of_range& e) {
        std::cerr << "警告：索引值 '" << indexValue << "' 超出int64范围，使用默认值0" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "警告：转换索引值时发生未知错误: " << e.what() << "，使用默认值0" << std::endl;
        return 0;
    }
}