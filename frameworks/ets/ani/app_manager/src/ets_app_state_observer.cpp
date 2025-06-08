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

#include "ets_app_state_observer.h"

#include "ani_common_ability_state_data.h"
#include "ani_common_util.h"
#include "ani_common_want.h"
#include "hilog_tag_wrapper.h"
#include "sts_app_manager_utils.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr const char *SIGNATURE_APP_STATE_DATA = "Lapplication/AppStateData/AppStateData;:V";
constexpr const char *SIGNATURE_ABILITY_STATE_DATA = "Lapplication/AbilityStateData/AbilityStateData;:V";
constexpr const char *SIGNATURE_PROCESS_DATA = "Lapplication/ProcessData/ProcessData;:V";
constexpr const char *CLASS_NAME_APPLIACTION_STATE_OBSERVER =
    "Lapplication/ApplicationStateObserver/ApplicationStateObserverImpl;";

EtsAppStateObserver::EtsAppStateObserver(ani_vm *etsVm) : etsVm_(etsVm) {}

EtsAppStateObserver::~EtsAppStateObserver() = default;

void EtsAppStateObserver::OnForegroundApplicationChanged(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnForegroundApplicationChanged bundleName:%{public}s,uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid appmgr");
        return;
    }
    HandleOnForegroundApplicationChanged(appStateData);
}

void EtsAppStateObserver::HandleOnForegroundApplicationChanged(const AppStateData &appStateData)
{
    ani_env *env = nullptr;
    if (!AttachAniEnv(env)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to AttachAniEnv");
        DetachAniEnv();
        return;
    }
    auto tmpMap = stsObserverObjectMap_;
    for (auto &item : tmpMap) {
        auto appStateDataObj = AppManagerSts::WrapAppStateData(env, appStateData);
        if (appStateDataObj == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null appStateDataObj");
            DetachAniEnv();
            return;
        }
        CallStsFunction(env, item.second, "onForegroundApplicationChanged", SIGNATURE_APP_STATE_DATA, appStateDataObj);
    }
    DetachAniEnv();
}

void EtsAppStateObserver::OnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnAbilityStateChanged");
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr may has cancelled storage");
        return;
    }
    HandleOnAbilityStateChanged(abilityStateData);
}

void EtsAppStateObserver::HandleOnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    ani_env *env = nullptr;
    if (!AttachAniEnv(env)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to AttachAniEnv");
        DetachAniEnv();
        return;
    }
    auto tmpMap = stsObserverObjectMap_;
    for (auto &item : tmpMap) {
        auto abilityStateDataObj = OHOS::AppExecFwk::WrapAbilityStateData(env, abilityStateData);
        if (abilityStateDataObj == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null abilityStateDataObj");
            DetachAniEnv();
        }
        CallStsFunction(env, item.second, "onAbilityStateChanged", SIGNATURE_ABILITY_STATE_DATA, abilityStateDataObj);
    }
    DetachAniEnv();
}

void EtsAppStateObserver::OnExtensionStateChanged(const AbilityStateData &abilityStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr may destroyed");
        return;
    }
    HandleOnExtensionStateChanged(abilityStateData);
}

void EtsAppStateObserver::HandleOnExtensionStateChanged(const AbilityStateData &abilityStateData)
{
    ani_env *env = nullptr;
    if (!AttachAniEnv(env)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to AttachAniEnv");
        DetachAniEnv();
        return;
    }
    auto tmpMap = stsObserverObjectMap_;
    for (auto &item : tmpMap) {
        auto abilityStateDataObj = OHOS::AppExecFwk::WrapAbilityStateData(env, abilityStateData);
        if (abilityStateDataObj == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null abilityStateDataObj");
            DetachAniEnv();
        }
        CallStsFunction(env, item.second, "onAbilityStateChanged", SIGNATURE_ABILITY_STATE_DATA, abilityStateDataObj);
    }
    DetachAniEnv();
}

void EtsAppStateObserver::OnProcessCreated(const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnProcessCreated");
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appmgr may has cancelled storage");
        return;
    }
    HandleOnProcessCreated(processData);
}

void EtsAppStateObserver::HandleOnProcessCreated(const ProcessData &processData)
{
    ani_env *env = nullptr;
    if (!AttachAniEnv(env)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to AttachAniEnv");
        DetachAniEnv();
        return;
    }
    auto tmpMap = stsObserverObjectMap_;
    for (auto &item : tmpMap) {
        auto processDataObj = AppManagerSts::WrapProcessData(env, processData);
        if (processDataObj == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null processDataObj");
            DetachAniEnv();
            return;
        }
        CallStsFunction(env, item.second, "onProcessCreated", SIGNATURE_PROCESS_DATA, processDataObj);
    }
    DetachAniEnv();
}

void EtsAppStateObserver::OnProcessStateChanged(const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr may destroyed");
        return;
    }
    HandleOnProcessStateChanged(processData);
}

void EtsAppStateObserver::HandleOnProcessStateChanged(const ProcessData &processData)
{
    ani_env *env = nullptr;
    if (!AttachAniEnv(env)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to AttachAniEnv");
        DetachAniEnv();
        return;
    }
    auto tmpMap = stsObserverObjectMap_;
    for (auto &item : tmpMap) {
        auto processDataObj = AppManagerSts::WrapProcessData(env, processData);
        if (processDataObj == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null processDataObj");
            DetachAniEnv();
            return;
        }
        CallStsFunction(env, item.second, "onProcessStateChanged", SIGNATURE_PROCESS_DATA, processDataObj);
    }
    DetachAniEnv();
}

void EtsAppStateObserver::OnProcessDied(const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appmgr may destroyed");
        return;
    }
    HandleOnProcessDied(processData);
}

void EtsAppStateObserver::HandleOnProcessDied(const ProcessData &processData)
{
    ani_env *env = nullptr;
    if (!AttachAniEnv(env)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to AttachAniEnv");
        DetachAniEnv();
        return;
    }
    auto tmpMap = stsObserverObjectMap_;
    for (auto &item : tmpMap) {
        auto processDataObj = AppManagerSts::WrapProcessData(env, processData);
        if (processDataObj == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null processDataObj");
            DetachAniEnv();
            return;
        }
        CallStsFunction(env, item.second, "onProcessDied", SIGNATURE_PROCESS_DATA, processDataObj);
    }
    DetachAniEnv();
}

void EtsAppStateObserver::OnAppStarted(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr may destroyed");
        return;
    }
    HandleOnAppStarted(appStateData);
}

void EtsAppStateObserver::HandleOnAppStarted(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    ani_env *env = nullptr;
    if (!AttachAniEnv(env)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to AttachAniEnv");
        DetachAniEnv();
        return;
    }
    auto tmpMap = stsObserverObjectMap_;
    for (auto &item : tmpMap) {
        auto appStateDataObj = AppManagerSts::WrapAppStateData(env, appStateData);
        if (appStateDataObj == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null appStateDataObj");
            DetachAniEnv();
            return;
        }
        CallStsFunction(env, item.second, "onAppStarted", SIGNATURE_APP_STATE_DATA, appStateDataObj);
    }
    DetachAniEnv();
}

void EtsAppStateObserver::OnAppStopped(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr may destroyed");
        return;
    }
    HandleOnAppStopped(appStateData);
}

void EtsAppStateObserver::HandleOnAppStopped(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    ani_env *env = nullptr;
    if (!AttachAniEnv(env)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to AttachAniEnv");
        DetachAniEnv();
        return;
    }
    auto tmpMap = stsObserverObjectMap_;
    for (auto &item : tmpMap) {
        auto appStateDataObj = AppManagerSts::WrapAppStateData(env, appStateData);
        if (appStateDataObj == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null appStateDataObj");
            DetachAniEnv();
            return;
        }
        CallStsFunction(env, item.second, "onAppStopped", SIGNATURE_APP_STATE_DATA, appStateDataObj);
    }
    DetachAniEnv();
}

void EtsAppStateObserver::CallStsFunction(
    ani_env *env, ani_object StsObserverObject, const char *methodName, const char *signature, ...)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call method:%{public}s", methodName);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return;
    }
    ani_class cls = nullptr;
    ani_method method = nullptr;
    ani_status status = ANI_OK;
    status = env->FindClass(CLASS_NAME_APPLIACTION_STATE_OBSERVER, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to find ApplicationStateObserver, status: %{public}d", status);
        return;
    }
    if (cls == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null cls");
        return;
    }
    if ((status = env->Class_FindMethod(cls, methodName, signature, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to find method, status: %{public}d", status);
        return;
    }
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null method");
        return;
    }
    env->ResetError();
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(StsObserverObject, method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to call %{public}s method,status: %{public}d", methodName, status);
        return;
    }
    va_end(args);
    return;
}

void EtsAppStateObserver::AddStsObserverObject(ani_env *env, const int32_t observerId, ani_object StsObserverObject)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AddStsObserverObject");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return;
    }
    ani_ref global = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GlobalReference_Create(StsObserverObject, &global)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status: %{public}d", status);
        return;
    }
    if (global == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null StsObserverObject ref");
        return;
    }
    stsObserverObjectMap_.emplace(observerId, reinterpret_cast<ani_object>(global));
}

bool EtsAppStateObserver::RemoveStsObserverObject(const int32_t observerId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "RemoveStsObserverObject");
    return (stsObserverObjectMap_.erase(observerId) == 1);
}

bool EtsAppStateObserver::FindObserverByObserverId(const int32_t observerId)
{
    return stsObserverObjectMap_.find(observerId) != stsObserverObjectMap_.end();
}

size_t EtsAppStateObserver::GetStsObserverMapSize()
{
    return stsObserverObjectMap_.size();
}

void EtsAppStateObserver::SetValid(const bool valid)
{
    valid_ = valid;
}

bool EtsAppStateObserver::AttachAniEnv(ani_env *&env) {
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "etsVm_ nullptr");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_options aniArgs {0, nullptr};
    if ((status = (etsVm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status: %{public}d", status);
        return false;
    } 
    return env == nullptr;
}

void EtsAppStateObserver::DetachAniEnv() {
    ani_status status = ANI_ERROR;
    if ((status = etsVm_->DetachCurrentThread()) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status: %{public}d", status);
    }
}
} // namespace AbilityRuntime
} // namespace OHOS