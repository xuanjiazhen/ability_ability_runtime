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

#include "quickfixcallbackstub_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>

#include "parcel.h"
#include "quick_fix_callback_stub.h"
#include "securec.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string ABILITYMGR_INTERFACE_TOKEN = u"ohos.appexecfwk.QuickFixCallback";
}

class QuickFixCallbackStubFuzz : public QuickFixCallbackStub {
public:
    QuickFixCallbackStubFuzz() = default;
    ~QuickFixCallbackStubFuzz() = default;

    void OnLoadPatchDone(int32_t resultCode, int32_t recordId) override {}
    void OnUnloadPatchDone(int32_t resultCode, int32_t recordId) override {}
    void OnReloadPageDone(int32_t resultCode, int32_t recordId) override {}
};

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    FUZZ_EXTRACT_CODE(data, size);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(IQuickFixCallback::ON_NOTIFY_LOAD_PATCH);

    parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);

    switch (code % 3) {
        case 0:
            actualCode = static_cast<uint32_t>(IQuickFixCallback::ON_NOTIFY_LOAD_PATCH);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 1:
            actualCode = static_cast<uint32_t>(IQuickFixCallback::ON_NOTIFY_UNLOAD_PATCH);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 2:
            actualCode = static_cast<uint32_t>(IQuickFixCallback::ON_NOTIFY_RELOAD_PAGE);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<QuickFixCallbackStub> stub = std::make_shared<QuickFixCallbackStubFuzz>();
    stub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
}

FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
