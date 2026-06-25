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

#include <string>

#include "abc_execute_param.h"
#include "cli_error_code.h"
#include "cli_manager_error_utils.h"
#include "cli_session_info.h"
#include "cli_tool_mgr_client.h"
#include "exec_cmd_param.h"
#include "exec_result.h"
#include "hilog_tag_wrapper.h"
#include "js_cli_event_handler_manager.h"
#include "js_cli_manager_utils.h"
#include "js_cli_session_event_callback.h"
#include "js_error_utils.h"
#include "napi_common_util.h"
#include "napi_common_want.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr int32_t INDEX_THREE = 3;
constexpr int32_t INDEX_FOUR = 4;

int32_t DispatchCliTool(const ExecToolParam &param, napi_env env,
    std::shared_ptr<NapiAsyncTask> asyncTask)
{
    CliToolMGRClient::ExecToolReplyCallback replyCallback =
        [env, asyncTask](int32_t resultCode, const CliSessionInfo &session) {
            JsCliEventHandlerManager::GetInstance().PostTask(
                [env, asyncTask, resultCode, session]() {
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
    return CliToolMGRClient::GetInstance().ExecTool(param, replyCallback);
}

int32_t DispatchCliCmd(const ExecCmdParam &param, napi_env env,
    std::shared_ptr<NapiAsyncTask> asyncTask,
    std::shared_ptr<JsCliSessionEventCallbackImpl> callback)
{
    CliToolMGRClient::ExecToolReplyCallback replyCallback =
        [env, asyncTask](int32_t resultCode, const CliSessionInfo &session) {
            JsCliEventHandlerManager::GetInstance().PostTask(
                [env, asyncTask, resultCode, session]() {
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
    return CliToolMGRClient::GetInstance().ExecCmd(param, replyCallback, callback);
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

napi_value JSCliManager::ExecCmd(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSCliManager, OnExecCmd);
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

    int32_t errCode = DispatchCliTool(param, env, asyncTask);
    if (errCode != ERR_OK) {
        asyncTask->Reject(env, CreateCliJsErrorByNativeErr(env, errCode));
    }
    return handleEscape.Escape(result);
}

napi_value JSCliManager::OnExecCmd(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSCliManager::OnExecCmd called");
    HandleEscape handleEscape(env);
    if (argc < INDEX_ONE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    ExecCmdParam param;
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], param.cmd) || param.cmd.empty()) {
        ThrowInvalidParamError(env, "cmd is required");
        return CreateJsUndefined(env);
    }

    // Parse optional execCmdOptions (argv[1])
    std::shared_ptr<JsCliSessionEventCallbackImpl> callback = nullptr;
    if (argc > INDEX_ONE && argv[INDEX_ONE] != nullptr) {
        napi_valuetype valueType = napi_undefined;
        napi_status status = napi_typeof(env, argv[INDEX_ONE], &valueType);
        if (status == napi_ok && valueType == napi_object) {
            // Parse the rest of the options into param
            std::string cmdOptionsMsg;
            if (!UnwrapExecCmdOptions(env, argv[INDEX_ONE], param, callback, cmdOptionsMsg)) {
                ThrowInvalidParamError(env, cmdOptionsMsg.empty() ? "Invalid ExecCmdOptions" : cmdOptionsMsg.c_str());
                return CreateJsUndefined(env);
            }
        }
    }

    napi_value result = nullptr;
    auto uasyncTask = CreateAsyncTaskWithLastParam(env, nullptr, nullptr, nullptr, &result);
    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);

    int32_t errCode = DispatchCliCmd(param, env, asyncTask, callback);
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

napi_value JSCliManager::ExecAbc(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSCliManager, OnExecAbc);
}

napi_value JSCliManager::OnExecAbc(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] Argument count: argc=%{public}zu", argc);
    HandleEscape handleEscape(env);

    if (argc < INDEX_ONE) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[JS_LAYER] Parameter validation failed: Too few parameters, got=%{public}zu, need at least 1", argc);
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AbcExecuteParam param;
    bool useObjectParam = false;

    // Check if first argument is an object (new interface)
    napi_valuetype firstType = napi_undefined;
    napi_status status = napi_typeof(env, argv[INDEX_ZERO], &firstType);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] First argument type check: type=%{public}d (0=undefined, 3=object, 4=array, 6=string)",
        firstType);

    if (status == napi_ok && firstType == napi_object) {
        // Check if it's an array (old interface with array as second arg)
        bool isArray = false;
        napi_is_array(env, argv[INDEX_ZERO], &isArray);
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] First argument isArray=%{public}d", isArray);

        if (!isArray) {
            // New interface: object parameter
            TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] Using new object parameter interface");
            useObjectParam = true;

            // Get abcPath property (optional if using HSP lookup)
            napi_value abcPathValue = nullptr;
            napi_get_named_property(env, argv[INDEX_ZERO], "abcPath", &abcPathValue);
            napi_valuetype abcPathType = napi_undefined;
            napi_typeof(env, abcPathValue, &abcPathType);
            if (abcPathType == napi_string) {
                AppExecFwk::UnwrapStringFromJS2(env, abcPathValue, param.abcPath);
                TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] Parsed abcPath='%{public}s'", param.abcPath.c_str());
            } else {
                TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] abcPath property not found or not string (type=%{public}d, may use HSP lookup)", abcPathType);
            }

            // Get HSP lookup properties (optional alternative to abcPath)
            napi_value hspBundleNameValue = nullptr;
            napi_get_named_property(env, argv[INDEX_ZERO], "hspBundleName", &hspBundleNameValue);
            napi_valuetype hspBundleNameType = napi_undefined;
            napi_typeof(env, hspBundleNameValue, &hspBundleNameType);
            if (hspBundleNameType == napi_string) {
                AppExecFwk::UnwrapStringFromJS2(env, hspBundleNameValue, param.hspBundleName);
                TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] Parsed hspBundleName='%{public}s'", param.hspBundleName.c_str());
            } else {
                TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] hspBundleName property not found or not string (type=%{public}d)", hspBundleNameType);
            }

            napi_value hspModuleNameValue = nullptr;
            napi_get_named_property(env, argv[INDEX_ZERO], "hspModuleName", &hspModuleNameValue);
            napi_valuetype hspModuleNameType = napi_undefined;
            napi_typeof(env, hspModuleNameValue, &hspModuleNameType);
            if (hspModuleNameType == napi_string) {
                AppExecFwk::UnwrapStringFromJS2(env, hspModuleNameValue, param.hspModuleName);
                TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] Parsed hspModuleName='%{public}s'", param.hspModuleName.c_str());
            } else {
                TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] hspModuleName property not found or not string (type=%{public}d)", hspModuleNameType);
            }

            // Validate: either abcPath or (hspBundleName + hspModuleName) must be provided
            if (param.abcPath.empty() && (param.hspBundleName.empty() || param.hspModuleName.empty())) {
                TAG_LOGE(AAFwkTag::CLI_TOOL, "[JS_LAYER] Parameter validation failed: Need abcPath OR (hspBundleName+hspModuleName)");
                TAG_LOGE(AAFwkTag::CLI_TOOL, "[JS_LAYER]   abcPath.empty=%{public}d, hspBundleName.empty=%{public}d, hspModuleName.empty=%{public}d",
                    param.abcPath.empty(), param.hspBundleName.empty(), param.hspModuleName.empty());
                ThrowInvalidParamError(env, "Either 'abcPath' or both 'hspBundleName' and 'hspModuleName' must be provided");
                return CreateJsUndefined(env);
            }

            // Get optional properties
            napi_value funcNameValue = nullptr;
            napi_get_named_property(env, argv[INDEX_ZERO], "functionName", &funcNameValue);
            napi_valuetype funcNameType = napi_undefined;
            napi_typeof(env, funcNameValue, &funcNameType);
            if (funcNameType == napi_string) {
                AppExecFwk::UnwrapStringFromJS2(env, funcNameValue, param.functionName);
                TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] Parsed functionName='%{public}s'", param.functionName.c_str());
            } else {
                TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] functionName property not found or not string (type=%{public}d)", funcNameType);
            }

            napi_value bundleNameValue = nullptr;
            napi_get_named_property(env, argv[INDEX_ZERO], "bundleName", &bundleNameValue);
            napi_valuetype bundleNameType = napi_undefined;
            napi_typeof(env, bundleNameValue, &bundleNameType);
            if (bundleNameType == napi_string) {
                AppExecFwk::UnwrapStringFromJS2(env, bundleNameValue, param.bundleName);
                TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] Parsed bundleName='%{public}s'", param.bundleName.c_str());
            } else {
                TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] bundleName property not found or not string (type=%{public}d)", bundleNameType);
            }

            napi_value moduleNameValue = nullptr;
            napi_get_named_property(env, argv[INDEX_ZERO], "moduleName", &moduleNameValue);
            napi_valuetype moduleNameType = napi_undefined;
            napi_typeof(env, moduleNameValue, &moduleNameType);
            if (moduleNameType == napi_string) {
                AppExecFwk::UnwrapStringFromJS2(env, moduleNameValue, param.moduleName);
                TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] Parsed moduleName='%{public}s'", param.moduleName.c_str());
            } else {
                TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] moduleName property not found or not string (type=%{public}d)", moduleNameType);
            }

            // Get args property (array)
            napi_value argsValue = nullptr;
            napi_get_named_property(env, argv[INDEX_ZERO], "args", &argsValue);
            napi_valuetype argsType = napi_undefined;
            napi_typeof(env, argsValue, &argsType);
            if (argsValue != nullptr && argsType != napi_undefined) {
                bool isArgsArray = false;
                napi_is_array(env, argsValue, &isArgsArray);
                TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] args property found, isArray=%{public}d, type=%{public}d", isArgsArray, argsType);
                if (isArgsArray) {
                    uint32_t length = 0;
                    napi_get_array_length(env, argsValue, &length);
                    TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] Parsed args array, length=%{public}u", length);
                    for (uint32_t i = 0; i < length; ++i) {
                        napi_value element;
                        napi_get_element(env, argsValue, i, &element);
                        std::string argStr;
                        if (AppExecFwk::UnwrapStringFromJS2(env, element, argStr)) {
                            param.args.push_back(argStr);
                            TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER]   arg[%{public}u]='%{public}s'", i, argStr.c_str());
                        }
                    }
                }
            }

            TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] New interface summary: abcPath='%{public}s', hsp=%{public}s/%{public}s, func='%{public}s', bundle='%{public}s', args=%{public}zu",
                     param.abcPath.c_str(), param.hspBundleName.c_str(), param.hspModuleName.c_str(),
                     param.functionName.c_str(), param.bundleName.c_str(), param.args.size());
        }
    }

    // Old interface: abcPath, args, bundleName
    if (!useObjectParam) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] Using old positional parameter interface");
        if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], param.abcPath) || param.abcPath.empty()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "[JS_LAYER] Parameter validation failed: abcPath is required and empty");
            ThrowInvalidParamError(env, "abcPath is required and must be a non-empty string");
            return CreateJsUndefined(env);
        }

        TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] Parsed abcPath='%{public}s'", param.abcPath.c_str());

        // Parse optional args parameter (argv[1])
        if (argc > INDEX_ONE && argv[INDEX_ONE] != nullptr) {
            napi_valuetype valueType = napi_undefined;
            napi_status status = napi_typeof(env, argv[INDEX_ONE], &valueType);
            if (status == napi_ok && valueType == napi_object) {
                bool isArray = false;
                napi_is_array(env, argv[INDEX_ONE], &isArray);
                if (isArray) {
                    uint32_t length = 0;
                    napi_get_array_length(env, argv[INDEX_ONE], &length);
                    TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] Parsed args array (old interface), length=%{public}u", length);
                    for (uint32_t i = 0; i < length; ++i) {
                        napi_value element;
                        napi_get_element(env, argv[INDEX_ONE], i, &element);
                        std::string argStr;
                        if (AppExecFwk::UnwrapStringFromJS2(env, element, argStr)) {
                            param.args.push_back(argStr);
                            TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER]   arg[%{public}u]='%{public}s'", i, argStr.c_str());
                        }
                    }
                }
            }
        }

        // Parse optional bundleName parameter (argv[2])
        if (argc > INDEX_TWO && argv[INDEX_TWO] != nullptr) {
            AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_TWO], param.bundleName);
            TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] Parsed bundleName='%{public}s'", param.bundleName.c_str());
        }

        TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] Old interface summary: abcPath='%{public}s', bundle='%{public}s', args=%{public}zu",
                 param.abcPath.c_str(), param.bundleName.c_str(), param.args.size());
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] Creating async task for execution");
    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);

    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, param]() {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] >>> ASYNC EXECUTE START: Calling CliToolMGRClient::ExecAbc");
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] >>> Passing to client layer: abcPath='%{public}s', hsp='%{public}s/%{public}s'",
            param.abcPath.c_str(), param.hspBundleName.c_str(), param.hspModuleName.c_str());
        *innerErrCode = CliToolMGRClient::GetInstance().ExecAbc(param);
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] <<< ASYNC EXECUTE COMPLETE: CliToolMGRClient::ExecAbc returned=%{public}d", *innerErrCode);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrCode](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] >>> COMPLETE CALLBACK: result=%{public}d, status=%{public}d", *innerErrCode, status);
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "[JS_LAYER] Execution failed: error=%{public}d", *innerErrCode);
            task.Reject(env, CreateCliJsErrorByNativeErr(env, *innerErrCode));
            return;
        }
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] Execution succeeded: resolving Promise");
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    napi_value asyncResult = nullptr;
    NapiAsyncTask::Schedule("JsCliManager::OnExecAbc", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &asyncResult));

    TAG_LOGI(AAFwkTag::CLI_TOOL, "[JS_LAYER] Async task scheduled, returning Promise to JavaScript");
    TAG_LOGI(AAFwkTag::CLI_TOOL, "========== [JS_LAYER] OnExecAbc EXIT ==========");
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
    BindNativeFunction(env, exportObj, "execCmd", moduleName, JSCliManager::ExecCmd);
    BindNativeFunction(env, exportObj, "subscribeSession", moduleName, JSCliManager::SubscribeSession);
    BindNativeFunction(env, exportObj, "clearSession", moduleName, JSCliManager::ClearSession);
    BindNativeFunction(env, exportObj, "querySession", moduleName, JSCliManager::QuerySession);
    BindNativeFunction(env, exportObj, "sendMessage", moduleName, JSCliManager::SendMessage);
    BindNativeFunction(env, exportObj, "getToolInfoByName", moduleName, JSCliManager::GetToolInfoByName);
    BindNativeFunction(env, exportObj, "queryToolSummaries", moduleName, JSCliManager::QueryToolSummaries);
    BindNativeFunction(env, exportObj, "queryTools", moduleName, JSCliManager::QueryTools);
    BindNativeFunction(env, exportObj, "execAbc", moduleName, JSCliManager::ExecAbc);

    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSCliManagerInit end");
    return CreateJsUndefined(env);
}

} // namespace CliTool
} // namespace OHOS
