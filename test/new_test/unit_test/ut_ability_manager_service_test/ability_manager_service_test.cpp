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
#include <mutex>

#define private public
#include "ability_manager_service.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AbilityManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityManagerServiceTest::SetUpTestCase()
{}

void AbilityManagerServiceTest::TearDownTestCase()
{}

void AbilityManagerServiceTest::SetUp()
{}

void AbilityManagerServiceTest::TearDown()
{}

/**
 * @tc.name: OnStart_001
 * @tc.desc: Verify Onstart callback method case 1 state_ is STATE_RUNNING
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceTest, OnStart_001, TestSize.Level1)
{
    std::shared_ptr<AAFwk::AbilityManagerService> abilityMgr = std::make_shared<AAFwk::AbilityManagerService>();
    abilityMgr->state_ = AAFwk::ServiceRunningState::STATE_RUNNING;
    abilityMgr->OnStart();
    EXPECT_EQ(abilityMgr->state_, AAFwk::ServiceRunningState::STATE_RUNNING);
}

/**
 * @tc.name: OnStart_002
 * @tc.desc: Verify Onstart callback methodcase 2
 *           state_ is STATE_NOT_START
 *           GetBundleManagerHelper return valid bundlemgr
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceTest, OnStart_002, TestSize.Level1)
{
    std::shared_ptr<AAFwk::AbilityManagerService> abilityMgr = std::make_shared<AAFwk::AbilityManagerService>();
    abilityMgr->state_ = AAFwk::ServiceRunningState::STATE_NOT_START;
    // OH_MOCK_METHOD_EXPECT_RET({DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()}, _, GetBundleManagerHelper);
    abilityMgr->OnStart();
    EXPECT_EQ(abilityMgr->state_, AAFwk::ServiceRunningState::STATE_RUNNING);
}

/**
 * @tc.name: OnStart_003
 * @tc.desc: Verify Onstart callback method case 3
 *           state_ is STATE_NOT_START
 *           GetBundleManagerHelper return nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceTest, OnStart_003, TestSize.Level1)
{
    std::shared_ptr<AAFwk::AbilityManagerService> abilityMgr = std::make_shared<AAFwk::AbilityManagerService>();
    abilityMgr->state_ = AAFwk::ServiceRunningState::STATE_NOT_START;
    // OH_MOCK_METHOD_EXPECT_RET({nullptr}, _, GetBundleManagerHelper);
    abilityMgr->OnStart();
    EXPECT_EQ(abilityMgr->state_, AAFwk::ServiceRunningState::STATE_RUNNING);
}
} // namespace AppExecFwk
} // namespace OHOS
