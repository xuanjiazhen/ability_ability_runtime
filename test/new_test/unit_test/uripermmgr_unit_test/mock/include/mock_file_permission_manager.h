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

#ifndef MOCK_FILE_PERMISSION_MANAGER_H
#define MOCK_FILE_PERMISSION_MANAGER_H

#include "oh_mock_utils.h"
#include <string>
#include <vector>

namespace OHOS {
namespace AccessControl {

struct PolicyInfo {
    std::string path;
    uint32_t mode;
};

} // namespace AccessControl

namespace AAFwk {

class MockFilePermissionManager {
public:
    OH_MOCK_METHOD(std::vector<bool>, MockFilePermissionManager, CheckUriPersistentPermission,
                   const std::vector<std::string>& uriStrs, uint32_t flag, uint32_t tokenId);

    OH_MOCK_METHOD(std::vector<bool>, MockFilePermissionManager, CheckDocsUriPermission,
                   const std::vector<std::string>& uriStrs, uint32_t flag, uint32_t tokenId);

    OH_MOCK_METHOD(AccessControl::PolicyInfo, MockFilePermissionManager, GetPathPolicyInfoFromUri, const std::string& uriStr,
                   uint32_t flag);

    OH_MOCK_METHOD(bool, MockFilePermissionManager, LoadDllLibrary);

    OH_MOCK_METHOD(void, MockFilePermissionManager, UnLoadDllLibrary);

    OH_MOCK_METHOD(int32_t, MockFilePermissionManager, GetFilePermission, const std::string& path, uint32_t& mode);
};

} // namespace AAFwk
} // namespace OHOS

#endif // MOCK_FILE_PERMISSION_MANAGER_H
