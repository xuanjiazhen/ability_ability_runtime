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

#include "js_cli_manager_utils.h"

#include <cstdint>

#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_util.h"

namespace OHOS {
namespace AbilityRuntime {

using namespace OHOS::AppExecFwk;

// CreateJsToolSummary
napi_value CreateJsToolSummary(napi_env env, const ToolSummary &summary)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_status createStatus = napi_create_object(env, &objValue);
    if (createStatus != napi_ok || objValue == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to create ToolSummary object");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "name", CreateJsValue(env, summary.name));
    napi_set_named_property(env, objValue, "version", CreateJsValue(env, summary.version));
    napi_set_named_property(env, objValue, "description", CreateJsValue(env, summary.description));

    return handleEscape.Escape(objValue);
}

napi_value CreateJsToolSummaryArray(napi_env env, const std::vector<ToolSummary> &summaries)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CreateJsToolSummaryArray: creating array with %{public}zu summaries", summaries.size());

    HandleEscape handleEscape(env);
    napi_value arrayValue = nullptr;
    napi_status createStatus = napi_create_array_with_length(env, summaries.size(), &arrayValue);
    if (createStatus != napi_ok || arrayValue == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to create array, status=%{public}d", createStatus);
        return nullptr;
    }

    uint32_t index = 0;
    for (const auto &summary : summaries) {
        napi_set_element(env, arrayValue, index++, CreateJsToolSummary(env, summary));
    }

    return handleEscape.Escape(arrayValue);
}

// CreateJsToolInfo
napi_value CreateJsToolInfo(napi_env env, const ToolInfo &info)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_status createStatus = napi_create_object(env, &objValue);
    if (createStatus != napi_ok || objValue == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to create ToolInfo object");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "name", CreateJsValue(env, info.name));
    napi_set_named_property(env, objValue, "version", CreateJsValue(env, info.version));
    napi_set_named_property(env, objValue, "description", CreateJsValue(env, info.description));
    napi_set_named_property(env, objValue, "executablePath", CreateJsValue(env, info.executablePath));
    napi_set_named_property(env, objValue, "permissions", CreateNativeArray(env, info.permissions));
    napi_set_named_property(env, objValue, "timeout", CreateJsValue(env, info.timeout));
    napi_set_named_property(env, objValue, "eventTypes", CreateNativeArray(env, info.eventTypes));
    napi_set_named_property(env, objValue, "registeredTime", CreateJsValue(env, info.registeredTime));
    napi_set_named_property(env, objValue, "enabled", CreateJsValue(env, info.enabled));
    napi_set_named_property(env, objValue, "hasSubcommands", CreateJsValue(env, info.hasSubcommands));

    // Parse JSON strings for schemas - use global JSON object
    napi_value global = nullptr;
    napi_get_global(env, &global);
    napi_value jsonObj = nullptr;
    napi_get_named_property(env, global, "JSON", &jsonObj);

    if (jsonObj != nullptr) {
        if (!info.inputSchema.empty()) {
            napi_value parseFunc = nullptr;
            napi_get_named_property(env, jsonObj, "parse", &parseFunc);
            if (parseFunc != nullptr) {
                napi_value inputSchemaStr = CreateJsValue(env, info.inputSchema);
                napi_value inputSchema = nullptr;
                napi_call_function(env, jsonObj, parseFunc, 1, &inputSchemaStr, &inputSchema);
                if (inputSchema != nullptr) {
                    napi_set_named_property(env, objValue, "inputSchema", inputSchema);
                }
            }
        }

        if (!info.outputSchema.empty()) {
            napi_value parseFunc = nullptr;
            napi_get_named_property(env, jsonObj, "parse", &parseFunc);
            if (parseFunc != nullptr) {
                napi_value outputSchemaStr = CreateJsValue(env, info.outputSchema);
                napi_value outputSchema = nullptr;
                napi_call_function(env, jsonObj, parseFunc, 1, &outputSchemaStr, &outputSchema);
                if (outputSchema != nullptr) {
                    napi_set_named_property(env, objValue, "outputSchema", outputSchema);
                }
            }
        }

        if (!info.argMapping.empty()) {
            napi_value parseFunc = nullptr;
            napi_get_named_property(env, jsonObj, "parse", &parseFunc);
            if (parseFunc != nullptr) {
                napi_value argMappingStr = CreateJsValue(env, info.argMapping);
                napi_value argMapping = nullptr;
                napi_call_function(env, jsonObj, parseFunc, 1, &argMappingStr, &argMapping);
                if (argMapping != nullptr) {
                    napi_set_named_property(env, objValue, "argMapping", argMapping);
                }
            }
        }

        if (!info.eventSchemas.empty()) {
            napi_value parseFunc = nullptr;
            napi_get_named_property(env, jsonObj, "parse", &parseFunc);
            if (parseFunc != nullptr) {
                napi_value eventSchemasStr = CreateJsValue(env, info.eventSchemas);
                napi_value eventSchemas = nullptr;
                napi_call_function(env, jsonObj, parseFunc, 1, &eventSchemasStr, &eventSchemas);
                if (eventSchemas != nullptr) {
                    napi_set_named_property(env, objValue, "eventSchemas", eventSchemas);
                }
            }
        }

        if (!info.subcommands.empty()) {
            napi_value parseFunc = nullptr;
            napi_get_named_property(env, jsonObj, "parse", &parseFunc);
            if (parseFunc != nullptr) {
                napi_value subcommandsStr = CreateJsValue(env, info.subcommands);
                napi_value subcommands = nullptr;
                napi_call_function(env, jsonObj, parseFunc, 1, &subcommandsStr, &subcommands);
                if (subcommands != nullptr) {
                    napi_set_named_property(env, objValue, "subcommands", subcommands);
                }
            }
        }
    }

    return handleEscape.Escape(objValue);
}

napi_value CreateJsToolInfoArray(napi_env env, const std::vector<ToolInfo> &infos)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CreateJsToolInfoArray: creating array with %{public}zu tools", infos.size());

    HandleEscape handleEscape(env);
    napi_value arrayValue = nullptr;
    napi_status createStatus = napi_create_array_with_length(env, infos.size(), &arrayValue);
    if (createStatus != napi_ok || arrayValue == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to create array, status=%{public}d", createStatus);
        return nullptr;
    }

    uint32_t index = 0;
    for (const auto &info : infos) {
        napi_set_element(env, arrayValue, index++, CreateJsToolInfo(env, info));
    }

    return handleEscape.Escape(arrayValue);
}

// CreateJsExecResult
napi_value CreateJsExecResult(napi_env env, const ExecResult &result)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_status createStatus = napi_create_object(env, &objValue);
    if (createStatus != napi_ok || objValue == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to create ExecResult object");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "exitCode", CreateJsValue(env, result.exitCode));
    napi_set_named_property(env, objValue, "outputText", CreateJsValue(env, result.outputText));
    napi_set_named_property(env, objValue, "errorText", CreateJsValue(env, result.errorText));
    napi_set_named_property(env, objValue, "signalNumber", CreateJsValue(env, result.signalNumber));
    napi_set_named_property(env, objValue, "timedOut", CreateJsValue(env, result.timedOut));
    napi_set_named_property(env, objValue, "executionTime", CreateJsValue(env, result.executionTime));

    return handleEscape.Escape(objValue);
}

// CreateJsSessionInfo
napi_value CreateJsSessionInfo(napi_env env, const SessionInfo &session)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CreateJsSessionInfo: sessionId='%{public}s', status='%{public}s'",
        session.sessionId.c_str(), session.status.c_str());

    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_status createStatus = napi_create_object(env, &objValue);
    if (createStatus != napi_ok || objValue == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CreateJsSessionInfo: Failed to create object, status=%{public}d", createStatus);
        return nullptr;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CreateJsSessionInfo: Object created successfully");

    // Set sessionId
    napi_value sessionIdValue = CreateJsValue(env, session.sessionId);
    napi_set_named_property(env, objValue, "sessionId", sessionIdValue);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CreateJsSessionInfo: Set sessionId='%{public}s'", session.sessionId.c_str());

    // Set toolName
    napi_value toolNameValue = CreateJsValue(env, session.toolName);
    napi_set_named_property(env, objValue, "toolName", toolNameValue);

    // Set status
    napi_value statusValue = CreateJsValue(env, session.status);
    napi_set_named_property(env, objValue, "status", statusValue);

    // Set startTime
    napi_value startTimeValue = CreateJsValue(env, session.startTime);
    napi_set_named_property(env, objValue, "startTime", startTimeValue);

    // Set endTime
    napi_value endTimeValue = CreateJsValue(env, session.endTime);
    napi_set_named_property(env, objValue, "endTime", endTimeValue);

    // Set result if exists
    if (session.result != nullptr) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "CreateJsSessionInfo: Creating result object");
        napi_value resultValue = CreateJsExecResult(env, *session.result);
        napi_set_named_property(env, objValue, "result", resultValue);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "CreateJsSessionInfo: Result object set");
    } else {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "CreateJsSessionInfo: No result (status='%{public}s')", session.status.c_str());
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "CreateJsSessionInfo: All properties set, returning object");
    return handleEscape.Escape(objValue);
}

// CreateJsSessionStatus
napi_value CreateJsSessionStatus(napi_env env, const SessionStatus &status)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_status createStatus = napi_create_object(env, &objValue);
    if (createStatus != napi_ok || objValue == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to create SessionStatus object");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "sessionId", CreateJsValue(env, status.sessionId));
    napi_set_named_property(env, objValue, "toolName", CreateJsValue(env, status.toolName));
    napi_set_named_property(env, objValue, "status", CreateJsValue(env, status.status));
    napi_set_named_property(env, objValue, "startTime", CreateJsValue(env, status.startTime));
    napi_set_named_property(env, objValue, "endTime", CreateJsValue(env, status.endTime));

    if (status.result != nullptr) {
        napi_value resultValue = CreateJsExecResult(env, *status.result);
        napi_set_named_property(env, objValue, "result", resultValue);
    }

    return handleEscape.Escape(objValue);
}

// CreateJsToolEvent
napi_value CreateJsToolEvent(napi_env env, const ToolEvent &event)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_status createStatus = napi_create_object(env, &objValue);
    if (createStatus != napi_ok || objValue == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to create ToolEvent object");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "type", CreateJsValue(env, event.type));
    napi_set_named_property(env, objValue, "eventData", CreateJsValue(env, event.data));
    napi_set_named_property(env, objValue, "timestamp", CreateJsValue(env, event.timestamp));

    // exitCode is only present for 'exit' type events
    if (event.type == "exit") {
        napi_set_named_property(env, objValue, "exitCode", CreateJsValue(env, event.exitCode));
    }

    return handleEscape.Escape(objValue);
}

// UnwrapStringMap
bool UnwrapStringMap(napi_env env, napi_value jsMap, std::vector<std::pair<std::string, std::string>> &map)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "UnwrapStringMap: called");
    if (!jsMap) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "UnwrapStringMap: jsMap is null");
        return false;
    }

    // Get all property names as an array
    napi_value propertyNames = nullptr;
    if (napi_get_property_names(env, jsMap, &propertyNames) != napi_ok) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "UnwrapStringMap: failed to get property names");
        return false;
    }

    uint32_t length = 0;
    if (napi_get_array_length(env, propertyNames, &length) != napi_ok) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "UnwrapStringMap: failed to get array length");
        return false;
    }

    // Iterate through properties in insertion order
    for (uint32_t i = 0; i < length; i++) {
        napi_value key = nullptr;
        napi_status status = napi_get_element(env, propertyNames, i, &key);
        if (status != napi_ok) {
            continue;
        }

        napi_value value = nullptr;
        status = napi_get_property(env, jsMap, key, &value);
        if (status != napi_ok) {
            continue;
        }

        std::string keyStr;
        std::string valueStr;
        if (UnwrapStringFromJS2(env, key, keyStr) && UnwrapStringFromJS2(env, value, valueStr)) {
            map.push_back({keyStr, valueStr});
        }
    }

    return true;
}

// UnwrapExecOptions
bool UnwrapExecOptions(napi_env env, napi_value jsOptions, ExecOptions &options)
{
    if (!jsOptions) {
        // Use default values
        options.background = false;
        options.yieldMs = 30000; // 30 seconds default
        options.timeout = 0;    // 0 means use tool default
        options.workingDir = "";
        return true;
    }

    // Parse background
    napi_value backgroundValue = nullptr;
    napi_get_named_property(env, jsOptions, "background", &backgroundValue);
    if (backgroundValue != nullptr) {
        napi_get_value_bool(env, backgroundValue, &options.background);
    } else {
        options.background = false;
    }

    // Parse yieldMs
    napi_value yieldMsValue = nullptr;
    napi_get_named_property(env, jsOptions, "yieldMs", &yieldMsValue);
    if (yieldMsValue != nullptr) {
        napi_get_value_int32(env, yieldMsValue, &options.yieldMs);
    } else {
        options.yieldMs = 30000;
    }

    // Parse timeout
    napi_value timeoutValue = nullptr;
    napi_get_named_property(env, jsOptions, "timeout", &timeoutValue);
    napi_valuetype valueType;
    napi_typeof(env, timeoutValue, &valueType);
    if (valueType == napi_number) {
        napi_get_value_int32(env, timeoutValue, &options.timeout);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "UnwrapExecOptions: timeout=%{public}d (from JS)", options.timeout);
    } else {
        options.timeout = 0;
        TAG_LOGI(AAFwkTag::ABILITYMGR, "UnwrapExecOptions: timeout not set, using 0 (default)");
    }

    // Parse workingDir
    napi_value workingDirValue = nullptr;
    napi_get_named_property(env, jsOptions, "workingDir", &workingDirValue);
    if (workingDirValue != nullptr) {
        std::string workingDirStr;
        if (UnwrapStringFromJS2(env, workingDirValue, workingDirStr)) {
            options.workingDir = workingDirStr;
        }
    }

    // Parse env (map<string, string>)
    napi_value envValue = nullptr;
    napi_get_named_property(env, jsOptions, "env", &envValue);
    if (envValue != nullptr) {
        // Get all property names from env object
        napi_value envPropertyNames = nullptr;
        if (napi_get_property_names(env, envValue, &envPropertyNames) == napi_ok) {
            uint32_t envLength = 0;
            if (napi_get_array_length(env, envPropertyNames, &envLength) == napi_ok) {
                for (uint32_t i = 0; i < envLength; i++) {
                    napi_value key = nullptr;
                    if (napi_get_element(env, envPropertyNames, i, &key) == napi_ok) {
                        napi_value value = nullptr;
                        if (napi_get_property(env, envValue, key, &value) == napi_ok) {
                            std::string keyStr, valueStr;
                            if (UnwrapStringFromJS2(env, key, keyStr) && UnwrapStringFromJS2(env, value, valueStr)) {
                                options.env[keyStr] = valueStr;
                            }
                        }
                    }
                }
            }
        }
    }

    return true;
}

} // namespace AbilityRuntime
} // namespace OHOS
