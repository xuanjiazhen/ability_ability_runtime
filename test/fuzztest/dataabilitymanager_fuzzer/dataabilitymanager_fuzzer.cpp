/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "dataabilitymanager_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <cstring>
#include <string>
#include <vector>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#define protected public
#include "data_ability_manager.h"
#include "data_ability_record.h"
#undef protected
#undef private

#include "ability_fuzz_util.h"
#include "ability_record.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t STRING_MAX_LENGTH = 128;
constexpr int32_t MAX_RECORDS = 10;
constexpr int32_t INT_MAX_VAL = 2147483647;

// Switch case upper bounds for ConsumeIntegralInRange
constexpr int32_t MAX_ACQUIRE_CASE = 9;
constexpr int32_t MAX_RELEASE_CASE = 9;
constexpr int32_t MAX_CONTAINS_CASE = 4;
constexpr int32_t MAX_ATTACH_CASE = 6;
constexpr int32_t MAX_TRANSITION_CASE = 7;
constexpr int32_t MAX_DIED_CASE = 5;
constexpr int32_t MAX_APP_STATE_CASE = 5;
constexpr int32_t MAX_RECORD_BY_ID_CASE = 4;
constexpr int32_t MAX_RECORD_BY_TOKEN_CASE = 5;
constexpr int32_t MAX_RECORD_BY_SCHED_CASE = 4;
constexpr int32_t MAX_DUMP_CASE = 6;
constexpr int32_t MAX_RUNNING_INFO_CASE = 3;
constexpr int32_t MAX_REPORT_CASE = 3;
constexpr int32_t MAX_OPERATION_CASE = 15;
constexpr int32_t APP_INDEX_SENTINEL = 999;
constexpr int32_t LOOP_COUNT_SMALL = 3;
constexpr int32_t STATE_RANGE_MAX = 10;

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
    int ScheduleNotifyExtensionBindStateInfo(
        int32_t bindState, const Want &want) override { return 0; }
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
    request.abilityInfo.applicationInfo.uid =
        fdp.ConsumeIntegral<int32_t>();
    request.appInfo.bundleName = request.abilityInfo.bundleName;
    request.appInfo.name = fdp.ConsumeRandomLengthString(64);
    return request;
}

AbilityRequest CreateNonDataAbilityRequest(FuzzedDataProvider& fdp)
{
    AbilityRequest request;
    request.abilityInfo.type = AbilityType::PAGE;
    request.abilityInfo.bundleName = fdp.ConsumeRandomLengthString(64);
    request.abilityInfo.name = fdp.ConsumeRandomLengthString(64);
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

sptr<Token> CreateMockToken(const AbilityRequest& request)
{
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(request);
    if (abilityRecord) {
        return abilityRecord->GetToken();
    }
    return nullptr;
}

std::string MakeDataAbilityName(const AbilityRequest& request)
{
    return request.abilityInfo.bundleName + "." + request.abilityInfo.name;
}

// ---------- FuzzAcquire helper functions ----------

void FuzzAcquireCase0(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateNonDataAbilityRequest(fdp);
    bool tryBind = fdp.ConsumeBool();
    bool isNotHap = fdp.ConsumeBool();
    sptr<IRemoteObject> client = nullptr;
    auto scheduler = manager.Acquire(request, tryBind, client, isNotHap);
    (void)scheduler;
}

void FuzzAcquireCase1(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateEmptyNameRequest();
    bool tryBind = fdp.ConsumeBool();
    bool isNotHap = fdp.ConsumeBool();
    sptr<IRemoteObject> client = nullptr;
    auto scheduler = manager.Acquire(request, tryBind, client, isNotHap);
    (void)scheduler;
}

void FuzzAcquireCase2(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    bool tryBind = fdp.ConsumeBool();
    bool isNotHap = fdp.ConsumeBool();
    sptr<IRemoteObject> client = nullptr;
    auto scheduler = manager.Acquire(request, tryBind, client, isNotHap);
    (void)scheduler;
}

void FuzzAcquireCase3(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    bool tryBind = fdp.ConsumeBool();
    sptr<IRemoteObject> client = sptr<MockAbilityScheduler>::MakeSptr();
    auto scheduler = manager.Acquire(request, tryBind, client, false);
    (void)scheduler;
}

void FuzzAcquireCase4(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    bool tryBind = fdp.ConsumeBool();
    sptr<IRemoteObject> client = sptr<MockAbilityScheduler>::MakeSptr();
    auto scheduler = manager.Acquire(request, tryBind, client, true);
    (void)scheduler;
}

void FuzzAcquireCase5(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    bool tryBind = fdp.ConsumeBool();
    auto token = CreateMockToken(request);
    if (token) {
        auto scheduler = manager.Acquire(request, tryBind, token, false);
        (void)scheduler;
    }
}

void FuzzAcquireCase6(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    bool tryBind = fdp.ConsumeBool();
    bool isNotHap = fdp.ConsumeBool();
    sptr<IRemoteObject> client = nullptr;
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    manager.dataAbilityRecordsLoaded_[name] = dataRecord;
    auto scheduler = manager.Acquire(request, tryBind, client, isNotHap);
    (void)scheduler;
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzAcquireCase7(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    bool tryBind = fdp.ConsumeBool();
    bool isNotHap = fdp.ConsumeBool();
    sptr<IRemoteObject> client = nullptr;
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->scheduler_ = nullptr;
    manager.dataAbilityRecordsLoaded_[name] = dataRecord;
    auto scheduler = manager.Acquire(request, tryBind, client, isNotHap);
    (void)scheduler;
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzAcquireCase8(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    bool tryBind = fdp.ConsumeBool();
    bool isNotHap = fdp.ConsumeBool();
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    dataRecord->ability_->SetAbilityState(ACTIVE);
    manager.dataAbilityRecordsLoaded_[name] = dataRecord;
    sptr<IRemoteObject> acquireClient =
        sptr<MockAbilityScheduler>::MakeSptr();
    auto scheduler =
        manager.Acquire(request, tryBind, acquireClient, isNotHap);
    (void)scheduler;
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzAcquireCase9(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    for (int i = 0; i < MAX_RECORDS &&
        fdp.remaining_bytes() > U32_AT_SIZE; i++) {
        AbilityRequest reqLoop = CreateDataAbilityRequest(fdp);
        auto scheduler = manager.Acquire(
            reqLoop, fdp.ConsumeBool(), nullptr, fdp.ConsumeBool());
        (void)scheduler;
    }
}

void FuzzAcquire(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase =
        fdp.ConsumeIntegralInRange<int32_t>(0, MAX_ACQUIRE_CASE);

    switch (testCase) {
        case 0: FuzzAcquireCase0(fdp, manager); break;
        case 1: FuzzAcquireCase1(fdp, manager); break;
        case 2: FuzzAcquireCase2(fdp, manager); break;
        case 3: FuzzAcquireCase3(fdp, manager); break;
        case 4: FuzzAcquireCase4(fdp, manager); break;
        case 5: FuzzAcquireCase5(fdp, manager); break;
        case 6: FuzzAcquireCase6(fdp, manager); break;
        case 7: FuzzAcquireCase7(fdp, manager); break;
        case 8: FuzzAcquireCase8(fdp, manager); break;
        case 9: FuzzAcquireCase9(fdp, manager); break;
        default: break;
    }
}

// ---------- FuzzRelease helper functions ----------

void FuzzReleaseCase0(DataAbilityManager& manager, bool isNotHap)
{
    int ret = manager.Release(nullptr, nullptr, isNotHap);
    (void)ret;
}

void FuzzReleaseCase1(DataAbilityManager& manager, bool isNotHap)
{
    sptr<IAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();
    int ret = manager.Release(scheduler, nullptr, isNotHap);
    (void)ret;
}

void FuzzReleaseCase2(DataAbilityManager& manager, bool isNotHap)
{
    sptr<IRemoteObject> client = sptr<MockAbilityScheduler>::MakeSptr();
    int ret = manager.Release(nullptr, client, isNotHap);
    (void)ret;
}

void FuzzReleaseCase3(DataAbilityManager& manager, bool isNotHap)
{
    sptr<IAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> client = sptr<MockAbilityScheduler>::MakeSptr();
    int ret = manager.Release(scheduler, client, isNotHap);
    (void)ret;
}

void FuzzReleaseCase4(FuzzedDataProvider& fdp, DataAbilityManager& manager,
    bool isNotHap)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    dataRecord->ability_->SetAbilityState(ACTIVE);
    manager.dataAbilityRecordsLoaded_[name] = dataRecord;

    sptr<IAbilityScheduler> scheduler = dataRecord->scheduler_;
    sptr<IRemoteObject> client = sptr<MockAbilityScheduler>::MakeSptr();
    dataRecord->AddClient(client, true, false);
    int ret = manager.Release(scheduler, client, isNotHap);
    (void)ret;
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzReleaseCase5(FuzzedDataProvider& fdp, DataAbilityManager& manager,
    bool isNotHap)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    dataRecord->ability_->SetAbilityState(ACTIVE);
    manager.dataAbilityRecordsLoaded_[name] = dataRecord;

    sptr<IAbilityScheduler> scheduler = dataRecord->scheduler_;
    sptr<IRemoteObject> client = sptr<MockAbilityScheduler>::MakeSptr();
    int ret = manager.Release(scheduler, client, isNotHap);
    (void)ret;
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzReleaseCase6(FuzzedDataProvider& fdp, DataAbilityManager& manager,
    bool isNotHap)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    dataRecord->ability_ = nullptr;
    manager.dataAbilityRecordsLoaded_[name] = dataRecord;

    sptr<IAbilityScheduler> scheduler = dataRecord->scheduler_;
    sptr<IRemoteObject> client = sptr<MockAbilityScheduler>::MakeSptr();
    int ret = manager.Release(scheduler, client, isNotHap);
    (void)ret;
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzReleaseCase7(FuzzedDataProvider& fdp, DataAbilityManager& manager,
    bool isNotHap)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    manager.dataAbilityRecordsLoaded_[name] = dataRecord;

    sptr<IAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> client = sptr<MockAbilityScheduler>::MakeSptr();
    int ret = manager.Release(scheduler, client, isNotHap);
    (void)ret;
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzReleaseCase8(FuzzedDataProvider& fdp, DataAbilityManager& manager,
    bool isNotHap)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    manager.dataAbilityRecordsLoaded_[name] = nullptr;

    sptr<IAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> client = sptr<MockAbilityScheduler>::MakeSptr();
    int ret = manager.Release(scheduler, client, isNotHap);
    (void)ret;
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzReleaseCase9(FuzzedDataProvider& fdp, DataAbilityManager& manager,
    bool isNotHap)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    dataRecord->ability_->SetAbilityState(ACTIVE);
    manager.dataAbilityRecordsLoaded_[name] = dataRecord;

    sptr<IAbilityScheduler> scheduler = dataRecord->scheduler_;
    sptr<IRemoteObject> client = sptr<MockAbilityScheduler>::MakeSptr();
    dataRecord->AddClient(client, true, false);
    dataRecord->AddClient(client, true, false);
    int ret = manager.Release(scheduler, client, isNotHap);
    (void)ret;
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzRelease(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase =
        fdp.ConsumeIntegralInRange<int32_t>(0, MAX_RELEASE_CASE);
    bool isNotHap = fdp.ConsumeBool();

    switch (testCase) {
        case 0: FuzzReleaseCase0(manager, isNotHap); break;
        case 1: FuzzReleaseCase1(manager, isNotHap); break;
        case 2: FuzzReleaseCase2(manager, isNotHap); break;
        case 3: FuzzReleaseCase3(manager, isNotHap); break;
        case 4: FuzzReleaseCase4(fdp, manager, isNotHap); break;
        case 5: FuzzReleaseCase5(fdp, manager, isNotHap); break;
        case 6: FuzzReleaseCase6(fdp, manager, isNotHap); break;
        case 7: FuzzReleaseCase7(fdp, manager, isNotHap); break;
        case 8: FuzzReleaseCase8(fdp, manager, isNotHap); break;
        case 9: FuzzReleaseCase9(fdp, manager, isNotHap); break;
        default: break;
    }
}

// ---------- FuzzContainsDataAbility helper functions ----------

void FuzzContainsCase0(DataAbilityManager& manager)
{
    bool result = manager.ContainsDataAbility(nullptr);
    (void)result;
}

void FuzzContainsCase1(DataAbilityManager& manager)
{
    sptr<IAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();
    bool result = manager.ContainsDataAbility(scheduler);
    (void)result;
}

void FuzzContainsCase2(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    manager.dataAbilityRecordsLoaded_[name] = dataRecord;

    sptr<IAbilityScheduler> scheduler = dataRecord->scheduler_;
    bool result = manager.ContainsDataAbility(scheduler);
    (void)result;
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzContainsCase3(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    manager.dataAbilityRecordsLoaded_[name] = dataRecord;

    sptr<IAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();
    bool result = manager.ContainsDataAbility(scheduler);
    (void)result;
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzContainsCase4(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    manager.dataAbilityRecordsLoaded_[name] = nullptr;

    sptr<IAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();
    bool result = manager.ContainsDataAbility(scheduler);
    (void)result;
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzContainsDataAbility(
    FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase =
        fdp.ConsumeIntegralInRange<int32_t>(0, MAX_CONTAINS_CASE);

    switch (testCase) {
        case 0: FuzzContainsCase0(manager); break;
        case 1: FuzzContainsCase1(manager); break;
        case 2: FuzzContainsCase2(fdp, manager); break;
        case 3: FuzzContainsCase3(fdp, manager); break;
        case 4: FuzzContainsCase4(fdp, manager); break;
        default: break;
    }
}

// ---------- FuzzAttachAbilityThread helper functions ----------

void FuzzAttachCase0(DataAbilityManager& manager)
{
    int ret = manager.AttachAbilityThread(nullptr, nullptr);
    (void)ret;
}

void FuzzAttachCase1(DataAbilityManager& manager)
{
    sptr<IAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();
    int ret = manager.AttachAbilityThread(scheduler, nullptr);
    (void)ret;
}

void FuzzAttachCase2(DataAbilityManager& manager)
{
    sptr<IRemoteObject> token = sptr<MockAbilityScheduler>::MakeSptr();
    int ret = manager.AttachAbilityThread(nullptr, token);
    (void)ret;
}

void FuzzAttachCase3(DataAbilityManager& manager)
{
    sptr<IAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> token = sptr<MockAbilityScheduler>::MakeSptr();
    int ret = manager.AttachAbilityThread(scheduler, token);
    (void)ret;
}

void FuzzAttachCase4(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    dataRecord->ability_->SetAbilityState(INITIAL);
    manager.dataAbilityRecordsLoading_[name] = dataRecord;

    sptr<IAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> token = dataRecord->ability_->GetToken();
    int ret = manager.AttachAbilityThread(scheduler, token);
    (void)ret;
    manager.dataAbilityRecordsLoading_.clear();
}

void FuzzAttachCase5(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    manager.dataAbilityRecordsLoading_[name] = nullptr;

    sptr<IAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> token = sptr<MockAbilityScheduler>::MakeSptr();
    int ret = manager.AttachAbilityThread(scheduler, token);
    (void)ret;
    manager.dataAbilityRecordsLoading_.clear();
}

void FuzzAttachCase6(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    manager.dataAbilityRecordsLoading_[name] = dataRecord;

    sptr<IAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> token = dataRecord->ability_->GetToken();
    int ret1 = manager.AttachAbilityThread(scheduler, token);
    int ret2 = manager.AttachAbilityThread(scheduler, token);
    (void)ret1;
    (void)ret2;
    manager.dataAbilityRecordsLoading_.clear();
}

void FuzzAttachAbilityThread(
    FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase =
        fdp.ConsumeIntegralInRange<int32_t>(0, MAX_ATTACH_CASE);

    switch (testCase) {
        case 0: FuzzAttachCase0(manager); break;
        case 1: FuzzAttachCase1(manager); break;
        case 2: FuzzAttachCase2(manager); break;
        case 3: FuzzAttachCase3(manager); break;
        case 4: FuzzAttachCase4(fdp, manager); break;
        case 5: FuzzAttachCase5(fdp, manager); break;
        case 6: FuzzAttachCase6(fdp, manager); break;
        default: break;
    }
}

// ---------- FuzzAbilityTransitionDone helper functions ----------

void FuzzTransitionCase0(DataAbilityManager& manager, int state)
{
    int ret = manager.AbilityTransitionDone(nullptr, state);
    (void)ret;
}

void FuzzTransitionCase1(DataAbilityManager& manager, int state)
{
    sptr<IRemoteObject> token = sptr<MockAbilityScheduler>::MakeSptr();
    int ret = manager.AbilityTransitionDone(token, state);
    (void)ret;
}

void FuzzTransitionCase2(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    dataRecord->ability_->SetAbilityState(ACTIVATING);
    manager.dataAbilityRecordsLoading_[name] = dataRecord;

    sptr<IRemoteObject> token = dataRecord->ability_->GetToken();
    int ret = manager.AbilityTransitionDone(
        token, AbilityLifeCycleState::ABILITY_STATE_ACTIVE);
    (void)ret;
    manager.dataAbilityRecordsLoading_.clear();
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzTransitionCase3(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    dataRecord->ability_->SetAbilityState(ACTIVATING);
    manager.dataAbilityRecordsLoading_[name] = dataRecord;

    sptr<IRemoteObject> token = dataRecord->ability_->GetToken();
    int ret = manager.AbilityTransitionDone(
        token, AbilityLifeCycleState::ABILITY_STATE_INACTIVE);
    (void)ret;
    manager.dataAbilityRecordsLoading_.clear();
}

void FuzzTransitionCase4(FuzzedDataProvider& fdp, DataAbilityManager& manager,
    int state)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    manager.dataAbilityRecordsLoading_[name] = nullptr;

    sptr<IRemoteObject> token = sptr<MockAbilityScheduler>::MakeSptr();
    int ret = manager.AbilityTransitionDone(token, state);
    (void)ret;
    manager.dataAbilityRecordsLoading_.clear();
}

void FuzzTransitionCase5(FuzzedDataProvider& fdp, DataAbilityManager& manager,
    int state)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    dataRecord->scheduler_ = nullptr;
    dataRecord->ability_->SetAbilityState(ACTIVATING);
    manager.dataAbilityRecordsLoading_[name] = dataRecord;

    sptr<IRemoteObject> token = dataRecord->ability_->GetToken();
    int ret = manager.AbilityTransitionDone(token, state);
    (void)ret;
    manager.dataAbilityRecordsLoading_.clear();
}

void FuzzTransitionCase6(FuzzedDataProvider& fdp, DataAbilityManager& manager,
    int state)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    dataRecord->ability_->SetAbilityState(INITIAL);
    manager.dataAbilityRecordsLoading_[name] = dataRecord;

    sptr<IRemoteObject> token = dataRecord->ability_->GetToken();
    int ret = manager.AbilityTransitionDone(token, state);
    (void)ret;
    manager.dataAbilityRecordsLoading_.clear();
}

void FuzzTransitionCase7(FuzzedDataProvider& fdp, DataAbilityManager& manager,
    int state)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    manager.dataAbilityRecordsLoading_[name] = dataRecord;

    sptr<IRemoteObject> token = dataRecord->ability_->GetToken();
    int ret = manager.AbilityTransitionDone(token, state);
    (void)ret;
    manager.dataAbilityRecordsLoading_.clear();
}

void FuzzAbilityTransitionDone(
    FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase =
        fdp.ConsumeIntegralInRange<int32_t>(0, MAX_TRANSITION_CASE);
    int state = fdp.ConsumeIntegralInRange<int>(0, STATE_RANGE_MAX);

    switch (testCase) {
        case 0: FuzzTransitionCase0(manager, state); break;
        case 1: FuzzTransitionCase1(manager, state); break;
        case 2: FuzzTransitionCase2(fdp, manager); break;
        case 3: FuzzTransitionCase3(fdp, manager); break;
        case 4: FuzzTransitionCase4(fdp, manager, state); break;
        case 5: FuzzTransitionCase5(fdp, manager, state); break;
        case 6: FuzzTransitionCase6(fdp, manager, state); break;
        case 7: FuzzTransitionCase7(fdp, manager, state); break;
        default: break;
    }
}

// ---------- FuzzOnAbilityDied helper functions ----------

void FuzzDiedCase0(DataAbilityManager& manager)
{
    manager.OnAbilityDied(nullptr);
}

void FuzzDiedCase1(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(request);
    if (abilityRecord) {
        abilityRecord->SetAbilityState(ACTIVE);
        manager.OnAbilityDied(abilityRecord);
    }
}

void FuzzDiedCase2(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    dataRecord->ability_->SetAbilityState(ACTIVE);
    manager.dataAbilityRecordsLoaded_[name] = dataRecord;

    auto abilityRecord = dataRecord->ability_;
    manager.OnAbilityDied(abilityRecord);
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzDiedCase3(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    request.abilityInfo.type = AbilityType::PAGE;
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    manager.dataAbilityRecordsLoaded_[name] = dataRecord;

    auto abilityRecord = AbilityRecord::CreateAbilityRecord(request);
    manager.OnAbilityDied(abilityRecord);
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzDiedCase4(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    manager.dataAbilityRecordsLoaded_[name] = nullptr;

    auto abilityRecord = AbilityRecord::CreateAbilityRecord(request);
    manager.OnAbilityDied(abilityRecord);
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzDiedCase5(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    dataRecord->ability_->SetAbilityState(ACTIVE);

    sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> client2 = sptr<MockAbilityScheduler>::MakeSptr();
    dataRecord->AddClient(client1, true, false);
    dataRecord->AddClient(client2, false, true);

    manager.dataAbilityRecordsLoaded_[name] = dataRecord;
    manager.dataAbilityRecordsLoaded_["other"] = nullptr;

    auto abilityRecord = dataRecord->ability_;
    manager.OnAbilityDied(abilityRecord);
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzOnAbilityDied(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase =
        fdp.ConsumeIntegralInRange<int32_t>(0, MAX_DIED_CASE);

    switch (testCase) {
        case 0: FuzzDiedCase0(manager); break;
        case 1: FuzzDiedCase1(fdp, manager); break;
        case 2: FuzzDiedCase2(fdp, manager); break;
        case 3: FuzzDiedCase3(fdp, manager); break;
        case 4: FuzzDiedCase4(fdp, manager); break;
        case 5: FuzzDiedCase5(fdp, manager); break;
        default: break;
    }
}

// ---------- FuzzOnAppStateChanged helper functions ----------

void FuzzAppStateCase0(DataAbilityManager& manager)
{
    AppInfo appInfo;
    appInfo.bundleName = "";
    manager.OnAppStateChanged(appInfo);
}

void FuzzAppStateCase1(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AppInfo appInfo;
    appInfo.bundleName = fdp.ConsumeRandomLengthString(64);
    appInfo.appIndex = fdp.ConsumeIntegral<int32_t>();
    appInfo.instanceKey = fdp.ConsumeRandomLengthString(32);
    appInfo.state = fdp.ConsumeIntegral<int32_t>();
    manager.OnAppStateChanged(appInfo);
}

void FuzzAppStateCase2(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    manager.dataAbilityRecordsLoaded_[name] = dataRecord;

    AppInfo appInfo;
    appInfo.bundleName = request.abilityInfo.bundleName;
    appInfo.appIndex = dataRecord->ability_->GetAppIndex();
    appInfo.instanceKey = dataRecord->ability_->GetInstanceKey();
    AppData appData;
    appData.appName = request.appInfo.name;
    appData.uid = request.abilityInfo.applicationInfo.uid;
    appInfo.appData.push_back(appData);
    manager.OnAppStateChanged(appInfo);
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzAppStateCase3(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    manager.dataAbilityRecordsLoaded_[name] = nullptr;

    AppInfo appInfo;
    appInfo.bundleName = request.abilityInfo.bundleName;
    manager.OnAppStateChanged(appInfo);
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzAppStateCase4(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    manager.dataAbilityRecordsLoading_[name] = dataRecord;

    AppInfo appInfo;
    appInfo.bundleName = request.abilityInfo.bundleName;
    appInfo.appIndex = dataRecord->ability_->GetAppIndex();
    appInfo.instanceKey = dataRecord->ability_->GetInstanceKey();
    AppData appData;
    appData.appName = request.appInfo.name;
    appInfo.appData.push_back(appData);
    manager.OnAppStateChanged(appInfo);
    manager.dataAbilityRecordsLoading_.clear();
}

void FuzzAppStateCase5(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    manager.dataAbilityRecordsLoaded_[name] = dataRecord;

    AppInfo appInfo;
    appInfo.bundleName = "different_bundle";
    appInfo.appIndex = APP_INDEX_SENTINEL;
    manager.OnAppStateChanged(appInfo);
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzOnAppStateChanged(
    FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase =
        fdp.ConsumeIntegralInRange<int32_t>(0, MAX_APP_STATE_CASE);

    switch (testCase) {
        case 0: FuzzAppStateCase0(manager); break;
        case 1: FuzzAppStateCase1(fdp, manager); break;
        case 2: FuzzAppStateCase2(fdp, manager); break;
        case 3: FuzzAppStateCase3(fdp, manager); break;
        case 4: FuzzAppStateCase4(fdp, manager); break;
        case 5: FuzzAppStateCase5(fdp, manager); break;
        default: break;
    }
}

// ---------- FuzzGetAbilityRecordById helper functions ----------

void FuzzRecordByIdCase0(DataAbilityManager& manager, int64_t id)
{
    auto record = manager.GetAbilityRecordById(id);
    (void)record;
}

void FuzzRecordByIdCase1(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    manager.dataAbilityRecordsLoaded_[name] = dataRecord;

    int64_t id = dataRecord->ability_->GetRecordId();
    auto record = manager.GetAbilityRecordById(id);
    (void)record;
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzRecordByIdCase2(FuzzedDataProvider& fdp, DataAbilityManager& manager,
    int64_t id)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    manager.dataAbilityRecordsLoaded_[name] = nullptr;

    auto record = manager.GetAbilityRecordById(id);
    (void)record;
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzRecordByIdCase3(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name1 = MakeDataAbilityName(request);
    auto dataRecord1 = std::make_shared<DataAbilityRecord>(request);
    dataRecord1->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    manager.dataAbilityRecordsLoaded_[name1] = dataRecord1;

    AbilityRequest request2 = CreateDataAbilityRequest(fdp);
    std::string name2 = MakeDataAbilityName(request2);
    manager.dataAbilityRecordsLoaded_[name2] = nullptr;

    auto record = manager.GetAbilityRecordById(INT_MAX_VAL);
    (void)record;
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzRecordByIdCase4(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    for (int i = 0; i < LOOP_COUNT_SMALL &&
        fdp.remaining_bytes() > U32_AT_SIZE; i++) {
        AbilityRequest reqLoop = CreateDataAbilityRequest(fdp);
        std::string nameLoop = MakeDataAbilityName(reqLoop);
        auto dataRecordLoop =
            std::make_shared<DataAbilityRecord>(reqLoop);
        dataRecordLoop->ability_ = std::make_shared<AbilityRecord>(
            reqLoop.want, reqLoop.abilityInfo, reqLoop.appInfo);
        manager.dataAbilityRecordsLoaded_[nameLoop] = dataRecordLoop;
    }
    auto record =
        manager.GetAbilityRecordById(fdp.ConsumeIntegral<int64_t>());
    (void)record;
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzGetAbilityRecordById(
    FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase =
        fdp.ConsumeIntegralInRange<int32_t>(0, MAX_RECORD_BY_ID_CASE);
    int64_t id = fdp.ConsumeIntegral<int64_t>();

    switch (testCase) {
        case 0: FuzzRecordByIdCase0(manager, id); break;
        case 1: FuzzRecordByIdCase1(fdp, manager); break;
        case 2: FuzzRecordByIdCase2(fdp, manager, id); break;
        case 3: FuzzRecordByIdCase3(fdp, manager); break;
        case 4: FuzzRecordByIdCase4(fdp, manager); break;
        default: break;
    }
}

// ---------- FuzzGetAbilityRecordByToken helper functions ----------

void FuzzRecordByTokenCase0(DataAbilityManager& manager)
{
    auto record = manager.GetAbilityRecordByToken(nullptr);
    (void)record;
}

void FuzzRecordByTokenCase1(DataAbilityManager& manager)
{
    sptr<IRemoteObject> token = sptr<MockAbilityScheduler>::MakeSptr();
    auto record = manager.GetAbilityRecordByToken(token);
    (void)record;
}

void FuzzRecordByTokenCase2(
    FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    manager.dataAbilityRecordsLoaded_[name] = dataRecord;

    sptr<IRemoteObject> token = dataRecord->ability_->GetToken();
    auto record = manager.GetAbilityRecordByToken(token);
    (void)record;
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzRecordByTokenCase3(
    FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    manager.dataAbilityRecordsLoading_[name] = dataRecord;

    sptr<IRemoteObject> token = dataRecord->ability_->GetToken();
    auto record = manager.GetAbilityRecordByToken(token);
    (void)record;
    manager.dataAbilityRecordsLoading_.clear();
}

void FuzzRecordByTokenCase4(
    FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    manager.dataAbilityRecordsLoaded_[name] = nullptr;
    manager.dataAbilityRecordsLoading_[name] = nullptr;

    sptr<IRemoteObject> token = sptr<MockAbilityScheduler>::MakeSptr();
    auto record = manager.GetAbilityRecordByToken(token);
    (void)record;
    manager.dataAbilityRecordsLoaded_.clear();
    manager.dataAbilityRecordsLoading_.clear();
}

void FuzzRecordByTokenCase5(
    FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    manager.dataAbilityRecordsLoaded_[name] = dataRecord;

    sptr<IRemoteObject> token = sptr<MockAbilityScheduler>::MakeSptr();
    auto record = manager.GetAbilityRecordByToken(token);
    (void)record;
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzGetAbilityRecordByToken(
    FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase =
        fdp.ConsumeIntegralInRange<int32_t>(0, MAX_RECORD_BY_TOKEN_CASE);

    switch (testCase) {
        case 0: FuzzRecordByTokenCase0(manager); break;
        case 1: FuzzRecordByTokenCase1(manager); break;
        case 2: FuzzRecordByTokenCase2(fdp, manager); break;
        case 3: FuzzRecordByTokenCase3(fdp, manager); break;
        case 4: FuzzRecordByTokenCase4(fdp, manager); break;
        case 5: FuzzRecordByTokenCase5(fdp, manager); break;
        default: break;
    }
}

// ---------- FuzzGetAbilityRecordByScheduler helper functions ----------

void FuzzRecordBySchedCase0(DataAbilityManager& manager)
{
    auto record = manager.GetAbilityRecordByScheduler(nullptr);
    (void)record;
}

void FuzzRecordBySchedCase1(DataAbilityManager& manager)
{
    sptr<IAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();
    auto record = manager.GetAbilityRecordByScheduler(scheduler);
    (void)record;
}

void FuzzRecordBySchedCase2(
    FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    manager.dataAbilityRecordsLoaded_[name] = dataRecord;

    sptr<IAbilityScheduler> scheduler = dataRecord->scheduler_;
    auto record = manager.GetAbilityRecordByScheduler(scheduler);
    (void)record;
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzRecordBySchedCase3(
    FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    manager.dataAbilityRecordsLoaded_[name] = dataRecord;

    sptr<IAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();
    auto record = manager.GetAbilityRecordByScheduler(scheduler);
    (void)record;
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzRecordBySchedCase4(
    FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    manager.dataAbilityRecordsLoaded_[name] = nullptr;

    sptr<IAbilityScheduler> scheduler =
        sptr<MockAbilityScheduler>::MakeSptr();
    auto record = manager.GetAbilityRecordByScheduler(scheduler);
    (void)record;
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzGetAbilityRecordByScheduler(
    FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase =
        fdp.ConsumeIntegralInRange<int32_t>(0, MAX_RECORD_BY_SCHED_CASE);

    switch (testCase) {
        case 0: FuzzRecordBySchedCase0(manager); break;
        case 1: FuzzRecordBySchedCase1(manager); break;
        case 2: FuzzRecordBySchedCase2(fdp, manager); break;
        case 3: FuzzRecordBySchedCase3(fdp, manager); break;
        case 4: FuzzRecordBySchedCase4(fdp, manager); break;
        default: break;
    }
}

// ---------- FuzzDumpFunctions helper functions ----------

void FuzzDumpCase0(DataAbilityManager& manager,
    const std::string& func, int line)
{
    manager.Dump(func.c_str(), line);
}

void FuzzDumpCase1(DataAbilityManager& manager, int line)
{
    manager.Dump(nullptr, line);
}

void FuzzDumpCase2(DataAbilityManager& manager,
    std::vector<std::string>& info, const std::string& args)
{
    manager.DumpState(info, args);
}

void FuzzDumpCase3(FuzzedDataProvider& fdp, DataAbilityManager& manager,
    std::vector<std::string>& info)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    manager.dataAbilityRecordsLoaded_[name] = dataRecord;

    std::string args = name;
    manager.DumpState(info, args);
    manager.DumpState(info, "nonexistent");
    manager.DumpState(info, "");
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzDumpCase4(DataAbilityManager& manager,
    std::vector<std::string>& info, bool isClient,
    const std::string& args)
{
    manager.DumpSysState(info, isClient, args);
}

void FuzzDumpCase5(FuzzedDataProvider& fdp, DataAbilityManager& manager,
    std::vector<std::string>& info)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    dataRecord->ability_->SetAbilityState(ACTIVE);
    manager.dataAbilityRecordsLoaded_[name] = dataRecord;

    manager.DumpSysState(info, true, name);
    manager.DumpSysState(info, false, "");
    manager.DumpSysState(info, true, "nonexistent");
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzDumpCase6(DataAbilityManager& manager,
    const std::string& func, int line)
{
    manager.DumpLocked(func.c_str(), line);
    manager.DumpLocked(nullptr, -1);
}

void FuzzDumpFunctions(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase =
        fdp.ConsumeIntegralInRange<int32_t>(0, MAX_DUMP_CASE);
    std::vector<std::string> info;
    std::string args = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    bool isClient = fdp.ConsumeBool();
    std::string func = fdp.ConsumeRandomLengthString(32);
    int line = fdp.ConsumeIntegral<int>();

    switch (testCase) {
        case 0: FuzzDumpCase0(manager, func, line); break;
        case 1: FuzzDumpCase1(manager, line); break;
        case 2: FuzzDumpCase2(manager, info, args); break;
        case 3: FuzzDumpCase3(fdp, manager, info); break;
        case 4: FuzzDumpCase4(manager, info, isClient, args); break;
        case 5: FuzzDumpCase5(fdp, manager, info); break;
        case 6: FuzzDumpCase6(manager, func, line); break;
        default: break;
    }
}

// ---------- FuzzGetAbilityRunningInfos helper functions ----------

void FuzzRunningInfoCase0(DataAbilityManager& manager, bool isPerm)
{
    std::vector<AbilityRunningInfo> info;
    manager.GetAbilityRunningInfos(info, isPerm);
}

void FuzzRunningInfoCase1(
    FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    manager.dataAbilityRecordsLoaded_[name] = dataRecord;

    std::vector<AbilityRunningInfo> info;
    manager.GetAbilityRunningInfos(info, true);
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzRunningInfoCase2(
    FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->ability_ = nullptr;
    manager.dataAbilityRecordsLoaded_[name] = dataRecord;

    std::vector<AbilityRunningInfo> info;
    manager.GetAbilityRunningInfos(info, false);
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzRunningInfoCase3(
    FuzzedDataProvider& fdp, DataAbilityManager& manager, bool isPerm)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    std::string name = MakeDataAbilityName(request);
    auto dataRecord = std::make_shared<DataAbilityRecord>(request);
    dataRecord->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    manager.dataAbilityRecordsLoading_[name] = dataRecord;
    manager.dataAbilityRecordsLoaded_[name] = nullptr;

    std::vector<AbilityRunningInfo> info;
    manager.GetAbilityRunningInfos(info, isPerm);
    manager.dataAbilityRecordsLoading_.clear();
    manager.dataAbilityRecordsLoaded_.clear();
}

void FuzzGetAbilityRunningInfos(
    FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase =
        fdp.ConsumeIntegralInRange<int32_t>(0, MAX_RUNNING_INFO_CASE);
    bool isPerm = fdp.ConsumeBool();

    switch (testCase) {
        case 0: FuzzRunningInfoCase0(manager, isPerm); break;
        case 1: FuzzRunningInfoCase1(fdp, manager); break;
        case 2: FuzzRunningInfoCase2(fdp, manager); break;
        case 3: FuzzRunningInfoCase3(fdp, manager, isPerm); break;
        default: break;
    }
}

// ---------- FuzzOnAbilityRequestDone ----------

void FuzzOnAbilityRequestDone(
    FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    sptr<IRemoteObject> token = sptr<MockAbilityScheduler>::MakeSptr();
    int32_t state = fdp.ConsumeIntegral<int32_t>();
    manager.OnAbilityRequestDone(token, state);
    manager.OnAbilityRequestDone(nullptr, state);
}

// ---------- FuzzReportFunctions helper functions ----------

void FuzzReportCase0(DataAbilityManager& manager, bool isNotHap)
{
    std::shared_ptr<DataAbilityRecord> record = nullptr;
    manager.ReportDataAbilityAcquired(nullptr, isNotHap, record);
}

void FuzzReportCase1(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    sptr<IRemoteObject> client = sptr<MockAbilityScheduler>::MakeSptr();
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    manager.ReportDataAbilityAcquired(client, false, record);
}

void FuzzReportCase2(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    sptr<IRemoteObject> client = sptr<MockAbilityScheduler>::MakeSptr();
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    manager.ReportDataAbilityAcquired(client, true, record);
    manager.ReportDataAbilityReleased(client, true, record);
}

void FuzzReportCase3(
    FuzzedDataProvider& fdp, DataAbilityManager& manager, bool isNotHap)
{
    sptr<IRemoteObject> client = sptr<MockAbilityScheduler>::MakeSptr();
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = std::make_shared<AbilityRecord>(
        request.want, request.abilityInfo, request.appInfo);
    manager.ReportDataAbilityReleased(client, false, record);
    manager.ReportDataAbilityReleased(nullptr, isNotHap, record);
}

void FuzzReportFunctions(
    FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase =
        fdp.ConsumeIntegralInRange<int32_t>(0, MAX_REPORT_CASE);
    bool isNotHap = fdp.ConsumeBool();

    switch (testCase) {
        case 0: FuzzReportCase0(manager, isNotHap); break;
        case 1: FuzzReportCase1(fdp, manager); break;
        case 2: FuzzReportCase2(fdp, manager); break;
        case 3: FuzzReportCase3(fdp, manager, isNotHap); break;
        default: break;
    }
}

// ---------- DoSomethingInterestingWithMyAPI helper functions ----------

void HandleLoadLocked(
    FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    std::string name = fdp.ConsumeRandomLengthString(64);
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto result = manager.LoadLocked(name, request);
    (void)result;
}

void HandleRestartDataAbility(
    FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(request);
    if (abilityRecord) {
        manager.RestartDataAbility(abilityRecord);
    }
    manager.RestartDataAbility(nullptr);
}

void DispatchFuzzOperation(
    FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t operationType =
        fdp.ConsumeIntegralInRange<int32_t>(0, MAX_OPERATION_CASE);

    switch (operationType) {
        case 0: FuzzAcquire(fdp, manager); break;
        case 1: FuzzRelease(fdp, manager); break;
        case 2: FuzzContainsDataAbility(fdp, manager); break;
        case 3: FuzzAttachAbilityThread(fdp, manager); break;
        case 4: FuzzAbilityTransitionDone(fdp, manager); break;
        case 5: FuzzOnAbilityDied(fdp, manager); break;
        case 6: FuzzOnAppStateChanged(fdp, manager); break;
        case 7: FuzzGetAbilityRecordById(fdp, manager); break;
        case 8: FuzzGetAbilityRecordByToken(fdp, manager); break;
        case 9: FuzzGetAbilityRecordByScheduler(fdp, manager); break;
        case 10: FuzzDumpFunctions(fdp, manager); break;
        case 11: FuzzGetAbilityRunningInfos(fdp, manager); break;
        case 12: FuzzOnAbilityRequestDone(fdp, manager); break;
        case 13: FuzzReportFunctions(fdp, manager); break;
        case 14: HandleLoadLocked(fdp, manager); break;
        case 15: HandleRestartDataAbility(fdp, manager); break;
        default: break;
    }
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < U32_AT_SIZE) {
        return false;
    }

    FuzzedDataProvider fdp(data, size);
    auto dataAbilityManager = std::make_shared<DataAbilityManager>();

    dataAbilityManager->dataAbilityRecordsLoaded_.clear();
    dataAbilityManager->dataAbilityRecordsLoading_.clear();

    DispatchFuzzOperation(fdp, *dataAbilityManager);

    dataAbilityManager->dataAbilityRecordsLoaded_.clear();
    dataAbilityManager->dataAbilityRecordsLoading_.clear();
    return true;
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
