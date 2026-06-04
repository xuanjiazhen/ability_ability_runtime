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

#include "nlohmann/json.hpp"

#define private public
#include "ccm_util.h"
#undef private
#include "cli_common_mock.h"
#include "permission_util.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t CUSTOM_CLI_LIMIT = 16;
constexpr Security::AccessToken::AccessTokenID TEST_TOKEN_ID = 100;
}

class CliCommonUtilTest : public testing::Test {
public:
    void SetUp() override
    {
        CliCommonMock::Reset();
        auto &ccmUtil = CcmUtil::GetInstance();
        ccmUtil.maxCliQuantity_.isLoaded = false;
        ccmUtil.maxCliQuantity_.value = DEFAULT_MAX_CLI_QUANTITY;
        ccmUtil.publicCliToolNames_.isLoaded = false;
        ccmUtil.publicCliToolNames_.value.clear();
    }

    void TearDown() override
    {
        CliCommonMock::Reset();
    }
};

/**
 * @tc.name: CcmUtil_GetCliConcurrencyLimit_0100
 * @tc.desc: Test ccm util loads parameter once and then uses cached value
 * @tc.type: FUNC
 */
HWTEST_F(CliCommonUtilTest, CcmUtil_GetCliConcurrencyLimit_0100, TestSize.Level1)
{
    CliCommonMock::intParameterValue = CUSTOM_CLI_LIMIT;
    EXPECT_EQ(CcmUtil::GetInstance().GetCliConcurrencyLimit(), CUSTOM_CLI_LIMIT);

    CliCommonMock::intParameterValue = CUSTOM_CLI_LIMIT + 1;
    EXPECT_EQ(CcmUtil::GetInstance().GetCliConcurrencyLimit(), CUSTOM_CLI_LIMIT);
}

/**
 * @tc.name: CcmUtil_GetPublicCliToolNames_0200
 * @tc.desc: Test public cli tool names use cached value
 * @tc.type: FUNC
 */
HWTEST_F(CliCommonUtilTest, CcmUtil_GetPublicCliToolNames_0200, TestSize.Level1)
{
    auto &ccmUtil = CcmUtil::GetInstance();
    ccmUtil.publicCliToolNames_.isLoaded = true;
    ccmUtil.publicCliToolNames_.value = {"testTool"};

    auto publicNames = ccmUtil.GetPublicCliToolNames();
    EXPECT_EQ(publicNames.size(), 1);
    EXPECT_NE(publicNames.find("testTool"), publicNames.end());
}

/**
 * @tc.name: CcmUtil_ParsePublicCliToolNames_0300
 * @tc.desc: Test parsing public cli tool names from string array
 * @tc.type: FUNC
 */
HWTEST_F(CliCommonUtilTest, CcmUtil_ParsePublicCliToolNames_0300, TestSize.Level1)
{
    nlohmann::json object = {
        {"publicCliTools", {"toolA", "toolB", "toolA"}}
    };
    std::set<std::string> publicNames;

    EXPECT_TRUE(CcmUtil::GetInstance().ParsePublicCliToolNames(object, publicNames));
    EXPECT_EQ(publicNames.size(), 2);
    EXPECT_NE(publicNames.find("toolA"), publicNames.end());
    EXPECT_NE(publicNames.find("toolB"), publicNames.end());
}

/**
 * @tc.name: CcmUtil_ParsePublicCliToolNames_0400
 * @tc.desc: Test invalid and empty public cli tool name entries are skipped
 * @tc.type: FUNC
 */
HWTEST_F(CliCommonUtilTest, CcmUtil_ParsePublicCliToolNames_0400, TestSize.Level1)
{
    nlohmann::json object = {
        {"publicCliTools", {"toolA", "", 1, false, nullptr, "toolB"}}
    };
    std::set<std::string> publicNames;

    EXPECT_TRUE(CcmUtil::GetInstance().ParsePublicCliToolNames(object, publicNames));
    EXPECT_EQ(publicNames.size(), 2);
    EXPECT_NE(publicNames.find("toolA"), publicNames.end());
    EXPECT_NE(publicNames.find("toolB"), publicNames.end());
}

/**
 * @tc.name: CcmUtil_ParsePublicCliToolNames_0500
 * @tc.desc: Test invalid public cli tool list config returns false and clears output
 * @tc.type: FUNC
 */
HWTEST_F(CliCommonUtilTest, CcmUtil_ParsePublicCliToolNames_0500, TestSize.Level1)
{
    std::set<std::string> publicNames = {"oldTool"};

    nlohmann::json missingKey = nlohmann::json::object();
    EXPECT_FALSE(CcmUtil::GetInstance().ParsePublicCliToolNames(missingKey, publicNames));
    EXPECT_TRUE(publicNames.empty());

    nlohmann::json notArray = {
        {"publicCliTools", "toolA"}
    };
    publicNames = {"oldTool"};
    EXPECT_FALSE(CcmUtil::GetInstance().ParsePublicCliToolNames(notArray, publicNames));
    EXPECT_TRUE(publicNames.empty());
}

/**
 * @tc.name: PermissionUtil_VerifyAccessToken_0200
 * @tc.desc: Test single permission grant and denial branches
 * @tc.type: FUNC
 */
HWTEST_F(CliCommonUtilTest, PermissionUtil_VerifyAccessToken_0200, TestSize.Level1)
{
    CliCommonMock::singlePermissionResult = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    EXPECT_TRUE(PermissionUtil::VerifyAccessToken(TEST_TOKEN_ID, "ohos.permission.EXEC_CLI_TOOL"));

    CliCommonMock::singlePermissionResult = Security::AccessToken::PermissionState::PERMISSION_DENIED;
    EXPECT_FALSE(PermissionUtil::VerifyAccessToken(TEST_TOKEN_ID, "ohos.permission.EXEC_CLI_TOOL"));
}
} // namespace CliTool
} // namespace OHOS
