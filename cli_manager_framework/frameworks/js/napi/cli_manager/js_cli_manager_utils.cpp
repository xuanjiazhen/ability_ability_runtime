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

napi_value CreateJsToolInfo(napi_env env, const ToolInfo &info)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_status createStatus = napi_create_object(env, &objValue);
    if (createStatus != napi_ok || objValue == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to create object");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "name", CreateJsValue(env, info.name));
    napi_set_named_property(env, objValue, "version", CreateJsValue(env, info.version));
    napi_set_named_property(env, objValue, "description", CreateJsValue(env, info.description));
    napi_set_named_property(env, objValue, "permissions", CreateNativeArray(env, info.permissions));
    napi_set_named_property(env, objValue, "timeout", CreateJsValue(env, info.timeout));
    napi_set_named_property(env, objValue, "supportedEvents", CreateNativeArray(env, info.supportedEvents));

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
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CreateJsToolInfoArray: array created successfully");

    uint32_t index = 0;
    for (const auto &info : infos) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "CreateJsToolInfoArray: setting element[%{public}u]: %{public}s",
            index, info.name.c_str());
        napi_set_element(env, arrayValue, index++, CreateJsToolInfo(env, info));
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "CreateJsToolInfoArray: completed, returning array");
    return handleEscape.Escape(arrayValue);
}

napi_value CreateJsExecResult(napi_env env, const ExecResult &result)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_status createStatus = napi_create_object(env, &objValue);
    if (createStatus != napi_ok || objValue == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to create object");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "taskId", CreateJsValue(env, result.taskId));
    napi_set_named_property(env, objValue, "exitCode", CreateJsValue(env, result.exitCode));
    napi_set_named_property(env, objValue, "stdout", CreateJsValue(env, result.stdOut));
    napi_set_named_property(env, objValue, "stderr", CreateJsValue(env, result.stdErr));
    napi_set_named_property(env, objValue, "signal", CreateJsValue(env, result.signal));
    napi_set_named_property(env, objValue, "timedOut", CreateJsValue(env, result.timedOut));
    napi_set_named_property(env, objValue, "executionTime", CreateJsValue(env, result.executionTime));

    return handleEscape.Escape(objValue);
}

bool UnwrapStringMap(napi_env env, napi_value jsMap, std::vector<std::pair<std::string, std::string>> &map)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "UnwrapStringMap: called");
    if (!jsMap) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "UnwrapStringMap: jsMap is null");
        return false;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "UnwrapStringMap: jsMap is not null, handling as plain JS object (Record<string, string>)");

    // Get all property names as an array
    napi_value propertyNames = nullptr;
    if (napi_get_property_names(env, jsMap, &propertyNames) != napi_ok) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "UnwrapStringMap: failed to get property names");
        return false;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "UnwrapStringMap: got propertyNames");

    uint32_t length = 0;
    if (napi_get_array_length(env, propertyNames, &length) != napi_ok) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "UnwrapStringMap: failed to get array length");
        return false;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "UnwrapStringMap: propertyNames length=%{public}u", length);

    // Iterate through properties in the order returned by napi_get_property_names
    for (uint32_t i = 0; i < length; i++) {
        napi_value key = nullptr;
        napi_status status = napi_get_element(env, propertyNames, i, &key);
        if (status != napi_ok) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "UnwrapStringMap: failed to get element at index %{public}u", i);
            continue;
        }

        napi_value value = nullptr;
        status = napi_get_property(env, jsMap, key, &value);
        if (status != napi_ok) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "UnwrapStringMap: failed to get property value at index %{public}u", i);
            continue;
        }

        std::string keyStr;
        std::string valueStr;
        if (UnwrapStringFromJS2(env, key, keyStr) && UnwrapStringFromJS2(env, value, valueStr)) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "UnwrapStringMap: [%{public}u] key='%{public}s', value='%{public}s'",
                i, keyStr.c_str(), valueStr.c_str());
            map.push_back({keyStr, valueStr});
        } else {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "UnwrapStringMap: [%{public}u] failed to unwrap key or value", i);
        }
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "UnwrapStringMap: parsed %{public}zu properties", map.size());
    TAG_LOGI(AAFwkTag::ABILITYMGR, "UnwrapStringMap: returning true");
    return true;
}

bool UnwrapExecOptions(napi_env env, napi_value jsOptions, ExecOptions &options)
{
    if (!jsOptions) {
        // Use default values
        options.mode = "sync";
        options.timeout = 30000; // 30 seconds default
        options.cwd = "";
        return true;
    }

    napi_value modeValue = nullptr;
    napi_get_named_property(env, jsOptions, "mode", &modeValue);
    if (modeValue != nullptr) {
        std::string modeStr;
        if (UnwrapStringFromJS2(env, modeValue, modeStr)) {
            options.mode = modeStr;
        } else {
            options.mode = "sync";
        }
    } else {
        options.mode = "sync";
    }

    napi_value timeoutValue = nullptr;
    napi_get_named_property(env, jsOptions, "timeout", &timeoutValue);
    if (timeoutValue != nullptr) {
        napi_get_value_int32(env, timeoutValue, &options.timeout);
    } else {
        options.timeout = 30000;
    }

    napi_value envValue = nullptr;
    napi_get_named_property(env, jsOptions, "env", &envValue);
    if (envValue != nullptr) {
        // TODO: env not supported yet, options.env is std::map but we only support vector<pair> now
        TAG_LOGI(AAFwkTag::ABILITYMGR, "UnwrapExecOptions: env field is not supported yet");
    }

    napi_value cwdValue = nullptr;
    napi_get_named_property(env, jsOptions, "cwd", &cwdValue);
    if (cwdValue != nullptr) {
        std::string cwdStr;
        if (UnwrapStringFromJS2(env, cwdValue, cwdStr)) {
            options.cwd = cwdStr;
        }
    }

    return true;
}

} // namespace AbilityRuntime
} // namespace OHOS
