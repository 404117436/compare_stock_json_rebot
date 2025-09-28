#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include "../include/MarketData.h"

void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << std::string(70, '=') << std::endl;
}

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return "";
    }

    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line;
    }
    file.close();
    return content;
}

void testSingleMarketData() {
    printSeparator("单条MarketData解析测试");

    // 从文件读取JSON数据
    std::string jsonContent = readFile("../example/market_data.json");
    if (jsonContent.empty()) {
        std::cout << "❌ 无法读取测试数据文件" << std::endl;
        return;
    }

    // 解析数据
    MarketData marketData;
    bool success = marketData.parseFromJsonString(jsonContent);

    if (success) {
        std::cout << "✓ JSON解析成功！" << std::endl;
        std::cout << "数据有效性: " << (marketData.isValid() ? "有效" : "无效") << std::endl;

        // 显示基本信息
        std::cout << "\n--- 基本信息 ---" << std::endl;
        std::cout << "证券代码: " << marketData.code << std::endl;
        std::cout << "证券名称: " << marketData.codename << std::endl;
        std::cout << "市场: " << marketData.market << std::endl;
        std::cout << "状态: " << marketData.status << std::endl;
        std::cout << "本地时间: " << marketData.local_time << std::endl;

        // 显示价格信息
        std::cout << "\n--- 价格信息 ---" << std::endl;
        std::cout << "最新价: " << marketData.new_price << std::endl;
        std::cout << "前收价: " << marketData.pre_price << std::endl;
        std::cout << "涨停价: " << marketData.uplimit_price << std::endl;
        std::cout << "跌停价: " << marketData.downlimit_price << std::endl;

        // 显示成交信息
        std::cout << "\n--- 成交信息 ---" << std::endl;
        std::cout << "成交量: " << marketData.volume << std::endl;
        std::cout << "成交额: " << marketData.turnover << std::endl;
        std::cout << "成交笔数: " << marketData.trade_num << std::endl;

        // 显示买卖盘信息
        std::cout << "\n--- 买卖盘信息 ---" << std::endl;
        std::cout << "买一价: " << marketData.getBidPrice(0) << ", 买一量: " << marketData.getBidVolume(0) << std::endl;
        std::cout << "卖一价: " << marketData.getAskPrice(0) << ", 卖一量: " << marketData.getAskVolume(0) << std::endl;
        std::cout << "买卖价差: " << marketData.getSpread() << std::endl;
        std::cout << "中间价: " << marketData.getMidPrice() << std::endl;

        // 显示十档行情
        marketData.printOrderBook();

    } else {
        std::cout << "❌ JSON解析失败" << std::endl;
    }
}

void testMarketDataContainer() {
    printSeparator("MarketDataContainer批量解析测试");

    // 创建容器
    MarketDataContainer container("测试数据源");

    // 从文件读取JSON数组
    std::string jsonArrayContent = readFile("../example/market_data_array.json");
    if (jsonArrayContent.empty()) {
        std::cout << "❌ 无法读取批量测试数据文件" << std::endl;
        return;
    }

    // 批量添加数据
    bool success = container.addFromJsonArray(jsonArrayContent);
    if (success) {
        std::cout << "✓ 批量解析成功！" << std::endl;
        std::cout << "容器大小: " << container.size() << std::endl;

        // 显示摘要信息
        container.printSummary();

        // 测试查找功能
        std::cout << "\n--- 查找测试 ---" << std::endl;

        // 按代码查找
        auto it = container.findByCode("000001");
        if (it != container.end()) {
            std::cout << "找到代码 000001: " << it->codename << std::endl;
        }

        // 按市场查找
        auto ushd_stocks = container.findByMarket("USHD");
        std::cout << "USHD市场股票数量: " << ushd_stocks.size() << std::endl;

        // 统计信息
        std::cout << "\n--- 统计信息 ---" << std::endl;
        std::cout << "USHD市场计数: " << container.countByMarket("USHD") << std::endl;
        std::cout << "SZSE市场计数: " << container.countByMarket("SZSE") << std::endl;
        std::cout << "总成交额: " << container.getTotalTurnover() << std::endl;
        std::cout << "总成交量: " << container.getTotalVolume() << std::endl;

    } else {
        std::cout << "❌ 批量解析失败" << std::endl;
        return;
    }

    // 测试排序功能
    std::cout << "\n--- 排序测试 ---" << std::endl;

    std::cout << "按成交额降序排序前：" << std::endl;
    for (size_t i = 0; i < container.size(); ++i) {
        std::cout << "  " << container[i].code << ": " << container[i].turnover << std::endl;
    }

    container.sortByTurnover(true);  // 按成交额降序
    std::cout << "\n按成交额降序排序后：" << std::endl;
    for (size_t i = 0; i < container.size(); ++i) {
        std::cout << "  " << container[i].code << ": " << container[i].turnover << std::endl;
    }

    // 测试按代码排序
    container.sortByCode();
    std::cout << "\n按代码排序后：" << std::endl;
    for (size_t i = 0; i < container.size(); ++i) {
        std::cout << "  " << container[i].code << ": " << container[i].codename << std::endl;
    }

    // 测试导出功能
    std::cout << "\n--- 导出测试 ---" << std::endl;

    bool csvExport = container.exportToCsv("market_data_export.csv");
    std::cout << "CSV导出: " << (csvExport ? "成功" : "失败") << std::endl;

    bool jsonExport = container.exportToJson("market_data_export.json");
    std::cout << "JSON导出: " << (jsonExport ? "成功" : "失败") << std::endl;
}

void testContainerOperations() {
    printSeparator("容器操作测试");

    MarketDataContainer container("手动测试");

    // 手动创建测试数据
    MarketData data1;
    data1.code = "TEST001";
    data1.codename = "测试股票1";
    data1.market = "TEST";
    data1.new_price = 10.50;
    data1.volume = 1000000;
    data1.turnover = 10500000.0;

    MarketData data2;
    data2.code = "TEST002";
    data2.codename = "测试股票2";
    data2.market = "TEST";
    data2.new_price = 8.25;
    data2.volume = 2000000;
    data2.turnover = 16500000.0;

    // 添加数据
    container.addMarketData(data1);
    container.addMarketData(std::move(data2));

    std::cout << "手动添加数据完成，容器大小: " << container.size() << std::endl;

    // 测试迭代器
    std::cout << "\n使用迭代器遍历:" << std::endl;
    for (const auto& data : container) {
        std::cout << "  " << data.code << " - " << data.codename
                  << " - 价格: " << data.new_price << std::endl;
    }

    // 测试索引访问
    std::cout << "\n使用索引访问:" << std::endl;
    for (size_t i = 0; i < container.size(); ++i) {
        std::cout << "  [" << i << "] " << container[i].code
                  << " - " << container[i].codename << std::endl;
    }

    // 测试查找和修改
    auto it = container.findByCode("TEST001");
    if (it != container.end()) {
        std::cout << "\n修改前 TEST001 价格: " << it->new_price << std::endl;
        it->new_price = 11.00;
        std::cout << "修改后 TEST001 价格: " << it->new_price << std::endl;
    }
}

void testDataValidation() {
    printSeparator("数据验证测试");

    // 测试有效数据
    MarketData validData;
    validData.code = "VALID001";
    validData.market = "TEST";
    validData.new_price = 10.50;
    validData.pre_price = 10.25;
    validData.uplimit_price = 11.28;
    validData.downlimit_price = 9.23;

    std::cout << "有效数据验证: " << (validData.isValid() ? "通过" : "失败") << std::endl;

    // 测试无效数据 - 空代码
    MarketData invalidData1;
    invalidData1.market = "TEST";
    invalidData1.new_price = 10.50;
    std::cout << "空代码数据验证: " << (invalidData1.isValid() ? "通过" : "失败") << std::endl;

    // 测试无效数据 - 负价格
    MarketData invalidData2;
    invalidData2.code = "INVALID001";
    invalidData2.market = "TEST";
    invalidData2.new_price = -10.50;
    std::cout << "负价格数据验证: " << (invalidData2.isValid() ? "通过" : "失败") << std::endl;

    // 测试无效数据 - 涨跌停价格错误
    MarketData invalidData3;
    invalidData3.code = "INVALID002";
    invalidData3.market = "TEST";
    invalidData3.new_price = 10.50;
    invalidData3.uplimit_price = 9.00;   // 涨停价小于跌停价
    invalidData3.downlimit_price = 11.00;
    std::cout << "涨跌停价格错误数据验证: " << (invalidData3.isValid() ? "通过" : "失败") << std::endl;
}

void testPerformance() {
    printSeparator("性能测试");

    auto start = std::chrono::high_resolution_clock::now();

    MarketDataContainer container("性能测试");

    // 创建大量测试数据
    const int testCount = 10000;
    std::cout << "创建 " << testCount << " 条测试数据..." << std::endl;

    for (int i = 0; i < testCount; ++i) {
        MarketData data;
        data.code = "PERF" + std::to_string(i);
        data.codename = "性能测试股票" + std::to_string(i);
        data.market = (i % 2 == 0) ? "SSE" : "SZSE";
        data.new_price = 10.0 + (i % 100) * 0.1;
        data.volume = 1000 + i * 100;
        data.turnover = data.new_price * data.volume;

        container.addMarketData(std::move(data));
    }

    auto middle = std::chrono::high_resolution_clock::now();

    // 测试查找性能
    std::cout << "测试查找性能..." << std::endl;
    int found = 0;
    for (int i = 0; i < 1000; ++i) {
        std::string code = "PERF" + std::to_string(i * 10);
        auto it = container.findByCode(code);
        if (it != container.end()) {
            found++;
        }
    }

    // 测试排序性能
    std::cout << "测试排序性能..." << std::endl;
    container.sortByTurnover(true);

    auto end = std::chrono::high_resolution_clock::now();

    // 计算耗时
    auto createTime = std::chrono::duration_cast<std::chrono::milliseconds>(middle - start);
    auto operationTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - middle);
    auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "\n性能统计：" << std::endl;
    std::cout << "创建 " << testCount << " 条数据耗时: " << createTime.count() << " 毫秒" << std::endl;
    std::cout << "查找和排序操作耗时: " << operationTime.count() << " 毫秒" << std::endl;
    std::cout << "总耗时: " << totalTime.count() << " 毫秒" << std::endl;
    std::cout << "找到记录数: " << found << std::endl;
    std::cout << "容器大小: " << container.size() << std::endl;
}

int main() {
    printSeparator("基于std::vector的MarketData结构体演示");

    std::cout << "这个演示展示了MarketData结构体和MarketDataContainer的功能：" << std::endl;
    std::cout << "- JSON行情数据解析" << std::endl;
    std::cout << "- std::vector容器管理" << std::endl;
    std::cout << "- 十档买卖盘数据处理" << std::endl;
    std::cout << "- 查找、排序、统计功能" << std::endl;
    std::cout << "- 数据验证和导出功能" << std::endl;

    try {
        // 单条数据解析测试
        testSingleMarketData();

        // 批量数据解析和容器测试
        testMarketDataContainer();

        // 容器操作测试
        testContainerOperations();

        // 数据验证测试
        testDataValidation();

        // 性能测试
        testPerformance();

        printSeparator("演示完成");
        std::cout << "✅ 所有测试完成！MarketData和MarketDataContainer功能正常。" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "❌ 程序异常: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}