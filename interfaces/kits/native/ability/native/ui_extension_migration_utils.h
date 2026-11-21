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

#include <functional>

#include "session_info.h"
#include "window.h"
#include "window_option.h"

namespace OHOS {
namespace AbilityRuntime {

struct MigrationFencingState {
    // Latest generation for which this UEA invoked the parent-window update interface.
    uint32_t lastGen = 0;
};

bool UpdateWindowParentForMigration(
    const sptr<Rosen::Window> &uiWindow,
    const sptr<Rosen::WindowOption> &option);

bool ShouldExecuteMigration(
    const sptr<AAFwk::SessionInfo> &sessionInfo,
    const MigrationFencingState &state);

/** Report that UEA invoked UpdateExtensionParent. Repeating the report is safe. */
void ReportParentUpdateInvoked(const sptr<IRemoteObject> &abilityToken,
    const sptr<AAFwk::SessionInfo> &sessionInfo);

bool ExecuteMigrationIfNeeded(
    const sptr<AAFwk::SessionInfo> &sessionInfo,
    const sptr<IRemoteObject> &abilityToken,
    const sptr<Rosen::Window> &uiWindow,
    const sptr<Rosen::WindowOption> &option,
    MigrationFencingState &fence,
    const std::function<void(const sptr<AAFwk::SessionInfo> &)> &contentSessionUpdater);

}
}

#endif
