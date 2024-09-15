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
#define protected public
#include "ams_mgr_scheduler.h"
#include "mock_ipc_skeleton.h"
#include "mock_permission_verification.h"
#include "mock_my_flag.h"
#undef private
#undef protected

#include "ability_info.h"
#include "application_info.h"
#include "mock_app_mgr_service_inner.h"
#include "mock_native_token.h"
#include "mock_task_handler_wrap.h"
#include "param.h"
#include "perf_profile.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr const char* SCENE_BOARD_BUNDLE_NAME = "com.ohos.sceneboard";
constexpr const char* SCENEBOARD_ABILITY_NAME = "com.ohos.sceneboard.MainAbility";
}

class AmsMgrSchedulerSecondTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        taskHandler_ = MockTaskHandlerWrap::CreateQueueHandler("AmsMgrSchedulerSecondTest");
        appMgrServiceInner_ = std::make_shared<MockAppMgrServiceInner>();
        appMgrServiceInner_->SetSceneBoardAttachFlag(true);
    }

    static void TearDownTestCase() {}

    void SetUp() override {}

    void TearDown() override {}

protected:
    static const std::string GetTestAppName()
    {
        return "test_app_name";
    }

    static const std::string GetTestAbilityName()
    {
        return "test_ability_name";
    }

    static std::shared_ptr<MockAppMgrServiceInner> appMgrServiceInner_;
    static std::shared_ptr<MockTaskHandlerWrap> taskHandler_;
};

std::shared_ptr<MockAppMgrServiceInner> AmsMgrSchedulerSecondTest::appMgrServiceInner_ = nullptr;
std::shared_ptr<MockTaskHandlerWrap> AmsMgrSchedulerSecondTest::taskHandler_ = nullptr;

/**
 * @tc.name: AmsMgrSchedulerSecondTest_LoadAbility_001
 * @tc.desc: Test LoadAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_LoadAbility_001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize params
     */
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);
    std::shared_ptr<AbilityInfo> abilityInfo = nullptr;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::shared_ptr<AbilityRuntime::LoadParam> loadParam = std::make_shared<AbilityRuntime::LoadParam>();

    /**
     * @tc.steps: step2. LoadAbility with null abilityInfo
     * @tc.expected: step2. expect GetSceneBoardAttachFlag true
     */
    amsMgrScheduler->LoadAbility(abilityInfo, appInfo, want, loadParam);
    EXPECT_TRUE(appMgrServiceInner_->GetSceneBoardAttachFlag()); // not changed
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_LoadAbility_002
 * @tc.desc: Test LoadAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_LoadAbility_002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize params
     */
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> appInfo = nullptr;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::shared_ptr<AbilityRuntime::LoadParam> loadParam = std::make_shared<AbilityRuntime::LoadParam>();

    /**
     * @tc.steps: step2. LoadAbility with null appInfo
     * @tc.expected: step2. expect GetSceneBoardAttachFlag true
     */
    amsMgrScheduler->LoadAbility(abilityInfo, appInfo, want, loadParam);
    EXPECT_TRUE(appMgrServiceInner_->GetSceneBoardAttachFlag()); // not changed
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_LoadAbility_003
 * @tc.desc: Test LoadAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_LoadAbility_003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize params
     */
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::shared_ptr<AbilityRuntime::LoadParam> loadParam = std::make_shared<AbilityRuntime::LoadParam>();

    /**
     * @tc.steps: step2. LoadAbility with error permission
     * @tc.expected: step2. expect GetSceneBoardAttachFlag true
     */
    IPCSkeleton::SetCallingUid(-1);
    amsMgrScheduler->LoadAbility(abilityInfo, appInfo, want, loadParam);
    EXPECT_TRUE(appMgrServiceInner_->GetSceneBoardAttachFlag());
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_LoadAbility_004
 * @tc.desc: Test LoadAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_LoadAbility_004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize params
     */
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::shared_ptr<AbilityRuntime::LoadParam> loadParam = std::make_shared<AbilityRuntime::LoadParam>();

    /**
     * @tc.steps: step2. LoadAbility with permission FOUNDATION_UID
     * @tc.expected: step2. expect taskhandler executed at least one time
     */
    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    amsMgrScheduler->LoadAbility(abilityInfo, appInfo, want, loadParam);
    EXPECT_CALL(*taskHandler_, SubmitTaskInner(_, _)).WillRepeatedly(Return(nullptr));
    EXPECT_TRUE(appMgrServiceInner_->GetSceneBoardAttachFlag());
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_LoadAbility_005
 * @tc.desc: Test LoadAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_LoadAbility_005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize params
     */
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->bundleName = SCENE_BOARD_BUNDLE_NAME;
    abilityInfo->name = SCENEBOARD_ABILITY_NAME;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::shared_ptr<AbilityRuntime::LoadParam> loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    EXPECT_TRUE(appMgrServiceInner_->GetSceneBoardAttachFlag()); // default true

    /**
     * @tc.steps: step2. LoadAbility with permission FOUNDATION_UID
     * @tc.expected: step2. expect appMgrServiceInner_ GetSceneBoardAttachFlag false
     */
    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    amsMgrScheduler->LoadAbility(abilityInfo, appInfo, want, loadParam);
    EXPECT_CALL(*taskHandler_, SubmitTaskInner(_, _)).WillRepeatedly(Return(nullptr));
    EXPECT_FALSE(appMgrServiceInner_->GetSceneBoardAttachFlag()); // false
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_LoadAbility_006
 * @tc.desc: Test LoadAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_LoadAbility_006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize params
     */
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->bundleName = SCENE_BOARD_BUNDLE_NAME;
    abilityInfo->name = "";
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::shared_ptr<AbilityRuntime::LoadParam> loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    appMgrServiceInner_->SetSceneBoardAttachFlag(true); // set true

    /**
     * @tc.steps: step2. LoadAbility with permission FOUNDATION_UID
     * @tc.expected: step2. expect appMgrServiceInner_ GetSceneBoardAttachFlag true
     */
    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    amsMgrScheduler->LoadAbility(abilityInfo, appInfo, want, loadParam);
    EXPECT_CALL(*taskHandler_, SubmitTaskInner(_, _)).WillRepeatedly(Return(nullptr));
    EXPECT_TRUE(appMgrServiceInner_->GetSceneBoardAttachFlag());
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_UpdateAbilityState_002
 * @tc.desc: Test UpdateAbilityState
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_UpdateAbilityState_002, TestSize.Level1)
{
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);
    EXPECT_CALL(*taskHandler_, SubmitTaskInner(_, _)).Times(0);
    IPCSkeleton::SetCallingUid(-1);
    amsMgrScheduler->UpdateAbilityState(nullptr, AbilityState::ABILITY_STATE_CREATE);
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_UpdateAbilityState_003
 * @tc.desc: Test UpdateAbilityState
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_UpdateAbilityState_003, TestSize.Level0)
{
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);
    EXPECT_CALL(*taskHandler_, SubmitTaskInner(_, _)).Times(1).WillRepeatedly(Return(nullptr));
    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    amsMgrScheduler->UpdateAbilityState(nullptr, AbilityState::ABILITY_STATE_CREATE);
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_KillProcessesByUserId_001
 * @tc.desc: Test KillProcessesByUserId
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_KillProcessesByUserId_001, TestSize.Level1)
{
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);
    EXPECT_CALL(*taskHandler_, SubmitTaskInner(_, _)).Times(0);
    appMgrServiceInner_.reset(); // not ready

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false 
     * @tc.expected: step1. expect taskHandler_ time(0)
     */
    amsMgrScheduler->KillProcessesByUserId(-1);
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_KillProcessesByUserId_002
 * @tc.desc: Test KillProcessesByUserId
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_KillProcessesByUserId_002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. amsMgrScheduler permission not ok 
     * @tc.expected: step1. expect taskHandler_ time(0)
     */
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);

    EXPECT_CALL(*taskHandler_, SubmitTaskInner(_, _)).Times(0);
    IPCSkeleton::SetCallingUid(-1);
    amsMgrScheduler->KillProcessesByUserId(-1);
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_KillProcessesByUserId_003
 * @tc.desc: Test KillProcessesByUserId
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_KillProcessesByUserId_003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. amsMgrScheduler 
     * @tc.expected: step1. expect taskHandler time(AtLeast(1))
     */
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);

    EXPECT_CALL(*taskHandler_, SubmitTaskInner(_, _)).Times(0);
    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    amsMgrScheduler->KillProcessesByUserId(-1);
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_KillProcessesByUserId_004
 * @tc.desc: Test KillProcessesByUserId
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_KillProcessesByUserId_004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. amsMgrScheduler 
     * @tc.expected: step1. expect taskHandler time(AtLeast(1))
     */
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);

    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    MockNativeToken::SetNativeToken();
    MyFlag::flag_ = 1;
    EXPECT_CALL(*taskHandler_, SubmitTaskInner(_, _)).Times(AtLeast(1)).WillRepeatedly(Return(nullptr));
    amsMgrScheduler->KillProcessesByUserId(-1);
    MyFlag::flag_ = 0;
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_KillProcessesByUserId_001
 * @tc.desc: Test KillProcessesByUserId
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_KillProcessesByPids_001, TestSize.Level1)
{
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);
    EXPECT_CALL(*taskHandler_, SubmitTaskInner(_, _)).Times(0);
    appMgrServiceInner_ = nullptr; // not ready

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false 
     * @tc.expected: step1. expect taskHandler_ time(0)
     */
    std::vector<int32_t> pids;
    amsMgrScheduler->KillProcessesByPids(pids);
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_KillProcessesByPids_002
 * @tc.desc: Test KillProcessesByUserId
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_KillProcessesByPids_002, TestSize.Level1)
{
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);
    EXPECT_CALL(*taskHandler_, SubmitTaskInner(_, _)).Times(0);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false 
     * @tc.expected: step1. expect taskHandler_ time(0)
     */
    std::vector<int32_t> pids = { 1 };
    amsMgrScheduler->KillProcessesByPids(pids);
}

} // AppExecFwk
} // OHOS

