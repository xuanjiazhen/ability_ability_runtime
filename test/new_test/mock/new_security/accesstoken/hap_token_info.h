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

#ifndef OHOS_ABILITY_RUNTIME_HAP_TOKEN_INFO_H
#define OHOS_ABILITY_RUNTIME_HAP_TOKEN_INFO_H

#include <string>
#include "access_token.h"

namespace OHOS {
namespace Security {
namespace AccessToken {

/**
 * @brief Declares hap token info class for mocking
 */
struct HapTokenInfo {
    char ver = 0;
    int userID = 0;
    std::string bundleName;
    int32_t apiVersion = 0;
    int instIndex = 0;
    int dlpType = 0;
    AccessTokenID tokenID = 0;
    AccessTokenAttr tokenAttr = 0;
};

}  // namespace AccessToken
}  // namespace Security
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_HAP_TOKEN_INFO_H
