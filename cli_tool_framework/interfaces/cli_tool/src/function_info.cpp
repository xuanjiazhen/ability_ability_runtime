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

bool ParseRequiredStringField(const nlohmann::json &json, const std::string &fieldName,
    std::string &output, bool allowEmpty = false)
{
    if (!json.contains(fieldName) || !json[fieldName].is_string()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: %{public}s is missing or not a string", fieldName.c_str());
        return false;
    }
    output = json[fieldName].get<std::string>();
    if (!allowEmpty && output.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: %{public}s is empty", fieldName.c_str());
        return false;
    }
    return true;
}

bool ParseInputSchema(const nlohmann::json &json, std::string &output)
{
    if (!json.contains("inputSchema") || !json["inputSchema"].is_object()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: inputSchema is missing or not an object");
        return false;
    }
    output = json["inputSchema"].dump();
    return true;
}

bool ParseOutputSchema(const nlohmann::json &json, std::string &output)
{
    if (!json.contains("outputSchema")) {
        return true;  // optional
    }
    const auto &outputSchema = json["outputSchema"];
    if (outputSchema.is_object()) {
        output = outputSchema.dump();
    } else if (outputSchema.is_string()) {
        output = outputSchema.get<std::string>();
    } else if (!outputSchema.is_null()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: outputSchema has invalid type");
        return false;
    }
    return true;
}

void ParseFunctionType(const nlohmann::json &json, FunctionType &output)
{
    output = FunctionType::INTENT_FUNCTION;  // default
    if (!json.contains("functionType")) {
        return;
    }
    const auto &functionType = json["functionType"];
    if (functionType.is_string()) {
        std::string typeStr = functionType.get<std::string>();
        output = FunctionInfo::StringToFunctionType(typeStr);
    } else if (functionType.is_number_integer()) {
        output = static_cast<FunctionType>(functionType.get<int32_t>());
    }
}

} // namespace

bool FunctionInfo::Marshalling(Parcel &parcel) const
{
    int32_t typeValue = static_cast<int32_t>(functionType);
    if (!parcel.WriteString(functionName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write functionName failed");
        return false;
    }
    if (!parcel.WriteString(namespace)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write namespace failed");
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
    if (!parcel.ReadString(function->funcNamespace)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read namespace failed");
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
    return ParseRequiredStringField(json, "functionName", function.functionName) &&
           ParseRequiredStringField(json, "namespace", function.funcNamespace) &&
           ParseRequiredStringField(json, "description", function.description, true) &&
           ParseInputSchema(json, function.inputSchema) &&
           ParseOutputSchema(json, function.outputSchema) &&
           (ParseFunctionType(json, function.functionType), true);
}

nlohmann::json FunctionInfo::ParseToJson() const
{
    nlohmann::json j;

    j["functionName"] = functionName;
    j["namespace"] = namespace;
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

    // namespace must not be empty
    if (function.funcNamespace.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: namespace is empty");
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
