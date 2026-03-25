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
    HandleEscape handleEscape(env);
    napi_value arrayValue = nullptr;
    napi_status createStatus = napi_create_array_with_length(env, infos.size(), &arrayValue);
    if (createStatus != napi_ok || arrayValue == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to create array");
        return nullptr;
    }

    uint32_t index = 0;
    for (const auto &info : infos) {
        napi_set_element(env, arrayValue, index++, CreateJsToolInfo(env, info));
    }

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

bool UnwrapStringMap(napi_env env, napi_value jsMap, std::map<std::string, std::string> &map)
{
    if (!jsMap) {
        return false;
    }

    napi_value global = nullptr;
    napi_get_global(env, &global);

    napi_value object = nullptr;
    napi_status status = napi_get_named_property(env, global, "Object", &object);
    if (status != napi_ok) {
        return false;
    }

    napi_value entries = nullptr;
    napi_value entriesFunc = nullptr;
    napi_get_named_property(env, jsMap, "entries", &entriesFunc);
    if (entriesFunc == nullptr) {
        // Handle as plain object
        napi_value propertyNames = nullptr;
        if (napi_get_property_names(env, jsMap, &propertyNames) != napi_ok) {
            return false;
        }

        uint32_t length = 0;
        napi_get_array_length(env, propertyNames, &length);

        for (uint32_t i = 0; i < length; i++) {
            napi_value key = nullptr;
            napi_get_element(env, propertyNames, i, &key);

            napi_value value = nullptr;
            napi_get_property(env, jsMap, key, &value);

            std::string keyStr;
            std::string valueStr;
            if (UnwrapStringFromJS2(env, key, keyStr) && UnwrapStringFromJS2(env, value, valueStr)) {
                map[keyStr] = valueStr;
            }
        }
    } else {
        // Handle as Map
        napi_call_function(env, jsMap, entriesFunc, 0, nullptr, &entries);

        bool isArray = false;
        napi_is_array(env, entries, &isArray);
        if (!isArray) {
            return false;
        }

        uint32_t length = 0;
        napi_get_array_length(env, entries, &length);

        for (uint32_t i = 0; i < length; i++) {
            napi_value entry = nullptr;
            napi_get_element(env, entries, i, &entry);

            napi_value key = nullptr;
            napi_get_element(env, entry, 0, &key);

            napi_value value = nullptr;
            napi_get_element(env, entry, 1, &value);

            std::string keyStr;
            std::string valueStr;
            if (UnwrapStringFromJS2(env, key, keyStr) && UnwrapStringFromJS2(env, value, valueStr)) {
                map[keyStr] = valueStr;
            }
        }
    }

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
        UnwrapStringMap(env, envValue, options.env);
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
