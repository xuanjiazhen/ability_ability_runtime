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

#include "accesstoken_kit.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
NativeTokenInfo AccessTokenKit::nativeTokenInfo = NativeTokenInfo();
int32_t AccessTokenKit::getNativeTokenInfoRet = 0;
HapTokenInfo AccessTokenKit::hapInfo = HapTokenInfo();
int32_t AccessTokenKit::getHapTokenInfoRet = 0;
int32_t AccessTokenKit::verifyAccessTokenRet = PermissionState::PERMISSION_GRANTED;
ATokenTypeEnum AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_INVALID;
std::vector<int32_t> AccessTokenKit::permissionStateList;

int32_t AccessTokenKit::VerifyAccessToken(AccessTokenID, const std::vector<std::string> &permissions,
    std::vector<int32_t> &permStateList)
{
    if (!permissionStateList.empty()) {
        permStateList = permissionStateList;
    } else {
        permStateList.assign(permissions.size(), verifyAccessTokenRet);
    }
    return verifyAccessTokenRet;
}

int32_t AccessTokenKit::VerifyAccessToken(AccessTokenID, const std::string &, bool)
{
    return verifyAccessTokenRet;
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
