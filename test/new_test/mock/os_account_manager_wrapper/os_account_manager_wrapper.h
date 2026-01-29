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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_OS_ACCOUNT_MANAGER_WRAPPER_H
#define MOCK_OHOS_ABILITY_RUNTIME_OS_ACCOUNT_MANAGER_WRAPPER_H

#include <vector>
#include "errors.h"
#include "oh_mock_utils.h"
#include "mock_singleton.h"

namespace OHOS {
namespace AppExecFwk {

// Forward declaration
class OsAccountManagerWrapper;

}  // namespace AppExecFwk

// Specialization for OsAccountManagerWrapper
template<>
class DelayedSingleton<AppExecFwk::OsAccountManagerWrapper> {
public:
    static AppExecFwk::OsAccountManagerWrapper* GetInstance() {
        return AppExecFwk::OsAccountManagerWrapper::DelayedSingleton_OsAccountManagerWrapper_GetInstance();
    }
};

namespace AppExecFwk {

class OsAccountManagerWrapper {
public:
    OH_MOCK_METHOD(int32_t, OsAccountManagerWrapper, QueryActiveOsAccountIds, std::vector<int32_t>&);

    OH_MOCK_METHOD_WITH_DECORATOR(static, OsAccountManagerWrapper*, OsAccountManagerWrapper,
        DelayedSingleton_OsAccountManagerWrapper_GetInstance);
};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MOCK_OHOS_ABILITY_RUNTIME_OS_ACCOUNT_MANAGER_WRAPPER_H