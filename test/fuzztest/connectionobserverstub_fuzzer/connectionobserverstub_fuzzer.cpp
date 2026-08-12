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

#include "connectionobserverstub_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "connection_observer_stub.h"
#include "connection_data.h"
#include "message_parcel.h"
#include "securec.h"
#include "string_ex.h"

using namespace OHOS::AbilityRuntime;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string CONNECTION_OBSERVER_INTERFACE_TOKEN = u"ohos.abilityruntime.connectionobserver";
}

class ConnectionObserverStubFuzz : public ConnectionObserverStub {
public:
    ConnectionObserverStubFuzz() = default;
    virtual ~ConnectionObserverStubFuzz() = default;
    void OnExtensionConnected(const ConnectionData &data) override {}
    void OnExtensionDisconnected(const ConnectionData &data) override {}
    void OnExtensionSuspended(const ConnectionData &data) override {}
    void OnExtensionResumed(const ConnectionData &data) override {}
#ifdef WITH_DLP
    void OnDlpAbilityOpened(const DlpStateData &data) override {}
    void OnDlpAbilityClosed(const DlpStateData &data) override {}
#endif // WITH_DLP
};

void WriteMaliciousConnectionData(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    parcel.WriteInt32(1);
    parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
    parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
    parcel.WriteString16(Str8ToStr16(FuzzUtil::BuildMaliciousBundleName(fdp)));
    parcel.WriteString16(Str8ToStr16(FuzzUtil::BuildSpecialCharString(fdp)));
    parcel.WriteString16(Str8ToStr16(FuzzUtil::BuildSpecialCharString(fdp)));
    parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 30));
    parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
    parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
    parcel.WriteString16(Str8ToStr16(FuzzUtil::BuildSpecialCharString(fdp)));
    parcel.WriteBool(fdp.ConsumeBool());
}

bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    FUZZ_EXTRACT_CODE(data, size);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(IConnectionObserver::ON_EXTENSION_CONNECTED);

    parcel.WriteInterfaceToken(CONNECTION_OBSERVER_INTERFACE_TOKEN);

    switch (code % 4) {
        case 0:
            actualCode = static_cast<uint32_t>(IConnectionObserver::ON_EXTENSION_CONNECTED);
            WriteMaliciousConnectionData(parcel, fdp);
            break;
        case 1:
            actualCode = static_cast<uint32_t>(IConnectionObserver::ON_EXTENSION_DISCONNECTED);
            WriteMaliciousConnectionData(parcel, fdp);
            break;
        case 2:
            actualCode = static_cast<uint32_t>(IConnectionObserver::ON_EXTENSION_SUSPENDED);
            WriteMaliciousConnectionData(parcel, fdp);
            break;
        case 3:
            actualCode = static_cast<uint32_t>(IConnectionObserver::ON_EXTENSION_RESUMED);
            WriteMaliciousConnectionData(parcel, fdp);
            break;
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<ConnectionObserverStub> stub = std::make_shared<ConnectionObserverStubFuzz>();
    stub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
} // namespace OHOS

FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
