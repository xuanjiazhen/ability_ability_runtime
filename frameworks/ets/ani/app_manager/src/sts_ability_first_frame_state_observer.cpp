/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifdef SUPPORT_GRAPHICS
#include "sts_ability_first_frame_state_observer.h"
#include "sts_app_manager_utils.h"

namespace OHOS {
namespace AppManagerSts {
STSAbilityFirstFrameStateObserver::STSAbilityFirstFrameStateObserver(ani_env *env) : env_(env) {}

void STSAbilityFirstFrameStateObserver::OnAbilityFirstFrameState(const AbilityFirstFrameStateData &abilityFirstFrameStateData)
{
    HandleOnAbilityFirstFrameState(abilityFirstFrameStateData);
}

void STSAbilityFirstFrameStateObserver::HandleOnAbilityFirstFrameState(const AbilityFirstFrameStateData &abilityFirstFrameStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    ani_object firstFrameStateDataObj = nullptr;
    if (!ConvertToAniAbilityFirstFrameStateData(env_, abilityFirstFrameStateData, firstFrameStateDataObj)) {
        TAG_LOGD(AAFwkTag::APPMGR, "ConvertToAniAbilityFirstFrameStateData faild");
        return;
    }
    if (stsObserverObject_ == nullptr) return;
    auto pReference = stsObserverObject_->getReference();
    if (pReference == nullptr || pReference->aniRef == nullptr) return;
    if (ANI_OK !=
        env_->Object_CallMethodByName_Void(static_cast<ani_object>(pReference->aniRef), "onAbilityFirstFrameDrawn", nullptr, firstFrameStateDataObj)) {
            TAG_LOGE(AAFwkTag::APPMGR, "'onAbilityFirstFrameDrawn' Object_CallMethodByName_Void faild");
        }
}

void STSAbilityFirstFrameStateObserver::SetStsObserverObject(const ani_object &stsObserverObject)
{
    if (stsObserverObject == nullptr) {
        TAG_LOGD(AAFwkTag::APPMGR, "stsObserverObject is null");
        return;
    }
    stsObserverObject_ = std::make_shared<AutoSTSReference>(env_, stsObserverObject);
}

void STSAbilityFirstFrameStateObserver::ResetStsObserverObject()
{
    stsObserverObject_ = nullptr;
}

std::shared_ptr<AutoSTSReference> STSAbilityFirstFrameStateObserver::GetAutoSTSReference()
{
    return stsObserverObject_;
}
void STSAbilityFirstFrameStateObserverManager::Init(ani_env *env)
{
    env_ = env;
}

void STSAbilityFirstFrameStateObserverManager::AddSTSAbilityFirstFrameStateObserver(const sptr<STSAbilityFirstFrameStateObserver> observer)
{
    if (observer == nullptr || observer->GetAutoSTSReference() == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null observer");
        return;
    }
    std::lock_guard<std::mutex> lock(observerListLock_);
    stsAbilityFirstFrameStateObserverList_.emplace_back(observer);
}

bool STSAbilityFirstFrameStateObserverManager::IsObserverObjectExist(const ani_object &stsObserverObject)
{
    if (GetObserverObject(stsObserverObject) == nullptr) {
        TAG_LOGD(AAFwkTag::APPMGR, "observer not exist");
        return false;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "observer exist");
    return true;
}

void STSAbilityFirstFrameStateObserverManager::RemoveAllStsObserverObjects(sptr<OHOS::AAFwk::IAbilityManager> &abilityManager)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null abilityManager");
        return;
    }
    std::lock_guard<std::mutex> lock(observerListLock_);
    for (auto observer : stsAbilityFirstFrameStateObserverList_) {
        abilityManager->UnregisterAbilityFirstFrameStateObserver(observer);
        observer->ResetStsObserverObject();
    }
    stsAbilityFirstFrameStateObserverList_.clear();
}

void STSAbilityFirstFrameStateObserverManager::RemoveStsObserverObject(
    sptr<OHOS::AAFwk::IAbilityManager> &abilityManager, const ani_object &stsObserverObject)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null abilityManager");
        return;
    }
    std::lock_guard<std::mutex> lock(observerListLock_);
    auto it = stsAbilityFirstFrameStateObserverList_.begin();
    for (; it != stsAbilityFirstFrameStateObserverList_.end(); ++it) {
        std::shared_ptr<AutoSTSReference> tmpObject = (*it)->GetAutoSTSReference();
        if (tmpObject == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null value");
            continue;
        }
        if (tmpObject->compare(stsObserverObject)) {
            abilityManager->UnregisterAbilityFirstFrameStateObserver(*it);
            (*it)->ResetStsObserverObject();
            stsAbilityFirstFrameStateObserverList_.erase(it);
            return;
        }
    }
}

std::shared_ptr<AutoSTSReference> STSAbilityFirstFrameStateObserverManager::GetObserverObject(const ani_object &stsObserverObject)
{
    if (stsObserverObject == nullptr) {
        TAG_LOGD(AAFwkTag::APPMGR, "null observer");
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(observerListLock_);
    for (auto &observer : stsAbilityFirstFrameStateObserverList_) {
        if (observer == nullptr) {
            TAG_LOGD(AAFwkTag::APPMGR, "null observer");
            continue;
        }
        std::shared_ptr<AutoSTSReference> tmpObject = observer->GetAutoSTSReference();
        if (tmpObject == nullptr) {
            TAG_LOGD(AAFwkTag::APPMGR, "null value");
            continue;
        }
        if (tmpObject->compare(stsObserverObject)) {
            return tmpObject;
        }
    }
    return nullptr;
}
} // namespace AppManagerSts
} // namespace OHOS
#endif // SUPPORT_GRAPHICS