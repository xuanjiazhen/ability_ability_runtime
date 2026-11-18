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

#include "js_function_manager.h"

#include <mutex>
#include <string>

#include "ability_context.h"
#include "cli_error_code.h"
#include "cli_manager_error_utils.h"
#include "cli_tool_mgr_client.h"
#include "function_info.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent/insight_intent_execute_param.h"
#include "invoke_function_executor.h"
#include "js_error_utils.h"
#include "js_function_hook.h"
#include "js_function_manager_utils.h"
#include "napi_base_context.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "string_wrapper.h"

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

struct InvokeFunctionTsfnContext {
    napi_deferred deferred = nullptr;
    napi_threadsafe_function tsfn = nullptr;
};

void InvokeFunctionFinalize(napi_env env, void *finalizeData, void *finalizeHint)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "InvokeFunctionTsfn finalize");
    if (finalizeData == nullptr) {
        return;
    }
    auto ctx = static_cast<std::shared_ptr<InvokeFunctionTsfnContext> *>(finalizeData);
    delete ctx;
}

void InvokeFunctionCallJs(napi_env env, napi_value jsCb, void *context, void *data)
{
    auto *holder = static_cast<FunctionResultHolder *>(data);
    if (holder == nullptr) {
        return;
    }
    if (env == nullptr || context == nullptr) {
        delete holder;
        return;
    }
    auto *ctx = static_cast<InvokeFunctionTsfnContext *>(context);
    HandleScope handleScope(env);

    if (holder->innerError == 0) {
        napi_value jsResult = CreateJsInvokeResult(env, holder->result);
        if (jsResult != nullptr) {
            napi_resolve_deferred(env, ctx->deferred, jsResult);
        } else {
            napi_reject_deferred(env, ctx->deferred, CreateCliJsErrorByNativeErr(env, ERR_INNER_PARAM_INVALID));
        }
    } else {
        napi_value jsError = CreateCliJsErrorByNativeErr(env, holder->innerError);
        napi_reject_deferred(env, ctx->deferred, jsError);
    }
    napi_release_threadsafe_function(ctx->tsfn, napi_tsfn_release);
    delete holder;
}

std::shared_ptr<AbilityRuntime::AbilityContext> GetAbilityContext(napi_env env, napi_value value)
{
    bool stageMode = false;
    napi_status status = AbilityRuntime::IsStageContext(env, value, stageMode);
    if (status != napi_ok || !stageMode) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "get ability context: not a stage context");
        return nullptr;
    }
    auto context = AbilityRuntime::GetStageModeContext(env, value);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "get ability context: get stage context failed");
        return nullptr;
    }
    auto abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context);
    if (abilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "get ability context: convert to AbilityContext failed");
        return nullptr;
    }
    return abilityContext;
}

// Resolve the optional UIAbilityContext carried by options.context (the 4th
// invokeFunction argument). The parameter is optional: an absent / null context
// keeps outContext null and the invocation proceeds without a caller token (the
// intent implementation side decides how a token-less invocation settles). When a
// context is supplied it must resolve to a UIAbilityContext; anything else is a
// parameter usage error.
bool ParseInvokeContext(napi_env env, napi_value options,
    std::shared_ptr<AbilityRuntime::AbilityContext> &outContext)
{
    outContext = nullptr;
    if (options == nullptr) {
        return true;
    }
    napi_valuetype optionsType = napi_undefined;
    napi_typeof(env, options, &optionsType);
    if (optionsType != napi_object) {
        return true;  // absent / non-object options carry no context
    }
    napi_value contextValue = nullptr;
    napi_get_named_property(env, options, "context", &contextValue);
    napi_valuetype contextType = napi_undefined;
    napi_typeof(env, contextValue, &contextType);
    if (contextType == napi_undefined || contextType == napi_null) {
        return true;  // optional: no context, no caller token
    }
    outContext = GetAbilityContext(env, contextValue);
    if (outContext == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "options.context does not resolve to a UIAbilityContext");
        return false;
    }
    return true;
}

bool ParseInvokeFunctionParam(napi_env env, size_t argc, napi_value *argv, InvokeFunctionParam &out,
    std::shared_ptr<AbilityRuntime::AbilityContext> &outContext)
{
    outContext = nullptr;
    if (argc < INDEX_THREE) {
        ThrowTooFewParametersError(env);
        return false;
    }
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], out.functionNamespace) ||
        out.functionNamespace.empty()) {
        ThrowInvalidParamError(env, "functionNamespace is required");
        return false;
    }
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ONE], out.functionName) ||
        out.functionName.empty()) {
        ThrowInvalidParamError(env, "functionName is required");
        return false;
    }
    if (!AppExecFwk::UnwrapWantParams(env, argv[INDEX_TWO], out.args)) {
        ThrowInvalidParamError(env, "args is required");
        return false;
    }
    // Reserved keys are transport-owned: caller-supplied same-named keys in args
    // are dropped (forgery defense).
    out.args.Remove("ohos.insightIntent.toolCallId");
    out.args.Remove("ohos.insightIntent.dmSessionId");
    out.args.Remove("ohos.task.id");
    if (argc >= INDEX_FOUR) {
        std::string optionsMsg;
        if (!UnwrapInvokeOptions(env, argv[INDEX_THREE], out.invokeOptions, optionsMsg)) {
            ThrowInvalidParamError(env, optionsMsg.empty() ? "option invalid" : optionsMsg.c_str());
            return false;
        }
        if (!ParseInvokeContext(env, argv[INDEX_THREE], outContext)) {
            ThrowInvalidParamError(env, "context is not a UIAbilityContext");
            return false;
        }
    }
    return true;
}

// Transport leg of the dual injection (ADR-8): non-empty ids ride the
// reserved wantParam keys to the executor.
void InjectTransportTraceIds(InvokeFunctionParam &param)
{
    if (!param.invokeOptions.toolCallId.empty()) {
        param.args.SetParam("ohos.insightIntent.toolCallId",
            AAFwk::String::Box(param.invokeOptions.toolCallId));
    }
    if (!param.invokeOptions.dmSessionId.empty()) {
        param.args.SetParam("ohos.insightIntent.dmSessionId",
            AAFwk::String::Box(param.invokeOptions.dmSessionId));
    }
    if (!param.invokeOptions.taskId.empty()) {
        param.args.SetParam("ohos.task.id", AAFwk::String::Box(param.invokeOptions.taskId));
    }
}
} // namespace

void JSFunctionManager::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSFunctionManager::Finalizer is called");
    std::unique_ptr<JSFunctionManager>(static_cast<JSFunctionManager*>(data));
}

napi_value JSFunctionManager::QueryFunctions(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSFunctionManager, OnQueryFunctions);
}

napi_value JSFunctionManager::QueryPublicFunctions(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSFunctionManager, OnQueryPublicFunctions);
}

napi_value JSFunctionManager::InvokeFunction(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSFunctionManager, OnInvokeFunction);
}

napi_value JSFunctionManager::InvokePublicFunction(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSFunctionManager, OnInvokePublicFunction);
}

napi_value JSFunctionManager::OnQueryFunctions(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSFunctionManager::OnQueryFunctions called");
    HandleEscape handleEscape(env);

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    auto functions = std::make_shared<std::vector<FunctionInfo>>();

    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, functions]() {
        *innerErrCode = CliToolMGRClient::GetInstance().GetAllFunctions(*functions);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrCode, functions](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "QueryFunctions error: %{public}d", *innerErrCode);
            task.Reject(env, CreateCliJsErrorByNativeErr(env, *innerErrCode));
            return;
        }

        napi_value jsArray = nullptr;
        napi_status napiStatus = napi_create_array(env, &jsArray);
        if (napiStatus != napi_ok) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "napi_create_array failed, %{public}d", napiStatus);
            task.Reject(env, CreateCliJsErrorByNativeErr(env, ERR_INNER_PARAM_INVALID));
            return;
        }
        for (size_t i = 0; i < functions->size(); i++) {
            napi_value jsFunction = CreateJsFunctionInfo(env, (*functions)[i]);
            if (jsFunction != nullptr) {
                napi_set_element(env, jsArray, i, jsFunction);
            }
        }
        task.ResolveWithNoError(env, jsArray);
    };

    napi_value asyncResult = nullptr;
    NapiAsyncTask::Schedule("JSFunctionManager::OnQueryFunctions", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &asyncResult));
    return handleEscape.Escape(asyncResult);
}

napi_value JSFunctionManager::OnQueryPublicFunctions(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSFunctionManager::OnQueryPublicFunctions called");
    HandleEscape handleEscape(env);

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    auto functions = std::make_shared<std::vector<FunctionInfo>>();

    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, functions]() {
        *innerErrCode = CliToolMGRClient::GetInstance().GetPublicFunctions(*functions);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrCode, functions](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "QueryPublicFunctions error: %{public}d", *innerErrCode);
            task.Reject(env, CreateCliJsErrorByNativeErr(env, *innerErrCode));
            return;
        }

        napi_value jsArray = nullptr;
        napi_status napiStatus = napi_create_array(env, &jsArray);
        if (napiStatus != napi_ok) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "napi_create_array failed, %{public}d", napiStatus);
            task.Reject(env, CreateCliJsErrorByNativeErr(env, ERR_INNER_PARAM_INVALID));
            return;
        }
        for (size_t i = 0; i < functions->size(); i++) {
            napi_value jsFunction = CreateJsFunctionInfo(env, (*functions)[i]);
            if (jsFunction != nullptr) {
                napi_set_element(env, jsArray, i, jsFunction);
            }
        }
        task.ResolveWithNoError(env, jsArray);
    };

    napi_value asyncResult = nullptr;
    NapiAsyncTask::Schedule("JSFunctionManager::OnQueryPublicFunctions", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &asyncResult));
    return handleEscape.Escape(asyncResult);
}

napi_value JSFunctionManager::OnInvokeFunction(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "JSFunctionManager::OnInvokeFunction called");
    HandleEscape handleEscape(env);

    InvokeFunctionParam param;
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext;
    if (!ParseInvokeFunctionParam(env, argc, argv, param, abilityContext)) {
        return CreateJsUndefined(env);  // validation already threw a JS exception
    }

    // options.context is optional on both entries; when supplied its token is
    // threaded to the service side for launch attribution.
    napi_value result = StartInvokeFunction(env, param, false, abilityContext);
    return handleEscape.Escape(result);
}

napi_value JSFunctionManager::OnInvokePublicFunction(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "JSFunctionManager::OnInvokePublicFunction called");
    HandleEscape handleEscape(env);

    InvokeFunctionParam param;
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext;
    if (!ParseInvokeFunctionParam(env, argc, argv, param, abilityContext)) {
        return CreateJsUndefined(env);  // validation already threw a JS exception
    }

    napi_value result = StartInvokeFunction(env, param, true, abilityContext);
    return handleEscape.Escape(result);
}

napi_value JSFunctionManager::StartInvokeFunction(napi_env env, const InvokeFunctionParam &param,
    bool isPublicInvoke, std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext)
{
    // Create promise
    napi_value promise = nullptr;
    napi_deferred deferred = nullptr;
    napi_status promiseStatus = napi_create_promise(env, &deferred, &promise);
    if (promiseStatus != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create promise");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    // Create threadsafe function context.
    auto tsfnContext = std::make_shared<InvokeFunctionTsfnContext>();
    tsfnContext->deferred = deferred;

    // Create threadsafe function
    napi_value workName = nullptr;
    napi_create_string_utf8(env, "InvokeFunctionTsfn", NAPI_AUTO_LENGTH, &workName);
    auto finalizeData = std::make_unique<std::shared_ptr<InvokeFunctionTsfnContext>>(tsfnContext);
    napi_status tsfnStatus = napi_create_threadsafe_function(env, nullptr, nullptr, workName, 0, 1,
        finalizeData.get(), InvokeFunctionFinalize,
        tsfnContext.get(), InvokeFunctionCallJs, &tsfnContext->tsfn);
    if (tsfnStatus != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create threadsafe function");
        napi_value jsError = CreateCliJsErrorByNativeErr(env, ERR_INNER_PARAM_INVALID);
        napi_reject_deferred(env, deferred, jsError);
        return promise;
    }
    finalizeData.release();

    // Bridge: forward the pure-C++ outcome from the executor (worker/binder thread)
    // into the threadsafe-function payload. InvokeFunctionExecutor owns all business
    // logic and is fully decoupled from napi.
    InvokeResultCallback bridge = [tsfnContext](const FunctionResultHolder &holder) {
        auto *data = new FunctionResultHolder(holder);
        napi_status status = napi_call_threadsafe_function(tsfnContext->tsfn, data, napi_tsfn_nonblocking);
        if (status != napi_ok) {
            delete data;
        }
    };

    InvokeFunctionParam invokeParam = param;
    InjectTransportTraceIds(invokeParam);

    // Caller UIAbility token for launch attribution (null when no UIAbilityContext
    // was supplied, e.g. the system entry).
    sptr<IRemoteObject> callerAbilityToken =
        (abilityContext != nullptr) ? abilityContext->GetToken() : nullptr;

    InvokeFunctionExecutor::Create()->Execute(invokeParam, bridge, isPublicInvoke, callerAbilityToken);

    return promise;
}

napi_value JSFunctionManager::RegisterFunctionHook(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSFunctionManager, OnRegisterFunctionHook);
}

napi_value JSFunctionManager::UnregisterFunctionHook(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSFunctionManager, OnUnregisterFunctionHook);
}

namespace {
sptr<JsFunctionHook> g_functionHookStub = nullptr;
std::mutex g_functionHookMutex;

void CleanupFunctionHookOnEnvDestroy(void* data)
{
    auto* envPtr = static_cast<napi_env*>(data);
    if (envPtr == nullptr) {
        return;
    }
    napi_env hookEnv = *envPtr;
    delete envPtr;

    sptr<JsFunctionHook> stub;
    {
        std::lock_guard<std::mutex> lock(g_functionHookMutex);
        if (g_functionHookStub == nullptr || !g_functionHookStub->IsSameEnv(hookEnv)) {
            return;
        }
        stub = g_functionHookStub;
        g_functionHookStub = nullptr;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CleanupFunctionHookOnEnvDestroy: auto unregister on env destroy");
    stub->ReleaseResources();
    CliToolMGRClient::GetInstance().UnregisterFunctionHook(stub);
}

bool ValidateFunctionHookObject(napi_env env, napi_value obj, uint32_t &activeMethods)
{
    activeMethods = 0;
    napi_valuetype type = napi_undefined;
    napi_typeof(env, obj, &type);
    if (type != napi_object) {
        return false;
    }
    static const struct {
        const char* name;
        uint32_t bit;
    } methodBits[] = {
        {"onBeforeInvokeFunction", 0x01},
        {"onAfterInvokeFunction",  0x02},
    };
    for (auto& m : methodBits) {
        napi_value fn = nullptr;
        napi_get_named_property(env, obj, m.name, &fn);
        napi_typeof(env, fn, &type);
        if (type == napi_function) {
            activeMethods |= m.bit;
        }
    }
    return activeMethods != 0; // 0 means no valid methods found — reject at NAPI layer
}
} // namespace

napi_value JSFunctionManager::OnRegisterFunctionHook(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "JSFunctionManager::OnRegisterFunctionHook called");
    HandleEscape handleEscape(env);
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    napi_create_promise(env, &deferred, &promise);

    if (argc < INDEX_ONE) {
        napi_reject_deferred(env, deferred, CreateCliJsErrorByNativeErr(env, ERR_INVALID_PARAM));
        return handleEscape.Escape(promise);
    }

    uint32_t activeMethods = 0;
    if (!ValidateFunctionHookObject(env, argv[0], activeMethods)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "OnRegisterFunctionHook: invalid hook object");
        napi_reject_deferred(env, deferred, CreateCliJsErrorByNativeErr(env, ERR_INVALID_PARAM));
        return handleEscape.Escape(promise);
    }

    std::lock_guard<std::mutex> lock(g_functionHookMutex);
    if (g_functionHookStub != nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "OnRegisterFunctionHook: functionHook stub already exists");
        napi_reject_deferred(env, deferred, CreateCliJsErrorByNativeErr(env, ERR_HOOK_ALREADY_REGISTERED));
        return handleEscape.Escape(promise);
    }

    auto stub = sptr<JsFunctionHook>(new JsFunctionHook(env, argv[0]));
    if (!stub->IsValid()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "OnRegisterFunctionHook: JsFunctionHook init failed");
        napi_reject_deferred(env, deferred, CreateCliJsErrorByNativeErr(env, ERR_NO_INIT));
        return handleEscape.Escape(promise);
    }
    ErrCode ret = CliToolMGRClient::GetInstance().RegisterFunctionHook(stub, static_cast<int32_t>(activeMethods));
    if (ret == ERR_OK) {
        g_functionHookStub = stub;
        napi_add_env_cleanup_hook(env, &CleanupFunctionHookOnEnvDestroy, new napi_env(env));
        napi_resolve_deferred(env, deferred, CreateJsUndefined(env));
    } else {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "RegisterFunctionHook failed: %{public}d", ret);
        napi_reject_deferred(env, deferred, CreateCliJsErrorByNativeErr(env, ret));
    }
    return handleEscape.Escape(promise);
}

napi_value JSFunctionManager::OnUnregisterFunctionHook(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "JSFunctionManager::OnUnregisterFunctionHook called");
    HandleEscape handleEscape(env);

    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    napi_create_promise(env, &deferred, &promise);

    if (argc < INDEX_ONE) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "OnUnregisterFunctionHook: missing hook argument");
        napi_reject_deferred(env, deferred, CreateCliJsErrorByNativeErr(env, ERR_INVALID_PARAM));
        return handleEscape.Escape(promise);
    }

    std::lock_guard<std::mutex> lock(g_functionHookMutex);
    if (g_functionHookStub == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "OnUnregisterFunctionHook: no functionHook stub registered");
        napi_reject_deferred(env, deferred, CreateCliJsErrorByNativeErr(env, ERR_HOOK_NOT_REGISTERED));
        return handleEscape.Escape(promise);
    }

    if (!g_functionHookStub->IsSameEnv(env)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "OnUnregisterFunctionHook: cross-thread unregister not supported");
        napi_reject_deferred(env, deferred, CreateCliJsErrorByNativeErr(env, ERR_INVALID_PARAM));
        return handleEscape.Escape(promise);
    }

    napi_value storedObj = nullptr;
    napi_get_reference_value(env, g_functionHookStub->GetCallbackRef(), &storedObj);
    bool same = false;
    napi_strict_equals(env, argv[0], storedObj, &same);
    if (!same) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "OnUnregisterFunctionHook: hook object does not match registered one");
        napi_reject_deferred(env, deferred, CreateCliJsErrorByNativeErr(env, ERR_INVALID_PARAM));
        return handleEscape.Escape(promise);
    }

    auto stub = g_functionHookStub;
    ErrCode ret = CliToolMGRClient::GetInstance().UnregisterFunctionHook(stub);
    if (ret == ERR_OK || ret == GET_CLI_TOOL_MGR_SERVICE_FAILED || ret == ERR_HOOK_NOT_REGISTERED) {
        if (ret != ERR_OK) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "OnUnregisterFunctionHook: ret=%{public}d, cleanup local stub", ret);
        }
        stub->ReleaseResources();
        g_functionHookStub = nullptr;
        napi_resolve_deferred(env, deferred, CreateJsUndefined(env));
    } else {
        napi_reject_deferred(env, deferred, CreateCliJsErrorByNativeErr(env, ret));
    }
    return handleEscape.Escape(promise);
}

napi_value JSFunctionManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "Init JSFunctionManager");

    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Null env or exportObj");
        return nullptr;
    }

    std::unique_ptr<JSFunctionManager> jsFunctionManager = std::make_unique<JSFunctionManager>();
    napi_status status = napi_wrap(env, exportObj, jsFunctionManager.get(),
        JSFunctionManager::Finalizer, nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "napi_wrap failed: %{public}d", status);
        return nullptr;
    }
    jsFunctionManager.release();

    const char *moduleName = "FunctionManager";
    BindNativeFunction(env, exportObj, "queryFunctions", moduleName, JSFunctionManager::QueryFunctions);
    BindNativeFunction(env, exportObj, "queryPublicFunctions", moduleName, JSFunctionManager::QueryPublicFunctions);
    BindNativeFunction(env, exportObj, "invokeFunction", moduleName, JSFunctionManager::InvokeFunction);
    BindNativeFunction(env, exportObj, "invokePublicFunction", moduleName, JSFunctionManager::InvokePublicFunction);
    BindNativeFunction(env, exportObj, "registerFunctionHook", moduleName, JSFunctionManager::RegisterFunctionHook);
    BindNativeFunction(env, exportObj, "unregisterFunctionHook", moduleName, JSFunctionManager::UnregisterFunctionHook);

    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSFunctionManagerInit end");
    return CreateJsUndefined(env);
}
} // namespace CliTool
} // namespace OHOS
