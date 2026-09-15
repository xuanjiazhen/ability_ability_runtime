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

#include <chrono>
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {

int64_t PendingWantGlobalManager::GetCurrentTimestampMs() const
{
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

void PendingWantGlobalManager::AddSharedRecord(sptr<PendingWantRecord> record)
{
    std::lock_guard<ffrt::mutex> locker(mutex_);

    GlobalPendingRecord entry;
    entry.record = record;
    entry.transferTime = GetCurrentTimestampMs();
    globalRecords_.push_back(std::move(entry));
}

void PendingWantGlobalManager::ShrinkIfNeeded()
{
    std::lock_guard<ffrt::mutex> locker(mutex_);

    if (globalRecords_.size() <= threshold_) {
        return;
    }

    size_t removeCount =
        static_cast<size_t>(globalRecords_.size() * shrinkRatio_);

    TAG_LOGI(AAFwkTag::WANTAGENT,
        "Global list overflow: %{public}zu > %{public}u, "
        "shrinking by %{public}zu", globalRecords_.size(), threshold_, removeCount);

    auto it = globalRecords_.begin();
    for (size_t i = 0; i < removeCount && it != globalRecords_.end(); ++i, ++it) {
        if (it->record != nullptr) {
            it->record->SetCanceled();
        }
    }
    globalRecords_.erase(globalRecords_.begin(), it);
}

void PendingWantGlobalManager::RemoveByCreatorPid(
    int32_t pid, const std::string &bundleName)
{
    std::lock_guard<ffrt::mutex> locker(mutex_);

    globalRecords_.remove_if(
        [pid, &bundleName](const GlobalPendingRecord &entry) {
            if (entry.record == nullptr) {
                return true;
            }
            return entry.record->GetCreatorPid() == pid &&
                entry.record->GetKey() != nullptr &&
                entry.record->GetKey()->GetBundleName() == bundleName;
        });
}

void PendingWantGlobalManager::RemoveByBundleName(
    const std::string &bundleName)
{
    std::lock_guard<ffrt::mutex> locker(mutex_);

    globalRecords_.remove_if(
        [&bundleName](const GlobalPendingRecord &entry) {
            if (entry.record == nullptr) {
                return true;
            }
            return entry.record->GetKey() != nullptr &&
                entry.record->GetKey()->GetBundleName() == bundleName;
        });
}

size_t PendingWantGlobalManager::GetSize()
{
    std::lock_guard<ffrt::mutex> locker(mutex_);
    return globalRecords_.size();
}

} // namespace AAFwk
} // namespace OHOS