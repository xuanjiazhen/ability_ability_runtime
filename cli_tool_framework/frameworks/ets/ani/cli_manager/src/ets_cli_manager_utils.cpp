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

#include "ets_cli_manager_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

napi_value CreateEtsToolInfo(napi_env env, const ToolInfo &tool)
{
    napi_value jsTool = nullptr;
    napi_create_object(env, &jsTool);

    napi_value name = nullptr;
    napi_create_string_utf8(env, tool.name.c_str(), tool.name.length(), &name);
    napi_set_named_property(env, jsTool, "name", name);

    napi_value version = nullptr;
    napi_create_string_utf8(env, tool.version.c_str(), tool.version.length(), &version);
    napi_set_named_property(env, jsTool, "version", version);

    napi_value description = nullptr;
    napi_create_string_utf8(env, tool.description.c_str(), tool.description.length(), &description);
    napi_set_named_property(env, jsTool, "description", description);

    napi_value executablePath = nullptr;
    napi_create_string_utf8(env, tool.executablePath.c_str(), tool.executablePath.length(), &executablePath);
    napi_set_named_property(env, jsTool, "executablePath", executablePath);

    napi_value permissions = nullptr;
    napi_create_array_with_length(env, tool.permissions.size(), &permissions);
    for (size_t i = 0; i < tool.permissions.size(); i++) {
        napi_value perm = nullptr;
        napi_create_string_utf8(env, tool.permissions[i].c_str(), tool.permissions[i].length(), &perm);
        napi_set_element(env, permissions, i, perm);
    }
    napi_set_named_property(env, jsTool, "permissions", permissions);

    napi_value inputSchema = nullptr;
    napi_create_string_utf8(env, tool.inputSchema.c_str(), tool.inputSchema.length(), &inputSchema);
    napi_set_named_property(env, jsTool, "inputSchema", inputSchema);

    napi_value outputSchema = nullptr;
    napi_create_string_utf8(env, tool.outputSchema.c_str(), tool.outputSchema.length(), &outputSchema);
    napi_set_named_property(env, jsTool, "outputSchema", outputSchema);

    napi_value argMapping = nullptr;
    napi_create_string_utf8(env, tool.argMapping.c_str(), tool.argMapping.length(), &argMapping);
    napi_set_named_property(env, jsTool, "argMapping", argMapping);

    napi_value eventSchemas = nullptr;
    napi_create_string_utf8(env, tool.eventSchemas.c_str(), tool.eventSchemas.length(), &eventSchemas);
    napi_set_named_property(env, jsTool, "eventSchemas", eventSchemas);

    napi_value timeout = nullptr;
    napi_create_int32(env, tool.timeout, &timeout);
    napi_set_named_property(env, jsTool, "timeout", timeout);

    napi_value eventTypes = nullptr;
    napi_create_array_with_length(env, tool.eventTypes.size(), &eventTypes);
    for (size_t i = 0; i < tool.eventTypes.size(); i++) {
        napi_value eventType = nullptr;
        napi_create_string_utf8(env, tool.eventTypes[i].c_str(), tool.eventTypes[i].length(), &eventType);
        napi_set_element(env, eventTypes, i, eventType);
    }
    napi_set_named_property(env, jsTool, "eventTypes", eventTypes);

    napi_value registeredTime = nullptr;
    napi_create_int64(env, tool.registeredTime, &registeredTime);
    napi_set_named_property(env, jsTool, "registeredTime", registeredTime);

    napi_value enabled = nullptr;
    napi_get_boolean(env, tool.enabled, &enabled);
    napi_set_named_property(env, jsTool, "enabled", enabled);

    napi_value hasSubcommands = nullptr;
    napi_get_boolean(env, tool.hasSubcommands, &hasSubcommands);
    napi_set_named_property(env, jsTool, "hasSubcommands", hasSubcommands);

    napi_value subcommands = nullptr;
    napi_create_string_utf8(env, tool.subcommands.c_str(), tool.subcommands.length(), &subcommands);
    napi_set_named_property(env, jsTool, "subcommands", subcommands);

    return jsTool;
}

napi_value CreateEtsToolInfoArray(napi_env env, const std::vector<ToolInfo> &tools)
{
    napi_value jsTools = nullptr;
    napi_create_array_with_length(env, tools.size(), &jsTools);

    for (size_t i = 0; i < tools.size(); i++) {
        napi_value jsTool = CreateEtsToolInfo(env, tools[i]);
        napi_set_element(env, jsTools, i, jsTool);
    }

    return jsTools;
}

napi_value CreateEtsToolSummary(napi_env env, const ToolSummary &summary)
{
    napi_value jsSummary = nullptr;
    napi_create_object(env, &jsSummary);

    napi_value name = nullptr;
    napi_create_string_utf8(env, summary.name.c_str(), summary.name.length(), &name);
    napi_set_named_property(env, jsSummary, "name", name);

    napi_value version = nullptr;
    napi_create_string_utf8(env, summary.version.c_str(), summary.version.length(), &version);
    napi_set_named_property(env, jsSummary, "version", version);

    napi_value description = nullptr;
    napi_create_string_utf8(env, summary.description.c_str(), summary.description.length(), &description);
    napi_set_named_property(env, jsSummary, "description", description);

    return jsSummary;
}

napi_value CreateEtsToolSummaryArray(napi_env env, const std::vector<ToolSummary> &summaries)
{
    napi_value jsSummaries = nullptr;
    napi_create_array_with_length(env, summaries.size(), &jsSummaries);

    for (size_t i = 0; i < summaries.size(); i++) {
        napi_value jsSummary = CreateEtsToolSummary(env, summaries[i]);
        napi_set_element(env, jsSummaries, i, jsSummary);
    }

    return jsSummaries;
}

bool ParseEtsToolInfo(napi_env env, napi_value etsTool, ToolInfo &tool)
{
    if (etsTool == nullptr) {
        return false;
    }

    napi_value name = nullptr;
    if (napi_get_named_property(env, etsTool, "name", &name) == napi_ok && name != nullptr) {
        napi_get_value_string_utf8(env, name, tool.name);
    }

    napi_value version = nullptr;
    if (napi_get_named_property(env, etsTool, "version", &version) == napi_ok && version != nullptr) {
        napi_get_value_string_utf8(env, version, tool.version);
    }

    napi_value description = nullptr;
    if (napi_get_named_property(env, etsTool, "description", &description) == napi_ok && description != nullptr) {
        napi_get_value_string_utf8(env, description, tool.description);
    }

    napi_value executablePath = nullptr;
    if (napi_get_named_property(env, etsTool, "executablePath", &executablePath) == napi_ok && executablePath != nullptr) {
        napi_get_value_string_utf8(env, executablePath, tool.executablePath);
    }

    napi_value inputSchema = nullptr;
    if (napi_get_named_property(env, etsTool, "inputSchema", &inputSchema) == napi_ok && inputSchema != nullptr) {
        napi_get_value_string_utf8(env, inputSchema, tool.inputSchema);
    }

    napi_value outputSchema = nullptr;
    if (napi_get_named_property(env, etsTool, "outputSchema", &outputSchema) == napi_ok && outputSchema != nullptr) {
        napi_get_value_string_utf8(env, outputSchema, tool.outputSchema);
    }

    napi_value argMapping = nullptr;
    if (napi_get_named_property(env, etsTool, "argMapping", &argMapping) == napi_ok && argMapping != nullptr) {
        napi_get_value_string_utf8(env, argMapping, tool.argMapping);
    }

    napi_value eventSchemas = nullptr;
    if (napi_get_named_property(env, etsTool, "eventSchemas", &eventSchemas) == napi_ok && eventSchemas != nullptr) {
        napi_get_value_string_utf8(env, eventSchemas, tool.eventSchemas);
    }

    napi_value timeout = nullptr;
    if (napi_get_named_property(env, etsTool, "timeout", &timeout) == napi_ok && timeout != nullptr) {
        napi_get_value_int32(env, timeout, &tool.timeout);
    }

    napi_value enabled = nullptr;
    if (napi_get_named_property(env, etsTool, "enabled", &enabled) == napi_ok && enabled != nullptr) {
        napi_get_value_bool(env, enabled, &tool.enabled);
    }

    napi_value hasSubcommands = nullptr;
    if (napi_get_named_property(env, etsTool, "hasSubcommands", &hasSubcommands) == napi_ok && hasSubcommands != nullptr) {
        napi_get_value_bool(env, hasSubcommands, &tool.hasSubcommands);
    }

    napi_value subcommands = nullptr;
    if (napi_get_named_property(env, etsTool, "subcommands", &subcommands) == napi_ok && subcommands != nullptr) {
        napi_get_value_string_utf8(env, subcommands, tool.subcommands);
    }

    return true;
}

} // namespace CliTool
} // namespace OHOS
