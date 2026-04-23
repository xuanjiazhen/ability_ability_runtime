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
#include <gmock/gmock.h>

#define private public
#include "cli_tool_manager_service.h"
#undef private

#include "cli_error_code.h"
#include "exec_options.h"
#include "tool_info.h"

using namespace testing::ext;
using namespace OHOS::CliTool;

namespace OHOS {
namespace CliTool {
class CliToolManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    void RegisterTestTool(const std::string& name, const std::string& schema);

    sptr<CliToolManagerService> service_;
};

void CliToolManagerServiceTest::SetUpTestCase(void)
{
    // Initialize test environment
}

void CliToolManagerServiceTest::TearDownTestCase(void)
{
    // Cleanup test environment
}

void CliToolManagerServiceTest::SetUp()
{
    service_ = CliToolManagerService::GetInstance();
    service_->activeSessionCount_.store(0);
}

void CliToolManagerServiceTest::TearDown()
{
    service_->activeSessionCount_.store(0);
}

void CliToolManagerServiceTest::RegisterTestTool(const std::string& name, const std::string& schema)
{
    ToolInfo tool;
    tool.name = name;
    tool.description = "Test tool: " + name;
    tool.executablePath = "/system/bin/" + name;
    tool.inputSchema = schema;
    CliToolDataManager::GetInstance().RegisterTool(tool);
}

/**
 * @tc.name: CliToolManagerService_GetInstance_0100
 * @tc.desc: Test GetInstance returns singleton instance
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, GetInstance_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_GetInstance_0100 start";

    auto instance1 = CliToolManagerService::GetInstance();
    auto instance2 = CliToolManagerService::GetInstance();

    EXPECT_EQ(instance1.GetRefPtr(), instance2.GetRefPtr());

    GTEST_LOG_(INFO) << "CliToolManagerService_GetInstance_0100 end";
}

/**
 * @tc.name: CliToolManagerService_ExecTool_0100
 * @tc.desc: Test ExecTool when session limit is exceeded
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, ExecTool_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0100 start";

    service_->activeSessionCount_.store(8);

    ExecToolParam param;
    param.toolName = "test_tool";
    param.subcommand = "";
    param.challenge = "test_challenge";

    std::map<std::string, std::string> args;
    CliSessionInfo session;

    int32_t result = service_->ExecTool(param, args, session);

    EXPECT_EQ(result, ERR_SESSION_LIMIT_EXCEEDED);
    EXPECT_EQ(service_->activeSessionCount_.load(), 8);

    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0100 end";
}

/**
 * @tc.name: CliToolManagerService_ExecTool_0200
 * @tc.desc: Test ExecTool when tool does not exist
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, ExecTool_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0200 start";

    ExecToolParam param;
    param.toolName = "non_existent_tool";
    param.subcommand = "";
    param.challenge = "test_challenge";

    std::map<std::string, std::string> args;
    CliSessionInfo session;

    int32_t result = service_->ExecTool(param, args, session);

    EXPECT_EQ(result, ERR_TOOL_NOT_EXIST);

    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0200 end";
}

/**
 * @tc.name: CliToolManagerService_ExecTool_0300
 * @tc.desc: Test ExecTool with empty tool name
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, ExecTool_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0300 start";

    ExecToolParam param;
    param.toolName = "";
    param.subcommand = "";
    param.challenge = "test_challenge";

    std::map<std::string, std::string> args;
    CliSessionInfo session;

    int32_t result = service_->ExecTool(param, args, session);

    EXPECT_EQ(result, ERR_TOOL_NOT_EXIST);

    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0300 end";
}

/**
 * @tc.name: CliToolManagerService_ExecTool_0400
 * @tc.desc: Test ExecTool with invalid argument in schema
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, ExecTool_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0400 start";

    std::string schema = R"({
        "properties": {
            "help": {"type": "boolean", "description": "Show help"}
        }
    })";
    RegisterTestTool("test_tool", schema);

    ExecToolParam param;
    param.toolName = "test_tool";
    param.subcommand = "";
    param.challenge = "test_challenge";

    std::map<std::string, std::string> args;
    args["invalid_arg"] = "value";

    CliSessionInfo session;

    int32_t result = service_->ExecTool(param, args, session);

    EXPECT_EQ(result, ERR_INVALID_PARAM);

    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0400 end";
}

/**
 * @tc.name: CliToolManagerService_ExecTool_0500
 * @tc.desc: Test ExecTool with invalid subcommand
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, ExecTool_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0500 start";

    std::string schema = R"({
        "properties": {
            "build": {
                "type": "object",
                "description": "Build subcommand"
            }
        }
    })";
    RegisterTestTool("test_tool_subcmd", schema);

    ExecToolParam param;
    param.toolName = "test_tool_subcmd";
    param.subcommand = "invalid_subcmd";
    param.challenge = "test_challenge";

    std::map<std::string, std::string> args;

    CliSessionInfo session;

    int32_t result = service_->ExecTool(param, args, session);

    EXPECT_EQ(result, ERR_TOOL_NOT_EXIST);

    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0500 end";
}

/**
 * @tc.name: CliToolManagerService_ExecTool_0600
 * @tc.desc: Test ExecTool with valid subcommand
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, ExecTool_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0600 start";

    std::string schema = R"({
        "properties": {
            "build": {
                "type": "object",
                "description": "Build subcommand"
            },
            "clean": {
                "type": "object",
                "description": "Clean subcommand"
            }
        }
    })";
    RegisterTestTool("test_tool_with_subcmd", schema);

    ExecToolParam param;
    param.toolName = "test_tool_with_subcmd";
    param.subcommand = "build";
    param.challenge = "test_challenge";

    std::map<std::string, std::string> args;

    CliSessionInfo session;

    // This should fail at GenerateSandboxConfig (non-HAP caller)
    // or CreateChildProcess (no actual process creation in test)
    int32_t result = service_->ExecTool(param, args, session);

    // Expected: ERR_NOT_HAP (in test environment)
    EXPECT_TRUE(result == ERR_NOT_HAP || result == ERR_INVALID_VALUE);

    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0600 end";
}

/**
 * @tc.name: CliToolManagerService_ExecTool_0700
 * @tc.desc: Test ExecTool session counter increment
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, ExecTool_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0700 start";

    std::string schema = R"({
        "properties": {
            "help": {"type": "boolean"}
        }
    })";
    RegisterTestTool("tool_counter", schema);

    service_->activeSessionCount_.store(7);

    ExecToolParam param;
    param.toolName = "tool_counter";
    param.subcommand = "";
    param.challenge = "test_challenge";

    std::map<std::string, std::string> args;
    args["help"] = "true";

    CliSessionInfo session;

    // Should fail due to being non-HAP caller, but before session limit check
    int32_t result = service_->ExecTool(param, args, session);

    EXPECT_EQ(result, ERR_NOT_HAP);
    EXPECT_EQ(service_->activeSessionCount_.load(), 7);

    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0700 end";
}

} // namespace CliTool
} // namespace OHOS
