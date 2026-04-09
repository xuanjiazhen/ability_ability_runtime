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

#ifndef OHOS_ABILITY_RUNTIME_ETS_INTEROP_ABILITY_MONITOR_H
#define OHOS_ABILITY_RUNTIME_ETS_INTEROP_ABILITY_MONITOR_H

#include "ani.h"
#include "ets_native_reference.h"
#include "iinterop_ability_monitor.h"

namespace OHOS {
namespace AbilityDelegatorEts {

class EtsInteropAbilityMonitor : public AppExecFwk::IInteropAbilityMonitor {
public:
    EtsInteropAbilityMonitor(ani_env *env, ani_object callback);
    ~EtsInteropAbilityMonitor() override;

    void OnAbilityStart(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability) override;
    void OnAbilityForeground(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability) override;
    void OnAbilityBackground(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability) override;
    void OnAbilityStop(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability) override;
    void OnWindowStageCreate(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability) override;
    void OnWindowStageRestore(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability) override;
    void OnWindowStageDestroy(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability) override;

private:
    ani_env *GetAniEnv();
    ani_ref GetAniRefFromProperty(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability);
    void CallLifecycleCBFunction(const char *methodName, ani_ref abilityRef);

private:
    ani_vm *vm_ = nullptr;
    ani_ref callbackRef_ = nullptr;
};

}  // namespace AbilityDelegatorEts
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ETS_INTEROP_ABILITY_MONITOR_H
