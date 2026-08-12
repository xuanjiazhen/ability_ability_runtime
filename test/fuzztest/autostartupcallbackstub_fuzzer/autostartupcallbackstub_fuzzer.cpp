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

#include "autostartupcallbackstub_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>

#include "ability_manager_ipc_interface_code.h"
#include "auto_startup_callback_stub.h"
#include "auto_startup_info.h"
#include "parcel.h"
#include "securec.h"

using namespace OHOS::AbilityRuntime;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string ABILITYMGR_INTERFACE_TOKEN = u"ohos.aafwk.AutoStartupCallBack";
}

class AutoStartupCallBackStubFuzz : public AutoStartupCallBackStub {
public:
    AutoStartupCallBackStubFuzz() = default;
    ~AutoStartupCallBackStubFuzz() = default;

    void OnAutoStartupOn(const AutoStartupInfo &info) override {}
    void OnAutoStartupOff(const AutoStartupInfo &info) override {}
};

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    FUZZ_EXTRACT_CODE(data, size);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::ON_AUTO_STARTUP_ON);

    parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);

    switch (code % 2) {
        case 0:
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::ON_AUTO_STARTUP_ON);
            {
                AutoStartupInfo info;
                parcel.WriteParcelable(info);
            }
            break;
        case 1:
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::ON_AUTO_STARTUP_OFF);
            {
                AutoStartupInfo info;
                parcel.WriteParcelable(info);
            }
            break;
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<AutoStartupCallBackStub> stub = std::make_shared<AutoStartupCallBackStubFuzz>();
    stub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
}

FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
