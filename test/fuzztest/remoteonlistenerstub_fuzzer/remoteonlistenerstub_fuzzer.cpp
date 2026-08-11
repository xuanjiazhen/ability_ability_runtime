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

#include "remoteonlistenerstub_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>

#include "remote_on_listener_stub.h"
#include "message_parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace {
const std::u16string ABILITYMGR_INTERFACE_TOKEN = u"ohos.aafwk.RemoteOnListener";
}

class RemoteOnListenerStubFuzzTest : public RemoteOnListenerStub {
public:
    RemoteOnListenerStubFuzzTest() = default;
    virtual ~RemoteOnListenerStubFuzzTest() {}
    void OnCallback(const OnCallbackInfo &info) override {}
};

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    uint32_t code = OHOS::FuzzUtil::GetU32Data(data);
    FuzzedDataProvider fdp(data + OHOS::FuzzUtil::U32_AT_SIZE, size - OHOS::FuzzUtil::U32_AT_SIZE);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(IRemoteOnListener::ON_CALLBACK);

    parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);

    switch (code % 1) {
        case 0: {
            actualCode = static_cast<uint32_t>(IRemoteOnListener::ON_CALLBACK);
            parcel.WriteUint32(static_cast<uint32_t>(OHOS::FuzzUtil::BuildIntegerOverflow(fdp)));
            parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteString(OHOS::FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            uint32_t arraySize = static_cast<uint32_t>(fdp.ConsumeIntegral<uint8_t>() % 4);
            parcel.WriteUint32(arraySize);
            for (uint32_t i = 0; i < arraySize; i++) {
                parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            }
            break;
        }
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<RemoteOnListenerStub> stub = std::make_shared<RemoteOnListenerStubFuzzTest>();

    stub->OnRemoteRequest(actualCode, parcel, reply, option);

    return true;
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        std::cout << "invalid data" << std::endl;
        return 0;
    }

    if (size < OHOS::FuzzUtil::U32_AT_SIZE) {
        return 0;
    }

    char* ch = static_cast<char*>(malloc(size + 1));
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size + 1, data, size) != EOK) {
        std::cout << "copy failed." << std::endl;
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}
