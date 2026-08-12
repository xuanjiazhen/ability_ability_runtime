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

#include "abilitymanagerstubselfstart_fuzzer.h"
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
#include "start_options.h"
#include "start_params_by_SCB.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string ABILITYMGR_INTERFACE_TOKEN = u"ohos.aafwk.AbilityManager";
}

class AbilityManagerStubSelfStartFuzz : public AbilityManagerStubFuzzBase {};

bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    FUZZ_EXTRACT_CODE(data, size);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::NDK_START_SELF_UI_ABILITY);

    switch (code % 11) {
        case 0: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::NDK_START_SELF_UI_ABILITY);
            parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SELF_UI_ABILITY_WITH_START_OPTIONS);
            parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            StartOptions startOptions;
            parcel.WriteParcelable(&startOptions);
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SELF_UI_ABILITY_WITH_PID_RESULT);
            parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            StartOptions startOptions;
            parcel.WriteParcelable(&startOptions);
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SELF_UI_ABILITY_WITH_TOKEN);
            parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            bool hasCaller = fdp.ConsumeBool();
            parcel.WriteBool(hasCaller);
            if (hasCaller) {
                parcel.WriteRemoteObject(nullptr);
            }
            OHOS::FuzzUtil::WriteAppSpawnMsgOob(parcel, fdp);
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SELF_UI_ABILITY_WITH_OPTIONS_AND_TOKEN);
            parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            StartOptions startOptions;
            parcel.WriteParcelable(&startOptions);
            bool hasCaller = fdp.ConsumeBool();
            parcel.WriteBool(hasCaller);
            if (hasCaller) {
                parcel.WriteRemoteObject(nullptr);
            }
            break;
        }
        case 5: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_SESSION_ABILITY_ADD_CALLER);
            parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            bool hasCaller = fdp.ConsumeBool();
            parcel.WriteBool(hasCaller);
            if (hasCaller) {
                parcel.WriteRemoteObject(nullptr);
            }
            bool hasSession = fdp.ConsumeBool();
            parcel.WriteBool(hasSession);
            if (hasSession) {
                SessionInfo sessionInfo;
                parcel.WriteParcelable(&sessionInfo);
            }
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 6: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_SESSION_ABILITY_FOR_OPTIONS);
            parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            StartOptions startOptions;
            parcel.WriteParcelable(&startOptions);
            bool hasCaller = fdp.ConsumeBool();
            parcel.WriteBool(hasCaller);
            if (hasCaller) {
                parcel.WriteRemoteObject(nullptr);
            }
            bool hasSession = fdp.ConsumeBool();
            parcel.WriteBool(hasSession);
            if (hasSession) {
                SessionInfo sessionInfo;
                parcel.WriteParcelable(&sessionInfo);
            }
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 7: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_AS_CALLER_FOR_OPTIONS);
            parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            StartOptions startOptions;
            parcel.WriteParcelable(&startOptions);
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
        case 8: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_EXTENSION_ABILITY_EMBEDDED);
            parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
            bool hasSession = fdp.ConsumeBool();
            parcel.WriteBool(hasSession);
            if (hasSession) {
                SessionInfo sessionInfo;
                parcel.WriteParcelable(&sessionInfo);
            }
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(OHOS::FuzzUtil::BuildSymlinkAttack(fdp));
            break;
        }
        case 9: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_EXTENSION_CONSTRAINED_EMBEDDED);
            parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
            bool hasSession = fdp.ConsumeBool();
            parcel.WriteBool(hasSession);
            if (hasSession) {
                SessionInfo sessionInfo;
                parcel.WriteParcelable(&sessionInfo);
            }
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 10: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_ABILITY_BY_SCB);
            parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
            bool hasSession = fdp.ConsumeBool();
            parcel.WriteBool(hasSession);
            if (hasSession) {
                SessionInfo sessionInfo;
                parcel.WriteParcelable(&sessionInfo);
            }
            AbilityRuntime::StartParamsBySCB params;
            parcel.WriteParcelable(&params);
            break;
        }
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<AbilityManagerStub> stub = std::make_shared<AbilityManagerStubSelfStartFuzz>();
    stub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
}  // namespace OHOS

FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
