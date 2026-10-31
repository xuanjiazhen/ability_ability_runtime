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

#include "backup_notification_manager.h"

#include "app_exit_reason_data_manager.h"
#include "hilog_tag_wrapper.h"
#include "session_info.h"
#include "session_manager_lite.h"
#include "ui_ability_lifecycle_manager.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* TAG = "BackupNotificationManager";
}

BackupNotificationManager::BackupNotificationManager() {}

BackupNotificationManager::~BackupNotificationManager() {}

void BackupNotificationManager::NotifyWindowSnapshotBeforeKill(int32_t pid, Reason reason,
    std::shared_ptr<UIAbilityLifecycleManager> uiAbilityManager)
{
    if (reason != Reason::REASON_RESOURCE_CONTROL && reason != Reason::REASON_PERFORMANCE_CONTROL) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "reason %{public}d is not resource/performance control, skip", reason);
        return;
    }
    if (uiAbilityManager == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "uiAbilityManager is nullptr");
        return;
    }
    auto records = uiAbilityManager->GetUIAbilityRecordsByPid(pid);
    if (records.empty()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "no UIAbilityRecord found for pid %{public}d", pid);
        return;
    }
    for (const auto &record : records) {
        if (record == nullptr) {
            continue;
        }
        auto sessionInfo = record->GetSessionInfo();
        if (sessionInfo == nullptr || sessionInfo->persistentId <= 0) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "invalid sessionInfo or persistentId");
            continue;
        }
        if (!HasRecoverInfo(record)) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "no recover info for persistentId %{public}d", sessionInfo->persistentId);
            continue;
        }
        int32_t persistentId = sessionInfo->persistentId;
        NotifyWindowUseSnapshot(persistentId);
        {
            std::unique_lock<std::shared_mutex> lock(mutex_);
            notifiedPersistentIds_.insert(persistentId);
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "notified window for persistentId %{public}d", persistentId);
    }
}

void BackupNotificationManager::NotifyWindowSnapshotDeleted(int32_t persistentId)
{
    bool needNotify = false;
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        needNotify = notifiedPersistentIds_.find(persistentId) != notifiedPersistentIds_.end();
    }
    if (!needNotify) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "persistentId %{public}d not in notified set, skip deletion notify",
            persistentId);
        return;
    }
    NotifyWindowSnapshotDeletedInternal(persistentId);
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        notifiedPersistentIds_.erase(persistentId);
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "notified window deletion for persistentId %{public}d", persistentId);
}

bool BackupNotificationManager::HasRecoverInfo(const UIAbilityRecordPtr &record)
{
    if (record == nullptr) {
        return false;
    }
    const auto &abilityInfo = record->GetAbilityInfo();
    auto sessionInfo = record->GetSessionInfo();
    if (sessionInfo == nullptr) {
        return false;
    }
    std::string abilityName = abilityInfo.name;
    if (abilityInfo.launchMode == AppExecFwk::LaunchMode::STANDARD) {
        abilityName += std::to_string(sessionInfo->persistentId);
    }
    bool hasRecoverInfo = false;
    auto ret = DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->GetAbilityRecoverInfo(
        abilityInfo.applicationInfo.accessTokenId, abilityInfo.moduleName, abilityName, hasRecoverInfo);
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "GetAbilityRecoverInfo failed ret %{public}d", ret);
        return false;
    }
    return hasRecoverInfo;
}

void BackupNotificationManager::NotifyWindowUseSnapshot(int32_t persistentId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "NotifyWindowUseSnapshot persistentId %{public}d", persistentId);
    auto sceneSessionManager = Rosen::SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
    if (sceneSessionManager == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "sceneSessionManager is nullptr");
        return;
    }
    // TODO: Replace with actual sceneSessionManager call when window interface is ready
    // auto err = sceneSessionManager->NotifySnapshotAvailable(persistentId);
}

void BackupNotificationManager::NotifyWindowSnapshotDeletedInternal(int32_t persistentId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "NotifyWindowSnapshotDeletedInternal persistentId %{public}d", persistentId);
    auto sceneSessionManager = Rosen::SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
    if (sceneSessionManager == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "sceneSessionManager is nullptr");
        return;
    }
    // TODO: Replace with actual sceneSessionManager call when window interface is ready
    // auto err = sceneSessionManager->NotifySnapshotDeleted(persistentId);
}
} // namespace AAFwk
} // namespace OHOS
