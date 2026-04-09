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

#include "hilog_tag_wrapper.h"
#include "iinterop_ability_monitor.h"

namespace OHOS {
namespace AppExecFwk {
IInteropAbilityMonitor::IInteropAbilityMonitor(const std::string &abilityName) : abilityName_(abilityName)
{}

IInteropAbilityMonitor::IInteropAbilityMonitor(const std::string &abilityName, const std::string &moduleName)
    : abilityName_(abilityName), moduleName_(moduleName)
{}

bool IInteropAbilityMonitor::Match(const std::shared_ptr<BaseDelegatorAbilityProperty> &ability, bool isNotify)
{
    if (!ability) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid ability property");
        return false;
    }

    const auto &aName = ability->name_;

    if (abilityName_.empty() || aName.empty()) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid name");
        return false;
    }

    if (abilityName_.compare(aName)) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "different name");
        return false;
    }

    const auto &aModuleName = ability->moduleName_;

    if (!moduleName_.empty() && moduleName_.compare(aModuleName) != 0) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "different moduleName, %{public}s and %{public}s",
            moduleName_.c_str(), aModuleName.c_str());
        return false;
    }

    TAG_LOGI(AAFwkTag::DELEGATOR, "ability name : %{public}s, isNotify : %{public}s",
        abilityName_.data(), (isNotify ? "true" : "false"));

    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
