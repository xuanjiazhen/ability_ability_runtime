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

#ifndef MOCK_INTERFACES_INNER_KITS_ACCESSTOKEN_KIT_H
#define MOCK_INTERFACES_INNER_KITS_ACCESSTOKEN_KIT_H

#include <string>
#include <vector>

#include "mock_common.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
class AccessTokenKit {
public:
    static ATokenTypeEnum GetTokenTypeFlag(AccessTokenID tokenID)
    {
        return ATokenTypeEnum::TOKEN_INVALID;
    }

    static ATokenTypeEnum GetTokenTypeFlag(FullTokenID tokenID)
    {
        return ATokenTypeEnum::TOKEN_INVALID;
    }

    static int GetHapTokenInfo(AccessTokenID tokenID, HapTokenInfo& hapTokenInfoRes)
    {
        return 0;
    }

    static int GetNativeTokenInfo(AccessTokenID tokenID, NativeTokenInfo& nativeTokenInfoRes)
    {
        return 0;
    }
 
    static int VerifyAccessToken(AccessTokenID tokenID, const std::string& permissionName)
    {
        return 0;
    }

    static int VerifyAccessToken(
        AccessTokenID callerTokenID, AccessTokenID firstTokenID, const std::string& permissionName)
    {
        return 0;
    }

    static int VerifyAccessToken(AccessTokenID tokenID, const std::string& permissionName, bool crossIpc)
    {
        return 0;
    }

    static int VerifyAccessToken(AccessTokenID callerTokenID,
        AccessTokenID firstTokenID, const std::string& permissionName, bool crossIpc)
    {
        return 0;
    }

    static int VerifyAccessToken(AccessTokenID tokenID, const std::vector<std::string>& permissionList,
        std::vector<int32_t>& permStateList, bool crossIpc = false)
    {
        return 0;
    }

    static void GetPermissionManagerInfo(PermissionGrantInfo& info) {}
};
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
#endif
