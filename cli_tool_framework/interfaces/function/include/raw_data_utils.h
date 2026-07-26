/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

#ifndef OHOS_ABILITY_RUNTIME_RAW_DATA_UTILS_H
#define OHOS_ABILITY_RUNTIME_RAW_DATA_UTILS_H

#include <cstdint>
#include <sstream>
#include <string>

#include <nlohmann/json.hpp>

#include "cli_error_code.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

// Read exactly len bytes into buf; returns false if the stream enters a failed state.
inline bool ReadRaw(std::stringstream &ss, void *buf, std::streamsize len)
{
    ss.read(reinterpret_cast<char *>(buf), len);
    return static_cast<bool>(ss);
}

// Deserialize a single item from the stream into out.
// T must expose: static bool ParseFromJson(const nlohmann::json &, T &).
// allowComments controls whether the JSON parser tolerates comments;
// parseFailCode is returned when JSON parsing or ParseFromJson fails.
template <typename T>
inline int32_t ReadOneItem(std::stringstream &ss, uint32_t ssLength, T &out,
    bool allowComments, int32_t parseFailCode)
{
    uint32_t itemSize = 0;
    if (!ReadRaw(ss, &itemSize, sizeof(itemSize))) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read item size, stream state invalid");
        return ERR_INVALID_VALUE;
    }
    std::streamoff curPos = ss.tellg();
    if (curPos < 0 || static_cast<uint32_t>(curPos) > ssLength ||
        itemSize > ssLength - static_cast<uint32_t>(curPos)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "itemSize:%{public}u is invalid", itemSize);
        return ERR_INVALID_VALUE;
    }
    std::string itemStr(itemSize, '\0');
    if (!ReadRaw(ss, itemStr.data(), itemSize)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read item data, stream state invalid");
        return ERR_INVALID_VALUE;
    }
    nlohmann::json j = nlohmann::json::parse(itemStr, nullptr, false, allowComments);
    if (j.is_discarded()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to parse JSON");
        return parseFailCode;
    }
    if (!T::ParseFromJson(j, out)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to parse item from JSON");
        return parseFailCode;
    }
    return ERR_OK;
}

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_RAW_DATA_UTILS_H
