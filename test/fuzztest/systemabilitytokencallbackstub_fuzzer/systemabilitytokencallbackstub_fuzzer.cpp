/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "fuzz_util.h"

#include <cstddef>
#include <cstdint>

#include "system_ability_token_callback_stub.h"
#include "message_parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;

namespace OHOS {
class SystemAbilityTokenCallbackStubFuzzTest : public SystemAbilityTokenCallbackStub {
public:
    SystemAbilityTokenCallbackStubFuzzTest() = default;
    virtual ~SystemAbilityTokenCallbackStubFuzzTest()
    {}
    int32_t SendResult(OHOS::AAFwk::Want& want, int32_t callerUid, int32_t requestCode,
        uint32_t accessToken, int32_t resultCode) override
    {
        return 0;
    }
};

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    uint32_t code = FuzzUtil::GetU32Data(data);

    MessageParcel parcel;
    parcel.WriteInterfaceToken(std::u16string(FuzzUtil::Tokens::SA_TOKEN_CALLBACK));
    parcel.WriteBuffer(data, size);
    parcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    std::shared_ptr<SystemAbilityTokenCallbackStub> systemabilitytokencallbackstub
        = std::make_shared<SystemAbilityTokenCallbackStubFuzzTest>();

    systemabilitytokencallbackstub->OnRemoteRequest(code, parcel, reply, option);

    return true;
}
}

FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
