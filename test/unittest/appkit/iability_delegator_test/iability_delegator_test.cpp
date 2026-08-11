/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <memory>
#include <string>

#define private public
#define protected public
#include "iability_delegator.h"
#undef private
#undef protected

#include "runtime.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {

class IAbilityDelegatorTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.number: IAbilityDelegatorTest_Create_0100
 * @tc.name: Create with null runtime
 * @tc.desc: Verify Create returns a base IAbilityDelegator when runtime is null.
 */
HWTEST_F(IAbilityDelegatorTest, IAbilityDelegatorTest_Create_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IAbilityDelegatorTest_Create_0100 start";
    auto delegator = IAbilityDelegator::Create(nullptr, nullptr, nullptr, nullptr);
    EXPECT_NE(delegator, nullptr);
    EXPECT_EQ(delegator->GetMonitorsNum(), 0u);
    EXPECT_EQ(delegator->GetStageMonitorsNum(), 0u);
    EXPECT_EQ(delegator->GetThreadName(), "");
    GTEST_LOG_(INFO) << "IAbilityDelegatorTest_Create_0100 end";
}

/**
 * @tc.number: IAbilityDelegatorTest_ClearAllMonitors_0100
 * @tc.name: ClearAllMonitors on base class
 * @tc.desc: Verify ClearAllMonitors on base IAbilityDelegator does nothing (no crash).
 */
HWTEST_F(IAbilityDelegatorTest, IAbilityDelegatorTest_ClearAllMonitors_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IAbilityDelegatorTest_ClearAllMonitors_0100 start";
    auto delegator = std::make_shared<IAbilityDelegator>();
    EXPECT_NE(delegator, nullptr);
    delegator->ClearAllMonitors();
    EXPECT_EQ(delegator->GetMonitorsNum(), 0u);
    EXPECT_EQ(delegator->GetStageMonitorsNum(), 0u);
    GTEST_LOG_(INFO) << "IAbilityDelegatorTest_ClearAllMonitors_0100 end";
}

/**
 * @tc.number: IAbilityDelegatorTest_PrepareAndOnRun_0100
 * @tc.name: Prepare and OnRun on base class
 * @tc.desc: Verify Prepare and OnRun on base IAbilityDelegator are no-ops (no crash).
 */
HWTEST_F(IAbilityDelegatorTest, IAbilityDelegatorTest_PrepareAndOnRun_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IAbilityDelegatorTest_PrepareAndOnRun_0100 start";
    auto delegator = std::make_shared<IAbilityDelegator>();
    EXPECT_NE(delegator, nullptr);
    delegator->Prepare();
    delegator->OnRun();
    EXPECT_EQ(delegator->GetThreadName(), "");
    GTEST_LOG_(INFO) << "IAbilityDelegatorTest_PrepareAndOnRun_0100 end";
}

/**
 * @tc.number: IAbilityDelegatorTest_ApiTargetVersion_0100
 * @tc.name: Get/SetApiTargetVersion
 * @tc.desc: Verify GetApiTargetVersion returns the value set by SetApiTargetVersion.
 */
HWTEST_F(IAbilityDelegatorTest, IAbilityDelegatorTest_ApiTargetVersion_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IAbilityDelegatorTest_ApiTargetVersion_0100 start";
    auto delegator = std::make_shared<IAbilityDelegator>();
    EXPECT_NE(delegator, nullptr);
    EXPECT_EQ(delegator->GetApiTargetVersion(), 0u);
    uint32_t version = 12;
    delegator->SetApiTargetVersion(version);
    EXPECT_EQ(delegator->GetApiTargetVersion(), version);
    GTEST_LOG_(INFO) << "IAbilityDelegatorTest_ApiTargetVersion_0100 end";
}
} // namespace AppExecFwk
} // namespace OHOS
