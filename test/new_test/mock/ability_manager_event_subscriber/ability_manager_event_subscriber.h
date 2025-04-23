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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_EVENT_SUBSCRIBER_H
#define MOCK_OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_EVENT_SUBSCRIBER_H

#include <mutex>
#include <unordered_set>
#include "matching_skills.h"

namespace OHOS {
namespace EventFwk {
class CommonEventData {};

class CommonEventSubscriber {
};

class CommonEventManager {
public:
    static bool SubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber)
    {
        return false;
    }
    static bool UnSubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber)
    {
        return false;
    }
};

struct CommonEventSubscribeInfo {
    explicit CommonEventSubscribeInfo(const MatchingSkills &matchingSkills) {}
    enum ThreadMode {
        HANDLER,     // the main thread of this ability.
        POST,        // the event dispatch thread.
        ASYNC,       // an asynchronous thread.
        BACKGROUND,  // the background thread.
        COMMON,      // common listening thread of a process.
    };

    void SetThreadMode(CommonEventSubscribeInfo::ThreadMode threadMode) {}
};
}
namespace AbilityRuntime {
class AbilityManagerEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    explicit AbilityManagerEventSubscriber(
        const EventFwk::CommonEventSubscribeInfo &subscribeInfo, const std::function<void()> &screenUnlockCallback,
        const std::function<void()> &userScreenUnlockCallback) {}
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif