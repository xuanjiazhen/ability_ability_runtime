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

#include "ability_manager_event_subscriber.h"

#include "ability_manager_errors.h"
#include "ability_util.h"
#include "common_event_support.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
AbilityEventMapManager::AbilityEventMapManager() {}
AbilityEventMapManager::~AbilityEventMapManager() {}

void AbilityEventMapManager::AddEvent(int32_t userId, const std::string& event)
{
    std::lock_guard<std::mutex> lock(mutex_);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AddEvent userId: %{public}d, event: %{public}s.",
        userId, event.c_str());
    auto iter = eventMap_.find(userId);
    if (iter != eventMap_.end()) {
        if (event == EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED) {
            iter->second.userUnlock_ = true;
            return;
        }
        iter->second.screenUnlock_ = true;
        return;
    }
    if (event == EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED) {
        eventMap_[userId] = LockEventData(true, false);
        return;
    }
    eventMap_[userId] = LockEventData(false, true);
}

void AbilityEventMapManager::RemoveUser(int32_t userId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "RemoveUser userId: %{public}d.", userId);
    eventMap_.erase(userId);
}

bool AbilityEventMapManager::CheckAllUnlocked(int32_t userId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = eventMap_.find(userId);
    if (iter != eventMap_.end()) {
        return iter->second.userUnlock_ && iter->second.screenUnlock_;
    }
    return false;
}

void AbilityEventMapManager::ClearAllEvents()
{
    std::lock_guard<std::mutex> lock(mutex_);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ClearAllEvents");
    eventMap_.clear();
}

AMSScreenUnlockEventSubscriber::AMSScreenUnlockEventSubscriber(
    const EventFwk::CommonEventSubscribeInfo &subscribeInfo, const std::function<void(int32_t)> &screenUnlockCallback)
    : EventFwk::CommonEventSubscriber(subscribeInfo), screenUnlockCallback_(screenUnlockCallback)
{}

void AMSScreenUnlockEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    const AAFwk::Want &want = data.GetWant();
    std::string action = want.GetAction();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "The action: %{public}s.", action.c_str());
    if (action != EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED) {
        return;
    }
    if (screenUnlockCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "screenUnlockCallback_ nullptr");
        return;
    }
    auto eventMapManager = DelayedSingleton<AbilityEventMapManager>::GetInstance();
    CHECK_POINTER_LOG(eventMapManager, "eventMapManager nullptr");
    eventMapManager->AddEvent(userId, action);
    if (eventMapManager->CheckAllUnlocked(userId)) {
        screenUnlockCallback_(userId);
        eventMapManager->RemoveUser(userId);
    }
}

AMSUserUnlockEventSubscriber::AMSUserUnlockEventSubscriber(
    const EventFwk::CommonEventSubscribeInfo &subscribeInfo, const std::function<void(int32_t)> &screenUnlockCallback,
    const std::function<void()> &userScreenUnlockCallback)
    : EventFwk::CommonEventSubscriber(subscribeInfo), screenUnlockCallback_(screenUnlockCallback),
    userScreenUnlockCallback_(userScreenUnlockCallback)
{}

void AMSUserUnlockEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    const AAFwk::Want &want = data.GetWant();
    std::string action = want.GetAction();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "The action: %{public}s.", action.c_str());
    if (action != EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED) {
        return;
    }
    if (userScreenUnlockCallback_ == nullptr || screenUnlockCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "nullptr callback");
        return;
    }
    auto eventMapManager = DelayedSingleton<AbilityEventMapManager>::GetInstance();
    CHECK_POINTER_LOG(eventMapManager, "eventMapManager nullptr");
    eventMapManager->AddEvent(userId, action);
    if (eventMapManager->CheckAllUnlocked(userId)) {
        screenUnlockCallback_(userId);
        eventMapManager->RemoveUser(userId);
        return;
    }
    userScreenUnlockCallback_();
}
} // namespace AbilityRuntime
} // namespace OHOS