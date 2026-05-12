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

#include "abilitystartwithwaitobservermanager_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#define protected public
#include "ability_start_with_wait_observer_manager.h"
#include "ability_start_with_wait_observer_stub.h"
#include "ability_start_with_wait_observer_proxy.h"
#include "ability_start_with_wait_observer_utils.h"
#undef protected
#undef private
#include "securec.h"
#include "ability_fuzz_util.h"
#include "ability_record.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t U32_AT_SIZE = 4;
constexpr int32_t MAX_TEST_LOOP = 100;
constexpr int32_t INT_MAX_VALUE = 2147483647;
using TerminateReason = AbilityStartWithWaitObserverUtil::TerminateReason;

class MockAbilityStartWithWaitObserver : public AbilityStartWithWaitObserverStub {
public:
    int32_t NotifyAATerminateWait(const AbilityStartWithWaitObserverData &abilityStartWithWaitData) override
    {
        return 0;
    }
};

sptr<IAbilityStartWithWaitObserver> CreateValidObserver()
{
    sptr<IRemoteObject> observerStub = sptr<MockAbilityStartWithWaitObserver>::MakeSptr();
    return sptr<AbilityStartWithWaitObserverProxy>::MakeSptr(observerStub);
}

sptr<IAbilityStartWithWaitObserver> CreateNullProxyObserver()
{
    sptr<IRemoteObject> observerStub = nullptr;
    return sptr<AbilityStartWithWaitObserverProxy>::MakeSptr(observerStub);
}

void FuzzRegisterObserverNullObserver(AbilityStartWithWaitObserverManager& manager)
{
    Want want;
    sptr<IAbilityStartWithWaitObserver> observer = nullptr;
    manager.RegisterObserver(want, observer);
}

void FuzzRegisterObserverNullProxy(AbilityStartWithWaitObserverManager& manager)
{
    Want want;
    sptr<IAbilityStartWithWaitObserver> observer = CreateNullProxyObserver();
    manager.RegisterObserver(want, observer);
}

void FuzzRegisterObserverValid(AbilityStartWithWaitObserverManager& manager, Want& want)
{
    sptr<IAbilityStartWithWaitObserver> observer = CreateValidObserver();
    manager.RegisterObserver(want, observer);
}

void FuzzRegisterObserverDuplicate(AbilityStartWithWaitObserverManager& manager)
{
    Want want;
    sptr<IAbilityStartWithWaitObserver> observer = CreateValidObserver();
    manager.RegisterObserver(want, observer);
    Want want2;
    manager.RegisterObserver(want2, observer);
}

void FuzzRegisterObserverWithNullInList(AbilityStartWithWaitObserverManager& manager)
{
    manager.observerList_.emplace(nullptr, AbilityForegroundInfo());
    Want want;
    sptr<IAbilityStartWithWaitObserver> observer = CreateValidObserver();
    manager.RegisterObserver(want, observer);
    manager.observerList_.clear();
}

void FuzzRegisterObserverMaxId(AbilityStartWithWaitObserverManager& manager)
{
    manager.observerId_ = INT_MAX_VALUE - 1;
    Want want1;
    sptr<IAbilityStartWithWaitObserver> observer1 = CreateValidObserver();
    manager.RegisterObserver(want1, observer1);
    
    Want want2;
    sptr<IAbilityStartWithWaitObserver> observer2 = CreateValidObserver();
    manager.RegisterObserver(want2, observer2);
    manager.observerId_ = 0;
}

void FuzzUnregisterObserverNull(AbilityStartWithWaitObserverManager& manager)
{
    sptr<IAbilityStartWithWaitObserver> observer = nullptr;
    manager.UnregisterObserver(observer);
}

void FuzzUnregisterObserverNullProxy(AbilityStartWithWaitObserverManager& manager)
{
    sptr<IAbilityStartWithWaitObserver> observer = CreateNullProxyObserver();
    manager.UnregisterObserver(observer);
}

void FuzzUnregisterObserverNotFound(AbilityStartWithWaitObserverManager& manager)
{
    sptr<IAbilityStartWithWaitObserver> observer = CreateValidObserver();
    manager.UnregisterObserver(observer);
}

void FuzzUnregisterObserverValid(AbilityStartWithWaitObserverManager& manager)
{
    Want want;
    sptr<IAbilityStartWithWaitObserver> observer = CreateValidObserver();
    manager.RegisterObserver(want, observer);
    manager.UnregisterObserver(observer);
}

void FuzzUnregisterObserverWithNullInList(AbilityStartWithWaitObserverManager& manager)
{
    Want want;
    sptr<IAbilityStartWithWaitObserver> observer = CreateValidObserver();
    manager.RegisterObserver(want, observer);
    manager.observerList_.emplace(nullptr, AbilityForegroundInfo());
    manager.UnregisterObserver(observer);
    manager.observerList_.clear();
}

void FuzzNotifyAATerminateWaitWantInvalidId(AbilityStartWithWaitObserverManager& manager)
{
    Want want;
    manager.NotifyAATerminateWait(want, TerminateReason::TERMINATE_FOR_NONE);
}

void FuzzNotifyAATerminateWaitWantNotFound(AbilityStartWithWaitObserverManager& manager)
{
    Want want;
    want.SetParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, INT_MAX_VALUE);
    manager.NotifyAATerminateWait(want, TerminateReason::TERMINATE_FOR_NON_UI_ABILITY);
}

void FuzzNotifyAATerminateWaitWantValid(FuzzedDataProvider& fdp, AbilityStartWithWaitObserverManager& manager)
{
    Want want;
    sptr<IAbilityStartWithWaitObserver> observer = CreateValidObserver();
    int32_t observerId = manager.RegisterObserver(want, observer);
    Want notifyWant;
    notifyWant.SetParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, observerId);
    TerminateReason reason = static_cast<TerminateReason>(fdp.ConsumeIntegralInRange<uint32_t>(0, 2));
    manager.NotifyAATerminateWait(notifyWant, reason);
}

void FuzzNotifyAATerminateWaitWantWithNullInList(AbilityStartWithWaitObserverManager& manager)
{
    manager.observerList_.emplace(nullptr, AbilityForegroundInfo());
    Want want;
    want.SetParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, 0);
    manager.NotifyAATerminateWait(want, TerminateReason::TERMINATE_FOR_NONE);
    manager.observerList_.clear();
}

void FuzzNotifyAATerminateWaitWantNullDeathRecipient(AbilityStartWithWaitObserverManager& manager)
{
    Want want;
    sptr<IAbilityStartWithWaitObserver> observer = CreateValidObserver();
    int32_t observerId = manager.RegisterObserver(want, observer);
    
    auto it = manager.observerList_.find(observer);
    if (it != manager.observerList_.end()) {
        it->second.deathRecipient = nullptr;
    }
    
    Want notifyWant;
    notifyWant.SetParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, observerId);
    manager.NotifyAATerminateWait(notifyWant, TerminateReason::TERMINATE_FOR_NONE);
}

void FuzzNotifyAATerminateWaitWantWrongId(AbilityStartWithWaitObserverManager& manager)
{
    Want want;
    sptr<IAbilityStartWithWaitObserver> observer = CreateValidObserver();
    int32_t observerId = manager.RegisterObserver(want, observer);
    
    Want notifyWant;
    notifyWant.SetParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, observerId + 100);
    manager.NotifyAATerminateWait(notifyWant, TerminateReason::TERMINATE_FOR_NONE);
    manager.UnregisterObserver(observer);
}

void FuzzNotifyAATerminateWaitRecordNull(AbilityStartWithWaitObserverManager& manager)
{
    manager.NotifyAATerminateWait(nullptr, TerminateReason::TERMINATE_FOR_NONE);
}

void FuzzNotifyAATerminateWaitRecordInvalidId(FuzzedDataProvider& fdp, AbilityStartWithWaitObserverManager& manager)
{
    AbilityRequest info;
    AbilityFuzzUtil::GetRandomAbilityRequestInfo(fdp, info);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(info);
    if (abilityRecord) {
        abilityRecord->RemoveSpecifiedWantParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY);
        manager.NotifyAATerminateWait(abilityRecord, TerminateReason::TERMINATE_FOR_UI_ABILITY_FOREGROUND_FAILED);
    }
}

void FuzzNotifyAATerminateWaitRecordNotFound(FuzzedDataProvider& fdp, AbilityStartWithWaitObserverManager& manager)
{
    AbilityRequest info;
    AbilityFuzzUtil::GetRandomAbilityRequestInfo(fdp, info);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(info);
    if (abilityRecord) {
        abilityRecord->want_.SetParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, INT_MAX_VALUE);
        manager.NotifyAATerminateWait(abilityRecord, TerminateReason::TERMINATE_FOR_NONE);
    }
}

void FuzzNotifyAATerminateWaitRecordValid(FuzzedDataProvider& fdp, AbilityStartWithWaitObserverManager& manager)
{
    Want want;
    sptr<IAbilityStartWithWaitObserver> observer = CreateValidObserver();
    int32_t observerId = manager.RegisterObserver(want, observer);
    
    AbilityRequest info;
    AbilityFuzzUtil::GetRandomAbilityRequestInfo(fdp, info);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(info);
    if (abilityRecord) {
        abilityRecord->want_.SetParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, observerId);
        TerminateReason reason = static_cast<TerminateReason>(fdp.ConsumeIntegralInRange<uint32_t>(0, 2));
        manager.NotifyAATerminateWait(abilityRecord, reason);
    }
}

void FuzzNotifyAATerminateWaitRecordWithNullInList(FuzzedDataProvider& fdp, AbilityStartWithWaitObserverManager& manager)
{
    manager.observerList_.emplace(nullptr, AbilityForegroundInfo());
    
    AbilityRequest info;
    AbilityFuzzUtil::GetRandomAbilityRequestInfo(fdp, info);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(info);
    if (abilityRecord) {
        abilityRecord->want_.SetParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, 0);
        manager.NotifyAATerminateWait(abilityRecord, TerminateReason::TERMINATE_FOR_NONE);
    }
    manager.observerList_.clear();
}

void FuzzNotifyAATerminateWaitRecordNullDeathRecipient(FuzzedDataProvider& fdp, AbilityStartWithWaitObserverManager& manager)
{
    Want want;
    sptr<IAbilityStartWithWaitObserver> observer = CreateValidObserver();
    int32_t observerId = manager.RegisterObserver(want, observer);
    
    auto it = manager.observerList_.find(observer);
    if (it != manager.observerList_.end()) {
        it->second.deathRecipient = nullptr;
    }
    
    AbilityRequest info;
    AbilityFuzzUtil::GetRandomAbilityRequestInfo(fdp, info);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(info);
    if (abilityRecord) {
        abilityRecord->want_.SetParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, observerId);
        manager.NotifyAATerminateWait(abilityRecord, TerminateReason::TERMINATE_FOR_NONE);
    }
}

void FuzzNotifyAATerminateWaitRecordWrongId(FuzzedDataProvider& fdp, AbilityStartWithWaitObserverManager& manager)
{
    Want want;
    sptr<IAbilityStartWithWaitObserver> observer = CreateValidObserver();
    int32_t observerId = manager.RegisterObserver(want, observer);
    
    AbilityRequest info;
    AbilityFuzzUtil::GetRandomAbilityRequestInfo(fdp, info);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(info);
    if (abilityRecord) {
        abilityRecord->want_.SetParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, observerId + 100);
        manager.NotifyAATerminateWait(abilityRecord, TerminateReason::TERMINATE_FOR_NONE);
    }
    manager.UnregisterObserver(observer);
}

void FuzzSetColdStartNullRecord(AbilityStartWithWaitObserverManager& manager)
{
    manager.SetColdStartForShellCall(nullptr);
}

void FuzzSetColdStartInvalidId(FuzzedDataProvider& fdp, AbilityStartWithWaitObserverManager& manager)
{
    AbilityRequest info;
    AbilityFuzzUtil::GetRandomAbilityRequestInfo(fdp, info);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(info);
    if (abilityRecord) {
        abilityRecord->RemoveSpecifiedWantParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY);
        manager.SetColdStartForShellCall(abilityRecord);
    }
}

void FuzzSetColdStartNotFound(FuzzedDataProvider& fdp, AbilityStartWithWaitObserverManager& manager)
{
    AbilityRequest info;
    AbilityFuzzUtil::GetRandomAbilityRequestInfo(fdp, info);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(info);
    if (abilityRecord) {
        abilityRecord->want_.SetParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, INT_MAX_VALUE);
        manager.SetColdStartForShellCall(abilityRecord);
    }
}

void FuzzSetColdStartValid(FuzzedDataProvider& fdp, AbilityStartWithWaitObserverManager& manager)
{
    Want want;
    sptr<IAbilityStartWithWaitObserver> observer = CreateValidObserver();
    int32_t observerId = manager.RegisterObserver(want, observer);
    
    AbilityRequest info;
    AbilityFuzzUtil::GetRandomAbilityRequestInfo(fdp, info);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(info);
    if (abilityRecord) {
        abilityRecord->want_.SetParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, observerId);
        manager.SetColdStartForShellCall(abilityRecord);
    }
}

void FuzzSetColdStartWithNullInList(FuzzedDataProvider& fdp, AbilityStartWithWaitObserverManager& manager)
{
    manager.observerList_.emplace(nullptr, AbilityForegroundInfo());
    
    AbilityRequest info;
    AbilityFuzzUtil::GetRandomAbilityRequestInfo(fdp, info);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(info);
    if (abilityRecord) {
        abilityRecord->want_.SetParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, 0);
        manager.SetColdStartForShellCall(abilityRecord);
    }
    manager.observerList_.clear();
}

void FuzzSetColdStartWrongId(FuzzedDataProvider& fdp, AbilityStartWithWaitObserverManager& manager)
{
    Want want;
    sptr<IAbilityStartWithWaitObserver> observer = CreateValidObserver();
    int32_t observerId = manager.RegisterObserver(want, observer);
    
    AbilityRequest info;
    AbilityFuzzUtil::GetRandomAbilityRequestInfo(fdp, info);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(info);
    if (abilityRecord) {
        abilityRecord->want_.SetParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, observerId + 100);
        manager.SetColdStartForShellCall(abilityRecord);
    }
    manager.UnregisterObserver(observer);
}

void FuzzGenerateDeathRecipientNullObserver(AbilityStartWithWaitObserverManager& manager)
{
    sptr<IAbilityStartWithWaitObserver> observer = nullptr;
    manager.GenerateDeathRecipient(observer);
}

void FuzzGenerateDeathRecipientNullProxy(AbilityStartWithWaitObserverManager& manager)
{
    sptr<IAbilityStartWithWaitObserver> observer = CreateNullProxyObserver();
    manager.GenerateDeathRecipient(observer);
}

void FuzzGenerateDeathRecipientValid(AbilityStartWithWaitObserverManager& manager)
{
    sptr<IAbilityStartWithWaitObserver> observer = CreateValidObserver();
    auto recipient = manager.GenerateDeathRecipient(observer);
}

void FuzzGenerateDeathRecipientCallbackNull(AbilityStartWithWaitObserverManager& manager)
{
    sptr<IAbilityStartWithWaitObserver> observer = CreateValidObserver();
    auto recipient = manager.GenerateDeathRecipient(observer);
    if (recipient) {
        wptr<IRemoteObject> remote = nullptr;
        recipient->OnRemoteDied(remote);
    }
}

void FuzzGenerateDeathRecipientCallbackValid(AbilityStartWithWaitObserverManager& manager)
{
    sptr<IRemoteObject> observerStub = sptr<MockAbilityStartWithWaitObserver>::MakeSptr();
    sptr<IAbilityStartWithWaitObserver> observer = sptr<AbilityStartWithWaitObserverProxy>::MakeSptr(observerStub);
    auto recipient = manager.GenerateDeathRecipient(observer);
    if (recipient && observerStub) {
        wptr<IRemoteObject> remote = observerStub;
        recipient->OnRemoteDied(remote);
    }
}

void FuzzMultipleObserversScenario(FuzzedDataProvider& fdp, AbilityStartWithWaitObserverManager& manager)
{
    int32_t count = fdp.ConsumeIntegralInRange<int32_t>(1, 5);
    std::vector<sptr<IAbilityStartWithWaitObserver>> observers;
    std::vector<int32_t> observerIds;
    
    for (int32_t i = 0; i < count; ++i) {
        Want want;
        sptr<IAbilityStartWithWaitObserver> observer = CreateValidObserver();
        int32_t observerId = manager.RegisterObserver(want, observer);
        observers.push_back(observer);
        observerIds.push_back(observerId);
    }
    
    for (size_t i = 0; i < observers.size(); ++i) {
        Want want;
        want.SetParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, observerIds[i]);
        TerminateReason reason = static_cast<TerminateReason>(fdp.ConsumeIntegralInRange<uint32_t>(0, 2));
        manager.NotifyAATerminateWait(want, reason);
    }
    
    for (auto& observer : observers) {
        manager.UnregisterObserver(observer);
    }
}

void FuzzCompleteLifecycleScenario(FuzzedDataProvider& fdp, AbilityStartWithWaitObserverManager& manager)
{
    Want want;
    sptr<IAbilityStartWithWaitObserver> observer = CreateValidObserver();
    int32_t observerId = manager.RegisterObserver(want, observer);
    
    AbilityRequest info;
    AbilityFuzzUtil::GetRandomAbilityRequestInfo(fdp, info);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(info);
    
    if (abilityRecord && observerId >= 0) {
        abilityRecord->want_.SetParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, observerId);
        manager.SetColdStartForShellCall(abilityRecord);
        
        TerminateReason reason = static_cast<TerminateReason>(fdp.ConsumeIntegralInRange<uint32_t>(0, 2));
        manager.NotifyAATerminateWait(abilityRecord, reason);
    }
    
    manager.UnregisterObserver(observer);
}

void FuzzObserverListEdgeCases(FuzzedDataProvider& fdp, AbilityStartWithWaitObserverManager& manager)
{
    manager.observerList_.clear();
    
    AbilityForegroundInfo info1;
    info1.observerId = 1;
    info1.bundleName = "test_bundle";
    info1.abilityName = "test_ability";
    info1.coldStart = true;
    info1.startTime = 1000;
    
    AbilityForegroundInfo info2;
    info2.observerId = 2;
    info2.bundleName = fdp.ConsumeRandomLengthString(32);
    info2.abilityName = fdp.ConsumeRandomLengthString(32);
    info2.coldStart = fdp.ConsumeBool();
    
    sptr<IAbilityStartWithWaitObserver> observer1 = CreateValidObserver();
    sptr<IAbilityStartWithWaitObserver> observer2 = CreateValidObserver();
    
    manager.observerList_.emplace(observer1, info1);
    manager.observerList_.emplace(observer2, info2);
    
    Want want;
    want.SetParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, 1);
    manager.NotifyAATerminateWait(want, TerminateReason::TERMINATE_FOR_NONE);
    
    want.SetParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, 2);
    manager.NotifyAATerminateWait(want, TerminateReason::TERMINATE_FOR_NON_UI_ABILITY);
}

void FuzzMaxObserverIdEdgeCase(AbilityStartWithWaitObserverManager& manager)
{
    manager.observerId_ = INT_MAX_VALUE;
    Want want;
    sptr<IAbilityStartWithWaitObserver> observer = CreateValidObserver();
    manager.RegisterObserver(want, observer);
    manager.observerId_ = 0;
}

void FuzzGetInstance()
{
    auto& instance1 = AbilityStartWithWaitObserverManager::GetInstance();
    auto& instance2 = AbilityStartWithWaitObserverManager::GetInstance();
    (void)instance1;
    (void)instance2;
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < U32_AT_SIZE) {
        return false;
    }
    
    FuzzedDataProvider fdp(data, size);
    auto& manager = AbilityStartWithWaitObserverManager::GetInstance();
    
    manager.observerList_.clear();
    manager.observerId_ = 0;
    
    int32_t operationType = fdp.ConsumeIntegralInRange<int32_t>(0, 29);
    switch (operationType) {
        case 0:
            FuzzRegisterObserverNullObserver(manager);
            break;
        case 1:
            FuzzRegisterObserverNullProxy(manager);
            break;
        case 2: {
            Want want;
            FuzzRegisterObserverValid(manager, want);
            break;
        }
        case 3:
            FuzzRegisterObserverDuplicate(manager);
            break;
        case 4:
            FuzzRegisterObserverWithNullInList(manager);
            break;
        case 5:
            FuzzRegisterObserverMaxId(manager);
            break;
        case 6:
            FuzzUnregisterObserverNull(manager);
            break;
        case 7:
            FuzzUnregisterObserverNullProxy(manager);
            break;
        case 8:
            FuzzUnregisterObserverNotFound(manager);
            break;
        case 9:
            FuzzUnregisterObserverValid(manager);
            break;
        case 10:
            FuzzUnregisterObserverWithNullInList(manager);
            break;
        case 11:
            FuzzNotifyAATerminateWaitWantInvalidId(manager);
            break;
        case 12:
            FuzzNotifyAATerminateWaitWantNotFound(manager);
            break;
        case 13:
            FuzzNotifyAATerminateWaitWantValid(fdp, manager);
            break;
        case 14:
            FuzzNotifyAATerminateWaitWantWithNullInList(manager);
            break;
        case 15:
            FuzzNotifyAATerminateWaitWantNullDeathRecipient(manager);
            break;
        case 16:
            FuzzNotifyAATerminateWaitWantWrongId(manager);
            break;
        case 17:
            FuzzNotifyAATerminateWaitRecordNull(manager);
            break;
        case 18:
            FuzzNotifyAATerminateWaitRecordInvalidId(fdp, manager);
            break;
        case 19:
            FuzzNotifyAATerminateWaitRecordNotFound(fdp, manager);
            break;
        case 20:
            FuzzNotifyAATerminateWaitRecordValid(fdp, manager);
            break;
        case 21:
            FuzzNotifyAATerminateWaitRecordWithNullInList(fdp, manager);
            break;
        case 22:
            FuzzNotifyAATerminateWaitRecordNullDeathRecipient(fdp, manager);
            break;
        case 23:
            FuzzNotifyAATerminateWaitRecordWrongId(fdp, manager);
            break;
        case 24:
            FuzzSetColdStartNullRecord(manager);
            break;
        case 25:
            FuzzSetColdStartInvalidId(fdp, manager);
            break;
        case 26:
            FuzzSetColdStartNotFound(fdp, manager);
            break;
        case 27:
            FuzzSetColdStartValid(fdp, manager);
            break;
        case 28:
            FuzzSetColdStartWithNullInList(fdp, manager);
            break;
        case 29:
            FuzzSetColdStartWrongId(fdp, manager);
            break;
    }
    
    manager.observerList_.clear();
    return true;
}

bool DoSomethingInterestingWithMyAPI2(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < U32_AT_SIZE) {
        return false;
    }
    
    FuzzedDataProvider fdp(data, size);
    auto& manager = AbilityStartWithWaitObserverManager::GetInstance();
    
    manager.observerList_.clear();
    manager.observerId_ = 0;
    
    int32_t operationType = fdp.ConsumeIntegralInRange<int32_t>(0, 9);
    switch (operationType) {
        case 0:
            FuzzGenerateDeathRecipientNullObserver(manager);
            break;
        case 1:
            FuzzGenerateDeathRecipientNullProxy(manager);
            break;
        case 2:
            FuzzGenerateDeathRecipientValid(manager);
            break;
        case 3:
            FuzzGenerateDeathRecipientCallbackNull(manager);
            break;
        case 4:
            FuzzGenerateDeathRecipientCallbackValid(manager);
            break;
        case 5:
            FuzzMultipleObserversScenario(fdp, manager);
            break;
        case 6:
            FuzzCompleteLifecycleScenario(fdp, manager);
            break;
        case 7:
            FuzzObserverListEdgeCases(fdp, manager);
            break;
        case 8:
            FuzzMaxObserverIdEdgeCase(manager);
            break;
        case 9:
            FuzzGetInstance();
            break;
    }
    
    manager.observerList_.clear();
    return true;
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < U32_AT_SIZE) {
        return 0;
    }
    
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    OHOS::DoSomethingInterestingWithMyAPI2(data, size);
    return 0;
}