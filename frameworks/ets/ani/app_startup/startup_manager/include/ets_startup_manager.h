/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef OHOS_ABILITY_RUNTIME_ETS_STARTUP_MANAGER_H
#define OHOS_ABILITY_RUNTIME_ETS_STARTUP_MANAGER_H

#include "ani.h"
#include "startup_manager.h"

namespace OHOS {
namespace AbilityRuntime {
class EtsStartupManager {
public:
    EtsStartupManager() = default;

    ~EtsStartupManager() = default;

    static ani_object OnRun(ani_env *env, ani_object startupTasks,
        ani_object config, ani_object call);

    static void OnRemoveAllResult(ani_env *env);

    static ani_object OnGetResult(ani_env *env, ani_string startupTask);

    static ani_boolean OnIsInitialized(ani_env *env, ani_string startupTask);

    static void OnRemoveResult(ani_env *env, ani_string startupTask);

private:

    static int32_t GetConfig(ani_env *env, ani_object value, std::shared_ptr<StartupConfig> &config);

    static int32_t RunStartupTask(ani_env *env, ani_object startupTasks, ani_object config,
         std::shared_ptr<StartupTaskManager> &startupTaskManager);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_STARTUP_MANAGER_H
