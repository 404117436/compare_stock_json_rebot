#include "StockDataComparator.h"
#include <algorithm>
#include <unordered_set>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <cmath>

// 列宽计算辅助结构体
struct ColumnWidths {
    size_t fieldName;
    size_t diffType;
    size_t valueA;
    size_t valueB;

    ColumnWidths() : fieldName(16), diffType(16), valueA(20), valueB(20) {}
};

// 格式化差异类型名称
static std::string getDiffTypeName(const FieldDifference& diff) {
    if (diff.differenceType == "missing_in_A") {
        return "MISSING_IN_A";
    } else if (diff.differenceType == "missing_in_B") {
        return "MISSING_IN_B";
    } else if (diff.differenceType == "value_different") {
        return "VALUE_DIFFERENT";
    } else if (diff.differenceType == "type_mismatch") {
        return "TYPE_MISMATCH";
    } else {
        return diff.differenceType;
    }
}

// 格式化值为字符串
static std::string formatValueString(const FieldDifference& diff, bool isValueA) {
    const CustomValue& value = isValueA ? diff.valueA : diff.valueB;
    bool exists = isValueA ? diff.existsInA : diff.existsInB;

    if (!exists) {
        return "null";
    }

    switch (value.getType()) {
        case JsonValueType::String:
            return "\"" + value.asString() + "\"";
        case JsonValueType::Int:
            return std::to_string(value.asInt());
        case JsonValueType::Double:
            return std::to_string(value.asDouble());
        case JsonValueType::Bool:
            return value.asBool() ? "true" : "false";
        case JsonValueType::Null:
            return "null";
        case JsonValueType::Array:
        case JsonValueType::Object:
            return "[复杂类型]";
        default:
            return "[未知类型]";
    }
}

// 计算各列的最优宽度
static ColumnWidths calculateColumnWidths(const std::vector<FieldDifference>& differences) {
    ColumnWidths widths;

    // 设置标题行的最小宽度
    widths.fieldName = std::max(widths.fieldName, static_cast<size_t>(16)); // "字段名"
    widths.diffType = std::max(widths.diffType, static_cast<size_t>(16));   // "差异类型"
    widths.valueA = std::max(widths.valueA, static_cast<size_t>(20));       // "A侧值"
    widths.valueB = std::max(widths.valueB, static_cast<size_t>(20));       // "B侧值"

    // 根据实际数据调整宽度
    for (const auto& diff : differences) {
        // 字段名宽度
        widths.fieldName = std::max(widths.fieldName, diff.fieldName.length() + 2);

        // 差异类型宽度
        std::string diffTypeName = getDiffTypeName(diff);
        widths.diffType = std::max(widths.diffType, diffTypeName.length() + 2);

        // A侧值宽度
        std::string valueAStr = formatValueString(diff, true);
        widths.valueA = std::max(widths.valueA, valueAStr.length() + 2);

        // B侧值宽度
        std::string valueBStr = formatValueString(diff, false);
        widths.valueB = std::max(widths.valueB, valueBStr.length() + 2);
    }

    return widths;
}

// 数据注入接口 - 拷贝版本
void StockDataComparator::setDataA(const std::vector<StockDataContainer>& data) {
    a_ = data;
}

void StockDataComparator::setDataB(const std::vector<StockDataContainer>& data) {
    b_ = data;
}

// 数据注入接口 - 移动版本
void StockDataComparator::setDataA(std::vector<StockDataContainer>&& data) {
    a_ = std::move(data);
}

void StockDataComparator::setDataB(std::vector<StockDataContainer>&& data) {
    b_ = std::move(data);
}

// 生成记录的唯一键
std::string StockDataComparator::generateRecordKey(const StockDataContainer& container) const {
    // 简化版：使用code + "_" + indexValue作为分组标识
    return container.getCode() + "_" + container.getIndexValue();
}


// 获取仅在A中的记录
std::vector<StockDataContainer> StockDataComparator::getOnlyInA() const {
    std::vector<StockDataContainer> result;
    std::unordered_set<std::string> keysB;

    // 收集B的所有键
    for (const auto& container : b_) {
        keysB.insert(generateRecordKey(container));
    }

    // 找出仅在A中的记录
    for (const auto& container : a_) {
        if (keysB.count(generateRecordKey(container)) == 0) {
            result.push_back(container);
        }
    }

    return result;
}

// 获取仅在B中的记录
std::vector<StockDataContainer> StockDataComparator::getOnlyInB() const {
    std::vector<StockDataContainer> result;
    std::unordered_set<std::string> keysA;

    // 收集A的所有键
    for (const auto& container : a_) {
        keysA.insert(generateRecordKey(container));
    }

    // 找出仅在B中的记录
    for (const auto& container : b_) {
        if (keysA.count(generateRecordKey(container)) == 0) {
            result.push_back(container);
        }
    }

    return result;
}

// 获取共同记录
std::vector<StockDataContainer> StockDataComparator::getCommon() const {
    std::vector<StockDataContainer> result;
    std::unordered_set<std::string> keysB;

    // 收集B的所有键
    for (const auto& container : b_) {
        keysB.insert(generateRecordKey(container));
    }

    // 找出共同记录
    for (const auto& container : a_) {
        if (keysB.count(generateRecordKey(container)) > 0) {
            result.push_back(container);
        }
    }

    return result;
}


// 计算统计信息
void StockDataComparator::calculateStatistics(ComparisonResult& result) const {
    // 简化版：不再计算索引分布
    (void)result; // 避免未使用参数警告
}

// 生成比较摘要
void StockDataComparator::generateSummary(ComparisonResult& result) const {
    std::stringstream ss;

    if (result.identical) {
        ss << "两个数据集完全相同";
    } else {
        ss << "数据集A: " << result.countA << "条记录, ";
        ss << "数据集B: " << result.countB << "条记录. ";
        ss << "共同记录: " << result.common << "条, ";
        ss << "仅A有: " << result.onlyInA << "条, ";
        ss << "仅B有: " << result.onlyInB << "条. ";
        ss << "相似度: " << std::fixed << std::setprecision(2) << (result.similarity * 100) << "%";
    }

    result.summary = ss.str();
}

// 清理数据
void StockDataComparator::clear() {
    a_.clear();
    b_.clear();
}

void StockDataComparator::clearA() {
    a_.clear();
}

void StockDataComparator::clearB() {
    b_.clear();
}

// 精确字段比较方法实现
bool StockDataComparator::compareRecordFields(const StockDataContainer& a, const StockDataContainer& b) const {
    // 1. 基础字段比较
    if (a.getCode() != b.getCode()) return false;
    if (a.getIndexKey() != b.getIndexKey()) return false;
    if (a.getIndexValue() != b.getIndexValue()) return false;

    // 2. 动态获取所有字段名进行比较
    std::vector<std::string> fieldsA = a.getFieldNames();
    std::vector<std::string> fieldsB = b.getFieldNames();

    // 合并所有字段名，去重
    std::unordered_set<std::string> allFieldNames;
    for (const auto& field : fieldsA) {
        allFieldNames.insert(field);
    }
    for (const auto& field : fieldsB) {
        allFieldNames.insert(field);
    }

    // 3. 逐字段比较，处理边界条件
    for (const auto& fieldName : allFieldNames) {
        bool hasA = a.hasField(fieldName);
        bool hasB = b.hasField(fieldName);

        // 处理四种情况
        if (hasA && hasB) {
            // 两个容器都有该字段，比较值
            CustomValue valueA = a.getFieldSafe(fieldName);
            CustomValue valueB = b.getFieldSafe(fieldName);
            if (!compareCustomValues(valueA, valueB)) {
                return false;
            }
        } else if (!hasA && !hasB) {
            // 两个容器都没有该字段，视为相等，继续下一个字段
            continue;
        } else {
            // 只有一个容器有该字段，视为不相等
            return false;
        }
    }

    return true;
}

bool StockDataComparator::compareCustomValues(const CustomValue& a, const CustomValue& b) const {
    // 类型必须相同
    if (a.getType() != b.getType()) {
        return false;
    }

    // 根据类型进行比较
    switch (a.getType()) {
        case JsonValueType::String:
            return compareStringFields(a.asString(), b.asString());
        case JsonValueType::Int:
            return a.asInt() == b.asInt();
        case JsonValueType::Double:
            return compareDoubleFields(a.asDouble(), b.asDouble());
        case JsonValueType::Bool:
            return a.asBool() == b.asBool();
        case JsonValueType::Null:
            return true; // 两个null值相等
        case JsonValueType::Array: {
            const JsonArrayData& arrayA = a.asArray();
            const JsonArrayData& arrayB = b.asArray();

            // 首先比较数组长度
            if (arrayA.size() != arrayB.size()) {
                return false;
            }

            // 逐个比较数组元素
            for (size_t i = 0; i < arrayA.size(); ++i) {
                if (!compareCustomValues(arrayA[i], arrayB[i])) {
                    return false;
                }
            }
            return true;
        }
        default:
            return false;
    }
}

bool StockDataComparator::compareStringFields(const std::string& a, const std::string& b) const {
    // 使用memcmp进行字符串比较
    if (a.length() != b.length()) {
        return false;
    }
    return std::memcmp(a.c_str(), b.c_str(), a.length()) == 0;
}

bool StockDataComparator::compareDoubleFields(double a, double b) const {
    // 对于浮点数比较，考虑精度问题
    const double epsilon = 1e-9;
    return std::abs(a - b) < epsilon;
}

// 详细字段比较方法 - 收集所有差异信息
RecordComparisonDetail StockDataComparator::compareRecordFieldsDetailed(const StockDataContainer& a, const StockDataContainer& b) const {
    RecordComparisonDetail detail;
    detail.recordKey = generateRecordKey(a);
    detail.raw_json_a = a.getRawJson();
    detail.raw_json_b = b.getRawJson();

    // 1. 基础字段比较
    if (a.getCode() != b.getCode() || a.getIndexKey() != b.getIndexKey() || a.getIndexValue() != b.getIndexValue()) {
        // 基础字段不匹配，直接返回不相同
        detail.identical = false;
        return detail;
    }

    // 2. 动态获取所有字段名进行比较
    std::vector<std::string> fieldsA = a.getFieldNames();
    std::vector<std::string> fieldsB = b.getFieldNames();

    // 合并所有字段名，去重
    std::unordered_set<std::string> allFieldNames;
    for (const auto& field : fieldsA) {
        allFieldNames.insert(field);
    }
    for (const auto& field : fieldsB) {
        allFieldNames.insert(field);
    }

    detail.identical = true; // 假设相同，有差异时会改为false

    // 3. 逐字段比较，收集所有差异信息
    for (const auto& fieldName : allFieldNames) {
        bool hasA = a.hasField(fieldName);
        bool hasB = b.hasField(fieldName);

        CustomValue valueA, valueB;
        if (hasA) valueA = a.getFieldSafe(fieldName);
        if (hasB) valueB = b.getFieldSafe(fieldName);

        // 处理四种情况并收集差异
        if (hasA && hasB) {
            // 两个容器都有该字段，比较值
            if (!compareCustomValues(valueA, valueB)) {
                FieldDifference diff = createFieldDifference(fieldName, hasA, hasB, valueA, valueB);
                detail.differences.push_back(diff);
                detail.identical = false;
            }
        } else if (!hasA && !hasB) {
            // 两个容器都没有该字段，不应该出现在allFieldNames中
            continue;
        } else {
            // 只有一个容器有该字段
            FieldDifference diff = createFieldDifference(fieldName, hasA, hasB, valueA, valueB);
            detail.differences.push_back(diff);
            detail.identical = false;
        }
    }

    return detail;
}

// 创建字段差异对象
FieldDifference StockDataComparator::createFieldDifference(const std::string& fieldName,
                                                          bool hasA, bool hasB,
                                                          const CustomValue& valueA, const CustomValue& valueB) const {
    FieldDifference diff;
    diff.fieldName = fieldName;
    diff.existsInA = hasA;
    diff.existsInB = hasB;
    diff.valueA = valueA;
    diff.valueB = valueB;

    // 确定差异类型
    if (!hasA && hasB) {
        diff.differenceType = "missing_in_A";
        diff.description = "字段仅在B中存在";
    } else if (hasA && !hasB) {
        diff.differenceType = "missing_in_B";
        diff.description = "字段仅在A中存在";
    } else if (hasA && hasB) {
        if (valueA.getType() != valueB.getType()) {
            diff.differenceType = "type_mismatch";
            diff.description = "字段类型不匹配";
        } else {
            diff.differenceType = "value_different";
            diff.description = "字段值不同";
        }
    }

    return diff;
}

// 描述差异类型
std::string StockDataComparator::describeDifferenceType(const FieldDifference& diff) const {
    std::stringstream ss;
    ss << "字段 '" << diff.fieldName << "': " << diff.description;

    if (diff.existsInA && diff.existsInB) {
        ss << " (A: ";
        switch (diff.valueA.getType()) {
            case JsonValueType::String: ss << "\"" << diff.valueA.asString() << "\""; break;
            case JsonValueType::Int: ss << diff.valueA.asInt(); break;
            case JsonValueType::Double: ss << diff.valueA.asDouble(); break;
            case JsonValueType::Bool: ss << (diff.valueA.asBool() ? "true" : "false"); break;
            case JsonValueType::Null: ss << "null"; break;
        }
        ss << ", B: ";
        switch (diff.valueB.getType()) {
            case JsonValueType::String: ss << "\"" << diff.valueB.asString() << "\""; break;
            case JsonValueType::Int: ss << diff.valueB.asInt(); break;
            case JsonValueType::Double: ss << diff.valueB.asDouble(); break;
            case JsonValueType::Bool: ss << (diff.valueB.asBool() ? "true" : "false"); break;
            case JsonValueType::Null: ss << "null"; break;
        }
        ss << ")";
    } else if (diff.existsInA) {
        ss << " (仅在A中: ";
        switch (diff.valueA.getType()) {
            case JsonValueType::String: ss << "\"" << diff.valueA.asString() << "\""; break;
            case JsonValueType::Int: ss << diff.valueA.asInt(); break;
            case JsonValueType::Double: ss << diff.valueA.asDouble(); break;
            case JsonValueType::Bool: ss << (diff.valueA.asBool() ? "true" : "false"); break;
            case JsonValueType::Null: ss << "null"; break;
        }
        ss << ")";
    } else if (diff.existsInB) {
        ss << " (仅在B中: ";
        switch (diff.valueB.getType()) {
            case JsonValueType::String: ss << "\"" << diff.valueB.asString() << "\""; break;
            case JsonValueType::Int: ss << diff.valueB.asInt(); break;
            case JsonValueType::Double: ss << diff.valueB.asDouble(); break;
            case JsonValueType::Bool: ss << (diff.valueB.asBool() ? "true" : "false"); break;
            case JsonValueType::Null: ss << "null"; break;
        }
        ss << ")";
    }

    return ss.str();
}

// 执行详细比较功能
ComparisonResult StockDataComparator::compareDetailed() const {
    ComparisonResult result;
    result.enableDetailedComparison = true;

    // 基础统计
    result.countA = a_.size();
    result.countB = b_.size();

    if (result.countA == 0 && result.countB == 0) {
        result.identical = true;
        result.similarity = 1.0;
        result.summary = "两个数据集都为空";
        return result;
    }

    // 第一阶段：按简化键(code+indexValue)分组
    std::unordered_map<std::string, std::vector<size_t>> groupA, groupB;
    std::unordered_set<std::string> allKeys;

    // 收集A的分组
    for (size_t i = 0; i < a_.size(); ++i) {
        std::string key = generateRecordKey(a_[i]);
        groupA[key].push_back(i);
        allKeys.insert(key);
    }

    // 收集B的分组
    for (size_t i = 0; i < b_.size(); ++i) {
        std::string key = generateRecordKey(b_[i]);
        groupB[key].push_back(i);
        allKeys.insert(key);
    }

    // 第二阶段：逐组进行详细字段比较
    size_t exactMatches = 0;
    size_t totalRecords = 0;

    for (const auto& key : allKeys) {
        const auto& indicesA = groupA[key];
        const auto& indicesB = groupB[key];

        if (indicesA.empty() && indicesB.empty()) {
            continue; // 不应该发生
        } else if (indicesA.empty()) {
            // 仅在B中存在
            result.onlyInB += indicesB.size();
            totalRecords += indicesB.size();
        } else if (indicesB.empty()) {
            // 仅在A中存在
            result.onlyInA += indicesA.size();
            totalRecords += indicesA.size();
        } else {
            // 两边都有，进行详细字段比较
            std::vector<bool> matchedB(indicesB.size(), false);

            for (size_t idxA : indicesA) {
                bool foundMatch = false;
                for (size_t j = 0; j < indicesB.size(); ++j) {
                    if (!matchedB[j]) {
                        RecordComparisonDetail detail = compareRecordFieldsDetailed(a_[idxA], b_[indicesB[j]]);
                        result.totalComparedRecords++;

                        if (detail.identical) {
                            matchedB[j] = true;
                            foundMatch = true;
                            exactMatches++;
                            break;
                        } else {
                            // 记录详细差异
                            result.addRecordDifference(detail);
                            result.recordsWithDifferences++;
                        }
                    }
                }
                if (!foundMatch) {
                    result.onlyInA++;
                }
                totalRecords++;
            }

            // 处理B中未匹配的记录
            for (size_t j = 0; j < indicesB.size(); ++j) {
                if (!matchedB[j]) {
                    result.onlyInB++;
                    totalRecords++;
                }
            }
        }
    }

    result.common = exactMatches;

    // 计算相似度
    if (totalRecords == 0) {
        result.similarity = 1.0;
    } else {
        result.similarity = static_cast<double>(exactMatches) / totalRecords;
    }

    // 判断是否完全相同
    result.identical = (result.onlyInA == 0 && result.onlyInB == 0 &&
                       result.countA == result.countB && exactMatches == result.countA);

    // 计算索引值统计
    calculateStatistics(result);

    // 生成摘要
    generateSummary(result);

    return result;
}

// ComparisonResult的方法实现
void ComparisonResult::printDetailedDifferences() const {
    if (!enableDetailedComparison) {
        std::cout << "详细比较未启用" << std::endl;
        return;
    }

    if (detailedDifferences.empty()) {
        std::cout << "没有发现字段级差异" << std::endl;
        return;
    }

    // 聚合输出每条记录的比较结果
    for (size_t i = 0; i < detailedDifferences.size(); ++i) {
        const auto& detail = detailedDifferences[i];

        std::cout << "\nA侧原始记录 (行 " << i << "):" << std::endl;
        std::cout << "  " << detail.raw_json_a << std::endl;

        std::cout << "B侧原始记录 (行 " << i << "):" << std::endl;
        std::cout << "  " << detail.raw_json_b << std::endl;

        std::cout << std::endl;
        std::cout << "差异类型: " << (detail.identical ? "IDENTICAL" : "DIFF") << std::endl;
        std::cout << std::endl;

        if (!detail.identical && !detail.differences.empty()) {
            // 计算动态列宽
            ColumnWidths widths = calculateColumnWidths(detail.differences);

            std::cout << "字段差异详情 (" << detail.differences.size() << " 个字段):" << std::endl;

            // 输出表格标题（使用动态列宽）
            std::cout << std::left
                      << std::setw(widths.fieldName) << "字段名"
                      << std::setw(widths.diffType) << "差异类型"
                      << std::setw(widths.valueA) << "A侧值"
                      << "B侧值" << std::endl;

            // 输出分割线（根据总宽度）
            size_t totalWidth = widths.fieldName + widths.diffType + widths.valueA + widths.valueB;
            std::cout << std::string(totalWidth, '-') << std::endl;

            for (const auto& diff : detail.differences) {
                // 使用辅助函数获取格式化的值
                std::string diffType = getDiffTypeName(diff);
                std::string valueAStr = formatValueString(diff, true);
                std::string valueBStr = formatValueString(diff, false);

                // 输出表格化的行（使用动态列宽）
                std::cout << std::left
                          << std::setw(widths.fieldName) << diff.fieldName
                          << std::setw(widths.diffType) << diffType
                          << std::setw(widths.valueA) << valueAStr
                          << valueBStr << std::endl;
            }
            std::cout << std::endl;
        } else if (detail.identical) {
            std::cout << "记录完全相同，无字段差异。" << std::endl << std::endl;
        }
    }
}

std::string ComparisonResult::generateDifferenceReport() const {
    std::stringstream ss;

    if (!enableDetailedComparison) {
        ss << "详细比较未启用\n";
        return ss.str();
    }

    ss << "=== 详细差异报告 ===\n";
    ss << "总比较记录数: " << totalComparedRecords << "\n";
    ss << "有差异的记录数: " << recordsWithDifferences << "\n";
    ss << "详细差异条目数: " << detailedDifferences.size() << "\n";

    if (detailedDifferences.empty()) {
        ss << "没有发现字段级差异\n";
        return ss.str();
    }

    for (size_t i = 0; i < detailedDifferences.size(); ++i) {
        const auto& detail = detailedDifferences[i];
        ss << "\n--- 记录 " << (i+1) << " [" << detail.recordKey << "] ---\n";
        ss << "相同: " << (detail.identical ? "是" : "否") << "\n";

        if (!detail.differences.empty()) {
            ss << "字段差异详情:\n";
            for (const auto& diff : detail.differences) {
                ss << "  " << diff.description << "\n";
            }
        }
    }

    return ss.str();
}

void ComparisonResult::addRecordDifference(const RecordComparisonDetail& detail) {
    detailedDifferences.push_back(detail);
}