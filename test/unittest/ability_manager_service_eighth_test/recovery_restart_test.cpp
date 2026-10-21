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

#include "ability_manager_service.h"
#include "app_mgr_util.h"
#include "ipc_skeleton.h"
#include "mock_app_mgr_service.h"
#include "mock_scene_board_judgement.h"

// Use the service library's singleton instead of instantiating a second copy in the test executable.
extern template std::shared_ptr<OHOS::AAFwk::AppScheduler>
    OHOS::DelayedSingleton<OHOS::AAFwk::AppScheduler>::instance_;

using namespace testing;
using namespace testing::ext;

namespace OHOS::AAFwk {
namespace {
constexpr int32_t USER_ID = 100;
constexpr int32_t APP_UID = 20000123;
constexpr pid_t APP_PID = 12345;
const std::string BUNDLE_NAME = "com.example.recovery";
const std::string INSTANCE_KEY = "recovery_instance";

class RecoveryAppMgrClient : public AppExecFwk::AppMgrClient {
public:
    MOCK_METHOD(int32_t, GetRunningProcessInfoByPid,
        (pid_t, AppExecFwk::RunningProcessInfo &), (const, override));
    MOCK_METHOD(AppExecFwk::AppMgrResultCode, KillApplicationByUid,
        (const std::string &, const int, const std::string &), (override));
};

class RecoveryAppMgrService : public AppExecFwk::MockAppMgrService {
public:
    MOCK_METHOD(int32_t, SignRestartAppFlag, (int32_t, const std::string &), (override));
    MOCK_METHOD(int32_t, KillAppSelfWithInstanceKey, (const std::string &, bool, const std::string &), (override));
};
} // namespace

class AbilityManagerServiceRecoveryRestartTest : public testing::Test {
public:
    void SetUp() override
    {
        originalAppMgr_ = AppMgrUtil::appMgr_;
        appMgr_ = sptr<RecoveryAppMgrService>::MakeSptr();
        AppMgrUtil::appMgr_ = appMgr_;
        auto scheduler = DelayedSingleton<AppScheduler>::GetInstance();
        originalClient_ = std::move(scheduler->appMgrClient_);
        auto client = std::make_unique<RecoveryAppMgrClient>();
        client_ = client.get();
        scheduler->appMgrClient_ = std::move(client);
        service_ = std::make_shared<AbilityManagerService>();
        auto managers = std::make_shared<SubManagersHelper>(nullptr, nullptr);
        managers->commonExtensionManagers_.emplace(USER_ID, std::make_shared<CommonExtensionManager>(USER_ID));
        managers->uiExtensionAbilityManagers_.emplace(USER_ID, std::make_shared<UIExtensionAbilityManager>(USER_ID));
        managers->uiAbilityManagers_.emplace(USER_ID, std::make_shared<UIAbilityLifecycleManager>());
        service_->subManagersHelper_ = managers;
        EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
            .WillRepeatedly(Return(true));
    }

    void TearDown() override
    {
        DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(originalClient_);
        AppMgrUtil::appMgr_ = originalAppMgr_;
        Mock::VerifyAndClearExpectations(&Rosen::SceneBoardJudgement::GetInstance());
        service_.reset();
    }

    AbilityManagerService::SignRestartAppFlagParam RecoveryParam()
    {
        EXPECT_CALL(*appMgr_, SignRestartAppFlag(APP_UID, INSTANCE_KEY)).WillOnce(Return(ERR_OK));
        AbilityManagerService::SignRestartAppFlagParam param = {
            USER_ID, APP_UID, INSTANCE_KEY, AppExecFwk::MultiAppModeType::UNSPECIFIED, true, false
        };
        param.bundleName = BUNDLE_NAME;
        return param;
    }

    std::shared_ptr<AbilityRecord> CreateRecoveryRecord(uint32_t tokenId)
    {
        AbilityRequest request;
        request.uid = APP_UID;
        request.want.SetElementName(BUNDLE_NAME, "EntryAbility");
        request.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
        request.abilityInfo.name = "EntryAbility";
        request.abilityInfo.bundleName = BUNDLE_NAME;
        request.appInfo.bundleName = BUNDLE_NAME;
        request.appInfo.accessTokenId = tokenId;
        auto record = AbilityRecord::CreateAbilityRecord(request);
        record->SetPid(APP_PID);
        record->SetAbilityState(AbilityState::FOREGROUND);
        return record;
    }

    sptr<AppExecFwk::IAppMgr> originalAppMgr_;
    sptr<RecoveryAppMgrService> appMgr_;
    std::unique_ptr<AppExecFwk::AppMgrClient> originalClient_;
    RecoveryAppMgrClient *client_ = nullptr;
    std::shared_ptr<AbilityManagerService> service_;
};

/**
 * @tc.name: RecoveryUsesValidatedBundleAndUid
 * @tc.desc: Recovery exits the application by explicit identity, independent of the Binder caller PID.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceRecoveryRestartTest, RecoveryUsesValidatedBundleAndUid, TestSize.Level1)
{
    EXPECT_CALL(*client_, KillApplicationByUid(BUNDLE_NAME, APP_UID, "RestartApp"))
        .WillOnce(Return(AppExecFwk::AppMgrResultCode::RESULT_OK));
    EXPECT_EQ(service_->SignRestartAppFlag(RecoveryParam()), ERR_OK);
}

/**
 * @tc.name: RecoveryReturnsExitFailure
 * @tc.desc: Failure to exit the old application is returned by the restart flag operation.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceRecoveryRestartTest, RecoveryReturnsExitFailure, TestSize.Level1)
{
    EXPECT_CALL(*client_, KillApplicationByUid(BUNDLE_NAME, APP_UID, "RestartApp"))
        .WillOnce(Return(AppExecFwk::AppMgrResultCode::ERROR_KILL_APPLICATION));
    EXPECT_EQ(service_->SignRestartAppFlag(RecoveryParam()), INNER_ERR);
}

/**
 * @tc.name: RecoveryPreservesInstanceBoundary
 * @tc.desc: Multi-instance recovery exits only the selected instance instead of every process with the UID.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceRecoveryRestartTest, RecoveryPreservesInstanceBoundary, TestSize.Level1)
{
    auto param = RecoveryParam();
    param.type = AppExecFwk::MultiAppModeType::MULTI_INSTANCE;
    EXPECT_CALL(*client_, KillApplicationByUid(_, _, _)).Times(0);
    EXPECT_CALL(*appMgr_, KillAppSelfWithInstanceKey(INSTANCE_KEY, false, "RestartInstance"))
        .WillOnce(Return(ERR_OK));
    EXPECT_EQ(service_->SignRestartAppFlag(param), ERR_OK);
}
/**
 * @tc.name: RecoveryEntryUsesRecordPid
 * @tc.desc: Recovery queries the record process rather than the Binder caller process.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceRecoveryRestartTest, RecoveryEntryUsesRecordPid, TestSize.Level1)
{
    auto record = CreateRecoveryRecord(IPCSkeleton::GetCallingTokenID());
    ASSERT_NE(IPCSkeleton::GetCallingPid(), APP_PID);
    EXPECT_CALL(*client_, GetRunningProcessInfoByPid(APP_PID, _))
        .WillOnce(Return(ERR_INVALID_VALUE));
    EXPECT_CALL(*client_, KillApplicationByUid(_, _, _)).Times(0);
    EXPECT_CALL(*appMgr_, SignRestartAppFlag(_, _)).Times(0);
    service_->ScheduleRecoverAbility(record->GetToken(), 0, nullptr);
}

/**
 * @tc.name: RecoveryRejectsForeignToken
 * @tc.desc: A record owned by another token cannot reach process lookup or application exit.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceRecoveryRestartTest, RecoveryRejectsForeignToken, TestSize.Level1)
{
    auto record = CreateRecoveryRecord(IPCSkeleton::GetCallingTokenID() + 1);
    EXPECT_CALL(*client_, GetRunningProcessInfoByPid(_, _)).Times(0);
    EXPECT_CALL(*client_, KillApplicationByUid(_, _, _)).Times(0);
    EXPECT_CALL(*appMgr_, SignRestartAppFlag(_, _)).Times(0);
    service_->ScheduleRecoverAbility(record->GetToken(), 0, nullptr);
}

/**
 * @tc.name: RecoveryRejectsForeignBundle
 * @tc.desc: An authenticated caller cannot recover a different application through the target Want.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceRecoveryRestartTest, RecoveryRejectsForeignBundle, TestSize.Level1)
{
    auto record = CreateRecoveryRecord(IPCSkeleton::GetCallingTokenID());
    Want want;
    want.SetElementName("com.example.other", "EntryAbility");
    EXPECT_CALL(*client_, GetRunningProcessInfoByPid(_, _)).Times(0);
    EXPECT_CALL(*client_, KillApplicationByUid(_, _, _)).Times(0);
    EXPECT_CALL(*appMgr_, SignRestartAppFlag(_, _)).Times(0);
    service_->ScheduleRecoverAbility(record->GetToken(), 0, &want);
}
} // namespace OHOS::AAFwk
