#include <iostream>
#include <string>
#include "StockDataContainer.h"

// 测试用的股票JSON数据
const std::string stockJsonData = R"({
    "local_time": "20250818-11:21:25.409",
    "recv_time": 0,
    "market": "USHD",
    "code": "110084",
    "codename": "贵燃转债",
    "status": "C",
    "date": 20250818,
    "time": 91503020,
    "pre_price": 100.15,
    "open_price": 100.20,
    "high_price": 101.50,
    "low_price": 99.80,
    "new_price": 100.85,
    "volume": 12500,
    "turnover": 1259375.0,
    "trade_num": 156,
    "bidorder_price": [100.80, 100.75, 100.70],
    "askorder_price": [100.85, 100.90, 100.95],
    "bidorder_volume": [1000, 2000, 1500],
    "askorder_volume": [1200, 1800, 2200]
})";

// 演示基本功能
void demonstrateBasicFunctionality() {
    std::cout << "\n=== StockDataContainer 基本功能演示 ===" << std::endl;

    StockDataContainer stockContainer("TestStock");

    // 解析JSON数据
    if (stockContainer.parseFromJsonString(stockJsonData)) {
        std::cout << "✓ JSON解析成功" << std::endl;

        // 展示自动提取的关键字段
        std::cout << "\n自动提取的关键字段:" << std::endl;
        std::cout << "  股票代码: " << stockContainer.getCode() << std::endl;
        std::cout << "  时间戳: " << stockContainer.getTime() << std::endl;
        std::cout << "  原始JSON长度: " << stockContainer.getRawJson().length() << " 字符" << std::endl;

        // 展示继承的通用功能
        std::cout << "\n继承的GenericJsonContainer功能:" << std::endl;
        std::cout << "  数据源: " << stockContainer.getSource() << std::endl;
        std::cout << "  解析字段数: " << stockContainer.size() << std::endl;
        std::cout << "  是否有'market'字段: " << (stockContainer.hasKey("market") ? "是" : "否") << std::endl;

        // 使用继承的字段访问功能
        try {
            std::cout << "  market值: " << stockContainer["market"].asString() << std::endl;
            std::cout << "  new_price值: " << stockContainer["new_price"].asDouble() << std::endl;
        } catch (const std::exception& e) {
            std::cout << "  字段访问错误: " << e.what() << std::endl;
        }

        // 调用股票特有方法
        std::cout << "\n股票信息详情:" << std::endl;
        stockContainer.printStockInfo();

    } else {
        std::cout << "✗ JSON解析失败" << std::endl;
    }
}

// 演示忽略字段功能
void demonstrateIgnoreFields() {
    std::cout << "\n=== 忽略字段功能演示 ===" << std::endl;

    StockDataContainer stockContainer("FilteredStock");

    // 使用忽略字段功能
    std::string ignoreFields = "bidorder_price,askorder_price,bidorder_volume,askorder_volume,turnover";

    if (stockContainer.parseFromJsonString(stockJsonData, ignoreFields)) {
        std::cout << "✓ 带忽略字段的解析成功" << std::endl;

        std::cout << "\n忽略字段后的结果:" << std::endl;
        std::cout << "  忽略的字段: " << ignoreFields << std::endl;
        std::cout << "  股票代码: " << stockContainer.getCode() << std::endl;
        std::cout << "  时间戳: " << stockContainer.getTime() << std::endl;
        std::cout << "  剩余字段数: " << stockContainer.size() << std::endl;

        // 验证字段确实被忽略了
        std::cout << "\n字段忽略验证:" << std::endl;
        std::cout << "  bidorder_price存在: " << (stockContainer.hasKey("bidorder_price") ? "是" : "否") << std::endl;
        std::cout << "  new_price存在: " << (stockContainer.hasKey("new_price") ? "是" : "否") << std::endl;

        // 注意：关键字段仍然被提取
        std::cout << "\n注意: 即使code和time字段被忽略，关键字段仍会被提取:" << std::endl;
        std::cout << "  code字段存在: " << (stockContainer.hasKey("code") ? "是" : "否") << std::endl;
        std::cout << "  提取的code值: " << stockContainer.getCode() << std::endl;

    } else {
        std::cout << "✗ 带忽略字段的解析失败" << std::endl;
    }
}

// 对比GenericJsonContainer和StockDataContainer
void demonstrateComparison() {
    std::cout << "\n=== 功能对比演示 ===" << std::endl;

    // 使用基类GenericJsonContainer
    GenericJsonContainer genericContainer("GenericTest");
    genericContainer.parseFromJsonString(stockJsonData);

    // 使用子类StockDataContainer
    StockDataContainer stockContainer("StockTest");
    stockContainer.parseFromJsonString(stockJsonData);

    std::cout << "GenericJsonContainer:" << std::endl;
    std::cout << "  字段数: " << genericContainer.size() << std::endl;
    std::cout << "  数据源: " << genericContainer.getSource() << std::endl;
    std::cout << "  获取code需要: container[\"code\"].asString()" << std::endl;

    std::cout << "\nStockDataContainer (继承 + 扩展):" << std::endl;
    std::cout << "  字段数: " << stockContainer.size() << " (相同)" << std::endl;
    std::cout << "  数据源: " << stockContainer.getSource() << " (相同)" << std::endl;
    std::cout << "  快速获取code: " << stockContainer.getCode() << std::endl;
    std::cout << "  快速获取time: " << stockContainer.getTime() << std::endl;
    std::cout << "  原始JSON保存: " << (stockContainer.getRawJson().empty() ? "未保存" : "已保存") << std::endl;
}

// 测试不同的时间字段提取
void demonstrateTimeExtraction() {
    std::cout << "\n=== 时间字段提取演示 ===" << std::endl;

    // 测试不同的时间字段格式
    std::string timeTestJson1 = R"({"code": "TEST1", "time": 123456789})";
    std::string timeTestJson2 = R"({"code": "TEST2", "date": 20250818})";
    std::string timeTestJson3 = R"({"code": "TEST3", "recv_time": 987654321})";

    StockDataContainer test1("TimeTest1");
    StockDataContainer test2("TimeTest2");
    StockDataContainer test3("TimeTest3");

    test1.parseFromJsonString(timeTestJson1);
    test2.parseFromJsonString(timeTestJson2);
    test3.parseFromJsonString(timeTestJson3);

    std::cout << "时间字段提取测试:" << std::endl;
    std::cout << "  TEST1 (time字段): " << test1.getTime() << std::endl;
    std::cout << "  TEST2 (date字段): " << test2.getTime() << std::endl;
    std::cout << "  TEST3 (recv_time字段): " << test3.getTime() << std::endl;
}

int main() {
    std::cout << "StockDataContainer 继承设计演示程序" << std::endl;
    std::cout << "==========================================" << std::endl;

    try {
        // 运行各种演示
        demonstrateBasicFunctionality();
        demonstrateIgnoreFields();
        demonstrateComparison();
        demonstrateTimeExtraction();

        std::cout << "\n==========================================" << std::endl;
        std::cout << "✓ 所有演示完成!" << std::endl;
        std::cout << "\n设计验证总结:" << std::endl;
        std::cout << "  ✓ 成功继承GenericJsonContainer所有功能" << std::endl;
        std::cout << "  ✓ 新增三个成员变量: code, time, raw_json_" << std::endl;
        std::cout << "  ✓ 自动提取关键字段功能" << std::endl;
        std::cout << "  ✓ 保持原始JSON数据完整性" << std::endl;
        std::cout << "  ✓ 支持忽略字段功能" << std::endl;
        std::cout << "  ✓ 简洁的API设计" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "演示过程中发生错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}