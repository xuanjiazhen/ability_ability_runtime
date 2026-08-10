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

#include "system_recovery_mode_constants.h"
#include "system_recovery_mode_policy.h"
#include "json_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace {
std::string g_startMode;
}

namespace system {
std::string GetParameter(const std::string &key, const std::string &def)
{
    return key == AAFwk::SYSTEM_RECOVERY_MODE_PARAMETER ? g_startMode : def;
}
} // namespace system

namespace AAFwk {

bool JsonUtils::LoadConfiguration(const std::string &, nlohmann::json &jsonBuf, const std::string &)
{
    jsonBuf = {
        { SYSTEM_RECOVERY_APP_WHITELIST_KEY, {
            "com.ohos.settings",
            "com.ohos.settingsdata",
            "com.ohos.systemui",
            "com.ohos.sceneboard",
            "com.ohos.launcher",
            "com.ohos.systemdialog",
            "com.example.product.recovery",
        } }
    };
    return true;
}

class SystemRecoveryModePolicyTest : public testing::Test {
public:
    void SetUp() override
    {
        g_startMode.clear();
        SystemRecoveryModePolicy::GetInstance().SetEnabled(false);
    }
};

/**
 * @tc.name: SetEnabled_0100
 * @tc.desc: The controller can update the in-memory recovery state.
 * @tc.type: FUNC
 */
HWTEST_F(SystemRecoveryModePolicyTest, SetEnabled_0100, TestSize.Level1)
{
    auto &policy = SystemRecoveryModePolicy::GetInstance();
    EXPECT_FALSE(policy.IsEnabled());
    policy.SetEnabled(true);
    EXPECT_TRUE(policy.IsEnabled());
    policy.SetEnabled(false);
    EXPECT_FALSE(policy.IsEnabled());
}

/**
 * @tc.name: ShouldBlockBundle_0100
 * @tc.desc: No bundle is blocked outside system recovery mode.
 * @tc.type: FUNC
 */
HWTEST_F(SystemRecoveryModePolicyTest, ShouldBlockBundle_0100, TestSize.Level1)
{
    EXPECT_FALSE(SystemRecoveryModePolicy::GetInstance().ShouldBlockBundle("com.example.thirdparty"));
}

/**
 * @tc.name: ShouldBlockBundle_0200
 * @tc.desc: A non-essential or unresolved bundle is blocked in recovery mode.
 * @tc.type: FUNC
 */
HWTEST_F(SystemRecoveryModePolicyTest, ShouldBlockBundle_0200, TestSize.Level1)
{
    auto &policy = SystemRecoveryModePolicy::GetInstance();
    policy.SetEnabled(true);

    EXPECT_TRUE(policy.ShouldBlockBundle("com.example.thirdparty"));
    EXPECT_TRUE(policy.ShouldBlockBundle(""));
}

/**
 * @tc.name: ShouldBlockBundle_0300
 * @tc.desc: A demo allowlisted bundle remains available in recovery mode.
 * @tc.type: FUNC
 */
HWTEST_F(SystemRecoveryModePolicyTest, ShouldBlockBundle_0300, TestSize.Level1)
{
    auto &policy = SystemRecoveryModePolicy::GetInstance();
    policy.SetEnabled(true);
    EXPECT_FALSE(policy.ShouldBlockBundle("com.ohos.settings"));
}

/**
 * @tc.name: ShouldBlockBundle_0400
 * @tc.desc: A product bundle added through the JSON configuration is allowed.
 * @tc.type: FUNC
 */
HWTEST_F(SystemRecoveryModePolicyTest, ShouldBlockBundle_0400, TestSize.Level1)
{
    auto &policy = SystemRecoveryModePolicy::GetInstance();
    policy.SetEnabled(true);
    EXPECT_FALSE(policy.ShouldBlockBundle("com.example.product.recovery"));
}

} // namespace AAFwk
} // namespace OHOS
