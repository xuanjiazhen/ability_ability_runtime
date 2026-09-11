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

#include "js_cli_hook_scheduler_stub.h"

#include <chrono>

#include "hilog_tag_wrapper.h"
#include "napi_common_util.h"
#include "napi_common_want.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t HOOK_TIMEOUT_MS = 5000;

napi_value CreateJsExecResult(napi_env env, const ExecResult& result)
{
    napi_value jsObj = nullptr;
    napi_create_object(env, &jsObj);
    napi_set_named_property(env, jsObj, "exitCode", AppExecFwk::WrapInt32ToJS(env, result.exitCode));
    napi_set_named_property(env, jsObj, "outputText", AppExecFwk::WrapStringToJS(env, result.outputText));
    napi_set_named_property(env, jsObj, "errorText", AppExecFwk::WrapStringToJS(env, result.errorText));
    napi_set_named_property(env, jsObj, "signalNumber", AppExecFwk::WrapInt32ToJS(env, result.signalNumber));
    napi_set_named_property(env, jsObj, "timeOut", AppExecFwk::WrapBoolToJS(env, result.timeout));
    napi_set_named_property(env, jsObj, "executionTime", AppExecFwk::WrapInt64ToJS(env, result.executionTime));
    return jsObj;
}

void ParseJsExecResult(napi_env env, napi_value jsObj, ExecResult& result)
{
    bool hasProp = false;
    if (napi_has_named_property(env, jsObj, "exitCode", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "exitCode", &prop);
        AppExecFwk::UnwrapInt32FromJS2(env, prop, result.exitCode);
    }
    if (napi_has_named_property(env, jsObj, "outputText", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "outputText", &prop);
        AppExecFwk::UnwrapStringFromJS2(env, prop, result.outputText);
    }
    if (napi_has_named_property(env, jsObj, "errorText", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "errorText", &prop);
        AppExecFwk::UnwrapStringFromJS2(env, prop, result.errorText);
    }
    if (napi_has_named_property(env, jsObj, "signalNumber", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "signalNumber", &prop);
        AppExecFwk::UnwrapInt32FromJS2(env, prop, result.signalNumber);
    }
    if (napi_has_named_property(env, jsObj, "timeOut", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "timeOut", &prop);
        AppExecFwk::UnwrapBoolFromJS2(env, prop, result.timeout);
    }
    if (napi_has_named_property(env, jsObj, "executionTime", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "executionTime", &prop);
        AppExecFwk::UnwrapInt64FromJS2(env, prop, result.executionTime);
    }
}

napi_value CreateJsExecOptions(napi_env env, const ExecOptions& opts)
{
    napi_value jsObj = nullptr;
    napi_create_object(env, &jsObj);
    napi_set_named_property(env, jsObj, "background", AppExecFwk::WrapBoolToJS(env, opts.background));
    napi_set_named_property(env, jsObj, "yieldMs", AppExecFwk::WrapInt64ToJS(env, opts.yieldMs));
    napi_set_named_property(env, jsObj, "timeout", AppExecFwk::WrapInt64ToJS(env, opts.timeout));
    return jsObj;
}

void ParseJsExecOptions(napi_env env, napi_value jsObj, ExecOptions& opts)
{
    bool hasProp = false;
    if (napi_has_named_property(env, jsObj, "background", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "background", &prop);
        AppExecFwk::UnwrapBoolFromJS2(env, prop, opts.background);
    }
    if (napi_has_named_property(env, jsObj, "yieldMs", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "yieldMs", &prop);
        AppExecFwk::UnwrapInt64FromJS2(env, prop, opts.yieldMs);
    }
    if (napi_has_named_property(env, jsObj, "timeout", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "timeout", &prop);
        AppExecFwk::UnwrapInt64FromJS2(env, prop, opts.timeout);
    }
}

napi_value CreateJsExecToolParam(napi_env env, const ExecToolParam& param)
{
    napi_value jsObj = nullptr;
    napi_create_object(env, &jsObj);
    napi_set_named_property(env, jsObj, "toolName", AppExecFwk::WrapStringToJS(env, param.toolName));
    napi_set_named_property(env, jsObj, "subCommand", AppExecFwk::WrapStringToJS(env, param.subcommand));
    napi_set_named_property(env, jsObj, "challenge", AppExecFwk::WrapStringToJS(env, param.challenge));
    napi_value jsArgs = AppExecFwk::WrapWantParams(env, param.args);
    napi_set_named_property(env, jsObj, "args", jsArgs);
    napi_set_named_property(env, jsObj, "options", CreateJsExecOptions(env, param.options));
    return jsObj;
}

void ParseJsExecToolParam(napi_env env, napi_value jsObj, ExecToolParam& param)
{
    bool hasProp = false;
    if (napi_has_named_property(env, jsObj, "toolName", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "toolName", &prop);
        AppExecFwk::UnwrapStringFromJS2(env, prop, param.toolName);
    }
    if (napi_has_named_property(env, jsObj, "subCommand", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "subCommand", &prop);
        AppExecFwk::UnwrapStringFromJS2(env, prop, param.subcommand);
    }
    if (napi_has_named_property(env, jsObj, "challenge", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "challenge", &prop);
        AppExecFwk::UnwrapStringFromJS2(env, prop, param.challenge);
    }
    if (napi_has_named_property(env, jsObj, "args", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "args", &prop);
        AppExecFwk::UnwrapWantParams(env, prop, param.args);
    }
    if (napi_has_named_property(env, jsObj, "options", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "options", &prop);
        ParseJsExecOptions(env, prop, param.options);
    }
}

napi_value CreateJsExecCmdParam(napi_env env, const ExecCmdParam& param)
{
    napi_value jsObj = nullptr;
    napi_create_object(env, &jsObj);
    napi_set_named_property(env, jsObj, "cmd", AppExecFwk::WrapStringToJS(env, param.cmd));
    napi_set_named_property(env, jsObj, "workDir", AppExecFwk::WrapStringToJS(env, param.workDir));
    napi_set_named_property(env, jsObj, "env", AppExecFwk::WrapStringToJS(env, param.env));
    napi_set_named_property(env, jsObj, "policy", AppExecFwk::WrapStringToJS(env, param.policy));
    napi_set_named_property(env, jsObj, "options", CreateJsExecOptions(env, param.options));
    return jsObj;
}

void ParseJsExecCmdParam(napi_env env, napi_value jsObj, ExecCmdParam& param)
{
    bool hasProp = false;
    if (napi_has_named_property(env, jsObj, "cmd", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "cmd", &prop);
        AppExecFwk::UnwrapStringFromJS2(env, prop, param.cmd);
    }
    if (napi_has_named_property(env, jsObj, "workDir", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "workDir", &prop);
        AppExecFwk::UnwrapStringFromJS2(env, prop, param.workDir);
    }
    if (napi_has_named_property(env, jsObj, "env", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "env", &prop);
        AppExecFwk::UnwrapStringFromJS2(env, prop, param.env);
    }
    if (napi_has_named_property(env, jsObj, "policy", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "policy", &prop);
        AppExecFwk::UnwrapStringFromJS2(env, prop, param.policy);
    }
    if (napi_has_named_property(env, jsObj, "options", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "options", &prop);
        ParseJsExecOptions(env, prop, param.options);
    }
}
} // namespace

JsCliHookSchedulerStub::JsCliHookSchedulerStub(napi_env env, napi_value jsHookObj)
    : env_(env)
{
    if (env == nullptr || jsHookObj == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "JsCliHookSchedulerStub: invalid env or jsHookObj");
        return;
    }
    napi_create_reference(env, jsHookObj, 1, &callbackRef_);
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, "JsCliHookScheduler", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 0, 1,
        this, &JsCliHookSchedulerStub::Finalize, this, &JsCliHookSchedulerStub::CallJs, &tsfn_);
}

JsCliHookSchedulerStub::~JsCliHookSchedulerStub()
{
    if (tsfn_ != nullptr && !released_.exchange(true)) {
        napi_release_threadsafe_function(tsfn_, napi_tsfn_abort);
    }
    if (callbackRef_ != nullptr && env_ != nullptr) {
        napi_delete_reference(env_, callbackRef_);
    }
}

void JsCliHookSchedulerStub::Finalize(napi_env env, void* data, void* hint)
{
    // threadsafe_function finalize — nothing extra to do; the Stub is owned by
    // CliToolManagerService and released when unregister or process death occurs.
}

ErrCode JsCliHookSchedulerStub::DispatchToJs(const std::string& method,
    ExecToolParam* toolParam, ExecCmdParam* cmdParam, ExecResult* execResult)
{
    if (tsfn_ == nullptr || callbackRef_ == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "DispatchToJs: tsfn or callbackRef is null, method=%{public}s", method.c_str());
        return ERR_OK;
    }
    auto* callData = new (std::nothrow) HookCallData();
    if (callData == nullptr) {
        return ERR_OK;
    }
    callData->methodName = method;
    callData->toolParam = toolParam;
    callData->cmdParam = cmdParam;
    callData->execResult = execResult;
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

ErrCode JsCliHookSchedulerStub::BeforeCallTool(ExecToolParam& param)
{
    return DispatchToJs("onBeforeCallTool", &param, nullptr, nullptr);
}

ErrCode JsCliHookSchedulerStub::AfterCallTool(ExecResult& execResult)
{
    return DispatchToJs("onAfterCallTool", nullptr, nullptr, &execResult);
}

ErrCode JsCliHookSchedulerStub::BeforeCallCmd(ExecCmdParam& param)
{
    return DispatchToJs("onBeforeCallCmd", nullptr, &param, nullptr);
}

ErrCode JsCliHookSchedulerStub::AfterCallCmd(ExecResult& execResult)
{
    return DispatchToJs("onAfterCallCmd", nullptr, nullptr, &execResult);
}

void JsCliHookSchedulerStub::CallJs(napi_env env, napi_value jsCb, void* context, void* data)
{
    auto* callData = static_cast<HookCallData*>(data);
    if (callData == nullptr) {
        return;
    }
    auto* self = static_cast<JsCliHookSchedulerStub*>(context);
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

    // Convert native param to JS object based on method name
    napi_value jsParam = nullptr;
    if (callData->methodName == "onBeforeCallTool" && callData->toolParam != nullptr) {
        jsParam = CreateJsExecToolParam(env, *callData->toolParam);
    } else if (callData->methodName == "onAfterCallTool" && callData->execResult != nullptr) {
        napi_value jsInner = CreateJsExecResult(env, *callData->execResult);
        napi_create_object(env, &jsParam);
        napi_set_named_property(env, jsParam, "execResult", jsInner);
    } else if (callData->methodName == "onBeforeCallCmd" && callData->cmdParam != nullptr) {
        jsParam = CreateJsExecCmdParam(env, *callData->cmdParam);
    } else if (callData->methodName == "onAfterCallCmd" && callData->execResult != nullptr) {
        napi_value jsInner = CreateJsExecResult(env, *callData->execResult);
        napi_create_object(env, &jsParam);
        napi_set_named_property(env, jsParam, "execResult", jsInner);
    } else {
        callData->promise->set_value();
        delete callData;
        return;
    }

    // Call JS method: jsObj.methodName(jsParam) — skip if method not defined
    napi_value jsMethod = nullptr;
    napi_get_named_property(env, jsObj, callData->methodName.c_str(), &jsMethod);
    napi_valuetype methodType = napi_undefined;
    napi_typeof(env, jsMethod, &methodType);
    if (methodType != napi_function) {
        TAG_LOGD(AAFwkTag::CLI_TOOL, "CallJs: method %{public}s not a function, skipping", callData->methodName.c_str());
        callData->promise->set_value();
        delete callData;
        return;
    }
    napi_value jsResult = nullptr;
    napi_value argv[1] = {jsParam};
    napi_status status = napi_call_function(env, jsObj, jsMethod,
        1, argv, &jsResult);
    if (status != napi_ok || jsResult == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "CallJs: napi_call_function failed or no return, method=%{public}s",
            callData->methodName.c_str());
        callData->promise->set_value();
        delete callData;
        return;
    }

    // Parse JS return value back to native param
    if (callData->methodName == "onBeforeCallTool" && callData->toolParam != nullptr) {
        ParseJsExecToolParam(env, jsResult, *callData->toolParam);
    } else if (callData->methodName == "onAfterCallTool" && callData->execResult != nullptr) {
        napi_value jsInner = nullptr;
        napi_get_named_property(env, jsResult, "execResult", &jsInner);
        if (jsInner != nullptr) {
            ParseJsExecResult(env, jsInner, *callData->execResult);
        }
    } else if (callData->methodName == "onBeforeCallCmd" && callData->cmdParam != nullptr) {
        ParseJsExecCmdParam(env, jsResult, *callData->cmdParam);
    } else if (callData->methodName == "onAfterCallCmd" && callData->execResult != nullptr) {
        napi_value jsInner = nullptr;
        napi_get_named_property(env, jsResult, "execResult", &jsInner);
        if (jsInner != nullptr) {
            ParseJsExecResult(env, jsInner, *callData->execResult);
        }
    }

    callData->jsExecuted.store(true);
    callData->promise->set_value();
    delete callData;
}

} // namespace CliTool
} // namespace OHOS
