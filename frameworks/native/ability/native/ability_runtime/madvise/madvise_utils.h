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

#ifndef MADVISE_UTILS_H
#define MADVISE_UTILS_H

#include <cstdint>
#include <string>
#include <vector>

namespace OHOS {
namespace AbilityRuntime {
namespace MadviseUtil {
bool MadviseSingleLibrary(const char* libName);

int32_t MadviseGeneralFiles(const std::vector<std::string>& filenames);

int32_t MadviseWithConfigFile(const char* bundleName);

// File offset span [start, end) within a single file. Used to release only the
// pages that correspond to a sub-region of a larger mapped file (e.g. a
// rawfile entry stored inside a hap zip archive).
struct FileOffsetSpan {
    uint64_t start;
    uint64_t end;
};

// Release file pages of `filename` that fall within any of the given file
// offset spans. VMAs of the file are obtained from /proc/self/maps; for each
// VMA the intersection with the span (in file-offset space) is computed and
// madvise(MADV_DONTNEED) is applied to the matching virtual sub-range. Only
// read-only, non-writable mappings are touched. Returns the number of VMA
// sub-ranges successfully released.
int32_t MadviseFilePagesAtOffsets(const std::string& filename, const std::vector<FileOffsetSpan>& spans);

} // namespace MadviseUtil
} // namespace AbilityRuntime
} // namespace OHOS
#endif // MADVISE_UTILS_H