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

#ifndef OHOS_ABILITY_RUNTIME_ACCESS_TOKEN_H
#define OHOS_ABILITY_RUNTIME_ACCESS_TOKEN_H

#include <stdint.h>

namespace OHOS {
namespace Security {
namespace AccessToken {

using AccessTokenID = uint32_t;
using AccessTokenIDEx = uint64_t;
using AccessTokenAttr = uint32_t;
using FullTokenID = uint64_t;

constexpr AccessTokenID INVALID_TOKENID = 0;

/**
 * @brief Token id type
 */
typedef enum TypeATokenTypeEnum {
    TOKEN_INVALID = -1,
    TOKEN_HAP = 0,
    TOKEN_NATIVE,
    TOKEN_SHELL,
    TOKEN_TYPE_BUTT,
} ATokenTypeEnum;

/**
 * @brief Permission used type enumeration
 */
enum PermUsedTypeEnum : int {
    PERMISSION_NOT_USED = 0,
    PERMISSION_USED,
    PERMISSION_ALWAYS_USED,
};

enum AccessTokenKitRet {
    RET_FAILED = -1,
    RET_SUCCESS = 0,
};

}  // namespace AccessToken
}  // namespace Security
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_ACCESS_TOKEN_H
