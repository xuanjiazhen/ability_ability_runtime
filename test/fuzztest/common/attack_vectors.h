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
constexpr size_t DEFAULT_MAX_STR_LEN = 256;
constexpr int32_t INVALID_ENUM_OFFSET = 0xFFFF;
constexpr int32_t MAX_NEST_DEPTH = 8;

inline std::string WriteTruncatedString(FuzzedDataProvider &fdp, size_t maxLen)
{
    std::string s = fdp.ConsumeRandomLengthString(maxLen);
    if (maxLen > 0 && s.size() > maxLen) {
        s.resize(maxLen);
    }
    return s;
}

// Build an oversized string exceeding normal limits to trigger buffer overflow.
inline std::string BuildOversizedString(FuzzedDataProvider &fdp, size_t targetLen = OVERSIZED_STRING_LENGTH,
    size_t maxLen = 0)
{
    std::string s;
    size_t buildLen = (maxLen > 0 && maxLen < targetLen) ? maxLen : targetLen;
    s.reserve(buildLen);
    while (s.size() < buildLen && fdp.remaining_bytes() > 0) {
        s += fdp.ConsumeRandomLengthString(256);
    }
    if (maxLen > 0 && s.size() > maxLen) {
        s.resize(maxLen);
    }
    return s;
}

// Build a string with special characters: null bytes, path traversal, format string, escape.
inline std::string BuildSpecialCharString(FuzzedDataProvider &fdp, size_t maxLen = 0)
{
    static const std::string specialChars = "\x00../\n\r\t\x1b%s%d\\;|`$(){}";
    std::string base = WriteTruncatedString(fdp, (maxLen > 0 && maxLen < 128) ? maxLen : 128);
    for (size_t i = 0; i < base.size(); i++) {
        if (fdp.ConsumeBool()) {
            uint8_t idx = fdp.ConsumeIntegral<uint8_t>() % specialChars.size();
        base[i] = specialChars[idx];
        }
    }
    if (maxLen > 0 && base.size() > maxLen) {
        base.resize(maxLen);
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
inline void WriteMaliciousWant(MessageParcel &parcel, FuzzedDataProvider &fdp,
    size_t maxStrLen = DEFAULT_MAX_STR_LEN)
{
   AAFwk::Want want;
    want.SetAction(BuildOversizedString(fdp, maxStrLen, maxStrLen));
    want.SetUri(BuildSpecialCharString(fdp, maxStrLen));
    want.SetEntity(BuildSpecialCharString(fdp, maxStrLen));
    want.SetType(BuildSpecialCharString(fdp, maxStrLen));
    want.AddEntity(BuildSpecialCharString(fdp, maxStrLen));
    want.SetBundle(BuildOversizedString(fdp, maxStrLen, maxStrLen));
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
inline std::string BuildMaliciousBundleName(FuzzedDataProvider &fdp, size_t maxLen = 0)
{
    if (fdp.ConsumeBool()) {
        return BuildOversizedString(fdp, (maxLen > 0 && maxLen < 2048) ? maxLen : 2048, maxLen);
    }
    return BuildSpecialCharString(fdp, maxLen);
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

inline std::string BuildUriAttackString(FuzzedDataProvider &fdp, size_t maxLen = 0)
{
    static const std::string uriPayloads[] = {
        "file:///../../etc/passwd",
        "file:///etc/shadow",
        "file://localhost/etc/passwd",
        "content://../../data/local/tmp",
        "http://127.0.0.1:0/",
        "https://localhost/admin",
        "file:///proc/self/environ",
        "content://com.attacker/../../data",
        "file:///sys/class/net",
        "file:///dev/null",
        "data:text/html,<script>",
        "javascript:alert(1)",
        "file:///../../system/bin/sh",
        "content://media/external/file/../../etc",
    };
    std::string result;
    if (fdp.ConsumeBool()) {
        result = uriPayloads[fdp.ConsumeIntegral<uint8_t>() % 14];
    } else {
        result = fdp.ConsumeRandomLengthString(128);
    }
    if (fdp.ConsumeBool()) {
        result += BuildSpecialCharString(fdp, 32);
    }
    if (maxLen > 0 && result.size() > maxLen) {
        result.resize(maxLen);
    }
    return result;
}

inline std::string BuildSandboxEscapePath(FuzzedDataProvider &fdp, size_t maxLen = 0)
{
    static const std::string escapePaths[] = {
        "../../etc/passwd",
        "../../../data/local/tmp",
        "..\\..\\windows\\system32",
        "/proc/self/environ",
        "/sys/class/net",
        "../../system/bin/sh",
        "/dev/null",
        "../../../../dbsync/sandbox",
        "..%2f..%2fetc%2fpasswd",
        "/data/local/tmp/../../etc",
        "/system/lib64/../../etc",
        "../appdata/../../data",
    };
    std::string result;
    if (fdp.ConsumeBool()) {
        result = escapePaths[fdp.ConsumeIntegral<uint8_t>() % 12];
    } else {
        result = fdp.ConsumeRandomLengthString(128);
    }
    if (fdp.ConsumeBool()) {
        result += BuildSpecialCharString(fdp, 32);
    }
    if (maxLen > 0 && result.size() > maxLen) {
        result.resize(maxLen);
    }
    return result;
}

// Build a SQL injection string with classic patterns.
inline std::string BuildSqlInjectionString(FuzzedDataProvider &fdp, size_t maxLen = DEFAULT_MAX_STR_LEN)
{
    static const std::string sqlPayloads[] = {
        "' OR '1'='1",
        "'; DROP TABLE users--",
        "UNION SELECT * FROM secrets--",
        "' AND 1=0 UNION SELECT password FROM accounts--",
        "'; INSERT INTO admin VALUES('hacker','pwned')--",
        "' OR SLEEP(5)--",
        "admin'--",
    };
    std::string result = fdp.ConsumeRandomLengthString(maxLen / 2);
    uint8_t payloadCount = fdp.ConsumeIntegral<uint8_t>() % 3;
    for (uint8_t i = 0; i < payloadCount; i++) {
        result += sqlPayloads[fdp.ConsumeIntegral<uint8_t>() % 7];
    }
    if (maxLen > 0 && result.size() > maxLen) {
        result.resize(maxLen);
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
inline void WriteMaliciousAppStateData(MessageParcel &parcel, FuzzedDataProvider &fdp,
    size_t maxStrLen = DEFAULT_MAX_STR_LEN)
{
    parcel.WriteInt32(BuildIntegerOverflow(fdp));
    parcel.WriteString(BuildMaliciousBundleName(fdp, maxStrLen));
    parcel.WriteString(BuildSpecialCharString(fdp, maxStrLen));
    parcel.WriteInt32(BuildInvalidEnum(fdp, 20));
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteInt32(BuildIntegerOverflow(fdp));
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteBool(fdp.ConsumeBool());
}

// Write a malicious AbilityStateData parcelable with invalid token info and overflow values.
inline void WriteMaliciousAbilityStateData(MessageParcel &parcel, FuzzedDataProvider &fdp,
    size_t maxStrLen = DEFAULT_MAX_STR_LEN)
{
    parcel.WriteInt32(BuildInvalidEnum(fdp, 30));
    parcel.WriteString(BuildMaliciousBundleName(fdp, maxStrLen));
    parcel.WriteString(BuildSpecialCharString(fdp, maxStrLen));
    parcel.WriteInt32(BuildIntegerOverflow(fdp));
    parcel.WriteInt32(BuildIntegerOverflow(fdp));
    parcel.WriteInt32(BuildInvalidEnum(fdp, 10));
    parcel.WriteInt32(BuildIntegerOverflow(fdp));
    parcel.WriteBool(fdp.ConsumeBool());
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
}
// Verify that an integer value is indeed out of valid range for precise overflow testing.
inline bool VerifyIntegerOverflow(int32_t value, int32_t validMax)
{
    return value > validMax || value < 0;
}

// Write a malicious ProcessData parcelable with overflow pid and malicious bundleName.
inline void WriteMaliciousProcessData(MessageParcel &parcel, FuzzedDataProvider &fdp,
    size_t maxStrLen = DEFAULT_MAX_STR_LEN)
{
    parcel.WriteInt32(BuildIntegerOverflow(fdp));
    parcel.WriteString(BuildMaliciousBundleName(fdp, maxStrLen));
    parcel.WriteString(BuildSpecialCharString(fdp, maxStrLen));
    parcel.WriteInt32(BuildInvalidEnum(fdp, 20));
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
}

// Write a malicious PageStateData parcelable with malicious bundleName and oversized url.
inline void WriteMaliciousPageStateData(MessageParcel &parcel, FuzzedDataProvider &fdp,
    size_t maxStrLen = DEFAULT_MAX_STR_LEN)
{
    parcel.WriteString(BuildMaliciousBundleName(fdp, maxStrLen));
    parcel.WriteString(BuildSpecialCharString(fdp, maxStrLen));
    parcel.WriteString(BuildSpecialCharString(fdp, maxStrLen));
    parcel.WriteString(BuildOversizedString(fdp, maxStrLen, maxStrLen));
    parcel.WriteInt32(BuildInvalidEnum(fdp, 10));
}

// Write a malicious ExtensionRunningInfo with malicious extensionName and overflow pid.
inline void WriteMaliciousExtensionRunningInfo(MessageParcel &parcel, FuzzedDataProvider &fdp,
    size_t maxStrLen = DEFAULT_MAX_STR_LEN)
{
    parcel.WriteString(BuildMaliciousBundleName(fdp, maxStrLen));
    parcel.WriteString(BuildSpecialCharString(fdp, maxStrLen));
    parcel.WriteInt32(BuildIntegerOverflow(fdp));
    parcel.WriteInt32(BuildInvalidEnum(fdp, 20));
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
}
// Parcelable serialization phase enum for stage-specific malicious construction.
enum ParcelablePhase {
    PHASE_BEFORE_SERIALIZE,
    PHASE_DURING_SERIALIZE,
    PHASE_AFTER_DESERIALIZE,
};

// Write a phase-aware malicious AppStateData with stage-specific attack vectors.
inline void WriteMaliciousAppStateDataPhased(MessageParcel &parcel, FuzzedDataProvider &fdp,
    ParcelablePhase phase, size_t maxStrLen = DEFAULT_MAX_STR_LEN)
{
    switch (phase) {
        case PHASE_BEFORE_SERIALIZE:
            parcel.WriteInt32(BuildIntegerOverflow(fdp));
            parcel.WriteString(BuildOversizedString(fdp, maxStrLen, maxStrLen));
            break;
        case PHASE_DURING_SERIALIZE:
            parcel.WriteString(BuildSpecialCharString(fdp, maxStrLen));
            {
                std::string u = BuildUnicodeAttackString(fdp);
                if (maxStrLen > 0 && u.size() > maxStrLen) {
                    u.resize(maxStrLen);
                }
                parcel.WriteString(u);
            }
            break;
        case PHASE_AFTER_DESERIALIZE:
            parcel.WriteInt32(BuildInvalidEnum(fdp, 20));
            parcel.WriteString(BuildMaliciousBundleName(fdp, maxStrLen));
            break;
    }
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteBool(fdp.ConsumeBool());
}

// Generate a string that precisely overflows a target buffer of given size (strcpy overflow).
inline std::string GenStrcpyOverflowString(FuzzedDataProvider &fdp, size_t targetBufSize)
{
    std::string s;
    size_t overflowSize = targetBufSize + (fdp.ConsumeIntegral<uint8_t>() % 128) + 1;
    s.reserve(overflowSize);
    while (s.size() < overflowSize && fdp.remaining_bytes() > 0) {
        s += fdp.ConsumeRandomLengthString(64);
    }
    return s;
}

// Generate a byte vector that precisely overflows a memcpy target buffer.
inline std::vector<uint8_t> GenMemcpyOverflowData(FuzzedDataProvider &fdp, size_t targetBufSize)
{
    std::vector<uint8_t> data;
    size_t overflowSize = targetBufSize + (fdp.ConsumeIntegral<uint8_t>() % 128) + 1;
    auto bytes = fdp.ConsumeBytes<uint8_t>(overflowSize);
    data.assign(bytes.begin(), bytes.end());
    while (data.size() < overflowSize) {
        data.push_back(fdp.ConsumeIntegral<uint8_t>());
    }
    return data;
}
// Build a malicious exit reason string for process termination fuzzing.
inline std::string BuildExitReason(FuzzedDataProvider &fdp, size_t maxLen = DEFAULT_MAX_STR_LEN)
{
    static const std::string reasons[] = {
        "SIGKILL", "SIGTERM", "OOM_RECLAIM", "SCHEDULER_TIMEOUT",
        "APP_CRASH", "ANR_TRIGGERED", "FORCE_STOP", "USER_REQUEST",
        "SYSTEM_REBOOT", "LOW_MEMORY", "RESOURCE_EXHAUSTED", "SECURITY_VIOLATION"
    };
    std::string result = reasons[fdp.ConsumeIntegral<uint8_t>() % 12];
    if (fdp.ConsumeBool()) {
        result += ":" + BuildSpecialCharString(fdp, 64);
    }
    if (maxLen > 0 && result.size() > maxLen) {
        result.resize(maxLen);
    }
    return result;
}

// Write malicious fault data with crash type and stack trace.
inline void WriteMaliciousFaultData(MessageParcel &parcel, FuzzedDataProvider &fdp,
    size_t maxStrLen = DEFAULT_MAX_STR_LEN)
{
    parcel.WriteInt32(BuildInvalidEnum(fdp, 20));
    parcel.WriteString(BuildOversizedString(fdp, maxStrLen));
    parcel.WriteInt32(BuildIntegerOverflow(fdp));
    parcel.WriteString(BuildSpecialCharString(fdp, maxStrLen));
    parcel.WriteInt32(BuildIntegerOverflow(fdp));
    parcel.WriteString(BuildSpecialCharString(fdp, maxStrLen));
}

// Write malicious configuration with oversized and special char values.
inline void WriteMaliciousConfiguration(MessageParcel &parcel, FuzzedDataProvider &fdp,
    size_t maxStrLen = DEFAULT_MAX_STR_LEN)
{
    parcel.WriteString(BuildOversizedString(fdp, maxStrLen));
    parcel.WriteInt32(BuildInvalidEnum(fdp, 10));
    parcel.WriteString(BuildSpecialCharString(fdp, maxStrLen));
    parcel.WriteString(BuildUnicodeAttackString(fdp));
    parcel.WriteBool(fdp.ConsumeBool());
    parcel.WriteInt32(BuildIntegerOverflow(fdp));
}

// Write malicious permission data with token and permission list.
inline void WriteMaliciousPermission(MessageParcel &parcel, FuzzedDataProvider &fdp,
    size_t maxStrLen = DEFAULT_MAX_STR_LEN)
{
    parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
    parcel.WriteString(BuildSpecialCharString(fdp, maxStrLen));
    parcel.WriteInt32(BuildInvalidEnum(fdp, 5));
    parcel.WriteInt32(BuildIntegerOverflow(fdp));
    parcel.WriteString(BuildSpecialCharString(fdp, maxStrLen));
    parcel.WriteBool(fdp.ConsumeBool());
}

// Write malicious attack awareness data with sensor and behavior patterns.
inline void WriteMaliciousAttackAware(MessageParcel &parcel, FuzzedDataProvider &fdp,
    size_t maxStrLen = DEFAULT_MAX_STR_LEN)
{
    parcel.WriteInt32(BuildInvalidEnum(fdp, 15));
    parcel.WriteString(BuildSpecialCharString(fdp, maxStrLen));
    parcel.WriteString(BuildSpecialCharString(fdp, maxStrLen));
    parcel.WriteInt32(BuildIntegerOverflow(fdp));
    parcel.WriteInt32(BuildIntegerOverflow(fdp));
    parcel.WriteString(BuildOversizedString(fdp, maxStrLen));
    parcel.WriteBool(fdp.ConsumeBool());
}
} // namespace FuzzUtil
} // namespace OHOS

#endif // FUZZTEST_OHOS_ABILITY_RUNTIME_ATTACK_VECTORS_H
