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

#include "ability_manager_service.h"
#include "ability_record.h"

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
constexpr uint8_t STATE = 17;
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

bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    bool boolParam = *data % ENABLE;
    int intParam = static_cast<int>(GetU32Data(data));
    std::shared_ptr<AbilityRecord> abilityRecord = GetFuzzAbilityRecord();
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    auto missionListManager = std::make_shared<MissionListManager>(intParam);
    auto launcherList = std::make_shared<MissionList>(MissionListType::LAUNCHER);
    missionListManager->launcherList_ = launcherList;
    missionListManager->defaultStandardList_ =
        std::make_shared<MissionList>(MissionListType::DEFAULT_STANDARD);
    missionListManager->defaultSingleList_ =
        std::make_shared<MissionList>(MissionListType::DEFAULT_SINGLE);
    missionListManager->currentMissionLists_.push_front(launcherList);
    if (!missionListManager->listenerController_) {
        missionListManager->listenerController_ =
            std::make_shared<MissionListenerController>();
    }
    DelayedSingleton<MissionInfoMgr>::GetInstance()->taskDataPersistenceMgr_ =
        DelayedSingleton<TaskDataPersistenceMgr>::GetInstance();
    missionListManager->ProcessPreload(abilityRecord);
    missionListManager->CompleteFirstFrameDrawing(token);
    AbilityRequest abilityRequest;
    missionListManager->GetTargetMissionList(abilityRecord, abilityRequest);
    missionListManager->MoveToTerminateList(abilityRecord);
    missionListManager->CompleteForegroundFailed(nullptr, AAFwk::AbilityState::FOREGROUND_INVALID_MODE);
    OHOS::AAFwk::AbilityState state = static_cast<OHOS::AAFwk::AbilityState>(*data % STATE);
    missionListManager->CompleteForegroundFailed(abilityRecord, state);
    missionListManager->UpdateMissionSnapshot(abilityRecord);

    uint32_t msgId[] = {
        static_cast<uint32_t>(AbilityManagerService::LOAD_TIMEOUT_MSG),
        static_cast<uint32_t>(AbilityManagerService::ACTIVE_TIMEOUT_MSG),
        static_cast<uint32_t>(AbilityManagerService::INACTIVE_TIMEOUT_MSG),
        static_cast<uint32_t>(AbilityManagerService::FOREGROUND_TIMEOUT_MSG),
        static_cast<uint32_t>(AbilityManagerService::BACKGROUND_TIMEOUT_MSG),
        static_cast<uint32_t>(AbilityManagerService::TERMINATE_TIMEOUT_MSG)
    };
    std::string msgContent;
    int i = 0;
    while (i < sizeof(msgId) / sizeof(uint32_t)) {
        missionListManager->GetContentAndTypeId(msgId[i++], msgContent, intParam);
    }
    uint32_t uint32Param = static_cast<uint32_t>(GetU32Data(data));
    missionListManager->GetContentAndTypeId(uint32Param, msgContent, intParam);
    missionListManager->PrintTimeOutLog(abilityRecord, uint32Param, boolParam);
    missionListManager->PostMissionLabelUpdateTask(intParam);
    missionListManager->NotifyMissionCreated(abilityRecord);

    return true;
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