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

#include "resident_memory_defrag_manager.h"
#include "hilog_tag_wrapper.h"
#include "ability_record.h"

namespace OHOS {
namespace AAFwk {
namespace {
const int MEMORY_DEFRAG_DELAY_SECONDS = 140;
}

ResidentMemoryDefragManager::ResidentMemoryDefragManager() = default;

ResidentMemoryDefragManager::~ResidentMemoryDefragManager() = default;

void ResidentMemoryDefragManager::RegisterAbility(const std::string &bundleName,
    const std::string &abilityName, const std::weak_ptr<AbilityRecord> &weakRecord)
{
    if (bundleName.empty() || abilityName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid parameters: bundleName or abilityName is empty");
        return;
    }

    std::string key = bundleName + "/" + abilityName;
    std::lock_guard<std::mutex> lock(mapMutex_);

    // Check if already registered to avoid duplicate tasks
    auto it = abilityRecords_.find(key);
    if (it != abilityRecords_.end()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Ability %{public}s already registered, skipping", key.c_str());
        return;
    }

    abilityRecords_[key] = weakRecord;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Registered resident ability for memory defrag: %{public}s", key.c_str());

    // Start the memory defragmentation task
    StartMemoryDefragTask(bundleName, abilityName, weakRecord);
}

void ResidentMemoryDefragManager::StartMemoryDefragTask(const std::string &bundleName,
    const std::string &abilityName, const std::weak_ptr<AbilityRecord> &weakRecord)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Starting memory defrag task for %{public}s/%{public}s, will trigger after %{public}ds",
        bundleName.c_str(), abilityName.c_str(), MEMORY_DEFRAG_DELAY_SECONDS);

    // Start a background thread to handle memory defragmentation after 140 seconds
    // Capture weak_ptr to safely access AbilityRecord after delay
    std::thread([bundleName, abilityName, weakRecord]() {
        // Sleep for 140 seconds
        std::this_thread::sleep_for(std::chrono::seconds(MEMORY_DEFRAG_DELAY_SECONDS));

        TAG_LOGI(AAFwkTag::ABILITYMGR, "Memory defrag timer triggered for %{public}s/%{public}s",
            bundleName.c_str(), abilityName.c_str());

        // Try to lock the weak_ptr to get shared_ptr
        auto abilityRecord = weakRecord.lock();
        if (abilityRecord == nullptr) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "AbilityRecord has been destroyed for %{public}s/%{public}s, "
                "skipping memory defrag", bundleName.c_str(), abilityName.c_str());
            return;
        }

        // Check if this is still marked as resident ability
        if (!abilityRecord->IsResidentAbility()) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityRecord is no longer marked as resident, skipping defrag");
            return;
        }

        TAG_LOGI(AAFwkTag::ABILITYMGR, "Performing memory defrag for resident ability %{public}s/%{public}s",
            bundleName.c_str(), abilityName.c_str());

        // Create a deep copy of the entire AbilityRecord to trigger memory reallocation
        // This helps reduce memory fragmentation by allocating fresh memory blocks for all members
        auto deepCopiedRecord = abilityRecord->DeepCopy();

        // The deep copied record will be destroyed here, triggering memory deallocation
        // This forces the system to allocate new contiguous memory blocks
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Deep copy completed and destroyed for %{public}s/%{public}s",
            bundleName.c_str(), abilityName.c_str());

        // Clear the resident ability flag to avoid repeated execution
        abilityRecord->SetResidentAbilityFlag(false);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Cleared resident flag for %{public}s/%{public}s",
            bundleName.c_str(), abilityName.c_str());
    }).detach();
}

void ResidentMemoryDefragManager::UnregisterAbility(const std::string &bundleName,
    const std::string &abilityName)
{
    if (bundleName.empty() || abilityName.empty()) {
        return;
    }

    std::string key = bundleName + "/" + abilityName;
    std::lock_guard<std::mutex> lock(mapMutex_);

    auto it = abilityRecords_.find(key);
    if (it != abilityRecords_.end()) {
        abilityRecords_.erase(it);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Unregistered resident ability: %{public}s", key.c_str());
    }
}
}  // namespace AAFwk
}  // namespace OHOS
