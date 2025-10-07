#include "StockDataContainer.h"
#include <iostream>
#include <sstream>

// 构造函数
StockDataContainer::StockDataContainer()
    : GenericJsonContainer("StockData"), code_(""), index_key_("time"), index_value_(0), index_decimal_(1), compare_key_(""), compare_value_(0) {
}

StockDataContainer::StockDataContainer(const std::string& source)
    : GenericJsonContainer(source), code_(""), index_key_("time"), index_value_(0), index_decimal_(1), compare_key_(""), compare_value_(0) {
}

StockDataContainer::StockDataContainer(const std::string& source, const std::string& index_key)
    : GenericJsonContainer(source), code_(""), index_key_(index_key), index_value_(0), index_decimal_(1), compare_key_(""), compare_value_(0) {
}

StockDataContainer::StockDataContainer(const std::string& source, const std::string& index_key, int64_t index_decimal)
    : GenericJsonContainer(source), code_(""), index_key_(index_key), index_value_(0), index_decimal_(index_decimal), compare_key_(""), compare_value_(0) {
}

StockDataContainer::StockDataContainer(const std::string& source, const std::string& index_key, int64_t index_decimal, const std::string& compare_key)
    : GenericJsonContainer(source), code_(""), index_key_(index_key), index_value_(0), index_decimal_(index_decimal), compare_key_(compare_key), compare_value_(0) {
}

// 重写JSON解析方法，自动提取关键字段
bool StockDataContainer::parseFromJsonString(const std::string& jsonStr) {
    // 保存原始JSON
    raw_json_ = jsonStr;

    // 调用父类解析方法
    if (GenericJsonContainer::parseFromJsonString(jsonStr)) {
        // 自动提取关键字段
        extractKeyFields();
        return true;
    }
    return false;
}

bool StockDataContainer::parseFromJsonString(const std::string& jsonStr, const std::string& ignore_fields) {
    // 保存原始JSON
    raw_json_ = jsonStr;

    // 调用父类解析方法（带忽略字段）
    if (GenericJsonContainer::parseFromJsonString(jsonStr, ignore_fields)) {
        // 自动提取关键字段
        extractKeyFields();
        return true;
    }
    return false;
}

// 从解析后的数据中自动提取关键字段
void StockDataContainer::extractKeyFields() {
    // 提取股票代码（固定字段）
    try {
        if (hasKey("code")) {
            const auto& codeValue = getValue("code");
            if (codeValue.isString()) {
                code_ = codeValue.asString();
            } else if (codeValue.isInt()) {
                code_ = std::to_string(codeValue.asInt());
            }
        }
    } catch (...) {
        code_ = "";  // 提取失败则置空
    }

    // 提取索引字段（动态配置）
    try {
        if (!index_key_.empty() && hasKey(index_key_)) {
            const auto& indexValue = getValue(index_key_);
            std::string rawIndexValue;
            if (indexValue.isString()) {
                rawIndexValue = indexValue.asString();
            } else if (indexValue.isInt()) {
                rawIndexValue = std::to_string(indexValue.asInt());
            } else if (indexValue.isDouble()) {
                rawIndexValue = std::to_string(indexValue.asDouble());
            } else if (indexValue.isBool()) {
                rawIndexValue = indexValue.asBool() ? "true" : "false";
            }
            // 使用新的转换方法
            index_value_ = convertIndexToComparableValue(rawIndexValue);
        }
    } catch (...) {
        index_value_ = 0;  // 提取失败则置为0
    }

    // 提取比较键（可选，动态配置）
    try {
        if (!compare_key_.empty() && hasKey(compare_key_)) {
            const auto& compareValue = getValue(compare_key_);

            // 直接提取原始值，不做精度除法
            if (compareValue.isInt()) {
                compare_value_ = compareValue.asInt();
            } else if (compareValue.isString()) {
                // 字符串尝试转整数
                compare_value_ = std::stoll(compareValue.asString());
            } else if (compareValue.isDouble()) {
                // 浮点数直接截断
                compare_value_ = static_cast<int64_t>(compareValue.asDouble());
            } else {
                compare_value_ = 0;
            }
        } else {
            compare_value_ = 0;  // 字段不存在或未设置，置为0
        }
    } catch (...) {
        compare_value_ = 0;  // 提取失败则置为0
    }
}

// 索引字段配置
void StockDataContainer::setIndexKey(const std::string& key) {
    index_key_ = key;
    // 如果已经有数据，重新提取索引字段
    if (!empty()) {
        extractKeyFields();
    }
}

// 精度控制
void StockDataContainer::setIndexDecimal(int64_t decimal) {
    if (decimal > 0) {
        index_decimal_ = decimal;
        // 如果已经有数据，重新提取索引字段
        if (!empty()) {
            extractKeyFields();
        }
    }
}

// 比较键配置
void StockDataContainer::setCompareKey(const std::string& key) {
    compare_key_ = key;
    // 如果已经有数据，重新提取比较键
    if (!empty()) {
        extractKeyFields();
    }
}

// 将索引值转换为可比较的数值
int64_t StockDataContainer::convertIndexToComparableValue(const std::string& indexValue) const {
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
    }
}

// 向后兼容接口
double StockDataContainer::getIndexAsDouble() const {
    return static_cast<double>(index_value_);
}

// 基本信息输出
void StockDataContainer::printStockInfo() const {
    std::cout << "=== StockDataContainer 信息 ===" << std::endl;
    std::cout << "数据源: " << getSource() << std::endl;
    std::cout << "股票代码: " << (code_.empty() ? "未提取" : code_) << std::endl;
    std::cout << "索引字段: " << index_key_ << std::endl;
    std::cout << "索引值: " << index_value_ << std::endl;
    std::cout << "索引精度: " << index_decimal_ << std::endl;
    std::cout << "原始JSON长度: " << raw_json_.length() << " 字符" << std::endl;
    std::cout << "解析字段数: " << size() << std::endl;

    // 显示一些常见的股票字段（如果存在）
    std::cout << "\n常见字段值:" << std::endl;

    // 股票名称
    try {
        if (hasKey("codename")) {
            std::cout << "  股票名称: " << getValue("codename").asString() << std::endl;
        }
    } catch (...) {}

    // 市场
    try {
        if (hasKey("market")) {
            std::cout << "  市场: " << getValue("market").asString() << std::endl;
        }
    } catch (...) {}

    // 当前价格
    try {
        if (hasKey("new_price")) {
            std::cout << "  当前价格: " << getValue("new_price").asDouble() << std::endl;
        }
    } catch (...) {}

    // 成交量
    try {
        if (hasKey("volume")) {
            std::cout << "  成交量: " << getValue("volume").asInt() << std::endl;
        }
    } catch (...) {}

    // 时间字符串
    try {
        if (hasKey("local_time")) {
            std::cout << "  本地时间: " << getValue("local_time").asString() << std::endl;
        }
    } catch (...) {}

    std::cout << std::endl;
}

// 字段存在性和访问方法实现
bool StockDataContainer::hasField(const std::string& fieldName) const {
    return hasKey(fieldName);  // 使用继承的hasKey方法
}

std::vector<std::string> StockDataContainer::getFieldNames() const {
    return getAllKeys();  // 使用继承的getAllKeys方法
}

CustomValue StockDataContainer::getFieldSafe(const std::string& fieldName) const {
    if (hasField(fieldName)) {
        return getValue(fieldName);  // 使用继承的getValue方法
    }
    return CustomValue();  // 返回null类型的CustomValue
}