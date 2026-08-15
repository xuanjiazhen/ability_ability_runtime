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
#include <memory>
#include <string>

#define private public
#define protected public
#include "forkall_helper.h"
#undef private
#undef protected

#include "app_launch_data.h"
#include "application_info.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {

class ForkAllHelperTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.number: ForkAllHelperTest_HandleDebugAppLaunchDelay_0100
 * @tc.name: Non-developer mode early return
 * @tc.desc: Verify HandleDebugAppLaunchDelay returns immediately when not in developer mode.
 */
HWTEST_F(ForkAllHelperTest, ForkAllHelperTest_HandleDebugAppLaunchDelay_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForkAllHelperTest_HandleDebugAppLaunchDelay_0100 start";
    AppLaunchData appLaunchData;
    bool isDeveloperMode = false;
    EXPECT_NO_FATAL_FAILURE(ForkAllHelper::HandleDebugAppLaunchDelay(appLaunchData, isDeveloperMode));
    GTEST_LOG_(INFO) << "ForkAllHelperTest_HandleDebugAppLaunchDelay_0100 end";
}

/**
 * @tc.number: ForkAllHelperTest_HandleDebugAppLaunchDelay_0200
 * @tc.name: Non-debug provision type early return
 * @tc.desc: Verify HandleDebugAppLaunchDelay returns when app is not debug provision type.
 */
HWTEST_F(ForkAllHelperTest, ForkAllHelperTest_HandleDebugAppLaunchDelay_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForkAllHelperTest_HandleDebugAppLaunchDelay_0200 start";
    AppLaunchData appLaunchData;
    ApplicationInfo appInfo;
    appInfo.appProvisionType = "release";
    appLaunchData.SetApplicationInfo(appInfo);
    bool isDeveloperMode = true;
    EXPECT_NO_FATAL_FAILURE(ForkAllHelper::HandleDebugAppLaunchDelay(appLaunchData, isDeveloperMode));
    GTEST_LOG_(INFO) << "ForkAllHelperTest_HandleDebugAppLaunchDelay_0200 end";
}

/**
 * @tc.number: ForkAllHelperTest_HandleDebugAppLaunchDelay_0300
 * @tc.name: Non-template image process type early return
 * @tc.desc: Verify HandleDebugAppLaunchDelay returns when image process type is not TEMPLATE.
 */
HWTEST_F(ForkAllHelperTest, ForkAllHelperTest_HandleDebugAppLaunchDelay_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForkAllHelperTest_HandleDebugAppLaunchDelay_0300 start";
    AppLaunchData appLaunchData;
    ApplicationInfo appInfo;
    appInfo.appProvisionType = "debug";
    appLaunchData.SetApplicationInfo(appInfo);
    appLaunchData.SetImageProcessType(static_cast<int32_t>(ImageProcessType::WORK));
    bool isDeveloperMode = true;
    EXPECT_NO_FATAL_FAILURE(ForkAllHelper::HandleDebugAppLaunchDelay(appLaunchData, isDeveloperMode));
    GTEST_LOG_(INFO) << "ForkAllHelperTest_HandleDebugAppLaunchDelay_0300 end";
}

/**
 * @tc.number: ForkAllHelperTest_HandleDebugAppLaunchDelay_0400
 * @tc.name: Template mode with zero delay
 * @tc.desc: Verify HandleDebugAppLaunchDelay with template mode and zero delay parameter does not hang.
 */
HWTEST_F(ForkAllHelperTest, ForkAllHelperTest_HandleDebugAppLaunchDelay_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForkAllHelperTest_HandleDebugAppLaunchDelay_0400 start";
    AppLaunchData appLaunchData;
    ApplicationInfo appInfo;
    appInfo.appProvisionType = "debug";
    appLaunchData.SetApplicationInfo(appInfo);
    appLaunchData.SetImageProcessType(static_cast<int32_t>(ImageProcessType::TEMPLATE));
    bool isDeveloperMode = true;
    EXPECT_NO_FATAL_FAILURE(ForkAllHelper::HandleDebugAppLaunchDelay(appLaunchData, isDeveloperMode));
    GTEST_LOG_(INFO) << "ForkAllHelperTest_HandleDebugAppLaunchDelay_0400 end";
}
} // namespace AppExecFwk
} // namespace OHOS
