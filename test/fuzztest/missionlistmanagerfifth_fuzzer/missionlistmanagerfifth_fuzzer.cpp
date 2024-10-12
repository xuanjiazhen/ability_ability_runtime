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
#define protected public
#include "mission_info_mgr.h"
#include "mission_list_manager.h"
#include "ability_record.h"
#undef private
#undef protected

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_TWO = 2;
constexpr int INPUT_THREE = 3;
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
constexpr size_t FOO_MAX_LEN = 1024;
constexpr uint8_t ENABLE = 2;
class MyAbilityConnection : public IAbilityConnection {
public:
    MyAbilityConnection() = default;
    virtual ~MyAbilityConnection() = default;
    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
                              const sptr<IRemoteObject> &remoteObject,
                              int resultCode) override {}
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
                                 int resultCode) override {}
    sptr<IRemoteObject> AsObject() override { return {}; }
};
} // namespace

uint32_t GetU32Data(const char *ptr)
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
    std::shared_ptr<AbilityRecord> abilityRecord =
        AbilityRecord::CreateAbilityRecord(abilityRequest);
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

void DoSomethingInterestingWithMyAPIOne(const char *data, size_t size)
{
    int intParam = static_cast<int>(GetU32Data(data));
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    auto missionListManager = std::make_shared<MissionListManager>(intParam);
    auto launcherList = std::make_shared<MissionList>(MissionListType::LAUNCHER);
    missionListManager->launcherList_ = launcherList;
    missionListManager->defaultStandardList_ =
        std::make_shared<MissionList>(MissionListType::DEFAULT_STANDARD);
    missionListManager->defaultSingleList_ =
        std::make_shared<MissionList>(MissionListType::DEFAULT_SINGLE);
    missionListManager->currentMissionLists_.push_front(launcherList);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionListManager->listenerController_ = nullptr;
    missionListManager->NotifyMissionUnfocused(int32Param);
    missionListManager->NotifyMissionFocused(int32Param);
    if (!missionListManager->listenerController_) {
        missionListManager->listenerController_ =
            std::make_shared<MissionListenerController>();
    }
    DelayedSingleton<MissionInfoMgr>::GetInstance()->taskDataPersistenceMgr_ =
        DelayedSingleton<TaskDataPersistenceMgr>::GetInstance();

    AbilityRequest abilityRequest;
    missionListManager->NotifyStartAbilityResult(abilityRequest, intParam);
    missionListManager->NotifyMissionUnfocused(int32Param);
    missionListManager->NotifyMissionFocused(int32Param);
}

void DoSomethingInterestingWithMyAPITwo(const char *data, size_t size)
{
    bool boolParam = *data % ENABLE;
    int intParam = static_cast<int>(GetU32Data(data));
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    auto missionListManager = std::make_shared<MissionListManager>(intParam);
    auto launcherList = std::make_shared<MissionList>(MissionListType::LAUNCHER);
    missionListManager->launcherList_ = launcherList;
    missionListManager->defaultStandardList_ =
        std::make_shared<MissionList>(MissionListType::DEFAULT_STANDARD);
    missionListManager->defaultSingleList_ =
        std::make_shared<MissionList>(MissionListType::DEFAULT_SINGLE);
    missionListManager->currentMissionLists_.push_front(launcherList);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    if (!missionListManager->listenerController_) {
        missionListManager->listenerController_ = std::make_shared<MissionListenerController>();
    }
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord;
    missionListManager->UpdateAbilityRecordLaunchReason(abilityRequest, abilityRecord);
    abilityRecord = GetFuzzAbilityRecord();
    missionListManager->UpdateAbilityRecordLaunchReason(abilityRequest, abilityRecord);
    abilityRequest.want.SetParam(Want::PARAM_ABILITY_RECOVERY_RESTART, true);
    missionListManager->UpdateAbilityRecordLaunchReason(abilityRequest, abilityRecord);
    abilityRequest.want.SetParam(Want::PARAM_ABILITY_RECOVERY_RESTART, false);
    abilityRecord->want_.SetParam(Want::PARAM_ABILITY_RECOVERY_RESTART, true);
    missionListManager->UpdateAbilityRecordLaunchReason(abilityRequest, abilityRecord);
    abilityRecord->want_.SetParam(Want::PARAM_ABILITY_RECOVERY_RESTART, false);
    missionListManager->UpdateAbilityRecordLaunchReason(abilityRequest, abilityRecord);
}

bool DoSomethingInterestingWithMyAPIThree(const char *data, size_t size)
{
    bool boolParam = *data % ENABLE;
    int intParam = static_cast<int>(GetU32Data(data));
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    Parcel wantParcel;
    Want *want = nullptr;
    if (wantParcel.WriteBuffer(data, size)) {
        want = Want::Unmarshalling(wantParcel);
        if (!want) {
            return false;
        }
    }
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    auto missionListManager = std::make_shared<MissionListManager>(intParam);
    auto launcherList = std::make_shared<MissionList>(MissionListType::LAUNCHER);
    missionListManager->launcherList_ = launcherList;
    missionListManager->defaultStandardList_ =
        std::make_shared<MissionList>(MissionListType::DEFAULT_STANDARD);
    missionListManager->defaultSingleList_ =
        std::make_shared<MissionList>(MissionListType::DEFAULT_SINGLE);
    missionListManager->currentMissionLists_.push_front(launcherList);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    if (!missionListManager->listenerController_) {
        missionListManager->listenerController_ = std::make_shared<MissionListenerController>();
    }
    std::vector<MissionValidResult> results;
    std::vector<int32_t> missionIds;
    for (int i = INPUT_ZERO; i < INPUT_THREE; ++i) {
        missionIds.push_back(int32Param);
    }
    missionListManager->IsValidMissionIds(missionIds, results);
    AbilityRequest request;
    missionListManager->NotifyStartSpecifiedAbility(request, *want);
    request.abilityInfoCallback = token;
    missionListManager->NotifyStartSpecifiedAbility(request, *want);
    AppExecFwk::ElementName element;
    missionListManager->GetAbilityRecordByName(element);
    missionListManager->GetAbilityRecordByNameFromCurrentMissionLists(element);
    AbilityRequest abilityRequest;
    missionListManager->CallAbilityLocked(abilityRequest);
    AppInfo info;
    missionListManager->UpdateAbilityRecordColdStartFlag(info, boolParam);
    if (want) {
        delete want;
        want = nullptr;
    }
    return true;
}

bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    DoSomethingInterestingWithMyAPIOne(data, size);
    DoSomethingInterestingWithMyAPITwo(data, size);
    auto res = DoSomethingInterestingWithMyAPIThree(data, size);

    return res;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }
    /* Validate the length of size */
    if (size < OHOS::U32_AT_SIZE || size > OHOS::FOO_MAX_LEN) {
        return 0;
    }
    char *ch = (char *)malloc(size + 1);
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