#pragma once

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <iomanip>
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

// 通用JSON容器类，使用vector<pair<string, CustomValue>>存储
class GenericJsonContainer {
private:
    JsonObjectData data_;               // 主要存储：vector<pair<string, CustomValue>>
    std::string source_;                // 数据源信息

public:
    // 构造函数
    GenericJsonContainer();
    explicit GenericJsonContainer(const std::string& source);

    // 从JSON字符串解析
    bool parseFromJsonString(const std::string& jsonStr);

    // 从JsonParser解析
    bool parseFromJsonParser(const JsonParser& parser);

    // 数据访问方法
    size_t size() const { return data_.size(); }
    bool empty() const { return data_.empty(); }
    void clear() { data_.clear(); }

    // 键值对操作
    void setValue(const std::string& key, const CustomValue& value);
    bool hasKey(const std::string& key) const;
    const CustomValue& getValue(const std::string& key) const;
    CustomValue& getValue(const std::string& key);

    // 获取所有键
    std::vector<std::string> getAllKeys() const;

    // 迭代器支持
    JsonObjectData::iterator begin() { return data_.begin(); }
    JsonObjectData::iterator end() { return data_.end(); }
    JsonObjectData::const_iterator begin() const { return data_.begin(); }
    JsonObjectData::const_iterator end() const { return data_.end(); }
    JsonObjectData::const_iterator cbegin() const { return data_.cbegin(); }
    JsonObjectData::const_iterator cend() const { return data_.cend(); }

    // 操作符重载
    CustomValue& operator[](const std::string& key);
    const CustomValue& operator[](const std::string& key) const;

    // 查找操作
    JsonObjectData::iterator find(const std::string& key);
    JsonObjectData::const_iterator find(const std::string& key) const;

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
    void parseJsonObject(const JsonValue& jsonObj, JsonObjectData& data);
    void parseJsonArray(const JsonValue& jsonArray, JsonArrayData& data);

    // 查找键的内部方法
    JsonObjectData::iterator findKey(const std::string& key);
    JsonObjectData::const_iterator findKey(const std::string& key) const;
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

// 批量容器管理类
class JsonContainerCollection {
private:
    std::vector<GenericJsonContainer> containers_;
    std::string collectionName_;

public:
    explicit JsonContainerCollection(const std::string& name = "DefaultCollection");

    // 添加容器
    void addContainer(const GenericJsonContainer& container);
    void addContainer(GenericJsonContainer&& container);

    // 从JSON数组字符串批量创建
    bool addFromJsonArray(const std::string& jsonArrayStr);

    // 访问方法
    size_t size() const { return containers_.size(); }
    bool empty() const { return containers_.empty(); }
    void clear() { containers_.clear(); }

    GenericJsonContainer& operator[](size_t index) { return containers_[index]; }
    const GenericJsonContainer& operator[](size_t index) const { return containers_[index]; }

    // 迭代器支持
    std::vector<GenericJsonContainer>::iterator begin() { return containers_.begin(); }
    std::vector<GenericJsonContainer>::iterator end() { return containers_.end(); }
    std::vector<GenericJsonContainer>::const_iterator begin() const { return containers_.begin(); }
    std::vector<GenericJsonContainer>::const_iterator end() const { return containers_.end(); }

    // 查找和过滤
    std::vector<GenericJsonContainer*> findByKey(const std::string& key);
    std::vector<GenericJsonContainer*> findByKeyValue(const std::string& key, const std::string& value);

    // 统计和输出
    void printSummary() const;
    void printAll() const;

    // 批量导出
    bool exportAllToJson(const std::string& filename) const;
};