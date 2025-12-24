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
#include "common_event_support.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
AbilityManagerEventSubscriber::AbilityManagerEventSubscriber(
    const EventFwk::CommonEventSubscribeInfo &subscribeInfo, const std::function<void(int32_t)> &screenUnlockCallback,
    const std::function<void()> &userScreenUnlockCallback)
    : EventFwk::CommonEventSubscriber(subscribeInfo), screenUnlockCallback_(screenUnlockCallback),
    userScreenUnlockCallback_(userScreenUnlockCallback)
{}

void AbilityManagerEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    const AAFwk::Want &want = data.GetWant();
    std::string action = want.GetAction();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "The action: %{public}s.", action.c_str());
    if (userScreenUnlockCallback_ == nullptr || screenUnlockCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "nullptr callback");
        return;
    }
    auto handleEvent = [&](const std::string &event, int32_t userId) {
        auto& eventMapManager = AbilityEventMapManager::GetInstance();
        auto result = eventMapManager->CheckAndRemoveEvent(userId, event);
        TAG_LOGD(AAFwkTag::ABILITYMGR, "handleEvent userId: %{public}d, event: %{public}s, checkResult: %{public}d",
            userId, event.c_str(), result);
        if (result == AAFwk::FOUND_USERID_AND_EVENT_IN_EVENT_MAP) {
            screenUnlockCallback_(userId);
            return;
        }
        if (result == AAFwk::NOT_FOUND_USERID_IN_EVENT_MAP) {
            eventMapManager->AddEvent(userId, action);
        }
    };
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED) {
        int32_t userId = data.GetCode();
        TAG_LOGI(AAFwkTag::ABILITYMGR, "COMMON_EVENT_USER_UNLOCKED userId: %{public}d.", userId);
        handleEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED, userId);
        userScreenUnlockCallback_();
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED) {
        int32_t userId = want.GetIntParam("userId", -1);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "COMMON_EVENT_SCREEN_UNLOCKED userId: %{public}d.", userId);
        handleEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED, userId);
    }
}

AbilityEventMapManager::AbilityEventMapManager() {}
AbilityEventMapManager::~AbilityEventMapManager() {}

void AbilityEventMapManager::AddEvent(int32_t userId, const std::string& event)
{
    std::lock_guard<std::mutex> lock(mutex_);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AddEvent userId: %{public}d, event: %{public}s.",
        userId, event.c_str());
    eventMap_[userId] = event;
}

int32_t AbilityEventMapManager::CheckAndRemoveEvent(int32_t userId, const std::string& expectedEvent)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = eventMap_.find(userId);
    if (iter != eventMap_.end()) {
        if (iter->second == expectedEvent) {
            eventMap_.erase(iter);
            return AAFwk::FOUND_USERID_AND_EVENT_IN_EVENT_MAP;
        }
        return AAFwk::FOUND_USERID_ONLY_IN_EVENT_MAP;
    }
    return AAFwk::NOT_FOUND_USERID_IN_EVENT_MAP;
}

void AbilityEventMapManager::ClearAllEvents()
{
    std::lock_guard<std::mutex> lock(mutex_);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ClearAllEvents");
    eventMap_.clear();
}
} // namespace AbilityRuntime
} // namespace OHOS