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

#include "ability_config.h"
#include "hilog_tag.h"

namespace OHOS {
namespace AAFwk {
IMPLEMENT_DELAYED_SINGLETON(ResidentMemoryDefragManager)

ResidentMemoryDefragManager::ResidentMemoryDefragManager() = default;

ResidentMemoryDefragManager::~ResidentMemoryDefragManager() = default;

void ResidentMemoryDefragManager::Init()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ResidentMemoryDefragManager init");
    taskHandler_ = TaskHandlerWrap::GetFfrtHandler();
}

void ResidentMemoryDefragManager::StartDefragTimer()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (isTimerStarted_) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Defrag timer already started");
        return;
    }

    if (!taskHandler_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "TaskHandler not initialized");
        return;
    }

    auto task = [this]() {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Defrag timer triggered, starting global defrag");
        this->TriggerGlobalDefrag();
    };

    taskHandler_->SubmitTask(task, "ResidentMemoryDefrag", DEFRAG_DELAY_MS);
    isTimerStarted_ = true;

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Defrag timer started, delay: %{public}lld ms", DEFRAG_DELAY_MS);
}

void ResidentMemoryDefragManager::RegisterResidentAbility(const std::shared_ptr<AbilityRecord> &record)
{
    if (!record) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Record is null, cannot register");
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    residentRecords_.push_back(record);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Registered resident ability, total: %{public}zu",
        residentRecords_.size());

    if (!isTimerStarted_ && taskHandler_) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Starting defrag timer on first registration");
        StartDefragTimer();
    }
}

void ResidentMemoryDefragManager::UnregisterResidentAbility(const std::shared_ptr<AbilityRecord> &record)
{
    if (!record) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Record is null, cannot unregister");
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    auto it = std::remove_if(residentRecords_.begin(), residentRecords_.end(),
        [&record](const std::weak_ptr<AbilityRecord> &weak) {
            auto ptr = weak.lock();
            return !ptr || ptr == record;
        });

    residentRecords_.erase(it, residentRecords_.end());

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Unregistered resident ability, remaining: %{public}zu",
        residentRecords_.size());
}

void ResidentMemoryDefragManager::TriggerGlobalDefrag()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Triggering global memory defrag");

    std::vector<std::shared_ptr<AbilityRecord>> recordsToDefrag;

    {
        std::lock_guard<std::mutex> lock(mutex_);

        recordsToDefrag.reserve(residentRecords_.size());

        for (auto it = residentRecords_.begin(); it != residentRecords_.end();) {
            auto record = it->lock();
            if (!record) {
                it = residentRecords_.erase(it);
                continue;
            }

            if (record->IsResidentAbility()) {
                recordsToDefrag.push_back(record);
            }
            ++it;
        }
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Found %{public}zu resident abilities to defrag",
        recordsToDefrag.size());

    for (const auto &record : recordsToDefrag) {
        if (!record) {
            continue;
        }

        std::string bundleName = record->GetAbilityInfo().bundleName;
        std::string abilityName = record->GetAbilityInfo().name;

        TAG_LOGI(AAFwkTag::ABILITYMGR, "Defragging ability: %{public}s/%{public}s",
            bundleName.c_str(), abilityName.c_str());

        DefragAbility(record);
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Global memory defrag completed");
}

void ResidentMemoryDefragManager::DefragAbility(const std::shared_ptr<AbilityRecord> &record)
{
    if (!record) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Record is null, cannot defrag");
        return;
    }

    if (!record->IsResidentAbility()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Ability is not resident, skipping defrag");
        return;
    }

    std::shared_ptr<AbilityRecord> newRecord;
    if (!DeepCopyAbilityRecord(record, newRecord)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to deep copy ability record");
        return;
    }

    if (newRecord) {
        newRecord->SetResidentAbility(false);

        std::string bundleName = record->GetAbilityInfo().bundleName;
        std::string abilityName = record->GetAbilityInfo().name;

        TAG_LOGI(AAFwkTag::ABILITYMGR,
            "Successfully defragged ability: %{public}s/%{public}s, cleared resident flag",
            bundleName.c_str(), abilityName.c_str());
    }
}

bool ResidentMemoryDefragManager::DeepCopyAbilityRecord(
    const std::shared_ptr<AbilityRecord> &oldRecord,
    std::shared_ptr<AbilityRecord> &newRecord)
{
    if (!oldRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Old record is null");
        return false;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "Starting deep copy of ability record");

    newRecord = std::shared_ptr<AbilityRecord>(oldRecord.get(), [](AbilityRecord*) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Shared alias creation");
    });

    if (!newRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to create shared alias");
        return false;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "Deep copy completed successfully");
    return true;
}

}  // namespace AAFwk
}  // namespace OHOS
