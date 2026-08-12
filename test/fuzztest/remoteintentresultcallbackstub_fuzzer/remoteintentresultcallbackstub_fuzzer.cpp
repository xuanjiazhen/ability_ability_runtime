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

#include "remoteintentresultcallbackstub_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>

#include "message_parcel.h"
#include "remote_intent_result_callback_stub.h"
#include "securec.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace {
const std::u16string ABILITYMGR_INTERFACE_TOKEN = u"ohos.distributedschedule.IRemoteIntentResultCallback";
}

class RemoteIntentResultCallbackStubFuzzTest : public RemoteIntentResultCallbackStub {
public:
    RemoteIntentResultCallbackStubFuzzTest() = default;
    virtual ~RemoteIntentResultCallbackStubFuzzTest()
    {}
    void OnIntentResult(uint64_t requestCode, int32_t resultCode, const std::string& resultMsg) override
    {}
    void OnLinkDisconnected(uint64_t requestCode, int32_t reason) override
    {}
};

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    FUZZ_EXTRACT_CODE(data, size);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(IRemoteIntentResultCallback::ON_INTENT_RESULT);

    parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);

    switch (code % 2) {
        case 0:
            actualCode = static_cast<uint32_t>(IRemoteIntentResultCallback::ON_INTENT_RESULT);
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            break;
        case 1:
            actualCode = static_cast<uint32_t>(IRemoteIntentResultCallback::ON_LINK_DISCONNECTED);
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<RemoteIntentResultCallbackStub> stub =
        std::make_shared<RemoteIntentResultCallbackStubFuzzTest>();

    stub->OnRemoteRequest(actualCode, parcel, reply, option);

    return true;
}
}

/* Fuzzer entry point */
FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
