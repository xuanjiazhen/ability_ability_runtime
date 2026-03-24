/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "schema_validator.h"
#include "hilog_tag_wrapper.h"
#include <regex>
#include <algorithm>

namespace OHOS {
namespace CliMgr {

ValidationResult SchemaValidator::Validate(
    const Json::Value& args,
    const Json::Value& schema)
{
    ValidationResult result;

    // 1. 检查必填字段
    if (schema.isMember("required")) {
        const Json::Value& required = schema["required"];
        for (const auto& field : required) {
            if (!args.isMember(field.asString())) {
                result.AddError("Missing required field: " + field.asString());
            }
        }
    }

    // 2. 检查每个属性
    if (schema.isMember("properties")) {
        const Json::Value& properties = schema["properties"];

        // 检查是否有未知字段（如果 additionalProperties 为 false）
        bool allowAdditional = true;
        if (schema.isMember("additionalProperties")) {
            const Json::Value& addProp = schema["additionalProperties"];
            if (addProp.isBool()) {
                allowAdditional = addProp.asBool();
            }
        }

        for (const auto& key : args.getMemberNames()) {
            if (!properties.isMember(key)) {
                if (!allowAdditional) {
                    result.AddError("Unknown field: " + key);
                }
                continue;
            }

            const Json::Value& propSchema = properties[key];
            const Json::Value& value = args[key];

            ValidateField(key, value, propSchema, result);
        }
    }

    return result;
}

void SchemaValidator::ValidateField(
    const std::string& fieldName,
    const Json::Value& value,
    const Json::Value& schema,
    ValidationResult& result)
{
    // 类型检查
    if (!ValidateType(fieldName, value, schema, result)) {
        return;  // 类型不匹配，后续检查没有意义
    }

    // 常量检查（const 字段）
    if (schema.isMember("const")) {
        if (!ValidateConst(value, schema["const"])) {
            result.AddError(fieldName + ": must be constant value");
            return;
        }
    }

    // 枚举检查（enum 字段）
    if (schema.isMember("enum")) {
        if (!ValidateEnum(value, schema["enum"])) {
            result.AddError(fieldName + ": value not in allowed enum");
            return;
        }
    }

    // 字符串约束
    if (value.isString()) {
        ValidateStringConstraints(fieldName, value.asString(), schema, result);
    }
    // 数值约束
    else if (value.isNumeric()) {
        ValidateNumberConstraints(fieldName, value, schema, result);
    }
    // 数组约束
    else if (value.isArray()) {
        ValidateArrayConstraints(fieldName, value, schema, result);
    }
    // 对象约束
    else if (value.isObject()) {
        ValidateObjectConstraints(fieldName, value, schema, result);
    }
}

bool SchemaValidator::ValidateType(
    const std::string& fieldName,
    const Json::Value& value,
    const Json::Value& schema,
    ValidationResult& result)
{
    // 获取类型定义
    if (!schema.isMember("type")) {
        return true;  // 没有类型定义，跳过检查
    }

    const Json::Value& typeDef = schema["type"];

    // 联合类型 type: ["string", "null"]
    if (typeDef.isArray()) {
        bool matched = false;
        for (const auto& typeItem : typeDef) {
            if (CheckType(value, typeItem.asString())) {
                matched = true;
                break;
            }
        }
        if (!matched) {
            result.AddError(fieldName + ": type mismatch, expected one of " +
                           typeDef.toStyledString());
            return false;
        }
        return true;
    }

    // 单一类型
    std::string type = typeDef.asString();
    if (!CheckType(value, type)) {
        result.AddError(fieldName + ": expected " + type);
        return false;
    }

    return true;
}

bool SchemaValidator::CheckType(const Json::Value& value, const std::string& expectedType)
{
    if (expectedType == "string") return value.isString();
    if (expectedType == "number") return value.isDouble() || value.isInt();
    if (expectedType == "integer") return value.isInt();
    if (expectedType == "boolean") return value.isBool();
    if (expectedType == "array") return value.isArray();
    if (expectedType == "object") return value.isObject();
    if (expectedType == "null") return value.isNull();
    return true;  // 任意类型
}

void SchemaValidator::ValidateStringConstraints(
    const std::string& fieldName,
    const Json::Value& value,
    const Json::Value& schema,
    ValidationResult& result)
{
    std::string strVal = value.asString();

    // minLength
    if (schema.isMember("minLength")) {
        int minLength = schema["minLength"].asInt();
        if (static_cast<int>(strVal.length()) < minLength) {
            result.AddError(fieldName + ": length must be >= " + std::to_string(minLength));
        }
    }

    // maxLength
    if (schema.isMember("maxLength")) {
        int maxLength = schema["maxLength"].asInt();
        if (static_cast<int>(strVal.length()) > maxLength) {
            result.AddError(fieldName + ": length must be <= " + std::to_string(maxLength));
        }
    }

    // pattern (正则表达式)
    if (schema.isMember("pattern")) {
        std::string pattern = schema["pattern"].asString();
        if (!ValidatePattern(strVal, pattern)) {
            result.AddError(fieldName + ": does not match pattern " + pattern);
        }
    }

    // format (内置格式)
    if (schema.isMember("format")) {
        std::string format = schema["format"].asString();
        if (!ValidateFormat(strVal, format)) {
            result.AddError(fieldName + ": does not match format " + format);
        }
    }
}

void SchemaValidator::ValidateNumberConstraints(
    const std::string& fieldName,
    const Json::Value& value,
    const Json::Value& schema,
    ValidationResult& result)
{
    double numVal = value.asDouble();

    // minimum
    if (schema.isMember("minimum")) {
        double minimum = schema["minimum"].asDouble();
        if (numVal < minimum) {
            result.AddError(fieldName + ": must be >= " + std::to_string(minimum));
        }
    }

    // maximum
    if (schema.isMember("maximum")) {
        double maximum = schema["maximum"].asDouble();
        if (numVal > maximum) {
            result.AddError(fieldName + ": must be <= " + std::to_string(maximum));
        }
    }

    // exclusiveMinimum
    if (schema.isMember("exclusiveMinimum")) {
        double exclusiveMin = schema["exclusiveMinimum"].asDouble();
        if (numVal <= exclusiveMin) {
            result.AddError(fieldName + ": must be > " + std::to_string(exclusiveMin));
        }
    }

    // exclusiveMaximum
    if (schema.isMember("exclusiveMaximum")) {
        double exclusiveMax = schema["exclusiveMaximum"].asDouble();
        if (numVal >= exclusiveMax) {
            result.AddError(fieldName + ": must be < " + std::to_string(exclusiveMax));
        }
    }

    // multipleOf
    if (schema.isMember("multipleOf")) {
        double multipleOf = schema["multipleOf"].asDouble();
        if (multipleOf > 0 && fmod(numVal, multipleOf) != 0) {
            result.AddError(fieldName + ": must be multiple of " + std::to_string(multipleOf));
        }
    }
}

void SchemaValidator::ValidateArrayConstraints(
    const std::string& fieldName,
    const Json::Value& value,
    const Json::Value& schema,
    ValidationResult& result)
{
    int arraySize = value.size();

    // minItems
    if (schema.isMember("minItems")) {
        int minItems = schema["minItems"].asInt();
        if (arraySize < minItems) {
            result.AddError(fieldName +": array size must be >= " + std::to_string(minItems));
        }
    }

    // maxItems
    if (schema.isMember("maxItems")) {
        int maxItems = schema["maxItems"].asInt();
        if (arraySize > maxItems) {
            result.AddError(fieldName + ": array size must be <= " + std::to_string(maxItems));
        }
    }

    // uniqueItems
    if (schema.isMember("uniqueItems") && schema["uniqueItems"].asBool()) {
        // 检查数组元素是否唯一
        Json::Value::Members members;
        for (const auto& item : value) {
            std::string itemStr = item.toStyledString();
            if (std::find(members.begin(), members.end(), itemStr) != members.end()) {
                result.AddError(fieldName + ": array items must be unique");
                break;
            }
            members.push_back(itemStr);
        }
    }

    // items 验证（验证数组元素）
    if (schema.isMember("items")) {
        const Json::Value& itemsSchema = schema["items"];
        for (const auto& item : value) {
            ValidationResult itemResult;
            ValidateField(fieldName + "[]", item, itemsSchema, itemResult);
            result.Merge(itemResult);
        }
    }
}

void SchemaValidator::ValidateObjectConstraints(
    const std::string& fieldName,
    const Json::Value& value,
    const Json::Value& schema,
    ValidationResult& result)
{
    // 递归验证嵌套对象
    if (schema.isMember("properties")) {
        const Json::Value& properties = schema["properties"];

        // 检查必填字段
        if (schema.isMember("required")) {
            const Json::Value& required = schema["required"];
            for (const auto& field : required) {
                if (!value.isMember(field.asString())) {
                    result.AddError(fieldName + "." + field.asString() + ": missing required field");
                }
            }
        }

        // 验证每个属性
        for (const auto& key : value.getMemberNames()) {
            if (!properties.isMember(key)) {
                continue;  // 未知字段由 additionalProperties 处理
            }

            const Json::Value& propSchema = properties[key];
            const Json::Value& propValue = value[key];

            ValidateField(fieldName + "." + key, propValue, propSchema, result);
        }
    }
}

bool SchemaValidator::ValidateFormat(const std::string& value, const std::string& format)
{
    if (format == "ipv4") {
        // 简单的 IPv4 验证
        std::regex ipv4Pattern(
            R"(^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)"
        );
        return std::regex_match(value, ipv4Pattern);
    }
    else if (format == "ipv6") {
        // 简单的 IPv6 验证（简化版）
        return value.find(':') != std::string::npos;
    }
    else if (format == "email") {
        // 简单的邮箱验证
        std::regex emailPattern(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
        return std::regex_match(value, emailPattern);
    }
    else if (format == "uri") {
        // 简单的 URI 验证
        return value.find("://") != std::string::npos;
    }
    else if (format == "hostname") {
        // 主机名验证
        std::regex hostnamePattern(R"(^[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(\.[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$)");
        return std::regex_match(value, hostnamePattern);
    }

    return true;  // 未知格式，不验证
}

bool SchemaValidator::ValidatePattern(const std::string& value, const std::string& pattern)
{
    try {
        std::regex regexPattern(pattern);
        return std::regex_match(value, regexPattern);
    } catch (const std::regex_error& e) {
        TAG_LOGE(TAG, "Invalid regex pattern: %{public}s, error: %{public}s",
                 pattern.c_str(), e.what());
        return false;
    }
}

bool SchemaValidator::ValidateEnum(const Json::Value& value, const Json::Value& enumSchema)
{
    for (const auto& enumValue : enumSchema) {
        if (value == enumValue) {
            return true;
        }
    }
    return false;
}

bool SchemaValidator::ValidateConst(const Json::Value& value, const Json::Value& constValue)
{
    return value == constValue;
}

} // namespace CliMgr
} // namespace OHOS
