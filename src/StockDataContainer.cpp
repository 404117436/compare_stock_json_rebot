#include "StockDataContainer.h"
#include <iostream>
#include <sstream>

// 构造函数
StockDataContainer::StockDataContainer()
    : GenericJsonContainer("StockData"), code_(""), index_key_("time"), index_value_("") {
}

StockDataContainer::StockDataContainer(const std::string& source)
    : GenericJsonContainer(source), code_(""), index_key_("time"), index_value_("") {
}

StockDataContainer::StockDataContainer(const std::string& source, const std::string& index_key)
    : GenericJsonContainer(source), code_(""), index_key_(index_key), index_value_("") {
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
            if (indexValue.isString()) {
                index_value_ = indexValue.asString();
            } else if (indexValue.isInt()) {
                index_value_ = std::to_string(indexValue.asInt());
            } else if (indexValue.isDouble()) {
                index_value_ = std::to_string(indexValue.asDouble());
            } else if (indexValue.isBool()) {
                index_value_ = indexValue.asBool() ? "true" : "false";
            }
        }
    } catch (...) {
        index_value_ = "";  // 提取失败则置空
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

// 向后兼容接口（当索引字段为数字类型时）
int64_t StockDataContainer::getIndexAsInt() const {
    try {
        if (!index_value_.empty()) {
            return std::stoll(index_value_);
        }
    } catch (...) {
        // 转换失败
    }
    return 0;
}

double StockDataContainer::getIndexAsDouble() const {
    try {
        if (!index_value_.empty()) {
            return std::stod(index_value_);
        }
    } catch (...) {
        // 转换失败
    }
    return 0.0;
}

// 基本信息输出
void StockDataContainer::printStockInfo() const {
    std::cout << "=== StockDataContainer 信息 ===" << std::endl;
    std::cout << "数据源: " << getSource() << std::endl;
    std::cout << "股票代码: " << (code_.empty() ? "未提取" : code_) << std::endl;
    std::cout << "索引字段: " << index_key_ << std::endl;
    std::cout << "索引值: " << (index_value_.empty() ? "未提取" : index_value_) << std::endl;
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