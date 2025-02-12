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

#include "sts_ui_ability.h"

#include <cstdlib>
#include <regex>

#include "ability_business_error.h"
#include "ability_delegator_registry.h"
#include "ability_manager_client.h"
#include "ability_recovery.h"
#include "ability_start_setting.h"
#include "app_recovery.h"
#include "connection_manager.h"
#include "context/application_context.h"
#include "context/context.h"
#include "display_util.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "string_wrapper.h"
#include "system_ability_definition.h"
#include "time_util.h"

namespace OHOS {
namespace AbilityRuntime {
UIAbility *StsUIAbility::Create(const std::unique_ptr<Runtime> &runtime)
{
    return new (std::nothrow) StsUIAbility(static_cast<STSRuntime &>(*runtime));
}

StsUIAbility::StsUIAbility(STSRuntime &stsRuntime) : stsRuntime_(stsRuntime)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

StsUIAbility::~StsUIAbility()
{
    TAG_LOGI(AAFwkTag::UIABILITY, "called");
    //stsRuntime_.FreeNativeReference(std::move(stsAbilityObj_));
}

void StsUIAbility::Init(std::shared_ptr<AppExecFwk::AbilityLocalRecord> record,
    const std::shared_ptr<OHOSApplication> application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null localAbilityRecord StsUIAbility");
        return;
    }
    auto abilityInfo = record->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityInfo StsUIAbility");
        return;
    }
    UIAbility::Init(record, application, handler, token);

    std::string srcPath(abilityInfo->package);
    if (!abilityInfo->isModuleJson) {
        /* temporary compatibility api8 + config.json */
        srcPath.append("/assets/sts/");
        if (!abilityInfo->srcPath.empty()) {
            srcPath.append(abilityInfo->srcPath);
        }
        srcPath.append("/").append(abilityInfo->name).append(".abc");
    } else {
        if (abilityInfo->srcEntrance.empty()) {
            TAG_LOGE(AAFwkTag::UIABILITY, "empty srcEntrance");
            return;
        }
        srcPath.append("/");
        srcPath.append(abilityInfo->srcEntrance);
        srcPath.erase(srcPath.rfind("."));
        srcPath.append(".abc");
        TAG_LOGD(AAFwkTag::UIABILITY, "stsAbility srcPath: %{public}s", srcPath.c_str());
    }

    std::string moduleName(abilityInfo->moduleName);
    moduleName.append("::").append(abilityInfo->name);

    UpdateAbilityObj(abilityInfo, moduleName, srcPath);
}

void StsUIAbility::UpdateAbilityObj(std::shared_ptr<AbilityInfo> abilityInfo,
    const std::string &moduleName, const std::string &srcPath)
{
    std::string key = moduleName + "::" + srcPath;
    std::unique_ptr<NativeReference> moduleObj = nullptr;
    stsAbilityObj_ = stsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo->hapPath, abilityInfo->compileMode == AppExecFwk::CompileMode::ES_MODULE,
        false, abilityInfo->srcEntrance);
}

ani_object WrapWantParams(ani_env* env, ani_class cls, const AAFwk::WantParams &wantParams)
{
    ani_method method = nullptr;
    env->Class_GetMethod(cls, "<init>", "I:V", &method);
    ani_object object = nullptr;
    env->Object_New(cls, method, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null object");
        return nullptr;
    }
    //TODO
    

    return object;
}

ani_object WrapWant(ani_env* env, ani_class cls, const Want &want)
{
    TAG_LOGE(AAFwkTag::UIABILITY, "sjr WrapWant");
    ani_method method = nullptr;
    ani_status status = ANI_ERROR;
    TAG_LOGE(AAFwkTag::UIABILITY, "sjr WrapWant  1");
    status = env->Class_GetMethod(cls, "<ctor>", ":V", &method);
    TAG_LOGE(AAFwkTag::UIABILITY, "sjr WrapWant  2");
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "sjr status : %{public}d", status);
    }
    ani_object object = nullptr;
    status = env->Object_New(cls, method, &object);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "sjr status : %{public}d", status);
    }
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null object");
        return nullptr;
    }
    ani_field field = nullptr;
    auto elementName = want.GetElement();

    status = env->Class_GetField(cls, "deviceId", &field);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "sjr status : %{public}d", status);
    }
    status = env->Object_SetField_Char(object, field, reinterpret_cast<ani_char>(elementName.GetDeviceID().c_str()));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "sjr status : %{public}d", status);
    }

    field = nullptr;
    env->Class_GetField(cls, "bundleName", &field);
    env->Object_SetField_Char(object, field, reinterpret_cast<ani_char>(elementName.GetBundleName().c_str()));

    field = nullptr;
    env->Class_GetField(cls, "abilityName", &field);
    env->Object_SetField_Char(object, field, reinterpret_cast<ani_char>(elementName.GetAbilityName().c_str()));

    field = nullptr;
    env->Class_GetField(cls, "moduleName", &field);
    env->Object_SetField_Char(object, field, reinterpret_cast<ani_char>(elementName.GetModuleName().c_str()));

    field = nullptr;
    env->Class_GetField(cls, "uri", &field);
    env->Object_SetField_Char(object, field, reinterpret_cast<ani_char>(want.GetUriString().c_str()));

    field = nullptr;
    env->Class_GetField(cls, "type", &field);
    env->Object_SetField_Char(object, field, reinterpret_cast<ani_char>(want.GetType().c_str()));

    field = nullptr;
    status = env->Class_GetField(cls, "flags", &field);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "sjr status : %{public}d", status);
    }
    status = env->Object_SetField_Int(object, field, static_cast<ani_int>(want.GetFlags()));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "sjr status : %{public}d", status);
    }
    TAG_LOGE(AAFwkTag::UIABILITY, "sjr status : %{public}d", want.GetFlags());
    

    field = nullptr;
    env->Class_GetField(cls, "action", &field);
    env->Object_SetField_Char(object, field, reinterpret_cast<ani_char>(want.GetAction().c_str()));
    
    // TODO
    // ani_object parameters = WrapWantParams(env, cls, want.GetParams());
    // field = nullptr;
    // env->Class_GetField(cls, "parameters", &field);
    // env->Object_SetField_Ref(object, field, reinterpret_cast<ani_ref>(&parameters));
    // object.fds = nullptr;
    // object.entities = nullptr;

    return object;
}

ani_object WrapLaunchParam(ani_env *env, ani_class cls, const AAFwk::LaunchParam &launchParam)
{
    ani_method method = nullptr;
    env->Class_GetMethod(cls, "<init>", "I:V", &method);
    ani_object object = nullptr;
    env->Object_New(cls, method, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null object");
        return nullptr;
    }
    ani_field field = nullptr;
    env->Class_GetField(cls, "launchReason", &field);
    env->Object_SetField_Char(object, field, reinterpret_cast<ani_char>(std::string("launchReason").c_str()));
    // TODO
    // object.launchReason = launchParam.launchReason;
    // object.launchReasonMessage = launchParam.launchReasonMessage;
    // object.lastExitReason = launchParam.lastExitReason;
    // object.lastExitMessage = launchParam.lastExitMessage;
    return object;
}

void StsUIAbility::OnStart(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    TAG_LOGE(AAFwkTag::UIABILITY, "[sjr] OnStart");

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnStart(want, sessionInfo);

    if (!stsAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "not found Ability.sts");
        return;
    }
    ani_method method = nullptr;
    auto env = stsRuntime_.GetAniEnv();
    if (!env) {
        TAG_LOGE(AAFwkTag::UIABILITY, "env not found Ability.sts");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_class wantCls = nullptr;
    status = env->FindClass("LWant;", &wantCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "sjr status : %{public}d", status);
    }
    if (wantCls == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null wantCls");
        return;
    }

    ani_object wantObj = WrapWant(env, wantCls, want);
    if (wantObj == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null wantObj");
        return;
    }
    ani_ref wantRef = reinterpret_cast<ani_ref>(&wantObj);

    ani_field field = nullptr;
    status = env->Class_GetField(stsAbilityObj_->aniCls, "launchWant", &field);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "sjr status : %{public}d", status);
    }
    status = env->Object_SetField_Ref(stsAbilityObj_->aniObj, field, wantRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "sjr status : %{public}d", status);
    }
    field = nullptr;
    status = env->Class_GetField(stsAbilityObj_->aniCls, "lastRequestWant", &field);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "sjr status : %{public}d", status);
    }
    status = env->Object_SetField_Ref(stsAbilityObj_->aniObj, field, wantRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "sjr status : %{public}d", status);
    }
    // env->Class_GetMethod(stsAbilityObj_->aniCls, "onCreate", nullptr, &method);
    // auto launchParam = GetLaunchParam();
    // if (InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
    //     launchParam.launchReason = AAFwk::LaunchReason::LAUNCHREASON_INSIGHT_INTENT;
    // }

    // ani_class launchParamCls = nullptr;
    // env->FindClass("LLaunchParam;", &launchParamCls);
    // if (launchParamCls == nullptr) {
    //     TAG_LOGE(AAFwkTag::UIABILITY, "null launchParamCls");
    //     return;
    // }

    // ani_object launchParamObj = WrapLaunchParam(env, launchParamCls, launchParam);
    // ani_ref launchParamRef = reinterpret_cast<ani_ref>(&launchParamObj);


    // ani_value args[] = { { .r = wantRef }, { .r = launchParamRef } };
    ani_value args[] = { { .r = wantRef } };
    status = env->Class_GetMethod(stsAbilityObj_->aniCls, "onCreate", "L:V", &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "sjr status : %{public}d", status);
    }
    // status = env->Object_CallMethod_Void_A(stsAbilityObj_->aniObj, method, args);
    ani_int res = -1;
    status = env->Object_CallMethod_Int(stsAbilityObj_->aniObj, method, &res, args);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "sjr status : %{public}d", status);
    }
    TAG_LOGE(AAFwkTag::UIABILITY, "sjr res : %{public}d", res);


    // env->Class_GetMethod(stsAbilityObj_->aniCls, "onCreate", ":V", &method);
    // if (status != ANI_OK) {
    //     TAG_LOGE(AAFwkTag::UIABILITY, "sjr status : %{public}d", status);
    // }
    // if (method == nullptr){
    //     TAG_LOGE(AAFwkTag::UIABILITY, "sjr method null");
    // }
    // status = env->Object_CallMethod_Void(stsAbilityObj_->aniObj, method);
    // if (status != ANI_OK) {
    //     TAG_LOGE(AAFwkTag::UIABILITY, "sjr status : %{public}d", status);
    // }


    // method = nullptr;
    // env->Class_GetMethod(stsAbilityObj_->aniCls, "onCreate", ":I", &method);
    // if (method == nullptr){
    //     TAG_LOGE(AAFwkTag::UIABILITY, "sjr method null");
    // }
    // status = env->Object_CallMethod_Int(stsAbilityObj_->aniObj, method, &res);
    // if (status != ANI_OK) {
    //     TAG_LOGE(AAFwkTag::UIABILITY, "sjr status not ANI_OK");
    // } 
    // TAG_LOGE(AAFwkTag::UIABILITY, "sjr status : %{public}d", status);
    // TAG_LOGE(AAFwkTag::UIABILITY, "sjr res : %{public}d", res);

    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void StsUIAbility::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnStop();

    if (!stsAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "not found Ability.sts");
        return;
    }
    ani_method method;
    auto env = stsRuntime_.GetAniEnv();
    if (!env) {
        TAG_LOGE(AAFwkTag::UIABILITY, "env not found Ability.sts");
        return;
    }
    env->Class_GetMethod(stsAbilityObj_->aniCls, "onDestroy", nullptr, &method);
    env->Object_CallMethod_Void(stsAbilityObj_->aniObj, method);
}

void StsUIAbility::OnNewWant(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnNewWant(want);

    //TODO
#ifdef SUPPORT_SCREEN
    // if (scene_) {
    //     scene_->OnNewWant(want);
    // }
    // HandleCollaboration(want);
#endif

    if (!stsAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "not found Ability.sts");
        return;
    }
    ani_method method;
    auto env = stsRuntime_.GetAniEnv();
    if (!env) {
        TAG_LOGE(AAFwkTag::UIABILITY, "env not found Ability.sts");
        return;
    }
    env->Class_GetMethod(stsAbilityObj_->aniCls, "onNewWant", nullptr, &method);
    env->Object_CallMethod_Void(stsAbilityObj_->aniObj, method);
}
#ifdef SUPPORT_SCREEN
void StsUIAbility::OnSceneCreated()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnSceneCreated();

    if (!stsAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "not found Ability.sts");
        return;
    }
    ani_method method;
    auto env = stsRuntime_.GetAniEnv();
    if (!env) {
        TAG_LOGE(AAFwkTag::UIABILITY, "env not found Ability.sts");
        return;
    }
    env->Class_GetMethod(stsAbilityObj_->aniCls, "onWindowStageCreate", nullptr, &method);
    env->Object_CallMethod_Void(stsAbilityObj_->aniObj, method);
}

void StsUIAbility::onSceneDestroyed()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    UIAbility::onSceneDestroyed();

    if (!stsAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "not found Ability.sts");
        return;
    }
    ani_method method;
    auto env = stsRuntime_.GetAniEnv();
    if (!env) {
        TAG_LOGE(AAFwkTag::UIABILITY, "env not found Ability.sts");
        return;
    }
    env->Class_GetMethod(stsAbilityObj_->aniCls, "onWindowStageDestroy", nullptr, &method);
    env->Object_CallMethod_Void(stsAbilityObj_->aniObj, method);
}

void StsUIAbility::OnForeground(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnForeground(want);

    if (!stsAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "not found Ability.sts");
        return;
    }
    ani_method method;
    auto env = stsRuntime_.GetAniEnv();
    if (!env) {
        TAG_LOGE(AAFwkTag::UIABILITY, "env not found Ability.sts");
        return;
    }
    env->Class_GetMethod(stsAbilityObj_->aniCls, "onForeground", nullptr, &method);
    env->Object_CallMethod_Void(stsAbilityObj_->aniObj, method);
}

void StsUIAbility::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnBackground();

    if (!stsAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "not found Ability.sts");
        return;
    }
    ani_method method;
    auto env = stsRuntime_.GetAniEnv();
    if (!env) {
        TAG_LOGE(AAFwkTag::UIABILITY, "env not found Ability.sts");
        return;
    }
    env->Class_GetMethod(stsAbilityObj_->aniCls, "onBackground", nullptr, &method);
    env->Object_CallMethod_Void(stsAbilityObj_->aniObj, method);
}
#endif
} // namespace AbilityRuntime
} // namespace OHOS
