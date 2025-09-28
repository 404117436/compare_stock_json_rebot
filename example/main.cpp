#include <iostream>
#include <iomanip>
#include "../include/JsonParser.h"

// 创建一个示例JSON字符串
const char* sampleJson = R"({
    "user": {
        "name": "张三",
        "age": 25,
        "email": "zhangsan@example.com",
        "active": true,
        "salary": 5000.50
    },
    "hobbies": ["阅读", "游泳", "编程"],
    "config": {
        "theme": "dark",
        "language": "zh-CN",
        "notifications": {
            "email": true,
            "push": false
        }
    },
    "scores": [85, 92, 78, 96, 88],
    "metadata": null
})";

void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << std::string(50, '=') << std::endl;
}

int main() {
    try {
        printSeparator("RapidJSON C++11 简易解析类示例");

        // 1. 基本解析和访问
        printSeparator("1. 基本JSON解析");
        JsonParser parser(sampleJson);

        std::cout << "✓ JSON解析成功！" << std::endl;
        std::cout << "文档有效: " << (parser.isValid() ? "是" : "否") << std::endl;

        // 2. 访问基本数据类型
        printSeparator("2. 访问基本数据类型");
        std::cout << "用户姓名: " << parser["user"]["name"].asString() << std::endl;
        std::cout << "用户年龄: " << parser["user"]["age"].asInt() << std::endl;
        std::cout << "用户邮箱: " << parser["user"]["email"].asString() << std::endl;
        std::cout << "用户状态: " << (parser["user"]["active"].asBool() ? "活跃" : "非活跃") << std::endl;
        std::cout << "用户工资: " << std::fixed << std::setprecision(2)
                  << parser["user"]["salary"].asDouble() << std::endl;

        // 3. 使用便捷方法
        printSeparator("3. 使用便捷方法");
        std::cout << "配置主题: " << parser["config"]["theme"].asString() << std::endl;
        std::cout << "配置语言: " << parser["config"]["language"].asString() << std::endl;

        // 4. 安全访问（检查键是否存在）
        printSeparator("4. 安全访问");
        auto userObj = parser["user"];
        if (userObj.hasKey("phone")) {
            std::cout << "用户电话: " << userObj["phone"].asString() << std::endl;
        } else {
            std::cout << "用户电话: 未提供" << std::endl;
        }

        // 检查空值
        auto metadata = parser["metadata"];
        if (metadata.isNull()) {
            std::cout << "元数据: null" << std::endl;
        }

        // 5. 数组处理
        printSeparator("5. 数组处理");
        auto hobbies = parser.getArray("hobbies");
        std::cout << "爱好数量: " << hobbies.arraySize() << std::endl;
        std::cout << "爱好列表: ";
        for (size_t i = 0; i < hobbies.arraySize(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << hobbies[i].asString();
        }
        std::cout << std::endl;

        // 6. C++11 range-based for 循环
        printSeparator("6. C++11 Range-based For 循环");
        auto scores = parser.getArray("scores");
        std::cout << "分数列表: ";
        for (const auto& score : scores) {
            std::cout << score.asInt() << " ";
        }
        std::cout << std::endl;

        // 计算平均分
        int total = 0, count = 0;
        for (const auto& score : scores) {
            total += score.asInt();
            count++;
        }
        std::cout << "平均分: " << std::fixed << std::setprecision(1)
                  << static_cast<double>(total) / count << std::endl;

        // 7. 对象遍历
        printSeparator("7. 对象遍历");
        auto configObj = parser.getObject("config");
        std::cout << "配置项: " << std::endl;
        for (const auto& key : configObj.getKeys()) {
            auto value = configObj[key];
            std::cout << "  " << key << ": ";
            if (value.isString()) {
                std::cout << value.asString();
            } else if (value.isBool()) {
                std::cout << (value.asBool() ? "true" : "false");
            } else if (value.isObject()) {
                std::cout << "[对象]";
            }
            std::cout << std::endl;
        }

        // 8. 嵌套对象访问
        printSeparator("8. 嵌套对象访问");
        auto notifications = parser["config"]["notifications"];
        std::cout << "通知设置:" << std::endl;
        std::cout << "  邮件通知: " << (notifications["email"].asBool() ? "开启" : "关闭") << std::endl;
        std::cout << "  推送通知: " << (notifications["push"].asBool() ? "开启" : "关闭") << std::endl;

        // 9. JSON输出
        printSeparator("9. JSON输出");
        std::cout << "紧凑格式:" << std::endl;
        std::cout << parser.toString() << std::endl;

        std::cout << "\n美化格式:" << std::endl;
        std::cout << parser.toPrettyString() << std::endl;

        // 10. 从文件读取（创建示例文件）
        printSeparator("10. 文件操作");

        // 创建一个简单的JSON文件
        std::ofstream file("test.json");
        file << R"({
    "app": "JsonParser测试",
    "version": "1.0.0",
    "features": ["简单易用", "类型安全", "C++11支持"]
})";
        file.close();

        // 从文件读取
        auto fileParser = JsonParser::fromFile("test.json");
        std::cout << "从文件读取的应用名: " << fileParser.getString("app") << std::endl;
        std::cout << "版本: " << fileParser.getString("version") << std::endl;

        auto features = fileParser.getArray("features");
        std::cout << "特性: ";
        for (const auto& feature : features) {
            std::cout << feature.asString() << " ";
        }
        std::cout << std::endl;

        // 11. 验证JSON结构
        printSeparator("11. 结构验证");
        std::vector<std::string> requiredKeys = {"user", "hobbies", "config"};
        bool structureValid = parser.validateStructure(requiredKeys);
        std::cout << "结构验证: " << (structureValid ? "通过" : "失败") << std::endl;

        // 12. 静态验证方法
        printSeparator("12. 静态验证");
        std::string testJson = R"({"valid": true})";
        std::string invalidJson = R"({"invalid": })";

        std::cout << "有效JSON检测: " << (JsonParser::isValidJsonString(testJson) ? "有效" : "无效") << std::endl;
        std::cout << "无效JSON检测: " << (JsonParser::isValidJsonString(invalidJson) ? "有效" : "无效") << std::endl;

        printSeparator("示例完成！");
        std::cout << "✓ 所有功能演示完成，JsonParser类工作正常！" << std::endl;

    } catch (const JsonParseException& e) {
        std::cerr << "JSON解析错误: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}