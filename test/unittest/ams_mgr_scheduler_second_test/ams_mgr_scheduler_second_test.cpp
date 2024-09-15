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
#include "mock_task_handler_wrap.h"
#include "mock_app_mgr_service_inner.h"
#include "param.h"
#include "perf_profile.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
class AmsMgrSchedulerSecondTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        taskHandler_ = MockTaskHandlerWrap::CreateQueueHandler("AmsMgrSchedulerSecondTest");
        appMgrServiceInner_ = std::make_shared<MockAppMgrServiceInner>();
        amsMgrScheduler_ = std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);
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

    static std::shared_ptr<AmsMgrScheduler> amsMgrScheduler_;
    static std::shared_ptr<MockAppMgrServiceInner> appMgrServiceInner_;
    static std::shared_ptr<MockTaskHandlerWrap> taskHandler_;
};

std::shared_ptr<AmsMgrScheduler> AmsMgrSchedulerSecondTest::amsMgrScheduler_ = nullptr;
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
    std::shared_ptr<AbilityInfo> abilityInfo = nullptr;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::shared_ptr<AbilityRuntime::LoadParam> loadParam = std::make_shared<AbilityRuntime::LoadParam>();

    /**
     * @tc.steps: step2. LoadAbility with null abilityInfo
     * @tc.expected: step2. expect taskhandler not executed
     */
    amsMgrScheduler_->LoadAbility(abilityInfo, appInfo, want, loadParam);
    EXPECT_CALL(*taskHandler_, SubmitTaskInner(_, _)).Times(0);
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
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> appInfo = nullptr;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::shared_ptr<AbilityRuntime::LoadParam> loadParam = std::make_shared<AbilityRuntime::LoadParam>();

    /**
     * @tc.steps: step2. LoadAbility with null appInfo
     * @tc.expected: step2. expect taskhandler not executed
     */
    amsMgrScheduler_->LoadAbility(abilityInfo, appInfo, want, loadParam);
    EXPECT_CALL(*taskHandler_, SubmitTaskInner(_, _)).Times(0);
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
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::shared_ptr<AbilityRuntime::LoadParam> loadParam = std::make_shared<AbilityRuntime::LoadParam>();

    /**
     * @tc.steps: step2. LoadAbility with error permission
     * @tc.expected: step2. expect taskhandler not executed
     */
    IPCSkeleton::SetCallingUid(-1);
    amsMgrScheduler_->LoadAbility(abilityInfo, appInfo, want, loadParam);
    EXPECT_CALL(*taskHandler_, SubmitTaskInner(_, _)).Times(0);
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
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::shared_ptr<AbilityRuntime::LoadParam> loadParam = std::make_shared<AbilityRuntime::LoadParam>();

    /**
     * @tc.steps: step2. LoadAbility with permission FOUNDATION_UID
     * @tc.expected: step2. expect taskhandler executed
     */
    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    amsMgrScheduler_->LoadAbility(abilityInfo, appInfo, want, loadParam);
    EXPECT_CALL(*taskHandler_, SubmitTaskInner(_, _)).Times(1);
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
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::shared_ptr<AbilityRuntime::LoadParam> loadParam = std::make_shared<AbilityRuntime::LoadParam>();

    /**
     * @tc.steps: step2. LoadAbility with permission FOUNDATION_UID
     * @tc.expected: step2. expect taskhandler executed
     */
    IPCSkeleton::SetCallingUid(-1);
    amsMgrScheduler_->LoadAbility(abilityInfo, appInfo, want, loadParam);
    EXPECT_CALL(*taskHandler_, SubmitTaskInner(_, _)).Times(0);
}
} // AppExecFwk
} // OHOS

