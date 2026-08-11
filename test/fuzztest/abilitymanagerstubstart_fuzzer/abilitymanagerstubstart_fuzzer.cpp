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

#include "abilitymanagerstubstart_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "ability_manager_stub.h"
#include "ability_manager_stub_mock.h"
#include "message_parcel.h"
#include "securec.h"
#include "session_info.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string ABILITYMGR_INTERFACE_TOKEN = u"ohos.aafwk.AbilityManager";
}

class AbilityManagerStubStartFuzz : public AbilityManagerStubFuzzBase {};

bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    uint32_t code = FuzzUtil::GetU32Data(data);
    FuzzedDataProvider fdp(data + FuzzUtil::U32_AT_SIZE, size - FuzzUtil::U32_AT_SIZE);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY);

    parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);

    switch (code % 6) {
        case 0: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteString(FuzzUtil::GenStrcpyOverflowString(fdp, 128));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_EXTENSION_ABILITY);
            bool hasSession = fdp.ConsumeBool();
            parcel.WriteBool(hasSession);
            if (hasSession) {
                SessionInfo sessionInfo;
                parcel.WriteParcelable(&sessionInfo);
            }
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_EXTENSION_ABILITY);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            OHOS::FuzzUtil::WriteMaliciousExtensionRunningInfo(parcel, fdp);
            bool hasCaller = fdp.ConsumeBool();
            parcel.WriteBool(hasCaller);
            if (hasCaller) {
                parcel.WriteRemoteObject(nullptr);
            }
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_FOR_RESULT_AS_CALLER);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            bool hasCaller = fdp.ConsumeBool();
            parcel.WriteBool(hasCaller);
            if (hasCaller) {
                parcel.WriteRemoteObject(nullptr);
            }
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_CALL_ABILITY);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteRemoteObject(nullptr);
            bool hasCaller = fdp.ConsumeBool();
            parcel.WriteBool(hasCaller);
            if (hasCaller) {
                parcel.WriteRemoteObject(nullptr);
            }
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 5: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_AS_CALLER_BY_TOKEN);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            bool hasCaller = fdp.ConsumeBool();
            parcel.WriteBool(hasCaller);
            if (hasCaller) {
                parcel.WriteRemoteObject(nullptr);
            }
            bool hasAsCaller = fdp.ConsumeBool();
            parcel.WriteBool(hasAsCaller);
            if (hasAsCaller) {
                parcel.WriteRemoteObject(nullptr);
            }
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<AbilityManagerStub> stub = std::make_shared<AbilityManagerStubStartFuzz>();
    stub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        std::cout << "invalid data" << std::endl;
        return 0;
    }

    if (size < OHOS::FuzzUtil::U32_AT_SIZE) {
        return 0;
    }

    char *ch = static_cast<char *>(malloc(size + 1));
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
