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

#ifndef OHOS_ABILITY_RUNTIME_FILE_PERMISSION_MANAGER_H
#define OHOS_ABILITY_RUNTIME_FILE_PERMISSION_MANAGER_H

#include <string>
#include <vector>
#include "uri.h"
#include "oh_mock_utils.h"
#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
#include "policy_info.h"
#else
#include "upms_policy_info.h"
#endif

namespace OHOS {
namespace AAFwk {
#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
using namespace AccessControl::SandboxManager;
#endif

class FilePermissionManager {
public:
    static OHOS::AccessControl::SandboxManager::PolicyInfo GetPathPolicyInfoFromUri(
        Uri& uri, uint32_t flag, const std::string& bundleName = "")
    {
        OH_MOCK_FUNCTION_BODY(OHOS::AccessControl::SandboxManager::PolicyInfo, FilePermissionManager,
            GetPathPolicyInfoFromUri, "Uri&, uint32_t, const std::string&")
    }
};

}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_FILE_PERMISSION_MANAGER_H
