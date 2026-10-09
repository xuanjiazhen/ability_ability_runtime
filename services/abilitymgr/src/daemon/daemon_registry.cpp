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

#include "daemon/daemon_registry.h"

#include <chrono>
#include <mutex>

#include "errors.h"

namespace OHOS {
namespace AAFwk {
DaemonRegistry &DaemonRegistry::GetInstance()
{
    static DaemonRegistry instance;
    return instance;
}

bool DaemonRegistry::IsRegistered(const std::string &bundleName, const std::string &daemonName) const
{
    std::shared_lock lock(mutex_);
    return records_.find(MakeKey(bundleName, daemonName)) != records_.end();
}

bool DaemonRegistry::IsEnabled(const std::string &bundleName, const std::string &daemonName) const
{
    std::shared_lock lock(mutex_);
    auto iter = records_.find(MakeKey(bundleName, daemonName));
    return iter != records_.end() && iter->second.status == DaemonStatus::ENABLED;
}

int32_t DaemonRegistry::GetStatus(
    const std::string &bundleName, const std::string &daemonName, DaemonRecord &record) const
{
    std::shared_lock lock(mutex_);
    auto iter = records_.find(MakeKey(bundleName, daemonName));
    if (iter == records_.end()) {
        return ERR_DAEMON_NOT_REGISTERED;
    }
    record = iter->second;
    return ERR_OK;
}

int32_t DaemonRegistry::UpdateEnabledStatus(const std::string &bundleName, const std::string &daemonName,
    bool enabled, bool &changed)
{
    changed = false;
    std::unique_lock lock(mutex_);
    auto iter = records_.find(MakeKey(bundleName, daemonName));
    if (iter == records_.end()) {
        return ERR_DAEMON_NOT_REGISTERED;
    }

    const auto targetStatus = enabled ? DaemonStatus::ENABLED : DaemonStatus::DISABLED;
    if (iter->second.status == targetStatus) {
        return ERR_OK;
    }

    auto updatedRecord = iter->second;
    updatedRecord.status = targetStatus;
    updatedRecord.enabledChangedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    if (persistHandler_ && !persistHandler_(updatedRecord)) {
        return ERR_DAEMON_STORAGE_FAILED;
    }
    iter->second = std::move(updatedRecord);
    changed = true;
    return ERR_OK;
}

void DaemonRegistry::UpsertRecord(const DaemonRecord &record)
{
    std::unique_lock lock(mutex_);
    records_.insert_or_assign(MakeKey(record.bundleName, record.daemonName), record);
}

void DaemonRegistry::RemoveRecord(const std::string &bundleName, const std::string &daemonName)
{
    std::unique_lock lock(mutex_);
    records_.erase(MakeKey(bundleName, daemonName));
}

void DaemonRegistry::SetPersistHandler(PersistHandler handler)
{
    std::unique_lock lock(mutex_);
    persistHandler_ = std::move(handler);
}

std::string DaemonRegistry::MakeKey(const std::string &bundleName, const std::string &daemonName)
{
    return bundleName + "#" + daemonName;
}
} // namespace AAFwk
} // namespace OHOS
