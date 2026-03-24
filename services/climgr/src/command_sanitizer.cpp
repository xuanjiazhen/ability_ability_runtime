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

#include "command_sanitizer.h"
#include "hilog_tag_wrapper.h"
#include <algorithm>
#include <regex>
#include <cstdlib>

namespace OHOS {
namespace CliMgr {

bool CommandSanitizer::SanitizeArgs(const Json::Value& rawInput,
                                  const Json::Value& schema,
                                  Json::Value& sanitizedOutput)
{
    if (!rawInput.isObject() || !schema.isObject()) {
        TAG_LOGE(TAG, "Invalid input or schema: not an object");
        return false;
    }

    sanitizedOutput = Json::Value(Json::objectValue);

    // 获取 properties
    if (!schema.isMember("properties") || !schema["properties"].isObject()) {
        TAG_LOGE(TAG, "Schema missing 'properties' field");
        return false;
    }

    const Json::Value& properties = schema["properties"];

    // 检查 required 字段
    std::vector<std::string> requiredFields;
    if (schema.isMember("required") && schema["required"].isArray()) {
        for (const auto& field : schema["required"]) {
            if (field.isString()) {
                requiredFields.push_back(field.asString());
            }
        }
    }

    // 验证必填字段
    for (const auto& field : requiredFields) {
        if (!rawInput.isMember(field)) {
            TAG_LOGE(TAG, "Missing required field: %{public}s", field.c_str());
            return false;
        }
    }

    // 验证每个字段
    for (const auto& key : rawInput.getMemberNames()) {
        // 检查 schema 中是否定义了该字段
        if (!properties.isMember(key)) {
            TAG_LOGW(TAG, "Unknown field in input: %{public}s, ignoring", key.c_str());
            continue;
        }

        const Json::Value& value = rawInput[key];
        const Json::Value& fieldSchema = properties[key];

        // 验证类型
        std::string expectedType = "string";
        if (fieldSchema.isMember("type")) {
            expectedType = fieldSchema["type"].asString();
        }

        if (!ValidateType(value, expectedType)) {
            TAG_LOGE(TAG, "Type mismatch for field %{public}s: expected %{public}s",
                     key.c_str(), expectedType.c_str());
            return false;
        }

        // 验证范围（数字类型）
        if (expectedType == "integer" || expectedType == "number") {
            if (!ValidateRange(value, fieldSchema)) {
                TAG_LOGE(TAG, "Value out of range for field %{public}s", key.c_str());
                return false;
            }
        }

        // 额外的字符串验证
        if (expectedType == "string") {
            std::string strValue = value.asString();

            // 检查格式
            if (fieldSchema.isMember("format")) {
                std::string format = fieldSchema["format"].asString();
                if (format == "ipv4" && !ValidateIP(strValue)) {
                    TAG_LOGE(TAG, "Invalid IPv4 address: %{public}s", strValue.c_str());
                    return false;
                }
            }

            // 检查危险字符
            if (!ValidateString(strValue)) {
                TAG_LOGE(TAG, "Dangerous characters in field %{public}s", key.c_str());
                return false;
            }

            // 转义字符串
            sanitizedOutput[key] = EscapeString(strValue);
        } else {
            // 其他类型直接复制
            sanitizedOutput[key] = value;
        }
    }

    return true;
}

bool CommandSanitizer::ValidateString(const std::string& value,
                                     const std::string& dangerousChars)
{
    return !ContainsDangerousChars(value, dangerousChars);
}

std::string CommandSanitizer::EscapeString(const std::string& input)
{
    std::string result;
    result.reserve(input.size() * 2);

    for (char c : input) {
        switch (c) {
            case '\\':
                result += "\\\\";
                break;
            case '"':
                result += "\\\"";
                break;
            case '$':
                result += "\\$";
                break;
            case '`':
                result += "\\`";
                break;
            default:
                result += c;
                break;
        }
    }

    return result;
}

bool CommandSanitizer::ValidatePath(const std::string& path)
{
    // 检查路径遍历攻击
    if (path.find("..") != std::string::npos) {
        TAG_LOGW(TAG, "Path contains '..': %{public}s", path.c_str());
        return false;
    }

    // 检查绝对路径
    if (!path.empty() && path[0] == '/') {
        // 允许绝对路径，但需要额外验证
    }

    return true;
}

bool CommandSanitizer::ValidateIP(const std::string& ip)
{
    // 简单的 IPv4 验证
    std::regex ipv4Pattern(
        R"(^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)"
    );

    return std::regex_match(ip, ipv4Pattern);
}

bool CommandSanitizer::ValidatePort(int port)
{
    return port >= 1 && port <= 65535;
}

bool CommandSanitizer::ValidateSchema(const Json::Value& schema)
{
    if (!schema.isObject()) {
        return false;
    }

    // 必须有 type 字段
    if (!schema.isMember("type")) {
        return false;
    }

    std::string type = schema["type"].asString();
    if (type != "object") {
        return false;
    }

    // object 类型必须有 properties
    if (!schema.isMember("properties")) {
        return false;
    }

    return true;
}

bool CommandSanitizer::ContainsDangerousChars(const std::string& str,
                                             const std::string& chars)
{
    return str.find_first_of(chars) != std::string::npos;
}

bool CommandSanitizer::ValidateType(const Json::Value& value,
                                    const std::string& expectedType)
{
    if (expectedType == "string") {
        return value.isString();
    } else if (expectedType == "integer") {
        return value.isInt();
    } else if (expectedType == "number") {
        return value.isDouble() || value.isInt();
    } else if (expectedType == "boolean") {
        return value.isBool();
    } else if (expectedType == "array") {
        return value.isArray();
    } else if (expectedType == "object") {
        return value.isObject();
    }

    return false;
}

bool CommandSanitizer::ValidateRange(const Json::Value& value,
                                     const Json::Value& schema)
{
    if (!value.isInt() && !value.isUInt()) {
        return false;
    }

    int intValue = value.asInt();

    // 检查 minimum
    if (schema.isMember("minimum")) {
        int minimum = schema["minimum"].asInt();
        if (intValue < minimum) {
            return false;
        }
    }

    // 检查 maximum
    if (schema.isMember("maximum")) {
        int maximum = schema["maximum"].asInt();
        if (intValue > maximum) {
            return false;
        }
    }

    return true;
}

} // namespace CliMgr
} // namespace OHOS
