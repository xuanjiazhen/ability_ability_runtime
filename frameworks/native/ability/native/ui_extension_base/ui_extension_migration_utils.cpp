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

namespace OHOS {
namespace AbilityRuntime {

bool IsValidParentWindowType(uint32_t parentWindowType)
{
    // WindowType enum range: [0, WINDOW_TYPE_UI_EXTENSION]; reject unknown raw values
    // from the wire instead of blind-casting them into Rosen::WindowType.
    return parentWindowType <= static_cast<uint32_t>(Rosen::WindowType::WINDOW_TYPE_UI_EXTENSION);
}

bool UpdateWindowParentForMigration(
    const sptr<AAFwk::SessionInfo> &sessionInfo,
    const sptr<Rosen::Window> &uiWindow)
{
    if (sessionInfo == nullptr || uiWindow == nullptr ||
        !IsValidParentWindowType(sessionInfo->parentWindowType)) {
        return false;
    }
    sptr<Rosen::WindowOption> option = sptr<Rosen::WindowOption>::MakeSptr();
    if (option == nullptr) {
        return false;
    }
    option->SetParentId(sessionInfo->hostWindowId);
    option->SetRealParentId(sessionInfo->realHostWindowId);
    option->SetParentWindowType(static_cast<Rosen::WindowType>(sessionInfo->parentWindowType));
    option->SetDisplayId(sessionInfo->displayId);
    option->SetCallerPersistentId(sessionInfo->callerPersistentId);
    return uiWindow->UpdateExtensionParent(option) == Rosen::WMError::WM_OK;
}

bool ShouldExecuteMigration(
    const sptr<AAFwk::SessionInfo> &sessionInfo,
    MigrationFencingState &state)
{
    if (sessionInfo == nullptr ||
        sessionInfo->originalCallerInfo.callerToken == nullptr) {
        return false;  // not a migration command
    }
    uint32_t gen = sessionInfo->originalCallerInfo.migrationGen;
    if (gen <= state.lastGen) {
        return false;  // stale command — skip re-parent to prevent out-of-order rollback
    }
    if (sessionInfo->hostWindowId == state.currentParentWindowId) {
        state.lastGen = gen;  // advance gen (this is current) but the parent is already correct
        return false;  // difference check: no-op, skip the window manager call
    }
    // Execution branch: the fence is NOT advanced here. CommitMigrationState advances
    // it after the re-parent succeeds, so a failure keeps this gen retryable.
    return true;
}

void CommitMigrationState(MigrationFencingState &state, uint32_t gen, uint32_t hostWindowId)
{
    state.lastGen = gen;
    state.currentParentWindowId = hostWindowId;
}

bool HasHostCallerChanged(const sptr<AAFwk::SessionInfo> &sessionInfo, MigrationFencingState &state)
{
    if (sessionInfo == nullptr) {
        return false;
    }
    if (state.lastCallerToken == sessionInfo->callerToken) {
        return false;  // same host ability (or both null): identity already known/notified
    }
    state.lastCallerToken = sessionInfo->callerToken;
    return true;
}

} // namespace AbilityRuntime
} // namespace OHOS
