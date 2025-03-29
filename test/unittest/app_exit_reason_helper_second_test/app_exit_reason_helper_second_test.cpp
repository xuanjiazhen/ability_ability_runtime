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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ability_event_handler.h"
#include "ability_manager_service.h"
#include "app_exit_reason_helper.h"
#include "bundle_mgr_helper.h"
#include "mock_iapp_mgr.h"
#include "mock_mission_list_manager_interface.h"
#include "mock_os_account_manager_wrapper.h"
#include "mock_scene_board_judgement.h"

#include "utils/ability_util.h"
#include "utils/app_mgr_util.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
namespace OHOS {
namespace AbilityRuntime {
class AppExitReasonHelperTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AppExitReasonHelperTest::SetUpTestCase(void) {}

void AppExitReasonHelperTest::TearDownTestCase(void) {}

void AppExitReasonHelperTest::SetUp() {}

void AppExitReasonHelperTest::TearDown() {}

/**
 * @tc.name: GetActiveAbilityList_0100
 * @tc.desc: GetActiveAbilityList
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppExitReasonHelperTest, GetActiveAbilityList_0100, TestSize.Level1)
{
    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
        .Times(AnyNumber())
        .WillRepeatedly(Return(true));
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(AAFwk::AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    std::weak_ptr<AAFwk::AbilityManagerService> service;
    service = AAFwk::AbilityManagerService::GetPubInstance();
    std::shared_ptr<AAFwk::AbilityEventHandler> eventHandler_;
    eventHandler_ = std::make_shared<AAFwk::AbilityEventHandler>(taskHandler_, service);
    std::shared_ptr<AAFwk::SubManagersHelper> subManagersHelper_;
    subManagersHelper_ = std::make_shared<AAFwk::SubManagersHelper>(taskHandler_, eventHandler_);
    std::shared_ptr<AAFwk::AppExitReasonHelper> appExitReasonHelper_;
    appExitReasonHelper_ = std::make_shared<AAFwk::AppExitReasonHelper>(subManagersHelper_);

    int32_t uid = 0;
    std::vector<std::string> abilityLists;
    appExitReasonHelper_->GetActiveAbilityList(uid, abilityLists);
    EXPECT_EQ(abilityLists.size(), 0);
}

/**
 * @tc.name: RecordUIAbilityExitReason_0100
 * @tc.desc: RecordUIAbilityExitReason
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppExitReasonHelperTest, RecordUIAbilityExitReason_0100, TestSize.Level1)
{
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(AAFwk::AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    std::weak_ptr<AAFwk::AbilityManagerService> service;
    service = AAFwk::AbilityManagerService::GetPubInstance();
    std::shared_ptr<AAFwk::AbilityEventHandler> eventHandler_;
    eventHandler_ = std::make_shared<AAFwk::AbilityEventHandler>(taskHandler_, service);
    std::shared_ptr<AAFwk::SubManagersHelper> subManagersHelper_;
    subManagersHelper_ = std::make_shared<AAFwk::SubManagersHelper>(taskHandler_, eventHandler_);
    std::shared_ptr<AAFwk::AppExitReasonHelper> appExitReasonHelper_;
    appExitReasonHelper_ = std::make_shared<AAFwk::AppExitReasonHelper>(subManagersHelper_);

    int32_t pid = 0;
    std::string abilityName("entry111");
    AAFwk::ExitReason exitReason = { AAFwk::REASON_USER_REQUEST, "User Request" };
    int32_t result = appExitReasonHelper_->RecordUIAbilityExitReason(pid, abilityName, exitReason);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: GetActiveAbilityListWithPid_0100
 * @tc.desc: GetActiveAbilityListWithPid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppExitReasonHelperTest, GetActiveAbilityListWithPid_0100, TestSize.Level1)
{
    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
        .Times(AnyNumber())
        .WillRepeatedly(Return(true));
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(AAFwk::AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    std::weak_ptr<AAFwk::AbilityManagerService> service;
    service = AAFwk::AbilityManagerService::GetPubInstance();
    std::shared_ptr<AAFwk::AbilityEventHandler> eventHandler_;
    eventHandler_ = std::make_shared<AAFwk::AbilityEventHandler>(taskHandler_, service);
    std::shared_ptr<AAFwk::SubManagersHelper> subManagersHelper_;
    subManagersHelper_ = std::make_shared<AAFwk::SubManagersHelper>(taskHandler_, eventHandler_);
    std::shared_ptr<AAFwk::AppExitReasonHelper> appExitReasonHelper_;
    appExitReasonHelper_ = std::make_shared<AAFwk::AppExitReasonHelper>(subManagersHelper_);

    int32_t uid = 0;
    int32_t pid = 0;
    std::vector<std::string> abilityList = {};
    int32_t result = appExitReasonHelper_->GetActiveAbilityListWithPid(uid, abilityList, pid);
    EXPECT_EQ(abilityList.size(), 0);
}

/**
 * @tc.name: RecordAppExitReason_0100
 * @tc.desc: RecordAppExitReason
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppExitReasonHelperTest, RecordAppExitReason_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecordAppExitReason_0100 start";

    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
        .Times(AnyNumber())
        .WillRepeatedly(Return(true));
    auto appExitReasonHelper = std::make_shared<AppExitReasonHelper>(nullptr);
    EXPECT_NE(appExitReasonHelper, nullptr);
    AbilityUtil::GetBundleManagerHelper()->getNameAndIndexForUid_ = false;
    ExitReason exitReason;
    int32_t result = appExitReasonHelper->RecordAppExitReason(exitReason);
    EXPECT_EQ(result, -1);

    AbilityUtil::GetBundleManagerHelper()->getNameAndIndexForUid_ = true;
    exitReason.reason = Reason::REASON_CPP_CRASH;
    std::shared_ptr<SubManagersHelper> subManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(subManagersHelper, nullptr);
    subManagersHelper->currentConnectManager_ = nullptr;
    subManagersHelper->currentUIAbilityManager_ = nullptr;
    appExitReasonHelper->subManagersHelper_ = subManagersHelper;
    result = appExitReasonHelper->RecordAppExitReason(exitReason);
    EXPECT_NE(result, 0);

    auto currentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(currentUIAbilityManager, nullptr);
    currentUIAbilityManager->sessionAbilityMap_.clear();
    appExitReasonHelper->subManagersHelper_->currentUIAbilityManager_ = currentUIAbilityManager;
    result = appExitReasonHelper->RecordAppExitReason(exitReason);
    EXPECT_NE(result, 0);

    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->SetPid(IPCSkeleton::GetCallingPid());
    abilityRecord->abilityInfo_.applicationInfo.uid = IPCSkeleton::GetCallingUid();
    abilityRecord->abilityInfo_.name = "test";
    abilityRecord->abilityInfo_.launchMode = AppExecFwk::LaunchMode::STANDARD;
    abilityRecord->sessionInfo_ = new SessionInfo();
    EXPECT_NE(abilityRecord->sessionInfo_, nullptr);
    appExitReasonHelper->subManagersHelper_->currentUIAbilityManager_->sessionAbilityMap_.emplace(
        IPCSkeleton::GetCallingPid(), abilityRecord);
    result = appExitReasonHelper->RecordAppExitReason(exitReason);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "RecordAppExitReason_0100 end";
}

/**
 * @tc.name: RecordProcessExitReason_0200
 * @tc.desc: RecordProcessExitReason
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppExitReasonHelperTest, RecordProcessExitReason_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecordProcessExitReason_0200 start";

    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
        .Times(AnyNumber())
        .WillRepeatedly(Return(true));
    auto appExitReasonHelper = std::make_shared<AppExitReasonHelper>(nullptr);
    EXPECT_NE(appExitReasonHelper, nullptr);

    int32_t pid = -1;
    ExitReason exitReason;
    bool fromKillWithReason = false;
    int32_t result = appExitReasonHelper->RecordProcessExitReason(pid, exitReason, fromKillWithReason);
    EXPECT_EQ(result, -1);

    pid = 1;
    appExitReasonHelper->subManagersHelper_ = nullptr;
    result = appExitReasonHelper->RecordProcessExitReason(pid, exitReason, fromKillWithReason);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "RecordProcessExitReason_0200 end";
}

/**
 * @tc.name: RecordProcessExitReason_0300
 * @tc.desc: RecordProcessExitReason
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppExitReasonHelperTest, RecordProcessExitReason_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecordProcessExitReason_0200 start";

    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
        .Times(AnyNumber())
        .WillRepeatedly(Return(true));
    auto appExitReasonHelper = std::make_shared<AppExitReasonHelper>(nullptr);
    EXPECT_NE(appExitReasonHelper, nullptr);

    AAFwk::AppMgrUtil::appMgr_ = new AppExecFwk::MockIAppMgr();
    EXPECT_NE(AAFwk::AppMgrUtil::appMgr_, nullptr);
    ExitReason exitReason;
    int32_t result = appExitReasonHelper->RecordProcessExitReason(0, 0, exitReason);
    EXPECT_EQ(result, -1);

    result = appExitReasonHelper->RecordProcessExitReason(1, 1, exitReason);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "RecordProcessExitReason_0300 end";
}

/**
 * @tc.name: RecordProcessExitReason_0400
 * @tc.desc: RecordProcessExitReason
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppExitReasonHelperTest, RecordProcessExitReason_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecordProcessExitReason_0400 start";

    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
        .Times(AnyNumber())
        .WillRepeatedly(Return(true));
    auto appExitReasonHelper = std::make_shared<AppExitReasonHelper>(nullptr);
    EXPECT_NE(appExitReasonHelper, nullptr);

    int32_t pid = -1;
    std::string bundleName;
    int32_t uid = 1;
    uint32_t accessTokenId = 0;
    ExitReason exitReason;
    AppExecFwk::RunningProcessInfo processInfo;
    bool fromKillWithReason = false;
    bool searchDead = true;
    appExitReasonHelper->subManagersHelper_ = nullptr;
    int32_t result = appExitReasonHelper->RecordProcessExitReason(
        pid, bundleName, uid, accessTokenId, exitReason, processInfo, fromKillWithReason, searchDead);
    EXPECT_NE(result, 0);

    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
        .Times(AnyNumber())
        .WillRepeatedly(Return(false));
    pid = 1;
    appExitReasonHelper->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(appExitReasonHelper->subManagersHelper_, nullptr);
    std::shared_ptr<MissionListManagerInterface> missionListManager =
        std::make_shared<MockMissionListManagerInterface>();
    EXPECT_NE(missionListManager, nullptr);
    appExitReasonHelper->subManagersHelper_->missionListManagers_.emplace(0, missionListManager);
    result = appExitReasonHelper->RecordProcessExitReason(
        pid, bundleName, uid, accessTokenId, exitReason, processInfo, fromKillWithReason, searchDead);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "RecordProcessExitReason_0400 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
