#pragma once

#include "StockDataContainer.h"
#include <vector>
#include <string>
#include <unordered_map>

/**
 * StockDataComparator - 股票数据比较器
 * 功能：对比两个股票数据集合，提供核心差异分析
 */

// 字段差异详情
struct FieldDifference {
    std::string fieldName;                      // 字段名
    bool existsInA;                             // 是否在A中存在
    bool existsInB;                             // 是否在B中存在
    CustomValue valueA;                         // A中的值
    CustomValue valueB;                         // B中的值
    std::string differenceType;                 // 差异类型: "missing", "type_mismatch", "value_different"
    std::string description;                    // 差异描述

    FieldDifference() : existsInA(false), existsInB(false) {}
};

// 记录比较详情
struct RecordComparisonDetail {
    bool identical;                             // 是否完全相同
    std::vector<FieldDifference> differences;   // 字段差异列表
    std::string recordKey;                      // 记录键(code_indexValue)
    std::string raw_json_a;                     // 原始JSON数据A
    std::string raw_json_b;                     // 原始JSON数据B

    RecordComparisonDetail() : identical(false) {}
};

// 比较结果结构体
struct ComparisonResult {
    // 基础统计
    size_t countA;                              // 数据集A的记录数
    size_t countB;                              // 数据集B的记录数

    // 差异统计
    size_t onlyInA;                             // 仅在A中存在的记录数
    size_t onlyInB;                             // 仅在B中存在的记录数
    size_t common;                              // 共同存在的记录数

    // 索引值统计
    std::unordered_map<std::string, size_t> indexStatsA;  // A的索引值分布
    std::unordered_map<std::string, size_t> indexStatsB;  // B的索引值分布

    // 比较摘要
    bool identical;                             // 两个数据集是否完全相同
    double similarity;                          // 相似度 (0.0-1.0)
    std::string summary;                        // 比较结果摘要

    // 详细差异信息
    std::vector<RecordComparisonDetail> detailedDifferences;  // 详细记录差异
    bool enableDetailedComparison;              // 是否启用详细比较
    size_t totalComparedRecords;                // 总比较记录数
    size_t recordsWithDifferences;              // 有差异的记录数

    ComparisonResult() : countA(0), countB(0), onlyInA(0), onlyInB(0),
                        common(0), identical(false), similarity(0.0),
                        enableDetailedComparison(false), totalComparedRecords(0), recordsWithDifferences(0) {}

    // 差异报告方法
    void printDetailedDifferences() const;
    std::string generateDifferenceReport() const;
    void addRecordDifference(const RecordComparisonDetail& detail);
};

class StockDataComparator {
private:
    std::vector<StockDataContainer> a_;         // 数据集A
    std::vector<StockDataContainer> b_;         // 数据集B

    // 内部辅助方法
    std::string generateRecordKey(const StockDataContainer& container) const;
    void calculateStatistics(ComparisonResult& result) const;
    void generateSummary(ComparisonResult& result) const;

    // 精确字段比较方法
    bool compareRecordFields(const StockDataContainer& a, const StockDataContainer& b) const;
    RecordComparisonDetail compareRecordFieldsDetailed(const StockDataContainer& a, const StockDataContainer& b) const;
    bool compareCustomValues(const CustomValue& a, const CustomValue& b) const;
    bool compareStringFields(const std::string& a, const std::string& b) const;
    bool compareDoubleFields(double a, double b) const;

    // 差异分析辅助方法
    FieldDifference createFieldDifference(const std::string& fieldName,
                                         bool hasA, bool hasB,
                                         const CustomValue& valueA, const CustomValue& valueB) const;
    std::string describeDifferenceType(const FieldDifference& diff) const;

public:
    // 构造函数
    StockDataComparator() = default;

    // 禁用拷贝构造和赋值
    StockDataComparator(const StockDataComparator&) = delete;
    StockDataComparator& operator=(const StockDataComparator&) = delete;

    // 数据注入接口
    void setDataA(const std::vector<StockDataContainer>& data);          // 设置数据集A
    void setDataB(const std::vector<StockDataContainer>& data);          // 设置数据集B
    void setDataA(std::vector<StockDataContainer>&& data);               // 移动设置数据集A
    void setDataB(std::vector<StockDataContainer>&& data);               // 移动设置数据集B

    // 数据访问接口
    const std::vector<StockDataContainer>& getDataA() const { return a_; }
    const std::vector<StockDataContainer>& getDataB() const { return b_; }
    size_t getSizeA() const { return a_.size(); }
    size_t getSizeB() const { return b_.size(); }

    // 核心比较功能
    ComparisonResult compareDetailed() const;                             // 执行详细比较（包含字段差异）

    // 差异检测
    std::vector<StockDataContainer> getOnlyInA() const;                   // 获取仅在A中的记录
    std::vector<StockDataContainer> getOnlyInB() const;                   // 获取仅在B中的记录
    std::vector<StockDataContainer> getCommon() const;                    // 获取共同记录

    // MISS记录处理
    RecordComparisonDetail createMissRecord(const StockDataContainer& record, bool missingInA) const;  // 创建缺失记录的差异详情

    // 清理数据
    void clear();                                                         // 清空所有数据
    void clearA();                                                        // 清空数据集A
    void clearB();                                                        // 清空数据集B
};