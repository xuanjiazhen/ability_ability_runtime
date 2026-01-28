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

#ifndef MOCK_TOKENID_KIT_H
#define MOCK_TOKENID_KIT_H

#include "oh_mock_utils.h"
#include <string>
#include <vector>

namespace OHOS {
namespace Security {
namespace AccessToken {

enum AtmType : int32_t {
    TOKEN_INVALID = -1,
    TOKEN_NATIVE = 0,
    TOKEN_HAP = 1,
    TOKEN_SHELL = 2,
    TOKEN_TYPE_BUTT,
};

class MockTokenIdKit {
public:
    OH_MOCK_METHOD(uint64_t, MockTokenIdKit, AllocateLocalTokenID, const std::string& remoteDeviceId,
                   uint64_t remoteTokenId);

    OH_MOCK_METHOD(int, MockTokenIdKit, GetTokenIdType, uint64_t tokenId);

    OH_MOCK_METHOD(uint32_t, MockTokenIdKit, GetAccessTokenId, uint64_t tokenId);

    OH_MOCK_METHOD(uint64_t, MockTokenIdKit, GetNativeTokenId, const std::string& processName);

    OH_MOCK_METHOD(bool, MockTokenIdKit, IsNativeTokenId, uint64_t tokenId);

    OH_MOCK_METHOD(bool, MockTokenIdKit, IsHapTokenId, uint64_t tokenId);

    OH_MOCK_METHOD(uint64_t, MockTokenIdKit, GetHapTokenID, int32_t userId, const std::string& bundleName,
                   int32_t instIndex);
};

} // namespace AccessToken
} // namespace Security
} // namespace OHOS

#endif // MOCK_TOKENID_KIT_H
