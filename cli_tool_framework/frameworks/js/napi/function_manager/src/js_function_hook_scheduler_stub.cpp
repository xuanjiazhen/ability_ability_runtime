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

#include "js_function_hook_scheduler_stub.h"

#include <chrono>

#include "hilog_tag_wrapper.h"
#include "napi_common_util.h"
#include "napi_common_want.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t HOOK_TIMEOUT_MS = 500;

napi_value CreateJsInvokeResult(napi_env env, const InvokeFunctionResult& result)
{
    napi_value jsObj = nullptr;
    napi_create_object(env, &jsObj);
    napi_set_named_property(env, jsObj, "success", AppExecFwk::WrapBoolToJS(env, result.invokeSuccess));
    if (result.result != nullptr) {
        napi_value jsData = AppExecFwk::WrapWantParams(env, *result.result);
        napi_set_named_property(env, jsObj, "data", jsData);
    }
    napi_set_named_property(env, jsObj, "errorCode", AppExecFwk::WrapInt32ToJS(env, result.errorCode));
    napi_set_named_property(env, jsObj, "errorMsg", AppExecFwk::WrapStringToJS(env, result.message));
    return jsObj;
}

void ParseJsInvokeResult(napi_env env, napi_value jsObj, InvokeFunctionResult& result)
{
    bool hasProp = false;
    if (napi_has_named_property(env, jsObj, "success", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "success", &prop);
        AppExecFwk::UnwrapBoolFromJS2(env, prop, result.invokeSuccess);
    }
    if (napi_has_named_property(env, jsObj, "errorCode", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "errorCode", &prop);
        AppExecFwk::UnwrapInt32FromJS2(env, prop, result.errorCode);
    }
    if (napi_has_named_property(env, jsObj, "errorMsg", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "errorMsg", &prop);
        AppExecFwk::UnwrapStringFromJS2(env, prop, result.message);
    }
    if (napi_has_named_property(env, jsObj, "data", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "data", &prop);
        if (prop != nullptr) {
            auto wantParams = std::make_shared<AAFwk::WantParams>();
            AppExecFwk::UnwrapWantParams(env, prop, *wantParams);
            result.result = wantParams;
        }
    }
}
} // namespace

JsFunctionHookSchedulerStub::JsFunctionHookSchedulerStub(napi_env env, napi_value jsHookObj)
    : env_(env)
{
    if (env == nullptr || jsHookObj == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "JsFunctionHookSchedulerStub: invalid env or jsHookObj");
        return;
    }
    napi_create_reference(env, jsHookObj, 1, &callbackRef_);
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, "JsFunctionHookScheduler", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 0, 1,
        this, &JsFunctionHookSchedulerStub::Finalize, this, &JsFunctionHookSchedulerStub::CallJs, &tsfn_);
}

JsFunctionHookSchedulerStub::~JsFunctionHookSchedulerStub()
{
    if (tsfn_ != nullptr && !released_.exchange(true)) {
        napi_release_threadsafe_function(tsfn_, napi_tsfn_abort);
    }
    if (callbackRef_ != nullptr && env_ != nullptr) {
        napi_delete_reference(env_, callbackRef_);
    }
}

void JsFunctionHookSchedulerStub::Finalize(napi_env env, void* data, void* hint)
{
    // threadsafe_function finalize — nothing extra to do.
}

ErrCode JsFunctionHookSchedulerStub::DispatchToJs(const std::string& method,
    const std::string& funcNamespace, const std::string& functionName,
    AAFwk::WantParams* param, InvokeFunctionResult* invokeResult)
{
    if (tsfn_ == nullptr || callbackRef_ == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "DispatchToJs: tsfn or callbackRef is null, method=%{public}s", method.c_str());
        return ERR_OK;
    }
    auto* callData = new (std::nothrow) FunctionHookCallData();
    if (callData == nullptr) {
        return ERR_OK;
    }
    callData->methodName = method;
    callData->funcNamespace = funcNamespace;
    callData->functionName = functionName;
    callData->param = param;
    callData->invokeResult = invokeResult;
    callData->promise = std::make_shared<std::promise<void>>();

    napi_status status = napi_call_threadsafe_function(tsfn_, callData, napi_tsfn_blocking);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "DispatchToJs: napi_call_threadsafe_function failed, status=%{public}d", status);
        delete callData;
        return ERR_OK;
    }

    auto future = callData->promise->get_future();
    if (future.wait_for(std::chrono::milliseconds(HOOK_TIMEOUT_MS)) != std::future_status::ready) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "DispatchToJs: JS callback timeout (500ms), method=%{public}s", method.c_str());
        return ERR_OK;
    }
    return ERR_OK;
}

ErrCode JsFunctionHookSchedulerStub::BeforeInvokeFunction(
    const std::string& funcNamespace, const std::string& functionName, AAFwk::WantParams& param)
{
    return DispatchToJs("beforeInvokeFunction", funcNamespace, functionName, &param, nullptr);
}

ErrCode JsFunctionHookSchedulerStub::AfterInvokeFunction(
    const std::string& funcNamespace, const std::string& functionName, InvokeFunctionResult& invokeResult)
{
    return DispatchToJs("afterInvokeFunction", funcNamespace, functionName, nullptr, &invokeResult);
}

void JsFunctionHookSchedulerStub::CallJs(napi_env env, napi_value jsCb, void* context, void* data)
{
    auto* callData = static_cast<FunctionHookCallData*>(data);
    if (callData == nullptr) {
        return;
    }
    auto* self = static_cast<JsFunctionHookSchedulerStub*>(context);
    if (env == nullptr || self == nullptr || self->callbackRef_ == nullptr) {
        callData->promise->set_value();
        delete callData;
        return;
    }

    napi_value jsObj = nullptr;
    napi_get_reference_value(env, self->callbackRef_, &jsObj);
    if (jsObj == nullptr) {
        callData->promise->set_value();
        delete callData;
        return;
    }

    // Build JS arguments: [functionNamespace, functionName, param/result]
    napi_value argv[3] = {nullptr};
    argv[0] = AppExecFwk::WrapStringToJS(env, callData->funcNamespace);
    argv[1] = AppExecFwk::WrapStringToJS(env, callData->functionName);

    if (callData->methodName == "beforeInvokeFunction" && callData->param != nullptr) {
        argv[2] = AppExecFwk::WrapWantParams(env, *callData->param);
    } else if (callData->methodName == "afterInvokeFunction" && callData->invokeResult != nullptr) {
        argv[2] = CreateJsInvokeResult(env, *callData->invokeResult);
    } else {
        callData->promise->set_value();
        delete callData;
        return;
    }

    // Call JS method: jsObj.methodName(ns, name, param)
    napi_value jsMethod = nullptr;
    napi_get_named_property(env, jsObj, callData->methodName.c_str(), &jsMethod);
    napi_value jsResult = nullptr;
    napi_status status = napi_call_function(env, jsObj, jsMethod,
        3, argv, &jsResult);
    if (status != napi_ok || jsResult == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "CallJs: napi_call_function failed or no return, method=%{public}s",
            callData->methodName.c_str());
        callData->promise->set_value();
        delete callData;
        return;
    }

    // Parse JS return value back to native
    if (callData->methodName == "beforeInvokeFunction" && callData->param != nullptr) {
        AppExecFwk::UnwrapWantParams(env, jsResult, *callData->param);
    } else if (callData->methodName == "afterInvokeFunction" && callData->invokeResult != nullptr) {
        ParseJsInvokeResult(env, jsResult, *callData->invokeResult);
    }

    callData->promise->set_value();
    delete callData;
}

} // namespace CliTool
} // namespace OHOS
