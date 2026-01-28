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

#ifndef MOCK_ACCESSTOKEN_KIT_H
#define MOCK_ACCESSTOKEN_KIT_H

#include "oh_mock_utils.h"
#include <string>
#include <vector>

namespace OHOS {
namespace Security {
namespace AccessToken {

struct AccessTokenInfo {
    uint32_t tokenId;
    int tokenAttr;
};

class MockAccessTokenKit {
public:
    OH_MOCK_METHOD(int, MockAccessTokenKit, VerifyAccessToken, uint32_t tokenId, const std::string& permissionName);

    OH_MOCK_METHOD(int, MockAccessTokenKit, VerifyAccessToken, uint32_t tokenId, const std::string& permissionName,
                   bool& isStateful);

    OH_MOCK_METHOD_RET_SPTR(AccessTokenInfo*, MockAccessTokenKit, GetAccessTokenInfo, uint32_t tokenId);

    OH_MOCK_METHOD(uint32_t, MockAccessTokenKit, GetHapTokenID, int userId, const std::string& bundleName,
                   int instIndex);

    OH_MOCK_METHOD(int, MockAccessTokenKit, GetTokenType, uint32_t tokenId);

    OH_MOCK_METHOD(uint64_t, MockAccessTokenKit, GetNativeTokenId, const std::string& processName);

    OH_MOCK_METHOD(bool, MockAccessTokenKit, IsSystemAppByTokenID, uint32_t tokenId);

    OH_MOCK_METHOD(int, MockAccessTokenKit, GetDefPermissionList, uint32_t tokenID,
                   std::vector<std::string>& permissions);
};

} // namespace AccessToken
} // namespace Security
} // namespace OHOS

#endif // MOCK_ACCESSTOKEN_KIT_H
