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

#include "ets_ability_ffi.h"

#include <cstddef>
#include <cstdint>
#include <memory>

#include "ani_common_want.h"
#include "app_recovery.h"
#include "connection_manager.h"
#include "ets_ability_context.h"
#include "ets_data_struct_converter.h"
#include "ets_runtime.h"
#include "ets_envsetup.h"
#include "ets_ui_ability.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "insight_intent_execute_param.h"
#include "string_wrapper.h"
#ifdef SUPPORT_SCREEN
#include "ani_window_stage.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *UI_ABILITY_CLASS_NAME = "L@ohos/app/ability/UIAbility/UIAbility;";

void OnDestroyPromiseCallback(ani_env *env, ani_object aniObj)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "OnDestroyPromiseCallback called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    ani_long destroyCallbackPtr = 0;
    ani_status status = ANI_ERROR;
    if ((status = env->Object_GetFieldByName_Long(aniObj, "destroyCallbackPoint", &destroyCallbackPtr)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Object_GetFieldByName_Long status: %{public}d", status);
        return;
    }
    if (destroyCallbackPtr == 0) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null destroyCallbackPtr");
        return;
    }
    auto *callbackInfo = reinterpret_cast<OHOS::AppExecFwk::AbilityTransactionCallbackInfo<> *>(destroyCallbackPtr);
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null callbackInfo");
        return;
    }
    callbackInfo->Call();
    AppExecFwk::AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);
}
} // namespace

std::once_flag EtsAbilityFFI::singletonFlag_;

EtsAbilityFFI::EtsAbilityFFI(AbilityHandle ability, RuntimeHandle runtime)
{
    ability_ = reinterpret_cast<AbilityRuntime::EtsUIAbility *>(ability);
    runtime_ = reinterpret_cast<AbilityRuntime::ETSRuntime *>(runtime);
}

EtsAbilityFFI::~EtsAbilityFFI()
{
    if (runtime_ == nullptr) {
        return;
    }
    auto env = runtime_->GetAniEnv();
    if (env == nullptr) {
        return;
    }

    if (shellContextRef_ && shellContextRef_->aniRef) {
        env->GlobalReference_Delete(shellContextRef_->aniRef);
    }
    if (etsWindowStageObj_ && etsWindowStageObj_->aniRef) {
        env->GlobalReference_Delete(etsWindowStageObj_->aniRef);
    }
}

bool EtsAbilityFFI::LoadModule(const std::string &moduleName, const std::string &modulePath, const std::string &hapPath,
    bool esmodule, bool useCommonChunk, const std::string &srcEntrance)
{
    if (runtime_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null runtime_");
        return false;
    }
    etsAbilityObj_ = runtime_->LoadModule(moduleName, modulePath, hapPath, esmodule, useCommonChunk, srcEntrance);
    if (etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "LoadModule failed");
        return false;
    }
    return true;
}

bool EtsAbilityFFI::BindNativeMethods()
{
    if (runtime_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null runtime_");
        return false;
    }
    auto env = runtime_->GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return false;
    }
    ani_class cls = nullptr;
    ani_status status = env->FindClass(UI_ABILITY_CLASS_NAME, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "FindClass failed status: %{public}d", status);
        return false;
    }
    std::call_once(singletonFlag_, [&status, env, cls]() {
        std::array functions = {
            ani_native_function { "nativeOnDestroyCallback", ":V", reinterpret_cast<void *>(OnDestroyPromiseCallback) },
        };
        status = env->Class_BindNativeMethods(cls, functions.data(), functions.size());
    });
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Class_BindNativeMethods failed status: %{public}d", status);
        return false;
    }
    return true;
}

bool EtsAbilityFFI::CreateEtsContext(int32_t screenMode,
    const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContex)
{
    if (runtime_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null runtime_");
        return false;
    }
    auto env = runtime_->GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return false;
    }
    if (screenMode == AAFwk::IDLE_SCREEN_MODE) {
        ani_object contextObj = CreateEtsAbilityContext(env, abilityContex, application);
        if (contextObj == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null contextObj");
            return false;
        }
        ani_ref contextGlobalRef = nullptr;
        env->GlobalReference_Create(contextObj, &contextGlobalRef);
        ani_status status = env->Object_SetFieldByName_Ref(etsAbilityObj_->aniObj, "context", contextGlobalRef);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::UIABILITY, "Object_SetFieldByName_Ref status: %{public}d", status);
            return false;
        }
        shellContextRef_ = std::make_shared<EtsEnv::ETSNativeReference>();
        shellContextRef_->aniObj = contextObj;
        shellContextRef_->aniRef = contextGlobalRef;
    }
    // to be done: CreateAniEmbeddableUIAbilityContext
    return true;
}

void EtsAbilityFFI::OnStart(const AAFwk::Want &want, const AAFwk::LaunchParam &launch)
{
    if (runtime_ == nullptr || etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null runtime_ or etsAbilityObj_");
        return;
    }
    auto env = runtime_->GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    ani_object wantObj = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantObj == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null wantObj");
        return;
    }
    ani_status status = ANI_ERROR;
    if ((status = env->Object_SetFieldByName_Ref(etsAbilityObj_->aniObj, "launchWant", wantObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "launchWant Object_SetFieldByName_Ref status: %{public}d", status);
        return;
    }
    if ((status = env->Object_SetFieldByName_Ref(etsAbilityObj_->aniObj, "lastRequestWant", wantObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "lastRequestWant Object_SetFieldByName_Ref status: %{public}d", status);
        return;
    }
    auto launchParam = launch;
    if (AppExecFwk::InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        launchParam.launchReason = AAFwk::LaunchReason::LAUNCHREASON_INSIGHT_INTENT;
    }
    ani_object launchParamObj = nullptr;
    if (!AbilityRuntime::WrapLaunchParam(env, launchParam, launchParamObj)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "WrapLaunchParam failed");
        return;
    }
    CallObjectMethod(false, "onCreate", nullptr, wantObj, launchParamObj);
}

void EtsAbilityFFI::OnStop()
{
    CallObjectMethod(false, "onDestroy", nullptr);
}

bool EtsAbilityFFI::CallOnDestroy(AppExecFwk::AbilityTransactionCallbackInfo<void> *callbackInfo, bool &isAsyncCallback)
{
    if (runtime_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null runtime_");
        return false;
    }
    auto env = runtime_->GetAniEnv();
    if (env == nullptr || etsAbilityObj_ == nullptr) {
        isAsyncCallback = false;
        OnStop();
        return false;
    }

    ani_long destroyCallbackPoint = reinterpret_cast<ani_long>(callbackInfo);
    ani_status status =
        env->Object_SetFieldByName_Long(etsAbilityObj_->aniObj, "destroyCallbackPoint", destroyCallbackPoint);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Object_SetFieldByName_Long status: %{public}d", status);
        return false;
    }
    isAsyncCallback = CallObjectMethod(true, "callOnDestroy", ":Z");
    return true;
}

void EtsAbilityFFI::OnWindowStageCreate()
{
    if (runtime_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null runtime_");
        return;
    }
    auto etsAppWindowStage = CreateAppWindowStage();
    if (etsAppWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null etsAppWindowStage");
        return;
    }
    UpdateEtsWindowStage(reinterpret_cast<ani_ref>(etsAppWindowStage));
    auto env = runtime_->GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    etsWindowStageObj_ = std::make_shared<EtsEnv::ETSNativeReference>();
    etsWindowStageObj_->aniObj = etsAppWindowStage;
    ani_ref entryObjectRef = nullptr;
    env->GlobalReference_Create(etsAppWindowStage, &entryObjectRef);
    etsWindowStageObj_->aniRef = entryObjectRef;
    CallObjectMethod(false, "onWindowStageCreate", nullptr, etsAppWindowStage);
}

void EtsAbilityFFI::OnWindowStageDestroy()
{
    UpdateEtsWindowStage(nullptr);
    CallObjectMethod(false, "onWindowStageDestroy", nullptr);
}

void EtsAbilityFFI::OnForeground(const AAFwk::Want &want)
{
    if (runtime_ == nullptr || etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null runtime_ or etsAbilityObj_");
        return;
    }
    auto env = runtime_->GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null wantObj");
        return;
    }
    if ((status = env->Object_SetFieldByName_Ref(etsAbilityObj_->aniObj, "lastRequestWant", wantRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "lastRequestWant Object_SetFieldByName_Ref status: %{public}d", status);
        return;
    }
    CallObjectMethod(false, "onForeground", nullptr, wantRef);
}

void EtsAbilityFFI::OnBackground()
{
    CallObjectMethod(false, "onBackground", nullptr);
}

ani_object EtsAbilityFFI::CreateAppWindowStage()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (runtime_ == nullptr || ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null runtime_ or ability_");
        return nullptr;
    }
    auto env = runtime_->GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return nullptr;
    }
    auto scene = ability_->GetScene();
    if (scene == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "scene not found");
        return nullptr;
    }
    ani_object etsWindowStage = CreateAniWindowStage(env, scene);
    if (etsWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null etsWindowStage");
        return nullptr;
    }
    return etsWindowStage;
}

void EtsAbilityFFI::UpdateEtsWindowStage(ani_ref windowStage)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "UpdateEtsWindowStage called");
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null shellContextRef_");
        return;
    }
    if (runtime_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null runtime_");
        return;
    }
    auto env = runtime_->GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    ani_status status = ANI_ERROR;
    if (windowStage == nullptr) {
        ani_ref undefinedRef = nullptr;
        env->GetUndefined(&undefinedRef);
        if ((status = env->Object_SetFieldByName_Ref(shellContextRef_->aniObj, "windowStage", undefinedRef)) !=
            ANI_OK) {
            TAG_LOGE(AAFwkTag::UIABILITY, "Object_SetFieldByName_Ref status: %{public}d", status);
            return;
        }
        return;
    }
    if ((status = env->Object_SetFieldByName_Ref(shellContextRef_->aniObj, "windowStage", windowStage)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Object_SetFieldByName_Ref status: %{public}d", status);
        return;
    }
}

void EtsAbilityFFI::OnNewWant(const AAFwk::Want &want, const AAFwk::LaunchParam &launch)
{
    if (runtime_ == nullptr || etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null runtime_ or etsAbilityObj_");
        return;
    }
    auto env = runtime_->GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    if (etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null etsAbilityObj_");
        return;
    }
    ani_object wantObj = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantObj == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null wantObj");
        return;
    }
    ani_status status = env->Object_SetFieldByName_Ref(etsAbilityObj_->aniObj, "lastRequestWant", wantObj);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "lastRequestWant Object_SetFieldByName_Ref status: %{public}d", status);
        return;
    }
    auto launchParam = launch;
    if (AppExecFwk::InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        launchParam.launchReason = AAFwk::LaunchReason::LAUNCHREASON_INSIGHT_INTENT;
    }
    ani_object launchParamObj = nullptr;
    if (!AbilityRuntime::WrapLaunchParam(env, launchParam, launchParamObj)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "WrapLaunchParam failed");
        return;
    }
    std::string methodName = "OnNewWant";
    CallObjectMethod(false, "onNewWant", nullptr, wantObj, launchParamObj);
}

std::weak_ptr<OHOS::EtsEnv::ETSNativeReference> EtsAbilityFFI::GetAbilityObject()
{
    return etsAbilityObj_;
}

bool EtsAbilityFFI::CallObjectMethod(bool withResult, const char *name, const char *signature, ...)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    TAG_LOGD(AAFwkTag::UIABILITY, "EtsUIAbility call ets, name: %{public}s", name);
    if (runtime_ == nullptr || etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null runtime_ or etsAbilityObj_");
        return false;
    }
    auto env = runtime_->GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return false;
    }
    auto obj = etsAbilityObj_->aniObj;
    auto cls = etsAbilityObj_->aniCls;

    ani_method method = nullptr;
    ani_status status = env->Class_FindMethod(cls, name, signature, &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Class_FindMethod status: %{public}d", status);
        env->ResetError();
        return false;
    }
    if (withResult) {
        ani_boolean res = ANI_FALSE;
        va_list args;
        va_start(args, signature);
        if ((status = env->Object_CallMethod_Boolean_V(obj, method, &res, args)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UIABILITY, "Object_CallMethod_Boolean_V status: %{public}d", status);
            runtime_->HandleUncaughtError();
            return false;
        }
        va_end(args);
        return res;
    }
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(obj, method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Object_CallMethod_Void_V status: %{public}d", status);
        runtime_->HandleUncaughtError();
        return false;
    }
    va_end(args);
    TAG_LOGD(AAFwkTag::UIABILITY, "CallObjectMethod end, name: %{public}s", name);
    return false;
}

FFIHandle EtsAbilityFFI::FFICreate(AbilityHandle ability, RuntimeHandle runtime)
{
    return new (std::nothrow) EtsAbilityFFI(ability, runtime);
}

void EtsAbilityFFI::FFIDelete(FFIHandle ffi)
{
    if (ffi != nullptr) {
        delete reinterpret_cast<EtsAbilityFFI *>(ffi);
    }
}

bool EtsAbilityFFI::FFILoadModule(FFIHandle ffi, const std::string &moduleName, const std::string &modulePath,
    const std::string &hapPath, bool esmodule, bool useCommonChunk, const std::string &srcEntrance)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ffi");
        return false;
    }
    EtsAbilityFFI *pFFI = reinterpret_cast<EtsAbilityFFI *>(ffi);
    return pFFI->LoadModule(moduleName, modulePath, hapPath, esmodule, useCommonChunk, srcEntrance);
}

bool EtsAbilityFFI::FFIBindNativeMethods(FFIHandle ffi)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ffi");
        return false;
    }
    EtsAbilityFFI *pFFI = reinterpret_cast<EtsAbilityFFI *>(ffi);
    return pFFI->BindNativeMethods();
}

bool EtsAbilityFFI::FFICreateEtsContext(FFIHandle ffi, int32_t screenMode,
    const std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<OHOS::AbilityRuntime::AbilityContext> &abilityContext)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ffi");
        return false;
    }
    EtsAbilityFFI *pFFI = reinterpret_cast<EtsAbilityFFI *>(ffi);
    return pFFI->CreateEtsContext(screenMode, application, abilityContext);
}

void EtsAbilityFFI::FFIOnStart(FFIHandle ffi, const OHOS::AAFwk::Want &want, const OHOS::AAFwk::LaunchParam &launch)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ffi");
        return;
    }
    EtsAbilityFFI *pFFI = reinterpret_cast<EtsAbilityFFI *>(ffi);
    pFFI->OnStart(want, launch);
}

void EtsAbilityFFI::FFIOnStop(FFIHandle ffi)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ffi");
        return;
    }
    EtsAbilityFFI *pFFI = reinterpret_cast<EtsAbilityFFI *>(ffi);
    pFFI->OnStop();
}

bool EtsAbilityFFI::FFICallOnDestroy(FFIHandle ffi,
    OHOS::AppExecFwk::AbilityTransactionCallbackInfo<void> *callbackInfo, bool &isAsyncCallback)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ffi");
        return false;
    }
    EtsAbilityFFI *pFFI = reinterpret_cast<EtsAbilityFFI *>(ffi);
    return pFFI->CallOnDestroy(callbackInfo, isAsyncCallback);
}

void EtsAbilityFFI::FFIOnWindowStageCreate(FFIHandle ffi)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ffi");
        return;
    }
    EtsAbilityFFI *pFFI = reinterpret_cast<EtsAbilityFFI *>(ffi);
    pFFI->OnWindowStageCreate();
}

void EtsAbilityFFI::FFIOnWindowStageDestroy(FFIHandle ffi)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ffi");
        return;
    }
    EtsAbilityFFI *pFFI = reinterpret_cast<EtsAbilityFFI *>(ffi);
    pFFI->OnWindowStageDestroy();
}

void EtsAbilityFFI::FFIOnForeground(FFIHandle ffi, const OHOS::AAFwk::Want &want)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ffi");
        return;
    }
    EtsAbilityFFI *pFFI = reinterpret_cast<EtsAbilityFFI *>(ffi);
    pFFI->OnForeground(want);
}

void EtsAbilityFFI::FFIOnBackground(FFIHandle ffi)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ffi");
        return;
    }
    EtsAbilityFFI *pFFI = reinterpret_cast<EtsAbilityFFI *>(ffi);
    pFFI->OnBackground();
}

void EtsAbilityFFI::FFIOnNewWant(FFIHandle ffi, const OHOS::AAFwk::Want &want, const OHOS::AAFwk::LaunchParam &launch)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ffi");
        return;
    }
    EtsAbilityFFI *pFFI = reinterpret_cast<EtsAbilityFFI *>(ffi);
    pFFI->OnNewWant(want, launch);
}

std::weak_ptr<OHOS::EtsEnv::ETSNativeReference> EtsAbilityFFI::FFIGetAbilityObject(FFIHandle ffi)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ffi");
        return {};
    }
    EtsAbilityFFI *pFFI = reinterpret_cast<EtsAbilityFFI *>(ffi);
    return pFFI->GetAbilityObject();
}

ETSAbilityFuncs *EtsAbilityFFI::RegisterFuncs()
{
    static ETSAbilityFuncs funcs = {
        .CreateEtsFFI = EtsAbilityFFI::FFICreate,
        .DeleteEtsFFI = EtsAbilityFFI::FFIDelete,
        .LoadModule = EtsAbilityFFI::FFILoadModule,
        .BindNativeMethods = EtsAbilityFFI::FFIBindNativeMethods,
        .CreateEtsContext = EtsAbilityFFI::FFICreateEtsContext,
        .OnStart = EtsAbilityFFI::FFIOnStart,
        .OnStop = EtsAbilityFFI::FFIOnStop,
        .CallOnDestroy = EtsAbilityFFI::FFICallOnDestroy,
        .OnWindowStageCreate = EtsAbilityFFI::FFIOnWindowStageCreate,
        .OnWindowStageDestroy = EtsAbilityFFI::FFIOnWindowStageDestroy,
        .OnForeground = EtsAbilityFFI::FFIOnForeground,
        .OnBackground = EtsAbilityFFI::FFIOnBackground,
        .OnNewWant = EtsAbilityFFI::FFIOnNewWant,
        .GetAbilityObject = EtsAbilityFFI::FFIGetAbilityObject,
    };

    return &funcs;
}
} // namespace AbilityRuntime
} // namespace OHOS

ETS_EXPORT extern "C" ETSAbilityFuncs *OHOS_ETS_Ability_RegisterFuncs()
{
    return OHOS::AbilityRuntime::EtsAbilityFFI::RegisterFuncs();
}