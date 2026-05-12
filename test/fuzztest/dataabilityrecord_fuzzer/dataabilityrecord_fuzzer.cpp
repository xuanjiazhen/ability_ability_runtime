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
#define protected public
#include "data_ability_record.h"
#include "data_ability_caller_recipient.h"
#undef protected
#undef private

#include "ability_fuzz_util.h"
#include "ability_record.h"

using namespace std::chrono;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t U32_AT_SIZE = 4;
constexpr int32_t MAX_CLIENTS = 10;
constexpr int32_t TIMEOUT_MS = 100;

// Upper bounds for ConsumeIntegralInRange switch cases
constexpr int32_t MAX_START_LOADING_CASE = 6;
constexpr int32_t MAX_WAIT_LOADED_CASE = 6;
constexpr int32_t MAX_GET_SCHEDULER_CASE = 5;
constexpr int32_t MAX_ATTACH_CASE = 6;
constexpr int32_t MAX_ON_TRANSITION_DONE_CASE = 7;
constexpr int32_t MAX_ADD_CLIENT_CASE = 8;
constexpr int32_t MAX_REMOVE_CLIENT_CASE = 8;
constexpr int32_t MAX_REMOVE_CLIENTS_CASE = 7;
constexpr int32_t MAX_GET_CLIENT_COUNT_CASE = 6;
constexpr int32_t MAX_KILL_BOUND_CLIENT_CASE = 5;
constexpr int32_t MAX_DUMP_CASE = 5;
constexpr int32_t MAX_ON_SCHEDULER_DIED_CASE = 5;
constexpr int32_t MAX_GET_DIED_CALLER_PID_CASE = 3;
constexpr int32_t MAX_OPERATION_TYPE = 13;
constexpr int32_t MAX_TRANSITION_STATE = 10;
constexpr int32_t REMOVE_CLIENT_LOOP_COUNT = 3;

class MockAbilityScheduler : public IRemoteStub<IAbilityScheduler> {
public:
    MockAbilityScheduler() = default;
    virtual ~MockAbilityScheduler() = default;

    int ScheduleAbilityTransaction(const Want &want,
        const LifeCycleStateInfo &stateInfo) override { return 0; }
    int ScheduleConnectAbility(const Want &want) override { return 0; }
    int ScheduleDisconnectAbility(const Want &want) override { return 0; }
    void ScheduleCommandAbility(const Want &want,
        bool restart, int visParam) override {}
    void ScheduleCommandAbilityWindow(const Want &want,
        const sptr<AAFwk::AbilityWindowInfo> &windowInfo,
        bool restart, int visParam) override {}
    int ScheduleSaveAbilityState(PacMap &stateData) override { return 0; }
    int ScheduleRestoreAbilityState(const PacMap &stateData) override { return 0; }
    int ScheduleNotifyExtensionBindStateInfo(int32_t bindState,
        const Want &want) override { return 0; }
    void DumpAbilityInfo(const std::vector<std::string> &params,
        std::vector<std::string> &info) override {}
    sptr<IRemoteObject> AsObject() override { return this; }
};

AbilityRequest CreateDataAbilityRequest(FuzzedDataProvider& fdp)
{
    AbilityRequest request;
    request.abilityInfo.type = AbilityType::DATA;
    request.abilityInfo.bundleName = fdp.ConsumeRandomLengthString(64);
    request.abilityInfo.name = fdp.ConsumeRandomLengthString(64);
    request.abilityInfo.moduleName = fdp.ConsumeRandomLengthString(64);
    request.abilityInfo.applicationInfo.bundleName =
        request.abilityInfo.bundleName;
    request.appInfo.bundleName = request.abilityInfo.bundleName;
    return request;
}

AbilityRequest CreateNonDataAbilityRequest()
{
    AbilityRequest request;
    request.abilityInfo.type = AbilityType::PAGE;
    request.abilityInfo.bundleName = "com.test.nondata";
    request.abilityInfo.name = "NonDataAbility";
    return request;
}

AbilityRequest CreateEmptyNameRequest()
{
    AbilityRequest request;
    request.abilityInfo.type = AbilityType::DATA;
    request.abilityInfo.bundleName = "";
    request.abilityInfo.name = "";
    return request;
}

// Helper to make a shared AbilityRecord from request fields
std::shared_ptr<AbilityRecord> MakeAbilityRecord(const AbilityRequest &request)
{
    return std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
}

// ---- FuzzStartLoading case helpers ----

void FuzzStartLoadingCase0(FuzzedDataProvider& /*fdp*/)
{
    AbilityRequest request = CreateNonDataAbilityRequest();
    auto record = std::make_shared<DataAbilityRecord>(request);
    int ret = record->StartLoading();
    (void)ret;
}

void FuzzStartLoadingCase1(FuzzedDataProvider& fdp)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    int ret = record->StartLoading();
    (void)ret;
}

void FuzzStartLoadingCase2(FuzzedDataProvider& fdp)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    int ret = record->StartLoading();
    (void)ret;
}

void FuzzStartLoadingCase3(FuzzedDataProvider& fdp)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    int ret = record->StartLoading();
    (void)ret;
}

void FuzzStartLoadingCase4(FuzzedDataProvider& fdp)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = nullptr;
    int ret = record->StartLoading();
    (void)ret;
}

void FuzzStartLoadingCase5(FuzzedDataProvider& fdp)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    int ret1 = record->StartLoading();
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    int ret2 = record->StartLoading();
    (void)ret1;
    (void)ret2;
}

void FuzzStartLoadingCase6(FuzzedDataProvider& fdp)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    int ret = record->StartLoading();
    (void)ret;
}

void FuzzStartLoading(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(
        0, MAX_START_LOADING_CASE);

    switch (testCase) {
        case 0: FuzzStartLoadingCase0(fdp); break;
        case 1: FuzzStartLoadingCase1(fdp); break;
        case 2: FuzzStartLoadingCase2(fdp); break;
        case 3: FuzzStartLoadingCase3(fdp); break;
        case 4: FuzzStartLoadingCase4(fdp); break;
        case 5: FuzzStartLoadingCase5(fdp); break;
        case 6: FuzzStartLoadingCase6(fdp); break;
        default: break;
    }
}

// ---- FuzzWaitForLoaded case helpers ----

void FuzzWaitForLoadedCase0(FuzzedDataProvider& fdp,
    ffrt::mutex &mutex, system_clock::duration &timeout)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = nullptr;
    int ret = record->WaitForLoaded(mutex, timeout);
    (void)ret;
}

void FuzzWaitForLoadedCase1(FuzzedDataProvider& fdp,
    ffrt::mutex &mutex, system_clock::duration &timeout)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->ability_->SetAbilityState(ACTIVE);
    int ret = record->WaitForLoaded(mutex, timeout);
    (void)ret;
}

void FuzzWaitForLoadedCase2(FuzzedDataProvider& fdp,
    ffrt::mutex &mutex, system_clock::duration &timeout)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->ability_->SetAbilityState(INITIAL);
    int ret = record->WaitForLoaded(mutex, timeout);
    (void)ret;
}

void FuzzWaitForLoadedCase3(FuzzedDataProvider& fdp,
    ffrt::mutex &mutex, system_clock::duration &timeout)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->ability_->SetAbilityState(ACTIVATING);
    int ret = record->WaitForLoaded(mutex, timeout);
    (void)ret;
}

void FuzzWaitForLoadedCase4(FuzzedDataProvider& fdp,
    ffrt::mutex &mutex, system_clock::duration &timeout)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = nullptr;
    record->ability_->SetAbilityState(ACTIVE);
    int ret = record->WaitForLoaded(mutex, timeout);
    (void)ret;
}

void FuzzWaitForLoadedCase5(FuzzedDataProvider& fdp,
    ffrt::mutex &mutex, system_clock::duration &timeout)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    record->ability_->SetAbilityState(INITIAL);
    int ret = record->WaitForLoaded(mutex, timeout);
    (void)ret;
}

void FuzzWaitForLoadedCase6(FuzzedDataProvider& fdp,
    ffrt::mutex &mutex, system_clock::duration &timeout)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    record->ability_->SetAbilityState(ACTIVE);
    int ret = record->WaitForLoaded(mutex, timeout);
    (void)ret;
}

void FuzzWaitForLoaded(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(
        0, MAX_WAIT_LOADED_CASE);
    ffrt::mutex mutex;
    system_clock::duration timeout = milliseconds(TIMEOUT_MS);

    switch (testCase) {
        case 0: FuzzWaitForLoadedCase0(fdp, mutex, timeout); break;
        case 1: FuzzWaitForLoadedCase1(fdp, mutex, timeout); break;
        case 2: FuzzWaitForLoadedCase2(fdp, mutex, timeout); break;
        case 3: FuzzWaitForLoadedCase3(fdp, mutex, timeout); break;
        case 4: FuzzWaitForLoadedCase4(fdp, mutex, timeout); break;
        case 5: FuzzWaitForLoadedCase5(fdp, mutex, timeout); break;
        case 6: FuzzWaitForLoadedCase6(fdp, mutex, timeout); break;
        default: break;
    }
}

// ---- FuzzGetScheduler case helpers ----

void FuzzGetSchedulerCase0(FuzzedDataProvider& fdp)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = nullptr;
    auto scheduler = record->GetScheduler();
    (void)scheduler;
}

void FuzzGetSchedulerCase1(FuzzedDataProvider& fdp)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = nullptr;
    auto scheduler = record->GetScheduler();
    (void)scheduler;
}

void FuzzGetSchedulerCase2(FuzzedDataProvider& fdp)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    record->ability_->SetAbilityState(INITIAL);
    auto scheduler = record->GetScheduler();
    (void)scheduler;
}

void FuzzGetSchedulerCase3(FuzzedDataProvider& fdp)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    record->ability_->SetAbilityState(ACTIVATING);
    auto scheduler = record->GetScheduler();
    (void)scheduler;
}

void FuzzGetSchedulerCase4(FuzzedDataProvider& fdp)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    record->ability_->SetAbilityState(ACTIVE);
    auto scheduler = record->GetScheduler();
    (void)scheduler;
}

void FuzzGetSchedulerCase5(FuzzedDataProvider& fdp)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    auto scheduler = record->GetScheduler();
    (void)scheduler;
}

void FuzzGetScheduler(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(
        0, MAX_GET_SCHEDULER_CASE);

    switch (testCase) {
        case 0: FuzzGetSchedulerCase0(fdp); break;
        case 1: FuzzGetSchedulerCase1(fdp); break;
        case 2: FuzzGetSchedulerCase2(fdp); break;
        case 3: FuzzGetSchedulerCase3(fdp); break;
        case 4: FuzzGetSchedulerCase4(fdp); break;
        case 5: FuzzGetSchedulerCase5(fdp); break;
        default: break;
    }
}

// ---- FuzzAttach case helpers ----

void FuzzAttachCase0(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &/*scheduler*/)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    int ret = record->Attach(nullptr);
    (void)ret;
}

void FuzzAttachCase1(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = nullptr;
    int ret = record->Attach(scheduler);
    (void)ret;
}

void FuzzAttachCase2(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    int ret = record->Attach(scheduler);
    (void)ret;
}

void FuzzAttachCase3(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->ability_->SetAbilityState(ACTIVATING);
    int ret = record->Attach(scheduler);
    (void)ret;
}

void FuzzAttachCase4(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->ability_->SetAbilityState(ACTIVE);
    int ret = record->Attach(scheduler);
    (void)ret;
}

void FuzzAttachCase5(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->ability_->SetAbilityState(INITIAL);
    int ret = record->Attach(scheduler);
    (void)ret;
}

void FuzzAttachCase6(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->ability_->SetAbilityState(INITIAL);
    int ret1 = record->Attach(scheduler);
    sptr<MockAbilityScheduler> scheduler2 =
        sptr<MockAbilityScheduler>::MakeSptr();
    int ret2 = record->Attach(scheduler2);
    (void)ret1;
    (void)ret2;
}

void FuzzAttach(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(
        0, MAX_ATTACH_CASE);
    sptr<MockAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();

    switch (testCase) {
        case 0: FuzzAttachCase0(fdp, scheduler); break;
        case 1: FuzzAttachCase1(fdp, scheduler); break;
        case 2: FuzzAttachCase2(fdp, scheduler); break;
        case 3: FuzzAttachCase3(fdp, scheduler); break;
        case 4: FuzzAttachCase4(fdp, scheduler); break;
        case 5: FuzzAttachCase5(fdp, scheduler); break;
        case 6: FuzzAttachCase6(fdp, scheduler); break;
        default: break;
    }
}

// ---- FuzzOnTransitionDone case helpers ----

void FuzzOnTransitionDoneCase0(FuzzedDataProvider& fdp, int state)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = nullptr;
    int ret = record->OnTransitionDone(state);
    (void)ret;
}

void FuzzOnTransitionDoneCase1(FuzzedDataProvider& fdp, int state)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = nullptr;
    int ret = record->OnTransitionDone(state);
    (void)ret;
}

void FuzzOnTransitionDoneCase2(FuzzedDataProvider& fdp, int state)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    record->ability_->SetAbilityState(INITIAL);
    int ret = record->OnTransitionDone(state);
    (void)ret;
}

void FuzzOnTransitionDoneCase3(FuzzedDataProvider& /*fdp*/, int /*state*/)
{
    AbilityRequest request = CreateNonDataAbilityRequest();
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    record->ability_->SetAbilityState(ACTIVATING);
    int ret = record->OnTransitionDone(
        AbilityLifeCycleState::ABILITY_STATE_INACTIVE);
    (void)ret;
}

void FuzzOnTransitionDoneCase4(FuzzedDataProvider& /*fdp*/, int /*state*/)
{
    AbilityRequest request = CreateNonDataAbilityRequest();
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    record->ability_->SetAbilityState(ACTIVATING);
    int ret = record->OnTransitionDone(
        AbilityLifeCycleState::ABILITY_STATE_FOREGROUND);
    (void)ret;
}

void FuzzOnTransitionDoneCase5(FuzzedDataProvider& /*fdp*/, int /*state*/)
{
    AbilityRequest request = CreateNonDataAbilityRequest();
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    record->ability_->SetAbilityState(ACTIVATING);
    int ret = record->OnTransitionDone(
        AbilityLifeCycleState::ABILITY_STATE_BACKGROUND);
    (void)ret;
}

void FuzzOnTransitionDoneCase6(FuzzedDataProvider& /*fdp*/, int /*state*/)
{
    AbilityRequest request = CreateNonDataAbilityRequest();
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    record->ability_->SetAbilityState(ACTIVATING);
    int ret = record->OnTransitionDone(
        AbilityLifeCycleState::ABILITY_STATE_ACTIVE);
    (void)ret;
}

void FuzzOnTransitionDoneCase7(FuzzedDataProvider& fdp, int state)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    record->ability_->SetAbilityState(ACTIVE);
    int ret = record->OnTransitionDone(state);
    (void)ret;
}

void FuzzOnTransitionDone(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(
        0, MAX_ON_TRANSITION_DONE_CASE);
    int state = fdp.ConsumeIntegralInRange<int>(0, MAX_TRANSITION_STATE);

    switch (testCase) {
        case 0: FuzzOnTransitionDoneCase0(fdp, state); break;
        case 1: FuzzOnTransitionDoneCase1(fdp, state); break;
        case 2: FuzzOnTransitionDoneCase2(fdp, state); break;
        case 3: FuzzOnTransitionDoneCase3(fdp, state); break;
        case 4: FuzzOnTransitionDoneCase4(fdp, state); break;
        case 5: FuzzOnTransitionDoneCase5(fdp, state); break;
        case 6: FuzzOnTransitionDoneCase6(fdp, state); break;
        case 7: FuzzOnTransitionDoneCase7(fdp, state); break;
        default: break;
    }
}

// ---- FuzzAddClient case helpers ----

void FuzzAddClientCase0(FuzzedDataProvider& fdp, bool tryBind, bool isNotHap)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    int ret = record->AddClient(nullptr, tryBind, isNotHap);
    (void)ret;
}

void FuzzAddClientCase1(FuzzedDataProvider& fdp,
    sptr<IRemoteObject> &client, bool tryBind, bool isNotHap)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = nullptr;
    int ret = record->AddClient(client, tryBind, isNotHap);
    (void)ret;
}

void FuzzAddClientCase2(FuzzedDataProvider& fdp,
    sptr<IRemoteObject> &client, bool tryBind, bool isNotHap)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = nullptr;
    int ret = record->AddClient(client, tryBind, isNotHap);
    (void)ret;
}

void FuzzAddClientCase3(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler,
    sptr<IRemoteObject> &client, bool tryBind, bool isNotHap)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(INITIAL);
    int ret = record->AddClient(client, tryBind, isNotHap);
    (void)ret;
}

void FuzzAddClientCase4(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler,
    sptr<IRemoteObject> &client, bool tryBind, bool isNotHap)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVATING);
    int ret = record->AddClient(client, tryBind, isNotHap);
    (void)ret;
}

void FuzzAddClientCase5(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler,
    sptr<IRemoteObject> &client, bool tryBind, bool isNotHap)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    int ret = record->AddClient(client, tryBind, isNotHap);
    (void)ret;
}

void FuzzAddClientCase6(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler,
    sptr<IRemoteObject> &client, bool tryBind, bool isNotHap)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    record->callerDeathRecipient_ =
        sptr<DataAbilityCallerRecipient>::MakeSptr(
            [](const wptr<IRemoteObject>&) {});
    int ret = record->AddClient(client, tryBind, isNotHap);
    (void)ret;
}

void FuzzAddClientCase7(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> client2 = sptr<MockAbilityScheduler>::MakeSptr();
    int ret1 = record->AddClient(client1, true, false);
    int ret2 = record->AddClient(client2, false, true);
    int ret3 = record->AddClient(client1, true, false);
    (void)ret1;
    (void)ret2;
    (void)ret3;
}

void FuzzAddClientCase8(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        sptr<IRemoteObject> clientLoop =
            sptr<MockAbilityScheduler>::MakeSptr();
        int ret = record->AddClient(
            clientLoop, fdp.ConsumeBool(), fdp.ConsumeBool());
        (void)ret;
    }
}

void FuzzAddClient(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(
        0, MAX_ADD_CLIENT_CASE);
    sptr<MockAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> client = sptr<MockAbilityScheduler>::MakeSptr();
    bool tryBind = fdp.ConsumeBool();
    bool isNotHap = fdp.ConsumeBool();

    switch (testCase) {
        case 0: FuzzAddClientCase0(fdp, tryBind, isNotHap); break;
        case 1: FuzzAddClientCase1(fdp, client, tryBind, isNotHap); break;
        case 2: FuzzAddClientCase2(fdp, client, tryBind, isNotHap); break;
        case 3:
            FuzzAddClientCase3(fdp, scheduler, client, tryBind, isNotHap);
            break;
        case 4:
            FuzzAddClientCase4(fdp, scheduler, client, tryBind, isNotHap);
            break;
        case 5:
            FuzzAddClientCase5(fdp, scheduler, client, tryBind, isNotHap);
            break;
        case 6:
            FuzzAddClientCase6(fdp, scheduler, client, tryBind, isNotHap);
            break;
        case 7: FuzzAddClientCase7(fdp, scheduler); break;
        case 8: FuzzAddClientCase8(fdp, scheduler); break;
        default: break;
    }
}

// ---- FuzzRemoveClient case helpers ----

void FuzzRemoveClientCase0(FuzzedDataProvider& fdp, bool isNotHap)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    int ret = record->RemoveClient(nullptr, isNotHap);
    (void)ret;
}

void FuzzRemoveClientCase1(FuzzedDataProvider& fdp,
    sptr<IRemoteObject> &client, bool isNotHap)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = nullptr;
    int ret = record->RemoveClient(client, isNotHap);
    (void)ret;
}

void FuzzRemoveClientCase2(FuzzedDataProvider& fdp,
    sptr<IRemoteObject> &client, bool isNotHap)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = nullptr;
    int ret = record->RemoveClient(client, isNotHap);
    (void)ret;
}

void FuzzRemoveClientCase3(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler,
    sptr<IRemoteObject> &client, bool isNotHap)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(INITIAL);
    int ret = record->RemoveClient(client, isNotHap);
    (void)ret;
}

void FuzzRemoveClientCase4(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler,
    sptr<IRemoteObject> &client, bool isNotHap)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    record->clients_.clear();
    int ret = record->RemoveClient(client, isNotHap);
    (void)ret;
}

void FuzzRemoveClientCase5(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler,
    sptr<IRemoteObject> &client, bool isNotHap)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
    record->AddClient(client1, true, false);
    int ret = record->RemoveClient(client, isNotHap);
    (void)ret;
}

void FuzzRemoveClientCase6(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler, bool isNotHap)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
    record->AddClient(client1, true, false);
    int ret = record->RemoveClient(client1, isNotHap);
    (void)ret;
}

void FuzzRemoveClientCase7(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> client2 = sptr<MockAbilityScheduler>::MakeSptr();
    record->AddClient(client1, true, false);
    record->AddClient(client2, false, true);
    int ret1 = record->RemoveClient(client1, false);
    int ret2 = record->RemoveClient(client2, true);
    (void)ret1;
    (void)ret2;
}

void FuzzRemoveClientCase8(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    for (int i = 0; i < REMOVE_CLIENT_LOOP_COUNT; i++) {
        sptr<IRemoteObject> clientLoop =
            sptr<MockAbilityScheduler>::MakeSptr();
        record->AddClient(clientLoop, true, false);
    }
    auto it = record->clients_.begin();
    while (it != record->clients_.end()) {
        sptr<IRemoteObject> clientLoop = it->client;
        int ret = record->RemoveClient(clientLoop, it->isNotHap);
        (void)ret;
        it = record->clients_.begin();
    }
}

void FuzzRemoveClient(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(
        0, MAX_REMOVE_CLIENT_CASE);
    sptr<MockAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> client = sptr<MockAbilityScheduler>::MakeSptr();
    bool isNotHap = fdp.ConsumeBool();

    switch (testCase) {
        case 0: FuzzRemoveClientCase0(fdp, isNotHap); break;
        case 1: FuzzRemoveClientCase1(fdp, client, isNotHap); break;
        case 2: FuzzRemoveClientCase2(fdp, client, isNotHap); break;
        case 3:
            FuzzRemoveClientCase3(fdp, scheduler, client, isNotHap);
            break;
        case 4:
            FuzzRemoveClientCase4(fdp, scheduler, client, isNotHap);
            break;
        case 5:
            FuzzRemoveClientCase5(fdp, scheduler, client, isNotHap);
            break;
        case 6: FuzzRemoveClientCase6(fdp, scheduler, isNotHap); break;
        case 7: FuzzRemoveClientCase7(fdp, scheduler); break;
        case 8: FuzzRemoveClientCase8(fdp, scheduler); break;
        default: break;
    }
}

// ---- FuzzRemoveClients case helpers ----

void FuzzRemoveClientsCase0(FuzzedDataProvider& fdp)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = nullptr;
    int ret = record->RemoveClients(nullptr);
    (void)ret;
}

void FuzzRemoveClientsCase1(FuzzedDataProvider& fdp)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = nullptr;
    int ret = record->RemoveClients(nullptr);
    (void)ret;
}

void FuzzRemoveClientsCase2(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(INITIAL);
    int ret = record->RemoveClients(nullptr);
    (void)ret;
}

void FuzzRemoveClientsCase3(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    record->clients_.clear();
    int ret = record->RemoveClients(nullptr);
    (void)ret;
}

void FuzzRemoveClientsCase4(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
    record->AddClient(client1, true, false);
    int ret = record->RemoveClients(nullptr);
    (void)ret;
}

void FuzzRemoveClientsCase5(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> client2 = sptr<MockAbilityScheduler>::MakeSptr();
    record->AddClient(client1, true, false);
    record->AddClient(client2, false, true);
    AbilityRequest clientRequest;
    clientRequest.abilityInfo.type = AbilityType::PAGE;
    clientRequest.abilityInfo.bundleName = "com.test.client";
    clientRequest.abilityInfo.name = "ClientAbility";
    auto clientRecord = std::make_shared<AbilityRecord>(
        clientRequest.want, clientRequest.abilityInfo,
        clientRequest.appInfo);
    int ret = record->RemoveClients(clientRecord);
    (void)ret;
}

void FuzzRemoveClientsCase6(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> client2 = sptr<MockAbilityScheduler>::MakeSptr();
    record->AddClient(client1, true, false);
    record->AddClient(client2, false, true);
    auto clientRecord = Token::GetAbilityRecordByToken(client1);
    int ret = record->RemoveClients(clientRecord);
    (void)ret;
}

void FuzzRemoveClientsCase7(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        sptr<IRemoteObject> clientLoop =
            sptr<MockAbilityScheduler>::MakeSptr();
        record->AddClient(
            clientLoop, fdp.ConsumeBool(), fdp.ConsumeBool());
    }
    int ret = record->RemoveClients(nullptr);
    (void)ret;
}

void FuzzRemoveClients(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(
        0, MAX_REMOVE_CLIENTS_CASE);
    sptr<MockAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();

    switch (testCase) {
        case 0: FuzzRemoveClientsCase0(fdp); break;
        case 1: FuzzRemoveClientsCase1(fdp); break;
        case 2: FuzzRemoveClientsCase2(fdp, scheduler); break;
        case 3: FuzzRemoveClientsCase3(fdp, scheduler); break;
        case 4: FuzzRemoveClientsCase4(fdp, scheduler); break;
        case 5: FuzzRemoveClientsCase5(fdp, scheduler); break;
        case 6: FuzzRemoveClientsCase6(fdp, scheduler); break;
        case 7: FuzzRemoveClientsCase7(fdp, scheduler); break;
        default: break;
    }
}

// ---- FuzzGetClientCount case helpers ----

void FuzzGetClientCountCase0(FuzzedDataProvider& fdp,
    sptr<IRemoteObject> &client)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = nullptr;
    size_t count = record->GetClientCount(client);
    (void)count;
}

void FuzzGetClientCountCase1(FuzzedDataProvider& fdp,
    sptr<IRemoteObject> &client)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = nullptr;
    size_t count = record->GetClientCount(client);
    (void)count;
}

void FuzzGetClientCountCase2(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler,
    sptr<IRemoteObject> &client)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(INITIAL);
    size_t count = record->GetClientCount(client);
    (void)count;
}

void FuzzGetClientCountCase3(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    size_t count = record->GetClientCount(nullptr);
    (void)count;
}

void FuzzGetClientCountCase4(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> client2 = sptr<MockAbilityScheduler>::MakeSptr();
    record->AddClient(client1, true, false);
    record->AddClient(client1, true, false);
    record->AddClient(client2, false, true);
    size_t count1 = record->GetClientCount(client1);
    size_t count2 = record->GetClientCount(client2);
    size_t count3 = record->GetClientCount(nullptr);
    (void)count1;
    (void)count2;
    (void)count3;
}

void FuzzGetClientCountCase5(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler,
    sptr<IRemoteObject> &client)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    record->AddClient(client, true, false);
    size_t count = record->GetClientCount(client);
    (void)count;
}

void FuzzGetClientCountCase6(FuzzedDataProvider& fdp)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    auto count = record->GetClientCount();
    (void)count;
}

void FuzzGetClientCount(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(
        0, MAX_GET_CLIENT_COUNT_CASE);
    sptr<MockAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> client = sptr<MockAbilityScheduler>::MakeSptr();

    switch (testCase) {
        case 0: FuzzGetClientCountCase0(fdp, client); break;
        case 1: FuzzGetClientCountCase1(fdp, client); break;
        case 2: FuzzGetClientCountCase2(fdp, scheduler, client); break;
        case 3: FuzzGetClientCountCase3(fdp, scheduler); break;
        case 4: FuzzGetClientCountCase4(fdp, scheduler); break;
        case 5:
            FuzzGetClientCountCase5(fdp, scheduler, client);
            break;
        case 6: FuzzGetClientCountCase6(fdp); break;
        default: break;
    }
}

// ---- FuzzKillBoundClientProcesses case helpers ----

void FuzzKillBoundClientCase0(FuzzedDataProvider& fdp)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = nullptr;
    int ret = record->KillBoundClientProcesses();
    (void)ret;
}

void FuzzKillBoundClientCase1(FuzzedDataProvider& fdp)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = nullptr;
    int ret = record->KillBoundClientProcesses();
    (void)ret;
}

void FuzzKillBoundClientCase2(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(INITIAL);
    int ret = record->KillBoundClientProcesses();
    (void)ret;
}

void FuzzKillBoundClientCase3(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> client2 = sptr<MockAbilityScheduler>::MakeSptr();
    record->AddClient(client1, true, false);
    record->AddClient(client2, false, true);
    int ret = record->KillBoundClientProcesses();
    (void)ret;
}

void FuzzKillBoundClientCase4(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    int ret = record->KillBoundClientProcesses();
    (void)ret;
}

void FuzzKillBoundClientCase5(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        sptr<IRemoteObject> clientLoop =
            sptr<MockAbilityScheduler>::MakeSptr();
        record->AddClient(
            clientLoop, fdp.ConsumeBool(), fdp.ConsumeBool());
    }
    int ret = record->KillBoundClientProcesses();
    (void)ret;
}

void FuzzKillBoundClientProcesses(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(
        0, MAX_KILL_BOUND_CLIENT_CASE);
    sptr<MockAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();

    switch (testCase) {
        case 0: FuzzKillBoundClientCase0(fdp); break;
        case 1: FuzzKillBoundClientCase1(fdp); break;
        case 2: FuzzKillBoundClientCase2(fdp, scheduler); break;
        case 3: FuzzKillBoundClientCase3(fdp, scheduler); break;
        case 4: FuzzKillBoundClientCase4(fdp, scheduler); break;
        case 5: FuzzKillBoundClientCase5(fdp, scheduler); break;
        default: break;
    }
}

// ---- FuzzDump case helpers ----

void FuzzDumpCase0(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = nullptr;
    record->Dump();
    std::vector<std::string> info;
    record->Dump(info);
}

void FuzzDumpCase1(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    record->Dump();
}

void FuzzDumpCase2(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> client2 = sptr<MockAbilityScheduler>::MakeSptr();
    record->AddClient(client1, true, false);
    record->AddClient(client2, false, true);
    std::vector<std::string> info;
    record->Dump(info);
}

void FuzzDumpCase3(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        sptr<IRemoteObject> clientLoop =
            sptr<MockAbilityScheduler>::MakeSptr();
        record->AddClient(
            clientLoop, fdp.ConsumeBool(), fdp.ConsumeBool());
    }
    std::vector<std::string> info;
    record->Dump(info);
}

void FuzzDumpCase4(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    std::vector<std::string> info;
    record->Dump(info);
}

void FuzzDumpCase5(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &/*scheduler*/)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->Dump();
}

void FuzzDump(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(
        0, MAX_DUMP_CASE);
    sptr<MockAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();

    switch (testCase) {
        case 0: FuzzDumpCase0(fdp, scheduler); break;
        case 1: FuzzDumpCase1(fdp, scheduler); break;
        case 2: FuzzDumpCase2(fdp, scheduler); break;
        case 3: FuzzDumpCase3(fdp, scheduler); break;
        case 4: FuzzDumpCase4(fdp, scheduler); break;
        case 5: FuzzDumpCase5(fdp, scheduler); break;
        default: break;
    }
}

// ---- FuzzOnSchedulerDied case helpers ----

void FuzzOnSchedulerDiedCase0(FuzzedDataProvider& fdp,
    wptr<IRemoteObject> &remote)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = nullptr;
    record->OnSchedulerDied(remote);
}

void FuzzOnSchedulerDiedCase1(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler,
    wptr<IRemoteObject> &remote)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    record->clients_.clear();
    record->OnSchedulerDied(remote);
}

void FuzzOnSchedulerDiedCase2(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler,
    wptr<IRemoteObject> &remote)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> client2 = sptr<MockAbilityScheduler>::MakeSptr();
    record->AddClient(client1, true, false);
    record->AddClient(client2, false, true);
    record->OnSchedulerDied(remote);
}

void FuzzOnSchedulerDiedCase3(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
    record->AddClient(client1, false, true);
    wptr<IRemoteObject> remoteClient = client1;
    record->OnSchedulerDied(remoteClient);
}

void FuzzOnSchedulerDiedCase4(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> client2 = sptr<MockAbilityScheduler>::MakeSptr();
    record->AddClient(client1, true, false);
    record->AddClient(client2, false, true);
    wptr<IRemoteObject> remoteNull = nullptr;
    record->OnSchedulerDied(remoteNull);
}

void FuzzOnSchedulerDiedCase5(FuzzedDataProvider& fdp,
    wptr<IRemoteObject> &remote)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->OnSchedulerDied(remote);
}

void FuzzOnSchedulerDied(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(
        0, MAX_ON_SCHEDULER_DIED_CASE);
    sptr<MockAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();
    wptr<IRemoteObject> remote = sptr<MockAbilityScheduler>::MakeSptr();

    switch (testCase) {
        case 0: FuzzOnSchedulerDiedCase0(fdp, remote); break;
        case 1: FuzzOnSchedulerDiedCase1(fdp, scheduler, remote); break;
        case 2: FuzzOnSchedulerDiedCase2(fdp, scheduler, remote); break;
        case 3: FuzzOnSchedulerDiedCase3(fdp, scheduler); break;
        case 4: FuzzOnSchedulerDiedCase4(fdp, scheduler); break;
        case 5: FuzzOnSchedulerDiedCase5(fdp, remote); break;
        default: break;
    }
}

// ---- FuzzGetDiedCallerPid case helpers ----

void FuzzGetDiedCallerPidCase0(FuzzedDataProvider& fdp)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    int32_t pid = record->GetDiedCallerPid(nullptr);
    (void)pid;
}

void FuzzGetDiedCallerPidCase1(FuzzedDataProvider& fdp)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    sptr<IRemoteObject> remote = sptr<MockAbilityScheduler>::MakeSptr();
    int32_t pid = record->GetDiedCallerPid(remote);
    (void)pid;
}

void FuzzGetDiedCallerPidCase2(FuzzedDataProvider& fdp,
    sptr<MockAbilityScheduler> &scheduler)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);
    sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
    record->AddClient(client1, true, false);
    int32_t pid = record->GetDiedCallerPid(client1);
    (void)pid;
}

void FuzzGetDiedCallerPidCase3(FuzzedDataProvider& fdp)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> client2 = sptr<MockAbilityScheduler>::MakeSptr();
    record->AddClient(client1, true, false);
    record->AddClient(client2, false, true);
    int32_t pid = record->GetDiedCallerPid(client2);
    (void)pid;
}

void FuzzGetDiedCallerPid(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(
        0, MAX_GET_DIED_CALLER_PID_CASE);
    sptr<MockAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();

    switch (testCase) {
        case 0: FuzzGetDiedCallerPidCase0(fdp); break;
        case 1: FuzzGetDiedCallerPidCase1(fdp); break;
        case 2: FuzzGetDiedCallerPidCase2(fdp, scheduler); break;
        case 3: FuzzGetDiedCallerPidCase3(fdp); break;
        default: break;
    }
}

void FuzzGetters(FuzzedDataProvider& fdp)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    sptr<MockAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();

    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = MakeAbilityRecord(request);
    record->scheduler_ = scheduler;
    record->ability_->SetAbilityState(ACTIVE);

    const AbilityRequest& req = record->GetRequest();
    auto abilityRecord = record->GetAbilityRecord();
    auto token = record->GetToken();

    (void)req;
    (void)abilityRecord;
    (void)token;

    auto record2 = std::make_shared<DataAbilityRecord>(request);
    record2->ability_ = nullptr;
    auto token2 = record2->GetToken();
    (void)token2;
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < U32_AT_SIZE) {
        return false;
    }

    FuzzedDataProvider fdp(data, size);
    int32_t operationType = fdp.ConsumeIntegralInRange<int32_t>(
        0, MAX_OPERATION_TYPE);

    switch (operationType) {
        case 0: FuzzStartLoading(fdp); break;
        case 1: FuzzWaitForLoaded(fdp); break;
        case 2: FuzzGetScheduler(fdp); break;
        case 3: FuzzAttach(fdp); break;
        case 4: FuzzOnTransitionDone(fdp); break;
        case 5: FuzzAddClient(fdp); break;
        case 6: FuzzRemoveClient(fdp); break;
        case 7: FuzzRemoveClients(fdp); break;
        case 8: FuzzGetClientCount(fdp); break;
        case 9: FuzzKillBoundClientProcesses(fdp); break;
        case 10: FuzzDump(fdp); break;
        case 11: FuzzOnSchedulerDied(fdp); break;
        case 12: FuzzGetDiedCallerPid(fdp); break;
        case 13: FuzzGetters(fdp); break;
        default: break;
    }

    return true;
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
