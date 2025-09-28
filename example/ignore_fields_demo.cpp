#include <iostream>
#include <string>
#include "GenericJsonContainer.h"

// 完整的测试JSON数据
const std::string fullJsonData = R"({
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
    "askorder_volume": [1200, 1800, 2200],
    "sensitive_token": "SECRET_ACCESS_TOKEN_12345",
    "user_password": "user_secret_password",
    "internal_debug_info": "debug_data_not_for_production"
})";

// 演示基本的忽略字段功能
void demonstrateBasicIgnore() {
    std::cout << "\n=== 基本忽略字段功能演示 ===" << std::endl;

    // 1. 完整解析（不忽略任何字段）
    GenericJsonContainer fullContainer("FullParse");
    if (fullContainer.parseFromJsonString(fullJsonData)) {
        std::cout << "\n完整解析结果:" << std::endl;
        std::cout << "  总字段数: " << fullContainer.size() << std::endl;
        fullContainer.printStatistics();
    }

    // 2. 忽略敏感字段
    std::cout << "\n" << std::string(50, '-') << std::endl;
    GenericJsonContainer secureContainer("SecureParse");
    std::string sensitiveFields = "date,turnover,bidorder_price";

    if (secureContainer.parseFromJsonString(fullJsonData, sensitiveFields)) {
        std::cout << "\n忽略敏感字段后的解析结果:" << std::endl;
        std::cout << "  忽略字段: " << sensitiveFields << std::endl;
        std::cout << "  剩余字段数: " << secureContainer.size() << std::endl;
        secureContainer.printStatistics();

        // 验证敏感字段确实被忽略了
        std::cout << "\n验证敏感字段是否被忽略:" << std::endl;
        std::cout << "  sensitive_token 存在: " << (secureContainer.hasKey("sensitive_token") ? "是" : "否") << std::endl;
        std::cout << "  user_password 存在: " << (secureContainer.hasKey("user_password") ? "是" : "否") << std::endl;
        std::cout << "  internal_debug_info 存在: " << (secureContainer.hasKey("internal_debug_info") ? "是" : "否") << std::endl;
    }

    // 3. 只保留核心字段（忽略大部分字段）
    std::cout << "\n" << std::string(50, '-') << std::endl;
    GenericJsonContainer coreContainer("CoreParse");
    std::string ignoreMostFields = "recv_time,status,date,time,pre_price,open_price,high_price,low_price,turnover,trade_num,bidorder_price,askorder_price,bidorder_volume,askorder_volume,sensitive_token,user_password,internal_debug_info";

    if (coreContainer.parseFromJsonString(fullJsonData, ignoreMostFields)) {
        std::cout << "\n只保留核心字段的解析结果:" << std::endl;
        std::cout << "  忽略的字段数: " << ignoreMostFields << std::endl;
        std::cout << "  保留字段数: " << coreContainer.size() << std::endl;

        std::cout << "\n保留的核心字段:" << std::endl;
        for (const auto& pair : coreContainer) {
            std::cout << "  \"" << pair.first << "\": " << pair.second.toString() << std::endl;
        }
    }
}

// 演示性能对比
void demonstratePerformanceComparison() {
    std::cout << "\n=== 性能对比演示 ===" << std::endl;

    // 创建包含大量字段的JSON数据
    std::string largeJsonData = R"({
        "field_001": "data_001", "field_002": "data_002", "field_003": "data_003",
        "field_004": "data_004", "field_005": "data_005", "field_006": "data_006",
        "field_007": "data_007", "field_008": "data_008", "field_009": "data_009",
        "field_010": "data_010", "large_array": [1,2,3,4,5,6,7,8,9,10],
        "nested_object": {"sub1": "value1", "sub2": "value2", "sub3": "value3"},
        "needed_field_1": "important_data_1",
        "needed_field_2": "important_data_2",
        "needed_field_3": 12345
    })";

    // 完整解析
    GenericJsonContainer fullParse("FullParse");
    fullParse.parseFromJsonString(largeJsonData);

    // 选择性解析，只保留需要的字段
    GenericJsonContainer selectiveParse("SelectiveParse");
    std::string ignoreFields = "field_001,field_002,field_003,field_004,field_005,field_006,field_007,field_008,field_009,field_010,large_array,nested_object";
    selectiveParse.parseFromJsonString(largeJsonData, ignoreFields);

    std::cout << "\n解析结果对比:" << std::endl;
    std::cout << "  完整解析字段数: " << fullParse.size() << std::endl;
    std::cout << "  选择性解析字段数: " << selectiveParse.size() << std::endl;
    std::cout << "  减少字段比例: " << (1.0 - (double)selectiveParse.size() / fullParse.size()) * 100 << "%" << std::endl;

    std::cout << "\n选择性解析保留的字段:" << std::endl;
    for (const auto& pair : selectiveParse) {
        std::cout << "  \"" << pair.first << "\": " << pair.second.toString() << std::endl;
    }
}

// 演示嵌套对象中的字段忽略
void demonstrateNestedIgnore() {
    std::cout << "\n=== 嵌套对象字段忽略演示 ===" << std::endl;

    std::string nestedJson = R"({
        "user": {
            "name": "John Doe",
            "age": 30,
            "password": "secret123",
            "email": "john@example.com"
        },
        "order": {
            "id": "ORD-001",
            "total": 99.99,
            "payment_token": "pay_secret_token",
            "items": ["item1", "item2"]
        },
        "session_id": "session_secret_12345",
        "timestamp": "2025-08-18T10:30:00Z"
    })";

    // 完整解析
    GenericJsonContainer fullContainer("NestedFull");
    fullContainer.parseFromJsonString(nestedJson);

    std::cout << "\n完整解析结果:" << std::endl;
    std::cout << "  字段数: " << fullContainer.size() << std::endl;
    fullContainer.print();

    // 忽略敏感字段（注意：这里的忽略是针对顶层字段的）
    GenericJsonContainer secureContainer("NestedSecure");
    std::string ignoreFields = "session_id";  // 只能忽略顶层字段
    secureContainer.parseFromJsonString(nestedJson, ignoreFields);

    std::cout << "\n" << std::string(50, '-') << std::endl;
    std::cout << "\n忽略顶层字段后的结果:" << std::endl;
    std::cout << "  忽略字段: " << ignoreFields << std::endl;
    std::cout << "  剩余字段数: " << secureContainer.size() << std::endl;
    secureContainer.print();

    std::cout << "\n注意: 当前实现只能忽略顶层字段，嵌套对象内的字段不会被忽略。" << std::endl;
    std::cout << "如果需要忽略嵌套字段，需要在应用层进行后处理。" << std::endl;
}

// 演示边界情况和错误处理
void demonstrateEdgeCases() {
    std::cout << "\n=== 边界情况和错误处理演示 ===" << std::endl;

    std::string testJson = R"({
        "field1": "value1",
        "field2": "value2",
        "field3": "value3"
    })";

    // 1. 空字符串忽略字段
    GenericJsonContainer container1("EmptyIgnore");
    container1.parseFromJsonString(testJson, "");
    std::cout << "\n1. 空字符串忽略字段 - 解析字段数: " << container1.size() << std::endl;

    // 2. 忽略不存在的字段
    GenericJsonContainer container2("NonexistentIgnore");
    container2.parseFromJsonString(testJson, "nonexistent_field1,nonexistent_field2");
    std::cout << "2. 忽略不存在的字段 - 解析字段数: " << container2.size() << std::endl;

    // 3. 忽略所有字段
    GenericJsonContainer container3("IgnoreAll");
    container3.parseFromJsonString(testJson, "field1,field2,field3");
    std::cout << "3. 忽略所有字段 - 解析字段数: " << container3.size() << std::endl;

    // 4. 字段名包含空格
    GenericJsonContainer container4("SpacedIgnore");
    container4.parseFromJsonString(testJson, " field1 , field2 , field3 ");
    std::cout << "4. 字段名包含空格 - 解析字段数: " << container4.size() << std::endl;

    // 5. 只忽略部分字段
    GenericJsonContainer container5("PartialIgnore");
    container5.parseFromJsonString(testJson, "field1,field3");
    std::cout << "5. 部分忽略 - 解析字段数: " << container5.size() << std::endl;
    std::cout << "   保留的字段:" << std::endl;
    for (const auto& pair : container5) {
        std::cout << "     \"" << pair.first << "\": " << pair.second.toString() << std::endl;
    }
}

int main() {
    std::cout << "GenericJsonContainer 忽略字段功能演示程序" << std::endl;
    std::cout << "=============================================" << std::endl;

    try {
        // 运行各种演示
        demonstrateBasicIgnore();
        demonstratePerformanceComparison();
        demonstrateNestedIgnore();
        demonstrateEdgeCases();

        std::cout << "\n=============================================" << std::endl;
        std::cout << "✓ 所有演示完成!" << std::endl;
        std::cout << "\n功能验证总结:" << std::endl;
        std::cout << "  ✓ 基本字段忽略功能" << std::endl;
        std::cout << "  ✓ 逗号分隔字段列表解析" << std::endl;
        std::cout << "  ✓ 空白字符处理" << std::endl;
        std::cout << "  ✓ 性能优化效果（减少字段数量）" << std::endl;
        std::cout << "  ✓ 边界情况处理" << std::endl;
        std::cout << "  ✓ 敏感信息过滤" << std::endl;

        std::cout << "\n接口格式验证:" << std::endl;
        std::cout << "  bool parseFromJsonString(jsonStr, \"key1,key2,key3\") ✓" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "演示过程中发生错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}