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

#ifndef OHOS_ABILITY_RUNTIME_TOKENID_PERMISSION_H
#define OHOS_ABILITY_RUNTIME_TOKENID_PERMISSION_H

#include <cstdint>
#include "oh_mock_utils.h"

namespace OHOS {
namespace AAFwk {

class TokenIdPermission {
public:
    TokenIdPermission(uint32_t tokenId)
    {
        tokenId_ = tokenId;
    }

    OH_MOCK_METHOD(uint32_t, TokenIdPermission, GetTokenId);

    OH_MOCK_METHOD(bool, TokenIdPermission, VerifyProxyAuthorizationUriPermission);

    OH_MOCK_METHOD(bool, TokenIdPermission, VerifyFileAccessManagerPermission);

    OH_MOCK_METHOD(bool, TokenIdPermission, VerifyReadImageVideoPermission);

    OH_MOCK_METHOD(bool, TokenIdPermission, VerifyWriteImageVideoPermission);

    OH_MOCK_METHOD(bool, TokenIdPermission, VerifyReadAudioPermission);

    OH_MOCK_METHOD(bool, TokenIdPermission, VerifyWriteAudioPermission);

    OH_MOCK_METHOD(bool, TokenIdPermission, VerifyRWDownloadPermission);

    OH_MOCK_METHOD(bool, TokenIdPermission, VerifyRWDeskTopPermission);

    OH_MOCK_METHOD(bool, TokenIdPermission, VerifyRWDocumentsPermission);

    OH_MOCK_METHOD(bool, TokenIdPermission, VerifySandboxAccessPermission);

private:
    uint32_t tokenId_ = 0;
};

}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_TOKENID_PERMISSION_H
