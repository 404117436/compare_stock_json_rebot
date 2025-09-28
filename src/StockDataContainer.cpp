#include "StockDataContainer.h"
#include <iostream>
#include <sstream>

// 构造函数
StockDataContainer::StockDataContainer()
    : GenericJsonContainer("StockData"), code_(""), time_(0) {
}

StockDataContainer::StockDataContainer(const std::string& source)
    : GenericJsonContainer(source), code_(""), time_(0) {
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
    // 提取股票代码
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

    // 提取时间戳
    try {
        // 优先尝试time字段
        if (hasKey("time")) {
            const auto& timeValue = getValue("time");
            if (timeValue.isInt()) {
                time_ = timeValue.asInt();
            } else if (timeValue.isString()) {
                // 尝试将字符串转换为数字
                try {
                    time_ = std::stoll(timeValue.asString());
                } catch (...) {
                    time_ = 0;
                }
            }
        }
        // 如果time字段不存在或无效，尝试其他时间字段
        else if (hasKey("date")) {
            const auto& dateValue = getValue("date");
            if (dateValue.isInt()) {
                time_ = dateValue.asInt();
            }
        }
        // 尝试recv_time字段
        else if (hasKey("recv_time")) {
            const auto& recvTimeValue = getValue("recv_time");
            if (recvTimeValue.isInt()) {
                time_ = recvTimeValue.asInt();
            }
        }
    } catch (...) {
        time_ = 0;  // 提取失败则置为0
    }
}

// 基本信息输出
void StockDataContainer::printStockInfo() const {
    std::cout << "=== StockDataContainer 信息 ===" << std::endl;
    std::cout << "数据源: " << getSource() << std::endl;
    std::cout << "股票代码: " << (code_.empty() ? "未提取" : code_) << std::endl;
    std::cout << "时间戳: " << time_ << std::endl;
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