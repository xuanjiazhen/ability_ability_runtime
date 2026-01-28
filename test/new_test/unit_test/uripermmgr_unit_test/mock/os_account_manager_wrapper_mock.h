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

#ifndef OHOS_ABILITY_RUNTIME_OS_ACCOUNT_MANAGER_WRAPPER_MOCK_H
#define OHOS_ABILITY_RUNTIME_OS_ACCOUNT_MANAGER_WRAPPER_MOCK_H

#include <vector>
#include "os_account_manager_wrapper.h"
#include "oh_mock_utils.h"

namespace OHOS {
namespace AccountSandbox {
namespace OsAccountManagerWrapper {

OH_MOCK_GLOBAL_METHOD_WITH_DECORATOR(static, int32_t, QueryActiveOsAccountIds, std::vector<int32_t>&);

}  // namespace OsAccountManagerWrapper
}  // namespace AccountSandbox
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_OS_ACCOUNT_MANAGER_WRAPPER_MOCK_H
