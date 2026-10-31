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

#ifndef OHOS_ABILITY_RUNTIME_BACKUP_NOTIFICATION_MANAGER_H
#define OHOS_ABILITY_RUNTIME_BACKUP_NOTIFICATION_MANAGER_H

#include <memory>
#include <unordered_set>
#include <shared_mutex>
#include <vector>
#include "ability_state.h"
#include "ui_ability_record.h"

namespace OHOS {
namespace AAFwk {
class UIAbilityLifecycleManager;

class BackupNotificationManager {
    DECLARE_DELAYED_SINGLETON(BackupNotificationManager)
public:
    void NotifyWindowSnapshotBeforeKill(int32_t pid, Reason reason,
        std::shared_ptr<UIAbilityLifecycleManager> uiAbilityManager);
    void NotifyWindowSnapshotDeleted(int32_t persistentId);

private:
    bool HasRecoverInfo(const UIAbilityRecordPtr &record);
    void NotifyWindowUseSnapshot(int32_t persistentId);
    void NotifyWindowSnapshotDeletedInternal(int32_t persistentId);

    std::shared_mutex mutex_;
    std::unordered_set<int32_t> notifiedPersistentIds_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_BACKUP_NOTIFICATION_MANAGER_H
