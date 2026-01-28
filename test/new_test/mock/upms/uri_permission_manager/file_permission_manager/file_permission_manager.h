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
#include "policy_info.h"
#include "oh_mock_utils.h"

namespace OHOS {
namespace AAFwk {

class FilePermissionManager {
public:
    OH_MOCK_METHOD_WITH_DECORATOR(static, OHOS::AccessControl::SandboxManager::PolicyInfo, FilePermissionManager, GetPathPolicyInfoFromUri,
        Uri&, uint32_t, const std::string&);
};

}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_FILE_PERMISSION_MANAGER_H
