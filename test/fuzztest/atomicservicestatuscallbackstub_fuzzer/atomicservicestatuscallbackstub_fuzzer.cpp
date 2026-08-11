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

#include "atomicservicestatuscallbackstub_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>

#include "atomic_service_status_callback_stub.h"
#include "parcel.h"
#include "securec.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string ABILITYMGR_INTERFACE_TOKEN = u"ohos.IAtomicServiceStatusCallback";
}

class AtomicServiceStatusCallbackStubFuzz : public AtomicServiceStatusCallbackStub {
public:
    AtomicServiceStatusCallbackStubFuzz() = default;
    ~AtomicServiceStatusCallbackStubFuzz() = default;

    void OnInstallFinished(int resultCode, const Want &want, int32_t userId) override {}
    void OnRemoteInstallFinished(int resultCode, const Want &want, int32_t userId) override {}
};

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    uint32_t code = OHOS::FuzzUtil::GetU32Data(data);
    FuzzedDataProvider fdp(data + OHOS::FuzzUtil::U32_AT_SIZE, size - OHOS::FuzzUtil::U32_AT_SIZE);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(IAtomicServiceStatusCallback::ON_FREE_INSTALL_DONE);

    parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);

    switch (code % 2) {
        case 0:
            actualCode = static_cast<uint32_t>(IAtomicServiceStatusCallback::ON_FREE_INSTALL_DONE);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 1:
            actualCode = static_cast<uint32_t>(IAtomicServiceStatusCallback::ON_REMOTE_FREE_INSTALL_DONE);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<AtomicServiceStatusCallbackStub> stub = std::make_shared<AtomicServiceStatusCallbackStubFuzz>();
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
