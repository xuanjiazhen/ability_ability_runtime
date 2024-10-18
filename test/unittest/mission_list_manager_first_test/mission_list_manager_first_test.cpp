/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#define private public
#define protected public
#include "ability_config.h"
#include "ability_info.h"
#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "mission.h"
#include "mission_info_mgr.h"
#include "mission_list_manager.h"
#include "startup_util.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
#ifdef WITH_DLP
const std::string DLP_INDEX = "ohos.dlp.params.index";
#endif // WITH_DLP
constexpr uint32_t INPUT_ZERO = 0;
constexpr uint32_t INPUT_ONE = 1;
constexpr uint32_t INPUT_TWO = 2;
constexpr uint32_t INPUT_FOUR = 4;
constexpr uint32_t INPUT_FIFTH = 5;
constexpr uint32_t INPUT_SIX = 6;
constexpr uint32_t INPUT_EIGHT = 8;
}
class MissionListManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void InitMockMission(std::shared_ptr<MissionListManager>& missionListManager,
        AbilityRequest& abilityRequest, Want& want, std::shared_ptr<AbilityRecord>& ability);
    std::shared_ptr<AbilityRecord> InitAbilityRecord();
};

void MissionListManagerTest::SetUpTestCase(void)
{}
void MissionListManagerTest::TearDownTestCase(void)
{}
void MissionListManagerTest::SetUp(void)
{}
void MissionListManagerTest::TearDown(void)
{}

void MissionListManagerTest::InitMockMission(std::shared_ptr<MissionListManager>& missionListManager,
    AbilityRequest& abilityRequest, Want& want, std::shared_ptr<AbilityRecord>& ability)
{
    missionListManager->Init();

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    AppExecFwk::ApplicationInfo applicationInfo;
    ability = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(11, ability, "missionName");
    mission->abilityRecord_ = ability;
    ability->SetSpecifiedFlag("flag");
    ability->SetIsNewWant(false);

    abilityRequest.callerToken = ability->GetToken();
    missionListManager->EnqueueWaitingAbility(abilityRequest);
    missionListManager->defaultStandardList_->AddMissionToTop(mission);
}

std::shared_ptr<AbilityRecord> MissionListManagerTest::InitAbilityRecord()
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    return abilityRecord;
}

bool g_notifyWindowTransitionCalled = false;
bool g_cancelStartingWindowCalled = false;

class MockWMSHandler : public IWindowManagerServiceHandler {
public:
    virtual void NotifyWindowTransition(sptr<AbilityTransitionInfo> fromInfo, sptr<AbilityTransitionInfo> toInfo,
        bool& animaEnabled)
    {
        g_notifyWindowTransitionCalled = true;
    }

    virtual int32_t GetFocusWindow(sptr<IRemoteObject>& abilityToken)
    {
        return 0;
    }

    virtual void StartingWindow(sptr<AbilityTransitionInfo> info,
        std::shared_ptr<Media::PixelMap> pixelMap, uint32_t bgColor) {}

    virtual void StartingWindow(sptr<AbilityTransitionInfo> info, std::shared_ptr<Media::PixelMap> pixelMap) {}

    virtual void CancelStartingWindow(sptr<IRemoteObject> abilityToken)
    {
        g_cancelStartingWindowCalled = true;
    }

    virtual int32_t MoveMissionsToForeground(const std::vector<int32_t>& missionIds, int32_t topMissionId)
    {
        return 0;
    }

    virtual int32_t MoveMissionsToBackground(const std::vector<int32_t>& missionIds, std::vector<int32_t>& result)
    {
        return 0;
    }

    virtual sptr<IRemoteObject> AsObject()
    {
        return nullptr;
    }

    virtual void NotifyAnimationAbilityDied(sptr<AbilityTransitionInfo> info) {}
};

/*
 * Feature: MissionListManager
 * Function: ClearAllMissions
 * SubFunction: NA
 * FunctionPoints: MissionListManager ClearAllMissions
 * EnvConditions: NA
 * CaseDescription: Verify ClearAllMissions
 */
HWTEST_F(MissionListManagerTest, ClearAllMissions_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearAllMissions_001 start");
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    sptr<IMissionListener> listener;
    missionListManager->listenerController_ = nullptr;
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto ability = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(11, ability, "missionName");
    mission->abilityRecord_ = ability;
    ability->SetSpecifiedFlag("flag");
    missionListManager->defaultSingleList_->AddMissionToTop(mission);
    missionListManager->defaultStandardList_->AddMissionToTop(mission);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = FOREGROUND;
    std::shared_ptr<Mission> missions = std::make_shared<Mission>(1, abilityRecord, "missionName");
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_front(missions);
    missionListManager->currentMissionLists_.push_front(missionList);
    auto result = missionListManager->ClearAllMissions();
    missionList = std::make_shared<MissionList>(MissionListType::LAUNCHER);
    missionList->missions_.push_front(missions);
    missionListManager->currentMissionLists_.push_front(missionList);
    result = missionListManager->ClearAllMissions();
    EXPECT_EQ(result, ERR_OK);
    missionListManager.reset();
    TAG_LOGI(AAFwkTag::TEST, "ClearAllMissions_001 end");
}

/*
 * Feature: MissionListManager
 * Function: ClearAllMissionsLocked
 * SubFunction: NA
 * FunctionPoints: MissionListManager ClearAllMissionsLocked
 * EnvConditions: NA
 * CaseDescription: Verify ClearAllMissionsLocked
 */
HWTEST_F(MissionListManagerTest, ClearAllMissionsLocked_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearAllMissionsLocked_001 start");
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    EXPECT_NE(missionListManager, nullptr);
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = FOREGROUND;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, "missionName");
    mission->SetLockedState(true);
    std::list<std::shared_ptr<Mission>> foregroundAbilities;
    std::list<std::shared_ptr<Mission>> missionList;
    missionList.push_front(mission);
    bool searchActive = true;
    missionListManager->ClearAllMissionsLocked(missionList, foregroundAbilities, searchActive);
    std::shared_ptr<Mission> missions = nullptr;
    std::list<std::shared_ptr<Mission>> missionLists;
    missionLists.push_front(missions);
    missionListManager->ClearAllMissionsLocked(missionLists, foregroundAbilities, searchActive);
    TAG_LOGI(AAFwkTag::TEST, "ClearAllMissionsLocked_001 end");
}

/*
 * Feature: MissionListManager
 * Function: ClearAllMissionsLocked
 * SubFunction: NA
 * FunctionPoints: MissionListManager ClearAllMissionsLocked
 * EnvConditions: NA
 * CaseDescription: Verify ClearAllMissionsLocked
 */
HWTEST_F(MissionListManagerTest, ClearAllMissionsLocked_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearAllMissionsLocked_002 start");
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    EXPECT_NE(missionListManager, nullptr);
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetUid(userId);
    std::shared_ptr<Mission> missions = std::make_shared<Mission>(1, abilityRecord, "missionName");
    missions->SetLockedState(false);
    missions->SetUnclearable(true);
    std::list<std::shared_ptr<Mission>> foregroundAbilities;
    std::list<std::shared_ptr<Mission>> missionLists;
    missionLists.push_front(missions);
    bool searchActive = true;
    missionListManager->ClearAllMissionsLocked(missionLists, foregroundAbilities, searchActive);
    std::shared_ptr<Mission> mission1 = std::make_shared<Mission>(1, abilityRecord, "missionName");
    mission1->SetLockedState(false);
    mission1->SetUnclearable(false);
    std::list<std::shared_ptr<Mission>> missionList1;
    missionList1.push_front(mission1);
    missionListManager->ClearAllMissionsLocked(missionList1, foregroundAbilities, searchActive);
    TAG_LOGI(AAFwkTag::TEST, "ClearAllMissionsLocked_002 end");
}

/*
 * Feature: MissionListManager
 * Function: ClearAllMissionsLocked
 * SubFunction: NA
 * FunctionPoints: MissionListManager ClearAllMissionsLocked
 * EnvConditions: NA
 * CaseDescription: Verify ClearAllMissionsLocked
 */
HWTEST_F(MissionListManagerTest, ClearAllMissionsLocked_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearAllMissionsLocked_003 start");
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    EXPECT_NE(missionListManager, nullptr);
    missionListManager->Init();
    missionListManager->isPrepareTerminateEnable_ = true;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = FOREGROUND;
    abilityRecord->SetUid(userId);
    std::shared_ptr<Mission> missions = std::make_shared<Mission>(1, abilityRecord, "missionName");
    missions->SetLockedState(false);
    missions->SetUnclearable(false);
    std::list<std::shared_ptr<Mission>> foregroundAbilities;
    std::list<std::shared_ptr<Mission>> missionLists;
    missionLists.push_front(missions);
    bool searchActive = true;
    missionListManager->ClearAllMissionsLocked(missionLists, foregroundAbilities, searchActive);
    searchActive = false;
    int32_t int32Param = 3;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(int32Param, abilityRecord);
    mission->SetLockedState(false);
    mission->SetUnclearable(false);
    std::list<std::shared_ptr<Mission>> missionList1;
    missionList1.push_front(mission);
    missionListManager->ClearAllMissionsLocked(missionList1, foregroundAbilities, searchActive);
    TAG_LOGI(AAFwkTag::TEST, "ClearAllMissionsLocked_003 end");
}

/*
 * Feature: MissionListManager
 * Function: ClearAllMissionsLocked
 * SubFunction: NA
 * FunctionPoints: MissionListManager ClearAllMissionsLocked
 * EnvConditions: NA
 * CaseDescription: Verify ClearAllMissionsLocked
 */
HWTEST_F(MissionListManagerTest, ClearAllMissionsLocked_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearAllMissionsLocked_004 start");
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    EXPECT_NE(missionListManager, nullptr);
    missionListManager->Init();
    missionListManager->isPrepareTerminateEnable_ = true;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = FOREGROUND;
    abilityRecord->SetUid(userId);
    abilityRecord->abilityInfo_.isStageBasedModel = true;
    std::shared_ptr<Mission> missions = std::make_shared<Mission>(1, abilityRecord, "missionName");
    missions->SetLockedState(false);
    missions->SetUnclearable(false);
    std::list<std::shared_ptr<Mission>> foregroundAbilities;
    std::list<std::shared_ptr<Mission>> missionLists;
    missionLists.push_front(missions);
    bool searchActive = true;
    missionListManager->ClearAllMissionsLocked(missionLists, foregroundAbilities, searchActive);
    searchActive = false;
    int32_t int32Param = 3;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(int32Param, abilityRecord);
    mission->SetLockedState(false);
    mission->SetUnclearable(false);
    std::list<std::shared_ptr<Mission>> missionList1;
    missionList1.push_front(mission);
    missionListManager->ClearAllMissionsLocked(missionList1, foregroundAbilities, searchActive);
    TAG_LOGI(AAFwkTag::TEST, "ClearAllMissionsLocked_004 end");
}

/*
 * Feature: MissionListManager
 * Function: UpdateSnapShot
 * SubFunction: NA
 * FunctionPoints: MissionListManager UpdateSnapShot
 * EnvConditions: NA
 * CaseDescription: Verify UpdateSnapShot
 */
HWTEST_F(MissionListManagerTest, UpdateSnapShot_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateSnapShot_001 start");
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    EXPECT_NE(missionListManager, nullptr);
    missionListManager->Init();
    sptr<IRemoteObject> token = nullptr;
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    missionListManager->UpdateSnapShot(token, pixelMap);
    TAG_LOGI(AAFwkTag::TEST, "UpdateSnapShot_001 end");
}

/*
 * Feature: MissionListManager
 * Function: UpdateSnapShot
 * SubFunction: NA
 * FunctionPoints: MissionListManager UpdateSnapShot
 * EnvConditions: NA
 * CaseDescription: Verify UpdateSnapShot
 */
HWTEST_F(MissionListManagerTest, UpdateSnapShot_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateSnapShot_002 start");
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    EXPECT_NE(missionListManager, nullptr);
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.excludeFromMissions = true;
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, "missionName");
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_back(mission);
    missionListManager->terminateAbilityList_.clear();
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultSingleList_ = missionList;
    auto res = missionListManager->GetAbilityRecordByToken(token);
    EXPECT_NE(res, nullptr);
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    missionListManager->UpdateSnapShot(token, pixelMap);
    TAG_LOGI(AAFwkTag::TEST, "UpdateSnapShot_002 end");
}

/*
 * Feature: MissionListManager
 * Function: UpdateSnapShot
 * SubFunction: NA
 * FunctionPoints: MissionListManager UpdateSnapShot
 * EnvConditions: NA
 * CaseDescription: Verify UpdateSnapShot
 */
HWTEST_F(MissionListManagerTest, UpdateSnapShot_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateSnapShot_003 start");
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    EXPECT_NE(missionListManager, nullptr);
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.excludeFromMissions = false;
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, "missionName");
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_back(mission);
    missionListManager->terminateAbilityList_.clear();
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultSingleList_ = missionList;
    auto res = missionListManager->GetAbilityRecordByToken(token);
    EXPECT_NE(res, nullptr);
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    missionListManager->listenerController_ = std::make_shared<MissionListenerController>();
    EXPECT_NE(missionListManager->listenerController_, nullptr);
    missionListManager->UpdateSnapShot(token, pixelMap);
    TAG_LOGI(AAFwkTag::TEST, "UpdateSnapShot_003 end");
}

/*
 * Feature: MissionListManager
 * Function: GetContentAndTypeId
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetContentAndTypeId
 * EnvConditions: NA
 * CaseDescription: Verify GetContentAndTypeId
 */
HWTEST_F(MissionListManagerTest, GetContentAndTypeId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetContentAndTypeId_001 start");
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    EXPECT_NE(missionListManager, nullptr);
    missionListManager->Init();
    uint32_t msgId = INPUT_ZERO;
    std::string msgContent = "msgContent";
    int typeId = 0;
    missionListManager->GetContentAndTypeId(msgId, msgContent, typeId);
    msgId = INPUT_ONE;
    missionListManager->GetContentAndTypeId(msgId, msgContent, typeId);
    msgId = INPUT_TWO;
    missionListManager->GetContentAndTypeId(msgId, msgContent, typeId);
    msgId = INPUT_FIFTH;
    missionListManager->GetContentAndTypeId(msgId, msgContent, typeId);
    msgId = INPUT_SIX;
    missionListManager->GetContentAndTypeId(msgId, msgContent, typeId);
    msgId = INPUT_FOUR;
    missionListManager->GetContentAndTypeId(msgId, msgContent, typeId);
    msgId = INPUT_EIGHT;
    missionListManager->GetContentAndTypeId(msgId, msgContent, typeId);
    TAG_LOGI(AAFwkTag::TEST, "GetContentAndTypeId_001 end");
}
}  // namespace AAFwk
}  // namespace OHOS
