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

void FuzzStartLoading(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 6);
    AbilityRequest request;
    
    switch (testCase) {
        case 0: {
            request = CreateNonDataAbilityRequest();
            auto record = std::make_shared<DataAbilityRecord>(request);
            int ret = record->StartLoading();
            (void)ret;
            break;
        }
        case 1: {
            request = CreateDataAbilityRequest(fdp);
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            int ret = record->StartLoading();
            (void)ret;
            break;
        }
        case 2: {
            request = CreateDataAbilityRequest(fdp);
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            int ret = record->StartLoading();
            (void)ret;
            break;
        }
        case 3: {
            request = CreateDataAbilityRequest(fdp);
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            int ret = record->StartLoading();
            (void)ret;
            break;
        }
        case 4: {
            request = CreateDataAbilityRequest(fdp);
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = nullptr;
            int ret = record->StartLoading();
            (void)ret;
            break;
        }
        case 5: {
            request = CreateDataAbilityRequest(fdp);
            auto record = std::make_shared<DataAbilityRecord>(request);
            int ret1 = record->StartLoading();
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            int ret2 = record->StartLoading();
            (void)ret1;
            (void)ret2;
            break;
        }
        case 6: {
            request = CreateDataAbilityRequest(fdp);
            auto record = std::make_shared<DataAbilityRecord>(request);
            int ret = record->StartLoading();
            (void)ret;
            break;
        }
    }
}

void FuzzWaitForLoaded(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 6);
    ffrt::mutex mutex;
    system_clock::duration timeout = milliseconds(TIMEOUT_MS);
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    
    switch (testCase) {
        case 0: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = nullptr;
            int ret = record->WaitForLoaded(mutex, timeout);
            (void)ret;
            break;
        }
        case 1: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->ability_->SetAbilityState(ACTIVE);
            int ret = record->WaitForLoaded(mutex, timeout);
            (void)ret;
            break;
        }
        case 2: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->ability_->SetAbilityState(INITIAL);
            int ret = record->WaitForLoaded(mutex, timeout);
            (void)ret;
            break;
        }
        case 3: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->ability_->SetAbilityState(ACTIVATING);
            int ret = record->WaitForLoaded(mutex, timeout);
            (void)ret;
            break;
        }
        case 4: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = nullptr;
            record->ability_->SetAbilityState(ACTIVE);
            int ret = record->WaitForLoaded(mutex, timeout);
            (void)ret;
            break;
        }
        case 5: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            record->ability_->SetAbilityState(INITIAL);
            int ret = record->WaitForLoaded(mutex, timeout);
            (void)ret;
            break;
        }
        case 6: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            record->ability_->SetAbilityState(ACTIVE);
            int ret = record->WaitForLoaded(mutex, timeout);
            (void)ret;
            break;
        }
    }
}

void FuzzGetScheduler(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 5);
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    
    switch (testCase) {
        case 0: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = nullptr;
            auto scheduler = record->GetScheduler();
            (void)scheduler;
            break;
        }
        case 1: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = nullptr;
            auto scheduler = record->GetScheduler();
            (void)scheduler;
            break;
        }
        case 2: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            record->ability_->SetAbilityState(INITIAL);
            auto scheduler = record->GetScheduler();
            (void)scheduler;
            break;
        }
        case 3: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            record->ability_->SetAbilityState(ACTIVATING);
            auto scheduler = record->GetScheduler();
            (void)scheduler;
            break;
        }
        case 4: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            record->ability_->SetAbilityState(ACTIVE);
            auto scheduler = record->GetScheduler();
            (void)scheduler;
            break;
        }
        case 5: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            auto scheduler = record->GetScheduler();
            (void)scheduler;
            break;
        }
    }
}

void FuzzAttach(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 6);
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    sptr<MockAbilityScheduler> scheduler = sptr<MockAbilityScheduler>::MakeSptr();
    
    switch (testCase) {
        case 0: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            int ret = record->Attach(nullptr);
            (void)ret;
            break;
        }
        case 1: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = nullptr;
            int ret = record->Attach(scheduler);
            (void)ret;
            break;
        }
        case 2: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            int ret = record->Attach(scheduler);
            (void)ret;
            break;
        }
        case 3: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->ability_->SetAbilityState(ACTIVATING);
            int ret = record->Attach(scheduler);
            (void)ret;
            break;
        }
        case 4: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->ability_->SetAbilityState(ACTIVE);
            int ret = record->Attach(scheduler);
            (void)ret;
            break;
        }
        case 5: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->ability_->SetAbilityState(INITIAL);
            int ret = record->Attach(scheduler);
            (void)ret;
            break;
        }
        case 6: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->ability_->SetAbilityState(INITIAL);
            int ret1 = record->Attach(scheduler);
            sptr<MockAbilityScheduler> scheduler2 = sptr<MockAbilityScheduler>::MakeSptr();
            int ret2 = record->Attach(scheduler2);
            (void)ret1;
            (void)ret2;
            break;
        }
    }
}

void FuzzOnTransitionDone(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 7);
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    int state = fdp.ConsumeIntegralInRange<int>(0, 10);
    
    switch (testCase) {
        case 0: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = nullptr;
            int ret = record->OnTransitionDone(state);
            (void)ret;
            break;
        }
        case 1: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = nullptr;
            int ret = record->OnTransitionDone(state);
            (void)ret;
            break;
        }
        case 2: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            record->ability_->SetAbilityState(INITIAL);
            int ret = record->OnTransitionDone(state);
            (void)ret;
            break;
        }
        case 3: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            record->ability_->SetAbilityState(ACTIVATING);
            int ret = record->OnTransitionDone(AbilityLifeCycleState::ABILITY_STATE_INACTIVE);
            (void)ret;
            break;
        }
        case 4: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            record->ability_->SetAbilityState(ACTIVATING);
            int ret = record->OnTransitionDone(AbilityLifeCycleState::ABILITY_STATE_FOREGROUND);
            (void)ret;
            break;
        }
        case 5: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            record->ability_->SetAbilityState(ACTIVATING);
            int ret = record->OnTransitionDone(AbilityLifeCycleState::ABILITY_STATE_BACKGROUND);
            (void)ret;
            break;
        }
        case 6: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            record->ability_->SetAbilityState(ACTIVATING);
            int ret = record->OnTransitionDone(AbilityLifeCycleState::ABILITY_STATE_ACTIVE);
            (void)ret;
            break;
        }
        case 7: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = sptr<MockAbilityScheduler>::MakeSptr();
            record->ability_->SetAbilityState(ACTIVE);
            int ret = record->OnTransitionDone(state);
            (void)ret;
            break;
        }
    }
}

void FuzzAddClient(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 8);
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    sptr<MockAbilityScheduler> scheduler = sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> client = sptr<MockAbilityScheduler>::MakeSptr();
    bool tryBind = fdp.ConsumeBool();
    bool isNotHap = fdp.ConsumeBool();
    
    switch (testCase) {
        case 0: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            int ret = record->AddClient(nullptr, tryBind, isNotHap);
            (void)ret;
            break;
        }
        case 1: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = nullptr;
            int ret = record->AddClient(client, tryBind, isNotHap);
            (void)ret;
            break;
        }
        case 2: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = nullptr;
            int ret = record->AddClient(client, tryBind, isNotHap);
            (void)ret;
            break;
        }
        case 3: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(INITIAL);
            int ret = record->AddClient(client, tryBind, isNotHap);
            (void)ret;
            break;
        }
        case 4: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVATING);
            int ret = record->AddClient(client, tryBind, isNotHap);
            (void)ret;
            break;
        }
        case 5: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            int ret = record->AddClient(client, tryBind, isNotHap);
            (void)ret;
            break;
        }
        case 6: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            record->callerDeathRecipient_ = sptr<DataAbilityCallerRecipient>::MakeSptr([](const wptr<IRemoteObject>&) {});
            int ret = record->AddClient(client, tryBind, isNotHap);
            (void)ret;
            break;
        }
        case 7: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
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
            break;
        }
        case 8: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                sptr<IRemoteObject> clientLoop = sptr<MockAbilityScheduler>::MakeSptr();
                int ret = record->AddClient(clientLoop, fdp.ConsumeBool(), fdp.ConsumeBool());
                (void)ret;
            }
            break;
        }
    }
}

void FuzzRemoveClient(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 8);
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    sptr<MockAbilityScheduler> scheduler = sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> client = sptr<MockAbilityScheduler>::MakeSptr();
    bool isNotHap = fdp.ConsumeBool();
    
    switch (testCase) {
        case 0: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            int ret = record->RemoveClient(nullptr, isNotHap);
            (void)ret;
            break;
        }
        case 1: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = nullptr;
            int ret = record->RemoveClient(client, isNotHap);
            (void)ret;
            break;
        }
        case 2: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = nullptr;
            int ret = record->RemoveClient(client, isNotHap);
            (void)ret;
            break;
        }
        case 3: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(INITIAL);
            int ret = record->RemoveClient(client, isNotHap);
            (void)ret;
            break;
        }
        case 4: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            record->clients_.clear();
            int ret = record->RemoveClient(client, isNotHap);
            (void)ret;
            break;
        }
        case 5: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
            record->AddClient(client1, true, false);
            int ret = record->RemoveClient(client, isNotHap);
            (void)ret;
            break;
        }
        case 6: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
            record->AddClient(client1, true, false);
            int ret = record->RemoveClient(client1, isNotHap);
            (void)ret;
            break;
        }
        case 7: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
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
            break;
        }
        case 8: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            for (int i = 0; i < 3; i++) {
                sptr<IRemoteObject> clientLoop = sptr<MockAbilityScheduler>::MakeSptr();
                record->AddClient(clientLoop, true, false);
            }
            auto it = record->clients_.begin();
            while (it != record->clients_.end()) {
                sptr<IRemoteObject> clientLoop = it->client;
                int ret = record->RemoveClient(clientLoop, it->isNotHap);
                (void)ret;
                it = record->clients_.begin();
            }
            break;
        }
    }
}

void FuzzRemoveClients(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 7);
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    sptr<MockAbilityScheduler> scheduler = sptr<MockAbilityScheduler>::MakeSptr();
    
    switch (testCase) {
        case 0: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = nullptr;
            int ret = record->RemoveClients(nullptr);
            (void)ret;
            break;
        }
        case 1: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = nullptr;
            int ret = record->RemoveClients(nullptr);
            (void)ret;
            break;
        }
        case 2: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(INITIAL);
            int ret = record->RemoveClients(nullptr);
            (void)ret;
            break;
        }
        case 3: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            record->clients_.clear();
            int ret = record->RemoveClients(nullptr);
            (void)ret;
            break;
        }
        case 4: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
            record->AddClient(client1, true, false);
            int ret = record->RemoveClients(nullptr);
            (void)ret;
            break;
        }
        case 5: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
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
            auto clientRecord = std::make_shared<AbilityRecord>(clientRequest.want, clientRequest.abilityInfo, clientRequest.appInfo);
            
            int ret = record->RemoveClients(clientRecord);
            (void)ret;
            break;
        }
        case 6: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
            sptr<IRemoteObject> client2 = sptr<MockAbilityScheduler>::MakeSptr();
            record->AddClient(client1, true, false);
            record->AddClient(client2, false, true);
            
            auto clientRecord = Token::GetAbilityRecordByToken(client1);
            int ret = record->RemoveClients(clientRecord);
            (void)ret;
            break;
        }
        case 7: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                sptr<IRemoteObject> clientLoop = sptr<MockAbilityScheduler>::MakeSptr();
                record->AddClient(clientLoop, fdp.ConsumeBool(), fdp.ConsumeBool());
            }
            int ret = record->RemoveClients(nullptr);
            (void)ret;
            break;
        }
    }
}

void FuzzGetClientCount(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 6);
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    sptr<MockAbilityScheduler> scheduler = sptr<MockAbilityScheduler>::MakeSptr();
    sptr<IRemoteObject> client = sptr<MockAbilityScheduler>::MakeSptr();
    
    switch (testCase) {
        case 0: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = nullptr;
            size_t count = record->GetClientCount(client);
            (void)count;
            break;
        }
        case 1: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = nullptr;
            size_t count = record->GetClientCount(client);
            (void)count;
            break;
        }
        case 2: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(INITIAL);
            size_t count = record->GetClientCount(client);
            (void)count;
            break;
        }
        case 3: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            size_t count = record->GetClientCount(nullptr);
            (void)count;
            break;
        }
        case 4: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
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
            break;
        }
        case 5: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            record->AddClient(client, true, false);
            size_t count = record->GetClientCount(client);
            (void)count;
            break;
        }
        case 6: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            auto count = record->GetClientCount();
            (void)count;
            break;
        }
    }
}

void FuzzKillBoundClientProcesses(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 5);
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    sptr<MockAbilityScheduler> scheduler = sptr<MockAbilityScheduler>::MakeSptr();
    
    switch (testCase) {
        case 0: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = nullptr;
            int ret = record->KillBoundClientProcesses();
            (void)ret;
            break;
        }
        case 1: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = nullptr;
            int ret = record->KillBoundClientProcesses();
            (void)ret;
            break;
        }
        case 2: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(INITIAL);
            int ret = record->KillBoundClientProcesses();
            (void)ret;
            break;
        }
        case 3: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
            sptr<IRemoteObject> client2 = sptr<MockAbilityScheduler>::MakeSptr();
            record->AddClient(client1, true, false);
            record->AddClient(client2, false, true);
            int ret = record->KillBoundClientProcesses();
            (void)ret;
            break;
        }
        case 4: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            int ret = record->KillBoundClientProcesses();
            (void)ret;
            break;
        }
        case 5: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                sptr<IRemoteObject> clientLoop = sptr<MockAbilityScheduler>::MakeSptr();
                record->AddClient(clientLoop, fdp.ConsumeBool(), fdp.ConsumeBool());
            }
            int ret = record->KillBoundClientProcesses();
            (void)ret;
            break;
        }
    }
}

void FuzzDump(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 5);
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    sptr<MockAbilityScheduler> scheduler = sptr<MockAbilityScheduler>::MakeSptr();
    std::vector<std::string> info;
    
    switch (testCase) {
        case 0: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = nullptr;
            record->Dump();
            record->Dump(info);
            break;
        }
        case 1: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            record->Dump();
            break;
        }
        case 2: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
            sptr<IRemoteObject> client2 = sptr<MockAbilityScheduler>::MakeSptr();
            record->AddClient(client1, true, false);
            record->AddClient(client2, false, true);
            record->Dump(info);
            break;
        }
        case 3: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                sptr<IRemoteObject> clientLoop = sptr<MockAbilityScheduler>::MakeSptr();
                record->AddClient(clientLoop, fdp.ConsumeBool(), fdp.ConsumeBool());
            }
            record->Dump(info);
            break;
        }
        case 4: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            record->Dump(info);
            break;
        }
        case 5: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->Dump();
            break;
        }
    }
}

void FuzzOnSchedulerDied(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 5);
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    sptr<MockAbilityScheduler> scheduler = sptr<MockAbilityScheduler>::MakeSptr();
    wptr<IRemoteObject> remote = sptr<MockAbilityScheduler>::MakeSptr();
    
    switch (testCase) {
        case 0: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = nullptr;
            record->OnSchedulerDied(remote);
            break;
        }
        case 1: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            record->clients_.clear();
            record->OnSchedulerDied(remote);
            break;
        }
        case 2: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
            sptr<IRemoteObject> client2 = sptr<MockAbilityScheduler>::MakeSptr();
            record->AddClient(client1, true, false);
            record->AddClient(client2, false, true);
            record->OnSchedulerDied(remote);
            break;
        }
        case 3: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
            record->AddClient(client1, false, true);
            wptr<IRemoteObject> remoteClient = client1;
            record->OnSchedulerDied(remoteClient);
            break;
        }
        case 4: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
            sptr<IRemoteObject> client2 = sptr<MockAbilityScheduler>::MakeSptr();
            record->AddClient(client1, true, false);
            record->AddClient(client2, false, true);
            wptr<IRemoteObject> remoteNull = nullptr;
            record->OnSchedulerDied(remoteNull);
            break;
        }
        case 5: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->OnSchedulerDied(remote);
            break;
        }
    }
}

void FuzzGetDiedCallerPid(FuzzedDataProvider& fdp)
{
    int32_t testCase = fdp.ConsumeIntegralInRange<int32_t>(0, 3);
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    sptr<MockAbilityScheduler> scheduler = sptr<MockAbilityScheduler>::MakeSptr();
    
    switch (testCase) {
        case 0: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            int32_t pid = record->GetDiedCallerPid(nullptr);
            (void)pid;
            break;
        }
        case 1: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            sptr<IRemoteObject> remote = sptr<MockAbilityScheduler>::MakeSptr();
            int32_t pid = record->GetDiedCallerPid(remote);
            (void)pid;
            break;
        }
        case 2: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
            record->scheduler_ = scheduler;
            record->ability_->SetAbilityState(ACTIVE);
            sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
            record->AddClient(client1, true, false);
            int32_t pid = record->GetDiedCallerPid(client1);
            (void)pid;
            break;
        }
        case 3: {
            auto record = std::make_shared<DataAbilityRecord>(request);
            sptr<IRemoteObject> client1 = sptr<MockAbilityScheduler>::MakeSptr();
            sptr<IRemoteObject> client2 = sptr<MockAbilityScheduler>::MakeSptr();
            record->AddClient(client1, true, false);
            record->AddClient(client2, false, true);
            int32_t pid = record->GetDiedCallerPid(client2);
            (void)pid;
            break;
        }
    }
}

void FuzzGetters(FuzzedDataProvider& fdp)
{
    AbilityRequest request = CreateDataAbilityRequest(fdp);
    sptr<MockAbilityScheduler> scheduler = sptr<MockAbilityScheduler>::MakeSptr();
    
    auto record = std::make_shared<DataAbilityRecord>(request);
    record->ability_ = std::make_shared<AbilityRecord>(request.want, request.abilityInfo, request.appInfo);
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
    int32_t operationType = fdp.ConsumeIntegralInRange<int32_t>(0, 13);
    
    switch (operationType) {
        case 0:
            FuzzStartLoading(fdp);
            break;
        case 1:
            FuzzWaitForLoaded(fdp);
            break;
        case 2:
            FuzzGetScheduler(fdp);
            break;
        case 3:
            FuzzAttach(fdp);
            break;
        case 4:
            FuzzOnTransitionDone(fdp);
            break;
        case 5:
            FuzzAddClient(fdp);
            break;
        case 6:
            FuzzRemoveClient(fdp);
            break;
        case 7:
            FuzzRemoveClients(fdp);
            break;
        case 8:
            FuzzGetClientCount(fdp);
            break;
        case 9:
            FuzzKillBoundClientProcesses(fdp);
            break;
        case 10:
            FuzzDump(fdp);
            break;
        case 11:
            FuzzOnSchedulerDied(fdp);
            break;
        case 12:
            FuzzGetDiedCallerPid(fdp);
            break;
        case 13:
            FuzzGetters(fdp);
            break;
    }
    
    return true;
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}