/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "sts_app_foreground_state_observer.h"
#include "hilog_tag_wrapper.h"
#include "sts_app_manager_utils.h"


namespace OHOS {
namespace AppManagerSts {
AniAppForegroundStateObserver::AniAppForegroundStateObserver(ani_env *env) : env_(env) {}

void AniAppForegroundStateObserver::OnAppStateChanged(const AppStateData &appStateData)
{
    HandleOnAppStateChanged(appStateData);
}

void AniAppForegroundStateObserver::HandleOnAppStateChanged(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    ani_object appStateDataObj;
    if (!ConvertToAniAppStateData(env_, appStateData, appStateDataObj)) {
        TAG_LOGE(AAFwkTag::APPMGR, "ConvertToAniAppStateData faild");
        return;
    }

    auto tmpMap = aniObserverObjectSet_;
    for (auto &item : tmpMap) {
        if (item == nullptr) {
            continue;
        }
        auto pReference = item->getReference();
        if (pReference == nullptr || pReference->aniRef == nullptr) {
            continue;
        }
        if (ANI_OK !=
            env_->Object_CallMethodByName_Void(static_cast<ani_object>(pReference->aniRef), "onAppStateChanged", nullptr, appStateDataObj)) {
                TAG_LOGE(AAFwkTag::APPMGR, "'onAppStarted' Object_CallMethodByName_Void faild");
            }
    }
}

void AniAppForegroundStateObserver::AddAniObserverObject(const ani_object &aniServerObject)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    if (aniServerObject == nullptr) {
        TAG_LOGD(AAFwkTag::APPMGR, "null observer");
        return;
    }
    
    std::lock_guard<std::mutex> lock(aniObserverObjectSetLock_);
    if (GetObserverObject(aniServerObject) != nullptr) {
        TAG_LOGD(AAFwkTag::APPMGR, "observer exist");
        return;
    }
    std::shared_ptr<AutoSTSReference> pReference = std::make_shared<AutoSTSReference>(env_, aniServerObject);
    if (!pReference->isInit()) {
        TAG_LOGD(AAFwkTag::APPMGR, "AutoSTSReference init faild");
        return;
    }
    aniObserverObjectSet_.emplace(pReference);
}

void AniAppForegroundStateObserver::RemoveAniObserverObject(const ani_object &aniServerObject)
{
    std::cout << "RemoveAniObserverObject enter" << std::endl;
    if (aniServerObject == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null observer");
        return;
    }
    std::lock_guard<std::mutex> lock(aniObserverObjectSetLock_);
    auto pReference = GetObserverObject(aniServerObject);
    if (pReference == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "pReference is nullptr");
        std::cout << "pReference is nullptr" << std::endl;
        return;
    }
    aniObserverObjectSet_.erase(pReference);
}

void AniAppForegroundStateObserver::RemoveAllAniObserverObjects()
{
    std::lock_guard<std::mutex> lock(aniObserverObjectSetLock_);
    aniObserverObjectSet_.clear();
}

bool AniAppForegroundStateObserver::IsEmpty()
{
    std::lock_guard<std::mutex> lock(aniObserverObjectSetLock_);
    return aniObserverObjectSet_.empty();
}

std::shared_ptr<AutoSTSReference> AniAppForegroundStateObserver::GetObserverObject(const ani_object &aniServerObject)
{
    if (aniServerObject == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null observer");
        return nullptr;
    }

    for (auto &pReference : aniObserverObjectSet_) {
        if (pReference == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null observer");
            continue;
        }
        if (pReference->compare(aniServerObject)) {
            return pReference;
        }
    }
    return nullptr;
}
} // namespace AppManagerSts
} // namespace OHOS