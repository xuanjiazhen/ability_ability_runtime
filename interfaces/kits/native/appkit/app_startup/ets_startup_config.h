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

#ifndef OHOS_ABILITY_RUNTIME_ETS_STARTUP_CONFIG_H
#define OHOS_ABILITY_RUNTIME_ETS_STARTUP_CONFIG_H

#include <memory>

#include "ets_runtime.h"
#include "startup_config.h"
#include "startup_utils.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
class JsStartupConfig : public StartupConfig {
public:
    JsStartupConfig(ani_env env);

    ~JsStartupConfig() override;

    int32_t Init(ani_ref &configEntry, std::shared_ptr<AAFwk::Want> want);

    int32_t Init(ani_object config);

    static ani_object BuildResult(ani_env env, const std::shared_ptr<StartupTaskResult> &result);

private:
    ani_env env_;

    void InitAwaitTimeout(ani_env env, ani_object config);
    void InitListener(ani_env env, ani_object config);
    void InitCustomization(ani_env env, ani_object configEntry, std::shared_ptr<AAFwk::Want> want);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_STARTUP_CONFIG_H
