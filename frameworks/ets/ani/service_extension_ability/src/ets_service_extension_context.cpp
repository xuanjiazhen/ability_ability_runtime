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
#include "ets_service_extension_context.h"

#include "ability_manager_client.h"
#include "ani_common_start_options.h"
#include "ani_common_want.h"
#include "common_fun_ani.h"
#include "ets_context_utils.h"
#include "ets_error_utils.h"
#include "ets_extension_context.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *SERVICE_EXTENSION_CONTEXT_CLASS_NAME =
    "Lapplication/ServiceExtensionContext/ServiceExtensionContext;";
constexpr const char *CLEANER_CLASS_NAME =
    "Lapplication/ServiceExtensionContext/Cleaner;";
constexpr const int ANI_ALREADY_BINDED = 8;

bool BindNativeMethods(ani_env *env, ani_class &cls)
{
    ani_status status = ANI_ERROR;
    std::array functions = {
        ani_native_function { "nativeTerminateSelf", "Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsServiceExtensionContext::TerminateSelf) },
        ani_native_function { "nativeStartAbility",
            "L@ohos/app/ability/Want/Want;Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsServiceExtensionContext::StartAbility) },
        ani_native_function { "nativeStartAbility", "L@ohos/app/ability/Want/Want;"
            "L@ohos/app/ability/StartOptions/StartOptions;Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsServiceExtensionContext::StartAbilityWithOption) },
        ani_native_function { "nativeStartServiceExtensionAbility",
            "L@ohos/app/ability/Want/Want;Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsServiceExtensionContext::StartServiceExtensionAbility) },
        ani_native_function { "nativeStopServiceExtensionAbility",
            "L@ohos/app/ability/Want/Want;Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsServiceExtensionContext::StopServiceExtensionAbility) },
    };
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::UI_EXT, "bind method status : %{public}d", status);
        return false;
    }
    ani_class cleanerCls = nullptr;
    status = env->FindClass(CLEANER_CLASS_NAME, &cleanerCls);
    if (status != ANI_OK || cleanerCls == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to find class, status : %{public}d", status);
        return false;
    }
    std::array CleanerMethods = {
        ani_native_function { "clean", nullptr, reinterpret_cast<void *>(EtsServiceExtensionContext::Finalizer) },
    };
    if ((status = env->Class_BindNativeMethods(cleanerCls, CleanerMethods.data(), CleanerMethods.size())) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "bind method status : %{public}d", status);
        return false;
    }
    return true;
}
} // namespace

void EtsServiceExtensionContext::Finalizer(ani_env *env, ani_object obj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "TerminateSelf");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    ani_long nativeEtsContextPtr;
    if (env->Object_GetFieldByName_Long(obj, "nativeEtsContext", &nativeEtsContextPtr) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to get nativeEtsContext");
        return;
    }
    if (nativeEtsContextPtr != 0) {
        delete reinterpret_cast<EtsServiceExtensionContext *>(nativeEtsContextPtr);
    }
}

void EtsServiceExtensionContext::TerminateSelf(ani_env *env, ani_object aniObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "TerminateSelf");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return;
    }
    etsServiceExtensionContext->OnTerminateSelf(env, aniObj, callback);
}

void EtsServiceExtensionContext::StartServiceExtensionAbility(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "StartServiceExtensionAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return;
    }
    etsServiceExtensionContext->OnStartServiceExtensionAbility(env, aniObj, wantObj, callbackobj);
}

void EtsServiceExtensionContext::StartAbility(ani_env *env, ani_object aniObj, ani_object wantObj, ani_object call)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "StartAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return;
    }
    etsServiceExtensionContext->OnStartAbility(env, aniObj, wantObj, nullptr, call);
}

void EtsServiceExtensionContext::StartAbilityWithOption(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object opt, ani_object call)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "StartAbilityWithOption");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return;
    }
    etsServiceExtensionContext->OnStartAbility(env, aniObj, wantObj, opt, call);
}

void EtsServiceExtensionContext::StopServiceExtensionAbility(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnStopServiceExtensionAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return;
    }
    etsServiceExtensionContext->OnStopServiceExtensionAbility(env, aniObj, wantObj, callbackobj);
}

EtsServiceExtensionContext *EtsServiceExtensionContext::GetEtsAbilityContext(
    ani_env *env, ani_object aniObj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "GetEtsAbilityContext");
    ani_class cls = nullptr;
    ani_long nativeContextLong;
    ani_field contextField = nullptr;
    ani_status status = ANI_ERROR;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(SERVICE_EXTENSION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to find class, status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindField(cls, "nativeEtsContext", &contextField)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to find filed, status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_GetField_Long(aniObj, contextField, &nativeContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to get filed, status : %{public}d", status);
        return nullptr;
    }
    auto weakContext = reinterpret_cast<EtsServiceExtensionContext *>(nativeContextLong);
    return weakContext;
}

void EtsServiceExtensionContext::OnTerminateSelf(ani_env *env, ani_object obj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnTerminateSelf");
    ani_object aniObject = nullptr;
    ErrCode ret = ERR_INVALID_VALUE;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "context is nullptr");
        ret = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ret));
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    ret = context->TerminateAbility();
    AppExecFwk::AsyncCallback(env, callback,
        EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
}

void EtsServiceExtensionContext::OnStartServiceExtensionAbility(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnStartServiceExtensionAbility");
    ani_object aniObject = nullptr;
    ErrCode ret = ERR_OK;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "context is nullptr");
        ret = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ret));
        AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "UnwrapWant failed");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "UnwrapWant failed");
        AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
        return;
    }
    ret = context->StartServiceExtensionAbility(want);
    aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
    AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
}

void EtsServiceExtensionContext::OnStopServiceExtensionAbility(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnStopServiceExtensionAbility");
    ani_object aniObject = nullptr;
    ErrCode ret = ERR_OK;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "context is nullptr");
        ret = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ret));
        AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "UnwrapWant failed");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "UnwrapWant failed");
        AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
        return;
    }
    ret = context->StopServiceExtensionAbility(want);
    aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
    AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
}

void EtsServiceExtensionContext::OnStartAbility(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object opt, ani_object callbackObj)
{
    ani_object aniObject = nullptr;
    AAFwk::Want want;
    ErrCode errCode = ERR_OK;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "UnwrapWant filed");
        AppExecFwk::AsyncCallback(env, callbackObj, aniObject, nullptr);
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "context is nullptr");
        errCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(errCode));
        AppExecFwk::AsyncCallback(env, callbackObj, aniObject, nullptr);
        return;
    }
    if ((want.GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND) {
        std::string startTime = std::to_string(
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
                .count());
        want.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);
        AddFreeInstallObserver(env, want, callbackObj, context);
    }
    if (opt != nullptr) {
        AAFwk::StartOptions startOptions;
        if (!AppExecFwk::UnwrapStartOptions(env, opt, startOptions)) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "UnwrapStartOptions filed");
            aniObject = EtsErrorUtil::CreateInvalidParamError(env, "UnwrapWant filed");
            AppExecFwk::AsyncCallback(env, callbackObj, aniObject, nullptr);
            return;
        }
        errCode = context->StartAbility(want, startOptions);
    } else {
        errCode = context->StartAbility(want);
    }
    aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, errCode);
    if ((want.GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND) {
        if (errCode != ERR_OK && freeInstallObserver_ != nullptr) {
            std::string bundleName = want.GetElement().GetBundleName();
            std::string abilityName = want.GetElement().GetAbilityName();
            std::string startTime = want.GetStringParam(AAFwk::Want::PARAM_RESV_START_TIME);
            freeInstallObserver_->OnInstallFinished(bundleName, abilityName, startTime, errCode);
        }
    } else {
        AppExecFwk::AsyncCallback(env, callbackObj, aniObject, nullptr);
    }
}

void EtsServiceExtensionContext::AddFreeInstallObserver(
    ani_env *env, const AAFwk::Want &want, ani_object callbackObj, std::shared_ptr<ServiceExtensionContext> context)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "AddFreeInstallObserver");
    if (!context) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        return;
    }
    if (!env) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    if (freeInstallObserver_ == nullptr) {
        ani_vm *etsVm = nullptr;
        ani_status status = ANI_ERROR;
        if ((status = env->GetVM(&etsVm)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
            return;
        }
        freeInstallObserver_ = new EtsFreeInstallObserver(etsVm);
        if (context->AddFreeInstallObserver(freeInstallObserver_)) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "addFreeInstallObserver error");
            return;
        }
    }
    std::string startTime = want.GetStringParam(AAFwk::Want::PARAM_RESV_START_TIME);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "addEtsObserver");
    std::string bundleName = want.GetElement().GetBundleName();
    std::string abilityName = want.GetElement().GetAbilityName();
    freeInstallObserver_->AddEtsObserverObject(env, bundleName, abilityName, startTime, callbackObj);
}

ani_object CreateEtsServiceExtensionContext(ani_env *env, std::shared_ptr<ServiceExtensionContext> context)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "CreateEtsServiceExtensionContext");
    if (env == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env or context");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object contextObj = nullptr;
    if ((env->FindClass(SERVICE_EXTENSION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to find class, status : %{public}d", status);
        return nullptr;
    }
    if (!BindNativeMethods(env, cls)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to BindNativeMethods");
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", "J:V", &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to find constructor, status : %{public}d", status);
        return nullptr;
    }
    std::unique_ptr<EtsServiceExtensionContext> workContext = std::make_unique<EtsServiceExtensionContext>(context);
    if (workContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to create etsServiceExtensionContext");
        return nullptr;
    }
    auto serviceContextPtr = new std::weak_ptr<ServiceExtensionContext> (workContext->GetAbilityContext());
    if ((status = env->Object_New(cls, method, &contextObj, (ani_long)workContext.release())) != ANI_OK ||
        contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to create object, status : %{public}d", status);
        return nullptr;
    }
    if (!ContextUtil::SetNativeContextLong(env, contextObj, (ani_long)(serviceContextPtr))) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to setNativeContextLong ");
        return nullptr;
    }
    ContextUtil::CreateEtsBaseContext(env, cls, contextObj, context);
    CreateEtsExtensionContext(env, cls, contextObj, context, context->GetAbilityInfo());
    return contextObj;
}

void EtsServiceExtensionContext::UnwrapCompletionHandlerInStartOptions(ani_env* env, ani_object param,
    AAFwk::StartOptions& options)
{
    TAG_LOGI(AAFwkTag::SERVICE_EXT, "UnwrapCompletionHandlerInStartOptions called");
    ani_ref completionHandler;
    ani_ref onRequestSuccObj;
    ani_ref onRequestFailObj;
    if (!ExtractCompletionHandler(env, param, completionHandler, onRequestSuccObj, onRequestFailObj)) {
        return;
    }
    if (!CreateAndRegisterCallback(env, completionHandler, onRequestSuccObj,onRequestFailObj, options)) {
        return;
    }
}

bool EtsServiceExtensionContext::ExtractCompletionHandler(ani_env* env, ani_object param,
    ani_ref& completionHandler, ani_ref& onRequestSuccObj, ani_ref& onRequestFailObj)
{
    if (env->Object_GetPropertyByName_Ref(param, "completionHandler", &completionHandler) != ANI_OK ||
        !completionHandler) {
        TAG_LOGD(AAFwkTag::SERVICE_EXT, "null completionHandler");
        return false;
    }
    TAG_LOGI(AAFwkTag::SERVICE_EXT, "completionHandler exists");
    ani_object handlerObj = reinterpret_cast<ani_object>(completionHandler);
    if (env->Object_GetPropertyByName_Ref(handlerObj, "onRequestSuccess", &onRequestSuccObj) != ANI_OK ||
        !onRequestSuccObj) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null onRequestSuccess");
        return false;
    }
    if (env->Object_GetPropertyByName_Ref(handlerObj, "onRequestFailure", &onRequestFailObj) != ANI_OK ||
        !onRequestFailObj) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null onRequestFailure");
        return false;
    }
    return true;
}

bool EtsServiceExtensionContext::CreateAndRegisterCallback(ani_env* env, ani_ref completionHandler,
    ani_ref onRequestSuccObj, ani_ref onRequestFailObj, AAFwk::StartOptions& options)
{
    ani_ref globalOnRequestSucc = nullptr;
    if (env->GlobalReference_Create(onRequestSuccObj, &globalOnRequestSucc) != ANI_OK ||
        !globalOnRequestSucc) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to create global ref for onRequestSuccess");
        return false;
    }
    ani_ref globalOnRequestFail = nullptr;
    if (env->GlobalReference_Create(onRequestFailObj, &globalOnRequestFail) != ANI_OK ||
        !globalOnRequestFail) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to create global ref for onRequestFailure");
        env->GlobalReference_Delete(globalOnRequestSucc);
        return false;
    }
    auto globalOnRequestSuccPtr = std::make_shared<ani_ref>(globalOnRequestSucc);
    auto globalOnRequestFailPtr = std::make_shared<ani_ref>(globalOnRequestFail);
    OnRequestResult onRequestSucc = [env, globalOnRequestSuccPtr](const AppExecFwk::ElementName& element,
        const std::string& message) {
        ani_object elementObj = WrapElementName(env, element);
        if (!elementObj) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "WrapElementName failed");
            return;
        }
        ani_string messageStr = nullptr;
        if (env->String_NewUTF8(message.c_str(), message.size(), &messageStr) != ANI_OK || !messageStr) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "String_NewUTF8 for message failed");
            return;
        }
        ani_class cls = nullptr;
        if (env->FindClass("Lohos/app/ElementName;", &cls) != ANI_OK || !cls) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "FindClass failed");
            return;
        }
        ani_method method = nullptr;
        if (env->Class_FindMethod(cls, "onRequestSuccess",
                                 "(Lohos/app/ElementName;Lstd/core/String;)V", &method) != ANI_OK || !method) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "Find method onRequestSuccess failed");
            return;
        }
        ani_object globalOnRequestSuccObj = reinterpret_cast<ani_object>(*globalOnRequestSuccPtr);
        env->Object_CallMethod_Void(globalOnRequestSuccObj, method, elementObj, messageStr);
    };
    OnRequestResult onRequestFail = [env, globalOnRequestFailPtr](const AppExecFwk::ElementName& element,
        const std::string& message) {
        ani_object elementObj = WrapElementName(env, element);
        if (!elementObj) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "WrapElementName failed");
            return;
        }
        ani_string messageStr = nullptr;
        if (env->String_NewUTF8(message.c_str(), message.size(), &messageStr) != ANI_OK || !messageStr) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "String_NewUTF8 for message failed");
            return;
        }
        ani_class cls = nullptr;
        if (env->FindClass("Lohos/app/ElementName;", &cls) != ANI_OK || !cls) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "FindClass failed");
            return;
        }
        ani_method method = nullptr;
        if (env->Class_FindMethod(cls, "onRequestFailure",
                                 "(Lohos/app/ElementName;Lstd/core/String;)V", &method) != ANI_OK || !method) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "Find method onRequestFailure failed");
            return;
        }
        ani_object globalOnRequestFailObj = reinterpret_cast<ani_object>(*globalOnRequestFailPtr);
        env->Object_CallMethod_Void(globalOnRequestFailObj, method, elementObj, messageStr);
    };
    auto context = context_.lock();
    if (!context) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        env->GlobalReference_Delete(globalOnRequestSucc);
        env->GlobalReference_Delete(globalOnRequestFail);
        return false;
    }
    uint64_t time = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count());
    std::string requestId = std::to_string(time);
    if (context->AddCompletionHandler(requestId, onRequestSucc, onRequestFail) != ERR_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "add completionHandler failed");
        env->GlobalReference_Delete(globalOnRequestSucc);
        env->GlobalReference_Delete(globalOnRequestFail);
        return false;
    }
    options.requestId_ = requestId;
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
