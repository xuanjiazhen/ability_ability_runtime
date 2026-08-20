/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "abilityconnectionstub_fuzzer.h"
#include "fuzz_util.h"

#include <cstddef>
#include <cstdint>

#include "ability_connect_callback_stub.h"
#include "message_parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;

namespace OHOS {
class AbilityConnectionStubFuzzTest : public AbilityConnectionStub {
public:
    AbilityConnectionStubFuzzTest() = default;
    virtual ~AbilityConnectionStubFuzzTest()
    {};
    void OnAbilityConnectDone(const AppExecFwk::ElementName& element,
        const sptr<IRemoteObject>& remoteObject, int resultCode) override
    {}
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) override
    {}
};

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    uint32_t code = FuzzUtil::GetU32Data(data);

    MessageParcel parcel;
    parcel.WriteInterfaceToken(std::u16string(FuzzUtil::Tokens::ABILITY_CONNECTION));
    parcel.WriteBuffer(data, size);
    parcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    std::shared_ptr<AbilityConnectionStub> abilityconnectionstub = std::make_shared<AbilityConnectionStubFuzzTest>();

    abilityconnectionstub->OnRemoteRequest(code, parcel, reply, option);

    return true;
}
}

FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
