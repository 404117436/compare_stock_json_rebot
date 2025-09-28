#pragma once

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <fstream>
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"

class JsonParseException : public std::runtime_error {
public:
    explicit JsonParseException(const std::string& message)
        : std::runtime_error("JsonParser Error: " + message) {}
};

class JsonValue {
private:
    const rapidjson::Value* value_;

public:
    explicit JsonValue(const rapidjson::Value* value) : value_(value) {}

    // 类型检查
    bool isString() const { return value_ && value_->IsString(); }
    bool isInt() const { return value_ && value_->IsInt(); }
    bool isDouble() const { return value_ && value_->IsDouble(); }
    bool isBool() const { return value_ && value_->IsBool(); }
    bool isArray() const { return value_ && value_->IsArray(); }
    bool isObject() const { return value_ && value_->IsObject(); }
    bool isNull() const { return !value_ || value_->IsNull(); }

    // 类型安全的获取方法
    std::string asString() const {
        if (!isString()) {
            throw JsonParseException("Value is not a string");
        }
        return value_->GetString();
    }

    int asInt() const {
        if (!isInt()) {
            throw JsonParseException("Value is not an integer");
        }
        return value_->GetInt();
    }

    double asDouble() const {
        if (!isDouble() && !isInt()) {
            throw JsonParseException("Value is not a number");
        }
        return value_->GetDouble();
    }

    bool asBool() const {
        if (!isBool()) {
            throw JsonParseException("Value is not a boolean");
        }
        return value_->GetBool();
    }

    // 安全的访问操作符
    JsonValue operator[](const std::string& key) const {
        if (!isObject()) {
            throw JsonParseException("Value is not an object");
        }
        auto it = value_->FindMember(key.c_str());
        if (it == value_->MemberEnd()) {
            return JsonValue(nullptr);  // 返回null值
        }
        return JsonValue(&it->value);
    }

    JsonValue operator[](size_t index) const {
        if (!isArray()) {
            throw JsonParseException("Value is not an array");
        }
        if (index >= value_->Size()) {
            throw JsonParseException("Array index out of bounds");
        }
        return JsonValue(&(*value_)[index]);
    }

    // 数组大小
    size_t arraySize() const {
        if (!isArray()) {
            throw JsonParseException("Value is not an array");
        }
        return value_->Size();
    }

    // 对象成员数量
    size_t objectSize() const {
        if (!isObject()) {
            throw JsonParseException("Value is not an object");
        }
        return value_->MemberCount();
    }

    // 检查对象是否包含指定键
    bool hasKey(const std::string& key) const {
        if (!isObject()) {
            return false;
        }
        return value_->HasMember(key.c_str());
    }

    // 获取对象的所有键名
    std::vector<std::string> getKeys() const {
        if (!isObject()) {
            throw JsonParseException("Value is not an object");
        }
        std::vector<std::string> keys;
        for (auto it = value_->MemberBegin(); it != value_->MemberEnd(); ++it) {
            keys.push_back(it->name.GetString());
        }
        return keys;
    }

    // 支持C++11的range-based for循环（针对数组）
    class ArrayIterator {
    private:
        rapidjson::Value::ConstValueIterator it_;
    public:
        explicit ArrayIterator(rapidjson::Value::ConstValueIterator it) : it_(it) {}
        JsonValue operator*() const { return JsonValue(&(*it_)); }
        ArrayIterator& operator++() { ++it_; return *this; }
        bool operator!=(const ArrayIterator& other) const { return it_ != other.it_; }
    };

    ArrayIterator begin() const {
        if (!isArray()) {
            throw JsonParseException("Value is not an array");
        }
        return ArrayIterator(value_->Begin());
    }

    ArrayIterator end() const {
        if (!isArray()) {
            throw JsonParseException("Value is not an array");
        }
        return ArrayIterator(value_->End());
    }
};

class JsonParser {
private:
    std::unique_ptr<rapidjson::Document> doc_;

public:
    // 构造函数
    JsonParser() : doc_(std::unique_ptr<rapidjson::Document>(new rapidjson::Document())) {}

    // 从字符串解析JSON
    explicit JsonParser(const std::string& jsonString) : JsonParser() {
        parseString(jsonString);
    }

    // 从文件解析JSON
    static JsonParser fromFile(const std::string& filename) {
        JsonParser parser;
        parser.parseFile(filename);
        return parser;
    }

    // 解析JSON字符串
    void parseString(const std::string& jsonString) {
        doc_->Parse(jsonString.c_str());
        if (doc_->HasParseError()) {
            throw JsonParseException("Failed to parse JSON string: " +
                std::to_string(doc_->GetParseError()));
        }
    }

    // 从文件解析JSON
    void parseFile(const std::string& filename) {
        std::ifstream ifs(filename);
        if (!ifs.is_open()) {
            throw JsonParseException("Cannot open file: " + filename);
        }

        rapidjson::IStreamWrapper isw(ifs);
        doc_->ParseStream(isw);
        if (doc_->HasParseError()) {
            throw JsonParseException("Failed to parse JSON file: " + filename +
                " Error: " + std::to_string(doc_->GetParseError()));
        }
    }

    // 检查解析是否成功
    bool isValid() const {
        return doc_ && !doc_->HasParseError();
    }

    // 获取根值
    JsonValue getRoot() const {
        if (!isValid()) {
            throw JsonParseException("Document is not valid");
        }
        return JsonValue(doc_.get());
    }

    // 便捷访问方法
    JsonValue operator[](const std::string& key) const {
        return getRoot()[key];
    }

    JsonValue operator[](size_t index) const {
        return getRoot()[index];
    }

    // 类型安全的获取方法
    std::string getString(const std::string& key) const {
        return (*this)[key].asString();
    }

    int getInt(const std::string& key) const {
        return (*this)[key].asInt();
    }

    double getDouble(const std::string& key) const {
        return (*this)[key].asDouble();
    }

    bool getBool(const std::string& key) const {
        return (*this)[key].asBool();
    }

    JsonValue getArray(const std::string& key) const {
        auto value = (*this)[key];
        if (!value.isArray()) {
            throw JsonParseException("Key '" + key + "' is not an array");
        }
        return value;
    }

    JsonValue getObject(const std::string& key) const {
        auto value = (*this)[key];
        if (!value.isObject()) {
            throw JsonParseException("Key '" + key + "' is not an object");
        }
        return value;
    }

    // 检查是否包含指定键
    bool hasKey(const std::string& key) const {
        return getRoot().hasKey(key);
    }

    // 获取所有键名
    std::vector<std::string> getKeys() const {
        return getRoot().getKeys();
    }

    // 将JSON转换为字符串
    std::string toString() const {
        if (!isValid()) {
            throw JsonParseException("Document is not valid");
        }

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc_->Accept(writer);
        return buffer.GetString();
    }

    // 美化输出的JSON字符串
    std::string toPrettyString() const {
        if (!isValid()) {
            throw JsonParseException("Document is not valid");
        }

        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        doc_->Accept(writer);
        return buffer.GetString();
    }

    // 获取详细的解析错误信息
    std::string getDetailedParseError() const;

    // 验证JSON结构是否包含所需的键
    bool validateStructure(const std::vector<std::string>& requiredKeys) const;

    // 合并另一个JSON对象
    void merge(const JsonParser& other);

    // 静态方法：验证JSON字符串格式
    static bool isValidJsonString(const std::string& jsonString);
};