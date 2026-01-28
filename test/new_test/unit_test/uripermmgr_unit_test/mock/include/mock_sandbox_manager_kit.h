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

#ifndef MOCK_SANDBOX_MANAGER_KIT_H
#define MOCK_SANDBOX_MANAGER_KIT_H

#include "oh_mock_utils.h"
#include <string>
#include <vector>

#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER

namespace OHOS {
namespace AccessControl {

struct PolicyInfo {
    std::string path;
    uint32_t mode;
};

struct SetInfo {
    uint64_t tokenId;
    std::vector<PolicyInfo> policyVec;
};

class MockSandboxManagerKit {
public:
    OH_MOCK_METHOD(int32_t, MockSandboxManagerKit, CheckPolicy, uint32_t tokenId,
                   const std::vector<PolicyInfo>& policyVec, std::vector<bool>& result);

    OH_MOCK_METHOD(int32_t, MockSandboxManagerKit, SetPolicy, uint32_t tokenId,
                   const std::vector<PolicyInfo>& policyVec);

    OH_MOCK_METHOD(int32_t, MockSandboxManagerKit, UnSetPolicy, uint32_t tokenId,
                   const std::vector<PolicyInfo>& policyVec);

    OH_MOCK_METHOD(int32_t, MockSandboxManagerKit, CheckPermission, uint32_t tokenId, const std::string& permissionName,
                   bool& result);

    OH_MOCK_METHOD(int32_t, MockSandboxManagerKit, SetPermission, uint32_t tokenId, const std::string& permissionName);
};

} // namespace AccessControl
} // namespace OHOS

#endif // ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER

#endif // MOCK_SANDBOX_MANAGER_KIT_H
