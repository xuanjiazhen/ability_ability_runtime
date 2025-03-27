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

#define private public
#include "ams_configuration_parameter.h"
#include "app_mgr_service_inner.h"
#undef private

#include <queue>
#include "app_spawn_client.h"
#include "app_utils.h"
#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "mock_ipc_skeleton.h"
#include "mock_permission_verification.h"
#include "task_handler_wrap.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AAFwk {
namespace {
constexpr int8_t DEFAULT_USERID = 0;
constexpr int32_t FOUNDATION_UID = 5523;
const std::string START_MSG_BUNDLE_NAME = "com.test.bundle";
const std::string START_MSG_BUNDLE_NAME2 = "com.test.bundle1";
const std::string START_MSG_BUNDLE_NAME3 = "com.test.bundle2";
}  // namespace
class AppMgrServiceInnerThirdTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AppMgrServiceInnerThirdTest::SetUpTestCase() {}

void AppMgrServiceInnerThirdTest::TearDownTestCase() {}

void AppMgrServiceInnerThirdTest::SetUp() {}

void AppMgrServiceInnerThirdTest::TearDown() {}

class TaskHandlerWrapTest : public TaskHandlerWrap {
public:
    explicit TaskHandlerWrapTest(const std::string& queueName) : TaskHandlerWrap(queueName) {};

    virtual ~TaskHandlerWrapTest() {};

protected:
    std::shared_ptr<InnerTaskHandle> SubmitTaskInner(
        std::function<void()>&& task, const TaskAttribute& taskAttr) override;
    bool CancelTaskInner(const std::shared_ptr<InnerTaskHandle>& taskHandle) override
    {
        return false;
    }

    void WaitTaskInner(const std::shared_ptr<InnerTaskHandle>& taskHandle) override
    {
        return;
    }

    uint64_t GetTaskCount() override
    {
        return tasks_.size();
    }
};

std::shared_ptr<InnerTaskHandle> TaskHandlerWrapTest::SubmitTaskInner(
    std::function<void()>&& task, const TaskAttribute& taskAttr)
{
    task();
    return nullptr;
}

 /**
/**
 * @tc.name: SetStartMsgCustomSandboxFlag_001
 * @tc.type: FUNC
 * @tc.Function: SetStartMsgCustomSandboxFlag
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerThirdTest, SetStartMsgCustomSandboxFlag_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerThirdTest SetStartMsgCustomSandboxFlag_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    AppSpawnStartMsg startMsg;
    startMsg.bundleName = START_MSG_BUNDLE_NAME;
    appMgrServiceInner->SetStartMsgCustomSandboxFlag(startMsg, DEFAULT_USERID);
    EXPECT_FALSE(startMsg.isCustomSandboxFlag);

    AmsConfigurationParameter::GetInstance().supportCustomSandbox_ = true;
    appMgrServiceInner->SetStartMsgCustomSandboxFlag(startMsg, DEFAULT_USERID);
    EXPECT_FALSE(startMsg.isCustomSandboxFlag);

    AppUtils::isStartOptionsWithAnimation_ = true;
    EXPECT_EQ(AppUtils::GetInstance().IsStartOptionsWithAnimation(), true);
    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->SetStartMsgCustomSandboxFlag(startMsg, DEFAULT_USERID);
    EXPECT_FALSE(startMsg.isCustomSandboxFlag);

    appMgrServiceInner->remoteClientManager_ = std::make_shared<RemoteClientManager>();
    EXPECT_NE(appMgrServiceInner->remoteClientManager_, nullptr);
    appMgrServiceInner->SetStartMsgCustomSandboxFlag(startMsg, DEFAULT_USERID);
    EXPECT_FALSE(startMsg.isCustomSandboxFlag);

    startMsg.bundleName = START_MSG_BUNDLE_NAME2;
    appMgrServiceInner->SetStartMsgCustomSandboxFlag(startMsg, DEFAULT_USERID);
    EXPECT_TRUE(startMsg.isCustomSandboxFlag);

    AppSpawnStartMsg startMsg1;
    startMsg1.bundleName = START_MSG_BUNDLE_NAME3;
    appMgrServiceInner->SetStartMsgCustomSandboxFlag(startMsg1, DEFAULT_USERID);
    EXPECT_FALSE(startMsg1.isCustomSandboxFlag);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerThirdTest SetStartMsgCustomSandboxFlag_001 end");
}

/**
 * @tc.name: GetRenderProcessTerminationStatus_002
 * @tc.type: FUNC
 * @tc.Function: GetRenderProcessTerminationStatus
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerThirdTest, GetRenderProcessTerminationStatus_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRenderProcessTerminationStatus_002 start";

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    pid_t renderPid = -1;
    int status = 0;

    appMgrServiceInner->appRunningManager_ = nullptr;
    int result = appMgrServiceInner->GetRenderProcessTerminationStatus(renderPid, status);
    EXPECT_NE(result, 0);

    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    EXPECT_NE(appMgrServiceInner->appRunningManager_, nullptr);
    result = appMgrServiceInner->GetRenderProcessTerminationStatus(renderPid, status);
    EXPECT_NE(result, 0);

    pid_t pid = IPCSkeleton::GetCallingPid();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    appRunningRecord->priorityObject_->SetPid(pid);
    EXPECT_NE(appRunningRecord, nullptr);
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.emplace(0, appRunningRecord);
    result = appMgrServiceInner->GetRenderProcessTerminationStatus(renderPid, status);
    EXPECT_NE(result, 0);

    appRunningRecord->renderPidSet_.emplace(renderPid);
    appMgrServiceInner->remoteClientManager_ = nullptr;
    result = appMgrServiceInner->GetRenderProcessTerminationStatus(renderPid, status);
    EXPECT_NE(result, 0);

    appMgrServiceInner->remoteClientManager_ = std::make_shared<RemoteClientManager>();
    EXPECT_NE(appMgrServiceInner->remoteClientManager_, nullptr);
    appMgrServiceInner->remoteClientManager_->nwebSpawnClient_ = nullptr;
    result = appMgrServiceInner->GetRenderProcessTerminationStatus(renderPid, status);
    EXPECT_NE(result, 0);

    appMgrServiceInner->remoteClientManager_->nwebSpawnClient_ = std::make_shared<AppSpawnClient>();
    EXPECT_NE(appMgrServiceInner->remoteClientManager_->nwebSpawnClient_, nullptr);
    result = appMgrServiceInner->GetRenderProcessTerminationStatus(renderPid, status);
    EXPECT_NE(result, 0);

    appRunningRecord->renderPidSet_.clear();
    renderPid = 0;
    appRunningRecord->renderPidSet_.emplace(renderPid);
    result = appMgrServiceInner->GetRenderProcessTerminationStatus(renderPid, status);
    EXPECT_EQ(result, 0);

    GTEST_LOG_(INFO) << "GetRenderProcessTerminationStatus_002 end";
}

/**
 * @tc.name: GetExceptionTimerId_002
 * @tc.type: FUNC
 * @tc.Function: GetExceptionTimerId
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerThirdTest, GetExceptionTimerId_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetExceptionTimerId_002 start";

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
#ifdef APP_MGR_SERVICE_HICOLLIE_ENABLE
#else
    FaultData faultData;
    int32_t pid = 0;
    int32_t callerUid = 0;
    int result = appMgrServiceInner->GetExceptionTimerId(faultData, "", nullptr, pid, callerUid);
    EXPECT_NE(result, 0);
#endif

    GTEST_LOG_(INFO) << "GetExceptionTimerId_002 end";
}

/**
 * @tc.name: KillFaultApp_003
 * @tc.type: FUNC
 * @tc.Function: KillFaultApp
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerThirdTest, KillFaultApp_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KillFaultApp_003 start";

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->taskHandler_ = nullptr;
    int pid = 0;
    std::string bundleName = "KillFaultAppTest";
    FaultData faultData;
    bool isNeedExit = true;
    int32_t result = appMgrServiceInner->KillFaultApp(pid, bundleName, faultData, isNeedExit);
    EXPECT_NE(result, 0);

    auto taskHandlerWrapTest = std::make_shared<TaskHandlerWrapTest>("");
    EXPECT_NE(taskHandlerWrapTest, nullptr);
    appMgrServiceInner->taskHandler_ = taskHandlerWrapTest;
    EXPECT_NE(appMgrServiceInner->taskHandler_, nullptr);
    bool rebultBranch = ProcessUtil::ProcessExist(pid);
    EXPECT_FALSE(rebultBranch);
    result = appMgrServiceInner->KillFaultApp(pid, bundleName, faultData, isNeedExit);
    EXPECT_EQ(result, 0);

    isNeedExit = false;
    faultData.forceExit = true;
    faultData.waitSaveState = false;
    result = appMgrServiceInner->KillFaultApp(pid, bundleName, faultData, isNeedExit);
    EXPECT_EQ(result, 0);

    GTEST_LOG_(INFO) << "KillFaultApp_003 end";
}

/**
 * @tc.name: SetAppFreezeFilter_004
 * @tc.type: FUNC
 * @tc.Function: SetAppFreezeFilter
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerThirdTest, SetAppFreezeFilter_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetAppFreezeFilter_004 start";

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->appRunningManager_ = nullptr;

    int32_t pid = 0;
    IPCSkeleton::SetCallingPid(1);
    bool result = appMgrServiceInner->SetAppFreezeFilter(pid);
    EXPECT_FALSE(result);

    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    EXPECT_NE(appMgrServiceInner->appRunningManager_, nullptr);
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.clear();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    EXPECT_NE(appMgrServiceInner->appRunningManager_, nullptr);
    auto appRunningRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    appRunningRecord->priorityObject_->SetPid(pid);
    appRunningRecord->mainBundleName_ = "";
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.emplace(0, appRunningRecord);
    result = appMgrServiceInner->SetAppFreezeFilter(pid);
    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "SetAppFreezeFilter_004 end";
}

/**
 * @tc.name: IsSharedBundleRunning_005
 * @tc.type: FUNC
 * @tc.Function: IsSharedBundleRunning
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerThirdTest, IsSharedBundleRunning_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsSharedBundleRunning_005 start";

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "IsSharedBundleRunningTest";
    uint32_t versionCode = 1;
    appMgrServiceInner->appRunningManager_ = nullptr;
    bool result = appMgrServiceInner->IsSharedBundleRunning(bundleName, versionCode);
    EXPECT_FALSE(result);

    MyFlag::flag_ = 1;
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    EXPECT_NE(appMgrServiceInner->appRunningManager_, nullptr);
    appMgrServiceInner->runningSharedBundleList_.clear();
    result = appMgrServiceInner->IsSharedBundleRunning(bundleName, versionCode);
    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "IsSharedBundleRunning_005 end";
}

/**
 * @tc.name: IsAppRunningByBundleNameAndUserId_006
 * @tc.type: FUNC
 * @tc.Function: IsAppRunningByBundleNameAndUserId
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerThirdTest, IsAppRunningByBundleNameAndUserId_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAppRunningByBundleNameAndUserId_006 start";

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    IPCSkeleton::SetCallingUid(1);
    int32_t userId = 0;
    bool isRunning = false;
    int32_t result = appMgrServiceInner->IsAppRunningByBundleNameAndUserId("", userId, isRunning);
    EXPECT_NE(result, 0);

    IPCSkeleton::SetCallingUid(FOUNDATION_UID);
    appMgrServiceInner->appRunningManager_ = nullptr;
    EXPECT_EQ(IPCSkeleton::GetCallingUid(), FOUNDATION_UID);
    result = appMgrServiceInner->IsAppRunningByBundleNameAndUserId("", userId, isRunning);
    EXPECT_NE(result, 0);

    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    EXPECT_NE(appMgrServiceInner->appRunningManager_, nullptr);
    result = appMgrServiceInner->IsAppRunningByBundleNameAndUserId("", userId, isRunning);
    EXPECT_EQ(result, 0);

    GTEST_LOG_(INFO) << "IsAppRunningByBundleNameAndUserId_006 end";
}
} // namespace AAFwk
} // namespace OHOS
