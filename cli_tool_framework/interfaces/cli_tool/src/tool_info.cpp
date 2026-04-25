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

namespace OHOS {
namespace CliTool {

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
    // name是必填字段，同时需要以ohos-或者hms-开头，后面根不超过16个字符的字符串
    if (name.empty()) {
        return false;
    }
    constexpr size_t PREFIX_LEN = 5;  // "ohos-" or "hms-" length
    constexpr size_t MAX_SUFFIX_LEN = 16;
    bool hasValidPrefix = false;
    if (name.length() > PREFIX_LEN) {
        if (name.substr(0, PREFIX_LEN) == "ohos-") {
            hasValidPrefix = true;
        } else if (name.substr(0, PREFIX_LEN) == "hms-") {
            hasValidPrefix = true;
        }
    }
    if (!hasValidPrefix) {
        return false;
    }
    if (name.length() > PREFIX_LEN + MAX_SUFFIX_LEN) {
        return false;
    }

    // version是非空
    if (version.empty()) {
        return false;
    }

    // description也是非空
    if (description.empty()) {
        return false;
    }

    // executablePath非空，是一个linux上的绝对路径
    if (executablePath.empty()) {
        return false;
    }
    if (executablePath[0] != '/') {
        return false;
    }

    // requirePermissions里面的每一项不能重复
    std::set<std::string> permSet;
    for (const auto &perm : requirePermissions) {
        if (permSet.find(perm) != permSet.end()) {
            return false;
        }
        permSet.insert(perm);
    }

    // inputSchema是一个json格式的字符串，且必填
    if (inputSchema.empty()) {
        return false;
    }
    nlohmann::json inputSchemaJson = nlohmann::json::parse(inputSchema, nullptr, false);
    if (inputSchemaJson.is_discarded()) {
        return false;
    }

    // outputSchema是一个json格式的字符串，且必填
    if (outputSchema.empty()) {
        return false;
    }
    nlohmann::json outputSchemaJson = nlohmann::json::parse(outputSchema, nullptr, false);
    if (outputSchemaJson.is_discarded()) {
        return false;
    }

    // eventSchemas是一个json格式的字符串，且必填
    if (eventSchemas.empty()) {
        return false;
    }
    nlohmann::json eventSchemasJson = nlohmann::json::parse(eventSchemas, nullptr, false);
    if (eventSchemasJson.is_discarded()) {
        return false;
    }

    // timeout需要>0 <=300
    if (timeout <= 0 || timeout > 300) {
        return false;
    }

    // argMapping必须要有值
    if (argMapping == nullptr) {
        return false;
    }
    if (!argMapping->Validate()) {
        return false;
    }

    // hasSubCommand为true时，subcommands需要有值；hasSubCommand为false时，subcommands应该为空
    if (hasSubCommand) {
        if (subcommands.empty()) {
            return false;
        }
        // 校验每个subcommand
        for (const auto &pair : subcommands) {
            if (!pair.second.Validate()) {
                return false;
            }
        }
    }

    return true;
}

} // namespace CliTool
} // namespace OHOS