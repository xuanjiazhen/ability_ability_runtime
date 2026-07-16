/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "accesstoken_kit.h"

#include "mock_my_flag.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
// Test-only interposition of AccessTokenKit::GetHostTokenId over the shared libaccesstoken_sdk.
// Defaults (MyFlag::mockGetHostTokenIdRet_ = -1, mockHostTokenId_ = 0) mimic a failing lookup so
// tests that do not arm the mock keep the prior "host not resolved" behavior.
int32_t AccessTokenKit::GetHostTokenId(AccessTokenID toolTokenId, AccessTokenID &hostTokenId)
{
    hostTokenId = OHOS::AAFwk::MyFlag::mockHostTokenId_;
    return OHOS::AAFwk::MyFlag::mockGetHostTokenIdRet_;
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
