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

#ifndef OHOS_ABILITY_RUNTIME_MIGRATION_MARKER_GUARD_H
#define OHOS_ABILITY_RUNTIME_MIGRATION_MARKER_GUARD_H

#include <mutex>

namespace OHOS {
namespace AAFwk {

// Serializes read-modify-write access to the unconfirmed-migration marker
// (SessionInfo::originalCallerInfo) across its three mutation domains:
//   - UIExtensionAbilityManager::StartAbilityLocked (at-least-once inherit + gen rollback)
//   - UIExtensionRecord::Update (same, preload/reuse path)
//   - AbilityConnectManager::ScheduleCommandAbilityWindowDone (confirm-clear)
//   - ExtensionRecordManager::GetMigrationUpdateRecord (credential read + gen stamp)
// Constructing the guard in the scope of a marker access block is enough; do NOT hold
// it across IPC calls.
class MigrationMarkerGuard {
public:
    MigrationMarkerGuard() : lock_(Mutex()) {}
    ~MigrationMarkerGuard() = default;
    MigrationMarkerGuard(const MigrationMarkerGuard &) = delete;
    MigrationMarkerGuard &operator=(const MigrationMarkerGuard &) = delete;

private:
    // Function-local static in an inline member function: one instance across TUs.
    static std::mutex &Mutex()
    {
        static std::mutex mutex;
        return mutex;
    }

    std::lock_guard<std::mutex> lock_;
};

} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_MIGRATION_MARKER_GUARD_H
