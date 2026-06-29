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

#ifndef OHOS_ABILITY_RUNTIME_FILE_PAGE_RELEASER_H
#define OHOS_ABILITY_RUNTIME_FILE_PAGE_RELEASER_H

#include <cstdint>
#include <string>
#include <vector>

namespace OHOS {
namespace AbilityRuntime {
namespace FilePageReleaser {

// Error codes aligned with interfaces/inner_api/error_utils.
constexpr int32_t ERR_OK = 0;
constexpr int32_t ERR_NOT_SYSTEM_APP = 202;
constexpr int32_t ERR_INVALID_VALUE = 401;
constexpr int32_t ERR_INTERNAL_ERROR = 16000050;

// Release file pages of the given hap/hsp/so files belonging to the calling
// process's own bundle (resolved via BundleMgrHelper::GetBundleInfoForSelf).
// `fileNames` are plain file names (no path separators, no ".."). The whole
// list is validated atomically: if any entry is illegal (bad suffix, contains
// a path separator, or not found in the caller's own bundle), nothing is
// released and ERR_INVALID_VALUE is returned.
// Returns ERR_OK on success.
int32_t ReleaseByFileNames(const std::vector<std::string>& fileNames);

// Release file pages of the top-level rawfile entries of `moduleName` of the
// calling process's own bundle. Subdirectories inside rawfile are NOT released.
// Returns ERR_OK on success.
int32_t ReleaseRawFileByModule(const std::string& moduleName);

} // namespace FilePageReleaser
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_FILE_PAGE_RELEASER_H
