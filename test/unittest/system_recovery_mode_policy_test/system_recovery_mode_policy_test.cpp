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
    }
};

/**
 * @tc.name: IsRecoveryMode_0100
 * @tc.desc: Recovery mode is active only for the exact system_recovery value.
 * @tc.type: FUNC
 */
HWTEST_F(SystemRecoveryModePolicyTest, IsRecoveryMode_0100, TestSize.Level1)
{
    auto &policy = SystemRecoveryModePolicy::GetInstance();
    EXPECT_FALSE(policy.IsRecoveryMode());

    g_startMode = "normal";
    EXPECT_FALSE(policy.IsRecoveryMode());

    g_startMode = "SYSTEM_RECOVERY";
    EXPECT_FALSE(policy.IsRecoveryMode());

    g_startMode = SYSTEM_RECOVERY_MODE_VALUE;
    EXPECT_TRUE(policy.IsRecoveryMode());
}

/**
 * @tc.name: ShouldBlockBundle_0100
 * @tc.desc: No bundle is blocked outside system recovery mode.
 * @tc.type: FUNC
 */
HWTEST_F(SystemRecoveryModePolicyTest, ShouldBlockBundle_0100, TestSize.Level1)
{
    RecoveryBundleKey key { "com.example.thirdparty", 100, 0 };
    EXPECT_FALSE(SystemRecoveryModePolicy::GetInstance().ShouldBlockBundle(key));
}

/**
 * @tc.name: ShouldBlockBundle_0200
 * @tc.desc: A non-essential or unresolved bundle is blocked in recovery mode.
 * @tc.type: FUNC
 */
HWTEST_F(SystemRecoveryModePolicyTest, ShouldBlockBundle_0200, TestSize.Level1)
{
    g_startMode = SYSTEM_RECOVERY_MODE_VALUE;
    auto &policy = SystemRecoveryModePolicy::GetInstance();

    RecoveryBundleKey thirdParty { "com.example.thirdparty", 100, 0 };
    EXPECT_TRUE(policy.ShouldBlockBundle(thirdParty));

    RecoveryBundleKey unresolved;
    EXPECT_TRUE(policy.ShouldBlockBundle(unresolved));
}

/**
 * @tc.name: ShouldBlockBundle_0300
 * @tc.desc: A demo allowlisted bundle remains available in recovery mode.
 * @tc.type: FUNC
 */
HWTEST_F(SystemRecoveryModePolicyTest, ShouldBlockBundle_0300, TestSize.Level1)
{
    g_startMode = SYSTEM_RECOVERY_MODE_VALUE;
    RecoveryBundleKey settings { "com.ohos.settings", 100, 0 };
    EXPECT_FALSE(SystemRecoveryModePolicy::GetInstance().ShouldBlockBundle(settings));
}

/**
 * @tc.name: ShouldBlockBundle_0400
 * @tc.desc: A product bundle added through the JSON configuration is allowed.
 * @tc.type: FUNC
 */
HWTEST_F(SystemRecoveryModePolicyTest, ShouldBlockBundle_0400, TestSize.Level1)
{
    g_startMode = SYSTEM_RECOVERY_MODE_VALUE;
    RecoveryBundleKey configuredBundle { "com.example.product.recovery", 100, 0 };
    EXPECT_FALSE(SystemRecoveryModePolicy::GetInstance().ShouldBlockBundle(configuredBundle));
}

} // namespace AAFwk
} // namespace OHOS
