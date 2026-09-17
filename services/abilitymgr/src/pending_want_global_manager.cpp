/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "pending_want_global_manager.h"

#include "ams_configuration_parameter.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {

void PendingWantGlobalManager::AddSharedRecord(sptr<PendingWantRecord> record)
{
    std::lock_guard<ffrt::mutex> locker(mutex_);
    GlobalPendingRecord entry;
    entry.record = record;
    globalRecords_.push_back(std::move(entry));
}

void PendingWantGlobalManager::ShrinkIfNeeded()
{
    std::lock_guard<ffrt::mutex> locker(mutex_);

    size_t threshold = static_cast<size_t>(
        AmsConfigurationParameter::GetInstance().GetWantAgentGlobalMaxCount());
    if (globalRecords_.size() <= threshold) {
        return;
    }

    size_t removeCount =
        static_cast<size_t>(globalRecords_.size() * shrinkRatio_);

    TAG_LOGI(AAFwkTag::WANTAGENT,
        "Global list overflow: %{public}zu > %{public}zu, "
        "shrinking by %{public}zu", globalRecords_.size(), threshold, removeCount);

    auto it = globalRecords_.begin();
    for (size_t i = 0; i < removeCount && it != globalRecords_.end(); ++i, ++it) {
        if (it->record != nullptr) {
            it->record->SetCanceled();
        }
    }
    globalRecords_.erase(globalRecords_.begin(), it);
}

} // namespace AAFwk
} // namespace OHOS