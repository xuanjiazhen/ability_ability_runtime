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
#include "app_running_manager.h"
#include "child_process_record.h"
#undef private

#include "app_record_id.h"
#include "exit_resident_process_manager.h"
#include "hilog_tag_wrapper.h"
#include "mock_app_mgr_service_inner.h"
#include "mock_application.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t USR_ID_100 = 100;
constexpr int32_t USR_ID_101 = 101;
}

class AppRunningManagerSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppRunningManagerSecondTest::SetUpTestCase(void)
{}

void AppRunningManagerSecondTest::TearDownTestCase(void)
{}

void AppRunningManagerSecondTest::SetUp()
{}

void AppRunningManagerSecondTest::TearDown()
{}

/**
 * @tc.name: AppRunningManager_CheckAppRunningRecordIsExistByUid_0100
 * @tc.desc: Test CheckAppRunningRecordIsExistByUid
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerSecondTest, AppRunningManager_CheckAppRunningRecordIsExistByUid_0100, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize AppRunningManager&AppRunningRecord instance
     * @tc.expected: expect step1 succeed
     */
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    BundleInfo bundleInfo;
    const std::string processName = "testProcessName";
    std::shared_ptr<AppRunningRecord> record =
        appRunningManager->CreateAppRunningRecord(appInfo, processName, bundleInfo);

    /**
     * @tc.steps: step2. SetUid USR_ID_100, SetRestartAppFlag false
     * @tc.expected: step2. expect CheckAppRunningRecordIsExistByUid true
     */
    record->SetUid(USR_ID_100);
    record->SetRestartAppFlag(false);
    bool ret = appRunningManager->CheckAppRunningRecordIsExistByUid(USR_ID_100);
    EXPECT_TRUE(ret);
 
    /**
     * @tc.steps: step3. SetRestartAppFlag true
     * @tc.expected: step3. expect CheckAppRunningRecordIsExistByUid false
     */
    record->SetRestartAppFlag(true);
    ret = appRunningManager->CheckAppRunningRecordIsExistByUid(USR_ID_100);
    EXPECT_FALSE(ret);
 
    /**
     * @tc.steps: step4. call CheckAppRunningRecordIsExistByUid USR_ID_101
     * @tc.expected: step4. expect call CheckAppRunningRecordIsExistByUid false
     */
    ret = appRunningManager->CheckAppRunningRecordIsExistByUid(USR_ID_101);
    EXPECT_FALSE(ret);

    /**
     * @tc.steps: step5. appRunningRecord is nullptr
     * @tc.expected: step5. expect CheckAppRunningRecordIsExistByUid false
     */
    appRunningManager->appRunningRecordMap_.clear();
    auto recordId = AppRecordId::Create();
    appRunningManager->appRunningRecordMap_.emplace(recordId, nullptr);
    ret = appRunningManager->CheckAppRunningRecordIsExistByUid(USR_ID_101);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: AppRunningManager_CheckAppCloneRunningRecordIsExistByBundleName_0100
 * @tc.desc: Test CheckAppCloneRunningRecordIsExistByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerSecondTest, AppRunningManager_CheckAppCloneRunningRecordIsExistByBundleName_0100,
    TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize AppRunningManager instance
     * @tc.expected: expect step1 succeed
     */
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    const std::string bundleName = "testBundleName";
    appInfo->bundleName = bundleName;
    const std::string processName = "testProcessName";
    BundleInfo bundleInfo;
    std::shared_ptr<AppRunningRecord> record =
        appRunningManager->CreateAppRunningRecord(appInfo, processName, bundleInfo);
    ASSERT_NE(record, nullptr);

    /**
     * @tc.steps: step2. SetRestartAppFlag false, SetAppIndex 0
     * @tc.expected: step2. expect isRunning true
     */
    bool isRunning = false;
    record->SetRestartAppFlag(false);
    record->SetAppIndex(0);
    appRunningManager->CheckAppCloneRunningRecordIsExistByBundleName(bundleName, 0, isRunning);
    EXPECT_TRUE(isRunning);

    /**
     * @tc.steps: step3. call CheckAppCloneRunningRecordIsExistByBundleName appIndex 1
     * @tc.expected: step3. expect isRunning false
     */
    isRunning = false;
    appRunningManager->CheckAppCloneRunningRecordIsExistByBundleName(bundleName, 1, isRunning); // 1 means appIndex
    EXPECT_FALSE(isRunning);

    /**
     * @tc.steps: step4. SetRestartAppFlag false, SetRestartAppFlag true.
     * @tc.expected: step4. expect isRunning false
     */
    record->SetRestartAppFlag(true);
    appRunningManager->CheckAppCloneRunningRecordIsExistByBundleName(bundleName, 0, isRunning);
    EXPECT_FALSE(isRunning);

    /**
     * @tc.steps: step5. SetRestartAppFlag false
     * @tc.expected: step5. expect isRunning false by empty BundleName
     */
    record->SetRestartAppFlag(false);
    appRunningManager->CheckAppCloneRunningRecordIsExistByBundleName("", 0, isRunning);
    EXPECT_FALSE(isRunning);

    /**
     * @tc.steps: step6. SetRestartAppFlag false, SetRestartAppFlag true.
     * @tc.expected: step6. expect isRunning false
     */
    appRunningManager->appRunningRecordMap_.clear();
    auto recordId = AppRecordId::Create();
    appRunningManager->appRunningRecordMap_.emplace(recordId, nullptr);
    appRunningManager->CheckAppCloneRunningRecordIsExistByBundleName("", 0, isRunning);
    EXPECT_FALSE(isRunning);

    /**
     * @tc.steps: step7. clear appRunningRecordMap_.
     * @tc.expected: step7. expect isRunning false
     */
    appRunningManager->appRunningRecordMap_.clear();
    int32_t ret = appRunningManager->CheckAppCloneRunningRecordIsExistByBundleName("", 0, isRunning);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: AppRunningManager_ProcessExitByBundleName_0100
 * @tc.desc: Test ProcessExitByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerSecondTest, AppRunningManager_ProcessExitByBundleName_0100, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize AppRunningManager instance
     * @tc.expected: expect step1 succeed
     */
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    const std::string bundleName = "testBundleName";
    appInfo->bundleName = bundleName;
    const std::string processName = "testProcessName";
    BundleInfo bundleInfo;
    std::shared_ptr<AppRunningRecord> record1 =
        appRunningManager->CreateAppRunningRecord(appInfo, processName, bundleInfo);
    ASSERT_NE(record1, nullptr);
    record1->appInfos_.emplace(bundleName, appInfo);

    /**
     * @tc.steps: step2. SetKeepAliveBundle false, SetPid valid pid
     * @tc.expected: step2. expect pids not empty
     */
    record1->SetKeepAliveBundle(false); // not resident process, can kill
    record1->GetPriorityObject()->SetPid(10000); // 10000 means valid process id
    std::list<pid_t> pids;
    appRunningManager->ProcessExitByBundleName(bundleName, pids, true);
    EXPECT_EQ(pids.size(), 1); // 1 means exit pid list size

    /**
     * @tc.steps: step3. ProcessExitByBundleName by empty bundleName
     * @tc.expected: step3. expect pids empty
     */
    pids.clear();
    appRunningManager->ProcessExitByBundleName("", pids, false);
    EXPECT_TRUE(pids.empty());

    /**
     * @tc.steps: step4. SetKeepAliveBundle false, SetPid Invalid pid
     * @tc.expected: step4. expect pids empty
     */
    record1->GetPriorityObject()->SetPid(0); // 10000 means invalid process id
    appRunningManager->ProcessExitByBundleName(bundleName, pids, false);
    EXPECT_TRUE(pids.empty());
}

/**
 * @tc.name: AppRunningManager_ProcessExitByBundleName_0200
 * @tc.desc: Test ProcessExitByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerSecondTest, AppRunningManager_ProcessExitByBundleName_0200, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize AppRunningManager instance
     * @tc.expected: expect step1 succeed
     */
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    const std::string bundleName = "testBundleName";
    appInfo->bundleName = bundleName;
    const std::string processName = "testProcessName";
    BundleInfo bundleInfo;
    std::shared_ptr<AppRunningRecord> record1 =
        appRunningManager->CreateAppRunningRecord(appInfo, processName, bundleInfo);
    ASSERT_NE(record1, nullptr);
    record1->appInfos_.emplace(bundleName, appInfo);

    /**
     * @tc.steps: step2. process resident process and ExitResidentProcessManager Memory Size Insufficent
     * @tc.expected: step2. expect pids not empty
     */
    std::vector<ExitResidentProcessInfo> processInfos;
    record1->SetUid(BASE_USER_RANGE);
    record1->SetKeepAliveBundle(true);
    record1->SetKeepAliveEnableState(true);
    record1->SetMainProcess(true);
    record1->GetPriorityObject()->SetPid(10000); // 10000 means valid process id
    ExitResidentProcessManager::GetInstance().HandleMemorySizeInSufficent(); // marked mem insufficent
    std::list<pid_t> pids;
    appRunningManager->ProcessExitByBundleName(bundleName, pids, false);
    EXPECT_TRUE(pids.empty());

    /**
     * @tc.steps: step2. process resident process and ExitResidentProcessManager Memory Size Insufficent
     * @tc.expected: step2. expect pids not empty, memory size sufficent
     */
    pids.clear();
    ExitResidentProcessManager::GetInstance().HandleMemorySizeSufficient(processInfos); // marked mem sufficent
    appRunningManager->ProcessExitByBundleName(bundleName, pids, true);
    EXPECT_TRUE(pids.empty());
}

/**
 * @tc.name: AppRunningManager_GetPidsByBundleNameUserIdAndAppIndex_0100
 * @tc.desc: Test GetPidsByBundleNameUserIdAndAppIndex
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerSecondTest, AppRunningManager_GetPidsByBundleNameUserIdAndAppIndex_0100, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize AppRunningManager instance
     * @tc.expected: expect step1 succeed
     */
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    appInfo->appIndex = 0;
    const std::string bundleName = "testBundleName";
    appInfo->bundleName = bundleName;
    const std::string processName = "testProcessName";
    BundleInfo bundleInfo;
    std::shared_ptr<AppRunningRecord> record1 =
        appRunningManager->CreateAppRunningRecord(appInfo, processName, bundleInfo);
    ASSERT_NE(record1, nullptr);
    record1->appInfos_.emplace(bundleName, appInfo);

    /**
     * @tc.steps: step2. SetKeepAliveBundle false, SetPid valid pid
     * @tc.expected: step2. expect pids not empty
     */
    record1->SetUid(0);
    record1->GetPriorityObject()->SetPid(10000); // 10000 means valid process id
    std::list<pid_t> pids;
    appRunningManager->GetPidsByBundleNameUserIdAndAppIndex(bundleName, 0, 0, pids);
    EXPECT_EQ(pids.size(), 1); // 1 means exit pid list size

    /**
     * @tc.steps: step3. GetPidsByBundleNameUserIdAndAppIndex by appIndex 1
     * @tc.expected: step3. expect pids empty
     */
    pids.clear();
    appRunningManager->GetPidsByBundleNameUserIdAndAppIndex(bundleName, 0, 1, pids); // 1 means appIndex
    EXPECT_TRUE(pids.empty()); // not exist

    /**
     * @tc.steps: step4. GetPidsByBundleNameUserIdAndAppIndex by userid 1
     * @tc.expected: step4. expect pids empty
     */
    appRunningManager->GetPidsByBundleNameUserIdAndAppIndex(bundleName, 1, 0, pids); // 1 means usrid
    EXPECT_TRUE(pids.empty()); // not exist

    /**
     * @tc.steps: step5. GetPidsByBundleNameUserIdAndAppIndex by empty bundleName
     * @tc.expected: step5. expect pids empty
     */
    appRunningManager->GetPidsByBundleNameUserIdAndAppIndex("", 0, 0, pids); // 1 means usrid
    EXPECT_TRUE(pids.empty()); // not exist

    /**
     * @tc.steps: step6. GetPidsByBundleNameUserIdAndAppIndex with invalid pid
     * @tc.expected: step6. expect pids empty
     */
    record1->GetPriorityObject()->SetPid(0); // set invalid pid
    appRunningManager->GetPidsByBundleNameUserIdAndAppIndex(bundleName, 0, 0, pids); // 1 means usrid
    EXPECT_TRUE(pids.empty()); // not exist

    /**
     * @tc.steps: step7. GetPidsByBundleNameUserIdAndAppIndex clear appInfos
     * @tc.expected: step7. expect pids empty
     */
    record1->GetPriorityObject()->SetPid(10000); // set invalid pid
    record1->appInfos_.clear();
    appRunningManager->GetPidsByBundleNameUserIdAndAppIndex(bundleName, 0, 0, pids); // 1 means usrid
    EXPECT_TRUE(pids.empty()); // not exist
}

/**
 * @tc.name: AppRunningManager_OnRemoteDied_0100
 * @tc.desc: Test OnRemoteDied
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerSecondTest, AppRunningManager_OnRemoteDied_0100, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize AppRunningManager instance
     * @tc.expected: expect step1 succeed
     */
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    const std::string bundleName = "testBundleName";
    appInfo->bundleName = bundleName;
    const std::string processName = "testProcessName";
    BundleInfo bundleInfo;
    std::shared_ptr<AppRunningRecord> record =
        appRunningManager->CreateAppRunningRecord(appInfo, processName, bundleInfo);
    ASSERT_NE(record, nullptr);
    std::shared_ptr<MockAppMgrServiceInner> appServiceInner = std::make_shared<MockAppMgrServiceInner>();
    sptr<MockApplication> mockApp1 = sptr<MockApplication>::MakeSptr();
    wptr<MockApplication> wp1 = mockApp1;

    /**
     * @tc.steps: step2. OnRemoteDied
     * @tc.expected: step2. expect not matched appRunningRecord
     */
    EXPECT_FALSE(appRunningManager->appRunningRecordMap_.empty());
    sptr<MockApplication> mockApp2 = sptr<MockApplication>::MakeSptr();
    wptr<MockApplication> wp2 = mockApp2;
    std::shared_ptr<AppRunningRecord> appRecord1 = appRunningManager->OnRemoteDied(wp2, appServiceInner); // not matched
    EXPECT_EQ(appRecord1, nullptr); // not matched

    /**
     * @tc.steps: step3. OnRemoteDied
     * @tc.expected: step3. expect clear matched appRunningRecord
     */
    record->SetApplicationClient(mockApp1);
    std::shared_ptr<AppRunningRecord> appRecord2 = appRunningManager->OnRemoteDied(wp1, appServiceInner);
    EXPECT_EQ(appRecord2, record); // matched
    EXPECT_EQ(record->GetApplicationClient(), nullptr); // client set nullptr
}
} // namespace AppExecFwk
} // namespace OHOS
