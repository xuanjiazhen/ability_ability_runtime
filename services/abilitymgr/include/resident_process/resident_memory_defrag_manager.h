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

#ifndef OHOS_ABILITY_RUNTIME_RESIDENT_MEMORY_DEFRAG_MANAGER_H
#define OHOS_ABILITY_RUNTIME_RESIDENT_MEMORY_DEFRAG_MANAGER_H

#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <chrono>
#include "ability_config.h"

namespace OHOS {
namespace AAFwk {
class AbilityRecord;

// Global manager for resident ability memory defragmentation
// This manager maintains weak_ptr references to resident abilities and triggers
// deep copy after 140 seconds to reduce memory fragmentation
class ResidentMemoryDefragManager {
public:
    static ResidentMemoryDefragManager& GetInstance()
    {
        static ResidentMemoryDefragManager instance;
        return instance;
    }

    // Register a resident ability for memory defragmentation
    // Called from AbilityRecord::Init() when PARAM_IS_RESIDENT_ABILITY is set
    void RegisterAbility(const std::string &bundleName, const std::string &abilityName,
        const std::weak_ptr<AbilityRecord> &weakRecord);

    // Unregister a resident ability
    void UnregisterAbility(const std::string &bundleName, const std::string &abilityName);

private:
    ResidentMemoryDefragManager();
    ~ResidentMemoryDefragManager();
    ResidentMemoryDefragManager(const ResidentMemoryDefragManager&) = delete;
    ResidentMemoryDefragManager& operator=(const ResidentMemoryDefragManager&) = delete;

    // Start the memory defragmentation task for a specific ability
    void StartMemoryDefragTask(const std::string &bundleName, const std::string &abilityName,
        const std::weak_ptr<AbilityRecord> &weakRecord);

    // Map of ability key (bundleName/abilityName) to weak_ptr
    std::unordered_map<std::string, std::weak_ptr<AbilityRecord>> abilityRecords_;
    std::mutex mapMutex_;
};

}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_RESIDENT_MEMORY_DEFRAG_MANAGER_H
