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

#include "abilityforegroundstateobserverstub_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>

#include "ability_foreground_state_observer_stub.h"
#include "ability_state_data.h"
#include "parcel.h"
#include "securec.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string ABILITYMGR_INTERFACE_TOKEN = u"ohos.appexecfwk.IAbilityForegroundStateObserver";
}

class AbilityForegroundStateObserverStubFuzz : public AbilityForegroundStateObserverStub {
public:
    AbilityForegroundStateObserverStubFuzz() = default;
    ~AbilityForegroundStateObserverStubFuzz() = default;

    void OnAbilityStateChanged(const AbilityStateData &abilityStateData) override {}
};

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    FUZZ_EXTRACT_CODE(data, size);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(IAbilityForegroundStateObserver::Message::ON_ABILITY_STATE_CHANGED);

    parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);

    switch (code % 1) {
        case 0:
            actualCode = static_cast<uint32_t>(IAbilityForegroundStateObserver::Message::ON_ABILITY_STATE_CHANGED);
            OHOS::FuzzUtil::WriteMaliciousAbilityStateData(parcel, fdp);
            break;
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<AbilityForegroundStateObserverStub> stub =
        std::make_shared<AbilityForegroundStateObserverStubFuzz>();
    stub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
}

FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
