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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_MONITOR_IMPL_H
#define OHOS_ABILITY_RUNTIME_ABILITY_MONITOR_IMPL_H

#include <memory>
#include "iability_monitor.h"
#include "sts_ability_monitor.h"
#include "sts_runtime.h"

namespace OHOS {
namespace AbilityDelegatorSts {
using namespace OHOS::AppExecFwk;
class AbilityMonitorImpl : public IAbilityMonitor {
public:
    /**
     * A constructor used to create a AbilityMonitor instance with the input parameter passed.
     *
     * @param name Indicates the specified ability name.
     * @param stsAbilityMonitor Indicates the STSAbilityMonitor object.
     */
    AbilityMonitorImpl(const std::string &name, const std::shared_ptr<STSAbilityMonitor> &stsAbilityMonitor);

    /**
     * A constructor used to create a AbilityMonitor instance with the input parameter passed.
     *
     * @param name Indicates the specified ability name.
     * @param moduleName Indicates the specified ability moduleName.
     * @param stsAbilityMonitor Indicates the STSAbilityMonitor object.
     */
    AbilityMonitorImpl(const std::string &name, const std::string &moduleName,
        const std::shared_ptr<STSAbilityMonitor> &stsAbilityMonitor);

    /**
     * Default deconstructor used to deconstruct.
     */
    ~AbilityMonitorImpl() = default;

    /**
     * Called when ability is started.
     *
     * @param abilityObj Indicates the ability object.
     */
    void OnSTSAbilityStart(const std::weak_ptr<AbilityRuntime::STSNativeReference> &abilityObj) override;

    /**
     * Called when ability is in foreground.
     *
     * @param abilityObj Indicates the ability object.
     */
    void OnSTSAbilityForeground(const std::weak_ptr<AbilityRuntime::STSNativeReference> &abilityObj) override;

    /**
     * Called when ability is in background.
     *
     * @param abilityObj Indicates the ability object.
     */
    void OnSTSAbilityBackground(const std::weak_ptr<AbilityRuntime::STSNativeReference> &abilityObj) override;

    /**
     * Called when ability is stopped.
     *
     * @param abilityObj Indicates the ability object.
     */
    void OnSTSAbilityStop(const std::weak_ptr<AbilityRuntime::STSNativeReference> &abilityObj) override;

    /**
     * Called when window stage is created.
     *
     * @param abilityObj Indicates the ability object.
     */
    void OnSTSWindowStageCreate(const std::weak_ptr<AbilityRuntime::STSNativeReference> &abilityObj) override;

    /**
     * Called when window stage is restored.
     *
     * @param abilityObj Indicates the ability object.
     */
    void OnSTSWindowStageRestore(const std::weak_ptr<AbilityRuntime::STSNativeReference> &abilityObj) override;

    /**
     * Called when window stage is destroyed.
     *
     * @param abilityObj Indicates the ability object.
     */
    void OnSTSWindowStageDestroy(const std::weak_ptr<AbilityRuntime::STSNativeReference> &abilityObj) override;

private:
    std::shared_ptr<STSAbilityMonitor> stsMonitor_;
};
}  // namespace AbilityDelegatorSts
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_MONITOR_IMPL_H
