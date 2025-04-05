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
#include "sts_app_state_observer.h"
#include "hilog_tag_wrapper.h"
#include "sts_app_manager_utils.h"

namespace OHOS {
namespace AppManagerSts {
AniAppStateObserver::AniAppStateObserver(ani_env *env) : env_(env) {}

AniAppStateObserver::~AniAppStateObserver() {
    RemoveAllAniObserverObject();
}

void AniAppStateObserver::OnForegroundApplicationChanged(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnForegroundApplicationChanged called");
    this->HandleOnForegroundApplicationChanged(appStateData);
}

void AniAppStateObserver::OnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnAbilityStateChanged called");
    this->HandleOnAbilityStateChanged(abilityStateData);
}

void AniAppStateObserver::OnExtensionStateChanged(const AbilityStateData &abilityStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnExtensionStateChanged called");
    this->HandleOnExtensionStateChanged(abilityStateData);
}

void AniAppStateObserver::OnProcessCreated(const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnProcessCreated called");
    this->HandleOnProcessCreated(processData);
}

void AniAppStateObserver::OnProcessStateChanged(const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnProcessStateChanged called");
    this->HandleOnProcessStateChanged(processData);
}

void AniAppStateObserver::OnProcessDied(const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnProcessDied called");
    this->HandleOnProcessDied(processData);
}

void OHOS::AppManagerSts::AniAppStateObserver::HandleOnForegroundApplicationChanged(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "HandleOnForegroundApplicationChanged called");
    TAG_LOGI(AAFwkTag::APPMGR, "bundleName:%{public}s, uid:%{public}d, state:%{public}d.",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);

    ani_object appStateDataObj = nullptr;
    if (!ConvertToAniAppStateData(env_, appStateData, appStateDataObj)) {
        TAG_LOGE(AAFwkTag::APPMGR, "ConvertToAniAppStateData faild");
        return;
    }

    auto tmpMap = AniObserverObjectMap_;
    for (auto &item : tmpMap) {
        if (item.second == nullptr) {
            continue;
        }
        auto pReference = item.second->getReference();
        if (pReference == nullptr || pReference->aniRef == nullptr) {
            continue;
        }
        if (ANI_OK !=
            env_->Object_CallMethodByName_Void(static_cast<ani_object>(pReference->aniRef), 
                    "onForegroundApplicationChanged", nullptr, appStateDataObj)) {
                TAG_LOGE(AAFwkTag::APPMGR, "'onForegroundApplicationChanged' Object_CallMethodByName_Void faild");
            }
    }
}

void AniAppStateObserver::HandleOnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "HandleOnAbilityStateChanged called");
    ani_object abilityStateDataObj;
    if (!ConvertToAniAbilityStateData(env_, abilityStateData, abilityStateDataObj)) {
        TAG_LOGE(AAFwkTag::APPMGR, "ConvertToAniAbilityStateData faild");
        return;
    }

    auto tmpMap = AniObserverObjectMap_;
    for (auto &item : tmpMap) {
        if (item.second == nullptr) {
            continue;
        }
        auto pReference = item.second->getReference();
        if (pReference == nullptr || pReference->aniRef == nullptr) {
            continue;
        }
        if (ANI_OK !=
            env_->Object_CallMethodByName_Void(static_cast<ani_object>(pReference->aniRef),
                    "onAbilityStateChanged", nullptr, abilityStateDataObj)) {
                TAG_LOGE(AAFwkTag::APPMGR, "'onAbilityStateChanged' Object_CallMethodByName_Void faild");
            }
    }
}

void AniAppStateObserver::HandleOnExtensionStateChanged(const AbilityStateData &abilityStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "HandleOnExtensionStateChanged called");
    ani_object abilityStateDataObj;
    if (!ConvertToAniAbilityStateData(env_, abilityStateData, abilityStateDataObj)) {
        TAG_LOGE(AAFwkTag::APPMGR, "ConvertToAniAbilityStateData faild");
        return;
    }
    auto tmpMap = AniObserverObjectMap_;
    for (auto &item : tmpMap) {
        if (item.second == nullptr) {
            continue;
        }
        auto pReference = item.second->getReference();
        if (pReference == nullptr || pReference->aniRef == nullptr) {
            continue;
        }
        if (ANI_OK !=
            env_->Object_CallMethodByName_Void(static_cast<ani_object>(pReference->aniRef),
                    "onAbilityStateChanged", nullptr, abilityStateDataObj)) {
                TAG_LOGE(AAFwkTag::APPMGR, "'onAbilityStateChanged' Object_CallMethodByName_Void faild");
            }
    }
}

void AniAppStateObserver::HandleOnProcessCreated(const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "HandleOnProcessCreated called");

    ani_object processDataObj;
    if (!ConvertToAniProcessData(env_, processData, processDataObj)) {
        TAG_LOGE(AAFwkTag::APPMGR, "ConvertToAniProcessData faild");
        return;
    }
    auto tmpMap = AniObserverObjectMap_;
    for (auto &item : tmpMap) {
        if (item.second == nullptr) {
            continue;
        }
        auto pReference = item.second->getReference();
        if (pReference == nullptr || pReference->aniRef == nullptr) {
            continue;
        }
        if (ANI_OK !=
            env_->Object_CallMethodByName_Void(static_cast<ani_object>(pReference->aniRef),
                    "onProcessCreated", nullptr, processDataObj)) {
                TAG_LOGE(AAFwkTag::APPMGR, "'onProcessCreated' Object_CallMethodByName_Void faild");
            }
        std::cout << item.first << std::endl;
    }
}

void AniAppStateObserver::HandleOnProcessStateChanged(const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "HandleOnProcessStateChanged called");
    ani_object processDataObj;
    if (!ConvertToAniProcessData(env_, processData, processDataObj)) {
        TAG_LOGE(AAFwkTag::APPMGR, "ConvertToAniProcessData faild");
        return;
    }
    auto tmpMap = AniObserverObjectMap_;
    for (auto &item : tmpMap) {
        if (item.second == nullptr) {
            continue;
        }
        auto pReference = item.second->getReference();
        if (pReference == nullptr || pReference->aniRef == nullptr) {
            continue;
        }
        if (ANI_OK !=
            env_->Object_CallMethodByName_Void(static_cast<ani_object>(pReference->aniRef),
                    "onProcessStateChanged", nullptr, processDataObj)) {
                TAG_LOGE(AAFwkTag::APPMGR, "'onProcessStateChanged' Object_CallMethodByName_Void faild");
            }
        std::cout << item.first << std::endl;
    }
}

void AniAppStateObserver::HandleOnProcessDied(const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "HandleOnProcessDied called");
    ani_object processDataObj;
    if (!ConvertToAniProcessData(env_, processData, processDataObj)) {
        TAG_LOGE(AAFwkTag::APPMGR, "ConvertToAniProcessData faild");
        return;
    }
    auto tmpMap = AniObserverObjectMap_;
    for (auto &item : tmpMap) {
        if (item.second == nullptr) {
            continue;
        }
        auto pReference = item.second->getReference();
        if (pReference == nullptr || pReference->aniRef == nullptr) {
            continue;
        }
        if (ANI_OK !=
            env_->Object_CallMethodByName_Void(static_cast<ani_object>(pReference->aniRef),
                    "onProcessDied", nullptr, processDataObj)) {
                TAG_LOGE(AAFwkTag::APPMGR, "'onProcessDied' Object_CallMethodByName_Void faild");
            }
    }
}

void AniAppStateObserver::AddAniObserverObject(const int32_t observerId, ani_object AniObserverObject)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AddAniObserverObject called");
    std::shared_ptr<AutoSTSReference> pReference = std::make_shared<AutoSTSReference>(env_, AniObserverObject);
    if (!pReference->isInit()) {
        TAG_LOGD(AAFwkTag::APPMGR, "make AutoSTSReference faild");
        return;
    };
    AniObserverObjectMap_.emplace(observerId, pReference);
}

bool AniAppStateObserver::RemoveAniObserverObject(const int32_t observerId)
{
    auto item = AniObserverObjectMap_.find(observerId);
    if (item == AniObserverObjectMap_.end()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not find observerId. observerId %{public}d", observerId);
        return false;
    }
    AniObserverObjectMap_.erase(item);
    return true;
}

void AniAppStateObserver::RemoveAllAniObserverObject()
{
    AniObserverObjectMap_.clear();
}

bool AniAppStateObserver::FindObserverByObserverId(const int32_t observerId)
{
    auto item = AniObserverObjectMap_.find(observerId);
    bool isExist = (item != AniObserverObjectMap_.end());
    return isExist;
}

size_t AniAppStateObserver::GetAniObserverMapSize()
{
    size_t length = AniObserverObjectMap_.size();
    return length;
}

void AniAppStateObserver::OnAppStarted(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnAppStarted called");
    this->HandleOnAppStarted(appStateData);
}

void AniAppStateObserver::OnAppStopped(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnAppStopped called");
    this->HandleOnAppStopped(appStateData);
}

void AniAppStateObserver::HandleOnAppStarted(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "HandleOnAppStarted called");
    ani_object appStateDataObj = nullptr;
    if (!ConvertToAniAppStateData(env_, appStateData, appStateDataObj)) {
        TAG_LOGE(AAFwkTag::APPMGR, "ConvertToAniAppStateData faild");
        return;
    }
    auto tmpMap = AniObserverObjectMap_;
    for (auto &item : tmpMap) {
        if (item.second == nullptr) {
            continue;
        }
        auto pReference = item.second->getReference();
        if (pReference == nullptr || pReference->aniRef == nullptr) {
            continue;
        }
        if (ANI_OK !=
            env_->Object_CallMethodByName_Void(static_cast<ani_object>(pReference->aniRef),
                    "onAppStarted", nullptr, appStateDataObj)) {
                TAG_LOGE(AAFwkTag::APPMGR, "'onAppStarted' Object_CallMethodByName_Void faild");
            }
    }
}

void AniAppStateObserver::HandleOnAppStopped(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "HandleOnAppStopped called");
    ani_object appStateDataObj = nullptr;
    if (!ConvertToAniAppStateData(env_, appStateData, appStateDataObj)) {
        TAG_LOGE(AAFwkTag::APPMGR, "ConvertToAniAppStateData faild");
        return;
    }
    auto tmpMap = AniObserverObjectMap_;
    for (auto &item : tmpMap) {
        if (item.second == nullptr) {
            continue;
        }
        auto pReference = item.second->getReference();
        if (pReference == nullptr || pReference->aniRef == nullptr) {
            continue;
        }
        if (ANI_OK !=
            env_->Object_CallMethodByName_Void(static_cast<ani_object>(pReference->aniRef),
                    "onAppStopped", nullptr, appStateDataObj)) {
                TAG_LOGE(AAFwkTag::APPMGR, "'onAppStopped' Object_CallMethodByName_Void faild");
            }
    }
}
} // namespace AppManagerSts
} // namespace OHOS
    