/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "ability_delegator.h"
#ifdef CJ_FRONTEND
#include "cj_ability_delegator_impl.h"
#endif
#include "js_ability_delegator_impl.h"

namespace OHOS {
namespace AppExecFwk {
std::shared_ptr<AbilityDelegator> AbilityDelegator::Create(const std::unique_ptr<AbilityRuntime::Runtime>& runtime,
    const std::shared_ptr<AbilityRuntime::Context>& context, std::unique_ptr<TestRunner> runner,
    const sptr<IRemoteObject>& observer)
{
    if (!runtime) {
        return std::make_shared<AbilityDelegator>();
    }

    switch (runtime->GetLanguage()) {
        case AbilityRuntime::Runtime::Language::JS:
            return JSAbilityDelegatorImpl::Create(context, std::move(runner), observer);
#ifdef CJ_FRONTEND
        case AbilityRuntime::Runtime::Language::CJ:
            return CJAbilityDelegatorImpl::Create(context, std::move(runner), observer);
#endif
        default:
            return std::make_shared<AbilityDelegator>();
    }
}

void AbilityDelegator::ClearAllMonitors() {}

size_t AbilityDelegator::GetMonitorsNum()
{
    return 0;
}

size_t AbilityDelegator::GetStageMonitorsNum()
{
    return 0;
}

std::string AbilityDelegator::GetThreadName() const
{
    return "";
}

void AbilityDelegator::Prepare() {}

void AbilityDelegator::OnRun() {}

} // namespace AppExecFwk
} // namespace OHOS