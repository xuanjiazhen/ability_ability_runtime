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

#include "statusbardelegatestub_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>

#include "message_parcel.h"
#include "securec.h"
#include "status_bar_delegate_stub.h"

using namespace OHOS::AbilityRuntime;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string INTERFACE_TOKEN = u"ohos.ability.StatusBarDelegate";
}

class StatusBarDelegateStubFuzzTest : public StatusBarDelegateStub {
public:
    StatusBarDelegateStubFuzzTest() = default;
    virtual ~StatusBarDelegateStubFuzzTest()
    {}
    int32_t CheckIfStatusBarItemExists(uint32_t accessTokenId, const std::string &instanceKey, bool& isExist) override
    {
        return 0;
    }
    int32_t AttachPidToStatusBarItem(uint32_t accessTokenId, int32_t pid, const std::string &instanceKey) override
    {
        return 0;
    }
    int32_t DetachPidToStatusBarItem(uint32_t accessTokenId, int32_t pid, const std::string &instanceKey) override
    {
        return 0;
    }
};

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    uint32_t code = OHOS::FuzzUtil::GetU32Data(data);
    FuzzedDataProvider fdp(data + OHOS::FuzzUtil::U32_AT_SIZE, size - OHOS::FuzzUtil::U32_AT_SIZE);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(StatusBarDelegateCmd::CHECK_IF_STATUS_BAR_ITEM_EXISTS);

    parcel.WriteInterfaceToken(INTERFACE_TOKEN);

    switch (code % 3) {
        case 0:
            actualCode = static_cast<uint32_t>(StatusBarDelegateCmd::CHECK_IF_STATUS_BAR_ITEM_EXISTS);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            break;
        case 1:
            actualCode = static_cast<uint32_t>(StatusBarDelegateCmd::ATTACH_PID_TO_STATUS_BAR_ITEM);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(OHOS::FuzzUtil::BuildSandboxEscapePath(fdp));
            break;
        case 2:
            actualCode = static_cast<uint32_t>(StatusBarDelegateCmd::DETACH_PID_TO_STATUS_BAR_ITEM);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(OHOS::FuzzUtil::BuildSandboxEscapePath(fdp));
            break;
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<StatusBarDelegateStub> stub = std::make_shared<StatusBarDelegateStubFuzzTest>();
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
