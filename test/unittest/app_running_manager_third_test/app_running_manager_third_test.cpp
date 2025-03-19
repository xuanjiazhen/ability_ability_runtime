/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <gtest/hwext/gtest-multithread.h>

#define private public
#include "app_running_manager.h"
#ifdef SUPPORT_CHILD_PROCESS
#include "child_process_record.h"
#endif // SUPPORT_CHILD_PROCESS
#undef private
#include "hilog_tag_wrapper.h"
#include "os_account_manager_wrapper.h"
#include "window_visibility_info.h"

using namespace testing;
using namespace testing::ext;
using namespace testing::mt;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t RECORD_ID = 1;
constexpr int32_t BASE_USER_RANGE = 200000;
constexpr pid_t PID = 10;
}
class AppRunningManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppRunningManagerTest::SetUpTestCase(void)
{}

void AppRunningManagerTest::TearDownTestCase(void)
{}

void AppRunningManagerTest::SetUp()
{}

void AppRunningManagerTest::TearDown()
{}

/**
 * @tc.name: IsAppRunningByBundleNameAndUserId_0100
 * @tc.desc: MultiProcess application cache check test.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, IsAppRunningByBundleNameAndUserId_0100, TestSize.Level1)
{
    static std::shared_ptr<AppRunningManager> appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    std::string bundleName = "bundleName";
    int32_t userId = 5;
    bool isRunning = false;
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    info->bundleName = bundleName;
    info->uid = 1010101;
    int32_t recordId = RECORD_ID;
    const std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(info, recordId, processName);
    appRunningRecord->SetUid(info->uid);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRunningRecord));
    int32_t res = appRunningManager->IsAppRunningByBundleNameAndUserId(bundleName, userId, isRunning);
    for (const auto &item : appRunningManager->GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        EXPECT_TRUE(isRunning);
    }
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: GetProcessInfosByUserId_0100
 * @tc.desc: MultiProcess application cache check test.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, GetProcessInfosByUserId_0100, TestSize.Level1)
{
    static std::shared_ptr<AppRunningManager> appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    std::list<SimpleProcessInfo> processInfos;
    int32_t userId = 5;
    std::list<pid_t> pids;

    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    info->uid = 1010101;
    int32_t recordId = RECORD_ID;
    const std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(info, recordId, processName);
    appRunningRecord->SetUid(info->uid);
    appRunningRecord->GetPriorityObject()->SetPid(PID);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRunningRecord));
    for (const auto &item : appRunningManager->appRunningRecordMap_) {
        const auto &appRecord = item.second;
        int32_t id = -1;
        int ret = DelayedSingleton<OsAccountManagerWrapper>::GetInstance()->
            GetOsAccountLocalIdFromUid(appRecord->GetUid(), id);
        EXPECT_EQ(ret, 0);
        EXPECT_EQ(id, userId);
        pid_t pid = appRecord->GetPid();
        EXPECT_TRUE((pid > 0));
    }
    int32_t res = appRunningManager->GetProcessInfosByUserId(userId, processInfos);
    EXPECT_FALSE(processInfos.empty());
}
} // namespace AppExecFwk
} // namespace OHOS
