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

#include "dataabilityobserverstub_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>

#include "data_ability_observer_stub.h"
#include "parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string DATA_ABILITY_OBSERVER_INTERFACE_TOKEN = u"ohos.aafwk.DataAbilityObserver";
}

class DataAbilityObserverStubFuzz : public DataAbilityObserverStub {
public:
    DataAbilityObserverStubFuzz() = default;
    virtual ~DataAbilityObserverStubFuzz()
    {}
    void OnChange() override
    {}
    void OnChangeExt(const ChangeInfo &changeInfo) override
    {}
    void OnChangePreferences(const std::string &key) override
    {}
};

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    FUZZ_EXTRACT_CODE(data, size);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(IDataAbilityObserver::DATA_ABILITY_OBSERVER_CHANGE);

    parcel.WriteInterfaceToken(DATA_ABILITY_OBSERVER_INTERFACE_TOKEN);

    switch (code % 3) {
        case 0:
            actualCode = static_cast<uint32_t>(IDataAbilityObserver::DATA_ABILITY_OBSERVER_CHANGE);
            break;
        case 1: {
            actualCode = static_cast<uint32_t>(IDataAbilityObserver::DATA_ABILITY_OBSERVER_CHANGE_EXT);
            parcel.WriteUint32(static_cast<uint32_t>(OHOS::FuzzUtil::BuildInvalidEnum(fdp, 5)));
            uint32_t uriCount = static_cast<uint32_t>(fdp.ConsumeIntegral<uint8_t>() % 4);
            parcel.WriteUint32(uriCount);
            for (uint32_t i = 0; i < uriCount; i++) {
                parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            }
            parcel.WriteUint32(static_cast<uint32_t>(fdp.ConsumeIntegral<uint8_t>() % 64));
            break;
        }
        case 2:
            actualCode = static_cast<uint32_t>(IDataAbilityObserver::DATA_ABILITY_OBSERVER_CHANGE_PREFERENCES);
            parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            break;
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<DataAbilityObserverStub> dataabilityobserverstub =
        std::make_shared<DataAbilityObserverStubFuzz>();
    dataabilityobserverstub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
}

FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
