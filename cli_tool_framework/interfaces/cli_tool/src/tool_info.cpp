/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

#include "tool_info.h"

#include <nlohmann/json.hpp>
#include <set>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

namespace {
constexpr size_t NAME_MAX_SUFFIX_LEN = 16;
constexpr int32_t TIMEOUT_MIN = 1;
constexpr int32_t TIMEOUT_MAX = 300;

bool IsValidName(const std::string &name)
{
    if (name.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "name is empty");
        return false;
    }
    std::string prefix;
    size_t suffixStart = 0;
    if (name.compare(0, 5, "ohos-") == 0) {
        prefix = "ohos-";
        suffixStart = 5;
    } else if (name.compare(0, 4, "hms-") == 0) {
        prefix = "hms-";
        suffixStart = 4;
    } else {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "name has invalid prefix: %{public}s", name.c_str());
        return false;
    }
    if (name.length() <= suffixStart) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "name too short: %{public}s", name.c_str());
        return false;
    }
    if (name.length() > suffixStart + NAME_MAX_SUFFIX_LEN) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "name too long: %{public}s", name.c_str());
        return false;
    }
    return true;
}

bool IsValidJsonSchema(const std::string &schema, const char *fieldName)
{
    if (schema.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s is empty", fieldName);
        return false;
    }
    nlohmann::json json = nlohmann::json::parse(schema, nullptr, false);
    if (json.is_discarded()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s is not valid JSON", fieldName);
        return false;
    }
    return true;
}

bool HasDuplicatePermissions(const std::vector<std::string> &permissions)
{
    std::set<std::string> permSet;
    for (const auto &perm : permissions) {
        if (permSet.find(perm) != permSet.end()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "duplicate permission: %{public}s", perm.c_str());
            return true;
        }
        permSet.insert(perm);
    }
    return false;
}

bool IsValidBasicFields(const ToolInfo &tool)
{
    if (!IsValidName(tool.name)) {
        return false;
    }
    if (tool.version.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "version is empty");
        return false;
    }
    if (tool.description.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "description is empty");
        return false;
    }
    if (tool.executablePath.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "executablePath is empty");
        return false;
    }
    if (tool.executablePath[0] != '/') {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "executablePath is not absolute path");
        return false;
    }
    return true;
}

bool IsValidSchemas(const ToolInfo &tool)
{
    if (!IsValidJsonSchema(tool.inputSchema, "inputSchema")) {
        return false;
    }
    if (!IsValidJsonSchema(tool.outputSchema, "outputSchema")) {
        return false;
    }
    if (!IsValidJsonSchema(tool.eventSchemas, "eventSchemas")) {
        return false;
    }
    return true;
}

bool IsValidSubCommands(const ToolInfo &tool)
{
    if (!tool.hasSubCommand) {
        return true;
    }
    if (tool.subcommands.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "hasSubCommand is true but subcommands is empty");
        return false;
    }
    for (const auto &pair : tool.subcommands) {
        if (!pair.second.Validate()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "subcommand %{public}s validation failed", pair.first.c_str());
            return false;
        }
    }
    return true;
}
} // namespace

// ToolInfo implementation
bool ToolInfo::Marshalling(Parcel &parcel) const
{
    // Serialize subcommands map to JSON string
    std::string subcommandsJson;
    if (!subcommands.empty()) {
        nlohmann::json j;
        for (const auto &pair : subcommands) {
            j[pair.first] = pair.second.ParseToJson();
        }
        subcommandsJson = j.dump();
    }

    return parcel.WriteString(name) &&
           parcel.WriteString(version) &&
           parcel.WriteString(description) &&
           parcel.WriteString(executablePath) &&
           parcel.WriteStringVector(requirePermissions) &&
           parcel.WriteString(inputSchema) &&
           parcel.WriteString(outputSchema) &&
           parcel.WriteBool(argMapping != nullptr) &&
           (argMapping == nullptr || argMapping->Marshalling(parcel)) &&
           parcel.WriteString(eventSchemas) &&
           parcel.WriteInt32(timeout) &&
           parcel.WriteStringVector(eventTypes) &&
           parcel.WriteBool(hasSubCommand) &&
           parcel.WriteString(subcommandsJson);
}

ToolInfo *ToolInfo::Unmarshalling(Parcel &parcel)
{
    auto *tool = new (std::nothrow) ToolInfo();
    if (tool == nullptr) {
        return nullptr;
    }

    bool hasArgMapping = false;
    std::string subcommandsJson;

    if (!parcel.ReadString(tool->name) ||
        !parcel.ReadString(tool->version) ||
        !parcel.ReadString(tool->description) ||
        !parcel.ReadString(tool->executablePath) ||
        !parcel.ReadStringVector(&tool->requirePermissions) ||
        !parcel.ReadString(tool->inputSchema) ||
        !parcel.ReadString(tool->outputSchema) ||
        !parcel.ReadBool(hasArgMapping)) {
        delete tool;
        return nullptr;
    }

    if (hasArgMapping) {
        tool->argMapping.reset(ArgMapping::Unmarshalling(parcel));
        if (tool->argMapping == nullptr) {
            delete tool;
            return nullptr;
        }
    }

    if (!parcel.ReadString(tool->eventSchemas) ||
        !parcel.ReadInt32(tool->timeout) ||
        !parcel.ReadStringVector(&tool->eventTypes) ||
        !parcel.ReadBool(tool->hasSubCommand) ||
        !parcel.ReadString(subcommandsJson)) {
        delete tool;
        return nullptr;
    }

    // Parse subcommands JSON string to map
    if (!subcommandsJson.empty()) {
        nlohmann::json j = nlohmann::json::parse(subcommandsJson, nullptr, false);
        if (!j.is_discarded() && j.is_object()) {
            for (auto it = j.begin(); it != j.end(); ++it) {
                tool->subcommands[it.key()] = SubCommandInfo::ParseFromJson(it.value());
            }
        }
    }

    return tool;
}

ToolInfo ToolInfo::ParseFromJson(const nlohmann::json &json)
{
    ToolInfo tool;

    if (json.contains("name") && json["name"].is_string()) {
        tool.name = json["name"];
    }
    if (json.contains("version") && json["version"].is_string()) {
        tool.version = json["version"];
    }
    if (json.contains("description") && json["description"].is_string()) {
        tool.description = json["description"];
    }
    if (json.contains("executablePath") && json["executablePath"].is_string()) {
        tool.executablePath = json["executablePath"];
    }
    if (json.contains("requirePermissions") && json["requirePermissions"].is_array()) {
        for (const auto &perm : json["requirePermissions"]) {
            if (perm.is_string()) {
                tool.requirePermissions.push_back(perm);
            }
        }
    }
    if (json.contains("inputSchema") && json["inputSchema"].is_object()) {
        tool.inputSchema = json["inputSchema"].dump();
    }
    if (json.contains("outputSchema") && json["outputSchema"].is_object()) {
        tool.outputSchema = json["outputSchema"].dump();
    }
    if (json.contains("argMapping") && json["argMapping"].is_object()) {
        tool.argMapping = ArgMapping::ParseFromJson(json["argMapping"]);
    }
    if (json.contains("eventSchemas") && json["eventSchemas"].is_object()) {
        tool.eventSchemas = json["eventSchemas"].dump();
    }
    if (json.contains("timeout") && json["timeout"].is_number()) {
        tool.timeout = json["timeout"];
    }
    if (json.contains("eventTypes") && json["eventTypes"].is_array()) {
        for (const auto &evt : json["eventTypes"]) {
            if (evt.is_string()) {
                tool.eventTypes.push_back(evt);
            }
        }
    }
    if (json.contains("hasSubCommand") && json["hasSubCommand"].is_boolean()) {
        tool.hasSubCommand = json["hasSubCommand"];
    }
    if (tool.hasSubCommand && json.contains("subcommands") && json["subcommands"].is_object()) {
        for (auto it = json["subcommands"].begin(); it != json["subcommands"].end(); ++it) {
            tool.subcommands[it.key()] = SubCommandInfo::ParseFromJson(it.value());
        }
    }

    return tool;
}

nlohmann::json ToolInfo::ParseToJson() const
{
    nlohmann::json j;

    j["name"] = name;
    j["version"] = version;
    j["description"] = description;
    j["executablePath"] = executablePath;
    j["requirePermissions"] = requirePermissions;
    if (!inputSchema.empty()) {
        j["inputSchema"] = nlohmann::json::parse(inputSchema, nullptr, false);
    }
    if (!outputSchema.empty()) {
        j["outputSchema"] = nlohmann::json::parse(outputSchema, nullptr, false);
    }
    if (argMapping != nullptr) {
        j["argMapping"] = argMapping->ParseToJson();
    }
    if (!eventSchemas.empty()) {
        j["eventSchemas"] = nlohmann::json::parse(eventSchemas, nullptr, false);
    }
    j["timeout"] = timeout;
    j["eventTypes"] = eventTypes;
    j["hasSubCommand"] = hasSubCommand;
    if (!subcommands.empty()) {
        nlohmann::json subcommandsJson;
        for (const auto &pair : subcommands) {
            subcommandsJson[pair.first] = pair.second.ParseToJson();
        }
        j["subcommands"] = subcommandsJson;
    }

    return j;
}

bool ToolInfo::Validate() const
{
    if (!IsValidBasicFields(*this)) {
        return false;
    }
    if (HasDuplicatePermissions(requirePermissions)) {
        return false;
    }
    if (!IsValidSchemas(*this)) {
        return false;
    }
    if (timeout < TIMEOUT_MIN || timeout > TIMEOUT_MAX) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "timeout invalid: %{public}d", timeout);
        return false;
    }
    if (argMapping == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "argMapping is null");
        return false;
    }
    if (!argMapping->Validate()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "argMapping validation failed");
        return false;
    }
    if (!IsValidSubCommands(*this)) {
        return false;
    }
    return true;
}

} // namespace CliTool
} // namespace OHOS