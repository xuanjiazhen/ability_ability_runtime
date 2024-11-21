/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_DELEGATOR_H
#define OHOS_ABILITY_RUNTIME_ABILITY_DELEGATOR_H

#include "ability_delegator_registry.h"
#include "ability_manager_client.h"
#include "context.h"
#include "hilog_tag_wrapper.h"
#include "itest_observer.h"
#include "ohos_application.h"
#include "runtime.h"
#include "test_runner.h"

namespace OHOS {
namespace AppExecFwk {

class AbilityDelegator {
public:
    static std::shared_ptr<AbilityDelegator> Create(const std::unique_ptr<AbilityRuntime::Runtime>& runtime,
        const std::shared_ptr<AbilityRuntime::Context>& context, std::unique_ptr<TestRunner>& runner,
        const sptr<IRemoteObject>& observer);
    AbilityDelegator() = default;
    virtual ~AbilityDelegator() = default;
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_ABILITY_DELEGATOR_H