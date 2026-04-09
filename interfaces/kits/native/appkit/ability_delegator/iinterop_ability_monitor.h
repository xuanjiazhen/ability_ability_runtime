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

#ifndef OHOS_ABILITY_RUNTIME_IINTEROP_ABILITY_MONITOR_H
#define OHOS_ABILITY_RUNTIME_IINTEROP_ABILITY_MONITOR_H

#include <memory>
#include <string>
#include "ability_delegator_infos.h"

namespace OHOS {
namespace AppExecFwk {
class IInteropAbilityMonitor {
public:
    explicit IInteropAbilityMonitor(const std::string &abilityName);

    IInteropAbilityMonitor(const std::string &abilityName, const std::string &moduleName);

    virtual ~IInteropAbilityMonitor() = default;

    bool Match(const std::shared_ptr<BaseDelegatorAbilityProperty> &ability, bool isNotify = false);

    virtual void OnAbilityCreate(const std::weak_ptr<BaseDelegatorAbilityProperty> &ability) {}
    virtual void OnAbilityForeground(const std::weak_ptr<BaseDelegatorAbilityProperty> &ability) {}
    virtual void OnAbilityBackground(const std::weak_ptr<BaseDelegatorAbilityProperty> &ability) {}
    virtual void OnAbilityDestroy(const std::weak_ptr<BaseDelegatorAbilityProperty> &ability) {}
    virtual void OnWindowStageCreate(const std::weak_ptr<BaseDelegatorAbilityProperty> &ability) {}
    virtual void OnWindowStageRestore(const std::weak_ptr<BaseDelegatorAbilityProperty> &ability) {}
    virtual void OnWindowStageDestroy(const std::weak_ptr<BaseDelegatorAbilityProperty> &ability) {}

    std::string abilityName_;
    std::string moduleName_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_IINTEROP_ABILITY_MONITOR_H
