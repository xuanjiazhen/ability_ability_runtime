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

#ifndef FUZZTEST_OHOS_ABILITY_RUNTIME_ATTACK_VECTORS_H
#define FUZZTEST_OHOS_ABILITY_RUNTIME_ATTACK_VECTORS_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>
#include "message_parcel.h"
#include "want.h"

namespace OHOS {
namespace FuzzUtil {
// Attack vector constants
constexpr size_t OVERSIZED_STRING_LENGTH = 4096;
constexpr int32_t INVALID_ENUM_OFFSET = 0xFFFF;
constexpr int32_t MAX_NEST_DEPTH = 8;

// Build an oversized string exceeding normal limits to trigger buffer overflow.
inline std::string BuildOversizedString(FuzzedDataProvider &fdp, size_t targetLen = OVERSIZED_STRING_LENGTH)
{
    std::string s;
    s.reserve(targetLen);
    while (s.size() < targetLen && fdp.remaining_bytes() > 0) {
        s += fdp.ConsumeRandomLengthString(256);
    }
    return s;
}

// Build a string with special characters: null bytes, path traversal, format string, escape.
inline std::string BuildSpecialCharString(FuzzedDataProvider &fdp)
{
    static const std::string specialChars = "\x00../\n\r\t\x1b%s%d\\;|`$(){}";
    std::string base = fdp.ConsumeRandomLengthString(128);
    for (size_t i = 0; i < base.size(); i++) {
        if (fdp.ConsumeBool()) {
            uint8_t idx = fdp.ConsumeIntegral<uint8_t>() % specialChars.size();
        base[i] = specialChars[idx];
        }
    }
    return base;
}

// Build an invalid enum value out of valid range to trigger unexpected code paths.
inline int32_t BuildInvalidEnum(FuzzedDataProvider &fdp, int32_t enumMax)
{
    int32_t val = fdp.ConsumeIntegral<int32_t>();
    if (fdp.ConsumeBool()) {
        val = enumMax + (val % 100) + 1;
    } else if (fdp.ConsumeBool()) {
        val = -1 - (val % 100);
    }
    return val;
}

// Build a vector of invalid pids (negative, zero, max int32) for KillProcesses fuzzing.
inline std::vector<int32_t> BuildInvalidPids(FuzzedDataProvider &fdp)
{
    std::vector<int32_t> pids;
    uint8_t count = fdp.ConsumeIntegral<uint8_t>() % VEC_MAX_SIZE;
    static const int32_t invalidPidValues[] = { -1, 0, INT32_MAX, INT32_MIN, 1, 99999 };
    for (uint8_t i = 0; i < count; i++) {
        if (fdp.ConsumeBool()) {
            pids.push_back(invalidPidValues[fdp.ConsumeIntegral<uint8_t>() % 6]);
        } else {
            pids.push_back(fdp.ConsumeIntegral<int32_t>());
        }
    }
    return pids;
}

// Write a malicious Want into parcel with oversized action, invalid URI, special char entity.
inline void WriteMaliciousWant(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
   AAFwk::Want want;
    want.SetAction(BuildOversizedString(fdp, 1024));
    want.SetUri(BuildSpecialCharString(fdp));
    want.SetEntity(BuildSpecialCharString(fdp));
    want.SetType(BuildSpecialCharString(fdp));
    want.AddEntity(BuildSpecialCharString(fdp));
    want.SetBundle(BuildOversizedString(fdp, 512));
    want.SetFlags(fdp.ConsumeIntegral<uint32_t>());
    want.Marshal(parcel);
}

// Write a deep nested parcel structure to trigger recursive parsing issues.
inline void WriteDeepNestedParcel(MessageParcel &parcel, FuzzedDataProvider &fdp, int depth = MAX_NEST_DEPTH)
{
    if (depth <= 0 || fdp.remaining_bytes() == 0) {
        parcel.WriteString(fdp.ConsumeRandomLengthString(64));
        return;
    }
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteString(BuildSpecialCharString(fdp));
    WriteDeepNestedParcel(parcel, fdp, depth - 1);
}

// Build a malicious bundleName with path traversal, special chars, or oversized content.
inline std::string BuildMaliciousBundleName(FuzzedDataProvider &fdp)
{
    if (fdp.ConsumeBool()) {
        return BuildOversizedString(fdp, 2048);
    }
    return BuildSpecialCharString(fdp);
}

// Build an integer overflow value near INT32_MAX to trigger arithmetic overflow.
inline int32_t BuildIntegerOverflow(FuzzedDataProvider &fdp)
{
    int32_t val = 0x7FFFFFFF;
    if (fdp.ConsumeBool()) {
        val += fdp.ConsumeIntegral<int32_t>() % 100 + 1;
    } else if (fdp.ConsumeBool()) {
        val = static_cast<int32_t>(0x80000000) + (fdp.ConsumeIntegral<int32_t>() % 100);
    }
    return val;
}

// Build a Unicode attack string with multi-byte UTF-8 edge cases (BOM, overlong, surrogate, beyond max).
inline std::string BuildUnicodeAttackString(FuzzedDataProvider &fdp)
{
    static const std::string unicodePayloads[] = {
        "\xEF\xBB\xBF",
        "\xC0\x80",
        "\xED\xA0\x80",
        "\xF4\x90\x80\x80",
        "\xE2\x80\x8E\xE2\x80\x8F",
        "\xE2\x80\xA6",
    };
    std::string result = fdp.ConsumeRandomLengthString(64);
    uint8_t payloadCount = fdp.ConsumeIntegral<uint8_t>() % 4;
    for (uint8_t i = 0; i < payloadCount; i++) {
        result += unicodePayloads[fdp.ConsumeIntegral<uint8_t>() % 6];
    }
    return result;
}

// Build a vector of malicious strings mixing oversized, special-char and unicode content.
inline std::vector<std::string> BuildMaliciousStringVector(FuzzedDataProvider &fdp)
{
    std::vector<std::string> vec;
    uint8_t count = fdp.ConsumeIntegral<uint8_t>() % VEC_MAX_SIZE;
    for (uint8_t i = 0; i < count; i++) {
        uint8_t choice = fdp.ConsumeIntegral<uint8_t>() % 3;
        if (choice == 0) {
            vec.push_back(BuildOversizedString(fdp, 1024));
        } else if (choice == 1) {
            vec.push_back(BuildSpecialCharString(fdp));
        } else {
            vec.push_back(BuildUnicodeAttackString(fdp));
        }
    }
    return vec;
}

// Build a vector of malicious int32_t with overflow and boundary values.
inline std::vector<int32_t> BuildMaliciousInt32Vector(FuzzedDataProvider &fdp)
{
    std::vector<int32_t> vec;
    uint8_t count = fdp.ConsumeIntegral<uint8_t>() % VEC_MAX_SIZE;
    static const int32_t boundaryValues[] = { 0x7FFFFFFF, static_cast<int32_t>(0x80000000), 0, -1, 1, 0x7FFFFFFE, 2 };
    for (uint8_t i = 0; i < count; i++) {
        uint8_t choice = fdp.ConsumeIntegral<uint8_t>() % 3;
        if (choice == 0) {
            vec.push_back(BuildIntegerOverflow(fdp));
        } else if (choice == 1) {
            vec.push_back(boundaryValues[fdp.ConsumeIntegral<uint8_t>() % 7]);
        } else {
            vec.push_back(fdp.ConsumeIntegral<int32_t>());
        }
    }
    return vec;
}

// Write a malicious AppStateData parcelable with overflow pid, invalid state, malicious bundleName.
inline void WriteMaliciousAppStateData(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    parcel.WriteInt32(BuildIntegerOverflow(fdp));
    parcel.WriteString(BuildMaliciousBundleName(fdp));
    parcel.WriteString(BuildSpecialCharString(fdp));
    parcel.WriteInt32(BuildInvalidEnum(fdp, 20));
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteInt32(BuildIntegerOverflow(fdp));
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteBool(fdp.ConsumeBool());
}

// Write a malicious AbilityStateData parcelable with invalid token info and overflow values.
inline void WriteMaliciousAbilityStateData(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    parcel.WriteInt32(BuildInvalidEnum(fdp, 30));
    parcel.WriteString(BuildMaliciousBundleName(fdp));
    parcel.WriteString(BuildSpecialCharString(fdp));
    parcel.WriteInt32(BuildIntegerOverflow(fdp));
    parcel.WriteInt32(BuildIntegerOverflow(fdp));
    parcel.WriteInt32(BuildInvalidEnum(fdp, 10));
    parcel.WriteInt32(BuildIntegerOverflow(fdp));
    parcel.WriteBool(fdp.ConsumeBool());
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
}
} // namespace FuzzUtil
} // namespace OHOS

#endif // FUZZTEST_OHOS_ABILITY_RUNTIME_ATTACK_VECTORS_H
