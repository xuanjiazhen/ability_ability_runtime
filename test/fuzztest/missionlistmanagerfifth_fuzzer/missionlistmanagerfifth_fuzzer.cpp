/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "missionlistmanagerfifth_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "mission_info_mgr.h"
#include "mission_list_manager.h"
#undef private

#include "securec.h"
#include "parcel.h"
#include "ability_record.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t FOO_MAX_LEN = 1024;
constexpr uint8_t ENABLE = 2;
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_TWO = 2;
constexpr int INPUT_THREE = 3;
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;

class MyAbilityConnection : public IAbilityConnection {
public:
    MyAbilityConnection() = default;
    virtual ~MyAbilityConnection() = default;
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode) override
    {}
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) override
    {}
    sptr<IRemoteObject> AsObject() override
    {
        return {};
    }
};
}

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[INPUT_TWO] << OFFSET_TWO) |
        ptr[INPUT_THREE];
}

std::shared_ptr<AbilityRecord> GetFuzzAbilityRecord()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord;
}

sptr<Token> GetFuzzAbilityToken()
{
    sptr<Token> token = nullptr;
    std::shared_ptr<AbilityRecord> abilityRecord = GetFuzzAbilityRecord();
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}

bool DoSomethingInterestingWithMyAPI01(const char* data, size_t size)
{
    int intParam = static_cast<int>(GetU32Data(data));
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    uint32_t uint32Param = static_cast<uint32_t>(GetU32Data(data));
    Parcel wantParcel;
    Want* want = nullptr;
    if (wantParcel.WriteBuffer(data, size)) {
        want = Want::Unmarshalling(wantParcel);
        if (!want) {
            return false;
        }
    }
    std::shared_ptr<AbilityRecord> abilityRecord = GetFuzzAbilityRecord();
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    sptr<IAbilityConnection> connect = new MyAbilityConnection();
    std::vector<std::string> info;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(int32Param, abilityRecord);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    auto missionListManager = std::make_shared<MissionListManager>(intParam);
    auto launcherList = std::make_shared<MissionList>(MissionListType::LAUNCHER);
    missionListManager->launcherList_ = launcherList;
    missionListManager->defaultStandardList_ = std::make_shared<MissionList>(MissionListType::DEFAULT_STANDARD);
    missionListManager->defaultSingleList_ = std::make_shared<MissionList>(MissionListType::DEFAULT_SINGLE);
    missionListManager->currentMissionLists_.push_front(launcherList);
    if (!missionListManager->listenerController_) {
        missionListManager->listenerController_ = std::make_shared<MissionListenerController>();
    }
    DelayedSingleton<MissionInfoMgr>::GetInstance()->taskDataPersistenceMgr_ =
    DelayedSingleton<TaskDataPersistenceMgr>::GetInstance();
    AbilityRequest abilityRequest;
    missionListManager->IsAppLastAbility(abilityRecord);
    std::shared_ptr<AbilityRecord> abilityRecords = nullptr;
    missionListManager->IsAppLastAbility(abilityRecords);
    missionListManager->SetLastExitReason(abilityRecord);
    missionListManager->SetLastExitReason(abilityRecords);
    missionListManager->GetActiveAbilityList(int32Param, info, int32Param);
    missionListManager->DoAbilityForeground(abilityRecord, uint32Param);
    missionListManager->DoAbilityForeground(abilityRecords, uint32Param);
    missionListManager->NotifyStartAbilityResult(abilityRequest, intParam);
    if (want) {
        delete want;
        want = nullptr;
    }
    return true;
}

bool DoSomethingInterestingWithMyAPI02(const char* data, size_t size)
{
    int intParam = static_cast<int>(GetU32Data(data));
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    std::string stringParam(data, size);
    Parcel wantParcel;
    Want* want = nullptr;
    if (wantParcel.WriteBuffer(data, size)) {
        want = Want::Unmarshalling(wantParcel);
        if (!want) {
            return false;
        }
    }
    std::shared_ptr<AbilityRecord> abilityRecord = GetFuzzAbilityRecord();
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    sptr<IAbilityConnection> connect = new MyAbilityConnection();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(int32Param, abilityRecord);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    auto missionListManager = std::make_shared<MissionListManager>(intParam);
    auto launcherList = std::make_shared<MissionList>(MissionListType::LAUNCHER);
    missionListManager->launcherList_ = launcherList;
    missionListManager->defaultStandardList_ = std::make_shared<MissionList>(MissionListType::DEFAULT_STANDARD);
    missionListManager->defaultSingleList_ = std::make_shared<MissionList>(MissionListType::DEFAULT_SINGLE);
    missionListManager->currentMissionLists_.push_front(launcherList);
    if (!missionListManager->listenerController_) {
        missionListManager->listenerController_ = std::make_shared<MissionListenerController>();
    }
    DelayedSingleton<MissionInfoMgr>::GetInstance()->taskDataPersistenceMgr_ =
    DelayedSingleton<TaskDataPersistenceMgr>::GetInstance();
    AbilityRequest abilityRequest;
    missionListManager->SignRestartAppFlag(stringParam);
    missionListManager->SendKeyEvent(abilityRequest);
    missionListManager->CallRequestDone(abilityRecord, token);
    std::shared_ptr<AbilityRecord> abilityRecords = nullptr;
    missionListManager->CallRequestDone(abilityRecords, token);
    sptr<IRemoteObject> callStub = nullptr;
    missionListManager->CallRequestDone(abilityRecord, callStub);
    missionListManager->CheckPrepareTerminateEnable(mission);
    missionListManager->isPrepareTerminateEnable_ = false;
    missionListManager->CheckPrepareTerminateEnable(mission);
    missionListManager->isPrepareTerminateEnable_ = true;
    missionListManager->CheckPrepareTerminateEnable(mission);
    std::shared_ptr<Mission> missions = nullptr;
    missionListManager->CheckPrepareTerminateEnable(missions);
    if (want) {
        delete want;
        want = nullptr;
    }
    return true;
}
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    bool res1 = DoSomethingInterestingWithMyAPI01(data, size);
    bool res2 = DoSomethingInterestingWithMyAPI02(data, size);
    return res1 && res2;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::U32_AT_SIZE || size > OHOS::FOO_MAX_LEN) {
        return 0;
    }

    char* ch = (char*)malloc(size + 1);
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