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

#ifndef MOCK_MEDIA_PERMISSION_MANAGER_H
#define MOCK_MEDIA_PERMISSION_MANAGER_H

#include "oh_mock_utils.h"
#include <string>
#include <vector>

#ifdef ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE

namespace OHOS {
namespace Media {

enum class PhotoPermissionType : int32_t {
    READ = 0,
    WRITE = 1,
};

class MockMediaPermissionHelper {
public:
    OH_MOCK_METHOD(bool, MockMediaPermissionHelper, CheckPermission, uint32_t tokenId, PhotoPermissionType type,
                   bool& isRemote);

    OH_MOCK_METHOD(bool, MockMediaPermissionHelper, CheckPermission, uint32_t tokenId, const std::vector<std::string>& uriList,
                   PhotoPermissionType type);

    OH_MOCK_METHOD(int32_t, MockMediaPermissionHelper, GrantPermission, uint32_t tokenId,
                   const std::vector<std::string>& uriList, PhotoPermissionType type);

    OH_MOCK_METHOD(int32_t, MockMediaPermissionHelper, RevokePermission, uint32_t tokenId,
                   const std::vector<std::string>& uriList, PhotoPermissionType type);
};

} // namespace Media
} // namespace OHOS

#endif // ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE

#endif // MOCK_MEDIA_PERMISSION_MANAGER_H
