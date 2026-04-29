/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "tool_util.h"

#include <algorithm>
#include <climits>
#include <nlohmann/json.hpp>
#include <random>
#include <sstream>
#include <utility>
#include <vector>

#include "accesstoken_kit.h"
#include "bundle_info.h"
#include "bundle_mgr_helper.h"
#include "cli_error_code.h"
#include "exec_tool_param.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "permission_util.h"
#include "session_record.h"
#include "tool_info.h"
#include "want_params.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t MILLISECOND_COEFFICIENT = 1000;
constexpr int64_t MAX_TIMEOUT = 30 * 60; // 30 m
}
int32_t ToolUtil::ValidateProperties(const ToolInfo &toolInfo, ExecToolParam &param,
    AccessToken::AccessTokenID tokenId)
{
    if (!param.subcommand.empty()) {
        if (!toolInfo.hasSubCommand) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "not have subcommand");
            return ERR_INVALID_PARAM;
        }

        auto search = toolInfo.subcommands.find(param.subcommand);
        if (search == toolInfo.subcommands.end()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "not have subcommand");
            return ERR_INVALID_PARAM;
        }
        if (!PermissionUtil::VerifyAccessToken(tokenId, search->second.requirePermissions)) {
            return ERR_PERMISSION_DENIED;
        }
    } else {
        if (!PermissionUtil::VerifyAccessToken(tokenId, toolInfo.requirePermissions)) {
            return ERR_PERMISSION_DENIED;
        }
    }

    if (param.options.timeout < 0 || param.options.yieldMs < 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "yieldMs or timeout < 0");
        return ERR_INVALID_PARAM;
    }

    if (param.options.timeout == 0) {
        param.options.timeout = MAX_TIMEOUT;
        TAG_LOGI(AAFwkTag::CLI_TOOL, "use max timeout");
    } else if (param.options.timeout > MAX_TIMEOUT) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Excessively large timeout");
        return ERR_INVALID_PARAM;
    }

    if (!param.options.background) {
        if (param.options.yieldMs == 0) {
            param.options.yieldMs = param.options.timeout * MILLISECOND_COEFFICIENT;
        } else if (param.options.yieldMs > param.options.timeout * MILLISECOND_COEFFICIENT) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "yieldTime exceeds timeout.");
            return ERR_INVALID_PARAM;
        }
    }

    return ValidateInputSchemaProperties(toolInfo.inputSchema, param.args);
}

int32_t ToolUtil::ValidateInputSchemaProperties(const std::string &inputSchema,
    const AAFwk::WantParams &args)
{
    if (args.IsEmpty()) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "args is empty");
        return ERR_OK;
    }

    if (inputSchema.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "inputSchema is empty");
        return ERR_INVALID_PARAM;
    }

    nlohmann::json schema = nlohmann::json::parse(inputSchema, nullptr, false);
    if (schema.is_discarded()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "discarded error");
        return ERR_NO_INIT;
    }
    if (!schema.contains("properties") || !schema["properties"].is_object()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "properties not found or invalid");
        return ERR_INVALID_PARAM;
    }
    auto properties = schema["properties"];
    for (auto &[key, value] : args.GetParams()) {
        if (!properties.contains(key)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "args key '%{public}s' not found in properties", key.c_str());
            return ERR_INVALID_PARAM;
        }

        // Validate type if specified in schema
        auto &propertySchema = properties[key];
        if (propertySchema.contains("type")) {
            std::string expectedType = propertySchema["type"].get<std::string>();
            if (!ValidateParamType(value, expectedType, propertySchema, key)) {
                TAG_LOGE(AAFwkTag::CLI_TOOL, "args key '%{public}s' type mismatch, expected: %{public}s",
                    key.c_str(), expectedType.c_str());
                return ERR_INVALID_PARAM;
            }
        }
    }
    return ERR_OK;
}

std::string ToolUtil::GenerateCliSessionId(const std::string &name, std::shared_ptr<SessionRecord> record)
{
    std::random_device seed;
    std::mt19937 rng(seed());
    std::uniform_int_distribution<int> uni(0, INT_MAX);
    int randomDigit = uni(rng);
    auto timestamp = std::chrono::system_clock::now().time_since_epoch();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp).count();
    if (record != nullptr) {
        record->startTime = time;
    }
    return name + "_" + std::to_string(time) + "_" + std::to_string(randomDigit);
}

bool ToolUtil::GenerateSandboxConfig(const std::string &challenge, AccessToken::AccessTokenID tokenId,
    std::string &sandboxConfig, std::string &bundleName)
{
    AppExecFwk::BundleInfo bundleInfo;
    if (!ToolUtil::GetBundleInfoByTokenId(tokenId, bundleInfo)) {
        return false;
    }

    nlohmann::json config;
    config["callerTokenId"] = IPCSkeleton::GetCallingFullTokenID();
    config["challenge"] = challenge;
    config["uid"] = IPCSkeleton::GetCallingUid();
    config["callerPid"] = IPCSkeleton::GetCallingPid();
    config["gid"] = bundleInfo.gid;
    config["appId"] = bundleInfo.appId;
    sandboxConfig = config.dump();
    bundleName = bundleInfo.name;
    TAG_LOGE(AAFwkTag::CLI_TOOL, "sandboxConfig: %{public}s", sandboxConfig.c_str());
    return true;
}

bool ToolUtil::GetBundleInfoByTokenId(AccessToken::AccessTokenID tokenId, AppExecFwk::BundleInfo &bundleInfo)
{
    auto tokenType = AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType != AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "caller is not hap");
        return false;
    }
    AccessToken::HapTokenInfo hapInfo;
    auto ret = AccessToken::AccessTokenKit::GetHapTokenInfo(tokenId, hapInfo);
    if (ret != AccessToken::AccessTokenKitRet::RET_SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetHapTokenInfo failed, ret:%{public}d", ret);
        return false;
    }

    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "bundlerMgrHelper is invalid");
        return false;
    }
    auto flag = static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION);
    if (hapInfo.instIndex == 0) {
        if (bundleMgrHelper->GetBundleInfoV9(hapInfo.bundleName, flag, bundleInfo, hapInfo.userID) != ERR_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Fail to get bundle info");
            return false;
        }
        return true;
    }
    if (bundleMgrHelper->GetCloneBundleInfo(hapInfo.bundleName, flag, hapInfo.instIndex, bundleInfo, hapInfo.userID) !=
        ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Fail to get bundle info");
        return false;
    }
    return true;
}

void ToolUtil::TransferToCmdParam(const ToolInfo &toolInfo, const AAFwk::WantParams &args, std::string &cmdLine)
{
    if (args.IsEmpty()) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Not has arg");
        return;
    }
    for (const auto &[key, value] : args.GetParams()) {
        if (value == nullptr) {
            continue;
        }

        // 规则2：value是boolean类型，value是true，将[key, value]映射为 --key；value是false，不对[key, value]进行映射
        if (IsBooleanType(value)) {
            bool boolValue = false;
            if (GetParamBoolValue(value, boolValue) && boolValue) {
                cmdLine += " --" + key;
            }
            continue;
        }

        // 规则3：value是Array类型，不支持Array嵌套，按照ProcessArrayExpansion进行映射
        if (IsArrayType(value)) {
            ProcessArrayExpansion(key, value, cmdLine);
            continue;
        }

        // 规则1：value是string、Integer、Long、Float、double类型，将[key, value]映射为 --key + " " + value转换为的string
        std::string strValue = GetParamStringValue(value);
        if (!strValue.empty()) {
            cmdLine += " --" + key + " " + strValue;
        }
    }
}

std::string ToolUtil::WantParamsToJson(const AAFwk::WantParams &wantParams)
{
    nlohmann::json result = nlohmann::json::object();

    for (const auto &[key, value] : wantParams.GetParams()) {
        // Check if value is another WantParams (nested object)
        AAFwk::WantParams nestedWantParams;
        if (ExtractWantParams(value, nestedWantParams)) {
            // Recursively convert nested WantParams to JSON
            result[key] = nlohmann::json::parse(WantParamsToJson(nestedWantParams), nullptr, false);
        } else if (value != nullptr) {
            // Convert IInterface to JSON value using GetParamJsonValue
            std::string valueJsonStr = GetParamJsonValue(value);
            nlohmann::json valueJson = nlohmann::json::parse(valueJsonStr, nullptr, false);
            if (!valueJson.is_discarded()) {
                result[key] = valueJson;
            }
        }
    }

    return result.dump();
}

std::string ToolUtil::GetParamStringValue(const sptr<AAFwk::IInterface> &value)
{
    if (value == nullptr) {
        return "";
    }

    // Try String
    if (auto strObj = AAFwk::IString::Query(value)) {
        std::string strValue;
        if (strObj->GetString(strValue) == ERR_OK) {
            return strValue;
        }
        return "";
    }

    // Try Boolean
    if (auto boolObj = AAFwk::IBoolean::Query(value)) {
        bool boolValue = false;
        if (boolObj->GetValue(boolValue) == ERR_OK) {
            return boolValue ? "true" : "false";
        }
        return "";
    }

    // Try Integer/Int
    if (auto intObj = AAFwk::IInteger::Query(value)) {
        int intValue = 0;
        if (intObj->GetValue(intValue) == ERR_OK) {
            return std::to_string(intValue);
        }
        return "";
    }

    // Try Long
    if (auto longObj = AAFwk::ILong::Query(value)) {
        long longValue = 0;
        if (longObj->GetValue(longValue) == ERR_OK) {
            return std::to_string(longValue);
        }
        return "";
    }

    // Try Float
    if (auto floatObj = AAFwk::IFloat::Query(value)) {
        float floatValue = 0.0f;
        if (floatObj->GetValue(floatValue) == ERR_OK) {
            return std::to_string(floatValue);
        }
        return "";
    }

    // Try Double
    if (auto doubleObj = AAFwk::IDouble::Query(value)) {
        double doubleValue = 0.0;
        if (doubleObj->GetValue(doubleValue) == ERR_OK) {
            return std::to_string(doubleValue);
        }
        return "";
    }

    // If no type matches, return empty string
    return "";
}

std::string ToolUtil::GetParamJsonValue(const sptr<AAFwk::IInterface> &value)
{
    if (value == nullptr) {
        return "null";
    }

    // Try WantParams (nested object) - should check before other types
    std::string jsonStr = ConvertWantParamsToJson(value);
    if (!jsonStr.empty()) {
        return jsonStr;
    }

    // Try Array - only single-level array support (no nested arrays)
    jsonStr = ConvertArrayToJson(value);
    if (!jsonStr.empty()) {
        return jsonStr;
    }

    // Try String - JSON strings are quoted
    jsonStr = ConvertStringToJson(value);
    if (!jsonStr.empty()) {
        return jsonStr;
    }

    // Try Boolean - JSON booleans are unquoted
    jsonStr = ConvertBooleanToJson(value);
    if (!jsonStr.empty()) {
        return jsonStr;
    }

    // Try Numeric types (int, long, float, double)
    jsonStr = ConvertNumericToJson(value);
    if (!jsonStr.empty()) {
        return jsonStr;
    }

    // If no type matches, return null
    return "null";
}

std::string ToolUtil::ConvertWantParamsToJson(const sptr<AAFwk::IInterface> &value)
{
    AAFwk::WantParams wantParams;
    if (ExtractWantParams(value, wantParams)) {
        return WantParamsToJson(wantParams);
    }
    return "";
}

std::string ToolUtil::ConvertArrayToJson(const sptr<AAFwk::IInterface> &value)
{
    if (auto arrayObj = AAFwk::IArray::Query(value)) {
        return BuildJsonArrayFromIArray(arrayObj, [](const sptr<AAFwk::IInterface>& element) {
            return GetParamJsonValue(element);
        });
    }
    return "";
}

std::string ToolUtil::ConvertStringToJson(const sptr<AAFwk::IInterface> &value)
{
    if (auto strObj = AAFwk::IString::Query(value)) {
        std::string strValue;
        if (strObj->GetString(strValue) != ERR_OK) {
            return "\"\"";
        }
        return "\"" + EscapeJsonString(strValue) + "\"";
    }
    return "";
}

std::string ToolUtil::ConvertBooleanToJson(const sptr<AAFwk::IInterface> &value)
{
    if (auto boolObj = AAFwk::IBoolean::Query(value)) {
        bool boolValue = false;
        if (boolObj->GetValue(boolValue) == ERR_OK) {
            return boolValue ? "true" : "false";
        }
        return "false";
    }
    return "";
}

std::string ToolUtil::ConvertNumericToJson(const sptr<AAFwk::IInterface> &value)
{
    // Try Integer/Int
    if (auto intObj = AAFwk::IInteger::Query(value)) {
        int intValue = 0;
        if (intObj->GetValue(intValue) == ERR_OK) {
            return std::to_string(intValue);
        }
        return "0";
    }

    // Try Long
    if (auto longObj = AAFwk::ILong::Query(value)) {
        long longValue = 0;
        if (longObj->GetValue(longValue) == ERR_OK) {
            return std::to_string(longValue);
        }
        return "0";
    }

    // Try Float
    if (auto floatObj = AAFwk::IFloat::Query(value)) {
        float floatValue = 0.0f;
        if (floatObj->GetValue(floatValue) == ERR_OK) {
            return std::to_string(floatValue);
        }
        return "0.0";
    }

    // Try Double
    if (auto doubleObj = AAFwk::IDouble::Query(value)) {
        double doubleValue = 0.0;
        if (doubleObj->GetValue(doubleValue) == ERR_OK) {
            return std::to_string(doubleValue);
        }
        return "0.0";
    }

    return "";
}

bool ToolUtil::GetParamBoolValue(const sptr<AAFwk::IInterface> &value, bool &result)
{
    if (value == nullptr) {
        return false;
    }

    // Try Boolean type directly
    if (auto boolObj = AAFwk::IBoolean::Query(value)) {
        if (boolObj->GetValue(result) == ERR_OK) {
            return true;
        }
        return false;
    }

    return false;
}

// ============================================================================
// Helper methods for code reuse
// ============================================================================

bool ToolUtil::ExtractWantParams(const sptr<AAFwk::IInterface> &value, AAFwk::WantParams &wantParams)
{
    if (value == nullptr) {
        return false;
    }

    auto wantParamsWrapper = AAFwk::IWantParams::Query(value);
    if (wantParamsWrapper == nullptr) {
        return false;
    }

    return wantParamsWrapper->GetValue(wantParams) == ERR_OK;
}

std::string ToolUtil::EscapeJsonString(const std::string &str)
{
    std::string escaped = "";
    for (char c : str) {
        if (c == '"') {
            escaped += "\\\"";
        } else if (c == '\\') {
            escaped += "\\\\";
        } else if (c == '\n') {
            escaped += "\\n";
        } else if (c == '\r') {
            escaped += "\\r";
        } else if (c == '\t') {
            escaped += "\\t";
        } else {
            escaped += c;
        }
    }
    return escaped;
}

void ToolUtil::IterateIArray(sptr<AAFwk::IArray> arrayObj,
    std::function<void(const sptr<AAFwk::IInterface>&)> elementHandler)
{
    if (arrayObj == nullptr) {
        return;
    }

    long arrayLength = 0;
    if (arrayObj->GetLength(arrayLength) != ERR_OK || arrayLength <= 0) {
        return;
    }

    for (long i = 0; i < arrayLength; ++i) {
        sptr<AAFwk::IInterface> elementValue;
        if (arrayObj->Get(i, elementValue) != ERR_OK || elementValue == nullptr) {
            continue;
        }

        // Skip nested arrays
        if (AAFwk::IArray::Query(elementValue) != nullptr) {
            continue;
        }

        elementHandler(elementValue);
    }
}

std::string ToolUtil::BuildJsonArrayFromIArray(sptr<AAFwk::IArray> arrayObj,
    std::function<std::string(const sptr<AAFwk::IInterface>&)> elementConverter)
{
    if (arrayObj == nullptr) {
        return "[]";
    }

    long arrayLength = 0;
    if (arrayObj->GetLength(arrayLength) != ERR_OK || arrayLength <= 0) {
        return "[]";
    }

    std::string result = "[";
    bool firstElement = true;

    IterateIArray(arrayObj, [&result, &firstElement, &elementConverter](const sptr<AAFwk::IInterface>& element) {
        if (!firstElement) {
            result += ",";
        }
        result += elementConverter(element);
        firstElement = false;
    });

    result += "]";
    return result;
}

void ToolUtil::ProcessArrayExpansion(const std::string &key, const sptr<AAFwk::IInterface> &value,
    std::string &cmdLine)
{
    auto arrayValue = AAFwk::IArray::Query(value);
    if (arrayValue == nullptr) {
        return;
    }

    long arrayLength = 0;
    if (arrayValue->GetLength(arrayLength) != ERR_OK || arrayLength <= 0) {
        return;
    }

    // Iterate through array elements and expand to command line
    // 规则3：Array类型展开为 --key value1 --key value2 ...
    for (long i = 0; i < arrayLength; ++i) {
        sptr<AAFwk::IInterface> elementValue;
        if (arrayValue->Get(i, elementValue) != ERR_OK || elementValue == nullptr) {
            continue;
        }

        // Skip nested arrays
        if (AAFwk::IArray::Query(elementValue) != nullptr) {
            continue;
        }

        std::string elementStr = GetParamStringValue(elementValue);
        if (!elementStr.empty()) {
            cmdLine += " --" + key + " " + elementStr;
        }
    }
}

// ============================================================================
// Type validation helpers for ValidateInputSchemaProperties
// ============================================================================

bool ToolUtil::ValidateParamType(const sptr<AAFwk::IInterface> &value, const std::string &expectedType,
    const nlohmann::json &propertySchema, const std::string &key)
{
    if (value == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "value is null for key '%{public}s'", key.c_str());
        return false;
    }
    if (expectedType == "array") {
        return ValidateArrayType(value, propertySchema, key);
    }
    if (expectedType == "object") {
        return ValidateObjectType(value, propertySchema, key);
    }
    return ValidateBasicType(value, expectedType);
}

bool ToolUtil::ValidateBasicType(const sptr<AAFwk::IInterface> &value, const std::string &expectedType)
{
    if (expectedType == "string") {
        return IsStringType(value);
    }
    if (expectedType == "boolean") {
        return IsBooleanType(value);
    }
    if (expectedType == "integer") {
        return IsIntegerType(value);
    }
    if (expectedType == "number") {
        return IsNumberType(value);
    }
    TAG_LOGW(AAFwkTag::CLI_TOOL, "Unknown type '%{public}s', allowing for compatibility", expectedType.c_str());
    return true;
}

bool ToolUtil::ValidateArrayType(const sptr<AAFwk::IInterface> &value,
    const nlohmann::json &propertySchema, const std::string &key)
{
    if (!IsArrayType(value)) {
        return false;
    }
    if (!propertySchema.contains("items") || !propertySchema["items"].is_object()) {
        return true;
    }
    auto arrayObj = AAFwk::IArray::Query(value);
    if (arrayObj == nullptr) {
        return false;
    }
    return ValidateArrayItems(arrayObj, propertySchema["items"], key);
}

bool ToolUtil::ValidateObjectType(const sptr<AAFwk::IInterface> &value,
    const nlohmann::json &propertySchema, const std::string &key)
{
    if (!IsObjectType(value)) {
        return false;
    }
    AAFwk::WantParams nestedParams;
    if (!ExtractWantParams(value, nestedParams)) {
        return false;
    }
    return ValidateNestedObject(nestedParams, propertySchema, key);
}

bool ToolUtil::ValidateArrayItems(sptr<AAFwk::IArray> arrayObj,
    const nlohmann::json &itemsSchema, const std::string &key)
{
    long arrayLength = 0;
    if (arrayObj->GetLength(arrayLength) != ERR_OK || arrayLength == 0) {
        return true;
    }
    std::string itemType = itemsSchema.value("type", "");
    if (itemType.empty()) {
        return true;
    }
    for (long i = 0; i < arrayLength; ++i) {
        sptr<AAFwk::IInterface> elementValue;
        if (arrayObj->Get(i, elementValue) != ERR_OK || elementValue == nullptr) {
            continue;
        }
        std::string elementKey = key + "[" + std::to_string(i) + "]";
        if (!ValidateParamType(elementValue, itemType, itemsSchema, elementKey)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Array '%{public}s' element %{public}ld type mismatch, expected: %{public}s",
                key.c_str(), i, itemType.c_str());
            return false;
        }
    }
    return true;
}

bool ToolUtil::ValidateNestedObject(const AAFwk::WantParams &nestedParams,
    const nlohmann::json &objectSchema, const std::string &parentKey)
{
    if (!objectSchema.contains("properties") || !objectSchema["properties"].is_object()) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "No nested properties defined for '%{public}s', skipping validation",
            parentKey.c_str());
        return true;
    }
    auto nestedProperties = objectSchema["properties"];
    if (objectSchema.contains("required") && objectSchema["required"].is_array()) {
        for (const auto &requiredProp : objectSchema["required"]) {
            if (!requiredProp.is_string()) {
                continue;
            }
            std::string requiredKey = requiredProp.get<std::string>();
            std::string fullKey = parentKey + "." + requiredKey;
            if (nestedParams.GetParams().find(requiredKey) == nestedParams.GetParams().end()) {
                TAG_LOGE(AAFwkTag::CLI_TOOL, "Required nested property '%{public}s' not found", fullKey.c_str());
                return false;
            }
        }
    }
    for (const auto &[nestedKey, nestedValue] : nestedParams.GetParams()) {
        std::string fullKey = parentKey + "." + nestedKey;
        if (!nestedProperties.contains(nestedKey)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Nested property '%{public}s' not found in schema", fullKey.c_str());
            return false;
        }
        auto &nestedPropertySchema = nestedProperties[nestedKey];
        if (nestedPropertySchema.contains("type")) {
            std::string expectedType = nestedPropertySchema["type"].get<std::string>();
            if (!ValidateParamType(nestedValue, expectedType, nestedPropertySchema, fullKey)) {
                TAG_LOGE(AAFwkTag::CLI_TOOL, "Nested property '%{public}s' type mismatch, expected: %{public}s",
                    fullKey.c_str(), expectedType.c_str());
                return false;
            }
        }
    }
    return true;
}

bool ToolUtil::IsStringType(const sptr<AAFwk::IInterface> &value)
{
    return AAFwk::IString::Query(value) != nullptr;
}

bool ToolUtil::IsBooleanType(const sptr<AAFwk::IInterface> &value)
{
    return AAFwk::IBoolean::Query(value) != nullptr;
}

bool ToolUtil::IsIntegerType(const sptr<AAFwk::IInterface> &value)
{
    return AAFwk::IInteger::Query(value) != nullptr || AAFwk::ILong::Query(value) != nullptr;
}

bool ToolUtil::IsNumberType(const sptr<AAFwk::IInterface> &value)
{
    return AAFwk::IInteger::Query(value) != nullptr ||
           AAFwk::ILong::Query(value) != nullptr ||
           AAFwk::IFloat::Query(value) != nullptr ||
           AAFwk::IDouble::Query(value) != nullptr;
}

bool ToolUtil::IsArrayType(const sptr<AAFwk::IInterface> &value)
{
    return AAFwk::IArray::Query(value) != nullptr;
}

bool ToolUtil::IsObjectType(const sptr<AAFwk::IInterface> &value)
{
    AAFwk::WantParams wantParams;
    return ExtractWantParams(value, wantParams);
}

} // namespace CliTool
} // namespace OHOS
