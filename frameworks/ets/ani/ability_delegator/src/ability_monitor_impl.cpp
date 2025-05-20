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

#include "ability_monitor_impl.h"

#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "sts_ability_delegator_utils.h"
#include "napi/native_common.h"

namespace OHOS {
namespace AbilityDelegatorSts {
using namespace OHOS::AbilityRuntime;
AbilityMonitorImpl::AbilityMonitorImpl(const std::string &name,
    const std::shared_ptr<STSAbilityMonitor> &stsAbilityMonitor)
    : IAbilityMonitor(name), stsMonitor_(stsAbilityMonitor)
{}

AbilityMonitorImpl::AbilityMonitorImpl(const std::string &name, const std::string &moduleName,
    const std::shared_ptr<STSAbilityMonitor> &stsAbilityMonitor)
    : IAbilityMonitor(name, moduleName), stsMonitor_(stsAbilityMonitor)
{}

void AbilityMonitorImpl::OnSTSAbilityStart(const std::weak_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    if (stsMonitor_ == nullptr) {
        return;
    }

    stsMonitor_->OnSTSAbilityCreate(abilityObj);
}

void AbilityMonitorImpl::OnSTSAbilityForeground(const std::weak_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    if (stsMonitor_ == nullptr) {
        return;
    }

    stsMonitor_->OnSTSAbilityForeground(abilityObj);
}

void AbilityMonitorImpl::OnSTSAbilityBackground(const std::weak_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    if (stsMonitor_ == nullptr) {
        return;
    }

    stsMonitor_->OnSTSAbilityBackground(abilityObj);
}

void AbilityMonitorImpl::OnSTSAbilityStop(const std::weak_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    if (stsMonitor_ == nullptr) {
        return;
    }

    stsMonitor_->OnSTSAbilityStop(abilityObj);
}

void AbilityMonitorImpl::OnSTSWindowStageCreate(const std::weak_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    if (stsMonitor_ == nullptr) {
        return;
    }

    stsMonitor_->OnSTSWindowStageCreate(abilityObj);
}

void AbilityMonitorImpl::OnSTSWindowStageRestore(const std::weak_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    if (stsMonitor_ == nullptr) {
        return;
    }

    stsMonitor_->OnSTSWindowStageRestore(abilityObj);
}

void AbilityMonitorImpl::OnSTSWindowStageDestroy(const std::weak_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    if (stsMonitor_ == nullptr) {
        return;
    }

    stsMonitor_->OnSTSWindowStageDestroy(abilityObj);
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
