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

#include "js_cli_manager.h"

#include <map>
#include <set>
#include <string>

#include "ability_manager_client.h"
#include "cli_error_code.h"
#include "cli_manager_error_utils.h"
#include "cli_session_info.h"
#include "cli_tool_mgr_client.h"
#include "hilog_tag_wrapper.h"
#include "js_cli_manager_utils.h"
#include "js_error_utils.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "napi_common_skill_execute.h"
#include "napi_base_context.h"
#include "skill/skill_execute_callback_stub.h"
#include "array_wrapper.h"
#include "bool_wrapper.h"
#include "double_wrapper.h"
#include "float_wrapper.h"
#include "int_wrapper.h"
#include "long_wrapper.h"
#include "string_wrapper.h"
#include "want_params.h"

#include "js_cli_event_handler_manager.h"
#include "js_cli_session_event_callback.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr int32_t INDEX_THREE = 3;
constexpr int32_t INDEX_FOUR = 4;
constexpr int32_t ERR_CONTEXT_NOT_ABILITY = 16000020;

bool VerifyAbilityContext(napi_env env, napi_value value)
{
    if (value == nullptr) {
        return false;
    }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType != napi_object) {
        return false;
    }
    napi_value abilityInfo = nullptr;
    napi_get_named_property(env, value, "abilityInfo", &abilityInfo);
    napi_valuetype infoType = napi_undefined;
    napi_typeof(env, abilityInfo, &infoType);
    return infoType == napi_object;
}

void ThrowContextNotAbilityError(napi_env env)
{
    ThrowError(env, ERR_CONTEXT_NOT_ABILITY, "The context is not ability context.");
}

std::string GetStringPropertyFromJs(napi_env env, napi_value obj, const std::string &key)
{
    napi_value value = nullptr;
    napi_get_named_property(env, obj, key.c_str(), &value);
    if (value == nullptr) {
        return "";
    }
    std::string result;
    if (!AppExecFwk::UnwrapStringFromJS2(env, value, result)) {
        return "";
    }
    return result;
}

std::string GetPropertyKeyFromJs(napi_env env, napi_value keyVal)
{
    size_t strLen = 0;
    napi_get_value_string_utf8(env, keyVal, nullptr, 0, &strLen);
    std::string key(strLen, '\0');
    napi_get_value_string_utf8(env, keyVal, key.data(), strLen + 1, &strLen);
    return key;
}

void SetSkillArrayString(const std::string &key, const std::vector<std::string> &values,
    AAFwk::WantParams &params)
{
    auto arr = sptr<AAFwk::IArray>(new (std::nothrow) AAFwk::Array(values.size(), AAFwk::g_IID_IString));
    if (arr == nullptr) { return; }
    for (size_t i = 0; i < values.size(); i++) {
        arr->Set(i, AAFwk::String::Box(values[i]));
    }
    params.SetParam(key, arr);
}

void SetSkillArrayBool(const std::string &key, const std::vector<bool> &values,
    AAFwk::WantParams &params)
{
    auto arr = sptr<AAFwk::IArray>(new (std::nothrow) AAFwk::Array(values.size(), AAFwk::g_IID_IBoolean));
    if (arr == nullptr) { return; }
    for (size_t i = 0; i < values.size(); i++) {
        arr->Set(i, AAFwk::Boolean::Box(values[i]));
    }
    params.SetParam(key, arr);
}

void SetSkillArrayDouble(const std::string &key, const std::vector<double> &values,
    AAFwk::WantParams &params)
{
    auto arr = sptr<AAFwk::IArray>(new (std::nothrow) AAFwk::Array(values.size(), AAFwk::g_IID_IDouble));
    if (arr == nullptr) { return; }
    for (size_t i = 0; i < values.size(); i++) {
        arr->Set(i, AAFwk::Double::Box(values[i]));
    }
    params.SetParam(key, arr);
}

void SetSkillArrayLong(const std::string &key, const std::vector<int64_t> &values,
    AAFwk::WantParams &params)
{
    auto arr = sptr<AAFwk::IArray>(new (std::nothrow) AAFwk::Array(values.size(), AAFwk::g_IID_ILong));
    if (arr == nullptr) { return; }
    for (size_t i = 0; i < values.size(); i++) {
        arr->Set(i, AAFwk::Long::Box(values[i]));
    }
    params.SetParam(key, arr);
}

void SetSkillArrayParam(napi_env env, const std::string &key, napi_value val,
    AAFwk::WantParams &params)
{
    uint32_t size = 0;
    if (!AppExecFwk::IsArrayForNapiValue(env, val, size) || size == 0) {
        return;
    }
    napi_value elem = nullptr;
    napi_get_element(env, val, 0, &elem);
    if (elem == nullptr) { return; }

    napi_valuetype elemType = napi_undefined;
    napi_typeof(env, elem, &elemType);
    switch (elemType) {
        case napi_string: {
            std::vector<std::string> values;
            if (AppExecFwk::UnwrapArrayStringFromJS(env, val, values)) {
                SetSkillArrayString(key, values, params);
            }
            break;
        }
        case napi_number: {
            std::vector<double> dblValues;
            if (AppExecFwk::UnwrapArrayDoubleFromJS(env, val, dblValues)) {
                SetSkillArrayDouble(key, dblValues, params);
            }
            break;
        }
        case napi_boolean: {
            std::vector<bool> values;
            if (AppExecFwk::UnwrapArrayBoolFromJS(env, val, values)) {
                SetSkillArrayBool(key, values, params);
            }
            break;
        }
        case napi_bigint: {
            std::vector<int64_t> values;
            if (AppExecFwk::UnwrapArrayInt64FromJS(env, val, values)) {
                SetSkillArrayLong(key, values, params);
            }
            break;
        }
        default:
            break;
    }
}

std::shared_ptr<AAFwk::WantParams> ExtractSkillArgs(napi_env env, napi_value obj)
{
    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    napi_value propertyNames = nullptr;
    napi_get_property_names(env, obj, &propertyNames);
    if (propertyNames == nullptr) {
        return skillArgs;
    }

    const std::set<std::string> reservedKeys = {
        "skillToolType", "bundleName", "moduleName", "skillName", "arkTSPath", "funcName"
    };

    uint32_t length = 0;
    napi_get_array_length(env, propertyNames, &length);
    for (uint32_t i = 0; i < length; i++) {
        napi_value keyVal = nullptr;
        napi_get_element(env, propertyNames, i, &keyVal);
        if (keyVal == nullptr) { continue; }
        std::string key = GetPropertyKeyFromJs(env, keyVal);
        if (key.empty() || reservedKeys.count(key) > 0) { continue; }

        // Build a sub-object containing only this key, then use UnwrapWantParams
        napi_value val = nullptr;
        napi_get_named_property(env, obj, key.c_str(), &val);
        if (val == nullptr) { continue; }

        // Determine type and set param
        napi_valuetype type = napi_undefined;
        napi_typeof(env, val, &type);
        switch (type) {
            case napi_string: {
                std::string str;
                if (AppExecFwk::UnwrapStringFromJS2(env, val, str)) {
                    skillArgs->SetParam(key, AAFwk::String::Box(str));
                }
                break;
            }
            case napi_number: {
                double dblVal = 0.0;
                napi_get_value_double(env, val, &dblVal);
                int32_t intVal = static_cast<int32_t>(dblVal);
                if (static_cast<double>(intVal) == dblVal) {
                    skillArgs->SetParam(key, AAFwk::Integer::Box(intVal));
                } else {
                    skillArgs->SetParam(key, AAFwk::Float::Box(static_cast<float>(dblVal)));
                }
                break;
            }
            case napi_boolean: {
                bool boolVal = false;
                napi_get_value_bool(env, val, &boolVal);
                skillArgs->SetParam(key, AAFwk::Boolean::Box(boolVal));
                break;
            }
            case napi_bigint: {
                int64_t int64Val = 0;
                bool lossless = true;
                napi_get_value_bigint_int64(env, val, &int64Val, &lossless);
                skillArgs->SetParam(key, AAFwk::Long::Box(int64Val));
                break;
            }
            case napi_object: {
                SetSkillArrayParam(env, key, val, *skillArgs);
                break;
            }
            default:
                break;
        }
    }
    return skillArgs;
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

class SkillExecuteCallbackImpl : public AAFwk::SkillExecuteCallbackStub {
public:
    SkillExecuteCallbackImpl(napi_env env, std::shared_ptr<AbilityRuntime::NapiAsyncTask> asyncTask)
        : env_(env), asyncTask_(std::move(asyncTask)) {}

    void OnExecuteDone(const std::string &requestCode, int32_t resultCode,
        const AppExecFwk::SkillExecuteResult &result) override
    {
        TAG_LOGI(AAFwkTag::CLI_TOOL,
            "SkillExecuteCallbackImpl::OnExecuteDone req:%{public}s code:%{public}d",
            requestCode.c_str(), resultCode);
        auto resultCopy = result;
        JsCliEventHandlerManager::GetInstance().PostTask([this, resultCode, resultCopy]() {
            HandleScope handleScope(env_);
            napi_value jsResult = WrapSkillExecuteResult(env_, resultCopy);
            asyncTask_->ResolveWithNoError(env_, jsResult);
        });
    }

private:
    napi_env env_ = nullptr;
    std::shared_ptr<AbilityRuntime::NapiAsyncTask> asyncTask_;
};

int32_t DispatchExecuteSkill(const std::string &bundleName, const std::string &moduleName,
    const std::string &skillName, const std::string &arkTSPath, const std::string &funcName,
    const std::shared_ptr<AAFwk::WantParams> &skillArgs,
    const sptr<SkillExecuteCallbackImpl> &callback)
{
    constexpr int32_t SKILL_TYPE_INDEPENDENT = -1;
    int32_t skillType = 0;
    auto queryRet = AAFwk::AbilityManagerClient::GetInstance()->QuerySkillType(
        bundleName, moduleName, skillName, skillType);
    if (queryRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "querySkillType failed:%{public}d", queryRet);
        return queryRet;
    }
    if (skillType == SKILL_TYPE_INDEPENDENT) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "independent skill not supported yet");
        return ERR_TOOL_NOT_EXIST;
    }
    return AAFwk::AbilityManagerClient::GetInstance()->ExecuteInAppSkill(
        bundleName, moduleName, skillName, arkTSPath, funcName, skillArgs, callback);
}
} // namespace

void JSCliManager::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSCliManager::Finalizer is called");
    std::unique_ptr<JSCliManager>(static_cast<JSCliManager*>(data));
}

napi_value JSCliManager::ExecTool(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSCliManager, OnExecTool);
}

napi_value JSCliManager::SubscribeSession(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSCliManager, OnSubscribeSession);
}

napi_value JSCliManager::ClearSession(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSCliManager, OnClearSession);
}

napi_value JSCliManager::QuerySession(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSCliManager, OnQuerySession);
}

napi_value JSCliManager::SendMessage(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSCliManager, OnSendMessage);
}

napi_value JSCliManager::GetToolInfoByName(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSCliManager, OnGetToolInfoByName);
}

napi_value JSCliManager::QueryToolSummaries(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSCliManager, OnQueryToolSummaries);
}

napi_value JSCliManager::QueryTools(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSCliManager, OnQueryTools);
}

napi_value JSCliManager::ExecSkillTool(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSCliManager, OnExecSkillTool);
}

napi_value JSCliManager::CompleteArkTSScriptInApp(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSCliManager, OnCompleteArkTSScriptInApp);
}

napi_value JSCliManager::OnExecTool(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSCliManager::OnExecTool called");
    HandleEscape handleEscape(env);
    if (argc < INDEX_FOUR) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    ExecToolParam param;
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], param.toolName) || param.toolName.empty()) {
        ThrowInvalidParamError(env, "Tool toolName is required");
        return CreateJsUndefined(env);
    }

    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ONE], param.subcommand)) {
        ThrowInvalidParamError(env, "Tool subcommand is required");
        return CreateJsUndefined(env);
    }

    if (!AppExecFwk::UnwrapWantParams(env, argv[INDEX_TWO], param.args)) {
        ThrowInvalidParamError(env, "Tool args is required");
        return CreateJsUndefined(env);
    }

    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_THREE], param.challenge) || param.challenge.empty()) {
        ThrowInvalidParamError(env, "Tool challenge is required");
        return CreateJsUndefined(env);
    }

    if (argc > INDEX_FOUR && argv[INDEX_FOUR] != nullptr) {
        if (!UnwrapExecOptions(env, argv[INDEX_FOUR], param.options)) {
            ThrowInvalidParamError(env, "Tool options is required");
            return CreateJsUndefined(env);
        }
    }

    napi_value result = nullptr;
    auto uasyncTask = CreateAsyncTaskWithLastParam(env, nullptr, nullptr, nullptr, &result);
    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);

    // Create callback task that will be invoked when ExecTool completes
    CliToolMGRClient::ExecToolReplyCallback replyCallback =
        [env, asyncTask](int32_t resultCode, const CliSessionInfo &session) {
            JsCliEventHandlerManager::GetInstance().PostTask([env, asyncTask, resultCode, session]() {
                HandleScope handleScope(env);
                if (resultCode != ERR_OK) {
                    asyncTask->Reject(env, CreateCliJsErrorByNativeErr(env, resultCode));
                    return;
                }
                napi_value jsSession = CreateJsCliSessionInfo(env, session);
                if (jsSession == nullptr) {
                    asyncTask->Reject(env, CreateJsUndefined(env));
                    return;
                }
                asyncTask->ResolveWithNoError(env, jsSession);
            });
        };
    int32_t errCode = CliToolMGRClient::GetInstance().ExecTool(param, replyCallback);
    if (errCode != ERR_OK) {
        asyncTask->Reject(env, CreateCliJsErrorByNativeErr(env, errCode));
    }
    return handleEscape.Escape(result);
}

napi_value JSCliManager::OnSubscribeSession(napi_env env, size_t argc, napi_value *argv)
{
    HandleEscape handleEscape(env);
    if (argc < INDEX_TWO) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string sessionId;
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], sessionId) || sessionId.empty()) {
        ThrowInvalidParamError(env, "sessionId is required");
        return CreateJsUndefined(env);
    }
    if (!IsValidToolEventCallback(env, argv[INDEX_ONE])) {
        ThrowInvalidParamError(env, "tool event callback is required");
        return CreateJsUndefined(env);
    }

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    auto callback = std::make_shared<JsCliSessionEventCallbackImpl>(env, argv[INDEX_ONE]);
    if (callback == nullptr || !callback->IsValid()) {
        ThrowInvalidParamError(env, "callback must be a function");
        return CreateJsUndefined(env);
    }
    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, callback, sessionId]() {
        std::string subscriptionId;
        *innerErrCode = CliToolMGRClient::GetInstance().SubscribeSession(sessionId, callback, subscriptionId);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrCode](napi_env env,
        NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            task.Reject(env, CreateCliJsErrorByNativeErr(env, *innerErrCode));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSCliManager::OnSubscribeSession", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JSCliManager::OnClearSession(napi_env env, size_t argc, napi_value *argv)
{
    HandleEscape handleEscape(env);
    if (argc < INDEX_ONE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string sessionId;
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], sessionId) || sessionId.empty()) {
        ThrowInvalidParamError(env, "sessionId is required");
        return CreateJsUndefined(env);
    }

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, sessionId]() {
        *innerErrCode = CliToolMGRClient::GetInstance().ClearSession(sessionId);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrCode](napi_env env, NapiAsyncTask &task,
        int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            task.Reject(env, CreateCliJsErrorByNativeErr(env, *innerErrCode));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSCliManager::OnClearSession", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JSCliManager::OnSendMessage(napi_env env, size_t argc, napi_value *argv)
{
    HandleEscape handleEscape(env);
    if (argc < INDEX_TWO) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    std::string sessionId;
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], sessionId) || sessionId.empty()) {
        ThrowInvalidParamError(env, "sessionId is required");
        return CreateJsUndefined(env);
    }

    std::string inputText;
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ONE], inputText)) {
        ThrowInvalidParamError(env, "inputText is required");
        return CreateJsUndefined(env);
    }

    napi_value asyncResult = nullptr;
    napi_value lastParam = nullptr;
    std::unique_ptr<NapiAsyncTask> uasyncTask =
        CreateAsyncTaskWithLastParam(env, lastParam, nullptr, nullptr, &asyncResult);
    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);

    CliToolMGRClient::EventReplyCallback replyCallback = [env, asyncTask](int32_t resultCode) {
        JsCliEventHandlerManager::GetInstance().PostTask([env, asyncTask, resultCode]() {
            HandleScope handleScope(env);
            if (resultCode != ERR_OK) {
                asyncTask->Reject(env, CreateCliJsErrorByNativeErr(env, resultCode));
                return;
            }
            asyncTask->ResolveWithNoError(env, CreateJsUndefined(env));
        });
    };
    int32_t errCode = CliToolMGRClient::GetInstance().SendMessage(sessionId, inputText, replyCallback);
    if (errCode != ERR_OK) {
        asyncTask->Reject(env, CreateCliJsErrorByNativeErr(env, errCode));
    }
    return handleEscape.Escape(asyncResult);
}

napi_value JSCliManager::OnQuerySession(napi_env env, size_t argc, napi_value *argv)
{
    HandleEscape handleEscape(env);
    if (argc < INDEX_ONE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string sessionId;
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], sessionId) || sessionId.empty()) {
        ThrowInvalidParamError(env, "sessionId is required");
        return CreateJsUndefined(env);
    }

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    auto session = std::make_shared<CliSessionInfo>();
    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, sessionId, session]() {
        *innerErrCode = CliToolMGRClient::GetInstance().QuerySession(sessionId, *session);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrCode, session](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            task.Reject(env, CreateCliJsErrorByNativeErr(env, *innerErrCode));
            return;
        }
        napi_value jsSession = CreateJsCliSessionInfo(env, *session);
        if (jsSession == nullptr) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Fail to create js CliSessionInfo");
            task.Reject(env, CreateJsUndefined(env));
            return;
        }
        task.ResolveWithNoError(env, jsSession);
    };
    napi_value asyncResult = nullptr;
    NapiAsyncTask::Schedule("JSCliManager::OnQuerySession", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &asyncResult));
    return handleEscape.Escape(asyncResult);
}

napi_value JSCliManager::OnGetToolInfoByName(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSCliManager::OnGetToolInfoByName called");
    HandleEscape handleEscape(env);
    if (argc < INDEX_ONE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    std::string toolName;
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], toolName) || toolName.empty()) {
        ThrowInvalidParamError(env, "Tool name is required");
        return CreateJsUndefined(env);
    }

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    auto tool = std::make_shared<ToolInfo>();

    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, toolName, tool]() {
        *innerErrCode = CliToolMGRClient::GetInstance().GetToolInfoByName(toolName, *tool);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrCode, tool](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "GetToolInfoByName error: %{public}d", *innerErrCode);
            task.Reject(env, CreateCliJsErrorByNativeErr(env, *innerErrCode));
            return;
        }

        napi_value jsTool = CreateJsToolInfo(env, *tool);
        if (jsTool == nullptr) {
            task.Reject(env, CreateJsUndefined(env));
            return;
        }

        task.ResolveWithNoError(env, jsTool);
    };

    napi_value asyncResult = nullptr;
    NapiAsyncTask::Schedule("JsCliManager::OnGetToolInfoByName", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &asyncResult));
    return handleEscape.Escape(asyncResult);
}

napi_value JSCliManager::OnQueryToolSummaries(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSCliManager::OnQueryToolSummaries called");
    HandleEscape handleEscape(env);

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    auto summaries = std::make_shared<std::vector<ToolSummary>>();

    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, summaries]() {
        *innerErrCode = CliToolMGRClient::GetInstance().GetAllToolSummaries(*summaries);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrCode, summaries](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "QueryToolSummaries error: %{public}d", *innerErrCode);
            task.Reject(env, CreateCliJsErrorByNativeErr(env, *innerErrCode));
            return;
        }

        napi_value jsArray = nullptr;
        napi_create_array(env, &jsArray);
        for (size_t i = 0; i < summaries->size(); i++) {
            napi_value jsSummary = CreateJsToolSummary(env, (*summaries)[i]);
            if (jsSummary != nullptr) {
                napi_set_element(env, jsArray, i, jsSummary);
            }
        }
        task.ResolveWithNoError(env, jsArray);
    };

    napi_value asyncResult = nullptr;
    NapiAsyncTask::Schedule("JsCliManager::OnQueryToolSummaries", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &asyncResult));
    return handleEscape.Escape(asyncResult);
}

napi_value JSCliManager::OnQueryTools(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSCliManager::OnQueryTools called");
    HandleEscape handleEscape(env);

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    auto tools = std::make_shared<std::vector<ToolInfo>>();

    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, tools]() {
        *innerErrCode = CliToolMGRClient::GetInstance().GetAllToolInfos(*tools);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrCode, tools](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "QueryTools error: %{public}d", *innerErrCode);
            task.Reject(env, CreateCliJsErrorByNativeErr(env, *innerErrCode));
            return;
        }

        napi_value jsArray = nullptr;
        napi_create_array(env, &jsArray);
        for (size_t i = 0; i < tools->size(); i++) {
            napi_value jsTool = CreateJsToolInfo(env, (*tools)[i]);
            if (jsTool != nullptr) {
                napi_set_element(env, jsArray, i, jsTool);
            }
        }
        task.ResolveWithNoError(env, jsArray);
    };

    napi_value asyncResult = nullptr;
    NapiAsyncTask::Schedule("JsCliManager::OnQueryTools", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &asyncResult));
    return handleEscape.Escape(asyncResult);
}

napi_value JSCliManager::OnExecSkillTool(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "JSCliManager::OnExecSkillTool called");
    HandleEscape handleEscape(env);
    if (argc < INDEX_TWO) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    // argv[0]: SkillToolParam { bundleName, moduleName, skillName, skillToolType, ...args }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[INDEX_ZERO], &valueType);
    if (valueType != napi_object) {
        ThrowInvalidParamError(env, "skillToolParam must be an object");
        return CreateJsUndefined(env);
    }

    // argv[1]: challenge (string, reserved for standalone skills, not used in in-app skill flow)

    auto skillToolType = GetStringPropertyFromJs(env, argv[INDEX_ZERO], "skillToolType");
    if (skillToolType.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "missing skillToolType");
        ThrowInvalidParamError(env, "skillToolType is required");
        return CreateJsUndefined(env);
    }

    auto bundleName = GetStringPropertyFromJs(env, argv[INDEX_ZERO], "bundleName");
    auto moduleName = GetStringPropertyFromJs(env, argv[INDEX_ZERO], "moduleName");
    auto skillName = GetStringPropertyFromJs(env, argv[INDEX_ZERO], "skillName");
    if (bundleName.empty() || moduleName.empty() || skillName.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "missing required fields");
        ThrowInvalidParamError(env, "bundleName, moduleName, skillName are required");
        return CreateJsUndefined(env);
    }

    // For OHOS_ARKTS_SCRIPT, extract arkTS-specific params
    auto arkTSPath = GetStringPropertyFromJs(env, argv[INDEX_ZERO], "arkTSPath");
    auto funcName = GetStringPropertyFromJs(env, argv[INDEX_ZERO], "funcName");
    auto skillArgs = ExtractSkillArgs(env, argv[INDEX_ZERO]);

    TAG_LOGI(AAFwkTag::CLI_TOOL,
        "execSkillTool bundle:%{public}s module:%{public}s skill:%{public}s "
        "type:%{public}s path:%{public}s func:%{public}s argsCount:%{public}zu",
        bundleName.c_str(), moduleName.c_str(), skillName.c_str(),
        skillToolType.c_str(), arkTSPath.c_str(), funcName.c_str(),
        skillArgs != nullptr ? skillArgs->GetParams().size() : 0);

    // Create deferred Promise — resolved later when callback arrives from AMS
    napi_value result = nullptr;
    auto uasyncTask = CreateAsyncTaskWithLastParam(env, nullptr, nullptr, nullptr, &result);
    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);

    // Create IPC callback that holds the deferred Promise
    auto callback = sptr<SkillExecuteCallbackImpl>::MakeSptr(env, asyncTask);

    // Dispatch IPC — on failure, reject immediately; on success, callback will resolve
    int32_t errCode = DispatchExecuteSkill(
        bundleName, moduleName, skillName, arkTSPath, funcName, skillArgs, callback);
    if (errCode != ERR_OK) {
        asyncTask->Reject(env, CreateCliJsErrorByNativeErr(env, errCode));
    }
    return handleEscape.Escape(result);
}

napi_value JSCliManager::OnCompleteArkTSScriptInApp(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "JSCliManager::OnCompleteArkTSScriptInApp called");
    HandleEscape handleEscape(env);
    if (argc < INDEX_THREE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    if (!VerifyAbilityContext(env, argv[INDEX_ZERO])) {
        ThrowContextNotAbilityError(env);
        return CreateJsUndefined(env);
    }

    auto context = GetStageModeContext(env, argv[INDEX_ZERO]);
    sptr<IRemoteObject> token = nullptr;
    if (context != nullptr) {
        token = context->GetToken();
    }
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

    TAG_LOGI(AAFwkTag::CLI_TOOL,
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
            TAG_LOGE(AAFwkTag::CLI_TOOL, "completeArkTSScriptInApp error: %{public}d", *innerErrCode);
            task.Reject(env, CreateCliJsErrorByNativeErr(env, *innerErrCode));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    napi_value asyncResult = nullptr;
    NapiAsyncTask::Schedule("JSCliManager::OnCompleteArkTSScriptInApp", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &asyncResult));
    return handleEscape.Escape(asyncResult);
}

napi_value JSCliManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "Init JSCliManager");

    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Null env or exportObj");
        return nullptr;
    }

    std::unique_ptr<JSCliManager> jsCliManager = std::make_unique<JSCliManager>();
    napi_wrap(env, exportObj, jsCliManager.release(), JSCliManager::Finalizer, nullptr, nullptr);

    const char *moduleName = "CliManager";
    BindNativeFunction(env, exportObj, "execTool", moduleName, JSCliManager::ExecTool);
    BindNativeFunction(env, exportObj, "subscribeSession", moduleName, JSCliManager::SubscribeSession);
    BindNativeFunction(env, exportObj, "clearSession", moduleName, JSCliManager::ClearSession);
    BindNativeFunction(env, exportObj, "querySession", moduleName, JSCliManager::QuerySession);
    BindNativeFunction(env, exportObj, "sendMsg", moduleName, JSCliManager::SendMessage);
    BindNativeFunction(env, exportObj, "getToolInfoByName", moduleName, JSCliManager::GetToolInfoByName);
    BindNativeFunction(env, exportObj, "queryToolSummaries", moduleName, JSCliManager::QueryToolSummaries);
    BindNativeFunction(env, exportObj, "queryTools", moduleName, JSCliManager::QueryTools);
    BindNativeFunction(env, exportObj, "execSkillTool", moduleName, JSCliManager::ExecSkillTool);
    BindNativeFunction(env, exportObj, "completeArkTSScriptInApp", moduleName,
        JSCliManager::CompleteArkTSScriptInApp);

    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSCliManagerInit end");
    return CreateJsUndefined(env);
}

} // namespace CliTool
} // namespace OHOS
