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

#ifndef OHOS_ABILITY_RUNTIME_STS_ABILITY_STAGE_MONITOR_H
#define OHOS_ABILITY_RUNTIME_STS_ABILITY_STAGE_MONITOR_H

#include <memory>
#include <string>
#include "native_engine/native_reference.h"
#include "sts_runtime.h"
#include "iability_stage_monitor.h"

namespace OHOS {
namespace AbilityDelegatorSts {
class STSAbilityStageMonitor : public AppExecFwk::IAbilityStageMonitor {
public:
    explicit STSAbilityStageMonitor(const std::string &moduleName, const std::string &srcEntrance);

    /**
     * Default deconstructor used to deconstruct.
     */
    ~STSAbilityStageMonitor() = default;

    /**
     * Sets the sts object.
     *
     * @param abilityStageMonitorObj Indicates the sts object.
     */
    void SetSTSAbilityStageMonitor(ani_env *env, ani_object &monitorObj);

    /**
     * Obtains the saved ets object.
     *
     * @return the saved ets object.
     */
    std::unique_ptr<AbilityRuntime::STSNativeReference> &GetStsAbilityStageMonitor()
    {
        return stsAbilityStageMonitor_;
    }

private:
    ani_env* GetAniEnv();

private:
    ani_vm* vm_ = nullptr;
    std::string moduleName_ = "";
    std::string srcEntrance_ = "";
    std::unique_ptr<AbilityRuntime::STSNativeReference> stsAbilityStageMonitor_ = nullptr;
};
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_STS_ABILITY_STAGE_MONITOR_H
