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
#include "number_convert_utils.h"

#include <charconv>
#include <iostream>
#include <string>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
bool NumberConvertUtils::ConvertToInt(const std::string &str, int32_t &value)
{
    auto res = std::from_chars(str.c_str(), str.c_str() + str.size(), value);
    return res.ec == std::errc();
}

int32_t NumberConvertUtils::ConvertToIntNoResult(const std::string &str)
{
    int32_t value = -1;
    auto res = std::from_chars(str.c_str(), str.c_str() + str.size(), value);
    if (res.ec != std::errc()) {
        TAG_LOGW(AAFwkTag::DIALOG, "invalid str: %{public}s", str.c_str());
    }
    return value;
}

uint64_t NumberConvertUtils::ConvertToUInt64NoResult(const std::string &str)
{
    uint64_t value = 0;
    auto res = std::from_chars(str.c_str(), str.c_str() + str.size(), value);
    if (res.ec != std::errc()) {
        TAG_LOGW(AAFwkTag::DIALOG, "invalid str: %{public}s", str.c_str());
    }
    return value;
}
}  // namespace AAFwk
}  // namespace OHOS