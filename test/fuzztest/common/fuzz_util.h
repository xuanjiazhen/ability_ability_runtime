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

#ifndef FUZZTEST_OHOS_ABILITY_RUNTIME_FUZZ_UTIL_H
#define FUZZTEST_OHOS_ABILITY_RUNTIME_FUZZ_UTIL_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>

namespace OHOS {
namespace FuzzUtil {
// Index constants for U32Data extraction from first 4 bytes
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_TWO = 2;
constexpr int INPUT_THREE = 3;

// Size and bit offset constants for U32Data extraction
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;

// Default max length for fuzz strings and max size for fuzz vectors
constexpr size_t STRING_MAX_LENGTH = 256;
constexpr size_t VEC_MAX_SIZE = 8;

// Extract uint32_t from the first 4 bytes of fuzz input data.
inline uint32_t GetU32Data(const char *ptr)
{
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) |
        (ptr[INPUT_TWO] << OFFSET_TWO) | ptr[INPUT_THREE];
}

// Build a vector of strings from fuzz data. Length is controlled to avoid memory explosion.
inline std::vector<std::string> BuildStringVector(FuzzedDataProvider &fdp,
    size_t maxLength = STRING_MAX_LENGTH, size_t maxSize = VEC_MAX_SIZE)
{
    std::vector<std::string> vec;
    uint8_t count = fdp.ConsumeIntegral<uint8_t>() % maxSize;
    for (uint8_t i = 0; i < count; i++) {
        vec.push_back(fdp.ConsumeRandomLengthString(maxLength));
    }
    return vec;
}

// Build a vector of int32_t from fuzz data. Length is controlled to avoid memory explosion.
inline std::vector<int32_t> BuildInt32Vector(FuzzedDataProvider &fdp, size_t maxSize = VEC_MAX_SIZE)
{
    std::vector<int32_t> vec;
    uint8_t count = fdp.ConsumeIntegral<uint8_t>() % maxSize;
    for (uint8_t i = 0; i < count; i++) {
        vec.push_back(fdp.ConsumeIntegral<int32_t>());
    }
    return vec;
}
} // namespace FuzzUtil
} // namespace OHOS

#define FUZZ_ENTRY_IMPL(Func) \
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) \
{ \
    if (data == nullptr || size < OHOS::FuzzUtil::U32_AT_SIZE) { return 0; } \
    char *ch = static_cast<char *>(malloc(size + 1)); \
    if (ch == nullptr) { return 0; } \
    (void)memset_s(ch, size + 1, 0x00, size + 1); \
    if (memcpy_s(ch, size + 1, data, size) != EOK) { free(ch); return 0; } \
    Func(ch, size); \
    free(ch); \
    return 0; \
}

#define FUZZ_EXTRACT_CODE(data, size) \
    uint32_t code = OHOS::FuzzUtil::GetU32Data(data); \
    FuzzedDataProvider fdp(data + OHOS::FuzzUtil::U32_AT_SIZE, size - OHOS::FuzzUtil::U32_AT_SIZE)

#endif // FUZZTEST_OHOS_ABILITY_RUNTIME_FUZZ_UTIL_H
