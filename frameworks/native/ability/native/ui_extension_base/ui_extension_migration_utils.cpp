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
    MigrationFencingState &state)
{
    if (sessionInfo == nullptr ||
        sessionInfo->originalCallerInfo.migrationGen == 0) {
        return false;
    }
    uint32_t gen = sessionInfo->originalCallerInfo.migrationGen;
    if (gen <= state.lastGen) {
        return false;
    }
    if (sessionInfo->hostWindowId == state.currentParentWindowId) {
        state.lastGen = gen;
        return false;
    }
    return true;
}

void CommitMigrationState(MigrationFencingState &state, uint32_t gen, uint32_t hostWindowId)
{
    state.lastGen = gen;
    state.currentParentWindowId = hostWindowId;
}

bool ExecuteMigrationIfNeeded(
    const sptr<AAFwk::SessionInfo> &sessionInfo,
    const sptr<Rosen::Window> &uiWindow,
    const sptr<Rosen::WindowOption> &option,
    MigrationFencingState &fence,
    const std::function<void(const sptr<AAFwk::SessionInfo> &)> &contentSessionUpdater)
{
    if (sessionInfo == nullptr) {
        return true;
    }
    if (!ShouldExecuteMigration(sessionInfo, fence)) {
        if (sessionInfo->originalCallerInfo.migrationGen != 0) {
            TAG_LOGI(AAFwkTag::UI_EXT, "migration skipped, persistentId: %{public}d, gen: %{public}u, "
                "lastGen: %{public}u, hostWindowId: %{public}u, currentParent: %{public}u",
                sessionInfo->persistentId, sessionInfo->originalCallerInfo.migrationGen,
                fence.lastGen, sessionInfo->hostWindowId, fence.currentParentWindowId);
        }
        return true;
    }
    TAG_LOGI(AAFwkTag::UI_EXT, "host window migration, update parent, persistentId: %{public}d, "
        "gen: %{public}u, parent: %{public}u -> %{public}u", sessionInfo->persistentId,
        sessionInfo->originalCallerInfo.migrationGen, fence.currentParentWindowId,
        sessionInfo->hostWindowId);
    if (!UpdateWindowParentForMigration(uiWindow, option)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "re-parent failed, keep session backgrounded");
        sessionInfo->originalCallerInfo.migrationGen = 0;
        return false;
    }
    CommitMigrationState(fence, sessionInfo->originalCallerInfo.migrationGen,
        sessionInfo->hostWindowId);
    TAG_LOGI(AAFwkTag::UI_EXT, "re-parent ok, hostWindowId updated to: %{public}u, persistentId: "
        "%{public}d", sessionInfo->hostWindowId, sessionInfo->persistentId);
    if (contentSessionUpdater) {
        contentSessionUpdater(sessionInfo);
    }
    return true;
}

}
}
