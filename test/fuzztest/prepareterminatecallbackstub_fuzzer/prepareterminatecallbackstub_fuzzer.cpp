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
FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
