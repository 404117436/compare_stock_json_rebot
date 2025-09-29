#pragma once

#include "GenericJsonContainer.h"
#include <string>
#include <cstdint>

/**
 * 股票数据容器类
 * 继承自GenericJsonContainer，专门用于解析股票行情数据
 *
 * 核心功能：
 * - 继承通用JSON解析能力
 * - 自动提取并缓存关键字段（code固定，index可配置）
 * - 保留原始JSON数据
 */
class StockDataContainer : public GenericJsonContainer {
private:
    std::string code_;         // 股票代码（固定提取"code"字段）
    std::string index_key_;    // 用户指定的索引字段名
    std::string index_value_;  // 索引字段对应的值
    std::string raw_json_;     // 原始JSON字符串

    // 从解析后的数据中自动提取关键字段
    void extractKeyFields();

public:
    // 构造函数
    StockDataContainer();                                                    // 默认索引字段为"time"
    explicit StockDataContainer(const std::string& source);                 // 默认索引字段为"time"
    StockDataContainer(const std::string& source, const std::string& index_key); // 指定索引字段

    // 重写JSON解析方法，自动提取关键字段
    bool parseFromJsonString(const std::string& jsonStr) override;
    bool parseFromJsonString(const std::string& jsonStr, const std::string& ignore_fields) override;

    // 索引字段配置
    void setIndexKey(const std::string& key);                              // 设置索引字段名
    const std::string& getIndexKey() const { return index_key_; }          // 获取当前索引字段名

    // 核心访问接口
    const std::string& getCode() const { return code_; }                   // 获取股票代码
    const std::string& getIndexValue() const { return index_value_; }      // 获取索引字段值
    const std::string& getRawJson() const { return raw_json_; }            // 获取原始JSON

    // 向后兼容接口（当索引字段为数字类型时）
    int64_t getIndexAsInt() const;                                         // 将索引值转为整数
    double getIndexAsDouble() const;                                       // 将索引值转为浮点数

    // 基本信息输出
    void printStockInfo() const;

    // 字段存在性和访问方法（用于比较器）
    bool hasField(const std::string& fieldName) const;                     // 检查字段是否存在
    std::vector<std::string> getFieldNames() const;                        // 获取所有字段名
    CustomValue getFieldSafe(const std::string& fieldName) const;          // 安全获取字段值
};