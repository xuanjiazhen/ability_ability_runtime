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

#include "abilityschedulerstub_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "ability_scheduler_stub.h"
#include "data_ability_observer_interface.h"
#include "message_parcel.h"
#include "securec.h"
#include "session_info.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string ABILITYSCHEDULER_INTERFACE_TOKEN = u"ohos.aafwk.AbilityScheduler";
}

class AbilitySchedulerStubFuzz : public AbilitySchedulerStub {
public:
    AbilitySchedulerStubFuzz() = default;
    ~AbilitySchedulerStubFuzz() = default;

    bool ScheduleAbilityTransaction(const Want &want, const LifeCycleStateInfo &targetState,
        sptr<SessionInfo> sessionInfo) override { return true; }
    void ScheduleShareData(const int32_t &uniqueId) override {}
    void SendResult(int requestCode, int resultCode, const Want &resultWant) override {}
    void ScheduleConnectAbility(const Want &want) override {}
    void ScheduleDisconnectAbility(const Want &want) override {}
    void ScheduleCommandAbility(const Want &want, bool restart, int startId) override {}
    void ScheduleCommandAbilityWindow(const Want &want, const sptr<SessionInfo> &sessionInfo,
        WindowCommand winCmd) override {}
    bool SchedulePrepareTerminateAbility() override { return true; }
    void ScheduleSaveAbilityState() override {}
    void ScheduleRestoreAbilityState(const PacMap &inState) override {}
    std::vector<std::string> GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter) override { return {}; }
    int OpenFile(const Uri &uri, const std::string &mode) override { return 0; }
    int OpenRawFile(const Uri &uri, const std::string &mode) override { return 0; }
    int Insert(const Uri &uri, const NativeRdb::ValuesBucket &value) override { return 0; }
    int Update(const Uri &uri, const NativeRdb::ValuesBucket &value,
        const NativeRdb::DataAbilityPredicates &predicates) override { return 0; }
    int Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates) override { return 0; }
    std::shared_ptr<AppExecFwk::PacMap> Call(const Uri &uri, const std::string &method,
        const std::string &arg, const AppExecFwk::PacMap &pacMap) override { return {}; }
    std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(const Uri &uri,
        std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates) override { return {}; }
    std::string GetType(const Uri &uri) override { return {}; }
    bool Reload(const Uri &uri, const PacMap &extras) override { return true; }
    int BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values) override { return 0; }
    bool ScheduleRegisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver) override
    {
        return true;
    }
    bool ScheduleUnregisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver) override
    {
        return true;
    }
    bool ScheduleNotifyChange(const Uri &uri) override { return true; }
    Uri NormalizeUri(const Uri &uri) override { return Uri{"abilityschedulerstub"}; }
    Uri DenormalizeUri(const Uri &uri) override { return Uri{"abilityschedulerstub"}; }
    std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> ExecuteBatch(
        const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> &operations) override { return {}; }
    void ContinueAbility(const std::string &deviceId, uint32_t versionCode) override {}
    void NotifyContinuationResult(int32_t result) override {}
    void DumpAbilityInfo(const std::vector<std::string> &params, std::vector<std::string> &info) override {}
    void CallRequest() override {}
    void OnExecuteIntent(const Want &want) override {}
    int CreateModalUIExtension(const Want &want) override { return 0; }
    void UpdateSessionToken(sptr<IRemoteObject> sessionToken) override {}
    void ScheduleCollaborate(const Want &want) override {}
    void ScheduleAbilityRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message, int32_t resultCode) override {}
    void ScheduleAbilityRequestSuccess(const std::string &requestId,
        const AppExecFwk::ElementName &element) override {}
    void ScheduleAbilitiesRequestDone(const std::string &requestKey, int32_t resultCode) override {}
};

bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    FUZZ_EXTRACT_CODE(data, size);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_OPENFILE);

    parcel.WriteInterfaceToken(ABILITYSCHEDULER_INTERFACE_TOKEN);

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

    parcel.RewindRead(0);
    std::shared_ptr<AbilitySchedulerStub> stub = std::make_shared<AbilitySchedulerStubFuzz>();
    stub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
}  // namespace OHOS

FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
