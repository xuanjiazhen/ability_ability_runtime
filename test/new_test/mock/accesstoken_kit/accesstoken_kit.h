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

#ifndef OHOS_ABILITY_RUNTIME_ACCESSTOKEN_KIT_H
#define OHOS_ABILITY_RUNTIME_ACCESSTOKEN_KIT_H

#include <string>
#include "accesstoken_kit.h"
#include "oh_mock_utils.h"

namespace OHOS {
namespace Security {
namespace AccessToken {

// ATokenTypeEnum enumeration
enum class ATokenTypeEnum : int {
    TOKEN_INVALID = -1,
    TOKEN_HAP = 0,
    TOKEN_NATIVE,
    TOKEN_SHELL,
    TOKEN_TYPE_BUTT,
};

// HapTokenInfo struct for mocking
struct HapTokenInfo {
    uint32_t accessTokenIdEx = 0;
    std::string bundleName;
    int32_t userID = 0;
    int32_t instIndex = 0;
};

class AccessTokenKit {
public:
    OH_MOCK_METHOD_WITH_DECORATOR(static, int, AccessTokenKit, GetTokenTypeFlag, uint32_t);

    OH_MOCK_METHOD_WITH_DECORATOR(static, int, AccessTokenKit, GetHapTokenInfo, uint32_t, HapTokenInfo&);
};

}  // namespace AccessToken
}  // namespace Security
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_ACCESSTOKEN_KIT_H
