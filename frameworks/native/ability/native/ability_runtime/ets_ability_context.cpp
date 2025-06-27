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

#include "ability_runtime/ets_ability_context.h"

#include "ani_common_ability_result.h"
#include "ani_common_configuration.h"
#include "ani_common_start_options.h"
#include "ani_common_want.h"
#include "ani_enum_convert.h"
#include "app_utils.h"
#include "common_fun_ani.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ets_context_utils.h"
#include "ets_error_utils.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
std::mutex EtsAbilityContext::requestCodeMutex_;
namespace {
static std::once_flag g_bindNativeMethodsFlag;
constexpr const char *UI_ABILITY_CONTEXT_CLASS_NAME = "Lapplication/UIAbilityContext/UIAbilityContext;";
constexpr const char *CLASSNAME_ASYNC_CALLBACK_WRAPPER = "Lutils/AbilityUtils/AsyncCallbackWrapper;";
const std::string APP_LINKING_ONLY = "appLinkingOnly";
} // namespace

std::shared_ptr<AbilityContext> EtsAbilityContext::GetAbilityContext(ani_env *env, ani_object aniObj)
{
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env or aniObj");
        return nullptr;
    }
    ani_long nativeContextLong;
    ani_status status = env->Object_GetFieldByName_Long(aniObj, "nativeContext", &nativeContextLong);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Object_GetFieldByName_Long status: %{public}d", status);
        return nullptr;
    }
    auto weakContext = reinterpret_cast<std::weak_ptr<AbilityContext> *>(nativeContextLong);
    return weakContext != nullptr ? weakContext->lock() : nullptr;
}

ani_object EtsAbilityContext::SetAbilityContext(ani_env *env, const std::shared_ptr<AbilityContext> &context)
{
    if (env == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env or context");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_object contextObj = nullptr;
    ani_method method = nullptr;
    ani_status status = env->FindClass(UI_ABILITY_CONTEXT_CLASS_NAME, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "FindClass status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":V", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Class_FindMethod status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &contextObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Object_New status: %{public}d", status);
        return nullptr;
    }

    auto workContext = new (std::nothrow) std::weak_ptr<AbilityContext>(context);
    if (workContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "workContext nullptr");
        return nullptr;
    }
    ani_long nativeContextLong = (ani_long)workContext;
    if ((status = env->Object_SetFieldByName_Long(contextObj, "nativeContext", nativeContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Object_SetFieldByName_Long status: %{public}d", status);
        delete workContext;
        workContext = nullptr;
        return nullptr;
    }
    return contextObj;
}

// to be done: free install
void EtsAbilityContext::StartAbility(ani_env *env, ani_object aniObj, ani_object wantObj, ani_object call)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::CONTEXT, "StartAbility called");
    GetInstance().OnStartAbility(env, aniObj, wantObj, nullptr, call);
}

void EtsAbilityContext::StartAbilityWithOptions(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object opt, ani_object call)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::CONTEXT, "StartAbilityWithOptions called");
    GetInstance().OnStartAbility(env, aniObj, wantObj, opt, call);
}

// to be done: free install
void EtsAbilityContext::StartAbilityForResult(ani_env *env, ani_object aniObj, ani_object wantObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "StartAbilityForResult called");
    GetInstance().OnStartAbilityForResult(env, aniObj, wantObj, nullptr, callback);
}

// to be done: free install
void EtsAbilityContext::StartAbilityForResultWithOptions(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object startOptionsObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "StartAbilityForResultWithOptions called");
    GetInstance().OnStartAbilityForResult(env, aniObj, wantObj, startOptionsObj, callback);
}

void EtsAbilityContext::TerminateSelf(ani_env *env, ani_object aniObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "TerminateSelf called");
    GetInstance().OnTerminateSelf(env, aniObj, callback);
}

void EtsAbilityContext::TerminateSelfWithResult(
    ani_env *env, ani_object aniObj, ani_object abilityResult, ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "TerminateSelfWithResult called");
    GetInstance().OnTerminateSelfWithResult(env, aniObj, abilityResult, callback);
}

void EtsAbilityContext::ReportDrawnCompleted(ani_env *env, ani_object aniObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "ReportDrawnCompleted called");
    GetInstance().OnReportDrawnCompleted(env, aniObj, callback);
}

int32_t EtsAbilityContext::GenerateRequestCode()
{
    static int32_t curRequestCode_ = 0;
    std::lock_guard lock(requestCodeMutex_);
    curRequestCode_ = (curRequestCode_ == INT_MAX) ? 0 : (curRequestCode_ + 1);
    return curRequestCode_;
}

void EtsAbilityContext::InheritWindowMode(ani_env *env, ani_object aniObj, AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "InheritWindowMode");
#ifdef SUPPORT_SCREEN
    // only split mode need inherit
    auto context = GetAbilityContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "context null");
        return;
    }
    auto windowMode = context->GetCurrentWindowMode();
    if (AAFwk::AppUtils::GetInstance().IsInheritWindowSplitScreenMode() &&
        (windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
            windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY)) {
        want.SetParam(AAFwk::Want::PARAM_RESV_WINDOW_MODE, windowMode);
    }
    TAG_LOGD(AAFwkTag::CONTEXT, "window mode is %{public}d", windowMode);
#endif
}

bool EtsAbilityContext::AsyncCallback(ani_env *env, ani_object call, ani_object error, ani_object result)
{
    if (env == nullptr || call == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "env or call is nullptr");
        return false;
    }
    ani_class clsCall = nullptr;
    ani_status status = env->FindClass(CLASSNAME_ASYNC_CALLBACK_WRAPPER, &clsCall);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "FindClass status: %{public}d", status);
        return false;
    }
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(clsCall, "invoke", "L@ohos/base/BusinessError;Lstd/core/Object;:V", &method)) !=
        ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Class_FindMethod status: %{public}d", status);
        return false;
    }
    if (error == nullptr) {
        ani_ref nullRef = nullptr;
        env->GetNull(&nullRef);
        error = reinterpret_cast<ani_object>(nullRef);
    }
    if (result == nullptr) {
        ani_ref undefinedRef = nullptr;
        env->GetUndefined(&undefinedRef);
        result = reinterpret_cast<ani_object>(undefinedRef);
    }
    if ((status = env->Object_CallMethod_Void(call, method, error, result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
        return false;
    }
    return true;
}

void EtsAbilityContext::OnStartAbility(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object opt, ani_object call)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AAFwk::Want want;
    if (!OHOS::AppExecFwk::UnwrapWant(env, wantObj, want)) {
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, must be a Want");
        return;
    }
    InheritWindowMode(env, aniObj, want);
    auto context = EtsAbilityContext::GetAbilityContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        EtsErrorUtil::ThrowInvalidParamError(env, "null context");
        return;
    }
    if ((want.GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND) {
        std::string startTime = std::to_string(
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
                .count());
        want.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);
    }
    ErrCode innerErrCode = ERR_OK;
    if (opt != nullptr) {
        AAFwk::StartOptions startOptions;
        if (!OHOS::AppExecFwk::UnwrapStartOptions(env, opt, startOptions)) {
            EtsErrorUtil::ThrowInvalidParamError(env,
                "Parse param startOptions failed, startOptions must be StartOptions.");
            TAG_LOGE(AAFwkTag::CONTEXT, "invalid options");
            return;
        }
        innerErrCode = context->StartAbility(want, startOptions, -1);
    } else {
        innerErrCode = context->StartAbility(want, -1);
    }
    ani_object aniObject = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK);
    if (innerErrCode != ERR_OK) {
        aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, innerErrCode);
    }
    if ((want.GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND) {
        // to be done: free install
    } else {
        AsyncCallback(env, call, aniObject, nullptr);
    }
}

// to be done: free install
void EtsAbilityContext::OnStartAbilityForResult(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object startOptionsObj, ani_object callback)
{
    auto context = EtsAbilityContext::GetAbilityContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetAbilityContext is nullptr");
        EtsErrorUtil::ThrowErrorByNativeErr(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
        return;
    }
    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(env, wantObj, want);
    AAFwk::StartOptions startOptions;
    if (startOptionsObj) {
        OHOS::AppExecFwk::UnwrapStartOptions(env, startOptionsObj, startOptions);
    }
    TAG_LOGE(AAFwkTag::CONTEXT, "displayId:%{public}d", startOptions.GetDisplayID());
    std::string startTime = std::to_string(
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count());
    ani_ref callbackRef = nullptr;
    env->GlobalReference_Create(callback, &callbackRef);
    ani_vm *etsVm = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GetVM(&etsVm)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
        return;
    }
    RuntimeTask task = [etsVm, callbackRef, element = want.GetElement(), flags = want.GetFlags(), startTime](
                           int resultCode, const AAFwk::Want &want, bool isInner) {
        TAG_LOGD(AAFwkTag::CONTEXT, "start async callback");
        ani_status status = ANI_ERROR;
        ani_env *env = nullptr;
        if ((status = etsVm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
            return;
        }
        std::string bundleName = element.GetBundleName();
        std::string abilityName = element.GetAbilityName();
        ani_object abilityResult = AppExecFwk::WrapAbilityResult(env, resultCode, want);
        if (abilityResult == nullptr) {
            TAG_LOGW(AAFwkTag::CONTEXT, "null abilityResult");
            isInner = true;
            resultCode = ERR_INVALID_VALUE;
        }
        auto errCode = isInner ? resultCode : 0;
        AsyncCallback(env, reinterpret_cast<ani_object>(callbackRef),
            OHOS::AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, errCode), abilityResult);
    };
    auto requestCode = GenerateRequestCode();
    (startOptionsObj == nullptr) ? context->StartAbilityForResult(want, requestCode, std::move(task))
                                 : context->StartAbilityForResult(want, startOptions, requestCode, std::move(task));
    return;
}

void EtsAbilityContext::OnTerminateSelf(ani_env *env, ani_object aniObj, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env or aniObj");
        ani_object aniObject = EtsErrorUtil::CreateInvalidParamError(env, "env null");
        AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    ani_object aniObject = nullptr;
    auto context = EtsAbilityContext::GetAbilityContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "context null");
        AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    ErrCode ret = context->TerminateSelf();
    if (ret == static_cast<ErrCode>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT) || ret == ERR_OK) {
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ret));
    } else {
        aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
    }
    AsyncCallback(env, callback, aniObject, nullptr);
}

void EtsAbilityContext::OnTerminateSelfWithResult(
    ani_env *env, ani_object aniObj, ani_object abilityResult, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        ani_object aniObject = EtsErrorUtil::CreateInvalidParamError(env, "env null");
        AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    ani_object aniObject = nullptr;
    auto context = EtsAbilityContext::GetAbilityContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetAbilityContext is nullptr");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "context null");
        AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    AAFwk::Want want;
    int resultCode = 0;
    OHOS::AppExecFwk::UnWrapAbilityResult(env, abilityResult, resultCode, want);
    context->SetTerminating(true);
    ErrCode ret = context->TerminateAbilityWithResult(want, resultCode);
    if (ret == static_cast<ErrCode>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT) || ret == ERR_OK) {
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ret));
    } else {
        aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
    }
    AsyncCallback(env, callback, aniObject, nullptr);
}

void EtsAbilityContext::OnReportDrawnCompleted(ani_env *env, ani_object aniObj, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        ani_object aniObject = EtsErrorUtil::CreateInvalidParamError(env, "env null");
        AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    ani_object aniObject = nullptr;
    auto context = GetAbilityContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "context null");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "context null");
        AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    ErrCode ret = context->ReportDrawnCompleted();
    if (ret == ERR_OK) {
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ret));
    } else {
        aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
    }
    AsyncCallback(env, callback, aniObject, nullptr);
}

void EtsAbilityContext::OnStartServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "call");
    ani_object errorObject = nullptr;
    ErrCode ret = ERR_OK;
    auto context = EtsAbilityContext::GetAbilityContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UIABICONTEXTLITY, "GetAbilityContext is nullptr");
        ret = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        errorObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ret));
        AppExecFwk::AsyncCallback(env, callbackobj, errorObject, nullptr);
        return;
    }
    AAFwk::Want want;
    if (!OHOS::AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "UnwrapWant filed");
        errorObject = EtsErrorUtil::CreateInvalidParamError(env, "UnwrapWant filed");
        AppExecFwk::AsyncCallback(env, callbackobj, errorObject, nullptr);
    }
    ret = context->StartServiceExtensionAbility(want);
    if (ret == ERR_OK) {
        errorObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ret));
    } else {
        errorObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
    }
    AppExecFwk::AsyncCallback(env, callbackobj, errorObject, nullptr);
}

void EtsAbilityContext::OnOpenLinkSync(ani_env *env, ani_object aniObj, ani_string aniLink,
    ani_object myCallbackobj, ani_object optionsObj, ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OnOpenLinkSync");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_boolean isOptionsUndefined = true;
    if ((status = env->Reference_IsUndefined(optionsObj, &isOptionsUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
    }
    ani_boolean isCallbackUndefined = true;
    if ((status = env->Reference_IsUndefined(callbackobj, &isCallbackUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
    }
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
    }
    GetInstance().OpenLinkInner(env, aniObj, aniLink, myCallbackobj, optionsObj, callbackobj,
        !isOptionsUndefined, !isCallbackUndefined);
}

bool EtsAbilityContext::OnIsTerminating(ani_env *env, ani_object aniObj)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "OnIsTerminating");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return false;
    }
    auto context = EtsAbilityContext::GetAbilityContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return false;
    }
    return context->IsTerminating();
}

void EtsAbilityContext::OnMoveAbilityToBackground(ani_env *env, ani_object aniObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "OnMoveAbilityToBackground");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    auto context = EtsAbilityContext::GetAbilityContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)), nullptr);
        return;
    }
    ErrCode ret = ERR_OK;
    ani_object errorObject = nullptr;
    ret = context->MoveUIAbilityToBackground();
    errorObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
    AppExecFwk::AsyncCallback(env, callback, errorObject, nullptr);
}

void EtsAbilityContext::OnRequestModalUIExtension(ani_env *env, ani_object aniObj,
    ani_string pickerWantObj, ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "OnRequestModalUIExtension");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    ani_object errorObject = nullptr;
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, pickerWantObj, want)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "parse want failed");
        ThrowETSInvalidParamError(env, "Parse param want failed, want must be Want.");
        return;
    }
    auto context = EtsAbilityContext::GetAbilityContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        errorObject = EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
        AppExecFwk::AsyncCallback(env, callbackObj, errorObject, nullptr);
        return;
    }

    ErrCode ret = ERR_OK;
    ret = AAFwk::AbilityManagerClient::GetInstance()->RequestModalUIExtension(want);
    errorObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
    AppExecFwk::AsyncCallback(env, callbackObj, errorObject, nullptr);
}

namespace {
bool BindNativeMethods(ani_env *env, ani_class &cls)
{
    ani_status status = env->FindClass(UI_ABILITY_CONTEXT_CLASS_NAME, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "FindClass status: %{public}d", status);
        return false;
    }
    std::call_once(g_bindNativeMethodsFlag, [&status, env, cls]() {
        std::array functions = {
            ani_native_function { "nativeStartAbilitySync",
                "L@ohos/app/ability/Want/Want;Lapplication/UIAbilityContext/AsyncCallbackWrapper;:V",
                reinterpret_cast<void *>(EtsAbilityContext::StartAbility) },
            ani_native_function { "nativeStartAbilitySync",
                "L@ohos/app/ability/Want/Want;L@ohos/app/ability/StartOptions/StartOptions;Lapplication/"
                "UIAbilityContext/AsyncCallbackWrapper;:V",
                reinterpret_cast<void *>(EtsAbilityContext::StartAbilityWithOptions) },
            ani_native_function { "nativeStartAbilityForResult",
                "L@ohos/app/ability/Want/Want;Lapplication/UIAbilityContext/AsyncCallbackWrapper;:V",
                reinterpret_cast<void *>(EtsAbilityContext::StartAbilityForResult) },
            ani_native_function { "nativeStartAbilityForResult",
                "L@ohos/app/ability/Want/Want;L@ohos/app/ability/StartOptions/StartOptions;Lapplication/"
                "UIAbilityContext/AsyncCallbackWrapper;:V",
                reinterpret_cast<void *>(EtsAbilityContext::StartAbilityForResultWithOptions) },
            ani_native_function { "nativeTerminateSelfSync", "Lapplication/UIAbilityContext/AsyncCallbackWrapper;:V",
                reinterpret_cast<void *>(EtsAbilityContext::TerminateSelf) },
            ani_native_function { "nativeTerminateSelfWithResult",
                "Lability/abilityResult/AbilityResult;Lapplication/UIAbilityContext/AsyncCallbackWrapper;:V",
                reinterpret_cast<void *>(EtsAbilityContext::TerminateSelfWithResult) },
            ani_native_function { "nativeReportDrawnCompletedSync",
                "Lapplication/UIAbilityContext/AsyncCallbackWrapper;:V",
                reinterpret_cast<ani_int *>(EtsAbilityContext::ReportDrawnCompleted) },
            ani_native_function { "nativeStartServiceExtensionAbilitySync", nullptr,
                reinterpret_cast<void*>(EtsAbilityContext::OnStartServiceExtensionAbility) },
            ani_native_function { "nativeIsTerminating", nullptr,
                reinterpret_cast<void*>(EtsAbilityContext::OnIsTerminating) },
            ani_native_function { "nativeMoveAbilityToBackground", nullptr,
                reinterpret_cast<void*>(EtsAbilityContext::OnMoveAbilityToBackground) },
            ani_native_function { "nativeRequestModalUIExtension", nullptr,
                reinterpret_cast<void*>(EtsAbilityContext::OnRequestModalUIExtension) },
        };
        status = env->Class_BindNativeMethods(cls, functions.data(), functions.size());
    });
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Class_BindNativeMethods status: %{public}d", status);
        return false;
    }
    return true;
}
} // namespace

ani_object CreateEtsAbilityContext(
    ani_env *env, const std::shared_ptr<AbilityContext> &context, const std::shared_ptr<OHOSApplication> &application)
{
    if (env == nullptr || context == nullptr || application == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env or context or application");
        return nullptr;
    }
    ani_class cls = nullptr;
    if (!BindNativeMethods(env, cls)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "BindNativeMethods failed");
        return nullptr;
    }
    ani_object contextObj = EtsAbilityContext::SetAbilityContext(env, context);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null contextObj");
        return nullptr;
    }
    ContextUtil::CreateEtsBaseContext(env, cls, contextObj, context);

    auto abilityInfo = context->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null abilityInfo");
        return nullptr;
    }
    ani_ref abilityInfoRef = AppExecFwk::CommonFunAni::ConvertAbilityInfo(env, *abilityInfo);
    ani_status status = env->Object_SetFieldByName_Ref(contextObj, "abilityInfo", abilityInfoRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Object_SetFieldByName_Ref status: %{public}d", status);
        return nullptr;
    }

    auto configuration = context->GetConfiguration();
    if (configuration == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null configuration");
        return nullptr;
    }
    ani_object configurationObj = OHOS::AppExecFwk::WrapConfiguration(env, *configuration);
    if ((status = env->Object_SetFieldByName_Ref(contextObj, "config", configurationObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Object_SetFieldByName_Ref status: %{public}d", status);
        return nullptr;
    }
    return contextObj;
}

void EtsAbilityContext::AddFreeInstallObserver(ani_env *env, const AAFwk::Want &want, ani_object callback,
    const std::shared_ptr<AbilityContext> &context, bool isAbilityResult, bool isOpenLink)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "AddFreeInstallObserver");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return;
    }
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return;
    }
    if (freeInstallObserver_ == nullptr) {
        ani_vm *etsVm = nullptr;
        ani_status status = ANI_ERROR;
        if ((status = env->GetVM(&etsVm)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
        }
        if (etsVm == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "null etsVm");
            return;
        }
        freeInstallObserver_ = new (std::nothrow) EtsFreeInstallObserver(etsVm);
        if (context->AddFreeInstallObserver(freeInstallObserver_) != ERR_OK) {
            TAG_LOGE(AAFwkTag::CONTEXT, "addFreeInstallObserver error");
            return;
        }
    }
    std::string startTime = want.GetStringParam(AAFwk::Want::PARAM_RESV_START_TIME);
    if (isOpenLink) {
        std::string url = want.GetUriString();
        freeInstallObserver_->AddEtsObserverObject(env, startTime, url, callback, isAbilityResult);
        return;
    }
    std::string bundleName = want.GetElement().GetBundleName();
    std::string abilityName = want.GetElement().GetAbilityName();
    freeInstallObserver_->AddEtsObserverObject(env, bundleName, abilityName, startTime, callback, isAbilityResult);
}

void EtsAbilityContext::OpenLinkInner(ani_env *env, ani_object aniObj, ani_string aniLink, ani_object myCallbackobj,
    ani_object optionsObj, ani_object callbackobj, bool haveOptionsParm, bool haveCallBackParm)
{
    ani_object aniObject = nullptr;
    std::string link("");
    AAFwk::OpenLinkOptions openLinkOptions;
    AAFwk::Want want;
    want.SetParam(APP_LINKING_ONLY, false);
    if (!AppExecFwk::GetStdString(env, aniLink, link)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "parse link failed");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "Parse param link failed, link must be string.");
        AppExecFwk::AsyncCallback(env, myCallbackobj, aniObject, nullptr);
        return;
    }
    if (haveOptionsParm) {
        TAG_LOGD(AAFwkTag::UIABILITY, "OpenLink Have option");
        EtsAbilityContext::UnWrapOpenLinkOptions(env, optionsObj, openLinkOptions, want);
    }
    want.SetUri(link);
    std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count());
    want.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);
    int requestCode = -1;
    ErrCode ErrCode = ERR_OK;
    auto context = EtsAbilityContext::GetAbilityContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "GetAbilityContext is nullptr");
        ErrCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ErrCode));
        AppExecFwk::AsyncCallback(env, myCallbackobj, aniObject, nullptr);
        return;
    }
    AddFreeInstallObserver(env, want, myCallbackobj, context, true);
    if (haveCallBackParm) {
        TAG_LOGD(AAFwkTag::UIABILITY, "OpenLink Have Callback");
        CreateOpenLinkTask(env, callbackobj, context, want, requestCode);
    }
    ErrCode = context->OpenLink(want, requestCode);
    if (ErrCode == AAFwk::ERR_OPEN_LINK_START_ABILITY_DEFAULT_OK) {
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ERR_OK));
    } else {
        aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ErrCode));
    }
    AppExecFwk::AsyncCallback(env, myCallbackobj, aniObject, nullptr);
}

void EtsAbilityContext::UnWrapOpenLinkOptions(ani_env *env, ani_object optionsObj,
    AAFwk::OpenLinkOptions &openLinkOptions, AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "UnWrapOpenLinkOptions");
    ani_status status = ANI_ERROR;
    ani_ref ParamRef = nullptr;
    if ((status = env->Object_GetPropertyByName_Ref(optionsObj, APP_LINKING_ONLY.c_str(), &ParamRef))  == ANI_OK) {
        bool appLinkingOnly = AppExecFwk::GetBoolOrUndefined(env, optionsObj, "appLinkingOnly");
        openLinkOptions.SetAppLinkingOnly(appLinkingOnly);
        want.SetParam(APP_LINKING_ONLY, appLinkingOnly);
    } else {
        TAG_LOGE(AAFwkTag::UIABILITY, "status: %{public}d", status);
    }
    if ((status = env->Object_GetPropertyByName_Ref(optionsObj, "parameters", &ParamRef))  == ANI_OK) {
        AAFwk::WantParams wantParam;
        if (AppExecFwk::UnwrapWantParams(env, ParamRef, wantParam)) {
            want.SetParams(wantParam);
        } else {
            TAG_LOGE(AAFwkTag::UIABILITY, "UnwrapWantParams failed");
        }
    } else {
        TAG_LOGE(AAFwkTag::UIABILITY, "status: %{public}d", status);
    }
    if (!want.HasParameter(APP_LINKING_ONLY)) {
        want.SetParam(APP_LINKING_ONLY, false);
    }
}

void EtsAbilityContext::CreateOpenLinkTask(ani_env *env, const ani_object callbackobj,
    std::shared_ptr<AbilityContext> context, AAFwk::Want &want, int &requestCode)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "CreateOpenLinkTask");
    want.SetParam(AAFwk::Want::PARAM_RESV_FOR_RESULT, true);
    ani_vm *etsVm = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GetVM(&etsVm)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
        return;
    }
    ani_ref callbackRef = nullptr;
    if ((status = env->GlobalReference_Create(callbackobj, &callbackRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    RuntimeTask task = [etsVm, callbackRef] (int resultCode, const AAFwk::Want &want, bool isInner) {
    TAG_LOGD(AAFwkTag::CONTEXT, "start async callback");
    ani_status status = ANI_ERROR;
    ani_env *env = nullptr;
    if ((status = etsVm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "status: %{public}d", status);
        return;
    }
    ani_object abilityResult = AppExecFwk::WrapAbilityResult(env, resultCode, want);
    if (abilityResult == nullptr) {
        TAG_LOGW(AAFwkTag::CONTEXT, "null abilityResult");
        isInner = true;
        resultCode = ERR_INVALID_VALUE;
    }
    auto errCode = isInner ? resultCode : 0;
    AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callbackRef),
        EtsErrorUtil::CreateErrorByNativeErr(env, errCode), abilityResult);
    };
    requestCode = GenerateRequestCode();
    context->InsertResultCallbackTask(requestCode, std::move(task));
}
} // namespace AbilityRuntime
} // namespace OHOS
