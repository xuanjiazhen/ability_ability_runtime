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

#ifndef OHOS_ABILITY_RUNTIME_DAEMON_REGISTRY_H
#define OHOS_ABILITY_RUNTIME_DAEMON_REGISTRY_H

#include <functional>
#include <shared_mutex>
#include <string>
#include <unordered_map>

#include "daemon_service_errors.h"

namespace OHOS {
namespace AAFwk {
struct DaemonRecord {
    std::string bundleName;
    std::string daemonName;
    DaemonStatus status = DaemonStatus::ENABLED;
    int32_t pid = 0;
    int64_t enabledChangedTime = 0;
};

class DaemonRegistry final {
public:
    using PersistHandler = std::function<bool(const DaemonRecord &)>;

    static DaemonRegistry &GetInstance();

    bool IsRegistered(const std::string &bundleName, const std::string &daemonName) const;
    int32_t GetStatus(const std::string &bundleName, const std::string &daemonName, DaemonRecord &record) const;

    // FEAT-DAEMON-01 supplies records and persistence when its registration path is integrated.
    void UpsertRecord(const DaemonRecord &record);
    void RemoveRecord(const std::string &bundleName, const std::string &daemonName);
    void SetPersistHandler(PersistHandler handler);

    // FEAT-DAEMON-01 register/unregister.
    int32_t Register(const DaemonRecord &record);
    int32_t Unregister(const std::string &bundleName, const std::string &daemonName, int32_t &pid);

    // FEAT-DAEMON-03 start/stop. Tracks the daemon process pid reported by appspawn.
    void SetPid(const std::string &bundleName, const std::string &daemonName, int32_t pid);

private:
    DaemonRegistry() = default;
    ~DaemonRegistry() = default;
    DaemonRegistry(const DaemonRegistry &) = delete;
    DaemonRegistry &operator=(const DaemonRegistry &) = delete;

    static std::string MakeKey(const std::string &bundleName, const std::string &daemonName);

    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, DaemonRecord> records_;
    PersistHandler persistHandler_;
};
} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_DAEMON_REGISTRY_H
