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
 * @tc.name: CliToolMGRClient_GetCliToolManager_0100
 * @tc.desc: Test GetCliToolManager returns null proxy when service unavailable
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, GetCliToolManager_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolMGRClient_GetCliToolManager_0100 start";

    auto& client = CliToolMGRClient::GetInstance();
    auto proxy = client.GetCliToolManager();

    // When service is unavailable, proxy should be null
    EXPECT_EQ(proxy, nullptr);

    GTEST_LOG_(INFO) << "CliToolMGRClient_GetCliToolManager_0100 end";
}

/**
 * @tc.name: CliToolMGRClient_ResetProxy_0100
 * @tc.desc: Test ResetProxy functionality
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, ResetProxy_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolMGRClient_ResetProxy_0100 start";

    auto& client = CliToolMGRClient::GetInstance();
    wptr<IRemoteObject> remote;

    // ResetProxy should handle null remote gracefully
    client.ResetProxy(remote);
    EXPECT_NE(&client, nullptr);

    GTEST_LOG_(INFO) << "CliToolMGRClient_ResetProxy_0100 end";
}

/**
 * @tc.name: CliToolMGRClient_QueryToolSummaries_0100
 * @tc.desc: Test QueryToolSummaries returns error when proxy is null
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, QueryToolSummaries_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolMGRClient_QueryToolSummaries_0100 start";

    auto& client = CliToolMGRClient::GetInstance();
    std::vector<ToolSummary> summaries;
    ErrCode ret = client.QueryToolSummaries(summaries);

    EXPECT_NE(ret, ERR_OK);

    GTEST_LOG_(INFO) << "CliToolMGRClient_QueryToolSummaries_0100 end";
}

/**
 * @tc.name: CliToolMGRClient_GetToolByName_0100
 * @tc.desc: Test GetToolByName returns error when proxy is null
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, GetToolByName_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolMGRClient_GetToolByName_0100 start";

    auto& client = CliToolMGRClient::GetInstance();
    ToolInfo tool;
    ErrCode ret = client.GetToolByName("test_tool", tool);

    EXPECT_NE(ret, ERR_OK);

    GTEST_LOG_(INFO) << "CliToolMGRClient_GetToolByName_0100 end";
}

/**
 * @tc.name: CliToolMGRClient_GetAllTools_0100
 * @tc.desc: Test GetAllTools returns error when proxy is null
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, GetAllTools_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolMGRClient_GetAllTools_0100 start";

    auto& client = CliToolMGRClient::GetInstance();
    std::vector<ToolInfo> tools;
    ErrCode ret = client.GetAllTools(tools);

    EXPECT_NE(ret, ERR_OK);

    GTEST_LOG_(INFO) << "CliToolMGRClient_GetAllTools_0100 end";
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

    EXPECT_NE(ret, ERR_OK);

    GTEST_LOG_(INFO) << "CliToolMGRClient_RegisterTool_0100 end";
}
} // namespace CliTool
} // namespace OHOS
