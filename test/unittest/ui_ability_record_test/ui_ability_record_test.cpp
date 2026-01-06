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

#include <gtest/gtest.h>

#include "ui_ability_record.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class UIAbilityRecordTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void UIAbilityRecordTest::SetUpTestCase(void)
{}
void UIAbilityRecordTest::TearDownTestCase(void)
{}
void UIAbilityRecordTest::SetUp()
{}
void UIAbilityRecordTest::TearDown()
{}

/**
 * @tc.name: ScheduleCollaborate_0010
 * @tc.desc: lifecycleDeal null or not.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, ScheduleCollaborate_0010, TestSize.Level2)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->lifecycleDeal_ = nullptr;
    abilityRecord->ScheduleCollaborate(abilityRequest.want);

    auto lifecycleDeal = std::make_shared<LifecycleDeal>();
    abilityRecord->lifecycleDeal_ = lifecycleDeal;
    EXPECT_CALL(*lifecycleDeal, ScheduleCollaborate).Times(1);
    abilityRecord->ScheduleCollaborate(abilityRequest.want);
}

/*
 * Feature: AbilityRecord
 * Function: PrepareTerminateAbilityDone
 * SubFunction: PrepareTerminateAbilityDone
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord PrepareTerminateAbilityDone
 */
HWTEST_F(UIAbilityRecordTest, AbilityRecord_PrepareTerminateAbilityDone_001, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->isPrepareTerminateAbilityCalled_.store(true);
    abilityRecord->PrepareTerminateAbilityDone(true);
    EXPECT_EQ(abilityRecord->isPrepareTerminate_, true);
}

/**
 * @tc.name: FromBaseRecord_0010
 * @tc.desc: FromBaseRecord.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, FromBaseRecord_0010, TestSize.Level2)
{
    std::shared_ptr<AbilityRecord> baseRecord;
    EXPECT_EQ(UIAbilityRecord::FromBaseRecord(baseRecord), nullptr);
    AbilityRequest abilityRequest;
    baseRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_EQ(UIAbilityRecord::FromBaseRecord(baseRecord), nullptr);
    
    baseRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(UIAbilityRecord::FromBaseRecord(baseRecord), nullptr);
}

/**
 * @tc.name: ForegroundAbility_0010
 * @tc.desc: ForegroundAbility.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, ForegroundAbility_0010, TestSize.Level2)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);

    abilityRecord->SetBackgroundDrivenFlag(true);
    abilityRecord->ForegroundAbility(0, false);
    EXPECT_TRUE(abilityRecord->IsLastWantBackgroundDriven());

    abilityRecord->ForegroundAbility(0, true);
    EXPECT_FALSE(abilityRecord->IsLastWantBackgroundDriven());

    abilityRecord->SetLastWant(std::make_shared<Want>());
    EXPECT_TRUE(abilityRecord->HasLastWant());
    abilityRecord->ForegroundAbility(0, false);
    EXPECT_TRUE(abilityRecord->HasLastWant());
    abilityRecord->ForegroundAbility(0, true);
    EXPECT_FALSE(abilityRecord->HasLastWant());
}

/**
 * @tc.name: PrepareTerminateAbility_0010
 * @tc.desc: PrepareTerminateAbility.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, PrepareTerminateAbility_0010, TestSize.Level2)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->isPrepareTerminateAbilityCalled_.store(false);
    abilityRecord->PrepareTerminateAbility(false);
    EXPECT_FALSE(abilityRecord->isPrepareTerminateAbilityCalled_.load());

    abilityRecord->PrepareTerminateAbility(true);
    EXPECT_TRUE(abilityRecord->isPrepareTerminateAbilityCalled_.load());
}

/**
 * @tc.name: PrepareTerminateAbilityDone_0010
 * @tc.desc: PrepareTerminateAbilityDone.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, PrepareTerminateAbilityDone_0010, TestSize.Level2)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->isPrepareTerminate_ = false;

    abilityRecord->isPrepareTerminateAbilityCalled_.store(false);
    abilityRecord->PrepareTerminateAbilityDone(true);
    EXPECT_FALSE(abilityRecord->isPrepareTerminate_);

    abilityRecord->isPrepareTerminateAbilityCalled_.store(true);
    abilityRecord->PrepareTerminateAbility(true);
    EXPECT_TRUE(abilityRecord->isPrepareTerminate_);
}

/**
 * @tc.name: CancelPrepareTerminate_0010
 * @tc.desc: CancelPrepareTerminate.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CancelPrepareTerminate_0010, TestSize.Level2)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->isPrepareTerminate_ = false;

    abilityRecord->isPrepareTerminateAbilityCalled_.store(false);
    abilityRecord->CancelPrepareTerminate();
    EXPECT_FALSE(abilityRecord->isPrepareTerminate_);

    abilityRecord->isPrepareTerminateAbilityCalled_.store(true);
    abilityRecord->CancelPrepareTerminate();
    EXPECT_TRUE(abilityRecord->isPrepareTerminate_);
}
}  // namespace AAFwk
}  // namespace OHOS
