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

#include "app_running_manager.h"
#include "app_running_record.h"
#include "child_process_record.h"
#include "app_record_id.h"
#include "exit_resident_process_manager.h"
#include "hilog_tag_wrapper.h"
#include "ability_record.h"
#include "app_mgr_service_dump_error_code.h"
#include "mock_app_scheduler.h"
#include "ability_scheduler_mock.h"
#include "child_process_request.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t USR_ID_100 = 100;
constexpr int32_t USR_ID_101 = 101;
const std::string BUNDLE_NAME = "testBundleName";
const std::string PROCESS_NAME = "testProcessName";
constexpr pid_t PID = 0;
}

class AppRunningManagerThirdTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<Token> MockToken();

protected:
    static BundleInfo bundleInfo;
    static std::shared_ptr<ApplicationInfo> appInfo_;
};

BundleInfo AppRunningManagerThirdTest::bundleInfo;
std::shared_ptr<ApplicationInfo> AppRunningManagerThirdTest::appInfo_ = nullptr;

void AppRunningManagerThirdTest::SetUpTestCase(void)
{
    appInfo_ = std::make_shared<ApplicationInfo>();
    appInfo_->bundleName = BUNDLE_NAME;
}

void AppRunningManagerThirdTest::TearDownTestCase(void)
{}

void AppRunningManagerThirdTest::SetUp()
{}

void AppRunningManagerThirdTest::TearDown()
{}

sptr<Token> AppRunningManagerThirdTest::MockToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (!abilityRecord) {
        return nullptr;
    }

    return abilityRecord->GetToken();
}

class MockIChildScheduler : public IChildScheduler {
public:
    MockIChildScheduler() = default;
    ~MockIChildScheduler() = default;
    bool ScheduleLoadChild() override
    {
        return true;
    }

    bool ScheduleExitProcessSafely() override
    {
        return true;
    }

    bool ScheduleRunNativeProc(const sptr<IRemoteObject> &mainProcessCb) override
    {
        return true;
    }

    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};

/**
 * @tc.name: AppRunningManager_OnChildProcessRemoteDied_0100
 * @tc.desc: Test OnChildProcessRemoteDied
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_OnChildProcessRemoteDied_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_OnChildProcessRemoteDied_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    wptr<IRemoteObject> remote = nullptr;
    EXPECT_EQ(appRunningManager->OnChildProcessRemoteDied(remote), nullptr);

    sptr<IRemoteObject> remoteObject = MockToken();
    wptr<IRemoteObject> remote1(remoteObject);
    ChildProcessRequest request;
    request.srcEntry = "./ets/AProcess.ts";
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo_, USR_ID_100, PROCESS_NAME);
    EXPECT_NE(appRunningRecord, nullptr);
    auto childRecord = ChildProcessRecord::CreateChildProcessRecord(USR_ID_100, request, appRunningRecord);
    EXPECT_NE(childRecord, nullptr);
    childRecord->scheduler_ = new (std::nothrow) MockIChildScheduler();
    EXPECT_NE(childRecord->scheduler_, nullptr);
    appRunningRecord->childProcessRecordMap_.emplace(PID, childRecord);
    appRunningManager->appRunningRecordMap_.emplace(PID, appRunningRecord);
    EXPECT_EQ(appRunningManager->OnChildProcessRemoteDied(remote1), nullptr);

    appRunningManager->appRunningRecordMap_.clear();
    appRunningRecord = nullptr;
    appRunningManager->appRunningRecordMap_.emplace(PID, appRunningRecord);
    EXPECT_EQ(appRunningManager->OnChildProcessRemoteDied(remote1), nullptr);

    appRunningRecord = std::make_shared<AppRunningRecord>(appInfo_, USR_ID_100, PROCESS_NAME);
    appRunningManager->appRunningRecordMap_.clear();
    appRunningManager->appRunningRecordMap_.emplace(PID, appRunningRecord);
    appRunningRecord->childProcessRecordMap_.clear();
    EXPECT_EQ(appRunningManager->OnChildProcessRemoteDied(remote1), nullptr);

    childRecord = nullptr;
    appRunningRecord->childProcessRecordMap_.emplace(PID, childRecord);
    EXPECT_EQ(appRunningManager->OnChildProcessRemoteDied(remote1), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_OnChildProcessRemoteDied_0100 end");
}

/**
 * @tc.name: AppRunningManager_GetAppRunningUniqueIdByPid_0100
 * @tc.desc: Test GetAppRunningUniqueIdByPid
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_GetAppRunningUniqueIdByPid_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_GetAppRunningUniqueIdByPid_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo_, USR_ID_100, PROCESS_NAME);
    appRunningManager->appRunningRecordMap_.emplace(PID, appRunningRecord);
    std::string appRunningUniqueId = "test";
    EXPECT_EQ(appRunningManager->GetAppRunningUniqueIdByPid(PID, appRunningUniqueId), ERR_OK);
    appRunningManager->appRunningRecordMap_.clear();
    EXPECT_EQ(appRunningManager->GetAppRunningUniqueIdByPid(PID, appRunningUniqueId), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_GetAppRunningUniqueIdByPid_0100 end");
}

/**
 * @tc.name: AppRunningManager_DumpIpcAllStart_0100
 * @tc.desc: Test DumpIpcAllStart
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_DumpIpcAllStart_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcAllStart_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo_, USR_ID_100, PROCESS_NAME);
    appRunningManager->appRunningRecordMap_.emplace(PID, appRunningRecord);
    appRunningRecord->appLifeCycleDeal_ = std::make_shared<AppLifeCycleDeal>();
    appRunningRecord->appLifeCycleDeal_->appThread_ = new (std::nothrow) MockAppScheduler();
    std::string result;
    EXPECT_EQ(appRunningManager->DumpIpcAllStart(result), ERR_OK);
    appRunningRecord->appLifeCycleDeal_->appThread_ = nullptr;
    EXPECT_EQ(appRunningManager->DumpIpcAllStart(result), DumpErrorCode::ERR_INTERNAL_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcAllStart_0100 end");
}

/**
 * @tc.name: AppRunningManager_DumpIpcAllStop_0100
 * @tc.desc: Test DumpIpcAllStop
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_DumpIpcAllStop_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcAllStop_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo_, USR_ID_100, PROCESS_NAME);
    appRunningManager->appRunningRecordMap_.emplace(PID, appRunningRecord);
    appRunningRecord->appLifeCycleDeal_ = std::make_shared<AppLifeCycleDeal>();
    appRunningRecord->appLifeCycleDeal_->appThread_ = new (std::nothrow) MockAppScheduler();
    std::string result;
    EXPECT_EQ(appRunningManager->DumpIpcAllStop(result), ERR_OK);
    appRunningRecord->appLifeCycleDeal_->appThread_ = nullptr;
    EXPECT_EQ(appRunningManager->DumpIpcAllStop(result), DumpErrorCode::ERR_INTERNAL_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcAllStop_0100 end");
}

/**
 * @tc.name: AppRunningManager_DumpIpcAllStat_0100
 * @tc.desc: Test DumpIpcAllStat
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_DumpIpcAllStat_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcAllStat_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo_, USR_ID_100, PROCESS_NAME);
    appRunningManager->appRunningRecordMap_.emplace(PID, appRunningRecord);
    appRunningRecord->appLifeCycleDeal_ = std::make_shared<AppLifeCycleDeal>();
    appRunningRecord->appLifeCycleDeal_->appThread_ = new (std::nothrow) MockAppScheduler();
    std::string result;
    EXPECT_EQ(appRunningManager->DumpIpcAllStat(result), ERR_OK);
    appRunningRecord->appLifeCycleDeal_->appThread_ = nullptr;
    EXPECT_EQ(appRunningManager->DumpIpcAllStat(result), DumpErrorCode::ERR_INTERNAL_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcAllStat_0100 end");
}

/**
 * @tc.name: AppRunningManager_DumpIpcStart_0100
 * @tc.desc: Test DumpIpcStart
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_DumpIpcStart_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcStart_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo_, USR_ID_100, PROCESS_NAME);
    appRunningManager->appRunningRecordMap_.emplace(PID, appRunningRecord);
    appRunningRecord->appLifeCycleDeal_ = std::make_shared<AppLifeCycleDeal>();
    appRunningRecord->appLifeCycleDeal_->appThread_ = new (std::nothrow) MockAppScheduler();
    std::string result;
    EXPECT_EQ(appRunningManager->DumpIpcStart(PID, result), ERR_OK);
    appRunningRecord->appLifeCycleDeal_->appThread_ = nullptr;
    EXPECT_EQ(appRunningManager->DumpIpcStart(PID,result), DumpErrorCode::ERR_INTERNAL_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcStart_0100 end");
}
} // namespace AppExecFwk
} // namespace OHOS
