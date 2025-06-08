/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "sts_app_state_observer.h"

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
    "Lapplication/ApplicationStateObserver/ApplicationStateObserver;";

StsAppStateObserver::StsAppStateObserver(ani_vm *etsVm) : etsVm_(etsVm) {}

StsAppStateObserver::~StsAppStateObserver() = default;

void StsAppStateObserver::OnForegroundApplicationChanged(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnForegroundApplicationChanged bundleName:%{public}s,uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid appmgr");
        return;
    }
    HandleOnForegroundApplicationChanged(appStateData);
}

void StsAppStateObserver::HandleOnForegroundApplicationChanged(const AppStateData &appStateData)
{
    ani_env *env = nullptr;
    if (!GetAniEnv(env)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to getAniEnv");
        return;
    }
    auto tmpMap = stsObserverObjectMap_;
    for (auto &item : tmpMap) {
        auto appStateDataObj = AppManagerSts::WrapAppStateData(env, appStateData);
        if (appStateDataObj == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null appStateDataObj");
            return;
        }
        CallStsFunction(env, item.second, "onForegroundApplicationChanged", SIGNATURE_APP_STATE_DATA, appStateDataObj);
    }
    DetachAniEnv();
}

void StsAppStateObserver::OnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnAbilityStateChanged");
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr may has cancelled storage");
        return;
    }
    HandleOnAbilityStateChanged(abilityStateData);
}

void StsAppStateObserver::HandleOnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    ani_env *env = nullptr;
    if (!GetAniEnv(env)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to getAniEnv");
        return;
    }
    auto tmpMap = stsObserverObjectMap_;
    for (auto &item : tmpMap) {
        auto abilityStateDataObj = OHOS::AppExecFwk::WrapAbilityStateData(env, abilityStateData);
        CallStsFunction(env, item.second, "onAbilityStateChanged", SIGNATURE_ABILITY_STATE_DATA, abilityStateDataObj);
    }
    DetachAniEnv();
}

void StsAppStateObserver::OnExtensionStateChanged(const AbilityStateData &abilityStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr may destroyed");
        return;
    }
    HandleOnExtensionStateChanged(abilityStateData);
}

void StsAppStateObserver::HandleOnExtensionStateChanged(const AbilityStateData &abilityStateData)
{
    ani_env *env = nullptr;
    if (!GetAniEnv(env)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to getAniEnv");
        return;
    }
    auto tmpMap = stsObserverObjectMap_;
    for (auto &item : tmpMap) {
        auto abilityStateDataObj = OHOS::AppExecFwk::WrapAbilityStateData(env, abilityStateData);
        CallStsFunction(env, item.second, "onAbilityStateChanged", SIGNATURE_ABILITY_STATE_DATA, abilityStateDataObj);
    }
    DetachAniEnv();
}

void StsAppStateObserver::OnProcessCreated(const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnProcessCreated");
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appmgr may has cancelled storage");
        return;
    }
    HandleOnProcessCreated(processData);
}

void StsAppStateObserver::HandleOnProcessCreated(const ProcessData &processData)
{
    ani_env *env = nullptr;
    if (!GetAniEnv(env)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to getAniEnv");
        return;
    }
    auto tmpMap = stsObserverObjectMap_;
    for (auto &item : tmpMap) {
        auto processDataObj = AppManagerSts::WrapProcessData(env, processData);
        if (processDataObj == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null processDataObj");
            return;
        }
        CallStsFunction(env, item.second, "onProcessCreated", SIGNATURE_PROCESS_DATA, processDataObj);
    }
    DetachAniEnv();
}

void StsAppStateObserver::OnProcessStateChanged(const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr may destroyed");
        return;
    }
    HandleOnProcessStateChanged(processData);
}

void StsAppStateObserver::HandleOnProcessStateChanged(const ProcessData &processData)
{
    ani_env *env = nullptr;
    if (!GetAniEnv(env)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to getAniEnv");
        return;
    }
    auto tmpMap = stsObserverObjectMap_;
    for (auto &item : tmpMap) {
        auto processDataObj = AppManagerSts::WrapProcessData(env, processData);
        if (processDataObj == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null processDataObj");
            return;
        }
        CallStsFunction(env, item.second, "onProcessStateChanged", SIGNATURE_PROCESS_DATA, processDataObj);
    }
    DetachAniEnv();
}

void StsAppStateObserver::OnProcessDied(const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appmgr may destroyed");
        return;
    }
    HandleOnProcessDied(processData);
}

void StsAppStateObserver::HandleOnProcessDied(const ProcessData &processData)
{
    ani_env *env = nullptr;
    if (!GetAniEnv(env)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to getAniEnv");
        return;
    }
    auto tmpMap = stsObserverObjectMap_;
    for (auto &item : tmpMap) {
        auto processDataObj = AppManagerSts::WrapProcessData(env, processData);
        if (processDataObj == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null processDataObj");
            return;
        }
        CallStsFunction(env, item.second, "onProcessDied", SIGNATURE_PROCESS_DATA, processDataObj);
    }
    DetachAniEnv();
}

void StsAppStateObserver::OnAppStarted(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr may destroyed");
        return;
    }
    HandleOnAppStarted(appStateData);
}

void StsAppStateObserver::HandleOnAppStarted(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    ani_env *env = nullptr;
    if (!GetAniEnv(env)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to getAniEnv");
        return;
    }
    auto tmpMap = stsObserverObjectMap_;
    for (auto &item : tmpMap) {
        auto appStateDataObj = AppManagerSts::WrapAppStateData(env, appStateData);
        if (appStateDataObj == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null appStateDataObj");
            return;
        }
        CallStsFunction(env, item.second, "onAppStarted", SIGNATURE_APP_STATE_DATA, appStateDataObj);
    }
    DetachAniEnv();
}

void StsAppStateObserver::OnAppStopped(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr may destroyed");
        return;
    }
    HandleOnAppStopped(appStateData);
}

void StsAppStateObserver::HandleOnAppStopped(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    ani_env *env = nullptr;
    if (!GetAniEnv(env)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to getAniEnv");
        return;
    }
    auto tmpMap = stsObserverObjectMap_;
    for (auto &item : tmpMap) {
        auto appStateDataObj = AppManagerSts::WrapAppStateData(env, appStateData);
        if (appStateDataObj == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null appStateDataObj");
            return;
        }
        CallStsFunction(env, item.second, "onAppStopped", SIGNATURE_APP_STATE_DATA, appStateDataObj);
    }
    DetachAniEnv();
}

void StsAppStateObserver::CallStsFunction(
    ani_env *env, ani_object StsObserverObject, const char *methodName, const char *signature, ...)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call method:%{public}s", methodName);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return;
    }
    ani_class cls;
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

void StsAppStateObserver::AddStsObserverObject(ani_env *env, const int32_t observerId, ani_object StsObserverObject)
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

bool StsAppStateObserver::RemoveStsObserverObject(const int32_t observerId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "RemoveStsObserverObject");
    bool result = (stsObserverObjectMap_.erase(observerId) == 1);
    return result;
}

bool StsAppStateObserver::FindObserverByObserverId(const int32_t observerId)
{
    auto item = stsObserverObjectMap_.find(observerId);
    bool isExist = (item != stsObserverObjectMap_.end());
    return isExist;
}

size_t StsAppStateObserver::GetStsObserverMapSize()
{
    size_t length = stsObserverObjectMap_.size();
    return length;
}

void StsAppStateObserver::SetValid(const bool valid)
{
    valid_ = valid;
}

bool StsAppStateObserver::GetAniEnv(ani_env *env) {
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "etsVm_ nullptr");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_option interopEnabled { "--interop=disable", nullptr };
    ani_options aniArgs { 1, &interopEnabled };
    if ((status = (etsVm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status: %{public}d", status);
        return false;
    } 
    if (env == nullptr) {
        return false;
    }
    return true;
}

void StsAppStateObserver::DetachAniEnv() {
    ani_status status = ANI_ERROR;
    if ((status = etsVm_->DetachCurrentThread()) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status: %{public}d", status);
    }
}
} // namespace AbilityRuntime
} // namespace OHOS