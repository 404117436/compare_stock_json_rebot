#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <string>
#include <exception>
#include <map>
#include <algorithm>
#include "StockDataComparator.h"
#include "StockDataBatchReader.h"

/**
 * JSON文件差异对比Demo
 * 功能：使用StockDataBatchReader读取两个JSON文件，使用StockDataComparator进行详细对比
 */

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
std::vector<StockDataContainer> loadDataFromFile(const std::string& filename, const std::string& indexKey = "time") {
    std::cout << "\n--- 读取文件: " << filename << " ---" << std::endl;

    try {
        // 创建批量读取器，使用较大的内存限制确保能处理所有数据
        StockDataBatchReader reader(filename, indexKey, 4096);
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

// 演示JSON文件对比功能
void demonstrateJsonFileComparison(const std::string& fileA, const std::string& fileB) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "JSON文件差异对比Demo" << std::endl;
    std::cout << std::string(60, '=') << std::endl;

    try {
        // 读取两个文件
        auto dataA = loadDataFromFile(fileA);
        auto dataB = loadDataFromFile(fileB);

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

        std::cout << "\n✅ JSON文件对比完成!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "❌ 对比过程中发生错误: " << e.what() << std::endl;
    }
}

// 主函数 - 演示不同的测试场景
int main() {
    std::cout << "StockDataComparator & StockDataBatchReader 集成演示" << std::endl;
    std::cout << "功能：JSON文件批量读取 + 详细差异对比" << std::endl;
    std::cout << std::string(60, '=') << std::endl;

    const std::string testFileA = "comparison_test_fileA.json";
    const std::string testFileB = "comparison_test_fileB.json";

    try {
        // 1. 创建测试文件
        std::cout << "\n=== 步骤1: 创建测试数据 ===" << std::endl;
        createTestFileA(testFileA);
        createTestFileB(testFileB);

        // 2. 演示JSON文件对比
        std::cout << "\n=== 步骤2: 执行文件对比 ===" << std::endl;
        demonstrateJsonFileComparison(testFileA, testFileB);

        // 3. 清理测试文件
        std::cout << "\n=== 步骤3: 清理测试文件 ===" << std::endl;
        std::remove(testFileA.c_str());
        std::remove(testFileB.c_str());
        std::cout << "✓ 测试文件已清理" << std::endl;

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

    return 0;
}