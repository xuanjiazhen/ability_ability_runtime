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

#include "accesstoken_kit.h"

namespace OHOS {
namespace Security {
namespace AccessToken {

// 可配置的 token 类型，默认返回 HAP 以支持 GetBundleInfoByTokenId 成功
static ATokenTypeEnum g_tokenTypeFlag = ATokenTypeEnum::TOKEN_HAP;

ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(uint64_t tokenID)
{
    (void)tokenID;
    return g_tokenTypeFlag;
}

// 提供设置函数用于测试
void SetTokenTypeFlag(ATokenTypeEnum tokenType)
{
    g_tokenTypeFlag = tokenType;
}

int32_t AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string &permissionName, bool crossIpc)
{
    (void)tokenID;
    (void)permissionName;
    (void)crossIpc;
    return PermissionState::PERMISSION_GRANTED;
}

int32_t AccessTokenKit::DeleteToolTokenByPid(int32_t pid)
{
    (void)pid;
    return 0;  // Mock implementation - return success
}

AccessTokenKitRet AccessTokenKit::GetHapTokenInfo(AccessTokenID tokenID, HapTokenInfo& hapTokenInfo)
{
    (void)tokenID;
    hapTokenInfo.bundleName = "test_bundle";
    hapTokenInfo.userID = 0;
    hapTokenInfo.instIndex = 0;
    return AccessTokenKitRet::RET_SUCCESS;
}

} // namespace AccessToken
} // namespace Security
} // namespace OHOS
