/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ETS_AUTO_FILL_MANAGER_H
#define OHOS_ABILITY_RUNTIME_ETS_AUTO_FILL_MANAGER_H

#include <map>
#include "ets_startup_manager.h"
#include "sts_runtime.h"
#include "startup_config.h"
#include "startup_manager.h"
#include "native_engine/native_value.h"

namespace OHOS {
namespace AbilityRuntime {

class EtsStartupManager {

  std::shared_ptr<StartupConfig> config_;

  int32_t GetConfig(ani_env *env, ani_int value, std::shared_ptr<StartupConfig> &config);
  
  void OnConfig(std::shared_ptr<StartupConfig> &config);
};

} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_AUTO_FILL_MANAGER_H