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

int32_t DaemonRegistry::Register(const DaemonRecord &record)
{
    std::unique_lock lock(mutex_);
    auto key = MakeKey(record.bundleName, record.daemonName);
    if (records_.find(key) != records_.end()) {
        return ERR_DAEMON_ALREADY_REGISTERED;
    }
    if (persistHandler_ && !persistHandler_(record)) {
        return ERR_DAEMON_STORAGE_FAILED;
    }
    records_.insert_or_assign(key, record);
    return ERR_OK;
}

int32_t DaemonRegistry::Unregister(const std::string &bundleName, const std::string &daemonName, int32_t &pid)
{
    pid = 0;
    std::unique_lock lock(mutex_);
    auto key = MakeKey(bundleName, daemonName);
    auto iter = records_.find(key);
    if (iter == records_.end()) {
        return ERR_DAEMON_NOT_REGISTERED;
    }
    pid = iter->second.pid;
    records_.erase(iter);
    return ERR_OK;
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
