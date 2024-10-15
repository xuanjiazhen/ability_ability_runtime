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
#include "ability_manager_service.h"
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
}
class MissionListManagerFirstTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void InitMockMission(std::shared_ptr<MissionListManager>& missionListManager,
        AbilityRequest& abilityRequest, Want& want, std::shared_ptr<AbilityRecord>& ability);
    std::shared_ptr<AbilityRecord> InitAbilityRecord();
};

void MissionListManagerFirstTest::SetUpTestCase(void)
{}
void MissionListManagerFirstTest::TearDownTestCase(void)
{}
void MissionListManagerFirstTest::SetUp(void)
{}
void MissionListManagerFirstTest::TearDown(void)
{}

void MissionListManagerFirstTest::InitMockMission(std::shared_ptr<MissionListManager>& missionListManager,
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

std::shared_ptr<AbilityRecord> MissionListManagerFirstTest::InitAbilityRecord()
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
 * Function: OnTimeOut
 * SubFunction: NA
 * FunctionPoints: MissionListManager OnTimeOut
 * EnvConditions: NA
 * CaseDescription: Verify OnTimeOut
 */
HWTEST_F(MissionListManagerFirstTest, OnTimeOut_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MissionListManagerFirstTest OnTimeOut_001 start");
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    uint32_t msgId = 0;
    int64_t eventId = 0;
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->defaultStandardList_ = missionList;
    missionListManager->OnTimeOut(msgId, eventId);
    missionListManager.reset();
    TAG_LOGI(AAFwkTag::TEST, "MissionListManagerFirstTest OnTimeOut_001 end");
}

/*
 * Feature: MissionListManager
 * Function: OnTimeOut
 * SubFunction: NA
 * FunctionPoints: MissionListManager OnTimeOut
 * EnvConditions: NA
 * CaseDescription: Verify OnTimeOut
 */
HWTEST_F(MissionListManagerFirstTest, OnTimeOut_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MissionListManagerFirstTest OnTimeOut_002 start");
    int userId = 3;
    uint32_t msgId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    int64_t abilityRecordId = 0;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->missionId_ = abilityRecordId;
    abilityRecord->SetStartingWindow(true);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionListManager->currentMissionLists_.clear();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    mission->abilityRecord_ = abilityRecord;
    mission->missionId_ = abilityRecordId;
    missionList->missions_.push_back(mission);
    missionListManager->defaultSingleList_  = missionList;
    missionListManager->defaultStandardList_ = missionList;
    missionListManager->OnTimeOut(msgId, abilityRecordId);
    bool isHalf = true;
    missionListManager->OnTimeOut(msgId, abilityRecordId, isHalf);
    int i = 0;
    uint32_t msgIds[] = {
        AbilityManagerService::LOAD_TIMEOUT_MSG,
        AbilityManagerService::ACTIVE_TIMEOUT_MSG,
        AbilityManagerService::INACTIVE_TIMEOUT_MSG,
        AbilityManagerService::FOREGROUND_TIMEOUT_MSG
    };
    while (i < sizeof(msgIds) / sizeof(uint32_t)) {
        missionListManager->OnTimeOut(msgIds[i], abilityRecordId);
        i++;
    }
    missionListManager->OnTimeOut(msgId, abilityRecordId);
    #ifdef SUPPORT_SCREEN
    missionListManager->OnTimeOut(msgId, abilityRecordId);
    #endif
    EXPECT_TRUE(missionListManager != nullptr);
    missionListManager.reset();
    TAG_LOGI(AAFwkTag::TEST, "MissionListManagerFirstTest OnTimeOut_002 end");
}

/*
 * Feature: MissionListManager
 * Function: HandleForegroundTimeout
 * SubFunction: NA
 * FunctionPoints: MissionListManager HandleForegroundTimeout
 * EnvConditions: NA
 * CaseDescription: Verify HandleForegroundTimeout
 */
HWTEST_F(MissionListManagerFirstTest, HandleForegroundTimeout_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MissionListManagerFirstTest HandleForegroundTimeout_001 start");
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> ability;
    missionListManager->HandleForegroundTimeout(ability);
    ability = InitAbilityRecord();
    int64_t abilityRecordId = 0;
    ability->missionId_ = abilityRecordId;
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionListManager->currentMissionLists_.clear();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, ability);
    mission->abilityRecord_ = ability;
    mission->missionId_ = abilityRecordId;
    missionList->missions_.push_back(mission);
    missionListManager->defaultSingleList_  = missionList;
    missionListManager->HandleForegroundTimeout(ability);
    EXPECT_TRUE(missionListManager != nullptr);
    missionListManager.reset();
    TAG_LOGI(AAFwkTag::TEST, "MissionListManagerFirstTest HandleForegroundTimeout_001 end");
}

/*
 * Feature: MissionListManager
 * Function: HandleForegroundTimeout
 * SubFunction: NA
 * FunctionPoints: MissionListManager HandleForegroundTimeout
 * EnvConditions: NA
 * CaseDescription: Verify HandleForegroundTimeout
 */
HWTEST_F(MissionListManagerFirstTest, HandleForegroundTimeout_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MissionListManagerFirstTest HandleForegroundTimeout_002 start");
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> ability = InitAbilityRecord();
    ability->currentState_ = AbilityState::FOREGROUNDING;
    int64_t abilityRecordId = 0;
    ability->missionId_ = abilityRecordId;
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionListManager->currentMissionLists_.clear();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, ability);
    mission->abilityRecord_ = ability;
    mission->missionId_ = abilityRecordId;
    missionList->missions_.push_back(mission);
    missionListManager->defaultSingleList_  = missionList;
    missionListManager->HandleForegroundTimeout(ability);
    ability->isLauncherAbility_ = true;
    ability->isLauncherRoot_ = true;
    mission = std::make_shared<Mission>(1, ability);
    mission->abilityRecord_ = ability;
    missionListManager->HandleForegroundTimeout(ability);
    EXPECT_TRUE(missionListManager != nullptr);
    missionListManager.reset();
    TAG_LOGI(AAFwkTag::TEST, "MissionListManagerFirstTest HandleForegroundTimeout_002 end");
}

/*
 * Feature: MissionListManager
 * Function: MoveToTerminateList
 * SubFunction: NA
 * FunctionPoints: MissionListManager MoveToTerminateList
 * EnvConditions: NA
 * CaseDescription: Verify MoveToTerminateList
 */
HWTEST_F(MissionListManagerFirstTest, MoveToTerminateList_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MissionListManagerFirstTest MoveToTerminateList_001 start");
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> ability = InitAbilityRecord();
    missionListManager->MoveToTerminateList(nullptr);
    int64_t abilityRecordId = 0;
    ability->missionId_ = abilityRecordId;
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<MissionList> missionList1 = std::make_shared<MissionList>();
    missionListManager->currentMissionLists_.clear();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, ability);
    mission->abilityRecord_ = ability;
    mission->missionId_ = abilityRecordId;
    mission->SetMissionList(nullptr);
    missionList->missions_.push_back(mission);
    missionListManager->defaultSingleList_  = missionList;
    missionListManager->MoveToTerminateList(ability);
    missionList->missions_.clear();
    mission->SetMissionList(missionList1);
    missionList->missions_.push_back(mission);
    missionList->type_ = MissionListType::CURRENT;
    missionListManager->defaultSingleList_  = missionList;
    missionListManager->MoveToTerminateList(ability);
    EXPECT_TRUE(missionListManager != nullptr);
    missionListManager.reset();
    TAG_LOGI(AAFwkTag::TEST, "MissionListManagerFirstTest MoveToTerminateList_001 end");
}

/*
 * Feature: MissionListManager
 * Function: MoveToTerminateList
 * SubFunction: NA
 * FunctionPoints: MissionListManager MoveToTerminateList
 * EnvConditions: NA
 * CaseDescription: Verify MoveToTerminateList
 */
HWTEST_F(MissionListManagerFirstTest, MoveToTerminateList_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MissionListManagerFirstTest MoveToTerminateList_002 start");
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> ability = InitAbilityRecord();
    int64_t abilityRecordId = 0;
    ability->missionId_ = abilityRecordId;
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<MissionList> missionList1 = std::make_shared<MissionList>();
    missionListManager->currentMissionLists_.clear();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, ability);
    mission->abilityRecord_ = ability;
    mission->missionId_ = abilityRecordId;
    mission->SetMissionList(missionList1);
    missionList->missions_.push_back(mission);
    missionListManager->defaultSingleList_  = missionList;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionIdMap_[abilityRecordId] = true;
    InnerMissionInfo info;
    info.missionInfo.id = 0;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->taskDataPersistenceMgr_ =
        DelayedSingleton<TaskDataPersistenceMgr>::GetInstance();
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionInfoList_.push_back(info);
    missionListManager->MoveToTerminateList(ability);
    missionListManager->listenerController_ = std::make_shared<MissionListenerController>();
    EXPECT_TRUE(missionListManager != nullptr);
    missionListManager->MoveToTerminateList(ability);
    missionListManager.reset();
    TAG_LOGI(AAFwkTag::TEST, "MissionListManagerFirstTest MoveToTerminateList_002 end");
}

/*
 * Feature: MissionListManager
 * Function: HandleLauncherDied
 * SubFunction: NA
 * FunctionPoints: MissionListManager HandleLauncherDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleLauncherDied
 */
HWTEST_F(MissionListManagerFirstTest, HandleLauncherDied_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MissionListManagerFirstTest HandleLauncherDied_001 start");
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> ability = InitAbilityRecord();
    int64_t abilityRecordId = 0;
    ability->missionId_ = abilityRecordId;
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<MissionList> missionList1 = std::make_shared<MissionList>();
    missionListManager->currentMissionLists_.clear();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, ability);
    mission->abilityRecord_ = ability;
    mission->missionId_ = abilityRecordId;
    mission->SetMissionList(missionList1);
    missionList->missions_.push_back(mission);
    missionListManager->defaultSingleList_  = missionList;
    missionListManager->HandleLauncherDied(ability);
    missionListManager->launcherList_ = missionList1;
    missionListManager->HandleLauncherDied(ability);
    EXPECT_TRUE(missionListManager != nullptr);
    missionListManager.reset();
    TAG_LOGI(AAFwkTag::TEST, "MissionListManagerFirstTest HandleLauncherDied_001 end");
}

/*
 * Feature: MissionListManager
 * Function: HandleLauncherDied
 * SubFunction: NA
 * FunctionPoints: MissionListManager HandleLauncherDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleLauncherDied
 */
HWTEST_F(MissionListManagerFirstTest, HandleLauncherDied_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MissionListManagerFirstTest HandleLauncherDied_002 start");
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> ability = InitAbilityRecord();
    int64_t abilityRecordId = 0;
    ability->missionId_ = abilityRecordId;
    ability->isLauncherRoot_ = true;
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<MissionList> missionList1 = std::make_shared<MissionList>();
    missionListManager->currentMissionLists_.clear();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, ability);
    mission->abilityRecord_ = ability;
    mission->missionId_ = abilityRecordId;
    mission->SetMissionList(missionList1);
    missionList->missions_.push_back(mission);
    missionListManager->defaultSingleList_  = missionList;
    missionListManager->launcherList_ = missionList1;
    missionListManager->HandleLauncherDied(ability);
    EXPECT_TRUE(missionListManager != nullptr);
    missionListManager.reset();
    TAG_LOGI(AAFwkTag::TEST, "MissionListManagerFirstTest HandleLauncherDied_002 end");
}

/*
 * Feature: MissionListManager
 * Function: HandleLauncherDied
 * SubFunction: NA
 * FunctionPoints: MissionListManager HandleLauncherDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleLauncherDied
 */
HWTEST_F(MissionListManagerFirstTest, HandleLauncherDied_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MissionListManagerFirstTest HandleLauncherDied_003 start");
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> ability = InitAbilityRecord();
    int64_t abilityRecordId = 0;
    ability->missionId_ = abilityRecordId;
    ability->currentState_ = FOREGROUND;
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<MissionList> missionList1 = std::make_shared<MissionList>();
    missionListManager->currentMissionLists_.clear();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, ability);
    mission->abilityRecord_ = ability;
    mission->missionId_ = abilityRecordId;
    mission->SetMissionList(missionList1);
    missionList->missions_.push_back(mission);
    missionListManager->defaultSingleList_  = missionList;
    missionListManager->launcherList_ = missionList1;
    missionListManager->HandleLauncherDied(ability);
    EXPECT_TRUE(missionListManager != nullptr);
    missionListManager.reset();
    TAG_LOGI(AAFwkTag::TEST, "MissionListManagerFirstTest HandleLauncherDied_003 end");
}
}  // namespace AAFwk
}  // namespace OHOS
