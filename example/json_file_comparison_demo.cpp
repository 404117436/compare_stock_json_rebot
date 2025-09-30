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
#include "StockDataComparator.h"
#include "StockDataBatchReader.h"

/**
 * JSON文件差异对比Demo
 * 功能：使用StockDataBatchReader读取两个JSON文件，使用StockDataComparator进行详细对比
 */

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

// 创建测试文件A - 完整的股票数据
void createTestFileA(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("无法创建测试文件: " + filename);
    }

    // 写入测试JSON数据 - 文件A包含完整字段
    file << R"({"code":"000001","time":"093000","price":10.50,"volume":1000,"market":"SZ","name":"平安银行"})" << std::endl;
    file << R"({"code":"000001","time":"093001","price":10.51,"volume":1200,"market":"SZ","name":"平安银行"})" << std::endl;
    file << R"({"code":"000001","time":"093002","price":10.52,"volume":800,"market":"SZ","name":"平安银行"})" << std::endl;

    file << R"({"code":"000002","time":"093000","price":20.30,"volume":800,"market":"SZ","name":"万科A"})" << std::endl;
    file << R"({"code":"000002","time":"093001","price":20.35,"volume":900,"market":"SZ","name":"万科A"})" << std::endl;

    file << R"({"code":"000003","time":"093000","price":15.80,"volume":600,"market":"SZ","name":"招商银行"})" << std::endl;
    file << R"({"code":"000003","time":"093001","price":15.85,"volume":700,"market":"SZ","name":"招商银行"})" << std::endl;

    // 只在A中存在的记录
    file << R"({"code":"000004","time":"093000","price":8.90,"volume":500,"market":"SZ","name":"国农科技"})" << std::endl;
    file << R"({"code":"000005","time":"093000","price":12.40,"volume":300,"market":"SZ","name":"世纪星源"})" << std::endl;

    file.close();
    std::cout << "✓ 测试文件A创建成功: " << filename << " (9条记录)" << std::endl;
}

// 创建测试文件B - 包含差异的股票数据
void createTestFileB(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("无法创建测试文件: " + filename);
    }

    // 写入测试JSON数据 - 文件B包含差异字段
    // 相同记录 (用于验证匹配)
    file << R"({"code":"000001","time":"093000","price":10.50,"volume":1000,"market":"SZ","name":"平安银行"})" << std::endl;

    // 字段值不同
    file << R"({"code":"000001","time":"093001","price":10.60,"volume":1200,"market":"SZ","name":"平安银行"})" << std::endl;
    file << R"({"code":"000001","time":"093002","price":10.65,"volume":850,"market":"SZ","name":"平安银行"})" << std::endl;

    // 字段缺失/新增 (volume缺失, turnover新增)
    file << R"({"code":"000002","time":"093000","price":20.30,"turnover":50000,"market":"SZ","name":"万科A"})" << std::endl;
    file << R"({"code":"000002","time":"093001","price":20.40,"turnover":55000,"market":"SZ","name":"万科A"})" << std::endl;

    // 类型不匹配 (price为字符串)
    file << R"({"code":"000003","time":"093000","price":"15.80","volume":600,"market":"SZ","name":"招商银行"})" << std::endl;

    // 只在B中存在的记录
    file << R"({"code":"600000","time":"093000","price":35.20,"volume":400,"market":"SH","name":"浦发银行"})" << std::endl;
    file << R"({"code":"600036","time":"093000","price":42.50,"volume":350,"market":"SH","name":"招商银行"})" << std::endl;

    file.close();
    std::cout << "✓ 测试文件B创建成功: " << filename << " (8条记录)" << std::endl;
}

// 使用StockDataBatchReader读取JSON文件数据
std::vector<StockDataContainer> loadDataFromFile(const std::string& filename, const std::string& indexKey = "time", const std::vector<std::string>& ignoreFields = {}) {
    std::cout << "\n--- 读取文件: " << filename << " ---" << std::endl;

    try {
        // 创建批量读取器，使用较大的内存限制确保能处理所有数据
        StockDataBatchReader reader(filename, indexKey, 4096, 1, ignoreFields);
        std::vector<StockDataContainer> allData;

        size_t totalRecords = 0;
        size_t batchCount = 0;

        while (true) {
            // 使用修改后的popBatch - 自动补充数据
            std::vector<StockDataContainer> batch;
            if (!reader.popBatch(batch)) {
                break; // 无更多数据
            }

            batchCount++;
            totalRecords += batch.size();

            std::cout << "  批次 " << batchCount << ": " << batch.size() << " 条记录";
            if (!batch.empty()) {
                std::cout << " (时间: " << batch[0].getIndexValue() << ")";
            }
            std::cout << std::endl;

            // 将批次数据合并到总集合
            allData.insert(allData.end(),
                          std::make_move_iterator(batch.begin()),
                          std::make_move_iterator(batch.end()));
        }

        std::cout << "✓ 文件读取完成: 总计 " << totalRecords << " 条记录，分 "
                  << batchCount << " 个批次" << std::endl;

        return allData;

    } catch (const std::exception& e) {
        std::cerr << "❌ 读取文件失败: " << e.what() << std::endl;
        return {};
    }
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
        timeStats[container.getIndexValue()]++;
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
            report << "字段差异详情 (" << record.differences.size() << " 个字段):" << std::endl;
            report << "字段名            差异类型         A侧值                        B侧值" << std::endl;
            report << "--------------------------------------------------------------------------------" << std::endl;

            for (const auto& diff : record.differences) {
                // 转换差异类型名称
                std::string diffType;
                if (diff.differenceType == "missing_in_A") {
                    diffType = "MISSING_IN_A";
                } else if (diff.differenceType == "missing_in_B") {
                    diffType = "MISSING_IN_B";
                } else if (diff.differenceType == "value_different") {
                    diffType = "VALUE_DIFFERENT";
                } else if (diff.differenceType == "type_mismatch") {
                    diffType = "TYPE_MISMATCH";
                } else {
                    diffType = diff.differenceType;
                }

                // 格式化A侧值
                std::string valueAStr = "null";
                if (diff.existsInA) {
                    switch (diff.valueA.getType()) {
                        case JsonValueType::String:
                            valueAStr = "\"" + diff.valueA.asString() + "\"";
                            break;
                        case JsonValueType::Int:
                            valueAStr = std::to_string(diff.valueA.asInt());
                            break;
                        case JsonValueType::Double:
                            valueAStr = std::to_string(diff.valueA.asDouble());
                            break;
                        case JsonValueType::Bool:
                            valueAStr = diff.valueA.asBool() ? "true" : "false";
                            break;
                        case JsonValueType::Null:
                            valueAStr = "null";
                            break;
                        case JsonValueType::Array:
                        case JsonValueType::Object:
                            valueAStr = "[复杂类型]";
                            break;
                        default:
                            valueAStr = "[未知类型]";
                            break;
                    }
                }

                // 格式化B侧值
                std::string valueBStr = "null";
                if (diff.existsInB) {
                    switch (diff.valueB.getType()) {
                        case JsonValueType::String:
                            valueBStr = "\"" + diff.valueB.asString() + "\"";
                            break;
                        case JsonValueType::Int:
                            valueBStr = std::to_string(diff.valueB.asInt());
                            break;
                        case JsonValueType::Double:
                            valueBStr = std::to_string(diff.valueB.asDouble());
                            break;
                        case JsonValueType::Bool:
                            valueBStr = diff.valueB.asBool() ? "true" : "false";
                            break;
                        case JsonValueType::Null:
                            valueBStr = "null";
                            break;
                        case JsonValueType::Array:
                        case JsonValueType::Object:
                            valueBStr = "[复杂类型]";
                            break;
                        default:
                            valueBStr = "[未知类型]";
                            break;
                    }
                }

                // 输出表格化的行（使用固定宽度格式化）
                char buffer[512];
                snprintf(buffer, sizeof(buffer), "%-16s%-16s%-28s%s",
                         diff.fieldName.c_str(),
                         diffType.c_str(),
                         valueAStr.c_str(),
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

// 演示JSON文件对比功能
void demonstrateJsonFileComparison(const std::string& fileA, const std::string& fileB, const std::string& outputDir, const std::vector<std::string>& ignoreFields = {}) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "JSON文件差异对比Demo" << std::endl;
    std::cout << std::string(60, '=') << std::endl;

    try {
        // 读取两个文件
        auto dataA = loadDataFromFile(fileA, "time", ignoreFields);
        auto dataB = loadDataFromFile(fileB, "time", ignoreFields);

        // 显示文件信息
        printFileInfo(dataA, fileA);
        printFileInfo(dataB, fileB);

        // 创建比较器并注入数据
        std::cout << "\n--- 执行数据对比 ---" << std::endl;
        StockDataComparator comparator;

        std::cout << "✓ 设置数据集A: " << dataA.size() << " 条记录" << std::endl;
        comparator.setDataA(std::move(dataA));

        std::cout << "✓ 设置数据集B: " << dataB.size() << " 条记录" << std::endl;
        comparator.setDataB(std::move(dataB));

        std::cout << "✓ 开始详细比较..." << std::endl;

        // 执行详细比较
        auto result = comparator.compareDetailed();

        // 输出结果
        printComparisonSummary(result);

        // 输出详细差异报告
        std::cout << "\n" << std::string(60, '-') << std::endl;
        std::cout << "详细差异报告" << std::endl;
        std::cout << std::string(60, '-') << std::endl;
        result.printDetailedDifferences();

        // 写入差异文件
        writeDifferencesToFiles(result, outputDir);

        std::cout << "\n✅ JSON文件对比完成!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "❌ 对比过程中发生错误: " << e.what() << std::endl;
    }
}

// 显示帮助信息
void printUsage(const char* programName) {
    std::cout << "用法: " << programName << " -a <文件A路径> -b <文件B路径> [-o <输出目录>] [-f <过滤字段>]" << std::endl;
    std::cout << std::endl;
    std::cout << "必需参数:" << std::endl;
    std::cout << "  -a <文件路径>  指定第一个JSON文件的绝对路径" << std::endl;
    std::cout << "  -b <文件路径>  指定第二个JSON文件的绝对路径" << std::endl;
    std::cout << std::endl;
    std::cout << "可选参数:" << std::endl;
    std::cout << "  -o <目录路径>  指定差异文件的输出目录，默认: ./diff_output/" << std::endl;
    std::cout << "  -f <字段列表>  指定需要忽略的字段，用逗号分隔（可选）" << std::endl;
    std::cout << "  -h, --help     显示此帮助信息" << std::endl;
    std::cout << std::endl;
    std::cout << "示例:" << std::endl;
    std::cout << "  " << programName << " -a /path/to/fileA.json -b /path/to/fileB.json" << std::endl;
    std::cout << "  " << programName << " -a data1.json -b data2.json -o /tmp/diff_result" << std::endl;
    std::cout << "  " << programName << " -a data1.json -b data2.json -f timestamp,debug_info" << std::endl;
    std::cout << "  " << programName << " -a data1.json -b data2.json  # 不过滤任何字段" << std::endl;
    std::cout << "  " << programName << " --help" << std::endl;
    std::cout << std::endl;
    std::cout << "功能: JSON文件批量读取 + 详细差异对比 + 按股票代码分组输出" << std::endl;
}

// 主函数 - 演示不同的测试场景
int main(int argc, char* argv[]) {
    // 参数解析
    std::string fileA;
    std::string fileB;
    std::string outputDir = "./diff_output";  // 默认输出目录
    std::vector<std::string> ignoreFields;    // 默认为空，表示不过滤

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
                std::cout << "✓ 将过滤字段: ";
                for (size_t j = 0; j < ignoreFields.size(); ++j) {
                    if (j > 0) std::cout << ", ";
                    std::cout << ignoreFields[j];
                }
                std::cout << std::endl;
            }
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

    std::cout << "StockDataComparator & StockDataBatchReader 集成演示" << std::endl;
    std::cout << "功能：JSON文件批量读取 + 详细差异对比 + 按Code分组输出" << std::endl;
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
    std::cout << std::string(60, '=') << std::endl;

    try {
        // 1. 验证文件信息
        std::cout << "\n=== 步骤1: 验证输入文件 ===" << std::endl;
        std::cout << "✓ 文件A: " << fileA << std::endl;
        std::cout << "✓ 文件B: " << fileB << std::endl;

        // 2. 执行JSON文件对比
        std::cout << "\n=== 步骤2: 执行文件对比 ===" << std::endl;
        demonstrateJsonFileComparison(fileA, fileB, outputDir, ignoreFields);

    } catch (const std::exception& e) {
        std::cerr << "❌ Demo执行失败: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\n🎉 Demo执行成功完成!" << std::endl;
    std::cout << "\n该Demo展示了以下关键功能:" << std::endl;
    std::cout << "  • StockDataBatchReader: 高效批量JSON文件读取" << std::endl;
    std::cout << "  • StockDataComparator: 详细字段级差异分析" << std::endl;
    std::cout << "  • 自动数据补充: popBatch()的智能数据管理" << std::endl;
    std::cout << "  • 完整差异报告: 字段存在性、类型、值的全面对比" << std::endl;
    std::cout << "  • 按Code分组输出: 自动按股票代码生成独立差异文件" << std::endl;
    std::cout << "  • 灵活文件输出: 支持自定义输出目录路径" << std::endl;

    return 0;
}