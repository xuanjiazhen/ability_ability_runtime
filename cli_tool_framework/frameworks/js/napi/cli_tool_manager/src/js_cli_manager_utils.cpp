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

#include "js_cli_manager_utils.h"

#include "cli_session_info.h"
#include "exec_options.h"
#include "hilog_tag_wrapper.h"
#include "napi_common_util.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace CliTool {
bool UnwrapStringMap(napi_env env, napi_value obj,
    std::map<std::string, std::string> &values)
{
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Input object is null");
        return false;
    }

    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, obj, &valueType);
    if (status != napi_ok || valueType != napi_object) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Input is not an object");
        return false;
    }

    napi_value propertyNames = nullptr;
    if (napi_get_property_names(env, obj, &propertyNames) != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get property names");
        return false;
    }
    uint32_t propertyCount = 0;
    if (napi_get_array_length(env, propertyNames, &propertyCount) != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get array length");
        return false;
    }

    values.clear();
    for (uint32_t i = 0; i < propertyCount; i++) {
        napi_value key = nullptr;
        if (napi_get_element(env, propertyNames, i, &key) != napi_ok) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get element");
            return false;
        }

        std::string keyStr;
        if (!AppExecFwk::UnwrapStringFromJS2(env, key, keyStr)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to unwrap key");
            return false;
        }

        napi_value value = nullptr;
        if (napi_get_named_property(env, obj, keyStr.c_str(), &value) != napi_ok) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get value");
            return false;
        }

        std::string valueStr;
        if (!AppExecFwk::UnwrapStringFromJS2(env, value, valueStr)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to unwrap value");
            return false;
        }

        values.emplace(std::make_pair(keyStr, valueStr));
    }

    return true;
}

bool UnwrapExecOptions(napi_env env, napi_value obj, ExecOptions &options)
{
    if (obj == nullptr) {
        return true;
    }

    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, obj, &valueType);
    if (status != napi_ok || valueType != napi_object) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Input is not an object");
        return false;
    }

    // Extract background (optional)
    napi_value backgroundProp = nullptr;
    if (napi_get_named_property(env, obj, "background", &backgroundProp) != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "invalid background property");
        return false;
    }
    if (!AppExecFwk::UnwrapBoolFromJS2(env, backgroundProp, options.background)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "unwrap background failed");
        return false;
    }

    // Extract yieldMs (optional)
    napi_value yieldMsProp = nullptr;
    if (napi_get_named_property(env, obj, "yieldMs", &yieldMsProp) != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "invalid yieldMs property");
        return false;
    }
    if (!AppExecFwk::UnwrapInt32FromJS2(env, yieldMsProp, options.yieldMs)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "unwrap yieldMs failed");
        return false;
    }

    // Extract timeout (optional)
    napi_value timeoutProp = nullptr;
    if (napi_get_named_property(env, obj, "timeout", &timeoutProp) != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "invalid timeout property");
        return false;
    }
    if (!AppExecFwk::UnwrapInt32FromJS2(env, timeoutProp, options.timeout)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "unwrap timeout failed");
        return false;
    }
    return true;
}

napi_value CreateJsCliSessionInfo(napi_env env, const CliSessionInfo &session)
{
    AbilityRuntime::HandleEscape handleEscape(env);
    napi_value jsObj = nullptr;
    napi_status status = napi_create_object(env, &jsObj);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create JS CliSessionInfo");
        return nullptr;
    }

    napi_set_named_property(env, jsObj, "sessionId", AppExecFwk::WrapStringToJS(env, session.sessionId));
    napi_set_named_property(env, jsObj, "toolName", AppExecFwk::WrapStringToJS(env, session.toolName));
    napi_set_named_property(env, jsObj, "status", AppExecFwk::WrapStringToJS(env, session.status));

    // Set result if present
    if (session.result != nullptr) {
        napi_value jsResult = nullptr;
        status = napi_create_object(env, &jsResult);
        if (status != napi_ok) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create JS ExecResult");
            return nullptr;
        }
        napi_set_named_property(env, jsResult, "exitCode", AppExecFwk::WrapInt32ToJS(env, session.result->exitCode));
        // Set outputText
        napi_value jsOutputText = AppExecFwk::WrapStringToJS(env, session.result->outputText);
        napi_set_named_property(env, jsResult, "outputText", jsOutputText);
        // Set errorText
        napi_set_named_property(env, jsResult, "errorText", AppExecFwk::WrapStringToJS(env, session.result->errorText));
        // Set signalNumber
        napi_value jsSignalNumber = AppExecFwk::WrapInt32ToJS(env, session.result->signalNumber);
        napi_set_named_property(env, jsResult, "signalNumber", jsSignalNumber);
        // Set timedOut
        napi_set_named_property(env, jsResult, "timedOut", AppExecFwk::WrapBoolToJS(env, session.result->timedOut));
        // Set executionTime
        napi_value jsExecutionTime = AppExecFwk::WrapInt64ToJS(env, session.result->executionTime);
        napi_set_named_property(env, jsResult, "executionTime", jsExecutionTime);
        napi_set_named_property(env, jsObj, "result", jsResult);
    }
    return handleEscape.Escape(jsObj);
}

} // namespace CliTool
} // namespace OHOS
