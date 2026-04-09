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
#include "ability_delegator_infos.h"

namespace OHOS {
namespace AppExecFwk {
class IInteropAbilityMonitor {
public:
    virtual ~IInteropAbilityMonitor() = default;

    virtual void OnAbilityStart(const std::weak_ptr<BaseDelegatorAbilityProperty> &ability) {}
    virtual void OnAbilityForeground(const std::weak_ptr<BaseDelegatorAbilityProperty> &ability) {}
    virtual void OnAbilityBackground(const std::weak_ptr<BaseDelegatorAbilityProperty> &ability) {}
    virtual void OnAbilityStop(const std::weak_ptr<BaseDelegatorAbilityProperty> &ability) {}
    virtual void OnWindowStageCreate(const std::weak_ptr<BaseDelegatorAbilityProperty> &ability) {}
    virtual void OnWindowStageRestore(const std::weak_ptr<BaseDelegatorAbilityProperty> &ability) {}
    virtual void OnWindowStageDestroy(const std::weak_ptr<BaseDelegatorAbilityProperty> &ability) {}
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_IINTEROP_ABILITY_MONITOR_H
