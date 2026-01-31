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
#include "access_token.h"
#include "hap_token_info.h"
#include "oh_mock_utils.h"

namespace OHOS {
namespace Security {
namespace AccessToken {

class AccessTokenKit {
public:
    OH_MOCK_METHOD_WITH_DECORATOR(static, ATokenTypeEnum, AccessTokenKit, GetTokenTypeFlag, uint32_t);

    OH_MOCK_METHOD_WITH_DECORATOR_AND_OUTPUT_1(static, int, hapInfo, AccessTokenKit, GetHapTokenInfo,
        uint32_t, HapTokenInfo& hapInfo);
};

}  // namespace AccessToken
}  // namespace Security
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_ACCESSTOKEN_KIT_H
