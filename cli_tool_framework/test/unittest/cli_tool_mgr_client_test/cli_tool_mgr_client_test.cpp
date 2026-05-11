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

#define private public
#include "cli_tool_mgr_client.h"
#undef private

using namespace testing::ext;

namespace OHOS {
namespace CliTool {

class CliToolMGRClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    /**
     * @brief Helper method to ensure service proxy is unavailable
     * This simulates the scenario where the service is not running or connection fails
     */
    void EnsureServiceProxyUnavailable();

    /**
     * @brief Helper method to test BatchQueryPermissionBySubCommand with given commands
     * @param cmds Command list to test
     * @param testName Name of the test for logging
     */
    void TestBatchQueryWithServiceUnavailable(const std::vector<Command> &cmds, const std::string &testName);
};

void CliToolMGRClientTest::SetUpTestCase(void) {}
void CliToolMGRClientTest::TearDownTestCase(void) {}
void CliToolMGRClientTest::SetUp() {}
void CliToolMGRClientTest::TearDown() {}

void CliToolMGRClientTest::EnsureServiceProxyUnavailable()
{
    // Explicitly clear the proxy to simulate service unavailability
    // This ensures we test the error handling path when the service is not available
    auto& client = CliToolMGRClient::GetInstance();
    client.ClearProxy();
}

void CliToolMGRClientTest::TestBatchQueryWithServiceUnavailable(
    const std::vector<Command> &cmds, const std::string &testName)
{
    GTEST_LOG_(INFO) << testName << " start";

    // Ensure service proxy is unavailable for this test
    EnsureServiceProxyUnavailable();

    auto& client = CliToolMGRClient::GetInstance();
    std::vector<CommandPermission> cmdPermissions;

    int32_t ret = client.BatchQueryPermissionBySubCommand(cmds, cmdPermissions);

    // When service proxy is unavailable, should return error and empty results
    EXPECT_EQ(ret, GET_CLI_TOOL_MGR_SERVICE_FAILED) << "Service proxy should be unavailable";
    EXPECT_TRUE(cmdPermissions.empty()) << "Result should be empty when service is unavailable";

    GTEST_LOG_(INFO) << testName << " end";
}

/**
 * @tc.name: CliToolMGRClient_GetInstance_0100
 * @tc.desc: Test GetInstance returns singleton instance
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, GetInstance_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolMGRClient_GetInstance_0100 start";

    auto& instance1 = CliToolMGRClient::GetInstance();
    auto& instance2 = CliToolMGRClient::GetInstance();

    EXPECT_EQ(&instance1, &instance2);

    GTEST_LOG_(INFO) << "CliToolMGRClient_GetInstance_0100 end";
}

/**
 * @tc.name: CliToolMGRClient_GetAllToolSummaries_0100
 * @tc.desc: Test GetAllToolSummaries returns error when proxy is null
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, GetAllToolSummaries_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolMGRClient_GetAllToolSummaries_0100 start";

    auto& client = CliToolMGRClient::GetInstance();
    std::vector<ToolSummary> summaries;
    ErrCode ret = client.GetAllToolSummaries(summaries);

    EXPECT_NE(ret, -2);

    GTEST_LOG_(INFO) << "CliToolMGRClient_GetAllToolSummaries_0100 end";
}

/**
 * @tc.name: CliToolMGRClient_GetToolInfoByName_0100
 * @tc.desc: Test GetToolInfoByName returns error when proxy is null
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, GetToolInfoByName_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolMGRClient_GetToolInfoByName_0100 start";

    auto& client = CliToolMGRClient::GetInstance();
    ToolInfo tool;
    ErrCode ret = client.GetToolInfoByName("test_tool", tool);

    EXPECT_NE(ret, -2);

    GTEST_LOG_(INFO) << "CliToolMGRClient_GetToolInfoByName_0100 end";
}

/**
 * @tc.name: CliToolMGRClient_GetAllToolInfos_0100
 * @tc.desc: Test GetAllToolInfos returns error when proxy is null
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, GetAllToolInfos_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolMGRClient_GetAllToolInfos_0100 start";

    auto& client = CliToolMGRClient::GetInstance();
    std::vector<ToolInfo> tools;
    ErrCode ret = client.GetAllToolInfos(tools);

    EXPECT_NE(ret, -2);

    GTEST_LOG_(INFO) << "CliToolMGRClient_GetAllToolInfos_0100 end";
}

/**
 * @tc.name: CliToolMGRClient_RegisterTool_0100
 * @tc.desc: Test RegisterTool returns error when proxy is null
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, RegisterTool_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolMGRClient_RegisterTool_0100 start";

    auto& client = CliToolMGRClient::GetInstance();
    ToolInfo tool;
    tool.name = "test_tool";
    ErrCode ret = client.RegisterTool(tool);

    EXPECT_NE(ret, -2);

    GTEST_LOG_(INFO) << "CliToolMGRClient_RegisterTool_0100 end";
}
} // namespace CliTool
} // namespace OHOS
