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

// Per-instance state for migration gen fencing: the latest executed migration
// generation and the current parent window id. lastCallerToken is the host-caller
// (ability) identity latch for the onCallerChanged notification.
struct MigrationFencingState {
    uint32_t lastGen = 0;
    uint32_t currentParentWindowId = 0;
    sptr<IRemoteObject> lastCallerToken = nullptr;
};

// Re-parent a window to the new host during a UIExtension host window migration.
// Returns true when the window now lives under the new host.
bool UpdateWindowParentForMigration(
    const sptr<AAFwk::SessionInfo> &sessionInfo,
    const sptr<Rosen::Window> &uiWindow);

// Validate a wire-provided parentWindowType before it is cast into Rosen::WindowType:
// the enum's legal range is [0, WINDOW_TYPE_UI_EXTENSION].
bool IsValidParentWindowType(uint32_t parentWindowType);

// Gen-fencing check ONLY: returns true for a newer migration (gen > lastGen) whose
// hostWindowId actually differs. It does NOT advance the fence for the execution
// branch — the caller must call CommitMigrationState after the re-parent actually
// succeeds, so a failed re-parent leaves the fence open for at-least-once retry.
// The no-op branch (same hostWindowId, newer gen) advances lastGen itself since no
// side effect is pending.
bool ShouldExecuteMigration(
    const sptr<AAFwk::SessionInfo> &sessionInfo,
    MigrationFencingState &state);

// Commit the fence after the re-parent took effect: records the executed
// generation and the now-current parent window id. Must be called only on
// UpdateWindowParentForMigration success.
void CommitMigrationState(
    MigrationFencingState &state,
    uint32_t gen,
    uint32_t hostWindowId);

// Host-caller (ability) identity latch for the onCallerChanged notification:
// returns true only when the session's callerToken differs from the last
// known/notified identity, and consumes the change into the latch (a notified
// identity never notifies again). Call on re-parent success only, right after
// CommitMigrationState; seed the latch at window creation instead so the
// initial session identity never triggers a notification.
bool HasHostCallerChanged(
    const sptr<AAFwk::SessionInfo> &sessionInfo,
    MigrationFencingState &state);

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_UI_EXTENSION_MIGRATION_UTILS_H
