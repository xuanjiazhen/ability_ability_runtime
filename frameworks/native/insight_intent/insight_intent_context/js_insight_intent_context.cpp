/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "js_insight_intent_context.h"

#include <mutex>

#include "ability_window_configuration.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "js_error_utils.h"
#include "js_insight_intent_execution_info.h"
#include "napi_common_want.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr static char CONTEXT_MODULE_NAME[] = "InsightIntentContext";
constexpr int32_t INDEX_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
}

namespace {
// Holds the napi_ref of the app-registered cancel listener. The ref is
// reference-counted so a callback invocation already posted to the JS thread
// stays valid even if the JS signal object is garbage collected concurrently.
struct JsCancelCallbackRefHolder {
    napi_env env = nullptr;
    napi_ref ref = nullptr;

    ~JsCancelCallbackRefHolder()
    {
        if (env != nullptr && ref != nullptr) {
            napi_delete_reference(env, ref);
        }
        env = nullptr;
        ref = nullptr;
    }
};

// JS wrapper of InsightIntentCancellationSignal. This is the signal proxy the
// system reconstructs in the executor process and hands to the app as the
// signal member of the InsightIntentExecutionContext trailing onExecuteXXX
// parameter: setOnCancelListener((reason) => {}) subscribes, isCancelled()
// polls. It marshals the callback from the IPC thread onto the JS thread.
class JsInsightIntentCancellationSignal
    : public std::enable_shared_from_this<JsInsightIntentCancellationSignal> {
public:
    JsInsightIntentCancellationSignal(napi_env env, const std::shared_ptr<InsightIntentCancellationSignal>& signal)
        : signal_(signal)
    {
        (void)env;
    }

    ~JsInsightIntentCancellationSignal()
    {
        if (signal_ != nullptr) {
            signal_->SetOnCancel(nullptr);
        }
    }

    // Must be called after make_shared so weak_from_this() returns a valid weak
    // reference (enable_shared_from_this is only bound once the shared_ptr
    // owns the object, i.e. after the constructor returns).
    void RegisterCancelCallback()
    {
        if (signal_ == nullptr) {
            return;
        }
        std::weak_ptr<JsInsightIntentCancellationSignal> weak = weak_from_this();
        signal_->SetOnCancel([weak](int32_t reason) {
            auto self = weak.lock();
            if (self != nullptr) {
                self->NotifyCancel(reason);
            }
        });
    }

    static std::shared_ptr<JsInsightIntentCancellationSignal> Unwrap(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
        std::shared_ptr<JsInsightIntentCancellationSignal>* holder = nullptr;
        napi_unwrap(env, thisVar, reinterpret_cast<void**>(&holder));
        return (holder != nullptr) ? *holder : std::shared_ptr<JsInsightIntentCancellationSignal>();
    }

    // The v0.5 external contract exposes isCancelled() as a method (not a getter).
    static napi_value IsCancelled(napi_env env, napi_callback_info info)
    {
        auto self = Unwrap(env, info);
        bool isCanceled = self != nullptr && self->signal_ != nullptr && self->signal_->IsCanceled();
        napi_value result = nullptr;
        napi_get_boolean(env, isCanceled, &result);
        return result;
    }

    static napi_value SetOnCancelListener(napi_env env, napi_callback_info info)
    {
        auto self = Unwrap(env, info);
        if (self == nullptr) {
            return CreateJsUndefined(env);
        }
        size_t argc = 1;
        napi_value argv[1] = { nullptr };
        napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
        if (argc < ARGC_ONE) {
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        napi_valuetype type = napi_undefined;
        napi_typeof(env, argv[INDEX_ZERO], &type);
        if (type != napi_function) {
            ThrowInvalidParamError(env, "Parameter error: setOnCancelListener callback must be a function.");
            return CreateJsUndefined(env);
        }

        napi_ref ref = nullptr;
        napi_create_reference(env, argv[INDEX_ZERO], 1, &ref);
        auto holder = std::make_shared<JsCancelCallbackRefHolder>();
        holder->env = env;
        holder->ref = ref;
        {
            std::lock_guard<std::mutex> lock(self->refMutex_);
            self->callbackHolder_ = holder;
        }
        return CreateJsUndefined(env);
    }

    // Runs on the IPC thread; posts the callback onto the JS thread with
    // (reason) as the listener argument.
    void NotifyCancel(int32_t reason)
    {
        std::shared_ptr<JsCancelCallbackRefHolder> holder;
        {
            std::lock_guard<std::mutex> lock(refMutex_);
            holder = callbackHolder_;
        }
        if (holder == nullptr || holder->env == nullptr || holder->ref == nullptr) {
            return;
        }
        if (napi_send_event(holder->env,
            [holder, reason]() {
                HandleScope handleScope(holder->env);
                napi_value callback = nullptr;
                napi_get_reference_value(holder->env, holder->ref, &callback);
                napi_value undefined = nullptr;
                napi_get_undefined(holder->env, &undefined);
                napi_value argv[1] = { nullptr };
                napi_create_int32(holder->env, reason, &argv[0]);
                napi_call_function(holder->env, undefined, callback, 1, argv, nullptr);
            }, napi_eprio_immediate) != napi_ok) {
            TAG_LOGE(AAFwkTag::INTENT, "send cancellation callback to JS thread failed");
        }
    }

private:
    std::shared_ptr<InsightIntentCancellationSignal> signal_ = nullptr;
    std::shared_ptr<JsCancelCallbackRefHolder> callbackHolder_ = nullptr;
    std::mutex refMutex_;
};

void JsInsightIntentCancellationSignalFinalizer(napi_env env, void* data, void* hint)
{
    if (data == nullptr) {
        return;
    }
    delete static_cast<std::shared_ptr<JsInsightIntentCancellationSignal>*>(data);
}

} // namespace

napi_value CreateJsInsightIntentCancellationSignal(
    napi_env env, const std::shared_ptr<InsightIntentCancellationSignal>& signal)
{
    HandleEscape handleEscape(env);
    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    if (signal == nullptr) {
        return handleEscape.Escape(obj);
    }
    auto wrapper = std::make_shared<JsInsightIntentCancellationSignal>(env, signal);
    wrapper->RegisterCancelCallback();
    auto holder = new (std::nothrow) std::shared_ptr<JsInsightIntentCancellationSignal>(wrapper);
    if (holder == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null cancellation signal holder");
        return handleEscape.Escape(obj);
    }
    napi_wrap(env, obj, holder, JsInsightIntentCancellationSignalFinalizer, nullptr, nullptr);
    BindNativeFunction(env, obj, "isCancelled", "InsightIntentCancellationSignal",
        JsInsightIntentCancellationSignal::IsCancelled);
    BindNativeFunction(env, obj, "setOnCancelListener", "InsightIntentCancellationSignal",
        JsInsightIntentCancellationSignal::SetOnCancelListener);
    return handleEscape.Escape(obj);
}

napi_value CreateJsInsightIntentExecutionContext(
    napi_env env, const std::shared_ptr<InsightIntentCancellationSignal>& signal)
{
    HandleEscape handleEscape(env);
    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    napi_value signalJs = CreateJsInsightIntentCancellationSignal(env, signal);
    napi_set_named_property(env, obj, "signal", signalJs);
    return handleEscape.Escape(obj);
}

void JsInsightIntentContext::Finalizer(napi_env env, void* data, void* hint)
{
    TAG_LOGI(AAFwkTag::INTENT, "called");
    std::unique_ptr<JsInsightIntentContext>(static_cast<JsInsightIntentContext*>(data));
}

napi_value JsInsightIntentContext::StartAbiity(napi_env env, napi_callback_info info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    GET_NAPI_INFO_AND_CALL(env, info, JsInsightIntentContext, OnStartAbility);
}

napi_value JsInsightIntentContext::SetReturnModeForUiAbilityForeground(napi_env env, napi_callback_info info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    GET_NAPI_INFO_AND_CALL(env, info, JsInsightIntentContext, OnSetReturnModeForUiAbilityForeground);
}

napi_value JsInsightIntentContext::SetReturnModeForUiExtensionAbility(napi_env env, napi_callback_info info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    GET_NAPI_INFO_AND_CALL(env, info, JsInsightIntentContext, OnSetReturnModeForUiExtensionAbility);
}

napi_value JsInsightIntentContext::GetInsightIntentExecutionInfo(napi_env env, napi_callback_info info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    GET_NAPI_INFO_AND_CALL(env, info, JsInsightIntentContext, OnGetInsightIntentExecutionInfo);
}

napi_value JsInsightIntentContext::IsExecutionCancelled(napi_env env, napi_callback_info info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    GET_NAPI_INFO_AND_CALL(env, info, JsInsightIntentContext, OnIsExecutionCancelled);
}

napi_value JsInsightIntentContext::OnStartAbility(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleEscape handleEscape(env);

    // unwrap want
    AAFwk::Want want;

    bool checkParamResult = CheckStartAbilityParam(env, info, want);
    if (!checkParamResult) {
        TAG_LOGE(AAFwkTag::INTENT, "check startAbility param failed");
        return CreateJsUndefined(env);
    }

    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null context");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    // verify if bundleName is empty or invalid
    if (want.GetElement().GetBundleName().empty() || want.GetElement().GetBundleName() != context->GetBundleName()) {
        TAG_LOGE(AAFwkTag::INTENT, "bundleName empty or invalid");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_OPERATION_NOT_SUPPORTED);
        return CreateJsUndefined(env);
    }
    // modify windowmode setting
    if (context->GetCurrentWindowMode() == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
        context->GetCurrentWindowMode() == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) {
        want.SetParam(AAFwk::Want::PARAM_RESV_WINDOW_MODE, context->GetCurrentWindowMode());
    }

    auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
    // create execute task
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, want, innerErrCode]() {
        auto context = weak.lock();
        if (!context) {
            TAG_LOGE(AAFwkTag::INTENT, "null context");
            *innerErrCode = static_cast<int>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }
        *innerErrCode = context->StartAbilityByInsightIntent(want);
    };
    // create complete task
    NapiAsyncTask::CompleteCallback complete = [innerErrCode](napi_env env, NapiAsyncTask& task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode == ERR_OK) {
            task.Resolve(env, CreateJsUndefined(env));
        } else {
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
        }
    };

    napi_value lastParam = (info.argc > 1) ? info.argv[1] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsInsightIntentContext::OnStartAbility", env,
        CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    TAG_LOGD(AAFwkTag::INTENT, "end");
    return handleEscape.Escape(result);
}

napi_value JsInsightIntentContext::OnSetReturnModeForUiAbilityForeground(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGE(AAFwkTag::INTENT, "called");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleScope handleScope(env);
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null context");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return CreateJsUndefined(env);
    }

    InsightIntentExecuteMode mode = static_cast<InsightIntentExecuteMode>(context->GetExecuteMode());
    if (mode != InsightIntentExecuteMode::UIABILITY_FOREGROUND) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid execute mode");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return CreateJsUndefined(env);
    }
    int32_t returnMode;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], returnMode)) {
        TAG_LOGE(AAFwkTag::INTENT, "convert returnMode failed");
        ThrowInvalidParamError(env, "Parse param returnMode failed, must be a returnMode.");
        return CreateJsUndefined(env);
    }
    context->SetDelayReturnMode(static_cast<InsightIntentReturnMode>(returnMode));
    return CreateJsUndefined(env);
}

napi_value JsInsightIntentContext::OnSetReturnModeForUiExtensionAbility(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGE(AAFwkTag::INTENT, "called");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleScope handleScope(env);
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null context");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return CreateJsUndefined(env);
    }

    InsightIntentExecuteMode mode = static_cast<InsightIntentExecuteMode>(context->GetExecuteMode());
    if (mode != InsightIntentExecuteMode::UIEXTENSION_ABILITY) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid execute mode");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return CreateJsUndefined(env);
    }

    int32_t returnMode;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], returnMode)) {
        TAG_LOGE(AAFwkTag::INTENT, "convert returnMode failed");
        ThrowInvalidParamError(env, "Parse param returnMode failed, must be a returnMode.");
        return CreateJsUndefined(env);
    }
    context->SetDelayReturnMode(static_cast<InsightIntentReturnMode>(returnMode));
    return CreateJsUndefined(env);
}

napi_value JsInsightIntentContext::OnGetInsightIntentExecutionInfo(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null context");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }
    AppExecFwk::InsightIntentExecutionInfo executeInfo;
    auto errCode = context->GetInsightIntentExecutionInfo(executeInfo);
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "get execution info failed, err: %{public}d", errCode);
        ThrowErrorByNativeErr(env, errCode);
        return CreateJsUndefined(env);
    }
    return CreateJsInsightIntentExecutionInfo(env, executeInfo);
}

napi_value JsInsightIntentContext::OnIsExecutionCancelled(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null context");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }
    napi_value result = nullptr;
    napi_get_boolean(env, context->IsExecutionCancelled(), &result);
    return result;
}

napi_value CreateJsInsightIntentContext(napi_env env, const std::shared_ptr<InsightIntentContext>& context)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    HandleEscape handleEscape(env);
    napi_value contextObj;
    napi_create_object(env, &contextObj);

    std::unique_ptr<JsInsightIntentContext> jsInsightIntentContext = std::make_unique<JsInsightIntentContext>(context);
    napi_wrap(env, contextObj, jsInsightIntentContext.release(), JsInsightIntentContext::Finalizer, nullptr, nullptr);

    std::string type = "InsightIntentContext";
    napi_set_named_property(env, contextObj, "contextType", CreateJsValue(env, type));
    BindNativeFunction(env, contextObj, "startAbility", CONTEXT_MODULE_NAME, JsInsightIntentContext::StartAbiity);
    BindNativeFunction(env, contextObj, "setReturnModeForUIAbilityForeground", CONTEXT_MODULE_NAME,
        JsInsightIntentContext::SetReturnModeForUiAbilityForeground);
    BindNativeFunction(env, contextObj, "setReturnModeForUIExtensionAbility", CONTEXT_MODULE_NAME,
        JsInsightIntentContext::SetReturnModeForUiExtensionAbility);
    BindNativeProperty(env, contextObj, "insightIntentExecutionInfo",
        JsInsightIntentContext::GetInsightIntentExecutionInfo);
    BindNativeFunction(env, contextObj, "isExecutionCancelled", CONTEXT_MODULE_NAME,
        JsInsightIntentContext::IsExecutionCancelled);
    TAG_LOGD(AAFwkTag::INTENT, "end");
    return handleEscape.Escape(contextObj);
}

bool CheckStartAbilityParam(napi_env env, NapiCallbackInfo& info, AAFwk::Want& want)
{
    if (info.argc == 0) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid argc");
        ThrowTooFewParametersError(env);
        return false;
    }

    // unwrap want
    bool unwrapWantFlag = OHOS::AppExecFwk::UnwrapWant(env, info.argv[0], want);
    if (!unwrapWantFlag) {
        TAG_LOGE(AAFwkTag::INTENT, "parse want fail");
        ThrowInvalidParamError(env, "Parameter error: Failed to parse want, must be a Want.");
        return false;
    }
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
