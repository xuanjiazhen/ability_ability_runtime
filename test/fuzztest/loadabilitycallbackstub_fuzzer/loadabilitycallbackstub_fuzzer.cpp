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

#include "loadabilitycallbackstub_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>

#include "load_ability_callback_stub.h"
#include "parcel.h"
#include "securec.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string ABILITYMGR_INTERFACE_TOKEN = u"ohos.appexecfwk.ILoadAbilityCallback";
}

class LoadAbilityCallbackStubFuzz : public LoadAbilityCallbackStub {
public:
    LoadAbilityCallbackStubFuzz() = default;
    ~LoadAbilityCallbackStubFuzz() = default;

    void OnFinish(int32_t pid) override {}
};

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    FUZZ_EXTRACT_CODE(data, size);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(ILoadAbilityCallback::Message::TRANSACT_ON_FINISH);

    parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);

    switch (code % 1) {
        case 0:
            actualCode = static_cast<uint32_t>(ILoadAbilityCallback::Message::TRANSACT_ON_FINISH);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<LoadAbilityCallbackStub> stub = std::make_shared<LoadAbilityCallbackStubFuzz>();
    stub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
}

FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
