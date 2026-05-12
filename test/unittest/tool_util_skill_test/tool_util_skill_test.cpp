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

#include "hilog_tag_wrapper.h"
#include "integer_wrapper.h"
#include "string_wrapper.h"
#include "bool_wrapper.h"
#include "want_params.h"
#include "skill/skill_execute_result.h"
#include "tool_util.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {

const std::string SESSION_ID = "cli_ohos-arkTSScript_123_456";

}  // namespace

class ToolUtilSkillTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ToolUtilSkillTest::SetUpTestCase(void) {}
void ToolUtilSkillTest::TearDownTestCase(void) {}
void ToolUtilSkillTest::SetUp() {}
void ToolUtilSkillTest::TearDown() {}

/**
 * @tc.name: ToolUtil_IsSkillTool_0100
 * @tc.desc: Test IsSkillTool returns true for ohos-arkTSScript
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilSkillTest, ToolUtil_IsSkillTool_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSkillTool_0100 start");
    EXPECT_TRUE(ToolUtil::IsSkillTool("ohos-arkTSScript"));
    TAG_LOGI(AAFwkTag::TEST, "IsSkillTool_0100 end");
}

/**
 * @tc.name: ToolUtil_IsSkillTool_0200
 * @tc.desc: Test IsSkillTool returns false for other tool names
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilSkillTest, ToolUtil_IsSkillTool_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSkillTool_0200 start");
    EXPECT_FALSE(ToolUtil::IsSkillTool("other-tool"));
    EXPECT_FALSE(ToolUtil::IsSkillTool(""));
    EXPECT_FALSE(ToolUtil::IsSkillTool("ohos-arkTSScript-v2"));
    TAG_LOGI(AAFwkTag::TEST, "IsSkillTool_0200 end");
}

/**
 * @tc.name: ToolUtil_NormalizeSkillParamKeys_0100
 * @tc.desc: Test NormalizeSkillParamKeys removes -- prefix
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilSkillTest, ToolUtil_NormalizeSkillParamKeys_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NormalizeSkillParamKeys_0100 start");

    AAFwk::WantParams args;
    args.SetParam("--skillName", AAFwk::String::Box("test"));
    args.SetParam("--scriptPath", AAFwk::String::Box("ets/test"));

    ToolUtil::NormalizeSkillParamKeys(args);

    EXPECT_EQ(args.GetStringParam("skillName"), "test");
    EXPECT_EQ(args.GetStringParam("scriptPath"), "ets/test");
    EXPECT_TRUE(args.GetStringParam("--skillName").empty());

    TAG_LOGI(AAFwkTag::TEST, "NormalizeSkillParamKeys_0100 end");
}

/**
 * @tc.name: ToolUtil_NormalizeSkillParamKeys_0200
 * @tc.desc: Test NormalizeSkillParamKeys removes - prefix
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilSkillTest, ToolUtil_NormalizeSkillParamKeys_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NormalizeSkillParamKeys_0200 start");

    AAFwk::WantParams args;
    args.SetParam("-skillName", AAFwk::String::Box("test"));

    ToolUtil::NormalizeSkillParamKeys(args);

    EXPECT_EQ(args.GetStringParam("skillName"), "test");
    EXPECT_TRUE(args.GetStringParam("-skillName").empty());

    TAG_LOGI(AAFwkTag::TEST, "NormalizeSkillParamKeys_0200 end");
}

/**
 * @tc.name: ToolUtil_NormalizeSkillParamKeys_0300
 * @tc.desc: Test NormalizeSkillParamKeys does not modify keys without prefix
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilSkillTest, ToolUtil_NormalizeSkillParamKeys_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NormalizeSkillParamKeys_0300 start");

    AAFwk::WantParams args;
    args.SetParam("skillName", AAFwk::String::Box("test"));
    args.SetParam("param1", AAFwk::String::Box("value"));

    ToolUtil::NormalizeSkillParamKeys(args);

    EXPECT_EQ(args.GetStringParam("skillName"), "test");
    EXPECT_EQ(args.GetStringParam("param1"), "value");

    TAG_LOGI(AAFwkTag::TEST, "NormalizeSkillParamKeys_0300 end");
}

/**
 * @tc.name: ToolUtil_ExpandArgsJsonString_0100
 * @tc.desc: Test ExpandArgsJsonString parses JSON string
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilSkillTest, ToolUtil_ExpandArgsJsonString_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExpandArgsJsonString_0100 start");

    AAFwk::WantParams args;
    args.SetParam("args", AAFwk::String::Box("{\"param1\":\"hello\",\"param2\":42}"));

    ToolUtil::ExpandArgsJsonString(args);

    EXPECT_EQ(args.GetStringParam("param1"), "hello");
    EXPECT_EQ(args.GetIntParam("param2"), 42);
    EXPECT_TRUE(args.GetStringParam("args").empty());

    TAG_LOGI(AAFwkTag::TEST, "ExpandArgsJsonString_0100 end");
}

/**
 * @tc.name: ToolUtil_ExpandArgsJsonString_0200
 * @tc.desc: Test ExpandArgsJsonString with boolean value
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilSkillTest, ToolUtil_ExpandArgsJsonString_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExpandArgsJsonString_0200 start");

    AAFwk::WantParams args;
    args.SetParam("args", AAFwk::String::Box("{\"flag\":true}"));

    ToolUtil::ExpandArgsJsonString(args);

    auto flag = args.GetParam("flag");
    EXPECT_NE(flag, nullptr);
    EXPECT_NE(AAFwk::IBoolean::Query(flag), nullptr);

    TAG_LOGI(AAFwkTag::TEST, "ExpandArgsJsonString_0200 end");
}

/**
 * @tc.name: ToolUtil_ExpandArgsJsonString_0300
 * @tc.desc: Test ExpandArgsJsonString with invalid JSON
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilSkillTest, ToolUtil_ExpandArgsJsonString_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExpandArgsJsonString_0300 start");

    AAFwk::WantParams args;
    args.SetParam("args", AAFwk::String::Box("not-json"));
    args.SetParam("param1", AAFwk::String::Box("existing"));

    ToolUtil::ExpandArgsJsonString(args);

    // Existing params should remain untouched when JSON is invalid
    EXPECT_EQ(args.GetStringParam("param1"), "existing");

    TAG_LOGI(AAFwkTag::TEST, "ExpandArgsJsonString_0300 end");
}

/**
 * @tc.name: ToolUtil_ExpandArgsJsonString_0400
 * @tc.desc: Test ExpandArgsJsonString with no args key
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilSkillTest, ToolUtil_ExpandArgsJsonString_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExpandArgsJsonString_0400 start");

    AAFwk::WantParams args;
    args.SetParam("skillName", AAFwk::String::Box("test"));

    ToolUtil::ExpandArgsJsonString(args);

    // No args key, nothing should change
    EXPECT_EQ(args.GetStringParam("skillName"), "test");

    TAG_LOGI(AAFwkTag::TEST, "ExpandArgsJsonString_0400 end");
}

/**
 * @tc.name: ToolUtil_FilterSkillArgs_0100
 * @tc.desc: Test FilterSkillArgs removes reserved keys
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilSkillTest, ToolUtil_FilterSkillArgs_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FilterSkillArgs_0100 start");

    AAFwk::WantParams args;
    args.SetParam("bundleName", AAFwk::String::Box("com.test"));
    args.SetParam("moduleName", AAFwk::String::Box("entry"));
    args.SetParam("skillName", AAFwk::String::Box("testSkill"));
    args.SetParam("scriptPath", AAFwk::String::Box("ets/test"));
    args.SetParam("functionName", AAFwk::String::Box("handle"));
    args.SetParam("param1", AAFwk::String::Box("hello"));
    args.SetParam("param2", AAFwk::Integer::Box(42));

    auto filtered = ToolUtil::FilterSkillArgs(args);
    ASSERT_NE(filtered, nullptr);

    // Reserved keys should be removed
    EXPECT_TRUE(filtered->GetStringParam("bundleName").empty());
    EXPECT_TRUE(filtered->GetStringParam("moduleName").empty());
    EXPECT_TRUE(filtered->GetStringParam("skillName").empty());
    EXPECT_TRUE(filtered->GetStringParam("scriptPath").empty());
    EXPECT_TRUE(filtered->GetStringParam("functionName").empty());

    // Custom params should remain
    EXPECT_EQ(filtered->GetStringParam("param1"), "hello");
    EXPECT_EQ(filtered->GetIntParam("param2"), 42);

    TAG_LOGI(AAFwkTag::TEST, "FilterSkillArgs_0100 end");
}

/**
 * @tc.name: ToolUtil_FilterSkillArgs_0200
 * @tc.desc: Test FilterSkillArgs with all custom args
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilSkillTest, ToolUtil_FilterSkillArgs_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FilterSkillArgs_0200 start");

    AAFwk::WantParams args;
    args.SetParam("data", AAFwk::String::Box("test"));

    auto filtered = ToolUtil::FilterSkillArgs(args);
    ASSERT_NE(filtered, nullptr);
    EXPECT_EQ(filtered->GetStringParam("data"), "test");

    TAG_LOGI(AAFwkTag::TEST, "FilterSkillArgs_0200 end");
}

/**
 * @tc.name: ToolUtil_BuildSkillSessionInfo_0100
 * @tc.desc: Test BuildSkillSessionInfo with success result
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilSkillTest, ToolUtil_BuildSkillSessionInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "BuildSkillSessionInfo_0100 start");

    AppExecFwk::SkillExecuteResult skillResult;
    skillResult.code = 0;
    skillResult.result = std::make_shared<AAFwk::WantParams>();
    skillResult.result->SetParam("answer", AAFwk::String::Box("done"));

    CliSessionInfo session = ToolUtil::BuildSkillSessionInfo(SESSION_ID, ERR_OK, skillResult);
    EXPECT_EQ(session.sessionId, SESSION_ID);
    EXPECT_EQ(session.toolName, "ohos-arkTSScript");
    EXPECT_EQ(session.status, "completed");
    ASSERT_NE(session.result, nullptr);
    EXPECT_EQ(session.result->exitCode, 0);

    TAG_LOGI(AAFwkTag::TEST, "BuildSkillSessionInfo_0100 end");
}

/**
 * @tc.name: ToolUtil_BuildSkillSessionInfo_0200
 * @tc.desc: Test BuildSkillSessionInfo with error result
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilSkillTest, ToolUtil_BuildSkillSessionInfo_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "BuildSkillSessionInfo_0200 start");

    AppExecFwk::SkillExecuteResult skillResult;
    skillResult.code = -1;
    skillResult.result = nullptr;

    CliSessionInfo session = ToolUtil::BuildSkillSessionInfo(SESSION_ID, -1, skillResult);
    EXPECT_EQ(session.status, "failed");
    ASSERT_NE(session.result, nullptr);
    EXPECT_EQ(session.result->exitCode, -1);

    TAG_LOGI(AAFwkTag::TEST, "BuildSkillSessionInfo_0200 end");
}

/**
 * @tc.name: ToolUtil_BuildSkillSessionInfo_0300
 * @tc.desc: Test BuildSkillSessionInfo with null result WantParams
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilSkillTest, ToolUtil_BuildSkillSessionInfo_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "BuildSkillSessionInfo_0300 start");

    AppExecFwk::SkillExecuteResult skillResult;
    skillResult.code = 0;
    skillResult.result = nullptr;

    CliSessionInfo session = ToolUtil::BuildSkillSessionInfo(SESSION_ID, ERR_OK, skillResult);
    EXPECT_EQ(session.status, "completed");
    ASSERT_NE(session.result, nullptr);
    EXPECT_EQ(session.result->exitCode, 0);

    TAG_LOGI(AAFwkTag::TEST, "BuildSkillSessionInfo_0300 end");
}

/**
 * @tc.name: ToolUtil_NormalizeSkillParamKeys_0400
 * @tc.desc: Test NormalizeSkillParamKeys with conflicting bare key
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilSkillTest, ToolUtil_NormalizeSkillParamKeys_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NormalizeSkillParamKeys_0400 start");

    AAFwk::WantParams args;
    // Both "skillName" and "--skillName" exist, bare key already present
    args.SetParam("skillName", AAFwk::String::Box("original"));
    args.SetParam("--skillName", AAFwk::String::Box("prefixed"));

    ToolUtil::NormalizeSkillParamKeys(args);

    // Bare key exists, so --skillName should NOT be renamed
    EXPECT_EQ(args.GetStringParam("skillName"), "original");

    TAG_LOGI(AAFwkTag::TEST, "NormalizeSkillParamKeys_0400 end");
}

/**
 * @tc.name: ToolUtil_NormalizeSkillParamKeys_0500
 * @tc.desc: Test NormalizeSkillParamKeys with single-dash key and no conflict
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilSkillTest, ToolUtil_NormalizeSkillParamKeys_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NormalizeSkillParamKeys_0500 start");

    AAFwk::WantParams args;
    args.SetParam("-functionName", AAFwk::String::Box("myFunc"));

    ToolUtil::NormalizeSkillParamKeys(args);

    EXPECT_EQ(args.GetStringParam("functionName"), "myFunc");
    EXPECT_TRUE(args.GetStringParam("-functionName").empty());

    TAG_LOGI(AAFwkTag::TEST, "NormalizeSkillParamKeys_0500 end");
}

/**
 * @tc.name: ToolUtil_ExpandArgsJsonString_0500
 * @tc.desc: Test ExpandArgsJsonString with reserved keys in JSON (should be skipped)
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilSkillTest, ToolUtil_ExpandArgsJsonString_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExpandArgsJsonString_0500 start");

    AAFwk::WantParams args;
    // Reserved keys should be skipped during expansion
    args.SetParam("args", AAFwk::String::Box(
        "{\"skillName\":\"ignored\",\"customParam\":\"kept\"}"));

    ToolUtil::ExpandArgsJsonString(args);

    EXPECT_TRUE(args.GetStringParam("skillName").empty());
    EXPECT_EQ(args.GetStringParam("customParam"), "kept");

    TAG_LOGI(AAFwkTag::TEST, "ExpandArgsJsonString_0500 end");
}

/**
 * @tc.name: ToolUtil_ExpandArgsJsonString_0600
 * @tc.desc: Test ExpandArgsJsonString with empty args string
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilSkillTest, ToolUtil_ExpandArgsJsonString_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExpandArgsJsonString_0600 start");

    AAFwk::WantParams args;
    args.SetParam("args", AAFwk::String::Box(""));

    ToolUtil::ExpandArgsJsonString(args);

    // Empty string should not cause issues
    EXPECT_TRUE(args.GetStringParam("args").empty());

    TAG_LOGI(AAFwkTag::TEST, "ExpandArgsJsonString_0600 end");
}

/**
 * @tc.name: ToolUtil_ExpandArgsJsonString_0700
 * @tc.desc: Test ExpandArgsJsonString with array value in JSON (not supported)
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilSkillTest, ToolUtil_ExpandArgsJsonString_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExpandArgsJsonString_0700 start");

    AAFwk::WantParams args;
    // Array values should be skipped by ExpandArgsFromJson
    args.SetParam("args", AAFwk::String::Box(
        "{\"name\":\"test\",\"items\":[1,2,3]}"));

    ToolUtil::ExpandArgsJsonString(args);

    EXPECT_EQ(args.GetStringParam("name"), "test");
    // Array "items" should not be expanded
    EXPECT_TRUE(args.GetStringParam("items").empty());

    TAG_LOGI(AAFwkTag::TEST, "ExpandArgsJsonString_0700 end");
}

/**
 * @tc.name: ToolUtil_FilterSkillArgs_0300
 * @tc.desc: Test FilterSkillArgs with empty args
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilSkillTest, ToolUtil_FilterSkillArgs_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FilterSkillArgs_0300 start");

    AAFwk::WantParams args;
    auto filtered = ToolUtil::FilterSkillArgs(args);
    ASSERT_NE(filtered, nullptr);
    EXPECT_TRUE(filtered->GetParams().empty());

    TAG_LOGI(AAFwkTag::TEST, "FilterSkillArgs_0300 end");
}

/**
 * @tc.name: ToolUtil_FilterSkillArgs_0400
 * @tc.desc: Test FilterSkillArgs with only reserved keys
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilSkillTest, ToolUtil_FilterSkillArgs_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FilterSkillArgs_0400 start");

    AAFwk::WantParams args;
    args.SetParam("bundleName", AAFwk::String::Box("com.test"));
    args.SetParam("moduleName", AAFwk::String::Box("entry"));
    args.SetParam("skillName", AAFwk::String::Box("test"));
    args.SetParam("scriptPath", AAFwk::String::Box("ets/test"));
    args.SetParam("functionName", AAFwk::String::Box("func"));

    auto filtered = ToolUtil::FilterSkillArgs(args);
    ASSERT_NE(filtered, nullptr);
    EXPECT_TRUE(filtered->GetParams().empty());

    TAG_LOGI(AAFwkTag::TEST, "FilterSkillArgs_0400 end");
}

/**
 * @tc.name: ToolUtil_BuildSkillSessionInfo_0400
 * @tc.desc: Test BuildSkillSessionInfo with empty sessionId
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilSkillTest, ToolUtil_BuildSkillSessionInfo_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "BuildSkillSessionInfo_0400 start");

    AppExecFwk::SkillExecuteResult skillResult;
    skillResult.code = 0;
    skillResult.result = std::make_shared<AAFwk::WantParams>();

    CliSessionInfo session = ToolUtil::BuildSkillSessionInfo("", ERR_OK, skillResult);
    EXPECT_EQ(session.sessionId, "");
    EXPECT_EQ(session.toolName, "ohos-arkTSScript");
    EXPECT_EQ(session.status, "completed");

    TAG_LOGI(AAFwkTag::TEST, "BuildSkillSessionInfo_0400 end");
}

/**
 * @tc.name: ToolUtil_BuildSkillSessionInfo_0500
 * @tc.desc: Test BuildSkillSessionInfo with result containing multiple params
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilSkillTest, ToolUtil_BuildSkillSessionInfo_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "BuildSkillSessionInfo_0500 start");

    AppExecFwk::SkillExecuteResult skillResult;
    skillResult.code = 0;
    skillResult.result = std::make_shared<AAFwk::WantParams>();
    skillResult.result->SetParam("key1", AAFwk::String::Box("val1"));
    skillResult.result->SetParam("key2", AAFwk::Integer::Box(99));

    CliSessionInfo session = ToolUtil::BuildSkillSessionInfo(SESSION_ID, ERR_OK, skillResult);
    ASSERT_NE(session.result, nullptr);
    EXPECT_EQ(session.result->exitCode, 0);
    EXPECT_FALSE(session.result->outputText.empty());

    TAG_LOGI(AAFwkTag::TEST, "BuildSkillSessionInfo_0500 end");
}

}  // namespace CliTool
}  // namespace OHOS
