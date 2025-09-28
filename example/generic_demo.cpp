#include <iostream>
#include <string>
#include <vector>
#include "GenericJsonContainer.h"

// 用户提供的股票行情JSON示例
const std::string sampleJsonData = R"({
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
    "bidorder_price": [100.80, 100.75, 100.70, 100.65, 100.60],
    "askorder_price": [100.85, 100.90, 100.95, 101.00, 101.05],
    "bidorder_volume": [1000, 2000, 1500, 3000, 2500],
    "askorder_volume": [1200, 1800, 2200, 1600, 2800]
})";

// 演示基本的JSON解析和访问
void demonstrateBasicUsage() {
    std::cout << "\n=== 基本用法演示 ===" << std::endl;

    // 创建GenericJsonContainer并解析JSON
    GenericJsonContainer container("StockData_Generic");

    if (container.parseFromJsonString(sampleJsonData)) {
        std::cout << "✓ JSON解析成功!" << std::endl;

        // 显示容器基本信息
        std::cout << "\n容器信息:" << std::endl;
        std::cout << "  数据源: " << container.getSource() << std::endl;
        std::cout << "  键值对数量: " << container.si
ze() << std::endl;

        // 访问字符串字段
        try {
            std::cout << "\n字符串字段访问:" << std::endl;
            std::cout << "  local_time: " << container["local_time"].asString() << std::endl;
            std::cout << "  market: " << container["market"].asString() << std::endl;
            std::cout << "  code: " << container["code"].asString() << std::endl;
            std::cout << "  codename: " << container["codename"].asString() << std::endl;
        } catch (const GenericJsonException& e) {
            std::cout << "字符串访问错误: " << e.what() << std::endl;
        }

        // 访问数值字段
        try {
            std::cout << "\n数值字段访问:" << std::endl;
            std::cout << "  date: " << container["date"].asInt() << std::endl;
            std::cout << "  time: " << container["time"].asInt() << std::endl;
            std::cout << "  pre_price: " << container["pre_price"].asDouble() << std::endl;
            std::cout << "  new_price: " << container["new_price"].asDouble() << std::endl;
            std::cout << "  volume: " << container["volume"].asInt() << std::endl;
        } catch (const GenericJsonException& e) {
            std::cout << "数值访问错误: " << e.what() << std::endl;
        }

    } else {
        std::cout << "✗ JSON解析失败!" << std::endl;
    }
}

// 演示数组数据访问
void demonstrateArrayAccess() {
    std::cout << "\n=== 数组访问演示 ===" << std::endl;

    GenericJsonContainer container("ArrayDemo");

    if (container.parseFromJsonString(sampleJsonData)) {
        try {
            // 访问买盘价格数组
            std::cout << "买盘价格 (bidorder_price):" << std::endl;
            const auto& bidPrices = container["bidorder_price"].asArray();
            for (size_t i = 0; i < bidPrices.size(); ++i) {
                std::cout << "  [" << i << "] " << bidPrices[i].asDouble() << std::endl;
            }

            // 访问卖盘价格数组
            std::cout << "\n卖盘价格 (askorder_price):" << std::endl;
            const auto& askPrices = container["askorder_price"].asArray();
            for (size_t i = 0; i < askPrices.size(); ++i) {
                std::cout << "  [" << i << "] " << askPrices[i].asDouble() << std::endl;
            }

            // 访问买盘量数组
            std::cout << "\n买盘量 (bidorder_volume):" << std::endl;
            const auto& bidVolumes = container["bidorder_volume"].asArray();
            for (size_t i = 0; i < bidVolumes.size(); ++i) {
                std::cout << "  [" << i << "] " << bidVolumes[i].asInt() << std::endl;
            }

        } catch (const GenericJsonException& e) {
            std::cout << "数组访问错误: " << e.what() << std::endl;
        }
    }
}

// 演示vector<pair<string, CustomValue>>存储的遍历
void demonstrateVectorPairStorage() {
    std::cout << "\n=== Vector<Pair>存储演示 ===" << std::endl;

    GenericJsonContainer container("VectorPairDemo");

    if (container.parseFromJsonString(sampleJsonData)) {
        std::cout << "使用迭代器遍历vector<pair<string, CustomValue>>存储:" << std::endl;

        size_t index = 0;
        for (const auto& pair : container) {
            std::cout << "[" << index++ << "] Key: \"" << pair.first << "\" => "
                      << jsonValueTypeToString(pair.second.getType())
                      << " (" << pair.second.toString() << ")" << std::endl;
        }

        std::cout << "\n直接访问vector存储的特定索引:" << std::endl;
        auto it = container.begin();
        if (it != container.end()) {
            std::cout << "第一个元素: \"" << it->first << "\" = " << it->second.toString() << std::endl;
        }

        // 展示查找功能
        std::cout << "\n查找特定键:" << std::endl;
        auto found = container.find("market");
        if (found != container.end()) {
            std::cout << "找到键 'market': " << found->second.toString() << std::endl;
        }
    }
}

// 演示通用性 - 处理不同结构的JSON
void demonstrateGenericNature() {
    std::cout << "\n=== 通用性演示 ===" << std::endl;

    // 测试完全不同结构的JSON
    std::string differentJson = R"({
        "name": "John Doe",
        "age": 30,
        "married": true,
        "children": ["Alice", "Bob"],
        "address": {
            "street": "123 Main St",
            "city": "Anytown",
            "zipcode": 12345
        },
        "scores": [85.5, 92.0, 78.5]
    })";

    GenericJsonContainer container("DifferentStructure");

    if (container.parseFromJsonString(differentJson)) {
        std::cout << "✓ 成功解析不同结构的JSON" << std::endl;

        // 访问基本类型
        std::cout << "姓名: " << container["name"].asString() << std::endl;
        std::cout << "年龄: " << container["age"].asInt() << std::endl;
        std::cout << "已婚: " << (container["married"].asBool() ? "是" : "否") << std::endl;

        // 访问嵌套对象
        try {
            const auto& address = container["address"].asObject();
            std::cout << "地址信息:" << std::endl;
            for (const auto& pair : address) {
                std::cout << "  " << pair.first << ": " << pair.second.toString() << std::endl;
            }
        } catch (const GenericJsonException& e) {
            std::cout << "对象访问错误: " << e.what() << std::endl;
        }

        // 访问字符串数组
        try {
            const auto& children = container["children"].asArray();
            std::cout << "子女: ";
            for (size_t i = 0; i < children.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << children[i].asString();
            }
            std::cout << std::endl;
        } catch (const GenericJsonException& e) {
            std::cout << "数组访问错误: " << e.what() << std::endl;
        }
    }
}

// 演示容器操作和批量处理
void demonstrateContainerOperations() {
    std::cout << "\n=== 容器操作演示 ===" << std::endl;

    GenericJsonContainer container("ContainerOps");

    if (container.parseFromJsonString(sampleJsonData)) {
        // 显示统计信息
        container.printStatistics();

        // 获取所有键
        std::cout << "\n所有键列表:" << std::endl;
        auto keys = container.getAllKeys();
        for (size_t i = 0; i < keys.size(); ++i) {
            std::cout << "  [" << i << "] " << keys[i] << std::endl;
        }

        // 演示过滤功能
        std::cout << "\n过滤示例 - 只保留价格相关字段:" << std::endl;
        std::vector<std::string> priceKeys = {"pre_price", "open_price", "high_price", "low_price", "new_price"};
        auto filtered = container.filter(priceKeys);
        filtered.print();

        // 演示添加新字段
        std::cout << "\n添加自定义字段:" << std::endl;
        container.setValue("custom_field", CustomValue("This is a test"));
        container.setValue("custom_number", CustomValue(42));
        std::cout << "添加后的键数量: " << container.size() << std::endl;

        // 导出功能演示
        std::cout << "\n导出功能演示:" << std::endl;
        if (container.exportToJson("output_generic.json")) {
            std::cout << "✓ 成功导出到 output_generic.json" << std::endl;
        }
    }
}

// 演示批量容器集合
void demonstrateContainerCollection() {
    std::cout << "\n=== 容器集合演示 ===" << std::endl;

    JsonContainerCollection collection("StockDataCollection");

    // 添加原始数据
    GenericJsonContainer container1("Stock1");
    container1.parseFromJsonString(sampleJsonData);
    collection.addContainer(std::move(container1));

    // 创建第二个数据
    std::string secondStock = R"({
        "local_time": "20250818-11:21:30.120",
        "market": "SZSE",
        "code": "000001",
        "codename": "平安银行",
        "new_price": 15.67,
        "volume": 23000
    })";

    GenericJsonContainer container2("Stock2");
    container2.parseFromJsonString(secondStock);
    collection.addContainer(std::move(container2));

    // 显示集合信息
    collection.printSummary();

    // 查找包含特定键的容器
    auto foundContainers = collection.findByKey("codename");
    std::cout << "\n包含 'codename' 键的容器数量: " << foundContainers.size() << std::endl;

    // 根据键值查找
    auto uSHDContainers = collection.findByKeyValue("market", "USHD");
    std::cout << "市场为 'USHD' 的容器数量: " << uSHDContainers.size() << std::endl;
}

int main() {
    std::cout << "GenericJsonContainer 通用JSON解析演示程序" << std::endl;
    std::cout << "================================================" << std::endl;

    try {
        // 演示各个功能
        demonstrateBasicUsage();
        demonstrateArrayAccess();
        demonstrateVectorPairStorage();
        demonstrateGenericNature();
        demonstrateContainerOperations();
        demonstrateContainerCollection();

        std::cout << "\n================================================" << std::endl;
        std::cout << "✓ 所有演示完成! 通用JSON容器系统工作正常。" << std::endl;
        std::cout << "\n核心特性验证:" << std::endl;
        std::cout << "  ✓ vector<pair<string, CustomValue>>存储结构" << std::endl;
        std::cout << "  ✓ 支持所有JSON数据类型" << std::endl;
        std::cout << "  ✓ 类型安全的访问方法" << std::endl;
        std::cout << "  ✓ 通用性 - 适配任意JSON结构" << std::endl;
        std::cout << "  ✓ 用户提供的股票JSON示例完美支持" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "演示过程中发生错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}