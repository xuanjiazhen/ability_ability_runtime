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

#include "function_info.h"

#include <nlohmann/json.hpp>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

namespace {
constexpr const char* FUNCTION_TYPE_INTENT_FUNCTION = "intentFunction";
}

bool FunctionInfo::Marshalling(Parcel &parcel) const
{
    int32_t typeValue = static_cast<int32_t>(functionType);
    if (!parcel.WriteString(functionName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write functionName failed");
        return false;
    }
    if (!parcel.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write bundleName failed");
        return false;
    }
    if (!parcel.WriteString(description)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write description failed");
        return false;
    }
    if (!parcel.WriteString(inputSchema)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write inputSchema failed");
        return false;
    }
    if (!parcel.WriteString(outputSchema)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write outputSchema failed");
        return false;
    }
    if (!parcel.WriteInt32(typeValue)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write functionType failed");
        return false;
    }
    return true;
}

FunctionInfo *FunctionInfo::Unmarshalling(Parcel &parcel)
{
    auto *function = new (std::nothrow) FunctionInfo();
    if (function == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to allocate FunctionInfo");
        return nullptr;
    }

    int32_t typeValue = 0;
    if (!parcel.ReadString(function->functionName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read functionName failed");
        delete function;
        return nullptr;
    }
    if (!parcel.ReadString(function->bundleName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read bundleName failed");
        delete function;
        return nullptr;
    }
    if (!parcel.ReadString(function->description)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read description failed");
        delete function;
        return nullptr;
    }
    if (!parcel.ReadString(function->inputSchema)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read inputSchema failed");
        delete function;
        return nullptr;
    }
    if (!parcel.ReadString(function->outputSchema)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read outputSchema failed");
        delete function;
        return nullptr;
    }
    if (!parcel.ReadInt32(typeValue)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read functionType failed");
        delete function;
        return nullptr;
    }

    function->functionType = static_cast<FunctionType>(typeValue);
    return function;
}

bool FunctionInfo::ParseFromJson(const nlohmann::json &json, FunctionInfo &function)
{
    // functionName is required
    if (!json.contains("functionName") || !json["functionName"].is_string()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: functionName is missing or not a string");
        return false;
    }
    std::string functionName = json["functionName"].get<std::string>();
    if (functionName.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: functionName is empty");
        return false;
    }
    function.functionName = functionName;

    // bundleName is required
    if (!json.contains("bundleName") || !json["bundleName"].is_string()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: bundleName is missing or not a string");
        return false;
    }
    std::string bundleName = json["bundleName"].get<std::string>();
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: bundleName is empty");
        return false;
    }
    function.bundleName = bundleName;

    // description is required
    if (!json.contains("description") || !json["description"].is_string()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: description is missing or not a string");
        return false;
    }
    function.description = json["description"].get<std::string>();

    // inputSchema is required and must be object
    if (!json.contains("inputSchema") || !json["inputSchema"].is_object()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: inputSchema is missing or not an object");
        return false;
    }
    function.inputSchema = json["inputSchema"].dump();

    // outputSchema is optional
    if (json.contains("outputSchema")) {
        if (json["outputSchema"].is_object()) {
            function.outputSchema = json["outputSchema"].dump();
        } else if (json["outputSchema"].is_string()) {
            function.outputSchema = json["outputSchema"].get<std::string>();
        } else if (!json["outputSchema"].is_null()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: outputSchema has invalid type");
            return false;
        }
    }

    // functionType is optional, default to INTENT_FUNCTION
    function.functionType = FunctionType::INTENT_FUNCTION;
    if (json.contains("functionType")) {
        if (json["functionType"].is_string()) {
            std::string typeStr = json["functionType"].get<std::string>();
            function.functionType = StringToFunctionType(typeStr);
        } else if (json["functionType"].is_number_integer()) {
            function.functionType = static_cast<FunctionType>(json["functionType"].get<int32_t>());
        }
    }

    return true;
}

nlohmann::json FunctionInfo::ParseToJson() const
{
    nlohmann::json j;

    j["functionName"] = functionName;
    j["bundleName"] = bundleName;
    j["description"] = description;

    if (!inputSchema.empty()) {
        nlohmann::json inputSchemaJson = nlohmann::json::parse(inputSchema, nullptr, false);
        if (!inputSchemaJson.is_discarded()) {
            j["inputSchema"] = inputSchemaJson;
        } else {
            j["inputSchema"] = inputSchema;
        }
    }

    if (!outputSchema.empty()) {
        nlohmann::json outputSchemaJson = nlohmann::json::parse(outputSchema, nullptr, false);
        if (!outputSchemaJson.is_discarded()) {
            j["outputSchema"] = outputSchemaJson;
        } else {
            j["outputSchema"] = outputSchema;
        }
    }

    j["functionType"] = FunctionTypeToString(functionType);

    return j;
}

bool FunctionInfo::Validate(const FunctionInfo &function)
{
    // functionName must not be empty
    if (function.functionName.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: functionName is empty");
        return false;
    }

    // bundleName must not be empty
    if (function.bundleName.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: bundleName is empty");
        return false;
    }

    // inputSchema is required and must be valid JSON
    if (function.inputSchema.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: inputSchema is empty");
        return false;
    }
    nlohmann::json inputSchemaJson = nlohmann::json::parse(function.inputSchema, nullptr, false);
    if (inputSchemaJson.is_discarded()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: inputSchema is not valid JSON");
        return false;
    }

    // outputSchema: if not empty, must be valid JSON
    if (!function.outputSchema.empty()) {
        nlohmann::json outputSchemaJson = nlohmann::json::parse(function.outputSchema, nullptr, false);
        if (outputSchemaJson.is_discarded()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: outputSchema is not valid JSON");
            return false;
        }
    }

    return true;
}

std::string FunctionInfo::FunctionTypeToString(FunctionType type)
{
    switch (type) {
        case FunctionType::INTENT_FUNCTION:
            return FUNCTION_TYPE_INTENT_FUNCTION;
        default:
            return FUNCTION_TYPE_INTENT_FUNCTION;
    }
}

FunctionType FunctionInfo::StringToFunctionType(const std::string &typeStr)
{
    if (typeStr == FUNCTION_TYPE_INTENT_FUNCTION) {
        return FunctionType::INTENT_FUNCTION;
    }
    return FunctionType::INTENT_FUNCTION;  // Default to INTENT_FUNCTION
}

} // namespace CliTool
} // namespace OHOS
