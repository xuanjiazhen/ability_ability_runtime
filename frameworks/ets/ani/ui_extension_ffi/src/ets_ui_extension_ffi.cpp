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

#include "ets_ui_extension_ffi.h"

#include "ani_common_configuration.h"
#include "ani_common_want.h"
#include "ets_data_struct_converter.h"
#include "ets_envsetup.h"
#include "ets_runtime.h"
#include "ets_ui_extension.h"
#include "ets_ui_extension_context.h"
#include "ets_ui_extension_content_session.h"

namespace OHOS {
namespace AbilityRuntime {
void EtsUIExtensionFFI::PromiseCallback(ani_env* env, ani_object aniObj)
{
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env or null aniObj");
        return;
    }
    ani_long destroyCallbackPoint = 0;
    ani_status status = ANI_ERROR;
    if ((status = env->Object_GetFieldByName_Long(aniObj, "destroyCallbackPoint", &destroyCallbackPoint)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    auto *callbackInfo = reinterpret_cast<AppExecFwk::AbilityTransactionCallbackInfo<> *>(destroyCallbackPoint);
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null callbackInfo");
        return;
    }
    callbackInfo->Call();
    AppExecFwk::AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);

    if ((status = env->Object_SetFieldByName_Long(aniObj, "destroyCallbackPoint",
        static_cast<ani_long>(0))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status : %{public}d", status);
        return;
    }
}

EtsUIExtensionFFI::EtsUIExtensionFFI(AbilityHandle ability, RuntimeHandle runtime)
{
    ability_ = reinterpret_cast<AbilityRuntime::EtsUIExtension *>(ability);
    runtime_ = reinterpret_cast<AbilityRuntime::ETSRuntime *>(runtime);
}

EtsUIExtensionFFI::~EtsUIExtensionFFI()
{
}

bool EtsUIExtensionFFI::LoadModule(const std::string &moduleName, const std::string &modulePath,
    const std::string &hapPath, bool esmodule, bool useCommonChunk, const std::string &srcEntrance)
{
    if (runtime_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null runtime_");
        return false;
    }
    etsUIExtensionObj_ = runtime_->LoadModule(moduleName, modulePath, hapPath, esmodule, useCommonChunk, srcEntrance);
    if (etsUIExtensionObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "LoadModule failed");
        return false;
    }

    auto env = runtime_->GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return false;
    }
    std::array functions = {
        ani_native_function { "nativeOnDestroyCallback", ":V",
            reinterpret_cast<void*>(EtsUIExtensionFFI::PromiseCallback) },
    };
    ani_status status = ANI_ERROR;
    if ((status = env->Class_BindNativeMethods(etsUIExtensionObj_->aniCls, functions.data(),
        functions.size())) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
    }
    
    return true;
}

ani_object EtsUIExtensionFFI::CreateETSContext(ani_env* env,
    std::shared_ptr<UIExtensionContext> context, int32_t screenMode)
{
    ani_object obj = CreateEtsUIExtensionContext(env, context);
    return obj;
}

void EtsUIExtensionFFI::BindContext(const std::shared_ptr<OHOS::AAFwk::Want> &want,
    const std::shared_ptr<UIExtensionContext> &context)
{
    if (runtime_ == nullptr || want == nullptr ||
        context == nullptr || etsUIExtensionObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null runtime_ or want or context or etsUIExtensionObj_");
        return;
    }

    auto env = runtime_->GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }

    int32_t screenMode = want->GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::IDLE_SCREEN_MODE);
    ani_object contextObj = CreateETSContext(env, context, screenMode);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null contextObj");
        return;
    }

    ani_field contextField;
    auto status = env->Class_FindField(etsUIExtensionObj_->aniCls, "context", &contextField);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "status: %{public}d", status);
        return;
    }

    ani_ref contextRef = nullptr;
    if ((status = env->GlobalReference_Create(contextObj, &contextRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "status: %{public}d", status);
        return;
    }

    if ((status = env->Object_SetField_Ref(etsUIExtensionObj_->aniObj, contextField, contextRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "status: %{public}d", status);
    }
}

void EtsUIExtensionFFI::OnStart(const AAFwk::Want &want, const AAFwk::LaunchParam &launch)
{
    if (runtime_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null runtime_");
        return;
    }
    auto env = runtime_->GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto launchParam = launch;
    const char *signature =
        "L@ohos/app/ability/AbilityConstant/AbilityConstant/LaunchParam;:V";
    if (InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        launchParam.launchReason = AAFwk::LaunchReason::LAUNCHREASON_INSIGHT_INTENT;
    }

    ani_object launchParamObj = nullptr;
    if (!WrapLaunchParam(env, launch, launchParamObj)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "WrapLaunchParam failed");
        return;
    }
    CallObjectMethod(false, "onCreate", signature, launchParamObj);
}

void EtsUIExtensionFFI::OnStop()
{
    CallObjectMethod(false, "onDestroy", nullptr);
}

bool EtsUIExtensionFFI::CallOnDestroy(AppExecFwk::AbilityTransactionCallbackInfo<void> *callbackInfo,
    bool &isAsyncCallback)
{
    if (runtime_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null runtime_");
        return false;
    }

    ani_long destroyCallbackPoint = (ani_long)callbackInfo;
    ani_status status = ANI_ERROR;
    ani_field field = nullptr;
    auto env = runtime_->GetAniEnv();
    if (env == nullptr || etsUIExtensionObj_ == nullptr) {
        isAsyncCallback = false;
        OnStop();
        return false;
    }
    if ((status = env->Class_FindField(etsUIExtensionObj_->aniCls, "destroyCallbackPoint", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
    }
    if ((status = env->Object_SetField_Long(etsUIExtensionObj_->aniObj, field, destroyCallbackPoint)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
    }

    isAsyncCallback = CallObjectMethod(true, "callOnDestroy", ":Z");
    return true;
}

void EtsUIExtensionFFI::OnForeground()
{
    CallObjectMethod(false, "onForeground", nullptr);
}

void EtsUIExtensionFFI::OnBackground()
{
    CallObjectMethod(false, "onBackground", nullptr);
}

bool EtsUIExtensionFFI::OnWindowStageCreate(const AAFwk::Want &want,
    const std::shared_ptr<OHOS::AbilityRuntime::Context> &context,
    const sptr<OHOS::AAFwk::SessionInfo> &sessionInfo,
    const sptr<OHOS::Rosen::Window> &uiWindow)
{
    if (runtime_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null runtime_");
        return false;
    }
    auto env = runtime_->GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return false;
    }
    ani_ref wantObj = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null wantObj");
        return false;
    }
    ani_ref wantRef = nullptr;
    ani_status status = ANI_OK;
    if ((status = env->GlobalReference_Create(wantObj, &wantRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
    }
    std::weak_ptr<Context> wkctx = context;
    etsUiExtContentSession_ = std::make_shared<EtsUIExtensionContentSession>(sessionInfo, uiWindow,
        wkctx, abilityResultListeners_);
    ani_object sessonObj = EtsUIExtensionContentSession::CreateEtsUIExtensionContentSession(env,
        sessionInfo, uiWindow, context, abilityResultListeners_, etsUiExtContentSession_);
    if ((status = env->GlobalReference_Create(sessonObj, &contentSession_)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
    }
    int32_t screenMode = want.GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::IDLE_SCREEN_MODE);
    if (screenMode == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        ability_->SetScreenMode(AAFwk::EMBEDDED_FULL_SCREEN_MODE);
        auto jsAppWindowStage = std::make_unique<EtsEnv::ETSNativeReference>();
        if (jsAppWindowStage == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "JsAppWindowStage is nullptr");
            return false;
        }
    } else {
        CallObjectMethod(false, "onSessionCreate", nullptr, wantObj, sessonObj);
    }
    return true;
}

void EtsUIExtensionFFI::OnSessionDestroy()
{
    ani_object contenSessionObj = static_cast<ani_object>(contentSession_);
    if (contenSessionObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "contenSessionObj null ptr");
    } else {
        CallObjectMethod(false, "onSessionDestroy", nullptr, contenSessionObj);
    }
}

void EtsUIExtensionFFI::OnAbilityResult(int requestCode, int resultCode, const OHOS::AAFwk::Want &resultData)
{
    if (abilityResultListeners_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "abilityResultListensers null");
        return;
    }
    abilityResultListeners_->OnAbilityResult(requestCode, resultCode, resultData);
}

void EtsUIExtensionFFI::RemoveListener(const uint64_t &componentId)
{
    if (abilityResultListeners_) {
        abilityResultListeners_->RemoveListener(componentId);
    }
}

bool EtsUIExtensionFFI::CallObjectMethod(bool withResult, const char *name, const char *signature, ...)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    if (runtime_ == nullptr || etsUIExtensionObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null runtime_ or etsUIExtensionObj_");
        return false;
    }

    auto env = runtime_->GetAniEnv();
    ani_status status = ANI_OK;
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(etsUIExtensionObj_->aniCls, name, signature, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return false;
    }
    env->ResetError();
    if (withResult) {
        ani_boolean res = ANI_FALSE;
        va_list args;
        va_start(args, signature);
        if ((status = env->Object_CallMethod_Boolean(etsUIExtensionObj_->aniObj, method, &res, args)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
            runtime_->HandleUncaughtError();
        }
        va_end(args);
        return res;
    }
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(etsUIExtensionObj_->aniObj, method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        runtime_->HandleUncaughtError();
        return false;
    }
    va_end(args);
    return false;
}

FFIHandle EtsUIExtensionFFI::FFICreate(AbilityHandle ability, RuntimeHandle runtime)
{
    return new (std::nothrow) EtsUIExtensionFFI(ability, runtime);
}

void EtsUIExtensionFFI::FFIDelete(FFIHandle ffi)
{
    if (ffi != nullptr) {
        delete reinterpret_cast<EtsUIExtensionFFI *>(ffi);
    }
}

bool EtsUIExtensionFFI::FFILoadModule(FFIHandle ffi, const std::string &moduleName,
    const std::string &modulePath, const std::string &hapPath,
    bool esmodule, bool useCommonChunk, const std::string &srcEntrance)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null ffi");
        return false;
    }
    EtsUIExtensionFFI *pFFI = reinterpret_cast<EtsUIExtensionFFI *>(ffi);
    return pFFI->LoadModule(moduleName, modulePath, hapPath, esmodule, useCommonChunk, srcEntrance);
}

void EtsUIExtensionFFI::FFIOnStart(FFIHandle ffi, const OHOS::AAFwk::Want &want,
    const OHOS::AAFwk::LaunchParam &launch)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null ffi");
        return;
    }
    EtsUIExtensionFFI *pFFI = reinterpret_cast<EtsUIExtensionFFI *>(ffi);
    return pFFI->OnStart(want, launch);
}

void EtsUIExtensionFFI::FFIOnStop(FFIHandle ffi)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null ffi");
        return;
    }
    EtsUIExtensionFFI *pFFI = reinterpret_cast<EtsUIExtensionFFI *>(ffi);
    return pFFI->OnStop();
}

bool EtsUIExtensionFFI::FFICallOnDestroy(FFIHandle ffi,
    AppExecFwk::AbilityTransactionCallbackInfo<void> *callbackInfo, bool &isAsyncCallback)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null ffi");
        return false;
    }
    EtsUIExtensionFFI *pFFI = reinterpret_cast<EtsUIExtensionFFI *>(ffi);
    return pFFI->CallOnDestroy(callbackInfo, isAsyncCallback);
}

void EtsUIExtensionFFI::FFIOnForeground(FFIHandle ffi)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null ffi");
        return;
    }
    EtsUIExtensionFFI *pFFI = reinterpret_cast<EtsUIExtensionFFI *>(ffi);
    return pFFI->OnForeground();
}

void EtsUIExtensionFFI::FFIOnBackground(FFIHandle ffi)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null ffi");
        return;
    }
    EtsUIExtensionFFI *pFFI = reinterpret_cast<EtsUIExtensionFFI *>(ffi);
    return pFFI->OnBackground();
}

bool EtsUIExtensionFFI::FFIOnWindowStageCreate(FFIHandle ffi, const AAFwk::Want &want,
    const std::shared_ptr<OHOS::AbilityRuntime::Context> &context,
    const OHOS::sptr<OHOS::AAFwk::SessionInfo> &sessionInfo,
    const OHOS::sptr<OHOS::Rosen::Window> &uiWindow)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null ffi");
        return false;
    }
    EtsUIExtensionFFI *pFFI = reinterpret_cast<EtsUIExtensionFFI *>(ffi);
    return pFFI->OnWindowStageCreate(want, context, sessionInfo, uiWindow);
}

void EtsUIExtensionFFI::FFIOnSessionDestroy(FFIHandle ffi)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null ffi");
        return;
    }
    EtsUIExtensionFFI *pFFI = reinterpret_cast<EtsUIExtensionFFI *>(ffi);
    return pFFI->OnSessionDestroy();
}

void EtsUIExtensionFFI::FFIBindContext(FFIHandle ffi, const std::shared_ptr<OHOS::AAFwk::Want> &want,
    const std::shared_ptr<UIExtensionContext> &context)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null ffi");
        return;
    }
    EtsUIExtensionFFI *pFFI = reinterpret_cast<EtsUIExtensionFFI *>(ffi);
    return pFFI->BindContext(want, context);
}

void EtsUIExtensionFFI::FFIOnAbilityResult(FFIHandle ffi, int requestCode,
    int resultCode, const OHOS::AAFwk::Want &resultData)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null ffi");
        return;
    }
    EtsUIExtensionFFI *pFFI = reinterpret_cast<EtsUIExtensionFFI *>(ffi);
    return pFFI->OnAbilityResult(requestCode, resultCode, resultData);
}

void EtsUIExtensionFFI::FFIRemoveListener(FFIHandle ffi, const uint64_t &componentId)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null ffi");
        return;
    }
    EtsUIExtensionFFI *pFFI = reinterpret_cast<EtsUIExtensionFFI *>(ffi);
    return pFFI->RemoveListener(componentId);
}

ETSUIExtensionFuncs *EtsUIExtensionFFI::RegisterFuncs()
{
    static ETSUIExtensionFuncs funcs = {
        .CreateEtsFFI = EtsUIExtensionFFI::FFICreate,
        .DeleteEtsFFI = EtsUIExtensionFFI::FFIDelete,
        .LoadModule = EtsUIExtensionFFI::FFILoadModule,
        .OnStart = EtsUIExtensionFFI::FFIOnStart,
        .OnStop = EtsUIExtensionFFI::FFIOnStop,
        .CallOnDestroy = EtsUIExtensionFFI::FFICallOnDestroy,
        .OnForeground = EtsUIExtensionFFI::FFIOnForeground,
        .OnBackground = EtsUIExtensionFFI::FFIOnBackground,
        .OnWindowStageCreate = EtsUIExtensionFFI::FFIOnWindowStageCreate,
        .OnSessionDestroy = EtsUIExtensionFFI::FFIOnSessionDestroy,
        .BindContext = EtsUIExtensionFFI::FFIBindContext,
        .OnAbilityResult = EtsUIExtensionFFI::FFIOnAbilityResult,
        .RemoveListener = EtsUIExtensionFFI::FFIRemoveListener,
    };

    return &funcs;
}
} // namespace AbilityRuntime
} // namespace OHOS

ETS_EXPORT extern "C" ETSUIExtensionFuncs *OHOS_ETS_UIExtension_RegisterFuncs()
{
    return OHOS::AbilityRuntime::EtsUIExtensionFFI::RegisterFuncs();
}