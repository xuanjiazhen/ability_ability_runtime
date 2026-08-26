/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
#include "systemabilitytokencallbackstub_fuzzer.h"
#include "attack_vectors.h"
#include "fuzz_util.h"

#include "system_ability_token_callback_stub.h"

using namespace OHOS::AAFwk;

namespace OHOS {
class SystemAbilityTokenCallbackStubFuzz : public SystemAbilityTokenCallbackStub {
public:
    SystemAbilityTokenCallbackStubFuzz() = default;
    ~SystemAbilityTokenCallbackStubFuzz() = default;
    int32_t SendResult(OHOS::AAFwk::Want &want, int32_t callerUid, int32_t requestCode,
        uint32_t accessToken, int32_t resultCode) override
    {
        return 0;
    }
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 1) {
        case 0: {
            actualCode = static_cast<uint32_t>(ISystemAbilityTokenCallback::SEND_RESULT);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(SystemAbilityTokenCallbackStubFuzz, FuzzUtil::Tokens::SA_TOKEN_CALLBACK)
} // namespace OHOS
