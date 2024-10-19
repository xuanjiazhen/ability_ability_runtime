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

#include <gtest/gtest.h>

#define private public
#include "ability_record.h"
#include "mission_list.h"
#include "mission_list_manager.h"
#undef private

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class MissionListManagerFirstTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
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

std::shared_ptr<AbilityRecord> MissionListManagerFirstTest::InitAbilityRecord()
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    return abilityRecord;
}

/*
 * @tc.name: MissionListManager_SetMissionIcon_001
 * @tc.desc: Test SetMissionIcon
 * @tc.type: FUNC
 */
HWTEST_F(MissionListManagerFirstTest, MissionListManager_SetMissionIcon_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_SetMissionIcon_001 start";
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    std::shared_ptr<Media::PixelMap> icon;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    EXPECT_NE(missionList, nullptr);
    missionList->missions_.push_back(nullptr);
    missionList->missions_.push_back(mission);
    missionListManager->defaultSingleList_ = missionList;
    abilityRecord->SetMissionId(mission->GetMissionId());
    abilityRecord->abilityInfo_.excludeFromMissions = false;
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    int res = missionListManager->SetMissionIcon(token, icon);
    EXPECT_EQ(res, 0);
    abilityRecord->abilityInfo_.excludeFromMissions = true;
    token = abilityRecord->GetToken();
    res = missionListManager->SetMissionIcon(token, icon);
    EXPECT_EQ(res, 0);
    missionListManager.reset();
    GTEST_LOG_(INFO) << "MissionListManager_SetMissionIcon_001 end";
}

/*
 * @tc.name: MissionListManager_UpdateAbilityRecordColdStartFlag_001
 * @tc.desc: Test UpdateAbilityRecordColdStartFlag
 * @tc.type: FUNC
 */
HWTEST_F(MissionListManagerFirstTest, MissionListManager_UpdateAbilityRecordColdStartFlag_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_UpdateAbilityRecordColdStartFlag_001 start";
    int userId = 2;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord1 = InitAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord2 = InitAbilityRecord();
    std::string processName = "processName";
    abilityRecord1->abilityInfo_.process = processName;
    abilityRecord2->abilityInfo_.applicationInfo.bundleName = processName;
    std::shared_ptr<Mission> mission1 = std::make_shared<Mission>(1, abilityRecord1, "missionName");
    EXPECT_NE(mission1, nullptr);
    std::shared_ptr<Mission> mission2 = std::make_shared<Mission>(2, abilityRecord2, "missionName");
    EXPECT_NE(mission2, nullptr);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    EXPECT_NE(missionList, nullptr);
    std::shared_ptr<MissionList> missionList2 = std::make_shared<MissionList>();
    EXPECT_NE(missionList2, nullptr);
    missionList->missions_.push_back(nullptr);
    missionList->missions_.push_back(mission1);
    missionList->missions_.push_back(mission2);
    AppInfo info;
    info.processName = processName;
    missionListManager->currentMissionLists_.push_back(missionList);
    missionListManager->currentMissionLists_.push_back(missionList2);
    missionListManager->defaultStandardList_ = missionList;
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->UpdateAbilityRecordColdStartFlag(info, true);
    EXPECT_NE(missionListManager, nullptr);
    missionListManager.reset();
    GTEST_LOG_(INFO) << "MissionListManager_UpdateAbilityRecordColdStartFlag_001 end";
}

/*
 * @tc.name: MissionListManager_RemoveMissionLocked_001
 * @tc.desc: Test RemoveMissionLocked
 * @tc.type: FUNC
 */
HWTEST_F(MissionListManagerFirstTest, MissionListManager_RemoveMissionLocked_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_RemoveMissionLocked_001 start";
    int userId = 0;
    int32_t missionId = 1;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->RemoveMissionLocked(missionId, true);
    missionListManager->RemoveMissionLocked(missionId, false);
    EXPECT_NE(missionListManager, nullptr);
    missionListManager.reset();
    GTEST_LOG_(INFO) << "MissionListManager_RemoveMissionLocked_001 end";
}
}  // namespace AAFwk
}  // namespace OHOS