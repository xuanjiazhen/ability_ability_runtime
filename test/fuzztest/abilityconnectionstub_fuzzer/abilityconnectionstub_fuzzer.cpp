/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
#include "ability_connect_callback_stub.h"
#include "abilityconnectionstub_fuzzer.h"
#include "attack_vectors.h"

#include "element_name.h"
#include "fuzz_util.h"

using namespace OHOS::AAFwk;

namespace OHOS {
class AbilityConnectionStubFuzz : public AbilityConnectionStub {
public:
    AbilityConnectionStubFuzz() = default;
    ~AbilityConnectionStubFuzz() = default;
    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int resultCode) override {}
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override {}
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 3) {
        case 0: {
            actualCode = static_cast<uint32_t>(IAbilityConnection::ON_ABILITY_CONNECT_DONE);
            AppExecFwk::ElementName element;
            parcel.WriteParcelable(&element);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(IAbilityConnection::ON_ABILITY_DISCONNECT_DONE);
            AppExecFwk::ElementName element;
            parcel.WriteParcelable(&element);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(IAbilityConnection::ON_REMOTE_STATE_CHANGED);
            AppExecFwk::ElementName element;
            parcel.WriteParcelable(&element);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(AbilityConnectionStubFuzz, FuzzUtil::Tokens::ABILITY_CONNECTION)
} // namespace OHOS
