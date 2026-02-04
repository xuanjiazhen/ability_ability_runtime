/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_SANDBOX_MANAGER_KIT_H
#define OHOS_ABILITY_RUNTIME_SANDBOX_MANAGER_KIT_H

#include <vector>
#include "policy_info.h"
#include "oh_mock_utils.h"

namespace OHOS {
namespace AccessControl {
namespace SandboxManager {
class SandboxManagerKit {

public:
    OH_MOCK_METHOD_WITH_DECORATOR_AND_OUTPUT_1(static, int32_t, results, SandboxManagerKit, CheckPersistPolicy,
        uint32_t, std::vector<PolicyInfo>&, std::vector<bool>& results);
};
}  // namespace SandboxManager
}  // namespace AccessControl
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_SANDBOX_MANAGER_KIT_H
