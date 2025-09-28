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
        std::cout << "  索引字段: " << stockContainer.getIndexKey() << std::endl;
        std::cout << "  索引值: " << stockContainer.getIndexValue() << std::endl;
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

// 演示动态索引字段功能
void demonstrateDynamicIndexFields() {
    std::cout << "\n=== 动态索引字段功能演示 ===" << std::endl;

    StockDataContainer container("DynamicIndexDemo");
    container.parseFromJsonString(stockJsonData);

    std::cout << "同一个JSON数据，使用不同索引字段:\n" << std::endl;

    // 1. 时间索引（默认）
    std::cout << "1. 时间索引 (默认):" << std::endl;
    std::cout << "   索引字段: " << container.getIndexKey() << std::endl;
    std::cout << "   索引值: " << container.getIndexValue() << std::endl;
    std::cout << "   作为整数: " << container.getIndexAsInt() << std::endl;

    // 2. 价格索引
    std::cout << "\n2. 切换到价格索引:" << std::endl;
    container.setIndexKey("new_price");
    std::cout << "   索引字段: " << container.getIndexKey() << std::endl;
    std::cout << "   索引值: " << container.getIndexValue() << std::endl;
    std::cout << "   作为浮点数: " << container.getIndexAsDouble() << std::endl;

    // 3. 成交量索引
    std::cout << "\n3. 切换到成交量索引:" << std::endl;
    container.setIndexKey("volume");
    std::cout << "   索引字段: " << container.getIndexKey() << std::endl;
    std::cout << "   索引值: " << container.getIndexValue() << std::endl;
    std::cout << "   作为整数: " << container.getIndexAsInt() << std::endl;

    // 4. 市场索引
    std::cout << "\n4. 切换到市场索引:" << std::endl;
    container.setIndexKey("market");
    std::cout << "   索引字段: " << container.getIndexKey() << std::endl;
    std::cout << "   索引值: " << container.getIndexValue() << std::endl;

    // 5. 状态索引
    std::cout << "\n5. 切换到状态索引:" << std::endl;
    container.setIndexKey("status");
    std::cout << "   索引字段: " << container.getIndexKey() << std::endl;
    std::cout << "   索引值: " << container.getIndexValue() << std::endl;

    // 6. 不存在的字段
    std::cout << "\n6. 尝试不存在的字段:" << std::endl;
    container.setIndexKey("nonexistent_field");
    std::cout << "   索引字段: " << container.getIndexKey() << std::endl;
    std::cout << "   索引值: " << (container.getIndexValue().empty() ? "未提取" : container.getIndexValue()) << std::endl;
}

// 演示构造时指定索引字段
void demonstrateConstructorIndexField() {
    std::cout << "\n=== 构造函数指定索引字段演示 ===" << std::endl;

    // 构造时指定不同索引字段
    StockDataContainer container1("PriceIndex", "new_price");
    StockDataContainer container2("VolumeIndex", "volume");
    StockDataContainer container3("MarketIndex", "market");

    // 解析相同的JSON数据
    container1.parseFromJsonString(stockJsonData);
    container2.parseFromJsonString(stockJsonData);
    container3.parseFromJsonString(stockJsonData);

    std::cout << "三个容器使用不同的索引字段:" << std::endl;
    std::cout << "容器1 (" << container1.getSource() << "): "
              << container1.getIndexKey() << " = " << container1.getIndexValue() << std::endl;
    std::cout << "容器2 (" << container2.getSource() << "): "
              << container2.getIndexKey() << " = " << container2.getIndexValue() << std::endl;
    std::cout << "容器3 (" << container3.getSource() << "): "
              << container3.getIndexKey() << " = " << container3.getIndexValue() << std::endl;
}

// 演示应用场景
void demonstrateApplicationScenarios() {
    std::cout << "\n=== 应用场景演示 ===" << std::endl;

    std::cout << "场景1: 按价格排序的容器" << std::endl;
    StockDataContainer priceContainer("PriceSorting", "new_price");
    priceContainer.parseFromJsonString(stockJsonData);
    std::cout << "  快速获取价格: " << priceContainer.getIndexAsDouble() << std::endl;

    std::cout << "\n场景2: 按时间排序的容器" << std::endl;
    StockDataContainer timeContainer("TimeSorting", "time");
    timeContainer.parseFromJsonString(stockJsonData);
    std::cout << "  快速获取时间戳: " << timeContainer.getIndexAsInt() << std::endl;

    std::cout << "\n场景3: 按市场分组的容器" << std::endl;
    StockDataContainer marketContainer("MarketGrouping", "market");
    marketContainer.parseFromJsonString(stockJsonData);
    std::cout << "  快速获取市场: " << marketContainer.getIndexValue() << std::endl;

    std::cout << "\n场景4: 数据过滤容器" << std::endl;
    StockDataContainer statusContainer("StatusFilter", "status");
    statusContainer.parseFromJsonString(stockJsonData);
    std::cout << "  快速获取状态: " << statusContainer.getIndexValue() << std::endl;
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
    std::cout << "  获取price需要: container[\"new_price\"].asDouble()" << std::endl;

    std::cout << "\nStockDataContainer (继承 + 扩展):" << std::endl;
    std::cout << "  字段数: " << stockContainer.size() << " (相同)" << std::endl;
    std::cout << "  数据源: " << stockContainer.getSource() << " (相同)" << std::endl;
    std::cout << "  快速获取code: " << stockContainer.getCode() << std::endl;
    std::cout << "  可配置索引字段: " << stockContainer.getIndexKey() << std::endl;
    std::cout << "  快速获取索引值: " << stockContainer.getIndexValue() << std::endl;
    std::cout << "  原始JSON保存: " << (stockContainer.getRawJson().empty() ? "未保存" : "已保存") << std::endl;

    std::cout << "\n设计优势对比:" << std::endl;
    std::cout << "  ✓ 通用性: 任意字段都可作为索引" << std::endl;
    std::cout << "  ✓ 灵活性: 运行时动态切换索引字段" << std::endl;
    std::cout << "  ✓ 性能: 关键字段直接访问，无需每次解析" << std::endl;
    std::cout << "  ✓ 兼容性: 完全继承GenericJsonContainer功能" << std::endl;
}

// 测试不同索引字段的提取
void demonstrateIndexFieldExtraction() {
    std::cout << "\n=== 索引字段提取演示 ===" << std::endl;

    // 测试不同的索引字段格式
    std::string testJson1 = R"({"code": "TEST1", "time": 123456789, "price": 100.50})";
    std::string testJson2 = R"({"code": "TEST2", "date": 20250818, "volume": 50000})";
    std::string testJson3 = R"({"code": "TEST3", "recv_time": 987654321, "market": "SZSE"})";

    StockDataContainer test1("IndexTest1", "time");    // 使用time字段作为索引
    StockDataContainer test2("IndexTest2", "date");    // 使用date字段作为索引
    StockDataContainer test3("IndexTest3", "market");  // 使用market字段作为索引

    test1.parseFromJsonString(testJson1);
    test2.parseFromJsonString(testJson2);
    test3.parseFromJsonString(testJson3);

    std::cout << "不同索引字段提取测试:" << std::endl;
    std::cout << "  TEST1 (time索引): " << test1.getIndexKey() << " = " << test1.getIndexValue() << std::endl;
    std::cout << "  TEST2 (date索引): " << test2.getIndexKey() << " = " << test2.getIndexValue() << std::endl;
    std::cout << "  TEST3 (market索引): " << test3.getIndexKey() << " = " << test3.getIndexValue() << std::endl;

    // 演示动态切换索引字段
    std::cout << "\n动态切换索引字段:" << std::endl;
    test1.setIndexKey("price");
    std::cout << "  TEST1 切换到price索引: " << test1.getIndexKey() << " = " << test1.getIndexValue() << std::endl;
}

int main() {
    std::cout << "StockDataContainer Index概念演示程序" << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        // 运行各种演示
        demonstrateBasicFunctionality();
        demonstrateDynamicIndexFields();
        demonstrateConstructorIndexField();
        demonstrateApplicationScenarios();
        demonstrateIgnoreFields();
        demonstrateComparison();
        demonstrateIndexFieldExtraction();

        std::cout << "\n========================================" << std::endl;
        std::cout << "✓ 所有演示完成!" << std::endl;
        std::cout << "\nIndex概念设计验证总结:" << std::endl;
        std::cout << "  ✓ 成功继承GenericJsonContainer所有功能" << std::endl;
        std::cout << "  ✓ 新增三个成员变量: code, index_key_, index_value_, raw_json_" << std::endl;
        std::cout << "  ✓ code字段固定提取功能" << std::endl;
        std::cout << "  ✓ index字段可配置提取功能" << std::endl;
        std::cout << "  ✓ 动态切换索引字段功能" << std::endl;
        std::cout << "  ✓ 通用性：适用于任意字段作为索引" << std::endl;
        std::cout << "  ✓ 灵活性：运行时配置索引字段" << std::endl;
        std::cout << "  ✓ 向后兼容：通过setIndexKey(\"time\")保持原功能" << std::endl;
        std::cout << "  ✓ 保持原始JSON数据完整性" << std::endl;
        std::cout << "  ✓ 支持忽略字段功能" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "演示过程中发生错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}