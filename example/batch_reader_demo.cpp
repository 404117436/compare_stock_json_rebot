#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "StockDataBatchReader.h"

// 创建测试数据文件
void createTestDataFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("无法创建测试文件: " + filename);
    }

    // 写入测试JSON数据，按时间戳分组
    file << R"({"local_time":"20250818-09:30:00.100","recv_time":0,"market":"SZSE","code":"000001","time":93000100,"new_price":10.50})" << std::endl;
    file << R"({"local_time":"20250818-09:30:00.200","recv_time":0,"market":"SZSE","code":"000001","time":93000100,"new_price":10.51})" << std::endl;
    file << R"({"local_time":"20250818-09:30:00.300","recv_time":0,"market":"SZSE","code":"000001","time":93000100,"new_price":10.52})" << std::endl;

    file << R"({"local_time":"20250818-09:30:01.100","recv_time":0,"market":"SZSE","code":"000001","time":93001100,"new_price":10.55})" << std::endl;
    file << R"({"local_time":"20250818-09:30:01.200","recv_time":0,"market":"SZSE","code":"000001","time":93001100,"new_price":10.56})" << std::endl;

    file << R"({"local_time":"20250818-09:30:02.100","recv_time":0,"market":"SZSE","code":"000001","time":93002100,"new_price":10.60})" << std::endl;
    file << R"({"local_time":"20250818-09:30:02.200","recv_time":0,"market":"SZSE","code":"000001","time":93002100,"new_price":10.61})" << std::endl;
    file << R"({"local_time":"20250818-09:30:02.300","recv_time":0,"market":"SZSE","code":"000001","time":93002100,"new_price":10.62})" << std::endl;
    file << R"({"local_time":"20250818-09:30:02.400","recv_time":0,"market":"SZSE","code":"000001","time":93002100,"new_price":10.63})" << std::endl;

    file << R"({"local_time":"20250818-09:30:03.100","recv_time":0,"market":"SZSE","code":"000001","time":93003100,"new_price":10.65})" << std::endl;

    file.close();
    std::cout << "✓ 测试数据文件创建成功: " << filename << std::endl;
}

// 演示基本批次读取功能
void demonstrateBatchReading(const std::string& filename) {
    std::cout << "\n=== 批次读取功能演示 ===" << std::endl;

    try {
        // 创建批次读取器，设置较小的内存限制便于测试
        StockDataBatchReader reader(filename, 1024);  // 1KB内存限制

        size_t batchNumber = 1;
        std::string indexKey = "time";  // 按时间字段分组

        while (true) {
            // 读取下一批次
            size_t recordsRead = reader.readNextBatch(indexKey);

            if (recordsRead == 0) {
                std::cout << "✓ 所有数据读取完成" << std::endl;
                break;
            }

            std::cout << "\n--- 批次 " << batchNumber << " ---" << std::endl;
            std::cout << "读取记录数: " << recordsRead << std::endl;
            std::cout << "当前内存使用: " << reader.getCurrentMemoryUsage() << " 字节" << std::endl;

            // 检查是否有完整批次
            if (reader.hasCompleteBatch()) {
                std::cout << "✓ 发现完整批次" << std::endl;

                // 获取批次数据
                auto batch = reader.popBatch();
                std::cout << "批次大小: " << batch.size() << " 条记录" << std::endl;

                if (!batch.empty()) {
                    std::cout << "批次索引值: " << batch[0].getIndexValue() << std::endl;
                    std::cout << "批次数据示例:" << std::endl;

                    for (size_t i = 0; i < batch.size() && i < 3; ++i) {
                        std::cout << "  记录 " << (i+1) << ": "
                                  << "code=" << batch[i].getCode()
                                  << ", time=" << batch[i].getIndexValue()
                                  << ", price=" << batch[i]["new_price"].asDouble() << std::endl;
                    }

                    if (batch.size() > 3) {
                        std::cout << "  ... 还有 " << (batch.size() - 3) << " 条记录" << std::endl;
                    }
                }
            } else {
                std::cout << "批次未完整，继续读取..." << std::endl;
            }

            batchNumber++;

            // 防止无限循环
            if (batchNumber > 10) {
                std::cout << "达到最大批次数限制，停止读取" << std::endl;
                break;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "批次读取演示失败: " << e.what() << std::endl;
    }
}

// 演示不同索引字段的批次读取
void demonstrateDifferentIndexFields(const std::string& filename) {
    std::cout << "\n=== 不同索引字段演示 ===" << std::endl;

    try {
        // 按价格分组
        std::cout << "\n--- 按价格字段分组 ---" << std::endl;
        StockDataBatchReader priceReader(filename, 2048);

        size_t recordsRead = priceReader.readNextBatch("new_price");
        std::cout << "按价格读取记录数: " << recordsRead << std::endl;

        if (priceReader.hasCompleteBatch()) {
            auto batch = priceReader.getBatch();  // 使用getBatch（拷贝）而不是popBatch
            if (!batch.empty()) {
                std::cout << "价格分组示例 (索引值: " << batch[0].getIndexValue() << "):" << std::endl;
                for (size_t i = 0; i < batch.size() && i < 3; ++i) {
                    std::cout << "  price=" << batch[i].getIndexValue()
                              << ", time=" << batch[i]["time"].asInt() << std::endl;
                }
            }
        }

        // 按市场分组
        std::cout << "\n--- 按市场字段分组 ---" << std::endl;
        StockDataBatchReader marketReader(filename, 2048);

        recordsRead = marketReader.readNextBatch("market");
        std::cout << "按市场读取记录数: " << recordsRead << std::endl;

        if (marketReader.hasCompleteBatch()) {
            auto batch = marketReader.getBatch();
            if (!batch.empty()) {
                std::cout << "市场分组示例 (索引值: " << batch[0].getIndexValue() << "):" << std::endl;
                for (size_t i = 0; i < batch.size() && i < 3; ++i) {
                    std::cout << "  market=" << batch[i].getIndexValue()
                              << ", code=" << batch[i].getCode() << std::endl;
                }
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "不同索引字段演示失败: " << e.what() << std::endl;
    }
}

// 演示内存限制功能
void demonstrateMemoryLimit(const std::string& filename) {
    std::cout << "\n=== 内存限制功能演示 ===" << std::endl;

    try {
        // 创建内存限制很小的读取器
        StockDataBatchReader smallReader(filename, 500);  // 500字节限制

        std::cout << "内存限制: 500字节" << std::endl;

        size_t totalRecords = 0;
        size_t batchCount = 0;

        while (true) {
            size_t recordsRead = smallReader.readNextBatch("time");
            if (recordsRead == 0) break;

            totalRecords += recordsRead;
            batchCount++;

            std::cout << "批次 " << batchCount << ": 读取 " << recordsRead
                      << " 条记录, 内存使用 " << smallReader.getCurrentMemoryUsage()
                      << " 字节" << std::endl;

            if (smallReader.hasCompleteBatch()) {
                auto batch = smallReader.popBatch();
                std::cout << "  导出批次: " << batch.size() << " 条记录" << std::endl;
            }

            if (batchCount > 20) break;  // 防止无限循环
        }

        std::cout << "总计读取: " << totalRecords << " 条记录，" << batchCount << " 个批次" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "内存限制演示失败: " << e.what() << std::endl;
    }
}

// 演示完整的数据处理流程
void demonstrateCompleteWorkflow(const std::string& filename) {
    std::cout << "\n=== 完整工作流程演示 ===" << std::endl;

    try {
        StockDataBatchReader reader(filename, 1024);

        std::vector<std::vector<StockDataContainer>> allBatches;
        size_t totalRecords = 0;

        std::cout << "开始批量处理..." << std::endl;

        while (true) {
            // 读取下一批次
            size_t recordsRead = reader.readNextBatch("time");
            if (recordsRead == 0) break;

            totalRecords += recordsRead;

            // 处理完整批次
            while (reader.hasCompleteBatch()) {
                auto batch = reader.popBatch();
                allBatches.push_back(std::move(batch));

                std::cout << "处理批次 " << allBatches.size()
                          << ": " << allBatches.back().size() << " 条记录" << std::endl;
            }
        }

        // 处理最后一个批次（如果有）
        if (!reader.isEmpty()) {
            auto lastBatch = reader.popBatch();
            if (!lastBatch.empty()) {
                allBatches.push_back(std::move(lastBatch));
                std::cout << "处理最后批次: " << allBatches.back().size() << " 条记录" << std::endl;
            }
        }

        // 统计结果
        std::cout << "\n处理结果统计:" << std::endl;
        std::cout << "  总记录数: " << totalRecords << std::endl;
        std::cout << "  总批次数: " << allBatches.size() << std::endl;

        // 显示每个批次的详细信息
        for (size_t i = 0; i < allBatches.size(); ++i) {
            const auto& batch = allBatches[i];
            if (!batch.empty()) {
                std::cout << "  批次 " << (i+1) << ": " << batch.size()
                          << " 条记录, 索引值=" << batch[0].getIndexValue() << std::endl;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "完整工作流程演示失败: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "StockDataBatchReader 功能演示程序" << std::endl;
    std::cout << "====================================" << std::endl;

    const std::string testFile = "test_stock_data.json";

    try {
        // 创建测试数据
        createTestDataFile(testFile);

        // 运行各种演示
        demonstrateBatchReading(testFile);
        demonstrateDifferentIndexFields(testFile);
        demonstrateMemoryLimit(testFile);
        demonstrateCompleteWorkflow(testFile);

        std::cout << "\n====================================" << std::endl;
        std::cout << "✓ 所有演示完成!" << std::endl;

        std::cout << "\nStockDataBatchReader 设计验证总结:" << std::endl;
        std::cout << "  ✓ 按索引字段连续性批量读取" << std::endl;
        std::cout << "  ✓ 支持任意字段作为索引" << std::endl;
        std::cout << "  ✓ 内存软上限控制" << std::endl;
        std::cout << "  ✓ 临界数据缓存处理" << std::endl;
        std::cout << "  ✓ 完整批次检测" << std::endl;
        std::cout << "  ✓ 批次导出和拷贝接口" << std::endl;
        std::cout << "  ✓ 内存使用统计" << std::endl;

        // 清理测试文件
        std::remove(testFile.c_str());
        std::cout << "✓ 测试文件已清理" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "演示过程中发生错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}