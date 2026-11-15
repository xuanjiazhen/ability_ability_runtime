/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ets_insight_intent_context.h"

#include <memory>
#include <mutex>
#include <new>

#include "ability_window_configuration.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ani_common_util.h"
#include "ani_common_want.h"
#include "ani_enum_convert.h"
#include "ani_task.h"
#include "ets_error_utils.h"
#include "ets_insight_intent_execution_info.h"
#include "insight_intent_constant.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *CONTEXT_CLASS_NAME = "@ohos.app.ability.InsightIntentContext.InsightIntentContext";
constexpr const char *SIGNAL_CLASS_NAME =
    "@ohos.app.ability.InsightIntentExecutor.InsightIntentCancellationSignal";
constexpr const char *EXECUTION_CONTEXT_CLASS_NAME =
    "@ohos.app.ability.InsightIntentExecutor.InsightIntentExecutionContext";
constexpr const char *SET_ON_CANCEL_LISTENER_SIGNATURE = "C{std.core.Function1}:";
}

namespace {
// Holds the ani_ref of the app-registered cancel listener. The ref is reference
// counted so a callback invocation already posted to the ANI thread stays valid
// even if the ETS signal object is garbage collected concurrently.
struct EtsCancelCallbackRefHolder {
    ani_vm *vm = nullptr;
    ani_ref ref = nullptr;

    ~EtsCancelCallbackRefHolder()
    {
        if (vm != nullptr && ref != nullptr) {
            bool isAttachThread = false;
            ani_env *env = AppExecFwk::AttachAniEnv(vm, isAttachThread);
            if (env != nullptr) {
                env->GlobalReference_Delete(ref);
                AppExecFwk::DetachAniEnv(vm, isAttachThread);
            }
        }
        vm = nullptr;
        ref = nullptr;
    }
};

// ETS wrapper of InsightIntentCancellationSignal. This is the signal proxy the
// system reconstructs in the executor process and hands to the app as the
// cancellationSignal member of the InsightIntentExecutionContext trailing
// onExecuteXXX parameter: setOnCancelListener((reason) => {}) subscribes,
// isCancelled() polls. It marshals the callback from the IPC thread onto the ANI
// thread.
class EtsInsightIntentCancellationSignal
    : public std::enable_shared_from_this<EtsInsightIntentCancellationSignal> {
public:
    EtsInsightIntentCancellationSignal(
        ani_vm *vm, const std::shared_ptr<InsightIntentCancellationSignal> &signal)
        : vm_(vm), signal_(signal)
    {
    }

    ~EtsInsightIntentCancellationSignal()
    {
        if (signal_ != nullptr) {
            signal_->SetOnCancel(nullptr);
        }
    }

    // Must be called after make_shared so weak_from_this() returns a valid weak
    // reference (enable_shared_from_this is only bound once the shared_ptr owns
    // the object, i.e. after the constructor returns).
    void RegisterCancelCallback()
    {
        if (signal_ == nullptr) {
            return;
        }
        std::weak_ptr<EtsInsightIntentCancellationSignal> weak = weak_from_this();
        signal_->SetOnCancel([weak](int32_t reason) {
            auto self = weak.lock();
            if (self != nullptr) {
                self->NotifyCancel(reason);
            }
        });
    }

    static std::shared_ptr<EtsInsightIntentCancellationSignal> Unwrap(ani_env *env, ani_object aniObj)
    {
        if (env == nullptr || aniObj == nullptr) {
            return std::shared_ptr<EtsInsightIntentCancellationSignal>();
        }
        ani_long nativeSignal = 0;
        if (env->Object_GetFieldByName_Long(aniObj, "nativeSignal", &nativeSignal) != ANI_OK) {
            return std::shared_ptr<EtsInsightIntentCancellationSignal>();
        }
        auto *holder = reinterpret_cast<std::shared_ptr<EtsInsightIntentCancellationSignal> *>(nativeSignal);
        return (holder != nullptr) ? *holder : std::shared_ptr<EtsInsightIntentCancellationSignal>();
    }

    // The external contract exposes isCancelled() as a method (not a getter).
    static ani_boolean IsCancelled(ani_env *env, ani_object aniObj)
    {
        auto self = Unwrap(env, aniObj);
        bool isCanceled = self != nullptr && self->signal_ != nullptr && self->signal_->IsCanceled();
        return isCanceled ? ANI_TRUE : ANI_FALSE;
    }

    static void SetOnCancelListener(ani_env *env, ani_object aniObj, ani_fn_object callback)
    {
        auto self = Unwrap(env, aniObj);
        if (self == nullptr || callback == nullptr) {
            return;
        }
        ani_ref ref = nullptr;
        ani_status status = env->GlobalReference_Create(callback, &ref);
        if (status != ANI_OK || ref == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "create cancel listener reference failed, status: %{public}d", status);
            return;
        }
        auto holder = std::make_shared<EtsCancelCallbackRefHolder>();
        holder->vm = self->vm_;
        holder->ref = ref;
        {
            std::lock_guard<std::mutex> lock(self->refMutex_);
            self->callbackHolder_ = holder;
        }
        if (self->signal_ != nullptr && self->signal_->IsCanceled()) {
            self->NotifyCancel(self->signal_->GetReason());
        }
    }

    // Runs on the IPC thread; posts the callback onto the ANI thread with
    // (reason) as the listener argument.
    void NotifyCancel(int32_t reason)
    {
        std::shared_ptr<EtsCancelCallbackRefHolder> holder;
        {
            std::lock_guard<std::mutex> lock(refMutex_);
            holder = callbackHolder_;
        }
        if (holder == nullptr || holder->vm == nullptr || holder->ref == nullptr) {
            return;
        }
        AniTask::AniSendEvent([holder, reason]() {
            ani_vm *vm = holder->vm;
            bool isAttachThread = false;
            ani_env *env = AppExecFwk::AttachAniEnv(vm, isAttachThread);
            if (env == nullptr) {
                TAG_LOGE(AAFwkTag::INTENT, "AttachAniEnv failed");
                return;
            }
            ani_ref argv[] = { AppExecFwk::CreateInt(env, reason) };
            ani_ref result = nullptr;
            ani_status status = env->FunctionalObject_Call(static_cast<ani_fn_object>(holder->ref), 1, argv, &result);
            if (status != ANI_OK) {
                TAG_LOGE(AAFwkTag::INTENT, "call cancel listener failed, status: %{public}d", status);
            }
            AppExecFwk::DetachAniEnv(vm, isAttachThread);
        });
    }

private:
    ani_vm *vm_ = nullptr;
    std::shared_ptr<InsightIntentCancellationSignal> signal_ = nullptr;
    std::shared_ptr<EtsCancelCallbackRefHolder> callbackHolder_ = nullptr;
    std::mutex refMutex_;
};
} // namespace
void EtsInsightIntentContext::Finalizer(ani_env *env, void *data, void *hint)
{
    TAG_LOGI(AAFwkTag::INTENT, "EtsInsightIntentContext::Finalizer called");
    std::unique_ptr<EtsInsightIntentContext>(static_cast<EtsInsightIntentContext*>(data));
}

void EtsInsightIntentContext::StartAbilitySyncCheck(ani_env *env, ani_object aniObj)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return;
    }
    auto context = GetContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null context");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }
}

ani_object EtsInsightIntentContext::StartAbilitySync(ani_env *env, ani_object aniObj, ani_object wantObj)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto context = GetContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get context failed.");
        return EtsErrorUtil::CreateInvalidParamError(env, "context null");
    }

    AAFwk::Want want;
    if (!OHOS::AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::INTENT, "parse wantParam failed");
        return EtsErrorUtil::CreateInvalidParamError(env, "Parse param want failed, want must be Want.");
    }

    return context->StartAbilityInner(env, want);
}

void EtsInsightIntentContext::SetReturnModeForUIAbilityForeground(ani_env *env,
    ani_object aniObj, ani_enum_item aniMode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto context = GetContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get context failed.");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    auto nativeContext = context->GetNativeContext();
    if (nativeContext == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get context failed.");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    InsightIntentExecuteMode mode = static_cast<InsightIntentExecuteMode>(nativeContext->GetExecuteMode());
    if (mode != InsightIntentExecuteMode::UIABILITY_FOREGROUND) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid execute mode");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    ani_int returnMode = 0;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, aniMode, returnMode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "param mode err");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    nativeContext->SetDelayReturnMode(static_cast<InsightIntentReturnMode>(returnMode));
}

void EtsInsightIntentContext::SetReturnModeForUIExtensionAbility(ani_env *env, ani_object aniObj, ani_enum_item aniMode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto context = GetContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get context failed.");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    auto nativeContext = context->GetNativeContext();
    if (nativeContext == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get context failed.");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    InsightIntentExecuteMode mode = static_cast<InsightIntentExecuteMode>(nativeContext->GetExecuteMode());
    if (mode != InsightIntentExecuteMode::UIEXTENSION_ABILITY) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid execute mode");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    ani_int returnMode = 0;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, aniMode, returnMode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "param mode err");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    nativeContext->SetDelayReturnMode(static_cast<InsightIntentReturnMode>(returnMode));
}

ani_object EtsInsightIntentContext::GetInsightIntentExecutionInfo(ani_env *env, ani_object aniObj)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto context = GetContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get context failed.");
        return EtsErrorUtil::CreateInvalidParamError(env, "context null");
    }
    auto nativeContext = context->GetNativeContext();
    if (nativeContext == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null native context");
        return EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }
    AppExecFwk::InsightIntentExecutionInfo info;
    auto errCode = nativeContext->GetInsightIntentExecutionInfo(info);
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "get execution info failed, err: %{public}d", errCode);
        return EtsErrorUtil::CreateErrorByNativeErr(env, errCode);
    }
    ani_object obj = CreateEtsInsightIntentExecutionInfo(env, info);
    if (obj == nullptr) {
        return EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }
    return obj;
}

ani_boolean EtsInsightIntentContext::IsExecutionCancelled(ani_env *env, ani_object aniObj)
{
    auto context = GetContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get context failed.");
        return ANI_FALSE;
    }
    auto nativeContext = context->GetNativeContext();
    if (nativeContext == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null native context");
        return ANI_FALSE;
    }
    return nativeContext->IsExecutionCancelled() ? ANI_TRUE : ANI_FALSE;
}

EtsInsightIntentContext *EtsInsightIntentContext::GetContext(ani_env *env, ani_object aniObj)
{
    ani_long nativeContextLong;
    ani_class cls {};
    ani_field contextField = nullptr;
    ani_status status = ANI_ERROR;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }

    if ((status = env->FindClass(CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindField(cls, "nativeContext", &contextField)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_GetField_Long(aniObj, contextField, &nativeContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    return reinterpret_cast<EtsInsightIntentContext *>(nativeContextLong);
}

ani_object EtsInsightIntentContext::StartAbilityInner(ani_env *env, AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::INTENT, "EtsInsightIntentContext::StartAbilityInner called");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);

    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null context");
        return EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }

    // verify if bundleName is empty or invalid
    auto bundleNameFromWant = want.GetElement().GetBundleName();
    if (bundleNameFromWant.empty() || bundleNameFromWant != context->GetBundleName()) {
        TAG_LOGE(AAFwkTag::INTENT, "bundleName empty or invalid");
        return EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_OPERATION_NOT_SUPPORTED);
    }
    // modify windowmode setting
    auto windowMode = context->GetCurrentWindowMode();
    if (windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
        windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) {
        want.SetParam(AAFwk::Want::PARAM_RESV_WINDOW_MODE, windowMode);
    }

    auto innerErrCode = context->StartAbilityByInsightIntent(want);
    if (innerErrCode == ERR_OK) {
        return EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK);
    } else {
        return EtsErrorUtil::CreateErrorByNativeErr(env, innerErrCode);
    }
}

std::unique_ptr<AppExecFwk::ETSNativeReference> CreateEtsInsightIntentContext(ani_env *env,
    EtsInsightIntentContext *context)
{
    TAG_LOGD(AAFwkTag::INTENT, "CreateEtsInsightIntentContext called");
    ani_long nativeContextLong = (ani_long)context;
    ani_class cls {};
    ani_status status = ANI_ERROR;
    ani_object contextObj = nullptr;
    ani_method method {};
    ani_field field = nullptr;
    ani_ref contextObjtRef = nullptr;
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null context");
        return std::unique_ptr<AppExecFwk::ETSNativeReference>();
    }
    auto nativeContext = context->GetNativeContext();
    if (nativeContext == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null context");
        return std::unique_ptr<AppExecFwk::ETSNativeReference>();
    }
    ani_int instanceId = (ani_int)context->GetNativeContext()->GetIntentId();
    if ((status = env->FindClass(CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return std::unique_ptr<AppExecFwk::ETSNativeReference>();
    }
    std::array functions = {
        ani_native_function { "nativeStartAbilitySync", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentContext::StartAbilitySync) },
        ani_native_function { "nativeStartAbilitySyncCheck", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentContext::StartAbilitySyncCheck) },
        ani_native_function { "setReturnModeForUIAbilityForeground", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentContext::SetReturnModeForUIAbilityForeground) },
        ani_native_function { "setReturnModeForUIExtensionAbility", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentContext::SetReturnModeForUIExtensionAbility) },
        ani_native_function { "nativeGetInsightIntentExecutionInfo", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentContext::GetInsightIntentExecutionInfo) },
        ani_native_function { "isExecutionCancelled", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentContext::IsExecutionCancelled) },
    };
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK) {
        if (status != ANI_ALREADY_BINDED) {
            TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
            return std::unique_ptr<AppExecFwk::ETSNativeReference>();
        }
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return std::unique_ptr<AppExecFwk::ETSNativeReference>();
    }
    if ((status = env->Object_New(cls, method, &contextObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return std::unique_ptr<AppExecFwk::ETSNativeReference>();
    }
    if ((status = env->Class_FindField(cls, "nativeContext", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return std::unique_ptr<AppExecFwk::ETSNativeReference>();
    }
    if ((status = env->Object_SetField_Long(contextObj, field, nativeContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return std::unique_ptr<AppExecFwk::ETSNativeReference>();
    }
    if ((status = env->Class_FindField(cls, "instanceId", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return std::unique_ptr<AppExecFwk::ETSNativeReference>();
    }
    if ((status = env->Object_SetField_Int(contextObj, field, instanceId)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return std::unique_ptr<AppExecFwk::ETSNativeReference>();
    }
    const auto toolCallId = nativeContext->GetToolCallId();
    if (!toolCallId.empty()) {
        ani_string aniToolCallId = nullptr;
        if ((status = env->String_NewUTF8(toolCallId.c_str(), toolCallId.size(), &aniToolCallId)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "create toolCallId failed, status: %{public}d", status);
            return std::unique_ptr<AppExecFwk::ETSNativeReference>();
        }
        if ((status = env->Object_SetFieldByName_Ref(contextObj, "toolCallId", aniToolCallId)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "set toolCallId failed, status: %{public}d", status);
            return std::unique_ptr<AppExecFwk::ETSNativeReference>();
        }
    }
    if ((status = env->GlobalReference_Create(contextObj, &contextObjtRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return std::unique_ptr<AppExecFwk::ETSNativeReference>();
    }
    auto nativeReference = std::make_unique<AppExecFwk::ETSNativeReference>();
    nativeReference->aniCls = cls;
    nativeReference->aniObj = contextObj;
    nativeReference->aniRef = contextObjtRef;
    return nativeReference;
}

ani_ref CreateEtsInsightIntentCancellationSignal(ani_env *env,
    const std::shared_ptr<InsightIntentCancellationSignal>& signal)
{
    TAG_LOGD(AAFwkTag::INTENT, "CreateEtsInsightIntentCancellationSignal called");
    if (env == nullptr || signal == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env or signal");
        return nullptr;
    }
    ani_class cls {};
    ani_status status = ANI_ERROR;
    ani_object signalObj = nullptr;
    ani_method ctor {};
    if ((status = env->FindClass(SIGNAL_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    std::array functions = {
        ani_native_function { "isCancelled", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentCancellationSignal::IsCancelled) },
        ani_native_function { "setOnCancelListener", SET_ON_CANCEL_LISTENER_SIGNATURE,
            reinterpret_cast<void *>(EtsInsightIntentCancellationSignal::SetOnCancelListener) },
    };
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK) {
        if (status != ANI_ALREADY_BINDED) {
            TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
            return nullptr;
        }
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, ctor, &signalObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    ani_vm *vm = nullptr;
    if (env->GetVM(&vm) != ANI_OK || vm == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get vm failed");
        return nullptr;
    }
    auto wrapper = std::make_shared<EtsInsightIntentCancellationSignal>(vm, signal);
    wrapper->RegisterCancelCallback();
    auto holder = new (std::nothrow) std::shared_ptr<EtsInsightIntentCancellationSignal>(wrapper);
    if (holder == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null cancellation signal holder");
        return nullptr;
    }
    if ((status = env->Object_SetFieldByName_Long(signalObj, "nativeSignal",
        reinterpret_cast<ani_long>(holder))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "set nativeSignal failed, status: %{public}d", status);
        return nullptr;
    }
    return signalObj;
}

ani_ref CreateEtsInsightIntentExecutionContext(ani_env *env,
    const std::shared_ptr<InsightIntentCancellationSignal>& signal)
{
    TAG_LOGD(AAFwkTag::INTENT, "CreateEtsInsightIntentExecutionContext called");
    if (env == nullptr || signal == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env or signal");
        return nullptr;
    }
    ani_ref signalObj = CreateEtsInsightIntentCancellationSignal(env, signal);
    if (signalObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "create cancellation signal failed");
        return nullptr;
    }
    ani_class cls {};
    ani_status status = ANI_ERROR;
    ani_object contextObj = nullptr;
    ani_method ctor {};
    if ((status = env->FindClass(EXECUTION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, ctor, &contextObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_SetFieldByName_Ref(contextObj, "cancellationSignal", signalObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "set signal failed, status: %{public}d", status);
        return nullptr;
    }
    return contextObj;
}
} // namespace AbilityRuntime
} // namespace OHOS
