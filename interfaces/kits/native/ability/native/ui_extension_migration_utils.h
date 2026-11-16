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
// generation and the current parent window id.
struct MigrationFencingState {
    uint32_t lastGen = 0;
    uint32_t currentParentWindowId = 0;
};

// Re-parent a window to the new host during a UIExtension host window migration.
// Returns true when the window now lives under the new host.
bool UpdateWindowParentForMigration(
    const sptr<AAFwk::SessionInfo> &sessionInfo,
    const sptr<Rosen::Window> &uiWindow);

// Gen-fencing check: returns true only for a newer migration (gen > lastGen) whose
// hostWindowId actually differs; on true, lastGen/currentParentWindowId are advanced.
bool ShouldExecuteMigration(
    const sptr<AAFwk::SessionInfo> &sessionInfo,
    MigrationFencingState &state);

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_UI_EXTENSION_MIGRATION_UTILS_H
