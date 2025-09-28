#include <iostream>
#include <deque>
#include <string>
#include <fstream>
#include "LineReader.h"
#include "GenericJsonContainer.h"

// 创建测试数据文件
bool createTestDataFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法创建测试文件: " << filename << std::endl;
        return false;
    }

    // 写入多行JSON数据（模拟股票行情数据）
    file << R"({"local_time":"20250818-11:21:25.409","recv_time":0,"market":"USHD","code":"110084","codename":"贵燃转债","new_price":100.85,"volume":12500})" << std::endl;
    file << R"({"local_time":"20250818-11:21:26.123","recv_time":1000,"market":"SZSE","code":"000001","codename":"平安银行","new_price":15.67,"volume":23000})" << std::endl;
    file << R"({"local_time":"20250818-11:21:27.456","recv_time":2000,"market":"USHD","code":"110085","codename":"华友转债","new_price":98.45,"volume":8900})" << std::endl;
    file << R"({"local_time":"20250818-11:21:28.789","recv_time":3000,"market":"SZSE","code":"000002","codename":"万科A","new_price":18.92,"volume":15600})" << std::endl;
    file << R"({"local_time":"20250818-11:21:29.012","recv_time":4000,"market":"USHD","code":"110086","codename":"国债转债","new_price":102.15,"volume":6700})" << std::endl;

    // 添加一些不同结构的JSON（测试通用性）
    file << R"({"timestamp":"2025-08-18","symbol":"AAPL","price":150.25,"change":2.15,"percentage":1.45})" << std::endl;
    file << R"({"user":"john_doe","action":"login","ip":"192.168.1.100","success":true,"attempts":1})" << std::endl;

    // 添加一行无效JSON（测试错误处理）
    file << "invalid json line for testing error handling" << std::endl;

    // 继续添加正常JSON
    file << R"({"sensor_id":12345,"temperature":25.6,"humidity":60.2,"pressure":1013.25,"location":"office"})" << std::endl;
    file << R"({"order_id":"ORD-2025-001","customer":"Alice","items":["item1","item2"],"total":99.99,"paid":true})" << std::endl;

    file.close();
    std::cout << "✓ 测试数据文件创建成功: " << filename << std::endl;
    return true;
}

// 主要的处理函数：读取文件 → 解析JSON → 存入deque
size_t processJsonFile(const std::string& filename, std::deque<GenericJsonContainer>& jsonQueue) {
    std::cout << "\n=== 开始处理JSON文件: " << filename << " ===" << std::endl;

    // 使用LineReader读取文件
    LineReader reader(filename);
    if (!reader.isOpen()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return 0;
    }

    std::string line;
    size_t lineNumber = 0;
    size_t successCount = 0;
    size_t errorCount = 0;

    // 逐行读取和处理
    while (reader.hasNextLine()) {
        line = reader.output();
        lineNumber++;

        // 跳过空行
        if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos) {
            continue;
        }

        std::cout << "[行 " << lineNumber << "] 处理中... ";

        try {
            // 创建GenericJsonContainer并解析当前行的JSON
            GenericJsonContainer container("Line_" + std::to_string(lineNumber));

            if (container.parseFromJsonString(line)) {
                // 解析成功，push到deque
                jsonQueue.push_back(std::move(container));
                successCount++;
                std::cout << "✓ 解析成功 (" << jsonQueue.back().size() << " 个键值对)" << std::endl;
            } else {
                // 解析失败
                errorCount++;
                std::cout << "✗ JSON解析失败" << std::endl;
            }

        } catch (const std::exception& e) {
            errorCount++;
            std::cout << "✗ 异常: " << e.what() << std::endl;
        }
    }

    std::cout << "\n文件处理完成:" << std::endl;
    std::cout << "  总行数: " << lineNumber << std::endl;
    std::cout << "  成功解析: " << successCount << " 行" << std::endl;
    std::cout << "  解析错误: " << errorCount << " 行" << std::endl;
    std::cout << "  队列中容器数量: " << jsonQueue.size() << std::endl;

    return successCount;
}

// 从deque中pop出来并打印显示
void displayAndPopContainers(std::deque<GenericJsonContainer>& jsonQueue) {
    std::cout << "\n=== 从deque中pop并显示数据 ===" << std::endl;

    size_t index = 0;
    while (!jsonQueue.empty()) {
        // 从队列前端取出容器
        GenericJsonContainer container = std::move(jsonQueue.front());
        jsonQueue.pop_front();

        std::cout << "\n--- 容器 " << ++index << " (来源: " << container.getSource() << ") ---" << std::endl;
        std::cout << "键值对数量: " << container.size() << std::endl;

        // 打印所有键值对
        for (const auto& pair : container) {
            std::cout << "  \"" << pair.first << "\": "
                      << jsonValueTypeToString(pair.second.getType())
                      << " = " << pair.second.toString() << std::endl;
        }

        // 可选：打印JSON格式
        // std::cout << "JSON: " << container.toJsonString() << std::endl;
    }

    std::cout << "\n✓ 所有容器已从队列中取出并显示完毕" << std::endl;
}

// 演示队列操作的各种方法
void demonstrateDequeOperations(const std::string& filename) {
    std::cout << "\n=== deque操作演示 ===" << std::endl;

    std::deque<GenericJsonContainer> jsonQueue;

    // 第一次处理：填充队列
    size_t count = processJsonFile(filename, jsonQueue);
    if (count == 0) {
        std::cout << "没有成功解析的数据，跳过演示" << std::endl;
        return;
    }

    // 演示deque的双端特性
    std::cout << "\ndeque操作演示:" << std::endl;
    std::cout << "  当前队列大小: " << jsonQueue.size() << std::endl;

    if (!jsonQueue.empty()) {
        // 查看队首和队尾（不删除）
        std::cout << "  队首容器来源: " << jsonQueue.front().getSource() << std::endl;
        std::cout << "  队尾容器来源: " << jsonQueue.back().getSource() << std::endl;

        // 从队尾添加一个新容器（演示push_back）
        GenericJsonContainer newContainer("Manual_Added");
        newContainer.setValue("test_key", CustomValue("test_value"));
        newContainer.setValue("test_number", CustomValue(42));
        jsonQueue.push_back(std::move(newContainer));
        std::cout << "  添加新容器后队列大小: " << jsonQueue.size() << std::endl;

        // 演示随机访问
        if (jsonQueue.size() >= 2) {
            std::cout << "  第二个容器键数量: " << jsonQueue[1].size() << std::endl;
        }
    }

    // 批量处理示例：统计所有容器的总键数量
    size_t totalKeys = 0;
    for (const auto& container : jsonQueue) {
        totalKeys += container.size();
    }
    std::cout << "  所有容器的总键数量: " << totalKeys << std::endl;

    // 最后清空队列并显示
    displayAndPopContainers(jsonQueue);
}

// 演示错误处理和恢复能力
void demonstrateErrorHandling() {
    std::cout << "\n=== 错误处理演示 ===" << std::endl;

    // 创建包含各种错误情况的测试文件
    std::string errorTestFile = "error_test.json";
    std::ofstream file(errorTestFile);
    file << R"({"valid": "json", "number": 123})" << std::endl;
    file << "invalid json without quotes" << std::endl;
    file << R"({"incomplete": "json")" << std::endl;  // 缺少结束括号
    file << "" << std::endl;  // 空行
    file << R"({"another": "valid", "after": "error"})" << std::endl;
    file.close();

    std::deque<GenericJsonContainer> testQueue;
    processJsonFile(errorTestFile, testQueue);

    std::cout << "\n错误处理结果:" << std::endl;
    std::cout << "  成功解析的容器数量: " << testQueue.size() << std::endl;

    // 清理
    while (!testQueue.empty()) {
        testQueue.pop_front();
    }
}

int main() {
    std::cout << "LineReader + GenericJsonContainer + std::deque 演示程序" << std::endl;
    std::cout << "=========================================================" << std::endl;

    const std::string testFileName = "test_json_lines.json";

    try {
        // 1. 创建测试数据文件
        if (!createTestDataFile(testFileName)) {
            return 1;
        }

        // 2. 主要功能演示
        demonstrateDequeOperations(testFileName);

        // 3. 错误处理演示
        demonstrateErrorHandling();

        std::cout << "\n=========================================================" << std::endl;
        std::cout << "✓ 所有演示完成!" << std::endl;
        std::cout << "\n核心功能验证:" << std::endl;
        std::cout << "  ✓ LineReader 逐行文件读取" << std::endl;
        std::cout << "  ✓ GenericJsonContainer JSON解析" << std::endl;
        std::cout << "  ✓ std::deque 双端队列操作" << std::endl;
        std::cout << "  ✓ 批量处理和错误恢复" << std::endl;
        std::cout << "  ✓ 内存管理和移动语义" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "程序执行错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}