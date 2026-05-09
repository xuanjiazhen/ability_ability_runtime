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

#include "js_script_manager.h"

#include <string>
#include <vector>

#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "js_error_utils.h"
#include "napi_common_skill_execute.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "napi_base_context.h"
#include "standalone_skill_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr int32_t INDEX_THREE = 3;
constexpr int32_t ERR_CONTEXT_NOT_ABILITY = 16000020;
constexpr int32_t ARGC_ONE = 1;

bool HasPropertyOfType(napi_env env, napi_value obj, const char *prop)
{
    napi_value value = nullptr;
    napi_get_named_property(env, obj, prop, &value);
    napi_valuetype type = napi_undefined;
    napi_typeof(env, value, &type);
    return type == napi_object;
}

bool VerifyContext(napi_env env, napi_value value)
{
    if (value == nullptr) {
        return false;
    }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType != napi_object) {
        return false;
    }
    return HasPropertyOfType(env, value, "abilityInfo") ||
        HasPropertyOfType(env, value, "extensionAbilityInfo");
}

void ThrowContextNotValidError(napi_env env)
{
    ThrowError(env, ERR_CONTEXT_NOT_ABILITY,
        "The context is not a valid ability or extension context.");
}

std::string ParseRequestCode(napi_env env, napi_value value)
{
    napi_valuetype type = napi_undefined;
    napi_typeof(env, value, &type);
    if (type == napi_string) {
        size_t len = 0;
        napi_get_value_string_utf8(env, value, nullptr, 0, &len);
        std::string result(len, '\0');
        napi_get_value_string_utf8(env, value, result.data(), len + 1, &len);
        return result;
    }
    if (type == napi_number) {
        double val = 0;
        napi_get_value_double(env, value, &val);
        return std::to_string(static_cast<int64_t>(val));
    }
    if (type == napi_bigint) {
        bool lossless = true;
        int64_t requestCode = 0;
        napi_get_value_bigint_int64(env, value, &requestCode, &lossless);
        return std::to_string(requestCode);
    }
    return "";
}

bool GetNamedProperty(napi_env env, napi_value object, const char *name, napi_value &value)
{
    value = nullptr;
    if (env == nullptr || object == nullptr || name == nullptr) {
        return false;
    }
    if (napi_get_named_property(env, object, name, &value) != napi_ok || value == nullptr) {
        return false;
    }
    return true;
}

bool IsNullOrUndefined(napi_env env, napi_value value)
{
    napi_valuetype valueType = napi_undefined;
    if (value == nullptr || napi_typeof(env, value, &valueType) != napi_ok) {
        return true;
    }
    return valueType == napi_undefined || valueType == napi_null;
}

bool GetRequiredInt32Property(napi_env env, napi_value object, const char *name, int32_t &output)
{
    napi_value value = nullptr;
    if (!GetNamedProperty(env, object, name, value) || IsNullOrUndefined(env, value)) {
        return false;
    }
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(env, value, &valueType) != napi_ok || valueType != napi_number) {
        return false;
    }
    return napi_get_value_int32(env, value, &output) == napi_ok;
}

bool GetOptionalUint32Property(napi_env env, napi_value object, const char *name, uint32_t &output, bool &hasValue)
{
    hasValue = false;
    napi_value value = nullptr;
    if (!GetNamedProperty(env, object, name, value) || IsNullOrUndefined(env, value)) {
        return true;
    }
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(env, value, &valueType) != napi_ok || valueType != napi_number) {
        return false;
    }
    if (napi_get_value_uint32(env, value, &output) != napi_ok) {
        return false;
    }
    hasValue = true;
    return true;
}

bool GetOptionalStringArrayProperty(napi_env env, napi_value object, const char *name,
    std::vector<std::string> &output, bool &hasValue)
{
    hasValue = false;
    napi_value value = nullptr;
    if (!GetNamedProperty(env, object, name, value) || IsNullOrUndefined(env, value)) {
        return true;
    }

    bool isArray = false;
    if (napi_is_array(env, value, &isArray) != napi_ok || !isArray) {
        return false;
    }

    uint32_t length = 0;
    if (napi_get_array_length(env, value, &length) != napi_ok) {
        return false;
    }

    std::vector<std::string> uris;
    for (uint32_t i = 0; i < length; i++) {
        napi_value element = nullptr;
        if (napi_get_element(env, value, i, &element) != napi_ok || element == nullptr) {
            return false;
        }
        napi_valuetype valueType = napi_undefined;
        if (napi_typeof(env, element, &valueType) != napi_ok || valueType != napi_string) {
            return false;
        }
        size_t strLen = 0;
        napi_get_value_string_utf8(env, element, nullptr, 0, &strLen);
        std::string uriStr(strLen, '\0');
        napi_get_value_string_utf8(env, element, uriStr.data(), strLen + 1, &strLen);
        uris.emplace_back(std::move(uriStr));
    }
    output = std::move(uris);
    hasValue = true;
    return true;
}

std::string StringifyObject(napi_env env, napi_value object)
{
    napi_value global = nullptr;
    if (napi_get_global(env, &global) != napi_ok || global == nullptr) {
        return "";
    }
    napi_value jsonObj = nullptr;
    if (napi_get_named_property(env, global, "JSON", &jsonObj) != napi_ok || jsonObj == nullptr) {
        return "";
    }
    napi_value stringifyFunc = nullptr;
    if (napi_get_named_property(env, jsonObj, "stringify", &stringifyFunc) != napi_ok || stringifyFunc == nullptr) {
        return "";
    }

    napi_value stringifyResult = nullptr;
    napi_value argv[ARGC_ONE] = { object };
    if (napi_call_function(env, jsonObj, stringifyFunc, ARGC_ONE, argv, &stringifyResult) != napi_ok ||
        stringifyResult == nullptr) {
        return "";
    }

    std::string result;
    if (!ConvertFromJsValue(env, stringifyResult, result)) {
        return "";
    }
    return result;
}

bool ParseStandaloneResult(napi_env env, napi_value value, StandaloneScriptExecuteResult &result, std::string &error)
{
    napi_valuetype valueType = napi_undefined;
    if (value == nullptr || napi_typeof(env, value, &valueType) != napi_ok || valueType != napi_object) {
        error = "result must be an object";
        return false;
    }

    if (!GetRequiredInt32Property(env, value, "code", result.code)) {
        error = "result.code must be a number";
        return false;
    }

    napi_value resultValue = nullptr;
    if (GetNamedProperty(env, value, "result", resultValue) && !IsNullOrUndefined(env, resultValue)) {
        if (napi_typeof(env, resultValue, &valueType) != napi_ok || valueType != napi_object) {
            error = "result.result must be an object";
            return false;
        }
        AAFwk::WantParams wantParams;
        if (!AppExecFwk::UnwrapWantParams(env, resultValue, wantParams)) {
            error = "result.result must be a valid object";
            return false;
        }
        result.resultJson = StringifyObject(env, resultValue);
        if (result.resultJson.empty()) {
            error = "result.result must be JSON-serializable";
            return false;
        }
        result.hasResult = true;
    }

    if (!GetOptionalStringArrayProperty(env, value, "uris", result.uris, result.hasUris)) {
        error = "result.uris must be an array of strings";
        return false;
    }

    if (!GetOptionalUint32Property(env, value, "flags", result.flags, result.hasFlags)) {
        error = "result.flags must be a number";
        return false;
    }

    return true;
}
} // namespace

void JSScriptManager::Finalizer(napi_env env, void *data, void *hint)
{
    std::unique_ptr<JSScriptManager>(static_cast<JSScriptManager *>(data));
}

napi_value JSScriptManager::CompleteArkTSScriptInApp(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSScriptManager, OnCompleteArkTSScriptInApp);
}

napi_value JSScriptManager::CompleteArkTSScript(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSScriptManager, OnCompleteArkTSScript);
}

napi_value JSScriptManager::OnCompleteArkTSScriptInApp(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "JSScriptManager::OnCompleteArkTSScriptInApp called");
    HandleEscape handleEscape(env);
    if (argc < INDEX_THREE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    if (!VerifyContext(env, argv[INDEX_ZERO])) {
        ThrowContextNotValidError(env);
        return CreateJsUndefined(env);
    }
    auto context = GetStageModeContext(env, argv[INDEX_ZERO]);
    sptr<IRemoteObject> token = (context != nullptr) ? context->GetToken() : nullptr;
    if (token == nullptr) {
        ThrowInvalidParamError(env, "failed to get token from context");
        return CreateJsUndefined(env);
    }
    std::string requestCode = ParseRequestCode(env, argv[INDEX_ONE]);
    if (requestCode.empty()) {
        ThrowInvalidParamError(env, "requestCode must be a non-empty string");
        return CreateJsUndefined(env);
    }
    AppExecFwk::SkillExecuteResult skillResult;
    if (!UnwrapSkillExecuteResult(env, argv[INDEX_TWO], skillResult)) {
        ThrowInvalidParamError(env, "result must be a valid ExecuteResult");
        return CreateJsUndefined(env);
    }
    TAG_LOGD(AAFwkTag::JSNAPI,
        "completeArkTSScriptInApp reqCode:%{public}s code:%{public}d",
        requestCode.c_str(), skillResult.code);
    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute =
        [innerErrCode, token, requestCode, skillResult]() {
        *innerErrCode = AAFwk::AbilityManagerClient::GetInstance()->ExecuteSkillDone(
            token, requestCode, skillResult.code, skillResult);
    };
    NapiAsyncTask::CompleteCallback complete =
        [innerErrCode](napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::JSNAPI,
                "completeArkTSScriptInApp error: %{public}d", *innerErrCode);
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };
    napi_value asyncResult = nullptr;
    NapiAsyncTask::Schedule("JSScriptManager::OnCompleteArkTSScriptInApp", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute),
            std::move(complete), &asyncResult));
    return handleEscape.Escape(asyncResult);
}

napi_value JSScriptManager::OnCompleteArkTSScript(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "JSScriptManager::OnCompleteArkTSScript called");
    HandleEscape handleEscape(env);
    if (argc != ARGC_ONE) {
        ThrowInvalidNumParametersError(env);
        return CreateJsUndefined(env);
    }

    StandaloneScriptExecuteResult standaloneResult;
    std::string error;
    if (!ParseStandaloneResult(env, argv[INDEX_ZERO], standaloneResult, error)) {
        ThrowInvalidParamError(env, error);
        return CreateJsUndefined(env);
    }

    auto innerErrCode = std::make_shared<AbilityErrorCode>(AbilityErrorCode::ERROR_OK);
    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, standaloneResult]() {
        if (!CompleteStandaloneScript(standaloneResult)) {
            *innerErrCode = AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT;
        }
    };
    NapiAsyncTask::CompleteCallback complete =
        [innerErrCode](napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != AbilityErrorCode::ERROR_OK) {
            TAG_LOGE(AAFwkTag::JSNAPI, "completeArkTSScript error: %{public}d", static_cast<int32_t>(*innerErrCode));
            task.Reject(env, CreateJsError(env, *innerErrCode));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    napi_value asyncResult = nullptr;
    NapiAsyncTask::Schedule("JSScriptManager::OnCompleteArkTSScript", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &asyncResult));
    return handleEscape.Escape(asyncResult);
}

napi_value JSScriptManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "Init JSScriptManager");

    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGW(AAFwkTag::JSNAPI, "Null env or exportObj");
        return nullptr;
    }

    std::unique_ptr<JSScriptManager> jsScriptManager = std::make_unique<JSScriptManager>();
    napi_wrap(env, exportObj, jsScriptManager.release(),
        JSScriptManager::Finalizer, nullptr, nullptr);

    const char *moduleName = "ScriptManager";
    BindNativeFunction(env, exportObj, "completeArkTSScriptInApp", moduleName,
        JSScriptManager::CompleteArkTSScriptInApp);
    BindNativeFunction(env, exportObj, "completeArkTSScript", moduleName,
        JSScriptManager::CompleteArkTSScript);

    TAG_LOGD(AAFwkTag::JSNAPI, "JSScriptManagerInit end");
    return CreateJsUndefined(env);
}

} // namespace AbilityRuntime
} // namespace OHOS
