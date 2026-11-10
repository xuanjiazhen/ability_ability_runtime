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

#include "invoke_function_executor.h"

#include "ability_manager_errors.h"
#include "cli_error_code.h"
#include "exec_options.h"
#include "cli_tool_mgr_client.h"
#include "ffrt.h"
#include "function_info.h"
#include "function_result_wrap.h"
#include "hilog_tag_wrapper.h"
#include "intent_client.h"
#include "int_wrapper.h"
#include "invoke_function_callback_client.h"
#include "invoke_function_param.h"
#include "insight_intent/insight_intent_execute_param.h"
#include "long_wrapper.h"
#include "remote_object_wrapper.h"
#include "string_wrapper.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t INVOKE_FUNCTION_TIMEOUT_US = 30000000;

// Read a string-typed want parameter; returns an empty string when the key is
// absent or holds a non-string value.
std::string GetWantParamString(const AAFwk::WantParams &wantParams, const std::string &key)
{
    const auto &params = wantParams.GetParams();
    auto it = params.find(key);
    if (it == params.end() || it->second == nullptr) {
        return "";
    }
    auto *iStr = AAFwk::IString::Query(it->second);
    if (iStr == nullptr) {
        return "";
    }
    return AAFwk::String::Unbox(iStr);
}

// Query the function info and validate its type. The public path enforces the
// ACCESS_PUBLIC_FUNCTION permission and the open-to-third-party (visible) check
// on the service side; the system path keeps the original GetFunctionInfo
// (system app + ACCESS_FUNCTION) semantics.
ErrCode QueryAndValidateFunctionInfo(const InvokeFunctionParam &param, bool isPublicInvoke,
    FunctionInfo &functionInfo)
{
    ErrCode queryErr;
    if (isPublicInvoke) {
        queryErr = CliToolMGRClient::GetInstance().GetPublicFunctionInfo(
            param.functionNamespace, param.functionName, functionInfo);
    } else {
        queryErr = CliToolMGRClient::GetInstance().GetFunctionInfo(
            param.functionNamespace, param.functionName, functionInfo);
    }
    if (queryErr != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "query function info failed: %{public}d", queryErr);
        return queryErr;
    }
    if (functionInfo.functionType != FunctionType::INTENT_FUNCTION) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Function type not supported: %{public}d",
            static_cast<int32_t>(functionInfo.functionType));
        return ERR_INNER_PARAM_INVALID;
    }
    return ERR_OK;
}

// Trace identifiers re-read after the before-invoke hook has run.
struct TraceIds {
    std::string toolCallId;
    std::string dmSessionId;
    std::string taskId;
};

// Inputs assembled after the before-invoke hook: the hook may rewrite the param
// (and its args), so both are carried forward to the intent call.
struct PreparedInvoke {
    InvokeFunctionParam hookParam;
    AAFwk::WantParams hookParams;
    TraceIds traceIds;
};

// Run the before-invoke hook and capture the post-hook trace ids; invalid
// hook-modified ids are dropped.
PreparedInvoke PrepareInvoke(const InvokeFunctionParam &param)
{
    PreparedInvoke prepared;
    prepared.hookParam = param;
    CliToolMGRClient::GetInstance().BeforeInvokeFunction(prepared.hookParam);
    prepared.hookParams = prepared.hookParam.args;
    TraceIds &ids = prepared.traceIds;
    ids.toolCallId = GetWantParamString(prepared.hookParams, "ohos.insightIntent.toolCallId");
    ids.dmSessionId = GetWantParamString(prepared.hookParams, "ohos.insightIntent.dmSessionId");
    ids.taskId = GetWantParamString(prepared.hookParams, "ohos.task.id");
    if (!IsValidTraceId(ids.toolCallId) || !IsValidTraceId(ids.dmSessionId)) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "invokeFunction execute: invalid hook-modified trace id dropped, length: %{public}zu/%{public}zu",
            ids.toolCallId.length(), ids.dmSessionId.length());
        if (!IsValidTraceId(ids.toolCallId)) {
            ids.toolCallId.clear();
        }
        if (!IsValidTraceId(ids.dmSessionId)) {
            ids.dmSessionId.clear();
        }
    }
    // dmSessionId is stamped on the Wrap only, never logged here.
    if (!ids.toolCallId.empty()) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "invokeFunction execute, toolCallId=%{public}s", ids.toolCallId.c_str());
    }
    return prepared;
}

void BuildExecuteIntentParam(const PreparedInvoke &prepared, const sptr<IRemoteObject> &callerAbilityToken,
    uint64_t intentId, const std::shared_ptr<AbilityRuntime::InsightIntentExecuteCallbackInterface> &client,
    AAFwk::ExecuteIntentParam &execParam)
{
    execParam.bundleName = prepared.hookParam.functionNamespace;
    execParam.intentName = prepared.hookParam.functionName;
    execParam.wantParam = prepared.hookParams;
    // Reuse the consent pre-registered record (0 = system path): box the id into
    // wantParam (INSIGHT_INTENT_EXECUTE_RECORD_ID); the AMS reads and strips it.
    if (intentId != 0) {
        execParam.wantParam.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_RECORD_ID,
            AAFwk::Long::Box64(static_cast<int64_t>(intentId)));
    }
    // Authoritative injection point (ADR-8): re-assert the re-validated values
    // before the intent call; the NAPI pre-injection is only the transport leg.
    if (!prepared.traceIds.toolCallId.empty()) {
        execParam.wantParam.SetParam("ohos.insightIntent.toolCallId",
            AAFwk::String::Box(prepared.traceIds.toolCallId));
    }
    if (!prepared.traceIds.dmSessionId.empty()) {
        execParam.wantParam.SetParam("ohos.insightIntent.dmSessionId",
            AAFwk::String::Box(prepared.traceIds.dmSessionId));
    }
    if (!prepared.traceIds.taskId.empty()) {
        execParam.wantParam.SetParam("ohos.task.id", AAFwk::String::Box(prepared.traceIds.taskId));
    }
    // Authoritative injection (transport leg): carry the caller ability token
    // (context->GetToken(), third-party path only) through wantParam for launch
    // attribution in StartAbilityInner. No identity / public-path flag is injected
    // anymore — the AMS derives the caller identity from the calling token.
    if (callerAbilityToken != nullptr) {
        // Serialize the caller token through the WantParamWrapper envelope. A bare
        // RemoteObjectWrap would be silently dropped by WantParams marshalling
        // (its WriteMarshalling has no IRemoteObjectWrap branch), so the remote
        // object must be nested under TYPE_PROPERTY / VALUE_PROPERTY.
        AAFwk::WantParams tokenWrap;
        tokenWrap.SetParam(AAFwk::TYPE_PROPERTY, AAFwk::String::Box(AAFwk::REMOTE_OBJECT));
        tokenWrap.SetParam(AAFwk::VALUE_PROPERTY, AAFwk::RemoteObjectWrap::Box(callerAbilityToken));
        execParam.wantParam.SetParam(AppExecFwk::INSIGHT_INTENT_CALLER_TOKEN,
            AAFwk::WantParamWrapper::Box(tokenWrap));
    }
    execParam.callback = client;
}
} // namespace

std::shared_ptr<InvokeFunctionExecutor> InvokeFunctionExecutor::Create()
{
    return std::make_shared<InvokeFunctionExecutor>();
}

void InvokeFunctionExecutor::Execute(const InvokeFunctionParam &param, InvokeResultCallback callback,
    bool isPublicInvoke, const sptr<IRemoteObject> &callerAbilityToken, uint64_t intentId)
{
    callback_ = std::move(callback);
    auto self = shared_from_this();
    SetupTimeout();
    ffrt::submit([self, param, isPublicInvoke, callerAbilityToken, intentId]() {
        self->DoExecute(param, isPublicInvoke, callerAbilityToken, intentId);
    });
}

void InvokeFunctionExecutor::SetupTimeout()
{
    auto self = shared_from_this();
    ffrt::submit([self]() {
        self->ReportError(ERR_FUNCTION_EXECUTE_TIMEOUT);
        }, ffrt::task_attr().delay(INVOKE_FUNCTION_TIMEOUT_US));
}

void InvokeFunctionExecutor::ReportError(int32_t errorCode)
{
    bool expected = false;
    if (completed_ == nullptr || !completed_->compare_exchange_strong(expected, true)) {
        return;  // already settled by the normal callback or another failure
    }
    FunctionResultHolder holder;
    holder.innerError = errorCode;
    if (callback_) {
        callback_(holder);
    }
}

void InvokeFunctionExecutor::DoExecute(const InvokeFunctionParam &param, bool isPublicInvoke,
    const sptr<IRemoteObject> &callerAbilityToken, uint64_t intentId)
{
    FunctionInfo functionInfo;
    ErrCode queryErr = QueryAndValidateFunctionInfo(param, isPublicInvoke, functionInfo);
    if (queryErr != ERR_OK) {
        ReportError(queryErr);
        return;
    }

    PreparedInvoke prepared = PrepareInvoke(param);

    auto self = shared_from_this();
    // Stamp the actual execution identifiers onto the after-hook Wrap.
    InvokeResultCallback wrappedCallback = [self, traceIds = prepared.traceIds](
        const FunctionResultHolder &holder) {
        FunctionResultWrap functionResultWrap;
        functionResultWrap.result = holder.result;
        functionResultWrap.toolCallId = traceIds.toolCallId;
        functionResultWrap.dmSessionId = traceIds.dmSessionId;
        CliToolMGRClient::GetInstance().AfterInvokeFunction(functionResultWrap);
        FunctionResultHolder out = holder;
        out.result = functionResultWrap.result;
        if (self->callback_) {
            self->callback_(out);
        }
    };
    auto client = std::make_shared<InvokeFunctionCallbackClient>(completed_, std::move(wrappedCallback));

    AAFwk::ExecuteIntentParam execParam;
    BuildExecuteIntentParam(prepared, callerAbilityToken, intentId, client, execParam);

    ErrCode err = AAFwk::IntentClient::GetInstance().ExecuteIntentByFunctionCall(execParam);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ExecuteIntentByFunctionCall failed: %{public}d", err);
        int32_t reportErr = ERR_FUNCTION_EXECUTE_FAILED;
        if (err == OHOS::ERR_PERMISSION_DENIED || err == OHOS::AAFwk::ERR_NOT_SYSTEM_APP ||
            err == OHOS::AAFwk::INTENT_NOT_EXIST) {
            reportErr = err;
        }
        ReportError(reportErr);
    }
}
} // namespace CliTool
} // namespace OHOS
