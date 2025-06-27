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

#ifndef OHOS_ABILITY_RUNTIME_ETS_ABILITY_STAGE_OBJECT_H
#define OHOS_ABILITY_RUNTIME_ETS_ABILITY_STAGE_OBJECT_H

#include "ability_delegator_infos.h"
#include "configuration.h"

using FFIHandle = void *;
using StageHandle = void *;
using RuntimeHandle = void *;

namespace OHOS {
namespace AppExecFwk {
class OHOSApplication;
class Configuration;
}

namespace AbilityRuntime {
class Context;
class ETSAbilityStageObject {
public:
    explicit ETSAbilityStageObject() {}
    ~ETSAbilityStageObject();

    bool Init(StageHandle ability, RuntimeHandle runtime);
    bool LoadModule(const std::string &moduleName, const std::string &modulePath, const std::string &hapPath,
        bool esmodule, bool useCommonChunk, const std::string &srcEntrance);
    void OnCreate();
    void OnDestroy();
    void OnConfigurationUpdated(const std::shared_ptr<AppExecFwk::Configuration> &fullConfig,
        const AppExecFwk::Configuration &configuration);
    void SetEtsAbilityStage(const std::shared_ptr<Context> &context);

private:
    FFIHandle ffi_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_ABILITY_STAGE_OBJECT_H
