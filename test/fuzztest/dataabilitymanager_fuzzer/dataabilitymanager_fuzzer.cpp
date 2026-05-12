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

class MockAbilityScheduler : public IRemoteStub<IAbilityScheduler> {
public:
    MockAbilityScheduler() = default;
    virtual ~MockAbilityScheduler() = default;

    int ScheduleAbilityTransaction(const Want &want, const LifeCycleStateInfo &stateInfo) override { return 0; }
    int ScheduleConnectAbility(const Want &want) override { return 0; }
    int ScheduleDisconnectAbility(const Want &want) override { return 0; }
    void ScheduleCommandAbility(const Want &want, bool restart, int visParam) override {}
    void ScheduleCommandAbilityWindow(const Want &want, const sptr<AAFwk::AbilityWindowInfo> &windowInfo,
        bool restart, int visParam) override {}
    int ScheduleSaveAbilityState(PacMap &stateData) override { return 0; }
    int ScheduleRestoreAbilityState(const PacMap &stateData) override { return 0; }
    int ScheduleNotifyExtensionBindStateInfo(int32_t bindState, const Want &want) override { return 0; }
    void DumpAbilityInfo(const std::vector<std::string> &params, std::vector<std::string> &info) override {}
    sptr<IRemoteObject> AsObject() override { return this; }
};

AbilityRequest CreateDataAbilityRequest(FuzzedDataProvider& fdp)
{
    AbilityRequest request;
    request.abilityInfo.type = AbilityType::DATA;
    request.abilityInfo.bundleName = fdp.ConsumeRandomLengthString(64);
    request.abilityInfo.name = fdp.ConsumeRandomLengthString(64);
    request.abilityInfo.moduleName = fdp.ConsumeRandomLengthString(64);
    request.abilityInfo.applicationInfo.bundleName = request.abilityInfo.bundleName;
    request.abilityInfo.applicationInfo.uid = fdp.ConsumeIntegral<int32_t>();
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

void FuzzAcquire(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 9);
    AbilityRequest request;
    sptr<IRemoteObject> client = nullptr;
    bool tryBind = fdp.ConsumeBool();
    bool isNotHap = fdp.ConsumeBool();
    
    switch (testCase) {
        case 0: {
            request = CreateNonDataAbilityRequest(fdp);
            auto scheduler = manager.Acquire(request, tryBind, client, isNotHap);
            (void)scheduler;
            break;
        }
        case 1: {
            request = CreateEmptyNameRequest();
            auto scheduler = manager.Acquire(request, tryBind, client, isNotHap);
            (void)scheduler;
            break;
        }
        case 2: {
            request = CreateDataAbilityRequest(fdp);
            auto scheduler = manager.Acquire(request, tryBind, client, isNotHap);
            (void)scheduler;
            break;
        }
        case 3: {
            request = CreateDataAbilityRequest(fdp);
            client = sptr<MockAbilityScheduler>::MakeSptr();
            isNotHap = false;
            auto scheduler = manager.Acquire(request, tryBind, client, isNotHap);
            (void)scheduler;
            break;
        }
        case 4: {
            request = CreateDataAbilityRequest(fdp);
            client = sptr<MockAbilityScheduler>::MakeSptr();
            isNotHap = true;
            auto scheduler = manager.Acquire(request, tryBind, client, isNotHap);
            (void)scheduler;
            break;
        }
        case 5: {
            request = CreateDataAbilityRequest(fdp);
            auto token = CreateMockToken(request);
            if (token) {
                auto scheduler = manager.Acquire(request, tryBind, token, false);
                (void)scheduler;
            }
            break;
        }
        case 6: {
            request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            auto scheduler = manager.Acquire(request, tryBind, client, isNotHap);
            (void)scheduler;
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 7: {
            request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->scheduler_ = nullptr;
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            auto scheduler = manager.Acquire(request, tryBind, client, isNotHap);
            (void)scheduler;
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 8: {
            request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            dataRecord->ability_->SetAbilityState(ACTIVE);
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            sptr<IRemoteObject> acquireClient = sptr<MockAbilityScheduler>::MakeSptr();
            auto scheduler = manager.Acquire(request, tryBind, acquireClient, isNotHap);
            (void)scheduler;
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 9: {
            for (int i = 0; i < MAX_RECORDS && fdp.remaining_bytes() > U32_AT_SIZE; i++) {
                AbilityRequest reqLoop = CreateDataAbilityRequest(fdp);
                auto scheduler = manager.Acquire(reqLoop, fdp.ConsumeBool(), nullptr, fdp.ConsumeBool());
                (void)scheduler;
            }
            break;
        }
    }
}

void FuzzRelease(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 9);
    sptr<IAbilityScheduler> scheduler = nullptr;
    sptr<IRemoteObject> client = nullptr;
    bool isNotHap = fdp.ConsumeBool();
    
    switch (testCase) {
        case 0: {
            int ret = manager.Release(nullptr, nullptr, isNotHap);
            (void)ret;
            break;
        }
        case 1: {
            scheduler = sptr<MockAbilityScheduler>::MakeSptr();
            int ret = manager.Release(scheduler, nullptr, isNotHap);
            (void)ret;
            break;
        }
        case 2: {
            client = sptr<MockAbilityScheduler>::MakeSptr();
            int ret = manager.Release(nullptr, client, isNotHap);
            (void)ret;
            break;
        }
        case 3: {
            scheduler = sptr<MockAbilityScheduler>::MakeSptr();
            client = sptr<MockAbilityScheduler>::MakeSptr();
            int ret = manager.Release(scheduler, client, isNotHap);
            (void)ret;
            break;
        }
        case 4: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            dataRecord->ability_->SetAbilityState(ACTIVE);
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            
            scheduler = dataRecord->scheduler_;
            client = sptr<MockAbilityScheduler>::MakeSptr();
            dataRecord->AddClient(client, true, false);
            int ret = manager.Release(scheduler, client, isNotHap);
            (void)ret;
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 5: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            dataRecord->ability_->SetAbilityState(ACTIVE);
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            
            scheduler = dataRecord->scheduler_;
            client = sptr<MockAbilityScheduler>::MakeSptr();
            int ret = manager.Release(scheduler, client, isNotHap);
            (void)ret;
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 6: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            dataRecord->ability_ = nullptr;
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            
            scheduler = dataRecord->scheduler_;
            client = sptr<MockAbilityScheduler>::MakeSptr();
            int ret = manager.Release(scheduler, client, isNotHap);
            (void)ret;
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 7: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            
            scheduler = sptr<MockAbilityScheduler>::MakeSptr();
            client = sptr<MockAbilityScheduler>::MakeSptr();
            int ret = manager.Release(scheduler, client, isNotHap);
            (void)ret;
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 8: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            manager.dataAbilityRecordsLoaded_[name] = nullptr;
            
            scheduler = sptr<MockAbilityScheduler>::MakeSptr();
            client = sptr<MockAbilityScheduler>::MakeSptr();
            int ret = manager.Release(scheduler, client, isNotHap);
            (void)ret;
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 9: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            dataRecord->ability_->SetAbilityState(ACTIVE);
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            
            scheduler = dataRecord->scheduler_;
            client = sptr<MockAbilityScheduler>::MakeSptr();
            dataRecord->AddClient(client, true, false);
            dataRecord->AddClient(client, true, false);
            int ret = manager.Release(scheduler, client, isNotHap);
            (void)ret;
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
    }
}

void FuzzContainsDataAbility(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 4);
    sptr<IAbilityScheduler> scheduler = nullptr;
    
    switch (testCase) {
        case 0: {
            bool result = manager.ContainsDataAbility(nullptr);
            (void)result;
            break;
        }
        case 1: {
            scheduler = sptr<MockAbilityScheduler>::MakeSptr();
            bool result = manager.ContainsDataAbility(scheduler);
            (void)result;
            break;
        }
        case 2: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            
            scheduler = dataRecord->scheduler_;
            bool result = manager.ContainsDataAbility(scheduler);
            (void)result;
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 3: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            
            scheduler = sptr<MockAbilityScheduler>::MakeSptr();
            bool result = manager.ContainsDataAbility(scheduler);
            (void)result;
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 4: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            manager.dataAbilityRecordsLoaded_[name] = nullptr;
            
            scheduler = sptr<MockAbilityScheduler>::MakeSptr();
            bool result = manager.ContainsDataAbility(scheduler);
            (void)result;
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
    }
}

void FuzzAttachAbilityThread(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 6);
    sptr<IAbilityScheduler> scheduler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    
    switch (testCase) {
        case 0: {
            int ret = manager.AttachAbilityThread(nullptr, nullptr);
            (void)ret;
            break;
        }
        case 1: {
            scheduler = sptr<MockAbilityScheduler>::MakeSptr();
            int ret = manager.AttachAbilityThread(scheduler, nullptr);
            (void)ret;
            break;
        }
        case 2: {
            token = sptr<MockAbilityScheduler>::MakeSptr();
            int ret = manager.AttachAbilityThread(nullptr, token);
            (void)ret;
            break;
        }
        case 3: {
            scheduler = sptr<MockAbilityScheduler>::MakeSptr();
            token = sptr<MockAbilityScheduler>::MakeSptr();
            int ret = manager.AttachAbilityThread(scheduler, token);
            (void)ret;
            break;
        }
        case 4: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            dataRecord->ability_->SetAbilityState(INITIAL);
            manager.dataAbilityRecordsLoading_[name] = dataRecord;
            
            scheduler = sptr<MockAbilityScheduler>::MakeSptr();
            token = dataRecord->ability_->GetToken();
            int ret = manager.AttachAbilityThread(scheduler, token);
            (void)ret;
            manager.dataAbilityRecordsLoading_.clear();
            break;
        }
        case 5: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            manager.dataAbilityRecordsLoading_[name] = nullptr;
            
            scheduler = sptr<MockAbilityScheduler>::MakeSptr();
            token = sptr<MockAbilityScheduler>::MakeSptr();
            int ret = manager.AttachAbilityThread(scheduler, token);
            (void)ret;
            manager.dataAbilityRecordsLoading_.clear();
            break;
        }
        case 6: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            manager.dataAbilityRecordsLoading_[name] = dataRecord;
            
            scheduler = sptr<MockAbilityScheduler>::MakeSptr();
            token = dataRecord->ability_->GetToken();
            int ret1 = manager.AttachAbilityThread(scheduler, token);
            int ret2 = manager.AttachAbilityThread(scheduler, token);
            (void)ret1;
            (void)ret2;
            manager.dataAbilityRecordsLoading_.clear();
            break;
        }
    }
}

void FuzzAbilityTransitionDone(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 7);
    sptr<IRemoteObject> token = nullptr;
    int state = fdp.ConsumeIntegralInRange<int>(0, 10);
    
    switch (testCase) {
        case 0: {
            int ret = manager.AbilityTransitionDone(nullptr, state);
            (void)ret;
            break;
        }
        case 1: {
            token = sptr<MockAbilityScheduler>::MakeSptr();
            int ret = manager.AbilityTransitionDone(token, state);
            (void)ret;
            break;
        }
        case 2: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            dataRecord->ability_->SetAbilityState(ACTIVATING);
            manager.dataAbilityRecordsLoading_[name] = dataRecord;
            
            token = dataRecord->ability_->GetToken();
            int ret = manager.AbilityTransitionDone(token, AbilityLifeCycleState::ABILITY_STATE_ACTIVE);
            (void)ret;
            manager.dataAbilityRecordsLoading_.clear();
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 3: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            dataRecord->ability_->SetAbilityState(ACTIVATING);
            manager.dataAbilityRecordsLoading_[name] = dataRecord;
            
            token = dataRecord->ability_->GetToken();
            int ret = manager.AbilityTransitionDone(token, AbilityLifeCycleState::ABILITY_STATE_INACTIVE);
            (void)ret;
            manager.dataAbilityRecordsLoading_.clear();
            break;
        }
        case 4: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            manager.dataAbilityRecordsLoading_[name] = nullptr;
            
            token = sptr<MockAbilityScheduler>::MakeSptr();
            int ret = manager.AbilityTransitionDone(token, state);
            (void)ret;
            manager.dataAbilityRecordsLoading_.clear();
            break;
        }
        case 5: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            dataRecord->scheduler_ = nullptr;
            dataRecord->ability_->SetAbilityState(ACTIVATING);
            manager.dataAbilityRecordsLoading_[name] = dataRecord;
            
            token = dataRecord->ability_->GetToken();
            int ret = manager.AbilityTransitionDone(token, state);
            (void)ret;
            manager.dataAbilityRecordsLoading_.clear();
            break;
        }
        case 6: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            dataRecord->ability_->SetAbilityState(INITIAL);
            manager.dataAbilityRecordsLoading_[name] = dataRecord;
            
            token = dataRecord->ability_->GetToken();
            int ret = manager.AbilityTransitionDone(token, state);
            (void)ret;
            manager.dataAbilityRecordsLoading_.clear();
            break;
        }
        case 7: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord1 = std::make_shared<DataAbilityRecord>(request);
            dataRecord1->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            manager.dataAbilityRecordsLoading_[name] = dataRecord1;
            
            token = dataRecord1->ability_->GetToken();
            int ret = manager.AbilityTransitionDone(token, state);
            (void)ret;
            manager.dataAbilityRecordsLoading_.clear();
            break;
        }
    }
}

void FuzzOnAbilityDied(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 5);
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    
    switch (testCase) {
        case 0: {
            manager.OnAbilityDied(nullptr);
            break;
        }
        case 1: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            abilityRecord = AbilityRecord::CreateAbilityRecord(request);
            if (abilityRecord) {
                abilityRecord->SetAbilityState(ACTIVE);
                manager.OnAbilityDied(abilityRecord);
            }
            break;
        }
        case 2: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            dataRecord->ability_->SetAbilityState(ACTIVE);
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            
            abilityRecord = dataRecord->ability_;
            manager.OnAbilityDied(abilityRecord);
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 3: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            request.abilityInfo.type = AbilityType::PAGE;
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            
            abilityRecord = AbilityRecord::CreateAbilityRecord(request);
            manager.OnAbilityDied(abilityRecord);
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 4: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            manager.dataAbilityRecordsLoaded_[name] = nullptr;
            
            abilityRecord = AbilityRecord::CreateAbilityRecord(request);
            manager.OnAbilityDied(abilityRecord);
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 5: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            dataRecord->ability_->SetAbilityState(ACTIVE);
            
            sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
            sptr<IRemoteObject> client2 = sptr<MockAbilityScheduler>::MakeSptr();
            dataRecord->AddClient(client1, true, false);
            dataRecord->AddClient(client2, false, true);
            
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            manager.dataAbilityRecordsLoaded_["other"] = nullptr;
            
            abilityRecord = dataRecord->ability_;
            manager.OnAbilityDied(abilityRecord);
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
    }
}

void FuzzOnAppStateChanged(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 5);
    AppInfo appInfo;
    
    switch (testCase) {
        case 0: {
            appInfo.bundleName = "";
            manager.OnAppStateChanged(appInfo);
            break;
        }
        case 1: {
            appInfo.bundleName = fdp.ConsumeRandomLengthString(64);
            appInfo.appIndex = fdp.ConsumeIntegral<int32_t>();
            appInfo.instanceKey = fdp.ConsumeRandomLengthString(32);
            appInfo.state = fdp.ConsumeIntegral<int32_t>();
            manager.OnAppStateChanged(appInfo);
            break;
        }
        case 2: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            
            appInfo.bundleName = request.abilityInfo.bundleName;
            appInfo.appIndex = dataRecord->ability_->GetAppIndex();
            appInfo.instanceKey = dataRecord->ability_->GetInstanceKey();
            AppData appData;
            appData.appName = request.appInfo.name;
            appData.uid = request.abilityInfo.applicationInfo.uid;
            appInfo.appData.push_back(appData);
            manager.OnAppStateChanged(appInfo);
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 3: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            manager.dataAbilityRecordsLoaded_[name] = nullptr;
            
            appInfo.bundleName = request.abilityInfo.bundleName;
            manager.OnAppStateChanged(appInfo);
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 4: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            manager.dataAbilityRecordsLoading_[name] = dataRecord;
            
            appInfo.bundleName = request.abilityInfo.bundleName;
            appInfo.appIndex = dataRecord->ability_->GetAppIndex();
            appInfo.instanceKey = dataRecord->ability_->GetInstanceKey();
            AppData appData;
            appData.appName = request.appInfo.name;
            appInfo.appData.push_back(appData);
            manager.OnAppStateChanged(appInfo);
            manager.dataAbilityRecordsLoading_.clear();
            break;
        }
        case 5: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            
            appInfo.bundleName = "different_bundle";
            appInfo.appIndex = 999;
            manager.OnAppStateChanged(appInfo);
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
    }
}

void FuzzGetAbilityRecordById(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 4);
    int64_t id = fdp.ConsumeIntegral<int64_t>();
    
    switch (testCase) {
        case 0: {
            auto record = manager.GetAbilityRecordById(id);
            (void)record;
            break;
        }
        case 1: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            
            id = dataRecord->ability_->GetRecordId();
            auto record = manager.GetAbilityRecordById(id);
            (void)record;
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 2: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            manager.dataAbilityRecordsLoaded_[name] = nullptr;
            
            auto record = manager.GetAbilityRecordById(id);
            (void)record;
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 3: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name1 = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord1 = std::make_shared<DataAbilityRecord>(request);
            dataRecord1->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            manager.dataAbilityRecordsLoaded_[name1] = dataRecord1;
            
            AbilityRequest request2 = CreateDataAbilityRequest(fdp);
            std::string name2 = request2.abilityInfo.bundleName + "." + request2.abilityInfo.name;
            manager.dataAbilityRecordsLoaded_[name2] = nullptr;
            
            id = INT_MAX_VAL;
            auto record = manager.GetAbilityRecordById(id);
            (void)record;
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 4: {
            for (int i = 0; i < 3 && fdp.remaining_bytes() > U32_AT_SIZE; i++) {
                AbilityRequest reqLoop = CreateDataAbilityRequest(fdp);
                std::string nameLoop = reqLoop.abilityInfo.bundleName + "." + reqLoop.abilityInfo.name;
                auto dataRecordLoop = std::make_shared<DataAbilityRecord>(reqLoop);
                dataRecordLoop->ability_ = std::make_shared<AbilityRecord>(reqLoop.want, reqLoop.abilityInfo, reqLoop.appInfo);
                manager.dataAbilityRecordsLoaded_[nameLoop] = dataRecordLoop;
            }
            auto record = manager.GetAbilityRecordById(fdp.ConsumeIntegral<int64_t>());
            (void)record;
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
    }
}

void FuzzGetAbilityRecordByToken(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 5);
    sptr<IRemoteObject> token = nullptr;
    
    switch (testCase) {
        case 0: {
            auto record = manager.GetAbilityRecordByToken(nullptr);
            (void)record;
            break;
        }
        case 1: {
            token = sptr<MockAbilityScheduler>::MakeSptr();
            auto record = manager.GetAbilityRecordByToken(token);
            (void)record;
            break;
        }
        case 2: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            
            token = dataRecord->ability_->GetToken();
            auto record = manager.GetAbilityRecordByToken(token);
            (void)record;
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 3: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            manager.dataAbilityRecordsLoading_[name] = dataRecord;
            
            token = dataRecord->ability_->GetToken();
            auto record = manager.GetAbilityRecordByToken(token);
            (void)record;
            manager.dataAbilityRecordsLoading_.clear();
            break;
        }
        case 4: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            manager.dataAbilityRecordsLoaded_[name] = nullptr;
            manager.dataAbilityRecordsLoading_[name] = nullptr;
            
            token = sptr<MockAbilityScheduler>::MakeSptr();
            auto record = manager.GetAbilityRecordByToken(token);
            (void)record;
            manager.dataAbilityRecordsLoaded_.clear();
            manager.dataAbilityRecordsLoading_.clear();
            break;
        }
        case 5: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            
            token = sptr<MockAbilityScheduler>::MakeSptr();
            auto record = manager.GetAbilityRecordByToken(token);
            (void)record;
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
    }
}

void FuzzGetAbilityRecordByScheduler(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 4);
    sptr<IAbilityScheduler> scheduler = nullptr;
    
    switch (testCase) {
        case 0: {
            auto record = manager.GetAbilityRecordByScheduler(nullptr);
            (void)record;
            break;
        }
        case 1: {
            scheduler = sptr<MockAbilityScheduler>::MakeSptr();
            auto record = manager.GetAbilityRecordByScheduler(scheduler);
            (void)record;
            break;
        }
        case 2: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            
            scheduler = dataRecord->scheduler_;
            auto record = manager.GetAbilityRecordByScheduler(scheduler);
            (void)record;
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 3: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            
            scheduler = sptr<MockAbilityScheduler>::MakeSptr();
            auto record = manager.GetAbilityRecordByScheduler(scheduler);
            (void)record;
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 4: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            manager.dataAbilityRecordsLoaded_[name] = nullptr;
            
            scheduler = sptr<MockAbilityScheduler>::MakeSptr();
            auto record = manager.GetAbilityRecordByScheduler(scheduler);
            (void)record;
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
    }
}

void FuzzDumpFunctions(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 6);
    std::vector<std::string> info;
    std::string args = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    bool isClient = fdp.ConsumeBool();
    std::string func = fdp.ConsumeRandomLengthString(32);
    int line = fdp.ConsumeIntegral<int>();
    
    switch (testCase) {
        case 0: {
            manager.Dump(func.c_str(), line);
            break;
        }
        case 1: {
            manager.Dump(nullptr, line);
            break;
        }
        case 2: {
            manager.DumpState(info, args);
            break;
        }
        case 3: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            
            args = name;
            manager.DumpState(info, args);
            manager.DumpState(info, "nonexistent");
            manager.DumpState(info, "");
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 4: {
            manager.DumpSysState(info, isClient, args);
            break;
        }
        case 5: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            dataRecord->ability_->SetAbilityState(ACTIVE);
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            
            manager.DumpSysState(info, true, name);
            manager.DumpSysState(info, false, "");
            manager.DumpSysState(info, true, "nonexistent");
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 6: {
            manager.DumpLocked(func.c_str(), line);
            manager.DumpLocked(nullptr, -1);
            break;
        }
    }
}

void FuzzGetAbilityRunningInfos(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 3);
    std::vector<AbilityRunningInfo> info;
    bool isPerm = fdp.ConsumeBool();
    
    switch (testCase) {
        case 0: {
            manager.GetAbilityRunningInfos(info, isPerm);
            break;
        }
        case 1: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            
            manager.GetAbilityRunningInfos(info, true);
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 2: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->ability_ = nullptr;
            manager.dataAbilityRecordsLoaded_[name] = dataRecord;
            
            manager.GetAbilityRunningInfos(info, false);
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
        case 3: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::string name = request.abilityInfo.bundleName + "." + request.abilityInfo.name;
            auto dataRecord = std::make_shared<DataAbilityRecord>(request);
            dataRecord->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            manager.dataAbilityRecordsLoading_[name] = dataRecord;
            manager.dataAbilityRecordsLoaded_[name] = nullptr;
            
            manager.GetAbilityRunningInfos(info, isPerm);
            manager.dataAbilityRecordsLoading_.clear();
            manager.dataAbilityRecordsLoaded_.clear();
            break;
        }
    }
}

void FuzzOnAbilityRequestDone(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    sptr<IRemoteObject> token = sptr<MockAbilityScheduler>::MakeSptr();
    int32_t state = fdp.ConsumeIntegral<int32_t>();
    manager.OnAbilityRequestDone(token, state);
    manager.OnAbilityRequestDone(nullptr, state);
}

void FuzzReportFunctions(FuzzedDataProvider& fdp, DataAbilityManager& manager)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 3);
    sptr<IRemoteObject> client = sptr<MockAbilityScheduler>::MakeSptr();
    bool isNotHap = fdp.ConsumeBool();
    std::shared_ptr<DataAbilityRecord> record = nullptr;
    
    switch (testCase) {
        case 0: {
            manager.ReportDataAbilityAcquired(nullptr, isNotHap, record);
            break;
        }
        case 1: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            manager.ReportDataAbilityAcquired(client, false, record);
            break;
        }
        case 2: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            record = std::make_shared<DataAbilityRecord>(request);
            manager.ReportDataAbilityAcquired(client, true, record);
            manager.ReportDataAbilityReleased(client, true, record);
            break;
        }
        case 3: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            manager.ReportDataAbilityReleased(client, false, record);
            manager.ReportDataAbilityReleased(nullptr, isNotHap, record);
            break;
        }
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
    
    int32_t operationType = fdp.ConsumeIntegralInRange<int32_t>(0, 15);
    
    switch (operationType) {
        case 0:
            FuzzAcquire(fdp, *dataAbilityManager);
            break;
        case 1:
            FuzzRelease(fdp, *dataAbilityManager);
            break;
        case 2:
            FuzzContainsDataAbility(fdp, *dataAbilityManager);
            break;
        case 3:
            FuzzAttachAbilityThread(fdp, *dataAbilityManager);
            break;
        case 4:
            FuzzAbilityTransitionDone(fdp, *dataAbilityManager);
            break;
        case 5:
            FuzzOnAbilityDied(fdp, *dataAbilityManager);
            break;
        case 6:
            FuzzOnAppStateChanged(fdp, *dataAbilityManager);
            break;
        case 7:
            FuzzGetAbilityRecordById(fdp, *dataAbilityManager);
            break;
        case 8:
            FuzzGetAbilityRecordByToken(fdp, *dataAbilityManager);
            break;
        case 9:
            FuzzGetAbilityRecordByScheduler(fdp, *dataAbilityManager);
            break;
        case 10:
            FuzzDumpFunctions(fdp, *dataAbilityManager);
            break;
        case 11:
            FuzzGetAbilityRunningInfos(fdp, *dataAbilityManager);
            break;
        case 12:
            FuzzOnAbilityRequestDone(fdp, *dataAbilityManager);
            break;
        case 13:
            FuzzReportFunctions(fdp, *dataAbilityManager);
            break;
        case 14: {
            std::string name = fdp.ConsumeRandomLengthString(64);
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            auto result = dataAbilityManager->LoadLocked(name, request);
            (void)result;
            break;
        }
        case 15: {
            AbilityRequest request = CreateDataAbilityRequest(fdp);
            std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(request);
            if (abilityRecord) {
                dataAbilityManager->RestartDataAbility(abilityRecord);
            }
            dataAbilityManager->RestartDataAbility(nullptr);
            break;
        }
    }
    
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