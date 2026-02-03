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
#include <vector>
#include <set>

#include "ability_record.h"
#include "singleton.h"
#include "task_handler_wrap.h"

namespace OHOS {
namespace AAFwk {
class ResidentMemoryDefragManager {
    DECLARE_DELAYED_SINGLETON(ResidentMemoryDefragManager)

public:
    void Init();
    void StartDefragTimer();

    void RegisterResidentAbility(const std::shared_ptr<AbilityRecord> &record);
    void UnregisterResidentAbility(const std::shared_ptr<AbilityRecord> &record);

private:
    void TriggerGlobalDefrag();

    void DefragAbility(const std::shared_ptr<AbilityRecord> &record);
    bool DeepCopyAbilityRecord(const std::shared_ptr<AbilityRecord> &oldRecord,
        std::shared_ptr<AbilityRecord> &newRecord);

    std::mutex mutex_;
    std::vector<std::weak_ptr<AbilityRecord>> residentRecords_;

    std::shared_ptr<TaskHandlerWrap> taskHandler_;
    bool isTimerStarted_ = false;

    static constexpr int64_t DEFRAG_DELAY_MS = 140000;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_RESIDENT_MEMORY_DEFRAG_MANAGER_H
