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

namespace OHOS {
namespace AAFwk {
using namespace testing::ext;

class MissionListManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void MissionListManagerTest::SetUpTestCase(void)
{}

void MissionListManagerTest::TearDownTestCase(void)
{}

void MissionListManagerTest::SetUp()
{}

void MissionListManagerTest::TearDown()
{}

/**
 * @tc.name: MissionListManager_DoAbilityForeground_001
 * @tc.desc: Test DoAbilityForeground
 * @tc.type: FUNC
 */
HWTEST_F(MissionListManagerTest, MissionListManager_DoAbilityForeground_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_DoAbilityForeground_001 begin";
    std::shared_ptr<MissionListManager> missionListManager = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    uint32_t flag = 0;

    int32_t ret = missionListManager->DoAbilityForeground(abilityRecord, flag);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);

    ret = missionListManager->DoAbilityForeground(abilityRecord, flag);
    EXPECT_EQ(ret, ERR_OK);
    ret = missionListManager->DoAbilityForeground(abilityRecord, flag);
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "MissionListManager_DoAbilityForeground_001 end";
}

/**
 * @tc.name: MissionListManager_GetActiveAbilityList_001
 * @tc.desc: Test GetActiveAbilityList
 * @tc.type: FUNC
 */
HWTEST_F(MissionListManagerTest, MissionListManager_GetActiveAbilityList_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_GetActiveAbilityList_001 begin";
    int32_t uid = 100;
    int32_t pid = 101;
    std::vector<std::string> abilityList;
    std::shared_ptr<MissionListManager> missionListManager = std::make_shared<MissionListManager>(0);
    missionListManager->GetActiveAbilityList(uid, abilityList, pid);

    std::shared_ptr<MissionList> missionListNull = nullptr;
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionListManager->currentMissionLists_.push_back(missionListNull);
    missionListManager->currentMissionLists_.push_back(missionList);
    missionListManager->defaultStandardList_ = missionList;
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->GetActiveAbilityList(uid, abilityList, pid);
    EXPECT_EQ(abilityList.size(), 0);

    uid = -1;
    pid = NO_PID;
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->abilityInfo_.name = "abilityName";
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(0, abilityRecord, "");
    missionList->missions_.push_back(mission);
    missionListManager->currentMissionLists_.push_back(missionList);
    missionListManager->defaultStandardList_ = missionList;
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->GetActiveAbilityList(uid, abilityList, pid);
    EXPECT_EQ(abilityList.size(), 1);
    GTEST_LOG_(INFO) << "MissionListManager_GetActiveAbilityList_001 end";
}

/**
 * @tc.name: MissionListManager_IsAppLastAbility_001
 * @tc.desc: Test IsAppLastAbility
 * @tc.type: FUNC
 */
HWTEST_F(MissionListManagerTest, MissionListManager_IsAppLastAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_IsAppLastAbility_001 begin";
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    std::shared_ptr<MissionListManager> missionListManager = std::make_shared<MissionListManager>(0);
    bool ret = missionListManager->IsAppLastAbility(abilityRecord);
    EXPECT_FALSE(ret);

    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    ret = missionListManager->IsAppLastAbility(abilityRecord);
    EXPECT_FALSE(ret);

    std::shared_ptr<MissionList> missionListNull = nullptr;
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionListManager->currentMissionLists_.push_back(missionListNull);
    missionListManager->currentMissionLists_.push_back(missionList);
    ret = missionListManager->IsAppLastAbility(abilityRecord);
    EXPECT_FALSE(ret);
    missionListManager->currentMissionLists_.clear();

    abilityRecord->abilityInfo_.name = "abilityName";
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(0, abilityRecord, "");
    missionList->missions_.push_back(mission);
    missionListManager->currentMissionLists_.push_back(missionList);
    ret = missionListManager->IsAppLastAbility(abilityRecord);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "MissionListManager_IsAppLastAbility_001 end";
}
}  // namespace AAFwk
}  // namespace OHOS
