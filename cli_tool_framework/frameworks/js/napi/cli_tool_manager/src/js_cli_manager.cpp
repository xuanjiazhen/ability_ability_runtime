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

#include <atomic>
#include <string>

#include "cli_error_code.h"
#include "cli_manager_error_utils.h"
#include "cli_session_info.h"
#include "cli_tool_mgr_client.h"
#include "exec_result.h"
#include "ffrt.h"
#include "function_info.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_result.h"
#include "intent_client.h"
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
constexpr int32_t INVOKE_FUNCTION_TIMEOUT_US = 30000000;

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

struct InvokeFunctionResultData {
    bool success = false;                                   // Promise resolve/reject decision
    int32_t errorCode = ERR_OK;                             // native err code for reject path
    int32_t resultCode = 0;                                 // executeResult.code, drives InvokeResult.success/errorCode
    std::shared_ptr<AAFwk::WantParams> result;              // data carried into InvokeResult.data
    std::string message;                                    // message carried into InvokeResult.message
};

struct InvokeFunctionTsfnContext {
    napi_deferred deferred = nullptr;
    napi_threadsafe_function tsfn = nullptr;
    std::shared_ptr<std::atomic<bool>> completed;
};

class JsInvokeFunctionCallbackClient
    : public InsightIntentExecuteCallbackInterface,
      public std::enable_shared_from_this<JsInvokeFunctionCallbackClient> {
public:
    explicit JsInvokeFunctionCallbackClient(std::shared_ptr<InvokeFunctionTsfnContext> tsfnContext)
        : tsfnContext_(tsfnContext) {}

    virtual ~JsInvokeFunctionCallbackClient() = default;

    void ProcessInsightIntentExecute(int32_t resultCode,
        AppExecFwk::InsightIntentExecuteResult executeResult) override
    {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "invokeFunction result callback, resultCode=%{public}d", resultCode);
        if (tsfnContext_ == nullptr || tsfnContext_->completed == nullptr) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "tsfnContext or completed is null");
            return;
        }
        bool expected = false;
        if (!tsfnContext_->completed->compare_exchange_strong(expected, true)) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "invokeFunction already completed");
            return;
        }
        auto *data = new InvokeFunctionResultData();
        // Two distinct authorities split the framework status from the app status:
        //  - Promise resolve/reject is decided SOLELY by resultCode (framework-level
        //    status from InsightIntentExecuteManager). resultCode == 0 means the
        //    intent was delivered and the target replied; anything else rejects.
        //  - InvokeResult.success/errorCode (built in CreateJsInvokeResult) is decided
        //    by the application-level executeResult.code, surfacing the target's own
        //    business result to the caller even when the framework delivery succeeded.
        data->success = (resultCode == 0);
        data->errorCode = (resultCode == 0) ? 0 : ERR_FUNCTION_EXECUTE_FAILED;
        data->resultCode = executeResult.code;  // app-level code drives InvokeResult
        data->result = executeResult.result;  // share WantParams ownership
        // message is reserved for future app-supplied description; empty for now.
        napi_call_threadsafe_function(tsfnContext_->tsfn, data, napi_tsfn_nonblocking);
    }

private:
    std::shared_ptr<InvokeFunctionTsfnContext> tsfnContext_;
};

void InvokeFunctionFinalize(napi_env env, void *finalizeData, void *finalizeHint)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "InvokeFunctionTsfn finalize");
    if (finalizeData == nullptr) {
        return;
    }
    auto ctx = static_cast<std::shared_ptr<InvokeFunctionTsfnContext> *>(finalizeData);
    // Note: callback registration in InsightIntentHostClient is managed by
    // IntentClient::ExecuteIntentByFunctionCall internally; it is auto-cleaned
    // when OnExecuteDone fires. intentKey is no longer exposed here.
    delete ctx;
}

void InvokeFunctionCallJs(napi_env env, napi_value jsCb, void *context, void *data)
{
    if (env == nullptr || context == nullptr || data == nullptr) {
        return;
    }
    auto *ctx = static_cast<InvokeFunctionTsfnContext *>(context);
    auto *resultData = static_cast<InvokeFunctionResultData *>(data);
    HandleScope handleScope(env);

    if (resultData->success) {
        napi_value jsResult = CreateJsInvokeResult(env, resultData->resultCode,
            resultData->result, resultData->message);
        napi_resolve_deferred(env, ctx->deferred, jsResult);
    } else {
        napi_value jsError = CreateCliJsErrorByNativeErr(env, resultData->errorCode);
        napi_reject_deferred(env, ctx->deferred, jsError);
    }
    napi_release_threadsafe_function(ctx->tsfn, napi_tsfn_release);
    delete resultData;
}

void ReportInvokeError(InvokeFunctionTsfnContext &tsfnContext, int32_t errorCode)
{
    if (tsfnContext.completed == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "completed is null");
        return;
    }
    bool expected = false;
    if (tsfnContext.completed->compare_exchange_strong(expected, true)) {
        auto *data = new InvokeFunctionResultData();
        data->success = false;
        data->errorCode = errorCode;
        napi_call_threadsafe_function(tsfnContext.tsfn, data, napi_tsfn_nonblocking);
    }
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

napi_value JSCliManager::InvokeFunction(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSCliManager, OnInvokeFunction);
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

void ExecuteInvokeFunction(const std::string &bundleName, const std::string &functionName,
    const AAFwk::WantParams &wantParams,
    const std::shared_ptr<InvokeFunctionTsfnContext> &tsfnContext)
{
    if (tsfnContext == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "tsfnContext is null");
        return;
    }

    // Step 1: Query function info
    FunctionInfo functionInfo;
    ErrCode queryErr = CliToolMGRClient::GetInstance().GetFunctionInfo(bundleName, functionName, functionInfo);
    if (queryErr != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetFunctionInfo failed: %{public}d", queryErr);
        // Preserve specific error semantics; collapse others (KVStore/IPC/JSON etc.) into QUERY_FAILED.
        int32_t reportErr = ERR_FUNCTION_QUERY_FAILED;
        if (queryErr == ERR_PERMISSION_DENIED || queryErr == ERR_FUNCTION_NOT_EXIST) {
            reportErr = queryErr;
        }
        ReportInvokeError(*tsfnContext, reportErr);
        return;
    }

    // Step 2: Validate function type
    if (functionInfo.functionType != FunctionType::INTENT_FUNCTION) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Function type not supported: %{public}d",
            static_cast<int32_t>(functionInfo.functionType));
        ReportInvokeError(*tsfnContext, ERR_FUNCTION_TYPE_NOT_SUPPORTED);
        return;
    }

    // Step 3: Execute intent via IntentClient (handles callback registration + IPC)
    auto client = std::make_shared<JsInvokeFunctionCallbackClient>(tsfnContext);

    AAFwk::ExecuteIntentParam param;
    param.bundleName = bundleName;
    param.intentName = functionName;
    param.wantParam = wantParams;
    param.callback = client;

    auto err = AAFwk::IntentClient::GetInstance().ExecuteIntentByFunctionCall(param);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ExecuteIntentByFunctionCall failed: %{public}d", err);
        // ExecuteIntentByFunctionCall returns the AAFwk/common error space. The ability
        // manager's authorization gate reports a permission failure as the common
        // OHOS::ERR_PERMISSION_DENIED (caller lacks EXECUTE_INSIGHT_INTENT); preserve that
        // precise semantics (-> business 201) instead of collapsing it into the generic
        // EXECUTE_FAILED. Qualify with OHOS:: to disambiguate from the cli
        // ERR_PERMISSION_DENIED (35700007) in this namespace.
        int32_t reportErr = (err == OHOS::ERR_PERMISSION_DENIED)
            ? ERR_PERMISSION_DENIED
            : ERR_FUNCTION_EXECUTE_FAILED;
        ReportInvokeError(*tsfnContext, reportErr);
    }
    // Success: result will come via ProcessInsightIntentExecute callback
}

void SetupInvokeTimeout(const std::shared_ptr<InvokeFunctionTsfnContext> &tsfnContext)
{
    ffrt::submit([tsfnContext]() {
        ReportInvokeError(*tsfnContext, ERR_FUNCTION_EXECUTE_TIMEOUT);
    }, ffrt::task_attr().delay(INVOKE_FUNCTION_TIMEOUT_US));
}

napi_value JSCliManager::OnInvokeFunction(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "JSCliManager::OnInvokeFunction called");
    HandleEscape handleEscape(env);
    if (argc < INDEX_THREE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    // Parse bundleName
    std::string bundleName;
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], bundleName) || bundleName.empty()) {
        ThrowInvalidParamError(env, "bundleName is required");
        return CreateJsUndefined(env);
    }

    // Parse function name
    std::string functionName;
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ONE], functionName) || functionName.empty()) {
        ThrowInvalidParamError(env, "name is required");
        return CreateJsUndefined(env);
    }

    // Parse args → WantParams
    AAFwk::WantParams wantParams;
    if (!AppExecFwk::UnwrapWantParams(env, argv[INDEX_TWO], wantParams)) {
        ThrowInvalidParamError(env, "args is required");
        return CreateJsUndefined(env);
    }

    // Create promise
    napi_value promise = nullptr;
    napi_deferred deferred = nullptr;
    napi_create_promise(env, &deferred, &promise);

    // Create threadsafe function context
    auto tsfnContext = std::make_shared<InvokeFunctionTsfnContext>();
    tsfnContext->deferred = deferred;
    tsfnContext->completed = std::make_shared<std::atomic<bool>>(false);

    // Create threadsafe function
    napi_value workName = nullptr;
    napi_create_string_utf8(env, "InvokeFunctionTsfn", NAPI_AUTO_LENGTH, &workName);
    napi_status tsfnStatus = napi_create_threadsafe_function(env, nullptr, nullptr, workName, 0, 1,
        new std::shared_ptr<InvokeFunctionTsfnContext>(tsfnContext), InvokeFunctionFinalize,
        tsfnContext.get(), InvokeFunctionCallJs, &tsfnContext->tsfn);
    if (tsfnStatus != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create threadsafe function");
        napi_value jsError = CreateCliJsErrorByNativeErr(env, ERR_FUNCTION_EXECUTE_FAILED);
        napi_reject_deferred(env, deferred, jsError);
        return handleEscape.Escape(promise);
    }

    // Submit ffrt tasks
    ffrt::submit([bundleName, functionName, wantParams, tsfnContext]() {
        ExecuteInvokeFunction(bundleName, functionName, wantParams, tsfnContext);
    });
    SetupInvokeTimeout(tsfnContext);

    return handleEscape.Escape(promise);
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
    BindNativeFunction(env, exportObj, "sendMessage", moduleName, JSCliManager::SendMessage);
    BindNativeFunction(env, exportObj, "getToolInfoByName", moduleName, JSCliManager::GetToolInfoByName);
    BindNativeFunction(env, exportObj, "queryToolSummaries", moduleName, JSCliManager::QueryToolSummaries);
    BindNativeFunction(env, exportObj, "queryTools", moduleName, JSCliManager::QueryTools);
    BindNativeFunction(env, exportObj, "invokeFunction", moduleName, JSCliManager::InvokeFunction);

    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSCliManagerInit end");
    return CreateJsUndefined(env);
}

} // namespace CliTool
} // namespace OHOS
