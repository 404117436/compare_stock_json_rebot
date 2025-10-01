#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <set>
#include "JsonParser.h"

// JSON值类型枚举
enum class JsonValueType {
    Null,
    Bool,
    Int,
    Double,
    String,
    Array,
    Object
};

// 前向声明
class CustomValue;
class GenericJsonContainer;

// 类型别名定义
using JsonKeyValuePair = std::pair<std::string, CustomValue>;
using JsonObjectData = std::vector<JsonKeyValuePair>;
using JsonArrayData = std::vector<CustomValue>;

// 通用JSON值类，支持所有JSON数据类型
class CustomValue {
private:
    JsonValueType type_;

    // 使用联合体存储不同类型的数据
    union ValueData {
        bool boolValue;
        int64_t intValue;
        double doubleValue;
        std::string* stringValue;
        JsonArrayData* arrayValue;
        JsonObjectData* objectValue;

        ValueData() : intValue(0) {}
        ~ValueData() {} // 由外部管理析构
    } data_;

    // 内部辅助方法
    void cleanup();
    void copyFrom(const CustomValue& other);

public:
    // 构造函数
    CustomValue();                              // 默认为null
    explicit CustomValue(bool value);           // bool构造
    explicit CustomValue(int value);            // int构造
    explicit CustomValue(int64_t value);        // int64构造
    explicit CustomValue(double value);         // double构造
    explicit CustomValue(const std::string& value);  // string构造
    explicit CustomValue(const char* value);    // C字符串构造
    explicit CustomValue(const JsonArrayData& array);   // 数组构造
    explicit CustomValue(const JsonObjectData& object); // 对象构造

    // 拷贝构造和赋值
    CustomValue(const CustomValue& other);
    CustomValue& operator=(const CustomValue& other);

    // 移动构造和赋值（C++11）
    CustomValue(CustomValue&& other);
    CustomValue& operator=(CustomValue&& other);

    // 析构函数
    ~CustomValue();

    // 类型检查方法
    JsonValueType getType() const { return type_; }
    bool isNull() const { return type_ == JsonValueType::Null; }
    bool isBool() const { return type_ == JsonValueType::Bool; }
    bool isInt() const { return type_ == JsonValueType::Int; }
    bool isDouble() const { return type_ == JsonValueType::Double; }
    bool isString() const { return type_ == JsonValueType::String; }
    bool isArray() const { return type_ == JsonValueType::Array; }
    bool isObject() const { return type_ == JsonValueType::Object; }
    bool isNumber() const { return isInt() || isDouble(); }

    // 类型安全的值获取方法
    bool asBool() const;
    int64_t asInt() const;
    double asDouble() const;
    const std::string& asString() const;
    const JsonArrayData& asArray() const;
    const JsonObjectData& asObject() const;

    // 可修改的引用获取（用于构建数据）
    JsonArrayData& getArrayRef();
    JsonObjectData& getObjectRef();

    // 数组操作
    size_t arraySize() const;
    void arrayPush(const CustomValue& value);
    CustomValue& arrayAt(size_t index);
    const CustomValue& arrayAt(size_t index) const;

    // 对象操作
    size_t objectSize() const;
    bool hasKey(const std::string& key) const;
    void setKey(const std::string& key, const CustomValue& value);
    CustomValue& getKey(const std::string& key);
    const CustomValue& getKey(const std::string& key) const;
    std::vector<std::string> getKeys() const;

    // 操作符重载
    CustomValue& operator[](const std::string& key);        // 对象访问
    const CustomValue& operator[](const std::string& key) const;
    CustomValue& operator[](size_t index);                  // 数组访问
    const CustomValue& operator[](size_t index) const;

    // 输出方法
    std::string toString() const;
    std::string toJsonString() const;
    void print(int indent = 0) const;

    // 静态创建方法
    static CustomValue createNull();
    static CustomValue createObject();
    static CustomValue createArray();
};

// 通用JSON容器类，使用unordered_map<string, CustomValue>存储
class GenericJsonContainer {
private:
    std::unordered_map<std::string, CustomValue> data_map_;  // 主要存储：哈希表实现O(1)查找
    std::string source_;                                     // 数据源信息

public:
    // 构造函数
    GenericJsonContainer();
    explicit GenericJsonContainer(const std::string& source);

    // 虚析构函数（因为有virtual方法）
    virtual ~GenericJsonContainer() = default;

    // 从JSON字符串解析
    virtual bool parseFromJsonString(const std::string& jsonStr);

    // 从JSON字符串解析（支持忽略指定字段）
    virtual bool parseFromJsonString(const std::string& jsonStr, const std::string& ignore_fields);

    // 从JsonParser解析
    bool parseFromJsonParser(const JsonParser& parser);

    // 从JsonParser解析（支持忽略指定字段）
    bool parseFromJsonParser(const JsonParser& parser, const std::string& ignore_fields);

    // 数据访问方法
    size_t size() const { return data_map_.size(); }
    bool empty() const { return data_map_.empty(); }
    void clear() { data_map_.clear(); }

    // 键值对操作
    void setValue(const std::string& key, const CustomValue& value);
    bool hasKey(const std::string& key) const;
    const CustomValue& getValue(const std::string& key) const;
    CustomValue& getValue(const std::string& key);

    // 获取所有键
    std::vector<std::string> getAllKeys() const;

    // 迭代器支持
    std::unordered_map<std::string, CustomValue>::iterator begin() { return data_map_.begin(); }
    std::unordered_map<std::string, CustomValue>::iterator end() { return data_map_.end(); }
    std::unordered_map<std::string, CustomValue>::const_iterator begin() const { return data_map_.begin(); }
    std::unordered_map<std::string, CustomValue>::const_iterator end() const { return data_map_.end(); }
    std::unordered_map<std::string, CustomValue>::const_iterator cbegin() const { return data_map_.cbegin(); }
    std::unordered_map<std::string, CustomValue>::const_iterator cend() const { return data_map_.cend(); }

    // 操作符重载
    CustomValue& operator[](const std::string& key);
    const CustomValue& operator[](const std::string& key) const;

    // 查找操作
    std::unordered_map<std::string, CustomValue>::iterator find(const std::string& key);
    std::unordered_map<std::string, CustomValue>::const_iterator find(const std::string& key) const;

    // 删除操作
    bool removeKey(const std::string& key);

    // 输出方法
    void print() const;
    void printDetailed() const;
    void printKeyValuePairs() const;
    std::string toString() const;
    std::string toJsonString() const;

    // 统计信息
    void printStatistics() const;
    size_t countByType(JsonValueType type) const;

    // 数据源管理
    const std::string& getSource() const { return source_; }
    void setSource(const std::string& source) { source_ = source; }

    // 导出功能
    bool exportToFile(const std::string& filename) const;
    bool exportToJson(const std::string& filename) const;

    // 批量操作支持
    void merge(const GenericJsonContainer& other);
    GenericJsonContainer filter(const std::vector<std::string>& keys) const;

private:
    // 内部辅助方法
    CustomValue parseJsonValue(const JsonValue& jsonValue);
    CustomValue parseJsonValue(const JsonValue& jsonValue, const std::unordered_set<std::string>& ignoreFields);
    void parseJsonObject(const JsonValue& jsonObj, JsonObjectData& data);
    void parseJsonObject(const JsonValue& jsonObj, JsonObjectData& data, const std::unordered_set<std::string>& ignoreFields);
    void parseJsonArray(const JsonValue& jsonArray, JsonArrayData& data);
    void parseJsonArray(const JsonValue& jsonArray, JsonArrayData& data, const std::unordered_set<std::string>& ignoreFields);

    // 字段忽略相关辅助方法（使用 unordered_set 提升查找性能）
    std::unordered_set<std::string> parseIgnoreFields(const std::string& ignore_fields) const;
};

// 异常类
class GenericJsonException : public std::runtime_error {
public:
    explicit GenericJsonException(const std::string& message)
        : std::runtime_error("GenericJson Error: " + message) {}
};

// 辅助函数
std::string jsonValueTypeToString(JsonValueType type);
void printJsonValueType(JsonValueType type);

