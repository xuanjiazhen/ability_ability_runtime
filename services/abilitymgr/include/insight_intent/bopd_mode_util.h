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

#ifndef OHOS_ABILITY_RUNTIME_BOPD_MODE_UTIL_H
#define OHOS_ABILITY_RUNTIME_BOPD_MODE_UTIL_H

#include <string>
#include <unordered_set>

#include "parameters.h"

namespace OHOS {
namespace AbilityRuntime {

constexpr const char* BOPD_MODE_PARAM = "ohos.boot.bopd.mode";
constexpr const char* BOPD_DEFAULT_VALUE = "0x2";

inline bool IsBopdMode()
{
    static const bool isBopd = []() {
        std::string value = OHOS::system::GetParameter(BOPD_MODE_PARAM, BOPD_DEFAULT_VALUE);
        static const std::unordered_set<std::string> BOPD_STATES = {
            "0x2", "0x3", "0x6", "0x7", "0xa", "0xe", "0xf"
        };
        return BOPD_STATES.count(value) > 0;
    }();
    return isBopd;
}

}  // namespace AbilityRuntime
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_BOPD_MODE_UTIL_H
