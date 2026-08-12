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

#include "wantreceiverstub_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "want_receiver_stub.h"
#include "message_parcel.h"
#include "securec.h"
#include "string_ex.h"
#include "want.h"
#include "want_params.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string WANT_RECEIVER_INTERFACE_TOKEN = u"ohos.aafwk.WantReceiver";
}

class WantReceiverStubFuzz : public WantReceiverStub {
public:
    WantReceiverStubFuzz() = default;
    virtual ~WantReceiverStubFuzz() = default;
    void Send(const int32_t resultCode) override {}
    void PerformReceive(const Want &want, int resultCode, const std::string &data, const WantParams &extras,
        bool serialized, bool sticky, int sendingUser) override {}
};

bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    FUZZ_EXTRACT_CODE(data, size);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(IWantReceiver::WANT_RECEIVER_SEND);

    parcel.WriteInterfaceToken(WANT_RECEIVER_INTERFACE_TOKEN);

    switch (code % 2) {
        case 0:
            actualCode = static_cast<uint32_t>(IWantReceiver::WANT_RECEIVER_SEND);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 1:
            actualCode = static_cast<uint32_t>(IWantReceiver::WANT_RECEIVER_PERFORM_RECEIVE);
            parcel.WriteInt32(1);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString16(Str8ToStr16(FuzzUtil::BuildMaliciousBundleName(fdp)));
            parcel.WriteInt32(1);
            WantParams wantParams;
            parcel.WriteParcelable(&wantParams);
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<WantReceiverStub> stub = std::make_shared<WantReceiverStubFuzz>();
    stub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
} // namespace OHOS

FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
