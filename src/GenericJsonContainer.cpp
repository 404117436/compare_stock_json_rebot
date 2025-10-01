#include "GenericJsonContainer.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <set>

// ==================== CustomValue 实现 ====================

// 默认构造函数（null）
CustomValue::CustomValue() : type_(JsonValueType::Null) {
    data_.intValue = 0;
}

// 各类型构造函数
CustomValue::CustomValue(bool value) : type_(JsonValueType::Bool) {
    data_.boolValue = value;
}

CustomValue::CustomValue(int value) : type_(JsonValueType::Int) {
    data_.intValue = static_cast<int64_t>(value);
}

CustomValue::CustomValue(int64_t value) : type_(JsonValueType::Int) {
    data_.intValue = value;
}

CustomValue::CustomValue(double value) : type_(JsonValueType::Double) {
    data_.doubleValue = value;
}

CustomValue::CustomValue(const std::string& value) : type_(JsonValueType::String) {
    data_.stringValue = new std::string(value);
}

CustomValue::CustomValue(const char* value) : type_(JsonValueType::String) {
    data_.stringValue = new std::string(value);
}

CustomValue::CustomValue(const JsonArrayData& array) : type_(JsonValueType::Array) {
    data_.arrayValue = new JsonArrayData(array);
}

CustomValue::CustomValue(const JsonObjectData& object) : type_(JsonValueType::Object) {
    data_.objectValue = new JsonObjectData(object);
}

// 拷贝构造函数
CustomValue::CustomValue(const CustomValue& other) : type_(other.type_) {
    copyFrom(other);
}

// 拷贝赋值操作符
CustomValue& CustomValue::operator=(const CustomValue& other) {
    if (this != &other) {
        cleanup();
        type_ = other.type_;
        copyFrom(other);
    }
    return *this;
}

// 移动构造函数
CustomValue::CustomValue(CustomValue&& other) : type_(other.type_) {
    data_ = other.data_;
    other.type_ = JsonValueType::Null;
    other.data_.intValue = 0;
}

// 移动赋值操作符
CustomValue& CustomValue::operator=(CustomValue&& other) {
    if (this != &other) {
        cleanup();
        type_ = other.type_;
        data_ = other.data_;
        other.type_ = JsonValueType::Null;
        other.data_.intValue = 0;
    }
    return *this;
}

// 析构函数
CustomValue::~CustomValue() {
    cleanup();
}

// 清理资源
void CustomValue::cleanup() {
    switch (type_) {
        case JsonValueType::String:
            delete data_.stringValue;
            break;
        case JsonValueType::Array:
            delete data_.arrayValue;
            break;
        case JsonValueType::Object:
            delete data_.objectValue;
            break;
        default:
            // 基本类型无需清理
            break;
    }
}

// 从其他对象拷贝数据
void CustomValue::copyFrom(const CustomValue& other) {
    switch (other.type_) {
        case JsonValueType::Null:
            data_.intValue = 0;
            break;
        case JsonValueType::Bool:
            data_.boolValue = other.data_.boolValue;
            break;
        case JsonValueType::Int:
            data_.intValue = other.data_.intValue;
            break;
        case JsonValueType::Double:
            data_.doubleValue = other.data_.doubleValue;
            break;
        case JsonValueType::String:
            data_.stringValue = new std::string(*other.data_.stringValue);
            break;
        case JsonValueType::Array:
            data_.arrayValue = new JsonArrayData(*other.data_.arrayValue);
            break;
        case JsonValueType::Object:
            data_.objectValue = new JsonObjectData(*other.data_.objectValue);
            break;
    }
}

// 类型安全的值获取方法
bool CustomValue::asBool() const {
    if (type_ != JsonValueType::Bool) {
        throw GenericJsonException("Value is not a boolean");
    }
    return data_.boolValue;
}

int64_t CustomValue::asInt() const {
    if (type_ != JsonValueType::Int) {
        throw GenericJsonException("Value is not an integer");
    }
    return data_.intValue;
}

double CustomValue::asDouble() const {
    if (type_ == JsonValueType::Double) {
        return data_.doubleValue;
    } else if (type_ == JsonValueType::Int) {
        return static_cast<double>(data_.intValue);
    } else {
        throw GenericJsonException("Value is not a number");
    }
}

const std::string& CustomValue::asString() const {
    if (type_ != JsonValueType::String) {
        throw GenericJsonException("Value is not a string");
    }
    return *data_.stringValue;
}

const JsonArrayData& CustomValue::asArray() const {
    if (type_ != JsonValueType::Array) {
        throw GenericJsonException("Value is not an array");
    }
    return *data_.arrayValue;
}

const JsonObjectData& CustomValue::asObject() const {
    if (type_ != JsonValueType::Object) {
        throw GenericJsonException("Value is not an object");
    }
    return *data_.objectValue;
}

// 可修改的引用获取
JsonArrayData& CustomValue::getArrayRef() {
    if (type_ != JsonValueType::Array) {
        throw GenericJsonException("Value is not an array");
    }
    return *data_.arrayValue;
}

JsonObjectData& CustomValue::getObjectRef() {
    if (type_ != JsonValueType::Object) {
        throw GenericJsonException("Value is not an object");
    }
    return *data_.objectValue;
}

// 数组操作
size_t CustomValue::arraySize() const {
    if (type_ != JsonValueType::Array) {
        return 0;
    }
    return data_.arrayValue->size();
}

void CustomValue::arrayPush(const CustomValue& value) {
    if (type_ != JsonValueType::Array) {
        throw GenericJsonException("Value is not an array");
    }
    data_.arrayValue->push_back(value);
}

CustomValue& CustomValue::arrayAt(size_t index) {
    if (type_ != JsonValueType::Array) {
        throw GenericJsonException("Value is not an array");
    }
    if (index >= data_.arrayValue->size()) {
        throw GenericJsonException("Array index out of bounds");
    }
    return (*data_.arrayValue)[index];
}

const CustomValue& CustomValue::arrayAt(size_t index) const {
    if (type_ != JsonValueType::Array) {
        throw GenericJsonException("Value is not an array");
    }
    if (index >= data_.arrayValue->size()) {
        throw GenericJsonException("Array index out of bounds");
    }
    return (*data_.arrayValue)[index];
}

// 对象操作
size_t CustomValue::objectSize() const {
    if (type_ != JsonValueType::Object) {
        return 0;
    }
    return data_.objectValue->size();
}

bool CustomValue::hasKey(const std::string& key) const {
    if (type_ != JsonValueType::Object) {
        return false;
    }

    for (const auto& pair : *data_.objectValue) {
        if (pair.first == key) {
            return true;
        }
    }
    return false;
}

void CustomValue::setKey(const std::string& key, const CustomValue& value) {
    if (type_ != JsonValueType::Object) {
        throw GenericJsonException("Value is not an object");
    }

    // 查找是否已存在
    for (auto& pair : *data_.objectValue) {
        if (pair.first == key) {
            pair.second = value;
            return;
        }
    }

    // 不存在则添加
    data_.objectValue->push_back(std::make_pair(key, value));
}

CustomValue& CustomValue::getKey(const std::string& key) {
    if (type_ != JsonValueType::Object) {
        throw GenericJsonException("Value is not an object");
    }

    for (auto& pair : *data_.objectValue) {
        if (pair.first == key) {
            return pair.second;
        }
    }

    throw GenericJsonException("Key '" + key + "' not found");
}

const CustomValue& CustomValue::getKey(const std::string& key) const {
    if (type_ != JsonValueType::Object) {
        throw GenericJsonException("Value is not an object");
    }

    for (const auto& pair : *data_.objectValue) {
        if (pair.first == key) {
            return pair.second;
        }
    }

    throw GenericJsonException("Key '" + key + "' not found");
}

std::vector<std::string> CustomValue::getKeys() const {
    if (type_ != JsonValueType::Object) {
        throw GenericJsonException("Value is not an object");
    }

    std::vector<std::string> keys;
    for (const auto& pair : *data_.objectValue) {
        keys.push_back(pair.first);
    }
    return keys;
}

// 操作符重载
CustomValue& CustomValue::operator[](const std::string& key) {
    return getKey(key);
}

const CustomValue& CustomValue::operator[](const std::string& key) const {
    return getKey(key);
}

CustomValue& CustomValue::operator[](size_t index) {
    return arrayAt(index);
}

const CustomValue& CustomValue::operator[](size_t index) const {
    return arrayAt(index);
}

// 转换为字符串
std::string CustomValue::toString() const {
    switch (type_) {
        case JsonValueType::Null:
            return "null";
        case JsonValueType::Bool:
            return data_.boolValue ? "true" : "false";
        case JsonValueType::Int:
            return std::to_string(data_.intValue);
        case JsonValueType::Double:
            return std::to_string(data_.doubleValue);
        case JsonValueType::String:
            return *data_.stringValue;
        case JsonValueType::Array:
            return "[Array(" + std::to_string(data_.arrayValue->size()) + ")]";
        case JsonValueType::Object:
            return "{Object(" + std::to_string(data_.objectValue->size()) + ")}";
        default:
            return "Unknown";
    }
}

std::string CustomValue::toJsonString() const {
    std::stringstream ss;

    switch (type_) {
        case JsonValueType::Null:
            ss << "null";
            break;
        case JsonValueType::Bool:
            ss << (data_.boolValue ? "true" : "false");
            break;
        case JsonValueType::Int:
            ss << data_.intValue;
            break;
        case JsonValueType::Double:
            ss << std::fixed << std::setprecision(4) << data_.doubleValue;
            break;
        case JsonValueType::String:
            ss << "\"" << *data_.stringValue << "\"";
            break;
        case JsonValueType::Array: {
            ss << "[";
            for (size_t i = 0; i < data_.arrayValue->size(); ++i) {
                if (i > 0) ss << ",";
                ss << (*data_.arrayValue)[i].toJsonString();
            }
            ss << "]";
            break;
        }
        case JsonValueType::Object: {
            ss << "{";
            for (size_t i = 0; i < data_.objectValue->size(); ++i) {
                if (i > 0) ss << ",";
                const auto& pair = (*data_.objectValue)[i];
                ss << "\"" << pair.first << "\":" << pair.second.toJsonString();
            }
            ss << "}";
            break;
        }
    }

    return ss.str();
}

void CustomValue::print(int indent) const {
    std::string indentStr(indent * 2, ' ');

    switch (type_) {
        case JsonValueType::Null:
            std::cout << "null";
            break;
        case JsonValueType::Bool:
            std::cout << (data_.boolValue ? "true" : "false");
            break;
        case JsonValueType::Int:
            std::cout << data_.intValue;
            break;
        case JsonValueType::Double:
            std::cout << std::fixed << std::setprecision(4) << data_.doubleValue;
            break;
        case JsonValueType::String:
            std::cout << "\"" << *data_.stringValue << "\"";
            break;
        case JsonValueType::Array:
            std::cout << "[\n";
            for (size_t i = 0; i < data_.arrayValue->size(); ++i) {
                std::cout << indentStr << "  ";
                (*data_.arrayValue)[i].print(indent + 1);
                if (i < data_.arrayValue->size() - 1) std::cout << ",";
                std::cout << "\n";
            }
            std::cout << indentStr << "]";
            break;
        case JsonValueType::Object:
            std::cout << "{\n";
            for (size_t i = 0; i < data_.objectValue->size(); ++i) {
                const auto& pair = (*data_.objectValue)[i];
                std::cout << indentStr << "  \"" << pair.first << "\": ";
                pair.second.print(indent + 1);
                if (i < data_.objectValue->size() - 1) std::cout << ",";
                std::cout << "\n";
            }
            std::cout << indentStr << "}";
            break;
    }
}

// 静态创建方法
CustomValue CustomValue::createNull() {
    return CustomValue();
}

CustomValue CustomValue::createObject() {
    return CustomValue(JsonObjectData());
}

CustomValue CustomValue::createArray() {
    return CustomValue(JsonArrayData());
}

// ==================== GenericJsonContainer 实现 ====================

GenericJsonContainer::GenericJsonContainer() : source_("Unknown") {
}

GenericJsonContainer::GenericJsonContainer(const std::string& source) : source_(source) {
}

bool GenericJsonContainer::parseFromJsonString(const std::string& jsonStr) {
    try {
        JsonParser parser(jsonStr);
        return parseFromJsonParser(parser);
    } catch (const JsonParseException& e) {
        std::cerr << "JSON解析错误: " << e.what() << std::endl;
        return false;
    }
}

bool GenericJsonContainer::parseFromJsonParser(const JsonParser& parser) {
    try {
        data_map_.clear();
        auto root = parser.getRoot();

        if (root.isObject()) {
            auto keys = root.getKeys();
            for (const auto& key : keys) {
                auto value = root[key];
                data_map_.emplace(key, parseJsonValue(value));  // O(1) 插入
            }
            return true;
        } else {
            std::cerr << "根节点不是JSON对象" << std::endl;
            return false;
        }
    } catch (const JsonParseException& e) {
        std::cerr << "解析错误: " << e.what() << std::endl;
        return false;
    } catch (const GenericJsonException& e) {
        std::cerr << "通用JSON错误: " << e.what() << std::endl;
        return false;
    }
}

// 从JSON字符串解析（支持忽略指定字段）
bool GenericJsonContainer::parseFromJsonString(const std::string& jsonStr, const std::string& ignore_fields) {
    try {
        JsonParser parser(jsonStr);
        return parseFromJsonParser(parser, ignore_fields);
    } catch (const JsonParseException& e) {
        std::cerr << "JSON解析错误: " << e.what() << std::endl;
        return false;
    }
}

// 从JsonParser解析（支持忽略指定字段）
bool GenericJsonContainer::parseFromJsonParser(const JsonParser& parser, const std::string& ignore_fields) {
    try {
        data_map_.clear();
        auto root = parser.getRoot();

        if (root.isObject()) {
            // 解析忽略字段集合
            std::set<std::string> ignoreFieldsSet = parseIgnoreFields(ignore_fields);
            auto keys = root.getKeys();
            for (const auto& key : keys) {
                // 跳过需要忽略的字段
                if (ignoreFieldsSet.count(key) > 0) {
                    continue;
                }
                auto value = root[key];
                data_map_.emplace(key, parseJsonValue(value, ignoreFieldsSet));  // O(1) 插入
            }
            return true;
        } else {
            std::cerr << "根节点不是JSON对象" << std::endl;
            return false;
        }
    } catch (const JsonParseException& e) {
        std::cerr << "解析错误: " << e.what() << std::endl;
        return false;
    } catch (const GenericJsonException& e) {
        std::cerr << "通用JSON错误: " << e.what() << std::endl;
        return false;
    }
}

// 解析JSON值
CustomValue GenericJsonContainer::parseJsonValue(const JsonValue& jsonValue) {
    if (jsonValue.isNull()) {
        return CustomValue::createNull();
    } else if (jsonValue.isBool()) {
        return CustomValue(jsonValue.asBool());
    } else if (jsonValue.isInt()) {
        return CustomValue(static_cast<int64_t>(jsonValue.asInt()));
    } else if (jsonValue.isDouble()) {
        return CustomValue(jsonValue.asDouble());
    } else if (jsonValue.isString()) {
        return CustomValue(jsonValue.asString());
    } else if (jsonValue.isArray()) {
        JsonArrayData arrayData;
        parseJsonArray(jsonValue, arrayData);
        return CustomValue(arrayData);
    } else if (jsonValue.isObject()) {
        JsonObjectData objectData;
        parseJsonObject(jsonValue, objectData);
        return CustomValue(objectData);
    } else {
        return CustomValue::createNull();
    }
}

// 解析JSON对象
void GenericJsonContainer::parseJsonObject(const JsonValue& jsonObj, JsonObjectData& data) {
    if (!jsonObj.isObject()) {
        throw GenericJsonException("值不是JSON对象");
    }

    auto keys = jsonObj.getKeys();
    for (const auto& key : keys) {
        auto value = jsonObj[key];
        CustomValue customValue = parseJsonValue(value);
        data.push_back(std::make_pair(key, customValue));
    }
}

// 解析JSON数组
void GenericJsonContainer::parseJsonArray(const JsonValue& jsonArray, JsonArrayData& data) {
    if (!jsonArray.isArray()) {
        throw GenericJsonException("值不是JSON数组");
    }

    for (size_t i = 0; i < jsonArray.arraySize(); ++i) {
        auto element = jsonArray[i];
        CustomValue customValue = parseJsonValue(element);
        data.push_back(customValue);
    }
}

// 键值对操作
void GenericJsonContainer::setValue(const std::string& key, const CustomValue& value) {
    data_map_[key] = value;  // O(1) 插入或更新
}

bool GenericJsonContainer::hasKey(const std::string& key) const {
    return data_map_.count(key) > 0;  // O(1) 查找
}

const CustomValue& GenericJsonContainer::getValue(const std::string& key) const {
    auto it = data_map_.find(key);  // O(1) 查找
    if (it != data_map_.end()) {
        return it->second;
    }
    throw GenericJsonException("键 '" + key + "' 不存在");
}

CustomValue& GenericJsonContainer::getValue(const std::string& key) {
    auto it = data_map_.find(key);  // O(1) 查找
    if (it != data_map_.end()) {
        return it->second;
    }
    throw GenericJsonException("键 '" + key + "' 不存在");
}

std::vector<std::string> GenericJsonContainer::getAllKeys() const {
    std::vector<std::string> keys;
    keys.reserve(data_map_.size());
    for (const auto& pair : data_map_) {
        keys.push_back(pair.first);
    }
    return keys;
}

// 操作符重载
CustomValue& GenericJsonContainer::operator[](const std::string& key) {
    return data_map_[key];  // O(1) 访问，不存在时自动创建默认值
}

const CustomValue& GenericJsonContainer::operator[](const std::string& key) const {
    return getValue(key);
}

// 查找操作
std::unordered_map<std::string, CustomValue>::iterator GenericJsonContainer::find(const std::string& key) {
    return data_map_.find(key);  // O(1) 查找
}

std::unordered_map<std::string, CustomValue>::const_iterator GenericJsonContainer::find(const std::string& key) const {
    return data_map_.find(key);  // O(1) 查找
}

// 删除操作
bool GenericJsonContainer::removeKey(const std::string& key) {
    return data_map_.erase(key) > 0;  // O(1) 删除，返回删除的元素数量
}

// 输出方法
void GenericJsonContainer::print() const {
    std::cout << "=== GenericJsonContainer (" << source_ << ") ===" << std::endl;
    std::cout << "键值对数量: " << data_map_.size() << std::endl;

    for (const auto& pair : data_map_) {
        std::cout << "\"" << pair.first << "\": ";
        std::cout << pair.second.toString() << std::endl;
    }
}

void GenericJsonContainer::printDetailed() const {
    std::cout << "=== 详细信息 (" << source_ << ") ===" << std::endl;
    std::cout << "键值对数量: " << data_map_.size() << std::endl;

    for (const auto& pair : data_map_) {
        std::cout << "\n键: \"" << pair.first << "\"" << std::endl;
        std::cout << "类型: " << jsonValueTypeToString(pair.second.getType()) << std::endl;
        std::cout << "值: ";
        pair.second.print(0);
        std::cout << std::endl;
    }
}

void GenericJsonContainer::printKeyValuePairs() const {
    std::cout << "=== 键值对列表 (" << source_ << ") ===" << std::endl;

    size_t i = 0;
    for (const auto& pair : data_map_) {
        std::cout << "[" << i << "] \"" << pair.first << "\" => "
                  << jsonValueTypeToString(pair.second.getType())
                  << " (" << pair.second.toString() << ")" << std::endl;
        ++i;
    }
}

std::string GenericJsonContainer::toString() const {
    std::stringstream ss;
    ss << "GenericJsonContainer(" << source_ << "): " << data_map_.size() << " pairs\n";

    for (const auto& pair : data_map_) {
        ss << "  \"" << pair.first << "\": " << pair.second.toString() << "\n";
    }

    return ss.str();
}

std::string GenericJsonContainer::toJsonString() const {
    std::stringstream ss;
    ss << "{";

    size_t i = 0;
    for (const auto& pair : data_map_) {
        if (i > 0) ss << ",";
        ss << "\"" << pair.first << "\":" << pair.second.toJsonString();
        ++i;
    }

    ss << "}";
    return ss.str();
}

// 统计信息
void GenericJsonContainer::printStatistics() const {
    std::cout << "\n=== 统计信息 (" << source_ << ") ===" << std::endl;
    std::cout << "总键值对数: " << data_map_.size() << std::endl;

    std::cout << "类型分布:" << std::endl;
    std::cout << "  Null: " << countByType(JsonValueType::Null) << std::endl;
    std::cout << "  Bool: " << countByType(JsonValueType::Bool) << std::endl;
    std::cout << "  Int: " << countByType(JsonValueType::Int) << std::endl;
    std::cout << "  Double: " << countByType(JsonValueType::Double) << std::endl;
    std::cout << "  String: " << countByType(JsonValueType::String) << std::endl;
    std::cout << "  Array: " << countByType(JsonValueType::Array) << std::endl;
    std::cout << "  Object: " << countByType(JsonValueType::Object) << std::endl;
}

size_t GenericJsonContainer::countByType(JsonValueType type) const {
    size_t count = 0;
    for (const auto& pair : data_map_) {
        if (pair.second.getType() == type) {
            count++;
        }
    }
    return count;
}

// 导出功能
bool GenericJsonContainer::exportToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法创建文件: " << filename << std::endl;
        return false;
    }

    file << toString();
    file.close();

    std::cout << "成功导出到文件: " << filename << std::endl;
    return true;
}

bool GenericJsonContainer::exportToJson(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法创建JSON文件: " << filename << std::endl;
        return false;
    }

    file << toJsonString();
    file.close();

    std::cout << "成功导出JSON到: " << filename << std::endl;
    return true;
}

// 批量操作
void GenericJsonContainer::merge(const GenericJsonContainer& other) {
    for (const auto& pair : other.data_map_) {
        setValue(pair.first, pair.second);
    }
}

GenericJsonContainer GenericJsonContainer::filter(const std::vector<std::string>& keys) const {
    GenericJsonContainer filtered(source_ + "_filtered");

    for (const auto& key : keys) {
        if (hasKey(key)) {
            filtered.setValue(key, getValue(key));
        }
    }

    return filtered;
}

// ==================== 辅助函数实现 ====================

std::string jsonValueTypeToString(JsonValueType type) {
    switch (type) {
        case JsonValueType::Null: return "Null";
        case JsonValueType::Bool: return "Bool";
        case JsonValueType::Int: return "Int";
        case JsonValueType::Double: return "Double";
        case JsonValueType::String: return "String";
        case JsonValueType::Array: return "Array";
        case JsonValueType::Object: return "Object";
        default: return "Unknown";
    }
}

void printJsonValueType(JsonValueType type) {
    std::cout << jsonValueTypeToString(type);
}


// ==================== 字段忽略功能实现 ====================

// 解析忽略字段字符串，将"key1,key2,key3"格式转换为set<string>
std::set<std::string> GenericJsonContainer::parseIgnoreFields(const std::string& ignore_fields) const {
    std::set<std::string> result;

    if (ignore_fields.empty()) {
        return result;  // 空字符串返回空集合
    }

    std::stringstream ss(ignore_fields);
    std::string field;

    // 使用逗号分割字符串
    while (std::getline(ss, field, ',')) {
        // 移除字段名两端的空白字符
        size_t start = field.find_first_not_of(" \t\r\n");
        size_t end = field.find_last_not_of(" \t\r\n");

        if (start != std::string::npos && end != std::string::npos) {
            std::string trimmedField = field.substr(start, end - start + 1);
            if (!trimmedField.empty()) {
                result.insert(trimmedField);
            }
        }
    }

    return result;
}

// 支持忽略字段的JSON值解析
CustomValue GenericJsonContainer::parseJsonValue(const JsonValue& jsonValue, const std::set<std::string>& ignoreFields) {
    if (jsonValue.isNull()) {
        return CustomValue::createNull();
    } else if (jsonValue.isBool()) {
        return CustomValue(jsonValue.asBool());
    } else if (jsonValue.isInt()) {
        return CustomValue(static_cast<int64_t>(jsonValue.asInt()));
    } else if (jsonValue.isDouble()) {
        return CustomValue(jsonValue.asDouble());
    } else if (jsonValue.isString()) {
        return CustomValue(jsonValue.asString());
    } else if (jsonValue.isArray()) {
        JsonArrayData arrayData;
        parseJsonArray(jsonValue, arrayData, ignoreFields);
        return CustomValue(arrayData);
    } else if (jsonValue.isObject()) {
        JsonObjectData objectData;
        parseJsonObject(jsonValue, objectData, ignoreFields);
        return CustomValue(objectData);
    } else {
        return CustomValue::createNull();
    }
}

// 支持忽略字段的JSON对象解析
void GenericJsonContainer::parseJsonObject(const JsonValue& jsonObj, JsonObjectData& data, const std::set<std::string>& ignoreFields) {
    if (!jsonObj.isObject()) {
        throw GenericJsonException("值不是JSON对象");
    }

    auto keys = jsonObj.getKeys();
    for (const auto& key : keys) {
        // 检查键是否在忽略列表中
        if (ignoreFields.find(key) != ignoreFields.end()) {
            continue;  // 跳过这个字段
        }

        auto value = jsonObj[key];
        CustomValue customValue = parseJsonValue(value, ignoreFields);
        data.push_back(std::make_pair(key, customValue));
    }
}

// 支持忽略字段的JSON数组解析
void GenericJsonContainer::parseJsonArray(const JsonValue& jsonArray, JsonArrayData& data, const std::set<std::string>& ignoreFields) {
    if (!jsonArray.isArray()) {
        throw GenericJsonException("值不是JSON数组");
    }

    for (size_t i = 0; i < jsonArray.arraySize(); ++i) {
        auto element = jsonArray[i];
        CustomValue customValue = parseJsonValue(element, ignoreFields);
        data.push_back(customValue);
    }
}