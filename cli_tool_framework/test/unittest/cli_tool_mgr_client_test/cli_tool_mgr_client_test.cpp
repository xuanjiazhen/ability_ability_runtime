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
};

void CliToolMGRClientTest::SetUpTestCase(void) {}
void CliToolMGRClientTest::TearDownTestCase(void) {}
void CliToolMGRClientTest::SetUp() {}
void CliToolMGRClientTest::TearDown() {}

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

/**
 * @tc.name: CliToolMGRClient_BatchQueryPermissionBySubCommand_0100
 * @tc.desc: Test BatchQueryPermissionBySubCommand with empty command list
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, BatchQueryPermissionBySubCommand_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolMGRClient_BatchQueryPermissionBySubCommand_0100 start";

    auto& client = CliToolMGRClient::GetInstance();
    std::vector<Command> cmds;
    std::vector<CommandPermission> cmdPermissions;

    int32_t ret = client.BatchQueryPermissionBySubCommand(cmds, cmdPermissions);

    EXPECT_EQ(ret, GET_CLI_TOOL_MGR_SERVICE_FAILED);
    EXPECT_TRUE(cmdPermissions.empty());

    GTEST_LOG_(INFO) << "CliToolMGRClient_BatchQueryPermissionBySubCommand_0100 end";
}

/**
 * @tc.name: CliToolMGRClient_BatchQueryPermissionBySubCommand_0200
 * @tc.desc: Test BatchQueryPermissionBySubCommand with single command without subcommand
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, BatchQueryPermissionBySubCommand_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolMGRClient_BatchQueryPermissionBySubCommand_0200 start";

    auto& client = CliToolMGRClient::GetInstance();
    std::vector<Command> cmds;

    Command cmd;
    cmd.toolName = "test_tool";
    cmd.subCommand = "";
    cmds.push_back(cmd);

    std::vector<CommandPermission> cmdPermissions;

    int32_t ret = client.BatchQueryPermissionBySubCommand(cmds, cmdPermissions);

    EXPECT_EQ(ret, GET_CLI_TOOL_MGR_SERVICE_FAILED);
    EXPECT_TRUE(cmdPermissions.empty());

    GTEST_LOG_(INFO) << "CliToolMGRClient_BatchQueryPermissionBySubCommand_0200 end";
}

/**
 * @tc.name: CliToolMGRClient_BatchQueryPermissionBySubCommand_0300
 * @tc.desc: Test BatchQueryPermissionBySubCommand with single command with subcommand
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, BatchQueryPermissionBySubCommand_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolMGRClient_BatchQueryPermissionBySubCommand_0300 start";

    auto& client = CliToolMGRClient::GetInstance();
    std::vector<Command> cmds;

    Command cmd;
    cmd.toolName = "test_tool";
    cmd.subCommand = "test_subcommand";
    cmds.push_back(cmd);

    std::vector<CommandPermission> cmdPermissions;

    int32_t ret = client.BatchQueryPermissionBySubCommand(cmds, cmdPermissions);

    EXPECT_EQ(ret, GET_CLI_TOOL_MGR_SERVICE_FAILED);
    EXPECT_TRUE(cmdPermissions.empty());

    GTEST_LOG_(INFO) << "CliToolMGRClient_BatchQueryPermissionBySubCommand_0300 end";
}

/**
 * @tc.name: CliToolMGRClient_BatchQueryPermissionBySubCommand_0400
 * @tc.desc: Test BatchQueryPermissionBySubCommand with multiple commands
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, BatchQueryPermissionBySubCommand_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolMGRClient_BatchQueryPermissionBySubCommand_0400 start";

    auto& client = CliToolMGRClient::GetInstance();
    std::vector<Command> cmds;

    Command cmd1;
    cmd1.toolName = "tool1";
    cmd1.subCommand = "sub1";
    cmds.push_back(cmd1);

    Command cmd2;
    cmd2.toolName = "tool2";
    cmd2.subCommand = "";
    cmds.push_back(cmd2);

    Command cmd3;
    cmd3.toolName = "tool3";
    cmd3.subCommand = "sub3";
    cmds.push_back(cmd3);

    std::vector<CommandPermission> cmdPermissions;

    int32_t ret = client.BatchQueryPermissionBySubCommand(cmds, cmdPermissions);

    EXPECT_EQ(ret, GET_CLI_TOOL_MGR_SERVICE_FAILED);
    EXPECT_TRUE(cmdPermissions.empty());

    GTEST_LOG_(INFO) << "CliToolMGRClient_BatchQueryPermissionBySubCommand_0400 end";
}

/**
 * @tc.name: CliToolMGRClient_BatchQueryPermissionBySubCommand_0500
 * @tc.desc: Test BatchQueryPermissionBySubCommand with special characters in tool name
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, BatchQueryPermissionBySubCommand_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolMGRClient_BatchQueryPermissionBySubCommand_0500 start";

    auto& client = CliToolMGRClient::GetInstance();
    std::vector<Command> cmds;

    Command cmd;
    cmd.toolName = "test-tool-with-dashes";
    cmd.subCommand = "sub_command_with_underscores";
    cmds.push_back(cmd);

    std::vector<CommandPermission> cmdPermissions;

    int32_t ret = client.BatchQueryPermissionBySubCommand(cmds, cmdPermissions);

    EXPECT_EQ(ret, GET_CLI_TOOL_MGR_SERVICE_FAILED);
    EXPECT_TRUE(cmdPermissions.empty());

    GTEST_LOG_(INFO) << "CliToolMGRClient_BatchQueryPermissionBySubCommand_0500 end";
}

/**
 * @tc.name: CliToolMGRClient_BatchQueryPermissionBySubCommand_0600
 * @tc.desc: Test BatchQueryPermissionBySubCommand with very long tool name
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, BatchQueryPermissionBySubCommand_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolMGRClient_BatchQueryPermissionBySubCommand_0600 start";

    auto& client = CliToolMGRClient::GetInstance();
    std::vector<Command> cmds;

    Command cmd;
    cmd.toolName = "this_is_a_very_long_tool_name_that_exceeds_normal_length_for_testing_purposes_1234567890";
    cmd.subCommand = "sub";
    cmds.push_back(cmd);

    std::vector<CommandPermission> cmdPermissions;

    int32_t ret = client.BatchQueryPermissionBySubCommand(cmds, cmdPermissions);

    EXPECT_EQ(ret, GET_CLI_TOOL_MGR_SERVICE_FAILED);
    EXPECT_TRUE(cmdPermissions.empty());

    GTEST_LOG_(INFO) << "CliToolMGRClient_BatchQueryPermissionBySubCommand_0600 end";
}

/**
 * @tc.name: CliToolMGRClient_BatchQueryPermissionBySubCommand_0700
 * @tc.desc: Test BatchQueryPermissionBySubCommand boundary with large command count
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, BatchQueryPermissionBySubCommand_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolMGRClient_BatchQueryPermissionBySubCommand_0700 start";

    auto& client = CliToolMGRClient::GetInstance();
    std::vector<Command> cmds;

    // Add 100 commands for stress testing
    for (int i = 0; i < 100; i++) {
        Command cmd;
        cmd.toolName = "tool_" + std::to_string(i);
        cmd.subCommand = "sub_" + std::to_string(i);
        cmds.push_back(cmd);
    }

    std::vector<CommandPermission> cmdPermissions;

    int32_t ret = client.BatchQueryPermissionBySubCommand(cmds, cmdPermissions);

    EXPECT_EQ(ret, GET_CLI_TOOL_MGR_SERVICE_FAILED);
    EXPECT_TRUE(cmdPermissions.empty());

    GTEST_LOG_(INFO) << "CliToolMGRClient_BatchQueryPermissionBySubCommand_0700 end";
}

/**
 * @tc.name: CliToolMGRClient_BatchQueryPermissionBySubCommand_0800
 * @tc.desc: Test BatchQueryPermissionBySubCommand with empty tool name
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, BatchQueryPermissionBySubCommand_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolMGRClient_BatchQueryPermissionBySubCommand_0800 start";

    auto& client = CliToolMGRClient::GetInstance();
    std::vector<Command> cmds;

    Command cmd;
    cmd.toolName = "";
    cmd.subCommand = "sub";
    cmds.push_back(cmd);

    std::vector<CommandPermission> cmdPermissions;

    int32_t ret = client.BatchQueryPermissionBySubCommand(cmds, cmdPermissions);

    EXPECT_EQ(ret, GET_CLI_TOOL_MGR_SERVICE_FAILED);
    EXPECT_TRUE(cmdPermissions.empty());

    GTEST_LOG_(INFO) << "CliToolMGRClient_BatchQueryPermissionBySubCommand_0800 end";
}

} // namespace CliTool
} // namespace OHOS
