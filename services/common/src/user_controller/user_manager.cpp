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

#include "user_manager.h"

#include <mutex>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
UserManager& UserManager::GetInstance()
{
    static UserManager instance;
    return instance;
}

void UserManager::ClearUserId(int32_t userId)
{
    std::lock_guard<ffrt::mutex> guard(userLock_);
    for (auto iter = displayIdMap_.begin(); iter != displayIdMap_.end();) {
        if (iter->second == userId) {
            displayIdMap_.erase(iter);
            break;
        }
    }
}

int32_t UserManager::GetCurrentUserId(uint64_t displayId)
{
    std::lock_guard<ffrt::mutex> guard(userLock_);
    auto iter = displayIdMap_.find(displayId);
    if (iter != displayIdMap_.end()) {
        return iter->second;
    }
    return 0;
}

bool UserManager::GetDisplayIdByUserId(int32_t userId, uint64_t &displayId)
{
    std::lock_guard<ffrt::mutex> guard(userLock_);
    for (auto &item : displayIdMap_) {
        if (item.second == userId) {
            displayId = item.first;
            return true;
        }
    }
    return false;
}

bool UserManager::IsCurrentUser(int32_t userId)
{
    std::lock_guard<ffrt::mutex> guard(userLock_);
    for (auto &item : displayIdMap_) {
        if (item.second == userId) {
            return true;
        }
    }
    return false;
}

void UserManager::SetCurrentUserId(int32_t userId, uint64_t displayId)
{
    std::lock_guard<ffrt::mutex> guard(userLock_);
    displayIdMap_[displayId] = userId;
}
}
}