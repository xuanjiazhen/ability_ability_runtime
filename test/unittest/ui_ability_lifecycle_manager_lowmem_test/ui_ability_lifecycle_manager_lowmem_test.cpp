/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "ability_manager_errors.h"
#include "ability_util.h"
#define private public
#define protected public
#include "ability_record.h"
#include "app_mgr_util.h"
#include "app_utils.h"
#include "scene_board/ui_ability_lifecycle_manager.h"
#undef protected
#undef private
#include "ability_config.h"
#include "app_scheduler.h"
#include "hilog_tag_wrapper.h"
#include "mock_app_mgr_service.h"
#include "mock_permission_verification.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t LOW_MEM_THRESHOLD = 10;
};

class UIAbilityLifecycleManagerLowMemTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<UIAbilityRecord> InitPageAbilityRecord();
    std::shared_ptr<UIAbilityRecord> InitNonPageAbilityRecord();
    std::shared_ptr<UIAbilityRecord> InitLauncherAbilityRecord();
    std::shared_ptr<UIAbilityRecord> InitKeepAliveAbilityRecord();
};

void UIAbilityLifecycleManagerLowMemTest::SetUpTestCase() {}

void UIAbilityLifecycleManagerLowMemTest::TearDownTestCase()
{
    UIAbilityLifecycleManager::SetMemoryPoolValue(0);
}

void UIAbilityLifecycleManagerLowMemTest::SetUp()
{
    UIAbilityLifecycleManager::SetMemoryPoolValue(0);
}

void UIAbilityLifecycleManagerLowMemTest::TearDown()
{
    UIAbilityLifecycleManager::SetMemoryPoolValue(0);
}

std::shared_ptr<UIAbilityRecord> UIAbilityLifecycleManagerLowMemTest::InitPageAbilityRecord()
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    return UIAbilityRecord::CreateAbilityRecord(abilityRequest);
}

std::shared_ptr<UIAbilityRecord> UIAbilityLifecycleManagerLowMemTest::InitNonPageAbilityRecord()
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "DataAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    return UIAbilityRecord::CreateAbilityRecord(abilityRequest);
}

std::shared_ptr<UIAbilityRecord> UIAbilityLifecycleManagerLowMemTest::InitLauncherAbilityRecord()
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.appInfo.isLauncherApp = true;
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    return UIAbilityRecord::CreateAbilityRecord(abilityRequest);
}

std::shared_ptr<UIAbilityRecord> UIAbilityLifecycleManagerLowMemTest::InitKeepAliveAbilityRecord()
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = LAUNCHER_BUNDLE_NAME;
    abilityRequest.abilityInfo.bundleName = LAUNCHER_BUNDLE_NAME;
    abilityRequest.abilityInfo.name = LAUNCHER_ABILITY_NAME;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    return UIAbilityRecord::CreateAbilityRecord(abilityRequest);
}

/**
 * @tc.name: SetMemoryPoolValue_BelowThreshold_001
 * @tc.desc: Set memory pool value below threshold, IsLowMem returns false
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerLowMemTest, SetMemoryPoolValue_BelowThreshold_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    UIAbilityLifecycleManager::SetMemoryPoolValue(0);
    EXPECT_FALSE(mgr->IsLowMem());
}

/**
 * @tc.name: SetMemoryPoolValue_BelowThreshold_002
 * @tc.desc: Set memory pool value below threshold, IsLowMem returns false
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerLowMemTest, SetMemoryPoolValue_BelowThreshold_002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    UIAbilityLifecycleManager::SetMemoryPoolValue(LOW_MEM_THRESHOLD - 1);
    EXPECT_FALSE(mgr->IsLowMem());
}

/**
 * @tc.name: SetMemoryPoolValue_AtThreshold_001
 * @tc.desc: Set memory pool value at threshold, IsLowMem returns true
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerLowMemTest, SetMemoryPoolValue_AtThreshold_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    UIAbilityLifecycleManager::SetMemoryPoolValue(LOW_MEM_THRESHOLD);
    EXPECT_TRUE(mgr->IsLowMem());
}

/**
 * @tc.name: SetMemoryPoolValue_AboveThreshold_001
 * @tc.desc: Set memory pool value above threshold, IsLowMem returns true
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerLowMemTest, SetMemoryPoolValue_AboveThreshold_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    UIAbilityLifecycleManager::SetMemoryPoolValue(LOW_MEM_THRESHOLD + 10);
    EXPECT_TRUE(mgr->IsLowMem());
}

/**
 * @tc.name: ShouldKillUIAbility_NotLowMem_001
 * @tc.desc: Memory sufficient, ShouldKillUIAbility returns false
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerLowMemTest, ShouldKillUIAbility_NotLowMem_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    UIAbilityLifecycleManager::SetMemoryPoolValue(0);
    auto abilityRecord = InitPageAbilityRecord();
    EXPECT_FALSE(mgr->ShouldKillUIAbility(abilityRecord));
}

/**
 * @tc.name: ShouldKillUIAbility_NullRecord_001
 * @tc.desc: Low memory with null ability record, ShouldKillUIAbility returns false
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerLowMemTest, ShouldKillUIAbility_NullRecord_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    UIAbilityLifecycleManager::SetMemoryPoolValue(LOW_MEM_THRESHOLD);
    EXPECT_FALSE(mgr->ShouldKillUIAbility(nullptr));
}

/**
 * @tc.name: ShouldKillUIAbility_NonPageType_001
 * @tc.desc: Low memory with non-PAGE ability type, ShouldKillUIAbility returns false
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerLowMemTest, ShouldKillUIAbility_NonPageType_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    UIAbilityLifecycleManager::SetMemoryPoolValue(LOW_MEM_THRESHOLD);
    auto abilityRecord = InitNonPageAbilityRecord();
    EXPECT_FALSE(mgr->ShouldKillUIAbility(abilityRecord));
}

/**
 * @tc.name: ShouldKillUIAbility_LauncherApp_001
 * @tc.desc: Low memory with launcher app, ShouldKillUIAbility returns false
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerLowMemTest, ShouldKillUIAbility_LauncherApp_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    UIAbilityLifecycleManager::SetMemoryPoolValue(LOW_MEM_THRESHOLD);
    auto abilityRecord = InitLauncherAbilityRecord();
    EXPECT_FALSE(mgr->ShouldKillUIAbility(abilityRecord));
}

/**
 * @tc.name: ShouldKillUIAbility_KeepAlive_001
 * @tc.desc: Low memory with keepAlive ability, ShouldKillUIAbility returns false
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerLowMemTest, ShouldKillUIAbility_KeepAlive_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    UIAbilityLifecycleManager::SetMemoryPoolValue(LOW_MEM_THRESHOLD);
    auto abilityRecord = InitKeepAliveAbilityRecord();
    EXPECT_FALSE(mgr->ShouldKillUIAbility(abilityRecord));
}

/**
 * @tc.name: ShouldKillUIAbility_AppMgrNull_001
 * @tc.desc: Low memory with null appMgr, ShouldKillUIAbility returns false
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerLowMemTest, ShouldKillUIAbility_AppMgrNull_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    UIAbilityLifecycleManager::SetMemoryPoolValue(LOW_MEM_THRESHOLD);
    auto abilityRecord = InitPageAbilityRecord();
    auto originAppMgr = AppMgrUtil::appMgr_;
    AppMgrUtil::appMgr_ = nullptr;
    EXPECT_FALSE(mgr->ShouldKillUIAbility(abilityRecord));
    AppMgrUtil::appMgr_ = originAppMgr;
}

/**
 * @tc.name: ShouldKillUIAbility_NotTerminating_001
 * @tc.desc: Low memory with app not terminating, ShouldKillUIAbility returns false
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerLowMemTest, ShouldKillUIAbility_NotTerminating_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    UIAbilityLifecycleManager::SetMemoryPoolValue(LOW_MEM_THRESHOLD);
    auto abilityRecord = InitPageAbilityRecord();
    auto originAppMgr = AppMgrUtil::appMgr_;
    auto appMgr = sptr<AppExecFwk::MockAppMgrService>::MakeSptr();
    EXPECT_CALL(*appMgr, IsTerminatingByPid)
        .WillOnce([](pid_t, bool &isTerminating) {
            isTerminating = false;
            return 0;
        });
    AppMgrUtil::appMgr_ = appMgr;
    EXPECT_FALSE(mgr->ShouldKillUIAbility(abilityRecord));
    AppMgrUtil::appMgr_ = originAppMgr;
}

/**
 * @tc.name: ShouldKillUIAbility_Terminating_001
 * @tc.desc: Low memory with app terminating, ShouldKillUIAbility returns true
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerLowMemTest, ShouldKillUIAbility_Terminating_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    UIAbilityLifecycleManager::SetMemoryPoolValue(LOW_MEM_THRESHOLD);
    auto abilityRecord = InitPageAbilityRecord();
    auto originAppMgr = AppMgrUtil::appMgr_;
    auto appMgr = sptr<AppExecFwk::MockAppMgrService>::MakeSptr();
    EXPECT_CALL(*appMgr, IsTerminatingByPid)
        .WillOnce([](pid_t, bool &isTerminating) {
            isTerminating = true;
            return 0;
        });
    AppMgrUtil::appMgr_ = appMgr;
    EXPECT_TRUE(mgr->ShouldKillUIAbility(abilityRecord));
    AppMgrUtil::appMgr_ = originAppMgr;
}
}  // namespace AAFwk
}  // namespace OHOS
