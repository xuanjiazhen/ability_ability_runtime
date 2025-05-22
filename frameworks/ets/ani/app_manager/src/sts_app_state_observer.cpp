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
#include "hilog_tag_wrapper.h"
#include "ani_common_util.h"
#include "ani_common_want.h"
#include "sts_app_manager_utils.h"
#include "ani_common_ability_state_data.h"
namespace OHOS {
namespace AbilityRuntime {

StsAppStateObserver::StsAppStateObserver(ani_vm *etsVm) : etsVm_(etsVm) {}

StsAppStateObserver::~StsAppStateObserver() = default;

void StsAppStateObserver::OnForegroundApplicationChanged(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid appmgr");
        return;
    }
    HandleOnForegroundApplicationChanged(appStateData);
}

void StsAppStateObserver::HandleOnForegroundApplicationChanged(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
        if (etsVm_ == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "etsVm_ nullptr");
            return;
        }
        ani_env *env = nullptr;
        ani_status status = ANI_ERROR;
        ani_option interopEnabled { "--interop=disable", nullptr };
        ani_options aniArgs { 1, &interopEnabled };
        if ((etsVm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::STSRUNTIME, "status : %{public}d", status);
            return;
        }
    auto tmpMap = stsObserverObjectMap_;
    for (auto &item : tmpMap) {
        auto appStateDataObj = AppManagerSts::WrapAppStateData(env, appStateData);
        const char *signature = "Lapplication/AppStateData/AppStateData;:V";
        CallStsFunction(env, item.second, "onForegroundApplicationChanged", signature, appStateDataObj);
    }
    if (etsVm_->DetachCurrentThread() != ANI_OK) {
        TAG_LOGE(AAFwkTag::STSRUNTIME, "status : %{public}d", status);
    }
}

void StsAppStateObserver::OnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr may has cancelled storage");
        return;
    }
    HandleOnAbilityStateChanged(abilityStateData);
}

void StsAppStateObserver::HandleOnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "etsVm_ nullptr");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    ani_option interopEnabled { "--interop=disable", nullptr };
    ani_options aniArgs { 1, &interopEnabled };
    if ((etsVm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::STSRUNTIME, "status : %{public}d", status);
        return;
    }
    auto tmpMap = stsObserverObjectMap_;
    for (auto &item : tmpMap) {
        auto abilityStateDataObj = OHOS::AppExecFwk::WrapAbilityStateData(env, abilityStateData);
        const char *signature = "Lapplication/AbilityStateData/AbilityStateData;:V";
        CallStsFunction(env, item.second, "onAbilityStateChanged", signature, abilityStateDataObj);
    }
    if (etsVm_->DetachCurrentThread() != ANI_OK) {
        TAG_LOGE(AAFwkTag::STSRUNTIME, "status : %{public}d", status);
    }
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
    TAG_LOGD(AAFwkTag::APPMGR, "called");
}

void StsAppStateObserver::OnProcessCreated(const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appmgr may has cancelled storage");
        return;
    }
    HandleOnProcessCreated(processData);
}

void StsAppStateObserver::HandleOnProcessCreated(const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
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
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "etsVm_ nullptr");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    ani_option interopEnabled { "--interop=disable", nullptr };
    ani_options aniArgs { 1, &interopEnabled };
    if ((etsVm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::STSRUNTIME, "wyh status : %{public}d", status);
        return;
    }
    auto tmpMap = stsObserverObjectMap_;
    for (auto &item : tmpMap) {
        auto processDataObj = AppManagerSts::WrapProcessData(env, processData);
        const char *signature = "Lapplication/ProcessData/ProcessData;:V";
        CallStsFunction(env, item.second, "onProcessStateChanged", signature, processDataObj);
    }
    if (etsVm_->DetachCurrentThread() != ANI_OK) {
        TAG_LOGE(AAFwkTag::STSRUNTIME, "status : %{public}d", status);
    }
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
    TAG_LOGD(AAFwkTag::APPMGR, "called");
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
    TAG_LOGD(AAFwkTag::APPMGR,
        "bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
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
    TAG_LOGD(AAFwkTag::APPMGR,
        "bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
}

void StsAppStateObserver::CallStsFunction(ani_env* env, ani_object StsObserverObject,
    const char *methodName, const char *signature, ...)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call method:%{public}s", methodName);

    ani_class cls;
    ani_method method = nullptr;
    ani_status status = ANI_OK;
    status = env->FindClass("Lapplication/ApplicationStateObserver/ApplicationStateObserver;", &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "find ApplicationStateObserver failed status : %{public}d", status);
        return;
    }
    if ((status = env->Class_FindMethod(cls, methodName, signature, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    env->ResetError();
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(StsObserverObject, method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    va_end(args);
    TAG_LOGD(AAFwkTag::APPMGR, "end");
    return;
}

void StsAppStateObserver::AddStsObserverObject(ani_env *env, const int32_t observerId, ani_object StsObserverObject)
{
    ani_ref global = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GlobalReference_Create(StsObserverObject, &global)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status : %{public}d", status);
        return;
    }
    stsObserverObjectMap_.emplace(observerId, reinterpret_cast<ani_object>(global));
}

bool StsAppStateObserver::RemoveStsObserverObject(const int32_t observerId)
{
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
}  // namespace AbilityRuntime
}  // namespace OHOS