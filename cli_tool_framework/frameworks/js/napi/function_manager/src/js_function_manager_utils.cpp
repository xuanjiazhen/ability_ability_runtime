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

#include "js_function_manager_utils.h"

#include "function_info.h"
#include "hilog_tag_wrapper.h"
#include "napi_common_util.h"
#include "napi_common_want.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace CliTool {

namespace {
constexpr size_t TRACE_ID_MAX_LEN = 256;

// Helper: set string property with null check and error handling
inline bool SetStringProperty(napi_env env, napi_value obj, const char* name, const std::string& value)
{
    napi_value jsValue = AppExecFwk::WrapStringToJS(env, value);
    if (jsValue == nullptr || napi_set_named_property(env, obj, name, jsValue) != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to set %{public}s", name);
        return false;
    }
    return true;
}

// Helper: set generic property with null check and error handling
inline bool SetProperty(napi_env env, napi_value obj, const char* name, napi_value jsValue)
{
    if (jsValue == nullptr || napi_set_named_property(env, obj, name, jsValue) != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to set %{public}s", name);
        return false;
    }
    return true;
}

// Helper: set optional string property
inline bool SetOptionalStringProperty(napi_env env, napi_value obj, const char* name, const std::string& value)
{
    if (value.empty()) {
        return true;
    }
    return SetStringProperty(env, obj, name, value);
}

// A trace identifier must be non-empty, at most 256 characters long and
// consist only of [A-Za-z0-9_-].
inline bool IsValidTraceId(const std::string& value)
{
    if (value.empty() || value.length() > TRACE_ID_MAX_LEN) {
        return false;
    }
    for (char c : value) {
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '_' || c == '-')) {
            return false;
        }
    }
    return true;
}

// Helper: Unwrap an optional trace id (toolCallId/dmSessionId) property from
// JavaScript object. Property absent means not passed and the output keeps the
// default empty string; property present but with an empty or invalid value is
// invalid.
inline bool UnwrapOptionalTraceId(napi_env env, napi_value options, const char* name,
    std::string& out, std::string& msg)
{
    bool hasProperty = false;
    if (napi_has_named_property(env, options, name, &hasProperty) != napi_ok) {
        msg = std::string("has ") + name + " failed";
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
        return false;
    }
    if (!hasProperty) {
        return true;
    }
    napi_value prop = nullptr;
    if (napi_get_named_property(env, options, name, &prop) != napi_ok) {
        msg = std::string("invalid ") + name + " property";
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
        return false;
    }
    // Explicit undefined/null is treated as absent (spec AC-1.7: "not provided").
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(env, prop, &valueType) != napi_ok) {
        msg = std::string("invalid ") + name + " property";
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
        return false;
    }
    if (valueType == napi_undefined || valueType == napi_null) {
        return true;
    }
    std::string traceId;
    if (!AppExecFwk::UnwrapStringFromJS2(env, prop, traceId)) {
        msg = std::string("Parameter error. The type of \"") + name + "\" must be string.";
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
        return false;
    }
    if (!IsValidTraceId(traceId)) {
        msg = std::string("Parameter error. The ") + name + " is invalid.";
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
        return false;
    }
    out = std::move(traceId);
    return true;
}
} // namespace

napi_value CreateJsFunctionInfo(napi_env env, const FunctionInfo& function)
{
    napi_value jsObj = nullptr;
    napi_status status = napi_create_object(env, &jsObj);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create JS FunctionInfo object");
        return nullptr;
    }

    if (!SetStringProperty(env, jsObj, "functionNamespace", function.functionNamespace) ||
        !SetStringProperty(env, jsObj, "functionName", function.functionName) ||
        !SetStringProperty(env, jsObj, "version", function.version) ||
        !SetStringProperty(env, jsObj, "description", function.description) ||
        !SetOptionalStringProperty(env, jsObj, "inputSchema", function.inputSchema) ||
        !SetOptionalStringProperty(env, jsObj, "outputSchema", function.outputSchema)) {
        return nullptr;
    }

    return jsObj;
}

napi_value CreateJsInvokeResult(napi_env env, int32_t resultCode,
    const std::shared_ptr<AAFwk::WantParams>& result, const std::string& message,
    const std::string& toolCallId, const std::string& dmSessionId)
{
    napi_value jsObj = nullptr;
    napi_status status = napi_create_object(env, &jsObj);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create JS InvokeResult");
        return nullptr;
    }

    napi_value jsSuccess = AppExecFwk::WrapBoolToJS(env, resultCode == 0);
    if (!SetProperty(env, jsObj, "success", jsSuccess)) {
        return nullptr;
    }

    if (result != nullptr) {
        napi_value jsData = AppExecFwk::CreateJsWantParams(env, *result);
        if (jsData != nullptr && !SetProperty(env, jsObj, "data", jsData)) {
            return nullptr;
        }
    }

    napi_value jsErrorCode = AppExecFwk::WrapInt32ToJS(env, resultCode);
    napi_value jsMessage = AppExecFwk::WrapStringToJS(env, message);
    if (!SetProperty(env, jsObj, "errorCode", jsErrorCode) ||
        !SetProperty(env, jsObj, "message", jsMessage)) {
        return nullptr;
    }

    // Echo the trace identifiers passed by the caller; identifiers that were
    // not passed (empty) are not exposed on the JS object.
    if (!SetOptionalStringProperty(env, jsObj, "toolCallId", toolCallId) ||
        !SetOptionalStringProperty(env, jsObj, "dmSessionId", dmSessionId)) {
        return nullptr;
    }

    return jsObj;
}

bool UnwrapInvokeOptions(napi_env env, napi_value options,
    std::string& toolCallId, std::string& dmSessionId, std::string& msg)
{
    if (options == nullptr) {
        return true;
    }
    napi_valuetype optionsType = napi_undefined;
    napi_status status = napi_typeof(env, options, &optionsType);
    if (status != napi_ok) {
        msg = "Parameter error. Failed to parse the option.";
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
        return false;
    }
    if (optionsType == napi_undefined || optionsType == napi_null) {
        return true;  // options absent: no identifiers to parse
    }
    if (optionsType != napi_object) {
        msg = "Parameter error. The type of \"option\" must be object.";
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
        return false;
    }
    return UnwrapOptionalTraceId(env, options, "toolCallId", toolCallId, msg) &&
        UnwrapOptionalTraceId(env, options, "dmSessionId", dmSessionId, msg);
}

} // namespace CliTool
} // namespace OHOS
