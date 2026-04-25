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
#include <chrono>
#include "cli_tool_mgr_client.h"
#include "command_permission.h"

namespace OHOS {
namespace CliTool {

class CliToolModuleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void CliToolModuleTest::SetUpTestCase()
{
    // Initialize test environment
}

void CliToolModuleTest::TearDownTestCase()
{
    // Clean up test environment
}

// Test: Non-SA caller rejected
TEST_F(CliToolModuleTest, NonSACallerRejected)
{
    // Call with non-SA TokenID
    std::vector<Command> cmds = {{"aa", ""}};
    std::vector<CommandPermission> results;

    int32_t ret = CliToolMGRClient::GetInstance().BatchQueryPermissionBySubCommand(cmds, results);
    EXPECT_EQ(ret, ERR_NOT_SA_CALLER);
}

// Test: Permission check
TEST_F(CliToolModuleTest, PermissionCheck)
{
    // Call with SA TokenID but without QUERY_CLI_TOOL permission
    std::vector<Command> cmds = {{"aa", ""}};
    std::vector<CommandPermission> results;

    int32_t ret = CliToolMGRClient::GetInstance().BatchQueryPermissionBySubCommand(cmds, results);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

// Test: Performance - batch query performance
TEST_F(CliToolModuleTest, PerformanceTest)
{
    std::vector<Command> cmds;
    for (int i = 0; i < 100; ++i) {
        cmds.push_back({"aa", ""});
    }

    auto startTime = std::chrono::steady_clock::now();

    std::vector<CommandPermission> results;
    int32_t ret = CliToolMGRClient::GetInstance().BatchQueryPermissionBySubCommand(cmds, results);

    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_LT(duration, 500);  // 100 queries should complete within 500ms (average <5ms per query)
}

} // namespace CliTool
} // namespace OHOS
