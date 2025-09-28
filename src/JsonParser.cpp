#include "JsonParser.h"
#include <fstream>
#include <iostream>

// JsonParser类的实现主要在头文件中
// 这里可以添加一些辅助函数或扩展功能

namespace {
    // 私有辅助函数：格式化解析错误信息
    std::string formatParseError(rapidjson::ParseErrorCode error, size_t offset) {
        switch (error) {
            case rapidjson::kParseErrorNone:
                return "No error";
            case rapidjson::kParseErrorDocumentEmpty:
                return "Document is empty";
            case rapidjson::kParseErrorDocumentRootNotSingular:
                return "Document root not singular";
            case rapidjson::kParseErrorValueInvalid:
                return "Invalid value at offset " + std::to_string(offset);
            case rapidjson::kParseErrorObjectMissName:
                return "Missing object name at offset " + std::to_string(offset);
            case rapidjson::kParseErrorObjectMissColon:
                return "Missing colon after object name at offset " + std::to_string(offset);
            case rapidjson::kParseErrorObjectMissCommaOrCurlyBracket:
                return "Missing comma or closing brace in object at offset " + std::to_string(offset);
            case rapidjson::kParseErrorArrayMissCommaOrSquareBracket:
                return "Missing comma or closing bracket in array at offset " + std::to_string(offset);
            case rapidjson::kParseErrorStringUnicodeEscapeInvalidHex:
                return "Invalid Unicode escape sequence at offset " + std::to_string(offset);
            case rapidjson::kParseErrorStringUnicodeSurrogateInvalid:
                return "Invalid Unicode surrogate at offset " + std::to_string(offset);
            case rapidjson::kParseErrorStringEscapeInvalid:
                return "Invalid escape sequence at offset " + std::to_string(offset);
            case rapidjson::kParseErrorStringMissQuotationMark:
                return "Missing quotation mark at offset " + std::to_string(offset);
            case rapidjson::kParseErrorStringInvalidEncoding:
                return "Invalid string encoding at offset " + std::to_string(offset);
            case rapidjson::kParseErrorNumberTooBig:
                return "Number too big at offset " + std::to_string(offset);
            case rapidjson::kParseErrorNumberMissFraction:
                return "Missing fraction part in number at offset " + std::to_string(offset);
            case rapidjson::kParseErrorNumberMissExponent:
                return "Missing exponent in number at offset " + std::to_string(offset);
            default:
                return "Unknown parse error at offset " + std::to_string(offset);
        }
    }
}

// 如果需要，可以在这里添加JsonParser类的额外方法实现
// 例如：更复杂的解析逻辑、自定义序列化等

// 示例：添加一个更详细的解析错误信息方法
std::string JsonParser::getDetailedParseError() const {
    if (!doc_->HasParseError()) {
        return "No parse error";
    }

    auto error = doc_->GetParseError();
    auto offset = doc_->GetErrorOffset();
    return formatParseError(error, offset);
}

// 示例：添加一个验证JSON结构的方法
bool JsonParser::validateStructure(const std::vector<std::string>& requiredKeys) const {
    if (!isValid()) {
        return false;
    }

    auto root = getRoot();
    if (!root.isObject()) {
        return false;
    }

    for (const auto& key : requiredKeys) {
        if (!root.hasKey(key)) {
            return false;
        }
    }

    return true;
}

// 示例：添加一个合并JSON对象的方法
void JsonParser::merge(const JsonParser& other) {
    if (!isValid() || !other.isValid()) {
        throw JsonParseException("Cannot merge invalid JSON documents");
    }

    // 这里是一个简化的合并实现
    // 实际应用中可能需要更复杂的合并逻辑

    // 将另一个JSON转换为字符串，然后重新解析合并
    // 注意：这是一个简化实现，实际应用中需要更精细的控制
    auto thisStr = toString();
    auto otherStr = other.toString();

    // 简单的字符串级别合并（这里仅作示例）
    // 实际应用中应该在rapidjson::Value级别进行合并
}

// 静态辅助方法：验证JSON字符串格式
bool JsonParser::isValidJsonString(const std::string& jsonString) {
    rapidjson::Document testDoc;
    testDoc.Parse(jsonString.c_str());
    return !testDoc.HasParseError();
}