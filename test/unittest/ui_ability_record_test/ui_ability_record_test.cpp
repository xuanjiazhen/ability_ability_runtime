/*
 * Copyright (c) 
 2025 Huawei Device Co., Ltd.
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

namespace OHOS {
namespace AAFwk {
class AbilityRecordTest : public testing::TestWithParam<OHOS::AAFwk::AbilityState> {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<AbilityRecord> GetAbilityRecord();
    AbilityRequest CreateValidAbilityRequest();

    std::shared_ptr<AbilityRecord> abilityRecord_{ nullptr };
    std::shared_ptr<AbilityResult> abilityResult_{ nullptr };
    std::shared_ptr<AbilityRequest> abilityRequest_{ nullptr };
    static constexpr unsigned int CHANGE_CONFIG_LOCALE = 0x00000001;
};

void AbilityRecordTest::SetUpTestCase(void)
{
}
void AbilityRecordTest::TearDownTestCase(void)
{
}

void AbilityRecordTest::SetUp(void)
{
}

void AbilityRecordTest::TearDown(void)
{
}

/*
 * Feature: AbilityRecord
 * Function: SetPromotePriority
 * SubFunction: SetPromotePriority
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetPromotePriority
 */
HWTEST_F(AbilityRecordTest, SetPromotePriority_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetPromotePriority_001 start";
    EXPECT_NE(abilityRecord_, nullptr);
    abilityRecord_->uiAbilityProperty_ = nullptr;
    abilityRecord_->SetPromotePriority(true);
    EXPECT_NE(abilityRecord_->uiAbilityProperty_, nullptr);
    EXPECT_TRUE(abilityRecord_->uiAbilityProperty_->promotePriority);

    abilityRecord_->uiAbilityProperty_ = nullptr;
    abilityRecord_->SetPromotePriority(false);
    EXPECT_NE(abilityRecord_->uiAbilityProperty_, nullptr);
    EXPECT_FALSE(abilityRecord_->uiAbilityProperty_->promotePriority);
    GTEST_LOG_(INFO) << "SetPromotePriority_001 end";
}

/*
 * Feature: AbilityRecord
 * Function: GetPromotePriority
 * SubFunction: GetPromotePriority
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetPromotePriority
 */
HWTEST_F(AbilityRecordTest, ShouldPromotePriority_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShouldPromotePriority_001 start";
    EXPECT_NE(abilityRecord_, nullptr);
    abilityRecord_->uiAbilityProperty_ = nullptr;
    EXPECT_FALSE(abilityRecord_->GetPromotePriority());

    abilityRecord_->uiAbilityProperty_ = std::make_shared<AbilityRecord::UIAbilityProperty>();
    EXPECT_FALSE(abilityRecord_->GetPromotePriority());

    abilityRecord_->uiAbilityProperty_->promotePriority = true;
    abilityRecord_->pid_ = -1;
    EXPECT_FALSE(abilityRecord_->GetPromotePriority());

    abilityRecord_->pid_ = 1000;
    EXPECT_TRUE(abilityRecord_->GetPromotePriority());
    GTEST_LOG_(INFO) << "ShouldPromotePriority_001 end";
}

/*
 * Feature: AbilityRecord
 * Function: PromotePriority
 * SubFunction: PromotePriority
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord PromotePriority
 */
HWTEST_F(AbilityRecordTest, PromotePriority_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PromotePriority_001 start";
    EXPECT_NE(abilityRecord_, nullptr);
    abilityRecord_->isStartedByCall_ = true;
    EXPECT_TRUE(abilityRecord_->IsStartedByCall());

    abilityRecord_->uiAbilityProperty_ = std::make_shared<AbilityRecord::UIAbilityProperty>();
    abilityRecord_->uiAbilityProperty_->promotePriority = true;
    abilityRecord_->pid_ = 1000;
    EXPECT_TRUE(abilityRecord_->GetPromotePriority());

    abilityRecord_->callerList_.clear();
    auto callerInfo = std::make_shared<CallerRecord>(100, abilityRecord_);
    abilityRecord_->callerList_.emplace_back(callerInfo);
    EXPECT_NE(abilityRecord_->GetCallerInfo(), nullptr);

    abilityRecord_->PromotePriority();
    GTEST_LOG_(INFO) << "PromotePriority_001 end";
}
} // namespace AAFwk
} // namespace OHOS