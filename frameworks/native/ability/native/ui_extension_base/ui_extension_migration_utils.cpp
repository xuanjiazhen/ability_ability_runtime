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

#include "ui_extension_migration_utils.h"

#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

bool UpdateWindowParentForMigration(
    const sptr<Rosen::Window> &uiWindow,
    const sptr<Rosen::WindowOption> &option)
{
    if (uiWindow == nullptr || option == nullptr) {
        return false;
    }
    return uiWindow->UpdateExtensionParent(option) == Rosen::WMError::WM_OK;
}

bool ShouldExecuteMigration(
    const sptr<AAFwk::SessionInfo> &sessionInfo,
    const MigrationFencingState &state)
{
    return sessionInfo != nullptr && sessionInfo->originalCallerInfo.migrationGen != 0 &&
        sessionInfo->originalCallerInfo.migrationGen > state.lastGen;
}

void ReportParentUpdateInvoked(const sptr<IRemoteObject> &abilityToken,
    const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    if (abilityToken == nullptr || sessionInfo == nullptr ||
        sessionInfo->originalCallerInfo.migrationGen == 0) {
        return;
    }
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->ReportUIExtensionParentUpdateInvoked(
        abilityToken, sessionInfo->persistentId, sessionInfo->originalCallerInfo.migrationGen);
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::UI_EXT, "migration report failed, persistentId: %{public}d, gen: %{public}u, ret: %{public}d",
            sessionInfo->persistentId, sessionInfo->originalCallerInfo.migrationGen, ret);
    }
}

bool ExecuteMigrationIfNeeded(
    const sptr<AAFwk::SessionInfo> &sessionInfo,
    const sptr<IRemoteObject> &abilityToken,
    const sptr<Rosen::Window> &uiWindow,
    const sptr<Rosen::WindowOption> &option,
    MigrationFencingState &fence,
    const std::function<void(const sptr<AAFwk::SessionInfo> &)> &contentSessionUpdater)
{
    if (sessionInfo == nullptr) {
        return true;
    }
    if (sessionInfo->originalCallerInfo.migrationGen != 0 && (uiWindow == nullptr || option == nullptr)) {
        return false;
    }
    if (!ShouldExecuteMigration(sessionInfo, fence)) {
        if (sessionInfo->originalCallerInfo.migrationGen != 0) {
            TAG_LOGI(AAFwkTag::UI_EXT, "migration skipped, persistentId: %{public}d, gen: %{public}u, "
                "lastGen: %{public}u, hostWindowId: %{public}u",
                sessionInfo->persistentId, sessionInfo->originalCallerInfo.migrationGen,
                fence.lastGen, sessionInfo->hostWindowId);
        }
        // A repeated delivery may follow a lost report. Older generations must not be reported.
        if (sessionInfo->originalCallerInfo.migrationGen != 0 &&
            sessionInfo->originalCallerInfo.migrationGen == fence.lastGen) {
            ReportParentUpdateInvoked(abilityToken, sessionInfo);
        }
        return true;
    }
    TAG_LOGI(AAFwkTag::UI_EXT, "host window migration, update parent, persistentId: %{public}d, "
        "gen: %{public}u, target parent: %{public}u", sessionInfo->persistentId,
        sessionInfo->originalCallerInfo.migrationGen, sessionInfo->hostWindowId);
    bool updated = UpdateWindowParentForMigration(uiWindow, option);
    // The ability layer records that it invoked the window API, independently of its result.
    fence.lastGen = sessionInfo->originalCallerInfo.migrationGen;
    ReportParentUpdateInvoked(abilityToken, sessionInfo);
    if (!updated) {
        TAG_LOGE(AAFwkTag::UI_EXT, "re-parent failed, keep session backgrounded");
        return false;
    }
    TAG_LOGI(AAFwkTag::UI_EXT, "re-parent ok, hostWindowId updated to: %{public}u, persistentId: "
        "%{public}d", sessionInfo->hostWindowId, sessionInfo->persistentId);
    if (contentSessionUpdater) {
        contentSessionUpdater(sessionInfo);
    }
    return true;
}

}
}
