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
    uint32_t lastGen = 0;
    uint32_t currentParentWindowId = 0;
};

bool UpdateWindowParentForMigration(
    const sptr<AAFwk::SessionInfo> &sessionInfo,
    const sptr<Rosen::Window> &uiWindow);

bool ShouldExecuteMigration(
    const sptr<AAFwk::SessionInfo> &sessionInfo,
    MigrationFencingState &state);

void CommitMigrationState(
    MigrationFencingState &state,
    uint32_t gen,
    uint32_t hostWindowId);

bool ExecuteMigrationIfNeeded(
    const sptr<AAFwk::SessionInfo> &sessionInfo,
    const sptr<Rosen::Window> &uiWindow,
    MigrationFencingState &fence,
    const std::function<void(const sptr<AAFwk::SessionInfo> &)> &contentSessionUpdater);

}
}

#endif
