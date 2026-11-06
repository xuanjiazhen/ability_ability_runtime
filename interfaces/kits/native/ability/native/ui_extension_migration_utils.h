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

#ifndef OHOS_ABILITY_RUNTIME_UI_EXTENSION_MIGRATION_UTILS_H
#define OHOS_ABILITY_RUNTIME_UI_EXTENSION_MIGRATION_UTILS_H

#include "session_info.h"
#include "window.h"
#include "window_option.h"

namespace OHOS {
namespace AbilityRuntime {

/**
 * @brief Per-instance state for migration gen fencing.
 *
 * Each UIExtension variant holds one instance of this struct and passes it to
 * ShouldExecuteMigration(). The struct tracks the latest migration generation
 * whose re-parent has been executed and the current parent window id.
 */
struct MigrationFencingState {
    uint32_t lastGen = 0;
    uint32_t currentParentWindowId = 0;
};

/**
 * @brief Re-parent a window to the new host during a UEC host window migration.
 *
 * Pure C++ shared helper used by every UIExtension variant (JS/ETS/CJ/autoFill).
 * Builds a WindowOption from the migrated session info and calls the wm
 * UpdateExtensionParent C++ interface — no napi involved.
 *
 * @param sessionInfo The migrated session info carrying the new host window id.
 * @param uiWindow The window to re-parent (from the variant's own uiWindowMap_).
 * @return true when the window now lives under the new host.
 */
inline bool UpdateWindowParentForMigration(
    const sptr<AAFwk::SessionInfo> &sessionInfo,
    const sptr<Rosen::Window> &uiWindow)
{
    if (sessionInfo == nullptr || uiWindow == nullptr) {
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

/**
 * @brief Migration gen-fencing check: should this migration command trigger a re-parent?
 *
 * Monotonic filter + difference detection. Returns true only when:
 *   1. gen > lastGen (this is a newer migration, not a stale out-of-order command), AND
 *   2. hostWindowId != currentParent (the parent actually needs to change).
 * When it returns true, the caller should also update lastGen and currentParent.
 *
 * @param sessionInfo The incoming FG command's session info (with declaration + gen).
 * @param lastGen [in/out] The last executed migration generation (updated on pass).
 * @param currentParent [in/out] The current parent window id (updated on pass).
 * @return true = execute the re-parent; false = skip (stale or no-op).
 */
inline bool ShouldExecuteMigration(
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
        return false;  // difference check: no-op, skip the wm call
    }
    state.lastGen = gen;
    state.currentParentWindowId = sessionInfo->hostWindowId;
    return true;
}

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_UI_EXTENSION_MIGRATION_UTILS_H
