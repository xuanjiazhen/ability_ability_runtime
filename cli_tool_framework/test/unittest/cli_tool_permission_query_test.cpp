/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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
#include "cli_tool_manager_service.h"
#include "command_permission.h"

namespace OHOS {
namespace CliTool {

class CliToolPermissionQueryTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void CliToolPermissionQueryTest::SetUpTestCase()
{
    // Initialize service and test data
}

void CliToolPermissionQueryTest::TearDownTestCase()
{
    // Clean up test data
}

// Test 1: Batch query main command permissions
TEST_F(CliToolPermissionQueryTest, BatchQueryMainCommandPermission)
{
    std::vector<Command> cmds = {
        {"aa", ""},
        {"bm", ""}
    };

    std::vector<CommandPermission> results;
    int32_t ret = CliToolManagerService::GetInstance()->BatchQueryPermissionBySubCommand(cmds, results);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(results.size(), 2);
    EXPECT_EQ(results[0].queryRet, QUERY_SUCCESS);
    EXPECT_EQ(results[1].queryRet, QUERY_SUCCESS);
}

// Test 2: Batch query subcommand permissions
TEST_F(CliToolPermissionQueryTest, BatchQuerySubCommandPermission)
{
    std::vector<Command> cmds = {
        {"aa", "start"},
        {"aa", "stop"}
    };

    std::vector<CommandPermission> results;
    int32_t ret = CliToolManagerService::GetInstance()->BatchQueryPermissionBySubCommand(cmds, results);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(results.size(), 2);
    EXPECT_EQ(results[0].queryRet, QUERY_SUCCESS);
    EXPECT_EQ(results[1].queryRet, QUERY_SUCCESS);
}

// Test 3: Query non-existent command
TEST_F(CliToolPermissionQueryTest, QueryNonExistentCommand)
{
    std::vector<Command> cmds = {
        {"nonexistent", ""}
    };

    std::vector<CommandPermission> results;
    int32_t ret = CliToolManagerService::GetInstance()->BatchQueryPermissionBySubCommand(cmds, results);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].queryRet, QUERY_COMMAND_NOT_EXIST);
    EXPECT_TRUE(results[0].permissions.empty());
}

// Test 4: Main command with empty permissions
TEST_F(CliToolPermissionQueryTest, MainCommandWithEmptyPermissions)
{
    std::vector<Command> cmds = {
        {"bm", ""}
    };

    std::vector<CommandPermission> results;
    int32_t ret = CliToolManagerService::GetInstance()->BatchQueryPermissionBySubCommand(cmds, results);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(results[0].queryRet, QUERY_SUCCESS);
    // Empty permissions means no permission required, not command not exist
}

// Test 5: Parameter validation - empty list
TEST_F(CliToolPermissionQueryTest, InvalidParameter_EmptyList)
{
    std::vector<Command> cmds; // Empty list
    std::vector<CommandPermission> results;

    int32_t ret = CliToolManagerService::GetInstance()->BatchQueryPermissionBySubCommand(cmds, results);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

// Test 6: Parameter validation - empty toolName
TEST_F(CliToolPermissionQueryTest, InvalidParameter_EmptyToolName)
{
    std::vector<Command> cmds = {
        {"", ""}
    };
    std::vector<CommandPermission> results;

    int32_t ret = CliToolManagerService::GetInstance()->BatchQueryPermissionBySubCommand(cmds, results);
    EXPECT_EQ(ret, ERR_OK);  // Interface call success
    EXPECT_EQ(results[0].queryRet, QUERY_COMMAND_NOT_EXIST);  // Single query result is not exist
}

// Test 7: Mixed query
TEST_F(CliToolPermissionQueryTest, MixedQuery)
{
    std::vector<Command> cmds = {
        {"aa", ""},           // Main command
        {"aa", "start"},      // Subcommand
        {"invalid", ""},      // Non-existent command
        {"aa", "invalid"}     // Non-existent subcommand
    };

    std::vector<CommandPermission> results;
    int32_t ret = CliToolManagerService::GetInstance()->BatchQueryPermissionBySubCommand(cmds, results);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(results.size(), 4);
    EXPECT_EQ(results[0].queryRet, QUERY_SUCCESS);
    EXPECT_EQ(results[1].queryRet, QUERY_SUCCESS);
    EXPECT_EQ(results[2].queryRet, QUERY_COMMAND_NOT_EXIST);
    EXPECT_EQ(results[3].queryRet, QUERY_COMMAND_NOT_EXIST);
}

} // namespace CliTool
} // namespace OHOS
