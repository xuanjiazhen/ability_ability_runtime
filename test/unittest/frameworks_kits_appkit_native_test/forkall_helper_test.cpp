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

#include <chrono>
#include <gtest/gtest.h>

#include "app_mgr_constants.h"
#include "forkall_helper.h"
#include "parameters.h"

using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr const char* SNAPSHOT_LAUNCH_DELAY_MS = "persist.resourceschedule.snapshot_launch_delay_ms";
constexpr const char* DEBUG_PROVISION_TYPE = "debug";
constexpr const char* RELEASE_PROVISION_TYPE = "release";

AppLaunchData CreateDebugTemplateLaunchData()
{
    AppLaunchData appLaunchData;
    ApplicationInfo applicationInfo;
    applicationInfo.appProvisionType = DEBUG_PROVISION_TYPE;
    appLaunchData.SetApplicationInfo(applicationInfo);
    appLaunchData.SetImageProcessType(static_cast<int32_t>(ImageProcessType::TEMPLATE));
    return appLaunchData;
}
} // namespace

class ForkAllHelperTest : public testing::Test {
public:
    void TearDown() override
    {
        OHOS::system::SetParameter(SNAPSHOT_LAUNCH_DELAY_MS, "0");
    }
};

/**
 * @tc.name: HandleDebugAppLaunchDelay_NotDeveloperMode_0100
 * @tc.desc: Do not delay debug template application launch when developer mode is disabled.
 * @tc.type: FUNC
 */
HWTEST_F(ForkAllHelperTest, HandleDebugAppLaunchDelay_NotDeveloperMode_0100, TestSize.Level1)
{
    auto appLaunchData = CreateDebugTemplateLaunchData();
    OHOS::system::SetParameter(SNAPSHOT_LAUNCH_DELAY_MS, "500");

    auto start = std::chrono::steady_clock::now();
    ForkAllHelper::HandleDebugAppLaunchDelay(appLaunchData, false);
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);

    EXPECT_LT(elapsed.count(), 400);
}

/**
 * @tc.name: HandleDebugAppLaunchDelay_NonDebugProvision_0100
 * @tc.desc: Do not delay non-debug provision applications even in developer mode.
 * @tc.type: FUNC
 */
HWTEST_F(ForkAllHelperTest, HandleDebugAppLaunchDelay_NonDebugProvision_0100, TestSize.Level1)
{
    auto appLaunchData = CreateDebugTemplateLaunchData();
    ApplicationInfo applicationInfo;
    applicationInfo.appProvisionType = RELEASE_PROVISION_TYPE;
    appLaunchData.SetApplicationInfo(applicationInfo);
    OHOS::system::SetParameter(SNAPSHOT_LAUNCH_DELAY_MS, "500");

    auto start = std::chrono::steady_clock::now();
    ForkAllHelper::HandleDebugAppLaunchDelay(appLaunchData, true);
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);

    EXPECT_LT(elapsed.count(), 400);
}

/**
 * @tc.name: HandleDebugAppLaunchDelay_NonTemplateProcess_0100
 * @tc.desc: Do not delay non-template processes for debug applications.
 * @tc.type: FUNC
 */
HWTEST_F(ForkAllHelperTest, HandleDebugAppLaunchDelay_NonTemplateProcess_0100, TestSize.Level1)
{
    auto appLaunchData = CreateDebugTemplateLaunchData();
    appLaunchData.SetImageProcessType(static_cast<int32_t>(ImageProcessType::WORK));
    OHOS::system::SetParameter(SNAPSHOT_LAUNCH_DELAY_MS, "500");

    auto start = std::chrono::steady_clock::now();
    ForkAllHelper::HandleDebugAppLaunchDelay(appLaunchData, true);
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);

    EXPECT_LT(elapsed.count(), 400);
}

/**
 * @tc.name: HandleDebugAppLaunchDelay_DebugTemplateProcess_0100
 * @tc.desc: Delay debug template application launch when developer mode is enabled.
 * @tc.type: FUNC
 */
HWTEST_F(ForkAllHelperTest, HandleDebugAppLaunchDelay_DebugTemplateProcess_0100, TestSize.Level1)
{
    auto appLaunchData = CreateDebugTemplateLaunchData();
    OHOS::system::SetParameter(SNAPSHOT_LAUNCH_DELAY_MS, "1");
    if (OHOS::system::GetIntParameter<int32_t>(SNAPSHOT_LAUNCH_DELAY_MS, 0) != 1) {
        SUCCEED() << "snapshot launch delay parameter is unavailable";
        return;
    }

    auto start = std::chrono::steady_clock::now();
    ForkAllHelper::HandleDebugAppLaunchDelay(appLaunchData, true);
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);

    EXPECT_GE(elapsed.count(), 1);
}
} // namespace AppExecFwk
} // namespace OHOS
