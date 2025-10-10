#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <string>
#include <exception>
#include <map>
#include <algorithm>
#include <ctime>
#include <sstream>
#include <cstring>
#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>
#include <chrono>
#include <dirent.h>
#include <unistd.h>
#include <set>
#include "StockDataComparator.h"
#include "StockDataBatchReader.h"
#include "FastCodeExtractor.h"

/**
 * JSON文件差异对比Demo
 * 功能：使用StockDataBatchReader读取两个JSON文件，使用StockDataComparator进行详细对比
 */

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

// 解析逗号分隔的字段列表
std::vector<std::string> parseIgnoreFields(const std::string& fieldsStr) {
    std::vector<std::string> fields;
    if (fieldsStr.empty()) {
        return fields;  // 空字符串返回空列表
    }

    std::stringstream ss(fieldsStr);
    std::string field;

    while (std::getline(ss, field, ',')) {
        // 去除前后空格
        field.erase(0, field.find_first_not_of(" \t"));
        field.erase(field.find_last_not_of(" \t") + 1);
        if (!field.empty()) {
            fields.push_back(field);
        }
    }
    return fields;
}


// 打印文件信息
void printFileInfo(const std::vector<StockDataContainer>& data, const std::string& filename) {
    std::cout << "\n=== " << filename << " 信息 ===" << std::endl;
    std::cout << "记录总数: " << data.size() << std::endl;

    if (data.empty()) {
        std::cout << "文件为空" << std::endl;
        return;
    }

    // 统计代码分布
    std::map<std::string, size_t> codeStats;
    std::map<std::string, size_t> timeStats;

    for (const auto& container : data) {
        codeStats[container.getCode()]++;
        timeStats[std::to_string(container.getIndexValue())]++;
    }

    std::cout << "股票代码分布: ";
    for (const auto& pair : codeStats) {
        std::cout << pair.first << "(" << pair.second << "条) ";
    }
    std::cout << std::endl;

    std::cout << "时间分布: ";
    for (const auto& pair : timeStats) {
        std::cout << pair.first << "(" << pair.second << "条) ";
    }
    std::cout << std::endl;

    // 显示前3条记录示例
    std::cout << "记录示例:" << std::endl;
    for (size_t i = 0; i < std::min(data.size(), size_t(3)); ++i) {
        const auto& container = data[i];
        std::cout << "  [" << (i+1) << "] code=" << container.getCode()
                  << ", time=" << container.getIndexValue();

        // 尝试显示价格信息
        try {
            if (container.hasField("price")) {
                auto price = container.getFieldSafe("price");
                if (price.getType() == JsonValueType::Double) {
                    std::cout << ", price=" << std::fixed << std::setprecision(2) << price.asDouble();
                } else if (price.getType() == JsonValueType::String) {
                    std::cout << ", price=\"" << price.asString() << "\"";
                }
            }
        } catch (...) {
            // 忽略价格获取错误
        }

        std::cout << std::endl;
    }
}

// 打印比较结果摘要
void printComparisonSummary(const ComparisonResult& result) {
    std::cout << "\n=== 比较结果摘要 ===" << std::endl;
    std::cout << "数据集A记录数: " << result.countA << std::endl;
    std::cout << "数据集B记录数: " << result.countB << std::endl;
    std::cout << "仅在A中存在: " << result.onlyInA << " 条记录" << std::endl;
    std::cout << "仅在B中存在: " << result.onlyInB << " 条记录" << std::endl;
    std::cout << "共同存在: " << result.common << " 条记录" << std::endl;

    if (result.enableDetailedComparison) {
        std::cout << "详细比较已启用:" << std::endl;
        std::cout << "  总比较记录数: " << result.totalComparedRecords << std::endl;
        std::cout << "  有差异的记录数: " << result.recordsWithDifferences << std::endl;
        std::cout << "  详细差异条目数: " << result.detailedDifferences.size() << std::endl;
    }

    std::cout << "完全相同: " << (result.identical ? "是" : "否") << std::endl;
    std::cout << "相似度: " << std::fixed << std::setprecision(2) << (result.similarity * 100) << "%" << std::endl;

    if (!result.summary.empty()) {
        std::cout << "摘要: " << result.summary << std::endl;
    }
}

// 从 recordKey 提取 code（下划线分隔符之前的部分）
std::string extractCodeFromRecordKey(const std::string& recordKey) {
    size_t underscorePos = recordKey.find('_');
    if (underscorePos != std::string::npos) {
        return recordKey.substr(0, underscorePos);
    }
    return recordKey;  // 如果没有找到下划线，返回原字符串作为回退
}

// 验证文件是否存在且可读
bool validateFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.good()) {
        std::cerr << "❌ 文件不存在或无法读取: " << filePath << std::endl;
        return false;
    }
    file.close();
    return true;
}

// 创建输出目录
bool createOutputDirectory(const std::string& dirPath) {
    // 检查目录是否已存在
    struct stat st;
    if (stat(dirPath.c_str(), &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return true;  // 目录已存在
        } else {
            std::cerr << "❌ 路径已存在但不是目录: " << dirPath << std::endl;
            return false;
        }
    }

    // 尝试创建目录
    if (mkdir(dirPath.c_str(), 0755) == 0) {
        std::cout << "✓ 创建输出目录: " << dirPath << std::endl;
        return true;
    } else {
        std::cerr << "❌ 创建目录失败: " << dirPath << std::endl;
        return false;
    }
}

// 为特定 code 生成差异报告
std::string generateCodeDifferenceReport(const std::string& code,
                                       const std::vector<RecordComparisonDetail>& codeRecords) {
    std::ostringstream report;

    // 获取当前时间
    time_t now = time(0);
    char* timeStr = ctime(&now);
    if (timeStr[strlen(timeStr)-1] == '\n') {
        timeStr[strlen(timeStr)-1] = '\0';  // 移除换行符
    }

    // 报告头部
    report << "股票代码: " << code << std::endl;
    report << "差异记录总数: " << codeRecords.size() << std::endl;
    report << "生成时间: " << timeStr << std::endl;
    report << std::endl;

    // 遍历该 code 的所有记录
    int recordIndex = 0;
    for (const auto& record : codeRecords) {
        report << "========================" << std::endl;
        report << std::endl;

        // 显示原始JSON记录
        report << "A侧原始记录 (行 " << recordIndex << "):" << std::endl;
        report << "  " << record.raw_json_a << std::endl;

        report << "B侧原始记录 (行 " << recordIndex << "):" << std::endl;
        report << "  " << record.raw_json_b << std::endl;

        report << std::endl;
        report << "差异类型: " << (record.identical ? "IDENTICAL" : "DIFF") << std::endl;
        report << std::endl;

        // 从 recordKey 提取时间部分（下划线分隔符之后的部分）
        std::string timeValue = "未知";
        size_t underscorePos = record.recordKey.find('_');
        if (underscorePos != std::string::npos && underscorePos + 1 < record.recordKey.length()) {
            timeValue = record.recordKey.substr(underscorePos + 1);
        }
        report << "记录标识: " << record.recordKey << std::endl;
        report << "时间标识: " << timeValue << std::endl;
        report << std::endl;

        if (!record.identical && !record.differences.empty()) {
            // 计算动态列宽
            ColumnWidths widths = calculateColumnWidths(record.differences);

            report << "字段差异详情 (" << record.differences.size() << " 个字段):" << std::endl;

            // 输出表格标题（使用动态列宽）
            char headerBuffer[1024];
            snprintf(headerBuffer, sizeof(headerBuffer), "%-*s%-*s%-*s%s",
                     static_cast<int>(widths.fieldName), "字段名",
                     static_cast<int>(widths.diffType), "差异类型",
                     static_cast<int>(widths.valueA), "A侧值",
                     "B侧值");
            report << headerBuffer << std::endl;

            // 输出分割线（根据总宽度）
            size_t totalWidth = widths.fieldName + widths.diffType + widths.valueA + widths.valueB;
            report << std::string(totalWidth, '-') << std::endl;

            for (const auto& diff : record.differences) {
                // 使用辅助函数获取格式化的值
                std::string diffType = getDiffTypeName(diff);
                std::string valueAStr = formatValueString(diff, true);
                std::string valueBStr = formatValueString(diff, false);

                // 输出表格化的行（使用动态列宽）
                char buffer[1024];
                snprintf(buffer, sizeof(buffer), "%-*s%-*s%-*s%s",
                         static_cast<int>(widths.fieldName), diff.fieldName.c_str(),
                         static_cast<int>(widths.diffType), diffType.c_str(),
                         static_cast<int>(widths.valueA), valueAStr.c_str(),
                         valueBStr.c_str());
                report << buffer << std::endl;
            }
            report << std::endl;
        } else if (record.identical) {
            report << "记录完全相同，无字段差异。" << std::endl;
            report << std::endl;
        }

        recordIndex++;
    }

    report << "========================" << std::endl;

    return report.str();
}

// 按 code 分组差异并写入文件
void writeDifferencesToFiles(const ComparisonResult& result, const std::string& outputDir) {
    std::cout << "\n=== 写入差异文件 ===" << std::endl;

    // 创建输出目录
    if (!createOutputDirectory(outputDir)) {
        std::cerr << "❌ 无法创建输出目录，跳过文件写入" << std::endl;
        return;
    }

    // 按 code 分组差异记录
    std::map<std::string, std::vector<RecordComparisonDetail>> codeGroups;

    for (const auto& detail : result.detailedDifferences) {
        std::string code = extractCodeFromRecordKey(detail.recordKey);
        codeGroups[code].push_back(detail);
    }

    if (codeGroups.empty()) {
        std::cout << "✓ 没有差异记录需要写入文件" << std::endl;
        return;
    }

    // 为每个 code 写入文件
    int filesWritten = 0;
    int totalRecords = 0;

    for (const auto& pair : codeGroups) {
        const std::string& code = pair.first;
        const std::vector<RecordComparisonDetail>& records = pair.second;

        // 生成文件名
        std::string filename = outputDir + "/" + code + ".txt";

        // 生成报告内容
        std::string reportContent = generateCodeDifferenceReport(code, records);

        // 写入文件
        std::ofstream outFile(filename);
        if (outFile.is_open()) {
            outFile << reportContent;
            outFile.close();

            std::cout << "✓ 写入差异文件: " << filename
                      << " (" << records.size() << " 条记录)" << std::endl;

            filesWritten++;
            totalRecords += records.size();
        } else {
            std::cerr << "❌ 无法创建文件: " << filename << std::endl;
        }
    }

    std::cout << "✓ 差异文件写入完成: " << filesWritten << " 个文件，总计 "
              << totalRecords << " 条差异记录" << std::endl;
}

// 获取批次的时间戳数值
int64_t getBatchTimestamp(const std::vector<StockDataContainer>& batch, const StockDataBatchReader& reader) {
    if (batch.empty()) return 0;
    return batch[0].getIndexValue();
}

// 覆盖式输出辅助函数
void printProgressOverwrite(const std::string& message) {
    std::cout << "\r" << std::string(80, ' ') << "\r" << message << std::flush;
}

// 为单条记录生成差异报告
std::string generateSingleRecordReport(const RecordComparisonDetail& detail) {
    std::ostringstream report;

    report << "========================" << std::endl;
    report << std::endl;

    // 显示原始JSON记录
    report << "A侧原始记录:" << std::endl;
    report << "  " << detail.raw_json_a << std::endl;

    report << "B侧原始记录:" << std::endl;
    report << "  " << detail.raw_json_b << std::endl;

    report << std::endl;
    report << "差异类型: " << (detail.identical ? "IDENTICAL" : "DIFF") << std::endl;
    report << std::endl;

    // 从 recordKey 提取时间部分
    std::string timeValue = "未知";
    size_t underscorePos = detail.recordKey.find('_');
    if (underscorePos != std::string::npos && underscorePos + 1 < detail.recordKey.length()) {
        timeValue = detail.recordKey.substr(underscorePos + 1);
    }
    report << "记录标识: " << detail.recordKey << std::endl;
    report << "时间标识: " << timeValue << std::endl;
    report << std::endl;

    if (!detail.identical && !detail.differences.empty()) {
        // 计算动态列宽
        ColumnWidths widths = calculateColumnWidths(detail.differences);

        report << "字段差异详情 (" << detail.differences.size() << " 个字段):" << std::endl;

        // 输出表格标题
        char headerBuffer[1024];
        snprintf(headerBuffer, sizeof(headerBuffer), "%-*s%-*s%-*s%s",
                 static_cast<int>(widths.fieldName), "字段名",
                 static_cast<int>(widths.diffType), "差异类型",
                 static_cast<int>(widths.valueA), "A侧值",
                 "B侧值");
        report << headerBuffer << std::endl;

        // 输出分割线
        size_t totalWidth = widths.fieldName + widths.diffType + widths.valueA + widths.valueB;
        report << std::string(totalWidth, '-') << std::endl;

        for (const auto& diff : detail.differences) {
            std::string diffType = getDiffTypeName(diff);
            std::string valueAStr = formatValueString(diff, true);
            std::string valueBStr = formatValueString(diff, false);

            char buffer[1024];
            snprintf(buffer, sizeof(buffer), "%-*s%-*s%-*s%s",
                     static_cast<int>(widths.fieldName), diff.fieldName.c_str(),
                     static_cast<int>(widths.diffType), diffType.c_str(),
                     static_cast<int>(widths.valueA), valueAStr.c_str(),
                     valueBStr.c_str());
            report << buffer << std::endl;
        }
        report << std::endl;
    } else if (detail.identical) {
        report << "记录完全相同，无字段差异。" << std::endl;
        report << std::endl;
    }

    return report.str();
}

// 流式写入单条记录到文件
void writeRecordToFile(const RecordComparisonDetail& detail, const std::string& outputDir) {
    std::string code = extractCodeFromRecordKey(detail.recordKey);
    std::string filename = outputDir + "/" + code + ".txt";

    // 检查文件是否是第一次写入（添加头部信息）
    bool isNewFile = false;
    std::ifstream checkFile(filename);
    if (!checkFile.good()) {
        isNewFile = true;
    }
    checkFile.close();

    // 以追加模式写入文件
    std::ofstream file(filename, std::ios::app);
    if (file.is_open()) {
        if (isNewFile) {
            // 写入文件头部信息
            time_t now = time(0);
            char* timeStr = ctime(&now);
            if (timeStr[strlen(timeStr)-1] == '\n') {
                timeStr[strlen(timeStr)-1] = '\0';
            }

            file << "股票代码: " << code << std::endl;
            file << "生成时间: " << timeStr << std::endl;
            file << "说明: 流式对比结果（按时间戳顺序）" << std::endl;
            file << std::endl;
        }

        // 写入单条记录
        file << generateSingleRecordReport(detail);
        file.close();
    }
}


// 处理仅在B中存在的数据（A中缺失）
void processMissInA(const std::vector<StockDataContainer>& batchB, const std::string& outputDir, double tolerance) {
    StockDataComparator comparator;
    comparator.setTolerance(tolerance);
    for (const auto& recordB : batchB) {
        RecordComparisonDetail detail = comparator.createMissRecord(recordB, true);
        writeRecordToFile(detail, outputDir);
    }
}

// 处理仅在A中存在的数据（B中缺失）
void processMissInB(const std::vector<StockDataContainer>& batchA, const std::string& outputDir, double tolerance) {
    StockDataComparator comparator;
    comparator.setTolerance(tolerance);
    for (const auto& recordA : batchA) {
        RecordComparisonDetail detail = comparator.createMissRecord(recordA, false);
        writeRecordToFile(detail, outputDir);
    }
}

// 处理时间戳匹配的正常对比
void processMatching(const std::vector<StockDataContainer>& batchA,
                    const std::vector<StockDataContainer>& batchB,
                    const std::string& outputDir, double tolerance) {
    // 使用现有的 StockDataComparator 进行字段级对比
    StockDataComparator comparator;
    comparator.setTolerance(tolerance);
    comparator.setDataA(batchA);
    comparator.setDataB(batchB);

    auto result = comparator.compareDetailed();

    // 流式写入每条差异记录
    for (const auto& detail : result.detailedDifferences) {
        writeRecordToFile(detail, outputDir);
    }
}

// ============================================================
// 前向声明
// ============================================================
void demonstrateJsonFileComparison(
    const std::string& fileA,
    const std::string& fileB,
    const std::string& outputDir,
    const std::vector<std::string>& ignoreFields,
    int64_t indexDecimal,
    double tolerance,
    const std::string& compareKey = "");

// ============================================================
// 分组模式相关辅助函数
// ============================================================

// 生成唯一时间戳字符串
std::string generateTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return std::to_string(ms);
}

// 检查文件是否存在
bool fileExists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

// 从目录中获取所有code列表
std::vector<std::string> getCodeListFromDirectory(const std::string& dir) {
    std::vector<std::string> codes;

    DIR* dp = opendir(dir.c_str());
    if (!dp) {
        std::cerr << "❌ 无法打开目录: " << dir << std::endl;
        return codes;
    }

    struct dirent* entry;
    while ((entry = readdir(dp)) != nullptr) {
        std::string filename = entry->d_name;

        // 跳过 . 和 ..
        if (filename == "." || filename == "..") {
            continue;
        }

        // 检查是否是 .txt 文件
        if (filename.size() > 4 && filename.substr(filename.size() - 4) == ".txt") {
            // 提取 code（去掉 .txt 后缀）
            std::string code = filename.substr(0, filename.size() - 4);
            codes.push_back(code);
        }
    }

    closedir(dp);
    return codes;
}

// 合并两个code列表并去重
std::vector<std::string> mergeCodes(const std::vector<std::string>& codesA,
                                    const std::vector<std::string>& codesB) {
    std::set<std::string> uniqueCodes;

    for (const auto& code : codesA) {
        uniqueCodes.insert(code);
    }
    for (const auto& code : codesB) {
        uniqueCodes.insert(code);
    }

    return std::vector<std::string>(uniqueCodes.begin(), uniqueCodes.end());
}

// 递归删除目录
void removeDirectory(const std::string& dir) {
    DIR* dp = opendir(dir.c_str());
    if (!dp) {
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dp)) != nullptr) {
        std::string filename = entry->d_name;

        if (filename == "." || filename == "..") {
            continue;
        }

        std::string fullPath = dir + "/" + filename;

        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                removeDirectory(fullPath);  // 递归删除子目录
            } else {
                unlink(fullPath.c_str());   // 删除文件
            }
        }
    }

    closedir(dp);
    rmdir(dir.c_str());  // 删除空目录
}

// 使用FastCodeExtractor分组文件
void splitInputFilesByCode(const std::string& fileA,
                          const std::string& fileB,
                          const std::string& splitDirA,
                          const std::string& splitDirB) {
    std::cout << "\n--- 按 code 分组输入文件 ---" << std::endl;

    // 分组文件A
    std::cout << "分组文件A: " << fileA << " → " << splitDirA << std::endl;
    auto statsA = FastCodeExtractor::splitByCode(fileA, splitDirA);
    std::cout << "  ✓ 文件A分组完成: " << statsA.uniqueCodes << " 个 code, "
              << statsA.successLines << " 条记录" << std::endl;

    // 分组文件B
    std::cout << "分组文件B: " << fileB << " → " << splitDirB << std::endl;
    auto statsB = FastCodeExtractor::splitByCode(fileB, splitDirB);
    std::cout << "  ✓ 文件B分组完成: " << statsB.uniqueCodes << " 个 code, "
              << statsB.successLines << " 条记录" << std::endl;
}

// 处理仅在B中存在的code（全部记录为MISS_IN_A）
void processCodeMissInA(const std::string& codeFileB,
                        const std::string& outputFile,
                        double tolerance,
                        const std::string& compareKey) {
    // 使用现有的流式处理逻辑，但所有记录都标记为MISS_IN_A
    try {
        StockDataBatchReader readerB(codeFileB, "time", 1, {}, compareKey);
        StockDataComparator comparator;
        comparator.setTolerance(tolerance);

        std::vector<StockDataContainer> batchB;

        while (readerB.popBatch(batchB)) {
            for (const auto& recordB : batchB) {
                RecordComparisonDetail detail = comparator.createMissRecord(recordB, true);
                writeRecordToFile(detail, outputFile);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "处理 MISS_IN_A 时出错: " << e.what() << std::endl;
    }
}

// 处理仅在A中存在的code（全部记录为MISS_IN_B）
void processCodeMissInB(const std::string& codeFileA,
                        const std::string& outputFile,
                        double tolerance,
                        const std::string& compareKey) {
    // 使用现有的流式处理逻辑，但所有记录都标记为MISS_IN_B
    try {
        StockDataBatchReader readerA(codeFileA, "time", 1, {}, compareKey);
        StockDataComparator comparator;
        comparator.setTolerance(tolerance);

        std::vector<StockDataContainer> batchA;

        while (readerA.popBatch(batchA)) {
            for (const auto& recordA : batchA) {
                RecordComparisonDetail detail = comparator.createMissRecord(recordA, false);
                writeRecordToFile(detail, outputFile);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "处理 MISS_IN_B 时出错: " << e.what() << std::endl;
    }
}

// 比较单个code的两个文件
void compareCodeFiles(const std::string& codeFileA,
                     const std::string& codeFileB,
                     const std::string& outputFile,
                     const std::string& outputDir,
                     const std::vector<std::string>& ignoreFields,
                     int64_t indexDecimal,
                     double tolerance,
                     const std::string& compareKey) {
    bool hasA = fileExists(codeFileA);
    bool hasB = fileExists(codeFileB);

    if (!hasA && !hasB) {
        // 两个文件都不存在，跳过
        return;
    }

    if (!hasA && hasB) {
        // 只有B存在，全部标记为MISS_IN_A
        processCodeMissInA(codeFileB, outputFile, tolerance, compareKey);
    } else if (hasA && !hasB) {
        // 只有A存在，全部标记为MISS_IN_B
        processCodeMissInB(codeFileA, outputFile, tolerance, compareKey);
    } else {
        // 两个文件都存在，使用全量加载模式（分组后文件通常很小）
        try {
            // 1. 全量加载两个文件
            auto dataA = FullFileLoader::loadAllRecords(
                codeFileA, "time", indexDecimal, ignoreFields, compareKey,
                500  // 单个code文件限制500MB
            );
            auto dataB = FullFileLoader::loadAllRecords(
                codeFileB, "time", indexDecimal, ignoreFields, compareKey,
                500
            );

            // 2. 创建比较器
            StockDataComparator comparator;
            comparator.setTolerance(tolerance);
            comparator.setDataA(std::move(dataA));
            comparator.setDataB(std::move(dataB));

            // 3. 执行详细比较
            auto result = comparator.compareDetailed();

            // 4. 流式写入差异记录
            for (const auto& detail : result.detailedDifferences) {
                writeRecordToFile(detail, outputDir);
            }

        } catch (const std::exception& e) {
            std::cerr << "❌ 比较code文件失败: " << e.what() << std::endl;
        }
    }
}

// 分组模式的主比较函数
void demonstrateGroupedComparison(const std::string& fileA,
                                 const std::string& fileB,
                                 const std::string& outputDir,
                                 const std::vector<std::string>& ignoreFields,
                                 int64_t indexDecimal,
                                 double tolerance,
                                 bool keepSplitFiles,
                                 const std::string& compareKey) {
    try {
        // 创建输出目录
        if (!createOutputDirectory(outputDir)) {
            std::cerr << "❌ 无法创建输出目录: " << outputDir << std::endl;
            return;
        }

        // 1. 创建临时目录
        std::string timestamp = generateTimestamp();
        std::string splitDirA = "/tmp/.json_compare_" + timestamp + "_a";
        std::string splitDirB = "/tmp/.json_compare_" + timestamp + "_b";

        std::cout << "临时目录A: " << splitDirA << std::endl;
        std::cout << "临时目录B: " << splitDirB << std::endl;

        // 2. 分组文件
        splitInputFilesByCode(fileA, fileB, splitDirA, splitDirB);

        // 3. 获取所有唯一code
        std::cout << "\n--- 获取code列表 ---" << std::endl;
        auto codesA = getCodeListFromDirectory(splitDirA);
        auto codesB = getCodeListFromDirectory(splitDirB);
        auto allCodes = mergeCodes(codesA, codesB);

        std::cout << "文件A中的code数: " << codesA.size() << std::endl;
        std::cout << "文件B中的code数: " << codesB.size() << std::endl;
        std::cout << "总共唯一code数: " << allCodes.size() << std::endl;

        // 4. 逐个code比较
        std::cout << "\n--- 开始逐个code比较 ---" << std::endl;

        int processedCodes = 0;
        int totalOnlyInA = 0;
        int totalOnlyInB = 0;
        int totalBoth = 0;

        for (const auto& code : allCodes) {
            processedCodes++;

            std::string codeFileA = splitDirA + "/" + code + ".txt";
            std::string codeFileB = splitDirB + "/" + code + ".txt";
            std::string codeOutput = outputDir + "/" + code + ".txt";

            bool hasA = fileExists(codeFileA);
            bool hasB = fileExists(codeFileB);

            std::cout << "\r处理中: [" << processedCodes << "/" << allCodes.size() << "] code=" << code;

            if (hasA && hasB) {
                totalBoth++;
            } else if (hasA) {
                totalOnlyInA++;
            } else if (hasB) {
                totalOnlyInB++;
            }

            std::cout << std::flush;

            // 比较当前code的文件
            compareCodeFiles(codeFileA, codeFileB, codeOutput, outputDir, ignoreFields, indexDecimal, tolerance, compareKey);
        }

        std::cout << std::endl;  // 换行

        // 5. 显示统计信息
        std::cout << "\n--- 分组比较完成 ---" << std::endl;
        std::cout << "总共处理code数: " << allCodes.size() << std::endl;
        std::cout << "两侧都存在: " << totalBoth << " 个code" << std::endl;
        std::cout << "仅在A中存在: " << totalOnlyInA << " 个code" << std::endl;
        std::cout << "仅在B中存在: " << totalOnlyInB << " 个code" << std::endl;
        std::cout << "✓ 差异结果已写入目录: " << outputDir << std::endl;

        // 6. 清理临时文件
        if (!keepSplitFiles) {
            std::cout << "\n--- 清理临时文件 ---" << std::endl;
            std::cout << "删除临时目录: " << splitDirA << std::endl;
            removeDirectory(splitDirA);
            std::cout << "删除临时目录: " << splitDirB << std::endl;
            removeDirectory(splitDirB);
        } else {
            std::cout << "\n--- 保留临时文件 ---" << std::endl;
            std::cout << "临时文件保留在: " << splitDirA << std::endl;
            std::cout << "临时文件保留在: " << splitDirB << std::endl;
        }

        std::cout << "\n✅ 分组模式JSON文件对比完成!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "❌ 分组对比过程中发生错误: " << e.what() << std::endl;
    }
}

// ============================================================
// 原有流式模式函数
// ============================================================

// 演示JSON文件对比功能（流式处理大文件）
void demonstrateJsonFileComparison(const std::string& fileA, const std::string& fileB, const std::string& outputDir, const std::vector<std::string>& ignoreFields, int64_t indexDecimal, double tolerance, const std::string& compareKey) {
    try {
        // 创建输出目录
        if (!createOutputDirectory(outputDir)) {
            std::cerr << "❌ 无法创建输出目录: " << outputDir << std::endl;
            return;
        }

        std::cout << "\n--- 初始化流式读取器 ---" << std::endl;

        // 创建两个流式读取器
        StockDataBatchReader readerA(fileA, "time", indexDecimal, ignoreFields, compareKey);
        StockDataBatchReader readerB(fileB, "time", indexDecimal, ignoreFields, compareKey);

        std::cout << "✓ 文件A读取器已创建: " << fileA << std::endl;
        std::cout << "✓ 文件B读取器已创建: " << fileB << std::endl;
        std::cout << "✓ 输出目录: " << outputDir << std::endl;

        std::cout << "\n--- 开始流式对比 ---" << std::endl;

        // 双游标流式对比主循环
        std::vector<StockDataContainer> batchA, batchB;
        bool hasMoreA = readerA.popBatch(batchA);
        bool hasMoreB = readerB.popBatch(batchB);

        int batchCounter = 0;
        int totalProcessedA = 0;
        int totalProcessedB = 0;
        int totalMissInA = 0;
        int totalMissInB = 0;
        int totalMatched = 0;

        while (hasMoreA || hasMoreB) {
            batchCounter++;

            if (!hasMoreA) {
                // A文件结束，B剩余数据为MISS_IN_A
                printProgressOverwrite("批次 " + std::to_string(batchCounter) + ": B剩余数据 " + std::to_string(batchB.size()) + " 条（A中缺失）");
                processMissInA(batchB, outputDir, tolerance);
                totalMissInA += batchB.size();
                totalProcessedB += batchB.size();
                hasMoreB = readerB.popBatch(batchB);

            } else if (!hasMoreB) {
                // B文件结束，A剩余数据为MISS_IN_B
                printProgressOverwrite("批次 " + std::to_string(batchCounter) + ": A剩余数据 " + std::to_string(batchA.size()) + " 条（B中缺失）");
                processMissInB(batchA, outputDir, tolerance);
                totalMissInB += batchA.size();
                totalProcessedA += batchA.size();
                hasMoreA = readerA.popBatch(batchA);

            } else {
                // 使用 convertIndexToComparableValue 比较时间戳
                int64_t timeA = getBatchTimestamp(batchA, readerA);
                int64_t timeB = getBatchTimestamp(batchB, readerB);

                if (timeA < timeB) {
                    // A的时间戳更早，在B中缺失
                    printProgressOverwrite("批次 " + std::to_string(batchCounter) + ": 时间戳 " + std::to_string(timeA) + " (" + std::to_string(batchA.size()) + " 条记录，B中缺失）");
                    processMissInB(batchA, outputDir, tolerance);
                    totalMissInB += batchA.size();
                    totalProcessedA += batchA.size();
                    hasMoreA = readerA.popBatch(batchA);

                } else if (timeA > timeB) {
                    // B的时间戳更早，在A中缺失
                    printProgressOverwrite("批次 " + std::to_string(batchCounter) + ": 时间戳 " + std::to_string(timeB) + " (" + std::to_string(batchB.size()) + " 条记录，A中缺失）");
                    processMissInA(batchB, outputDir, tolerance);
                    totalMissInA += batchB.size();
                    totalProcessedB += batchB.size();
                    hasMoreB = readerB.popBatch(batchB);

                } else {
                    // 时间戳相同，进行正常字段对比
                    printProgressOverwrite("批次 " + std::to_string(batchCounter) + ": 时间戳 " + std::to_string(timeA) + " (A:" + std::to_string(batchA.size()) + " 条, B:" + std::to_string(batchB.size()) + " 条，对比中...）");
                    processMatching(batchA, batchB, outputDir, tolerance);
                    totalMatched += batchA.size();
                    totalProcessedA += batchA.size();
                    totalProcessedB += batchB.size();
                    hasMoreA = readerA.popBatch(batchA);
                    hasMoreB = readerB.popBatch(batchB);
                }
            }
        }

        // 覆盖式输出结束后换行，确保后续信息正常显示
        std::cout << std::endl;

        std::cout << "\n--- 流式对比完成 ---" << std::endl;
        std::cout << "总共处理批次: " << batchCounter << std::endl;
        std::cout << "文件A总记录数: " << totalProcessedA << std::endl;
        std::cout << "文件B总记录数: " << totalProcessedB << std::endl;
        std::cout << "时间戳匹配记录: " << totalMatched << std::endl;
        std::cout << "仅在A中存在: " << totalMissInB << " 条记录" << std::endl;
        std::cout << "仅在B中存在: " << totalMissInA << " 条记录" << std::endl;

        // 计算相似度
        int totalRecords = totalProcessedA + totalProcessedB;
        if (totalRecords > 0) {
            double similarity = (double)(totalMatched * 2) / totalRecords * 100.0;
            std::cout << "相似度: " << std::fixed << std::setprecision(2) << similarity << "%" << std::endl;
        }

        std::cout << "✓ 差异结果已写入目录: " << outputDir << std::endl;
        std::cout << "\n✅ 流式JSON文件对比完成!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "❌ 流式对比过程中发生错误: " << e.what() << std::endl;
    }
}

// 显示帮助信息
void printUsage(const char* programName) {
    std::cout << "用法: " << programName << " -a <文件A路径> -b <文件B路径> [选项]" << std::endl;
    std::cout << std::endl;
    std::cout << "必需参数:" << std::endl;
    std::cout << "  -a <文件路径>  指定第一个JSON文件的绝对路径" << std::endl;
    std::cout << "  -b <文件路径>  指定第二个JSON文件的绝对路径" << std::endl;
    std::cout << std::endl;
    std::cout << "可选参数:" << std::endl;
    std::cout << "  -o <目录路径>      指定差异文件的输出目录，默认: ./diff_output/" << std::endl;
    std::cout << "  -f <字段列表>      指定需要忽略的字段，用逗号分隔（可选）" << std::endl;
    std::cout << "  -decimal <精度值>  指定时间戳索引精度，默认: 1" << std::endl;
    std::cout << "  -t <容差值>        指定浮点数比较容差，默认: 1e-9" << std::endl;
    std::cout << "  -index <字段名>    指定额外的比较键字段（用于精确匹配）" << std::endl;
    std::cout << "  -split             启用分组模式（先按code分组，再比较）" << std::endl;
    std::cout << "  --keep-split       保留临时分组文件（用于调试）" << std::endl;
    std::cout << "  -h, --help         显示此帮助信息" << std::endl;
    std::cout << std::endl;
    std::cout << "对比模式:" << std::endl;
    std::cout << "  流式模式（默认）  按时间戳顺序流式对比，速度快，内存低" << std::endl;
    std::cout << "  分组模式（-split）先将文件按code完全分组，再逐个code对比" << std::endl;
    std::cout << "                     - 优势：code数据完全隔离，易于调试" << std::endl;
    std::cout << "                     - 劣势：需要额外I/O和磁盘空间" << std::endl;
    std::cout << std::endl;
    std::cout << "示例:" << std::endl;
    std::cout << "  " << programName << " -a /path/to/fileA.json -b /path/to/fileB.json" << std::endl;
    std::cout << "  " << programName << " -a data1.json -b data2.json -o /tmp/diff_result" << std::endl;
    std::cout << "  " << programName << " -a data1.json -b data2.json -f timestamp,debug_info" << std::endl;
    std::cout << "  " << programName << " -a data1.json -b data2.json -index order_id" << std::endl;
    std::cout << "  " << programName << " -a data1.json -b data2.json -index level -decimal 1000" << std::endl;
    std::cout << "  " << programName << " -a data1.json -b data2.json -split  # 分组模式" << std::endl;
    std::cout << "  " << programName << " -a data1.json -b data2.json -split --keep-split" << std::endl;
    std::cout << "  " << programName << " -a data1.json -b data2.json -decimal 1000" << std::endl;
    std::cout << "  " << programName << " -a data1.json -b data2.json -t 0.001" << std::endl;
    std::cout << "  " << programName << " --help" << std::endl;
    std::cout << std::endl;
    std::cout << "功能: JSON文件批量读取 + 详细差异对比 + 按股票代码分组输出" << std::endl;
}

// 主函数 - 演示不同的测试场景
int main(int argc, char* argv[]) {
    // 记录程序开始时间
    auto startTime = std::chrono::high_resolution_clock::now();

    // 参数解析
    std::string fileA;
    std::string fileB;
    std::string outputDir = "./diff_output";  // 默认输出目录
    std::vector<std::string> ignoreFields;    // 默认为空，表示不过滤
    int64_t indexDecimal = 1;                 // 默认索引精度为1
    double tolerance = 1e-9;                  // 默认浮点数比较容差
    std::string compareKey;                   // 比较键字段名（默认为空）
    bool enableSplit = false;                 // 是否启用分组模式
    bool keepSplitFiles = false;              // 是否保留分组文件

    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help" || arg == "-help") {
            printUsage(argv[0]);
            return 0;
        }
        else if (arg == "-a") {
            if (i + 1 >= argc) {
                std::cerr << "❌ 参数 -a 需要指定文件路径!" << std::endl;
                printUsage(argv[0]);
                return 1;
            }
            fileA = argv[++i];
        }
        else if (arg == "-b") {
            if (i + 1 >= argc) {
                std::cerr << "❌ 参数 -b 需要指定文件路径!" << std::endl;
                printUsage(argv[0]);
                return 1;
            }
            fileB = argv[++i];
        }
        else if (arg == "-o") {
            if (i + 1 >= argc) {
                std::cerr << "❌ 参数 -o 需要指定输出目录!" << std::endl;
                printUsage(argv[0]);
                return 1;
            }
            outputDir = argv[++i];
        }
        else if (arg == "-f") {
            if (i + 1 >= argc) {
                std::cerr << "❌ 参数 -f 需要指定过滤字段列表!" << std::endl;
                printUsage(argv[0]);
                return 1;
            }
            std::string fieldsStr = argv[++i];
            ignoreFields = parseIgnoreFields(fieldsStr);

            // 显示解析到的字段
            if (!ignoreFields.empty()) {
                std::cout << std::endl;
                std::cout << std::endl;
                std::cout << "✓ 将过滤字段: ";
                for (size_t j = 0; j < ignoreFields.size(); ++j) {
                    if (j > 0) std::cout << ", ";
                    std::cout << ignoreFields[j];
                }
                std::cout << std::endl;
            }
        }
        else if (arg == "-decimal") {
            if (i + 1 >= argc) {
                std::cerr << "❌ 参数 -decimal 需要指定精度值!" << std::endl;
                printUsage(argv[0]);
                return 1;
            }
            try {
                indexDecimal = std::stoll(argv[++i]);
                if (indexDecimal <= 0) {
                    std::cerr << "❌ decimal 精度值必须为正整数!" << std::endl;
                    return 1;
                }
            } catch (const std::exception& e) {
                std::cerr << "❌ decimal 参数格式错误: " << argv[i] << std::endl;
                return 1;
            }
        }
        else if (arg == "-t") {
            if (i + 1 >= argc) {
                std::cerr << "❌ 参数 -t 需要指定容差值!" << std::endl;
                printUsage(argv[0]);
                return 1;
            }
            try {
                tolerance = std::stod(argv[++i]);
                if (tolerance <= 0.0) {
                    std::cerr << "❌ 容差值必须为正数!" << std::endl;
                    return 1;
                }
            } catch (const std::exception& e) {
                std::cerr << "❌ 容差参数格式错误: " << argv[i] << std::endl;
                return 1;
            }
        }
        else if (arg == "-index") {
            if (i + 1 >= argc) {
                std::cerr << "❌ 参数 -index 需要指定字段名!" << std::endl;
                printUsage(argv[0]);
                return 1;
            }
            compareKey = argv[++i];
        }
        else if (arg == "-split" || arg == "--group-by-code") {
            enableSplit = true;
        }
        else if (arg == "--keep-split") {
            keepSplitFiles = true;
        }
        else {
            std::cerr << "❌ 未知参数: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }

    // 验证必需参数
    if (fileA.empty() || fileB.empty()) {
        std::cerr << "❌ 必须指定两个输入文件!" << std::endl;
        std::cerr << "   使用 -a 指定文件A，使用 -b 指定文件B" << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    // 验证文件是否存在
    if (!validateFile(fileA)) {
        return 1;
    }
    if (!validateFile(fileB)) {
        return 1;
    }
    std::cout << "文件A: " << fileA << std::endl;
    std::cout << "文件B: " << fileB << std::endl;
    std::cout << "输出目录: " << outputDir << std::endl;
    if (!ignoreFields.empty()) {
        std::cout << "忽略字段: ";
        for (size_t i = 0; i < ignoreFields.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << ignoreFields[i];
        }
        std::cout << std::endl;
    } else {
        std::cout << "字段过滤: 未启用（读取所有字段）" << std::endl;
    }
    std::cout << "索引精度: " << indexDecimal << std::endl;
    std::cout << "容差值: " << tolerance << std::endl;
    if (!compareKey.empty()) {
        std::cout << "比较键字段: " << compareKey << std::endl;
    } else {
        std::cout << "比较键字段: 未设置（仅使用 code + time）" << std::endl;
    }
    std::cout << std::string(60, '=') << std::endl;

    try {
        // 1. 验证文件信息
        std::cout << "\n=== 步骤1: 验证输入文件 ===" << std::endl;
        std::cout << "✓ 文件A: " << fileA << std::endl;
        std::cout << "✓ 文件B: " << fileB << std::endl;

        // 2. 执行JSON文件对比
        std::cout << "\n=== 步骤2: 执行文件对比 ===" << std::endl;

        if (enableSplit) {
            // 分组模式
            std::cout << ">>> 使用分组模式（先按code分组，再比较）<<<" << std::endl;
            demonstrateGroupedComparison(fileA, fileB, outputDir, ignoreFields, indexDecimal, tolerance, keepSplitFiles, compareKey);
        } else {
            // 流式模式（默认）
            std::cout << ">>> 使用流式模式（按时间戳流式对比）<<<" << std::endl;
            demonstrateJsonFileComparison(fileA, fileB, outputDir, ignoreFields, indexDecimal, tolerance, compareKey);
        }

    } catch (const std::exception& e) {
        std::cerr << "❌ 执行失败: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\n🎉 执行成功完成!" << std::endl;

    // 计算并输出程序运行耗时
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    double seconds = duration.count() / 1000.0;
    std::cout << "程序运行耗时: " << std::fixed << std::setprecision(3) << seconds << "s" << std::endl;

    return 0;
}