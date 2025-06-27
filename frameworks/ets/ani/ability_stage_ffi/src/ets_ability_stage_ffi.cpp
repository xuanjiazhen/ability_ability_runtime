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

#include "ets_ability_stage_ffi.h"

#include "ani_common_configuration.h"
#include "context.h"
#include "ets_ability_stage_context.h"
#include "ets_envsetup.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AbilityRuntime {

EtsAbilityStageFFI::EtsAbilityStageFFI(StageHandle stage, RuntimeHandle runtime)
{
    stage_ = reinterpret_cast<AbilityRuntime::EtsAbilityStage *>(stage);
    runtime_ = reinterpret_cast<AbilityRuntime::ETSRuntime *>(runtime);
}

bool EtsAbilityStageFFI::LoadModule(const std::string &moduleName, const std::string &modulePath,
    const std::string &hapPath, bool esmodule, bool useCommonChunk, const std::string &srcEntrance)
{
    if (runtime_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null runtime_");
        return false;
    }
    etsAbilityStageObj_ = runtime_->LoadModule(moduleName, modulePath, hapPath, esmodule, useCommonChunk, srcEntrance);
    if (etsAbilityStageObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "LoadModule failed");
        return false;
    }
    return true;
}

void EtsAbilityStageFFI::OnCreate()
{
    CallObjectMethod(false, "onCreate", ":V");
}

void EtsAbilityStageFFI::OnDestroy()
{
    CallObjectMethod(false, "onDestroy", ":V");
}

void EtsAbilityStageFFI::OnConfigurationUpdated(const std::shared_ptr<OHOS::AppExecFwk::Configuration> &fullConfig,
    const OHOS::AppExecFwk::Configuration &configuration)
{
    if (runtime_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null runtime_");
        return;
    }

    auto env = runtime_->GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
 
    ETSAbilityStageContext::ConfigurationUpdated(env, fullConfig);
    ani_object configObj = OHOS::AppExecFwk::WrapConfiguration(env, configuration);
    if (configObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null configObj");
        return;
    }
    CallObjectMethod(false, "onConfigurationUpdate", "L@ohos/app/ability/Configuration/Configuration;:V", configObj);
}

bool EtsAbilityStageFFI::CallObjectMethod(bool withResult, const char *name, const char *signature, ...) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    TAG_LOGD(AAFwkTag::APPKIT, "CallObjectMethod: name:%{public}s", name);
    if (runtime_ == nullptr || etsAbilityStageObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null runtime_ or etsAbilityStageObj_");
        return false;
    }
    auto env = runtime_->GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return false;
    }
    ani_status status = ANI_OK;
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(etsAbilityStageObj_->aniCls, name, signature, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        return false;
    }
    env->ResetError();
    if (withResult) {
        ani_boolean res = ANI_FALSE;
        va_list args;
        va_start(args, signature);
        if ((status = env->Object_CallMethod_Boolean(etsAbilityStageObj_->aniObj, method, &res, args)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
            runtime_->HandleUncaughtError();
        }
        va_end(args);
        return res;
    }
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(etsAbilityStageObj_->aniObj, method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        runtime_->HandleUncaughtError();
        return false;
    }
    va_end(args);
    return false;
}

void EtsAbilityStageFFI::SetEtsAbilityStage(const std::shared_ptr<OHOS::AbilityRuntime::Context> &context)
{
    if (context == nullptr || etsAbilityStageObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context or etsAbilityStageObj_");
        return;
    }
    if (runtime_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null runtime_");
        return;
    }
    auto env = runtime_->GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    ani_object stageCtxObj = ETSAbilityStageContext::CreateEtsAbilityStageContext(env, context);
    if (stageCtxObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "CreateEtsAbilityStageContext failed");
        return;
    }

    ani_status status = ANI_OK;
    ani_field contextField;
    status = env->Class_FindField(etsAbilityStageObj_->aniCls, "context", &contextField);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Class_GetField context failed");
        return;
    }
    ani_ref stageCtxObjRef = nullptr;
    if (env->GlobalReference_Create(stageCtxObj, &stageCtxObjRef) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "GlobalReference_Create stageCtxObj failed");
        return;
    }
    if (env->Object_SetField_Ref(etsAbilityStageObj_->aniObj, contextField, stageCtxObjRef) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_SetField_Ref stageCtxObj failed");
    }
}

FFIHandle EtsAbilityStageFFI::FFICreate(StageHandle stage, RuntimeHandle runtime)
{
    return new (std::nothrow) EtsAbilityStageFFI(stage, runtime);
}

void EtsAbilityStageFFI::FFIDelete(FFIHandle ffi)
{
    if (ffi != nullptr) {
        delete reinterpret_cast<EtsAbilityStageFFI *>(ffi);
    }
}

bool EtsAbilityStageFFI::FFILoadModule(FFIHandle ffi, const std::string &moduleName,
    const std::string &modulePath, const std::string &hapPath,
    bool esmodule, bool useCommonChunk, const std::string &srcEntrance)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ffi");
        return false;
    }
    EtsAbilityStageFFI *pFFI = reinterpret_cast<EtsAbilityStageFFI *>(ffi);
    return pFFI->LoadModule(moduleName, modulePath, hapPath, esmodule, useCommonChunk, srcEntrance);
}

void EtsAbilityStageFFI::FFIOnCreate(FFIHandle ffi)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ffi");
        return;
    }
    EtsAbilityStageFFI *pFFI = reinterpret_cast<EtsAbilityStageFFI *>(ffi);
    return pFFI->OnCreate();
}

void EtsAbilityStageFFI::FFIOnDestroy(FFIHandle ffi)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ffi");
        return;
    }
    EtsAbilityStageFFI *pFFI = reinterpret_cast<EtsAbilityStageFFI *>(ffi);
    return pFFI->OnDestroy();
}

void EtsAbilityStageFFI::FFIOnConfigurationUpdated(FFIHandle ffi,
    const std::shared_ptr<OHOS::AppExecFwk::Configuration> &fullConfig,
    const OHOS::AppExecFwk::Configuration &configuration)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ffi");
        return;
    }
    EtsAbilityStageFFI *pFFI = reinterpret_cast<EtsAbilityStageFFI *>(ffi);
    return pFFI->OnConfigurationUpdated(fullConfig, configuration);
}

void EtsAbilityStageFFI::FFISetEtsAbilityStage(FFIHandle ffi,
    const std::shared_ptr<OHOS::AbilityRuntime::Context> &context)
{
    if (ffi == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ffi");
        return;
    }
    EtsAbilityStageFFI *pFFI = reinterpret_cast<EtsAbilityStageFFI *>(ffi);
    return pFFI->SetEtsAbilityStage(context);
}

ETSAbilityStageFuncs *EtsAbilityStageFFI::RegisterFuncs()
{
    static ETSAbilityStageFuncs funcs = {
        .CreateEtsFFI = EtsAbilityStageFFI::FFICreate,
        .DeleteEtsFFI = EtsAbilityStageFFI::FFIDelete,
        .LoadModule = EtsAbilityStageFFI::FFILoadModule,
        .OnCreate = EtsAbilityStageFFI::FFIOnCreate,
        .OnDestroy = EtsAbilityStageFFI::FFIOnDestroy,
        .OnConfigurationUpdated = EtsAbilityStageFFI::FFIOnConfigurationUpdated,
        .SetEtsAbilityStage = EtsAbilityStageFFI::FFISetEtsAbilityStage,
    };

    return &funcs;
}
} // namespace AbilityRuntime
} // namespace OHOS

ETS_EXPORT extern "C" ETSAbilityStageFuncs *OHOS_ETS_Ability_Stage_RegisterFuncs()
{
    return OHOS::AbilityRuntime::EtsAbilityStageFFI::RegisterFuncs();
}