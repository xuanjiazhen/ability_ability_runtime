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

#include "prepareterminatecallbackstub_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>

#include "prepare_terminate_callback_stub.h"
#include "message_parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace {
const std::u16string PREPARETERMINATECALLBACK_INTERFACE_TOKEN = u"ohos.aafwk.prepareTerminateCallback";
}

class PrepareTerminateCallbackStubFuzzTest : public PrepareTerminateCallbackStub {
public:
    PrepareTerminateCallbackStubFuzzTest() = default;
    virtual ~PrepareTerminateCallbackStubFuzzTest()
    {}
};

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(IPrepareTerminateCallback::ON_DO_PREPARE_TERMINATE);

    parcel.WriteInterfaceToken(PREPARETERMINATECALLBACK_INTERFACE_TOKEN);
    parcel.RewindRead(0);

    std::shared_ptr<PrepareTerminateCallbackStub> prepareterminatecallbackstub =
        std::make_shared<PrepareTerminateCallbackStubFuzzTest>();

    prepareterminatecallbackstub->OnRemoteRequest(actualCode, parcel, reply, option);

    return true;
}
}

/* Fuzzer entry point */
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
