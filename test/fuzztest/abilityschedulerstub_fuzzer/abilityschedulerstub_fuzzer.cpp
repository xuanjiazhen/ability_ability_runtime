/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
#include "abilityschedulerstub_fuzzer.h"
#include "attack_vectors.h"
#include "fuzz_util.h"

using namespace OHOS::AAFwk;

namespace OHOS {
class AbilitySchedulerStubFuzz : public AbilitySchedulerStub {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 10) {
        case 0: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_OPENFILE);
            Uri uri(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteParcelable(&uri);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            OHOS::FuzzUtil::WriteFdLeakPattern(parcel, fdp);
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_OPENRAWFILE);
            Uri uri(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteParcelable(&uri);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            OHOS::FuzzUtil::WritePipeFdLeak(parcel, fdp);
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_GETFILETYPES);
            Uri uri(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteParcelable(&uri);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_GETTYPE);
            Uri uri(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteParcelable(&uri);
            OHOS::FuzzUtil::WriteMmapCorruption(parcel, fdp);
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_ABILITY_CONNECT);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 5: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_ABILITY_DISCONNECT);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 6: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_ABILITY_COMMAND);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 7: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_ABILITY_PREPARE_TERMINATE);
            OHOS::FuzzUtil::WriteAppSpawnMsgOob(parcel, fdp);
            break;
        }
        case 8: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_ABILITY_TRANSACTION);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            OHOS::FuzzUtil::WriteDeepNestedParcel(parcel, fdp);
            bool hasSession = fdp.ConsumeBool();
            parcel.WriteBool(hasSession);
            if (hasSession) {
                SessionInfo sessionInfo;
                parcel.WriteParcelable(&sessionInfo);
            }
            break;
        }
        case 9: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SEND_RESULT);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 100));
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            OHOS::FuzzUtil::WriteUncheckedReadResult(parcel, fdp);
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(AbilitySchedulerStubFuzz, FuzzUtil::Tokens::ABILITY_SCHEDULER)
} // namespace OHOS
