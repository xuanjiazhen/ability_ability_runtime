/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "dataabilityrecord_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#include "data_ability_record.h"
#undef private

#include "ability_fuzz_util.h"
#include "ability_scheduler_interface.h"
#include "iremote_stub.h"

using namespace std::chrono;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
constexpr int32_t API_START_LOADING = 0;
constexpr int32_t API_WAIT_AND_GET_SCHEDULER = 1;
constexpr int32_t API_ATTACH_AND_TRANSITION = 2;
constexpr int32_t API_CLIENT_MANAGEMENT = 3;
constexpr int32_t API_KILL_AND_DUMP = 4;
constexpr int32_t API_OTHER = 5;
constexpr int32_t API_START_AND_ATTACH = 6;
constexpr int32_t API_CLIENT_AND_KILL = 7;
constexpr int32_t MAX_API_CASE = API_CLIENT_AND_KILL;

class FuzzAbilityScheduler : public IRemoteStub<IAbilityScheduler> {
public:
    bool ScheduleAbilityTransaction(const Want &want,
        const LifeCycleStateInfo &targetState,
        sptr<SessionInfo> sessionInfo = nullptr) override
    {
        return true;
    }
    void ScheduleShareData(const int32_t &uniqueId) override {}
    void SendResult(int requestCode, int resultCode,
        const Want &resultWant) override {}
    void ScheduleConnectAbility(const Want &want) override {}
    void ScheduleDisconnectAbility(const Want &want) override {}
    void ScheduleCommandAbility(const Want &want,
        bool restart, int startId) override {}
    void ScheduleCommandAbilityWindow(const Want &want,
        const sptr<SessionInfo> &sessionInfo,
        WindowCommand winCmd) override {}
    bool SchedulePrepareTerminateAbility() override { return false; }
    void ScheduleSaveAbilityState() override {}
    void ScheduleRestoreAbilityState(const PacMap &inState) override {}
    std::vector<std::string> GetFileTypes(const Uri &uri,
        const std::string &mimeTypeFilter) override { return {}; }
    int OpenFile(const Uri &uri,
        const std::string &mode) override { return 0; }
    int OpenRawFile(const Uri &uri,
        const std::string &mode) override { return 0; }
    int Insert(const Uri &uri,
        const NativeRdb::ValuesBucket &value) override { return 0; }
    int Update(const Uri &uri,
        const NativeRdb::ValuesBucket &value,
        const NativeRdb::DataAbilityPredicates &predicates) override
    {
        return 0;
    }
    int Delete(const Uri &uri,
        const NativeRdb::DataAbilityPredicates &predicates) override
    {
        return 0;
    }
    std::shared_ptr<PacMap> Call(const Uri &uri,
        const std::string &method, const std::string &arg,
        const PacMap &pacMap) override { return nullptr; }
    std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(
        const Uri &uri, std::vector<std::string> &columns,
        const NativeRdb::DataAbilityPredicates &predicates) override
    {
        return nullptr;
    }
    std::string GetType(const Uri &uri) override { return ""; }
    bool Reload(const Uri &uri, const PacMap &extras) override
    {
        return false;
    }
    int BatchInsert(const Uri &uri,
        const std::vector<NativeRdb::ValuesBucket> &values) override
    {
        return 0;
    }
    bool ScheduleRegisterObserver(const Uri &uri,
        const sptr<IDataAbilityObserver> &dataObserver) override
    {
        return false;
    }
    bool ScheduleUnregisterObserver(const Uri &uri,
        const sptr<IDataAbilityObserver> &dataObserver) override
    {
        return false;
    }
    bool ScheduleNotifyChange(const Uri &uri) override { return false; }
    Uri NormalizeUri(const Uri &uri) override { return Uri(""); }
    Uri DenormalizeUri(const Uri &uri) override { return Uri(""); }
    std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> ExecuteBatch(
        const std::vector<std::shared_ptr<
            AppExecFwk::DataAbilityOperation>> &operations) override
    {
        return {};
    }
    void ContinueAbility(const std::string &deviceId,
        uint32_t versionCode) override {}
    void NotifyContinuationResult(int32_t result) override {}
    void DumpAbilityInfo(const std::vector<std::string> &params,
        std::vector<std::string> &info) override {}
    int CreateModalUIExtension(const Want &want) override { return 0; }
    void OnExecuteIntent(const Want &want) override {}
    void CallRequest() override {}
    void UpdateSessionToken(
        sptr<IRemoteObject> sessionToken) override {}
    void ScheduleCollaborate(const Want &want) override {}
    void ScheduleAbilityRequestFailure(const std::string &requestId,
        const AppExecFwk::ElementName &element,
        const std::string &message, int32_t resultCode) override {}
    void ScheduleAbilityRequestSuccess(const std::string &requestId,
        const AppExecFwk::ElementName &element) override {}
    void ScheduleAbilitiesRequestDone(const std::string &requestKey,
        int32_t resultCode) override {}
};

AbilityRequest CreateDataRequest()
{
    AbilityRequest request;
    request.appInfo.bundleName = "com.example.fuzzTest";
    request.abilityInfo.name = "DataAbility";
    request.abilityInfo.type = AbilityType::DATA;
    return request;
}
} // namespace

void FuzzStartLoading(FuzzedDataProvider &fdp)
{
    auto request = CreateDataRequest();
    auto record = std::make_shared<DataAbilityRecord>(request);
    // normal StartLoading (will try to load ability)
    record->StartLoading();
    // already started (ability_ non-null)
    auto ability = AbilityRecord::CreateAbilityRecord(request);
    if (ability) {
        record->ability_ = ability;
        record->StartLoading();
    }
    // wrong type
    AbilityRequest wrongReq;
    wrongReq.abilityInfo.type = AbilityType::PAGE;
    auto wrongRecord = std::make_shared<DataAbilityRecord>(wrongReq);
    wrongRecord->StartLoading();
}

void FuzzWaitAndGetScheduler(FuzzedDataProvider &fdp)
{
    auto request = CreateDataRequest();
    auto record = std::make_shared<DataAbilityRecord>(request);
    auto scheduler = sptr<IAbilityScheduler>(new FuzzAbilityScheduler());
    ffrt::mutex mutex;
    system_clock::duration timeout = 10ms;
    // null ability
    record->WaitForLoaded(mutex, timeout);
    record->GetScheduler();
    // set ability to ACTIVE state
    auto ability = AbilityRecord::CreateAbilityRecord(request);
    if (!ability) {
        return;
    }
    ability->SetAbilityState(ACTIVE);
    record->ability_ = ability;
    record->scheduler_ = scheduler;
    // already ACTIVE - should return immediately
    record->WaitForLoaded(mutex, timeout);
    // GetScheduler in ACTIVE state
    record->GetScheduler();
    // not ACTIVE state
    ability->SetAbilityState(INITIAL);
    record->GetScheduler();
}

void FuzzAttachAndTransition(FuzzedDataProvider &fdp)
{
    auto request = CreateDataRequest();
    auto record = std::make_shared<DataAbilityRecord>(request);
    auto scheduler = sptr<IAbilityScheduler>(new FuzzAbilityScheduler());
    // null scheduler
    record->Attach(nullptr);
    // no ability
    record->Attach(scheduler);
    // set up ability in INITIAL state
    auto ability = AbilityRecord::CreateAbilityRecord(request);
    if (!ability) {
        return;
    }
    record->ability_ = ability;
    // already attached (scheduler_ non-null)
    record->scheduler_ = scheduler;
    record->Attach(scheduler);
    // fresh record for normal attach
    auto record2 = std::make_shared<DataAbilityRecord>(request);
    record2->ability_ = ability;
    ability->SetAbilityState(INITIAL);
    record2->Attach(scheduler);
    // OnTransitionDone - null ability/scheduler
    auto record3 = std::make_shared<DataAbilityRecord>(request);
    record3->OnTransitionDone(AbilityLifeCycleState::ABILITY_STATE_ACTIVE);
    // wrong current state
    record2->OnTransitionDone(
        AbilityLifeCycleState::ABILITY_STATE_INACTIVE);
    // success path - set to ACTIVATING then transition to ACTIVE
    ability->SetAbilityState(ACTIVATING);
    record2->OnTransitionDone(
        AbilityLifeCycleState::ABILITY_STATE_ACTIVE);
}

void FuzzClientManagement(FuzzedDataProvider &fdp)
{
    auto request = CreateDataRequest();
    auto record = std::make_shared<DataAbilityRecord>(request);
    auto scheduler = sptr<IAbilityScheduler>(new FuzzAbilityScheduler());
    bool tryBind = fdp.ConsumeBool();
    bool isNotHap = fdp.ConsumeBool();
    // null client
    record->AddClient(nullptr, tryBind, isNotHap);
    // not attached
    sptr<IRemoteObject> client = new FuzzAbilityScheduler();
    record->AddClient(client, tryBind, isNotHap);
    // set up for ACTIVE state
    auto ability = AbilityRecord::CreateAbilityRecord(request);
    if (!ability) {
        return;
    }
    ability->SetAbilityState(ACTIVE);
    record->ability_ = ability;
    record->scheduler_ = scheduler;
    // AddClient in ACTIVE state
    record->AddClient(client, tryBind, isNotHap);
    // GetClientCount
    record->GetClientCount(client);
    record->GetClientCount(nullptr);
    // RemoveClient
    record->RemoveClient(client, isNotHap);
    record->RemoveClient(nullptr, isNotHap);
    // RemoveClients with filter
    auto clientRecord = AbilityRecord::CreateAbilityRecord(request);
    record->RemoveClients(clientRecord);
    record->RemoveClients(nullptr);
}

void FuzzKillAndDump(FuzzedDataProvider &fdp)
{
    auto request = CreateDataRequest();
    auto record = std::make_shared<DataAbilityRecord>(request);
    auto scheduler = sptr<IAbilityScheduler>(new FuzzAbilityScheduler());
    // KillBoundClientProcesses without setup
    record->KillBoundClientProcesses();
    // Dump without ability
    record->Dump();
    std::vector<std::string> info;
    record->Dump(info);
    // set up for ACTIVE state
    auto ability = AbilityRecord::CreateAbilityRecord(request);
    if (!ability) {
        return;
    }
    ability->SetAbilityState(ACTIVE);
    record->ability_ = ability;
    record->scheduler_ = scheduler;
    // AddClient then Kill
    sptr<IRemoteObject> client = new FuzzAbilityScheduler();
    record->AddClient(client, true, false);
    record->KillBoundClientProcesses();
    // Dump with ability and clients
    record->Dump();
    record->Dump(info);
}

void FuzzOtherApis(FuzzedDataProvider &fdp)
{
    auto request = CreateDataRequest();
    auto record = std::make_shared<DataAbilityRecord>(request);
    auto scheduler = sptr<IAbilityScheduler>(new FuzzAbilityScheduler());
    // GetToken without ability
    record->GetToken();
    // GetRequest
    record->GetRequest();
    // GetAbilityRecord without ability
    record->GetAbilityRecord();
    // set up ability
    auto ability = AbilityRecord::CreateAbilityRecord(request);
    if (!ability) {
        return;
    }
    ability->SetAbilityState(ACTIVE);
    record->ability_ = ability;
    record->scheduler_ = scheduler;
    record->GetToken();
    record->GetAbilityRecord();
    // OnSchedulerDied with empty clients
    wptr<IRemoteObject> remote;
    record->OnSchedulerDied(remote);
    // OnSchedulerDied with clients
    sptr<IRemoteObject> client = new FuzzAbilityScheduler();
    record->AddClient(client, false, true);
    record->OnSchedulerDied(client);
    // GetDiedCallerPid
    record->GetDiedCallerPid(nullptr);
    record->GetDiedCallerPid(client);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    auto apiCase = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_API_CASE);
    switch (apiCase) {
        case API_START_LOADING:
            FuzzStartLoading(fdp);
            break;
        case API_WAIT_AND_GET_SCHEDULER:
            FuzzWaitAndGetScheduler(fdp);
            break;
        case API_ATTACH_AND_TRANSITION:
            FuzzAttachAndTransition(fdp);
            break;
        case API_CLIENT_MANAGEMENT:
            FuzzClientManagement(fdp);
            break;
        case API_KILL_AND_DUMP:
            FuzzKillAndDump(fdp);
            break;
        case API_OTHER:
            FuzzOtherApis(fdp);
            break;
        case API_START_AND_ATTACH:
            FuzzStartLoading(fdp);
            FuzzAttachAndTransition(fdp);
            break;
        case API_CLIENT_AND_KILL:
            FuzzClientManagement(fdp);
            FuzzKillAndDump(fdp);
            break;
        default:
            break;
    }
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
