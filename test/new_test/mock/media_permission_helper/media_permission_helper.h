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

#ifndef OHOS_ABILITY_RUNTIME_MEDIA_PERMISSION_HELPER_H
#define OHOS_ABILITY_RUNTIME_MEDIA_PERMISSION_HELPER_H

#include <vector>
#include <string>
#include "oh_mock_utils.h"

namespace OHOS {
namespace Media {

// PhotoPermissionType enum
enum class PhotoPermissionType {
    TEMPORARY_READ_IMAGEVIDEO = 0,
    TEMPORARY_WRITE_IMAGEVIDEO,
    TEMPORARY_READWRITE_IMAGEVIDEO,
};

// HideSensitiveType enum
enum class HideSensitiveType {
    HIDE_SENSITIVE_NONE = 0,
    HIDE_SENSITIVE_YES = 1,
};

class MediaPermissionHelper {
public:
    OH_MOCK_METHOD_WITH_DECORATOR(static, MediaPermissionHelper*, MediaPermissionHelper, GetMediaPermissionHelper);

    OH_MOCK_VOID_METHOD(MediaPermissionHelper, InitMediaPermissionHelper);

    // CheckPhotoUriPermission has output parameter 'results' (3rd param)
    OH_MOCK_METHOD_WITH_OUTPUT_1(int32_t, results, MediaPermissionHelper, CheckPhotoUriPermission,
        uint32_t, const std::vector<std::string>&, std::vector<bool>&, const std::vector<uint32_t>&);

    OH_MOCK_METHOD(int32_t, MediaPermissionHelper, GrantPhotoUriPermission,
        uint32_t, uint32_t, std::vector<std::string>&, std::vector<PhotoPermissionType>&, HideSensitiveType);

    OH_MOCK_METHOD(int32_t, MediaPermissionHelper, CancelPhotoUriPermission,
        uint32_t, uint32_t, std::vector<std::string>&);
};

}  // namespace Media
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_MEDIA_PERMISSION_HELPER_H
