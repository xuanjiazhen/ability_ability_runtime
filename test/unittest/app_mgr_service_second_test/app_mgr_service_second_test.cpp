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
#include "app_mgr_service.h"
#undef private
#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "mock_permission_verification.h"
#include "mock_my_flag.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AppExecFwk {
namespace {
    static int32_t uid = 1000;
}
class AppMgrServiceSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppMgrServiceSecondTest::SetUpTestCase(void)
{}

void AppMgrServiceSecondTest::TearDownTestCase(void)
{}

void AppMgrServiceSecondTest::SetUp()
{}

void AppMgrServiceSecondTest::TearDown()
{}

/*
 * Feature: AppMgrService
 * Function: JudgeSandboxByPid
 * SubFunction: NA
 * FunctionPoints: AppMgrService JudgeSandboxByPid
 * EnvConditions: NA
 * CaseDescription: Verify JudgeSandboxByPid
 */
HWTEST_F(AppMgrServiceSecondTest, JudgeSandboxByPid_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_JudgeSandboxByPid_0100 start");
    int pid = 1;
    bool isSandbox = false;
    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->appMgrServiceInner_ = nullptr;
    auto res = appMgrService->JudgeSandboxByPid(pid, isSandbox);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner_, nullptr);
    appMgrService->SetInnerService(appMgrServiceInner_);
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_ =
        AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    res = appMgrService->JudgeSandboxByPid(pid, isSandbox);
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_JudgeSandboxByPid_0100 end");
}

/*
 * Feature: AppMgrService
 * Function: StartNativeProcessForDebugger
 * SubFunction: NA
 * FunctionPoints: AppMgrService StartNativeProcessForDebugger
 * EnvConditions: NA
 * CaseDescription: Verify StartNativeProcessForDebugger
 */
HWTEST_F(AppMgrServiceSecondTest, StartNativeProcessForDebugger_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_StartNativeProcessForDebugger_0100 start");
    Want want;
    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->appMgrServiceInner_ = nullptr;
    auto res = appMgrService->StartNativeProcessForDebugger(want);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner_, nullptr);
    appMgrService->SetInnerService(appMgrServiceInner_);
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_ =
        AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    res = appMgrService->StartNativeProcessForDebugger(want);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_StartNativeProcessForDebugger_0100 end");
}

/*
 * Feature: AppMgrService
 * Function: IsSharedBundleRunning
 * SubFunction: NA
 * FunctionPoints: AppMgrService IsSharedBundleRunning
 * EnvConditions: NA
 * CaseDescription: Verify IsSharedBundleRunning
 */
HWTEST_F(AppMgrServiceSecondTest, IsSharedBundleRunning_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_IsSharedBundleRunning_0100 start");
    std::string bundleName = "bundleName";
    uint32_t versionCode = 1;
    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->appMgrServiceInner_ = nullptr;
    auto res = appMgrService->IsSharedBundleRunning(bundleName, versionCode);
    EXPECT_EQ(res, true);
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner_, nullptr);
    appMgrService->SetInnerService(appMgrServiceInner_);
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_ =
        AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    res = appMgrService->IsSharedBundleRunning(bundleName, versionCode);
    EXPECT_EQ(res, false);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_IsSharedBundleRunning_0100 end");
}

/*
 * Feature: AppMgrService
 * Function: SetAppFreezeFilter
 * SubFunction: NA
 * FunctionPoints: AppMgrService SetAppFreezeFilter
 * EnvConditions: NA
 * CaseDescription: Verify SetAppFreezeFilter
 */
HWTEST_F(AppMgrServiceSecondTest, SetAppFreezeFilter_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_SetAppFreezeFilter_0100 start");
    int32_t pid = 1;
    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->appMgrServiceInner_ = nullptr;
    auto res = appMgrService->SetAppFreezeFilter(pid);
    EXPECT_EQ(res, true);
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner_, nullptr);
    appMgrService->SetInnerService(appMgrServiceInner_);
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_ =
        AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    res = appMgrService->SetAppFreezeFilter(pid);
    EXPECT_EQ(res, false);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_SetAppFreezeFilter_0100 end");
}

/*
 * Feature: AppMgrService
 * Function: NotifyPageShow
 * SubFunction: NA
 * FunctionPoints: AppMgrService NotifyPageShow
 * EnvConditions: NA
 * CaseDescription: Verify NotifyPageShow
 */
HWTEST_F(AppMgrServiceSecondTest, NotifyPageShow_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_NotifyPageShow_0100 start");
    sptr<IRemoteObject> token = nullptr;
    PageStateData pageStateData;
    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->appMgrServiceInner_ = nullptr;
    auto res = appMgrService->NotifyPageShow(token, pageStateData);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner_, nullptr);
    appMgrService->SetInnerService(appMgrServiceInner_);
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_ =
        AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    auto result = appMgrService->NotifyPageShow(token, pageStateData);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_NotifyPageShow_0100 end");
}

/*
 * Feature: AppMgrService
 * Function: NotifyPageHide
 * SubFunction: NA
 * FunctionPoints: AppMgrService NotifyPageHide
 * EnvConditions: NA
 * CaseDescription: Verify NotifyPageHide
 */
HWTEST_F(AppMgrServiceSecondTest, NotifyPageHide_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_NotifyPageHide_0100 start");
    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    PageStateData pageStateData;
    appMgrService->appMgrServiceInner_ = nullptr;
    auto res = appMgrService->NotifyPageHide(nullptr, pageStateData);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner_, nullptr);
    appMgrService->SetInnerService(appMgrServiceInner_);
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_ =
        AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    res = appMgrService->NotifyPageHide(nullptr, pageStateData);
    EXPECT_EQ(res, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_NotifyPageHide_0100 end");
}

/*
 * Feature: AppMgrService
 * Function: RegisterAppRunningStatusListener
 * SubFunction: NA
 * FunctionPoints: AppMgrService RegisterAppRunningStatusListener
 * EnvConditions: NA
 * CaseDescription: Verify RegisterAppRunningStatusListener
 */
HWTEST_F(AppMgrServiceSecondTest, RegisterAppRunningStatusListener_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_RegisterAppRunningStatusListener_0100 start");
    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->appMgrServiceInner_ = nullptr;
    auto res = appMgrService->RegisterAppRunningStatusListener(nullptr);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner_, nullptr);
    appMgrService->SetInnerService(appMgrServiceInner_);
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_ =
        AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    res = appMgrService->RegisterAppRunningStatusListener(nullptr);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_RegisterAppRunningStatusListener_0100 end");
}

/*
 * Feature: AppMgrService
 * Function: RegisterAppForegroundStateObserver
 * SubFunction: NA
 * FunctionPoints: AppMgrService RegisterAppForegroundStateObserver
 * EnvConditions: NA
 * CaseDescription: Verify RegisterAppForegroundStateObserver
 */
HWTEST_F(AppMgrServiceSecondTest, RegisterAppForegroundStateObserver_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_RegisterAppForegroundStateObserver_0100 start");
    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->appMgrServiceInner_ = nullptr;
    auto res = appMgrService->RegisterAppForegroundStateObserver(nullptr);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner_, nullptr);
    appMgrService->SetInnerService(appMgrServiceInner_);
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_ =
        AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    res = appMgrService->RegisterAppForegroundStateObserver(nullptr);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_RegisterAppForegroundStateObserver_0100 end");
}

/*
 * Feature: AppMgrService
 * Function: IsFinalAppProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService IsFinalAppProcess
 * EnvConditions: NA
 * CaseDescription: Verify IsFinalAppProcess
 */
HWTEST_F(AppMgrServiceSecondTest, IsFinalAppProcess_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_IsFinalAppProcess_0100 start");
    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->appMgrServiceInner_ = nullptr;
    auto res = appMgrService->IsFinalAppProcess();
    EXPECT_EQ(res, false);
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner_, nullptr);
    appMgrService->SetInnerService(appMgrServiceInner_);
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_ =
        AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    res = appMgrService->IsFinalAppProcess();
    EXPECT_EQ(res, false);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_IsFinalAppProcess_0100 end");
}

/*
 * Feature: AppMgrService
 * Function: SignRestartAppFlag
 * SubFunction: NA
 * FunctionPoints: AppMgrService SignRestartAppFlag
 * EnvConditions: NA
 * CaseDescription: Verify SignRestartAppFlag
 */
HWTEST_F(AppMgrServiceSecondTest, SignRestartAppFlag_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_SignRestartAppFlag_0100 start");
    std::string instanceKey = "app_instance_0";
    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->appMgrServiceInner_ = nullptr;
    auto res = appMgrService->SignRestartAppFlag(uid, instanceKey);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner_, nullptr);
    appMgrService->SetInnerService(appMgrServiceInner_);
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_ =
        AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    res = appMgrService->SignRestartAppFlag(uid, instanceKey);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_SignRestartAppFlag_0100 end");
}

/*
 * Feature: AppMgrService
 * Function: GetAppRunningUniqueIdByPid
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetAppRunningUniqueIdByPid
 * EnvConditions: NA
 * CaseDescription: Verify GetAppRunningUniqueIdByPid
 */
HWTEST_F(AppMgrServiceSecondTest, GetAppRunningUniqueIdByPid_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_GetAppRunningUniqueIdByPid_0100 start");
    std::shared_ptr<AppMgrService> appMgrService = std::make_shared<AppMgrService>();
    pid_t pid = 1;
    std::string appRunningUniqueId = "testId";
    int32_t ret = appMgrService->GetAppRunningUniqueIdByPid(pid, appRunningUniqueId);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_ =
        AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    ret = appMgrService->GetAppRunningUniqueIdByPid(pid, appRunningUniqueId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    MyFlag::flag_ = 0;
    ret = appMgrService->GetAppRunningUniqueIdByPid(pid, appRunningUniqueId);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_GetAppRunningUniqueIdByPid_0100 end");
}

/*
 * Feature: AppMgrService
 * Function: NotifyMemorySizeStateChanged
 * SubFunction: NA
 * FunctionPoints: AppMgrService NotifyMemorySizeStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify NotifyMemorySizeStateChanged
 */
HWTEST_F(AppMgrServiceSecondTest, NotifyMemorySizeStateChanged_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_NotifyMemorySizeStateChanged_0100 start");
    std::shared_ptr<AppMgrService> appMgrService = std::make_shared<AppMgrService>();
    bool isMemorySizeSufficient = true;
    int32_t ret = appMgrService->NotifyMemorySizeStateChanged(isMemorySizeSufficient);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_ =
        AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    ret = appMgrService->NotifyMemorySizeStateChanged(isMemorySizeSufficient);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_NotifyMemorySizeStateChanged_0100 end");
}

/*
 * Feature: AppMgrService
 * Function: SetSupportedProcessCacheSelf
 * SubFunction: NA
 * FunctionPoints: AppMgrService SetSupportedProcessCacheSelf
 * EnvConditions: NA
 * CaseDescription: Verify SetSupportedProcessCacheSelf
 */
HWTEST_F(AppMgrServiceSecondTest, SetSupportedProcessCacheSelf_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_SetSupportedProcessCacheSelf_0100 start");
    std::shared_ptr<AppMgrService> appMgrService = std::make_shared<AppMgrService>();
    bool isSupport = true;
    int32_t ret = appMgrService->SetSupportedProcessCacheSelf(isSupport);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_ =
        AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    ret = appMgrService->SetSupportedProcessCacheSelf(isSupport);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_SetSupportedProcessCacheSelf_0100 end");
}

/*
 * Feature: AppMgrService
 * Function: CheckCallingIsUserTestMode
 * SubFunction: NA
 * FunctionPoints: AppMgrService CheckCallingIsUserTestMode
 * EnvConditions: NA
 * CaseDescription: Verify CheckCallingIsUserTestMode
 */
HWTEST_F(AppMgrServiceSecondTest, CheckCallingIsUserTestMode_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_CheckCallingIsUserTestMode_0100 start");
    std::shared_ptr<AppMgrService> appMgrService = std::make_shared<AppMgrService>();
    pid_t pid = 1;
    bool isUserTest = true;
    int32_t ret = appMgrService->CheckCallingIsUserTestMode(pid, isUserTest);
    EXPECT_EQ(ret, AAFwk::CHECK_PERMISSION_FAILED);

    appMgrService->SetInnerService(nullptr);
    ret = appMgrService->CheckCallingIsUserTestMode(pid, isUserTest);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceSecondTest_CheckCallingIsUserTestMode_0100 end");
}
} // namespace AppExecFwk
} // namespace OHOS