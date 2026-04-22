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
#include <gmock/gmock.h>
#include <parcel.h>

#include "tool_info.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {

class ToolInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ToolInfoTest::SetUpTestCase(void) {}
void ToolInfoTest::TearDownTestCase(void) {}
void ToolInfoTest::SetUp() {}
void ToolInfoTest::TearDown() {}

// ==================== ArgMapping Tests ====================

/**
 * @tc.name: ArgMapping_Marshalling_0100
 * @tc.desc: Test ArgMapping Marshalling with FLAG type
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ArgMapping_Marshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ArgMapping_Marshalling_0100 start";

    ArgMapping mapping;
    mapping.type = ArgMappingType::FLAG;
    mapping.separator = " ";
    mapping.order = "";
    mapping.templates = R"({"key": "value"})";

    Parcel parcel;
    bool ret = mapping.Marshalling(parcel);

    EXPECT_TRUE(ret);

    GTEST_LOG_(INFO) << "ArgMapping_Marshalling_0100 end";
}

/**
 * @tc.name: ArgMapping_Marshalling_0200
 * @tc.desc: Test ArgMapping Marshalling with all types
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ArgMapping_Marshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ArgMapping_Marshalling_0200 start";

    std::vector<ArgMappingType> types = {
        ArgMappingType::FLAG,
        ArgMappingType::POSITIONAL,
        ArgMappingType::FLATTENED,
        ArgMappingType::JSONSTRING,
        ArgMappingType::MIXED
    };

    for (auto type : types) {
        ArgMapping mapping;
        mapping.type = type;
        mapping.separator = ",";
        mapping.order = "a,b,c";
        mapping.templates = "{}";

        Parcel parcel;
        bool ret = mapping.Marshalling(parcel);
        EXPECT_TRUE(ret);
    }

    GTEST_LOG_(INFO) << "ArgMapping_Marshalling_0200 end";
}

/**
 * @tc.name: ArgMapping_Unmarshalling_0100
 * @tc.desc: Test ArgMapping Unmarshalling success
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ArgMapping_Unmarshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ArgMapping_Unmarshalling_0100 start";

    ArgMapping original;
    original.type = ArgMappingType::POSITIONAL;
    original.separator = ",";
    original.order = "arg1,arg2";
    original.templates = R"({"target": "--target={value}"})";

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    ArgMapping *result = ArgMapping::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->type, ArgMappingType::POSITIONAL);
    EXPECT_EQ(result->separator, ",");
    EXPECT_EQ(result->order, "arg1,arg2");
    EXPECT_EQ(result->templates, R"({"target": "--target={value}"})");

    delete result;

    GTEST_LOG_(INFO) << "ArgMapping_Unmarshalling_0100 end";
}

/**
 * @tc.name: ArgMapping_Unmarshalling_0200
 * @tc.desc: Test ArgMapping Unmarshalling fail with empty parcel
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ArgMapping_Unmarshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ArgMapping_Unmarshalling_0200 start";

    Parcel parcel;
    ArgMapping *result = ArgMapping::Unmarshalling(parcel);

    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "ArgMapping_Unmarshalling_0200 end";
}

/**
 * @tc.name: ArgMapping_RoundTrip_0100
 * @tc.desc: Test ArgMapping Marshalling and Unmarshalling round trip
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ArgMapping_RoundTrip_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ArgMapping_RoundTrip_0100 start";

    ArgMapping original;
    original.type = ArgMappingType::MIXED;
    original.separator = "|";
    original.order = "x,y,z";
    original.templates = R"({"verbose": {"if_true": "-v", "if_false": ""}})";

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    ArgMapping *restored = ArgMapping::Unmarshalling(parcel);

    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->type, original.type);
    EXPECT_EQ(restored->separator, original.separator);
    EXPECT_EQ(restored->order, original.order);
    EXPECT_EQ(restored->templates, original.templates);

    delete restored;

    GTEST_LOG_(INFO) << "ArgMapping_RoundTrip_0100 end";
}

// ==================== SubCommandInfo Tests ====================

/**
 * @tc.name: SubCommandInfo_Marshalling_0100
 * @tc.desc: Test SubCommandInfo Marshalling with argMapping
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, SubCommandInfo_Marshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SubCommandInfo_Marshalling_0100 start";

    SubCommandInfo subCmd;
    subCmd.description = "Test subcommand";
    subCmd.requirePermissions = {"ohos.permission.INTERNET"};
    subCmd.inputSchema = "{}";
    subCmd.outputSchema = "{}";
    subCmd.argMapping = std::make_shared<ArgMapping>();
    subCmd.argMapping->type = ArgMappingType::FLAG;
    subCmd.eventTypes = {"stdout", "stderr"};
    subCmd.eventSchemas = "{}";

    Parcel parcel;
    bool ret = subCmd.Marshalling(parcel);

    EXPECT_TRUE(ret);

    GTEST_LOG_(INFO) << "SubCommandInfo_Marshalling_0100 end";
}

/**
 * @tc.name: SubCommandInfo_Marshalling_0200
 * @tc.desc: Test SubCommandInfo Marshalling without argMapping
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, SubCommandInfo_Marshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SubCommandInfo_Marshalling_0200 start";

    SubCommandInfo subCmd;
    subCmd.description = "Test subcommand without argMapping";
    subCmd.requirePermissions = {};
    subCmd.inputSchema = "{}";
    subCmd.outputSchema = "{}";
    subCmd.argMapping = nullptr;
    subCmd.eventTypes = {};
    subCmd.eventSchemas = "{}";

    Parcel parcel;
    bool ret = subCmd.Marshalling(parcel);

    EXPECT_TRUE(ret);

    GTEST_LOG_(INFO) << "SubCommandInfo_Marshalling_0200 end";
}

/**
 * @tc.name: SubCommandInfo_Unmarshalling_0100
 * @tc.desc: Test SubCommandInfo Unmarshalling with argMapping
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, SubCommandInfo_Unmarshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SubCommandInfo_Unmarshalling_0100 start";

    SubCommandInfo original;
    original.description = "Original subcommand";
    original.requirePermissions = {"ohos.permission.READ_STORAGE"};
    original.inputSchema = R"({"type": "object"})";
    original.outputSchema = R"({"type": "string"})";
    original.argMapping = std::make_shared<ArgMapping>();
    original.argMapping->type = ArgMappingType::JSONSTRING;
    original.argMapping->separator = "";
    original.argMapping->order = "";
    original.argMapping->templates = "{}";
    original.eventTypes = {"exit"};
    original.eventSchemas = R"({"exit": {"type": "object"}})";

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    SubCommandInfo *result = SubCommandInfo::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->description, "Original subcommand");
    EXPECT_EQ(result->requirePermissions.size(), 1u);
    EXPECT_TRUE(result->argMapping != nullptr);
    EXPECT_EQ(result->argMapping->type, ArgMappingType::JSONSTRING);

    delete result;

    GTEST_LOG_(INFO) << "SubCommandInfo_Unmarshalling_0100 end";
}

/**
 * @tc.name: SubCommandInfo_Unmarshalling_0200
 * @tc.desc: Test SubCommandInfo Unmarshalling without argMapping
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, SubCommandInfo_Unmarshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SubCommandInfo_Unmarshalling_0200 start";

    SubCommandInfo original;
    original.description = "No argMapping";
    original.requirePermissions = {};
    original.inputSchema = "{}";
    original.outputSchema = "{}";
    original.argMapping = nullptr;
    original.eventTypes = {};
    original.eventSchemas = "{}";

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    SubCommandInfo *result = SubCommandInfo::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->description, "No argMapping");
    EXPECT_TRUE(result->argMapping == nullptr);

    delete result;

    GTEST_LOG_(INFO) << "SubCommandInfo_Unmarshalling_0200 end";
}

/**
 * @tc.name: SubCommandInfo_Unmarshalling_0300
 * @tc.desc: Test SubCommandInfo Unmarshalling fail with empty parcel
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, SubCommandInfo_Unmarshalling_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SubCommandInfo_Unmarshalling_0300 start";

    Parcel parcel;
    SubCommandInfo *result = SubCommandInfo::Unmarshalling(parcel);

    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "SubCommandInfo_Unmarshalling_0300 end";
}

// ==================== ToolInfo Tests ====================

/**
 * @tc.name: ToolInfo_Marshalling_0100
 * @tc.desc: Test ToolInfo Marshalling with argMapping
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Marshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Marshalling_0100 start";

    ToolInfo tool;
    tool.name = "test_tool";
    tool.version = "1.0.0";
    tool.description = "Test tool";
    tool.executablePath = "/bin/test";
    tool.requirePermissions = {"ohos.permission.INTERNET"};
    tool.inputSchema = "{}";
    tool.outputSchema = "{}";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.argMapping->type = ArgMappingType::FLAG;
    tool.eventSchemas = "{}";
    tool.timeout = 30000;
    tool.eventTypes = {"stdout"};
    tool.registeredTime = 1234567890;
    tool.enabled = true;
    tool.hasSubCommand = false;
    tool.subcommands = "{}";

    Parcel parcel;
    bool ret = tool.Marshalling(parcel);

    EXPECT_TRUE(ret);

    GTEST_LOG_(INFO) << "ToolInfo_Marshalling_0100 end";
}

/**
 * @tc.name: ToolInfo_Marshalling_0200
 * @tc.desc: Test ToolInfo Marshalling without argMapping
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Marshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Marshalling_0200 start";

    ToolInfo tool;
    tool.name = "test_tool_no_arg";
    tool.version = "2.0.0";
    tool.description = "Tool without argMapping";
    tool.executablePath = "/bin/test2";
    tool.requirePermissions = {};
    tool.inputSchema = "{}";
    tool.outputSchema = "{}";
    tool.argMapping = nullptr;
    tool.eventSchemas = "{}";
    tool.timeout = 0;
    tool.eventTypes = {};
    tool.registeredTime = 0;
    tool.enabled = false;
    tool.hasSubCommand = true;
    tool.subcommands = R"({"sub1": {"description": "sub1"}})";

    Parcel parcel;
    bool ret = tool.Marshalling(parcel);

    EXPECT_TRUE(ret);

    GTEST_LOG_(INFO) << "ToolInfo_Marshalling_0200 end";
}

/**
 * @tc.name: ToolInfo_Unmarshalling_0100
 * @tc.desc: Test ToolInfo Unmarshalling with argMapping
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Unmarshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Unmarshalling_0100 start";

    ToolInfo original;
    original.name = "original_tool";
    original.version = "3.0.0";
    original.description = "Original tool description";
    original.executablePath = "/usr/bin/original";
    original.requirePermissions = {"ohos.permission.CAMERA", "ohos.permission.MICROPHONE"};
    original.inputSchema = R"({"type": "object", "properties": {"input": {"type": "string"}}})";
    original.outputSchema = R"({"type": "string"})";
    original.argMapping = std::make_shared<ArgMapping>();
    original.argMapping->type = ArgMappingType::POSITIONAL;
    original.argMapping->separator = "";
    original.argMapping->order = "arg1,arg2,arg3";
    original.argMapping->templates = "{}";
    original.eventSchemas = "{}";
    original.timeout = 60000;
    original.eventTypes = {"stdout", "stderr", "exit"};
    original.registeredTime = 9876543210;
    original.enabled = true;
    original.hasSubCommand = true;
    original.subcommands = R"({"build": {"description": "Build subcommand"}})";

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    ToolInfo *result = ToolInfo::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->name, "original_tool");
    EXPECT_EQ(result->version, "3.0.0");
    EXPECT_EQ(result->requirePermissions.size(), 2u);
    EXPECT_TRUE(result->argMapping != nullptr);
    EXPECT_EQ(result->argMapping->type, ArgMappingType::POSITIONAL);
    EXPECT_EQ(result->timeout, 60000);
    EXPECT_TRUE(result->enabled);
    EXPECT_TRUE(result->hasSubCommand);

    delete result;

    GTEST_LOG_(INFO) << "ToolInfo_Unmarshalling_0100 end";
}

/**
 * @tc.name: ToolInfo_Unmarshalling_0200
 * @tc.desc: Test ToolInfo Unmarshalling without argMapping
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Unmarshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Unmarshalling_0200 start";

    ToolInfo original;
    original.name = "simple_tool";
    original.version = "1.0.0";
    original.description = "Simple tool";
    original.executablePath = "/bin/simple";
    original.requirePermissions = {};
    original.inputSchema = "{}";
    original.outputSchema = "{}";
    original.argMapping = nullptr;
    original.eventSchemas = "{}";
    original.timeout = 0;
    original.eventTypes = {};
    original.registeredTime = 0;
    original.enabled = false;
    original.hasSubCommand = false;
    original.subcommands = "{}";

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    ToolInfo *result = ToolInfo::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->name, "simple_tool");
    EXPECT_TRUE(result->argMapping == nullptr);
    EXPECT_FALSE(result->enabled);
    EXPECT_FALSE(result->hasSubCommand);

    delete result;

    GTEST_LOG_(INFO) << "ToolInfo_Unmarshalling_0200 end";
}

/**
 * @tc.name: ToolInfo_Unmarshalling_0300
 * @tc.desc: Test ToolInfo Unmarshalling fail with empty parcel
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Unmarshalling_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Unmarshalling_0300 start";

    Parcel parcel;
    ToolInfo *result = ToolInfo::Unmarshalling(parcel);

    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "ToolInfo_Unmarshalling_0300 end";
}

// ==================== ExecOptions Tests ====================

/**
 * @tc.name: ExecOptions_Marshalling_0100
 * @tc.desc: Test ExecOptions Marshalling success
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ExecOptions_Marshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExecOptions_Marshalling_0100 start";

    ExecOptions options;
    options.background = true;
    options.yieldMs = 5000;
    options.timeout = 60000;
    options.workingDir = "/tmp/work";

    Parcel parcel;
    bool ret = options.Marshalling(parcel);

    EXPECT_TRUE(ret);

    GTEST_LOG_(INFO) << "ExecOptions_Marshalling_0100 end";
}

/**
 * @tc.name: ExecOptions_Unmarshalling_0100
 * @tc.desc: Test ExecOptions Unmarshalling success
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ExecOptions_Unmarshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExecOptions_Unmarshalling_0100 start";

    ExecOptions original;
    original.background = false;
    original.yieldMs = 30000;
    original.timeout = 120000;
    original.workingDir = "/home/user/workspace";

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    ExecOptions *result = ExecOptions::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->background, false);
    EXPECT_EQ(result->yieldMs, 30000);
    EXPECT_EQ(result->timeout, 120000);
    EXPECT_EQ(result->workingDir, "/home/user/workspace");

    delete result;

    GTEST_LOG_(INFO) << "ExecOptions_Unmarshalling_0100 end";
}

/**
 * @tc.name: ExecOptions_Unmarshalling_0200
 * @tc.desc: Test ExecOptions Unmarshalling fail with empty parcel
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ExecOptions_Unmarshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExecOptions_Unmarshalling_0200 start";

    Parcel parcel;
    ExecOptions *result = ExecOptions::Unmarshalling(parcel);

    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "ExecOptions_Unmarshalling_0200 end";
}

// ==================== ExecResult Tests ====================

/**
 * @tc.name: ExecResult_Marshalling_0100
 * @tc.desc: Test ExecResult Marshalling success
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ExecResult_Marshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExecResult_Marshalling_0100 start";

    ExecResult result;
    result.exitCode = 0;
    result.outputText = "Success output";
    result.errorText = "";
    result.signalNumber = 0;
    result.timedOut = false;
    result.executionTime = 1234;

    Parcel parcel;
    bool ret = result.Marshalling(parcel);

    EXPECT_TRUE(ret);

    GTEST_LOG_(INFO) << "ExecResult_Marshalling_0100 end";
}

/**
 * @tc.name: ExecResult_Marshalling_0200
 * @tc.desc: Test ExecResult Marshalling with error
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ExecResult_Marshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExecResult_Marshalling_0200 start";

    ExecResult result;
    result.exitCode = 1;
    result.outputText = "";
    result.errorText = "Error occurred";
    result.signalNumber = 9;
    result.timedOut = true;
    result.executionTime = 5000;

    Parcel parcel;
    bool ret = result.Marshalling(parcel);

    EXPECT_TRUE(ret);

    GTEST_LOG_(INFO) << "ExecResult_Marshalling_0200 end";
}

/**
 * @tc.name: ExecResult_Unmarshalling_0100
 * @tc.desc: Test ExecResult Unmarshalling success
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ExecResult_Unmarshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExecResult_Unmarshalling_0100 start";

    ExecResult original;
    original.exitCode = 127;
    original.outputText = "Command output\nMultiple lines";
    original.errorText = "Command error";
    original.signalNumber = 15;
    original.timedOut = true;
    original.executionTime = 99999;

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    ExecResult *result = ExecResult::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->exitCode, 127);
    EXPECT_EQ(result->outputText, "Command output\nMultiple lines");
    EXPECT_EQ(result->errorText, "Command error");
    EXPECT_EQ(result->signalNumber, 15);
    EXPECT_TRUE(result->timedOut);
    EXPECT_EQ(result->executionTime, 99999);

    delete result;

    GTEST_LOG_(INFO) << "ExecResult_Unmarshalling_0100 end";
}

/**
 * @tc.name: ExecResult_Unmarshalling_0200
 * @tc.desc: Test ExecResult Unmarshalling fail with empty parcel
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ExecResult_Unmarshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExecResult_Unmarshalling_0200 start";

    Parcel parcel;
    ExecResult *result = ExecResult::Unmarshalling(parcel);

    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "ExecResult_Unmarshalling_0200 end";
}

// ==================== SessionInfo Tests ====================

/**
 * @tc.name: SessionInfo_Marshalling_0100
 * @tc.desc: Test SessionInfo Marshalling with result
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, SessionInfo_Marshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionInfo_Marshalling_0100 start";

    SessionInfo session;
    session.sessionId = "session-123";
    session.toolName = "test_tool";
    session.status = "completed";
    session.startTime = 1000000;
    session.endTime = 2000000;
    session.result = std::make_shared<ExecResult>();
    session.result->exitCode = 0;
    session.result->outputText = "Done";
    session.result->errorText = "";
    session.result->signalNumber = 0;
    session.result->timedOut = false;
    session.result->executionTime = 1000000;

    Parcel parcel;
    bool ret = session.Marshalling(parcel);

    EXPECT_TRUE(ret);

    GTEST_LOG_(INFO) << "SessionInfo_Marshalling_0100 end";
}

/**
 * @tc.name: SessionInfo_Marshalling_0200
 * @tc.desc: Test SessionInfo Marshalling without result
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, SessionInfo_Marshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionInfo_Marshalling_0200 start";

    SessionInfo session;
    session.sessionId = "session-456";
    session.toolName = "running_tool";
    session.status = "running";
    session.startTime = 3000000;
    session.endTime = 0;
    session.result = nullptr;

    Parcel parcel;
    bool ret = session.Marshalling(parcel);

    EXPECT_TRUE(ret);

    GTEST_LOG_(INFO) << "SessionInfo_Marshalling_0200 end";
}

/**
 * @tc.name: SessionInfo_Unmarshalling_0100
 * @tc.desc: Test SessionInfo Unmarshalling with result
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, SessionInfo_Unmarshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionInfo_Unmarshalling_0100 start";

    SessionInfo original;
    original.sessionId = "session-789";
    original.toolName = "my_tool";
    original.status = "failed";
    original.startTime = 5000000;
    original.endTime = 6000000;
    original.result = std::make_shared<ExecResult>();
    original.result->exitCode = 1;
    original.result->outputText = "";
    original.result->errorText = "Failed with error";
    original.result->signalNumber = 0;
    original.result->timedOut = false;
    original.result->executionTime = 1000000;

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    SessionInfo *result = SessionInfo::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->sessionId, "session-789");
    EXPECT_EQ(result->toolName, "my_tool");
    EXPECT_EQ(result->status, "failed");
    EXPECT_TRUE(result->result != nullptr);
    EXPECT_EQ(result->result->exitCode, 1);

    delete result;

    GTEST_LOG_(INFO) << "SessionInfo_Unmarshalling_0100 end";
}

/**
 * @tc.name: SessionInfo_Unmarshalling_0200
 * @tc.desc: Test SessionInfo Unmarshalling without result
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, SessionInfo_Unmarshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionInfo_Unmarshalling_0200 start";

    SessionInfo original;
    original.sessionId = "session-running";
    original.toolName = "async_tool";
    original.status = "running";
    original.startTime = 7000000;
    original.endTime = 0;
    original.result = nullptr;

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    SessionInfo *result = SessionInfo::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->sessionId, "session-running");
    EXPECT_TRUE(result->result == nullptr);

    delete result;

    GTEST_LOG_(INFO) << "SessionInfo_Unmarshalling_0200 end";
}

/**
 * @tc.name: SessionInfo_Unmarshalling_0300
 * @tc.desc: Test SessionInfo Unmarshalling fail with empty parcel
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, SessionInfo_Unmarshalling_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionInfo_Unmarshalling_0300 start";

    Parcel parcel;
    SessionInfo *result = SessionInfo::Unmarshalling(parcel);

    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "SessionInfo_Unmarshalling_0300 end";
}

// ==================== ToolEvent Tests ====================

/**
 * @tc.name: ToolEvent_Marshalling_0100
 * @tc.desc: Test ToolEvent Marshalling success
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolEvent_Marshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolEvent_Marshalling_0100 start";

    ToolEvent event;
    event.type = "stdout";
    event.eventData = "Output line 1\nOutput line 2";
    event.exitCode = 0;
    event.timestamp = 1234567890;

    Parcel parcel;
    bool ret = event.Marshalling(parcel);

    EXPECT_TRUE(ret);

    GTEST_LOG_(INFO) << "ToolEvent_Marshalling_0100 end";
}

/**
 * @tc.name: ToolEvent_Marshalling_0200
 * @tc.desc: Test ToolEvent Marshalling with different event types
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolEvent_Marshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolEvent_Marshalling_0200 start";

    std::vector<std::string> eventTypes = {"stdout", "stderr", "exit", "error"};

    for (const auto &type : eventTypes) {
        ToolEvent event;
        event.type = type;
        event.eventData = "Event data for " + type;
        event.exitCode = (type == "exit") ? 0 : 0;
        event.timestamp = 1000000;

        Parcel parcel;
        bool ret = event.Marshalling(parcel);
        EXPECT_TRUE(ret);
    }

    GTEST_LOG_(INFO) << "ToolEvent_Marshalling_0200 end";
}

/**
 * @tc.name: ToolEvent_Unmarshalling_0100
 * @tc.desc: Test ToolEvent Unmarshalling success
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolEvent_Unmarshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolEvent_Unmarshalling_0100 start";

    ToolEvent original;
    original.type = "exit";
    original.eventData = R"({"code": 0, "message": "success"})";
    original.exitCode = 0;
    original.timestamp = 9876543210;

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    ToolEvent *result = ToolEvent::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->type, "exit");
    EXPECT_EQ(result->eventData, R"({"code": 0, "message": "success"})");
    EXPECT_EQ(result->exitCode, 0);
    EXPECT_EQ(result->timestamp, 9876543210);

    delete result;

    GTEST_LOG_(INFO) << "ToolEvent_Unmarshalling_0100 end";
}

/**
 * @tc.name: ToolEvent_Unmarshalling_0200
 * @tc.desc: Test ToolEvent Unmarshalling fail with empty parcel
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolEvent_Unmarshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolEvent_Unmarshalling_0200 start";

    Parcel parcel;
    ToolEvent *result = ToolEvent::Unmarshalling(parcel);

    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "ToolEvent_Unmarshalling_0200 end";
}

// ==================== SessionStatus Tests ====================

/**
 * @tc.name: SessionStatus_Marshalling_0100
 * @tc.desc: Test SessionStatus Marshalling with result
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, SessionStatus_Marshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionStatus_Marshalling_0100 start";

    SessionStatus status;
    status.sessionId = "status-123";
    status.toolName = "completed_tool";
    status.status = "completed";
    status.startTime = 1000000;
    status.endTime = 2000000;
    status.result = std::make_shared<ExecResult>();
    status.result->exitCode = 0;
    status.result->outputText = "Completed successfully";
    status.result->errorText = "";
    status.result->signalNumber = 0;
    status.result->timedOut = false;
    status.result->executionTime = 1000000;

    Parcel parcel;
    bool ret = status.Marshalling(parcel);

    EXPECT_TRUE(ret);

    GTEST_LOG_(INFO) << "SessionStatus_Marshalling_0100 end";
}

/**
 * @tc.name: SessionStatus_Marshalling_0200
 * @tc.desc: Test SessionStatus Marshalling without result
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, SessionStatus_Marshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionStatus_Marshalling_0200 start";

    SessionStatus status;
    status.sessionId = "status-456";
    status.toolName = "running_tool";
    status.status = "running";
    status.startTime = 3000000;
    status.endTime = 0;
    status.result = nullptr;

    Parcel parcel;
    bool ret = status.Marshalling(parcel);

    EXPECT_TRUE(ret);

    GTEST_LOG_(INFO) << "SessionStatus_Marshalling_0200 end";
}

/**
 * @tc.name: SessionStatus_Unmarshalling_0100
 * @tc.desc: Test SessionStatus Unmarshalling with result
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, SessionStatus_Unmarshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionStatus_Unmarshalling_0100 start";

    SessionStatus original;
    original.sessionId = "status-789";
    original.toolName = "failed_tool";
    original.status = "failed";
    original.startTime = 5000000;
    original.endTime = 6000000;
    original.result = std::make_shared<ExecResult>();
    original.result->exitCode = 137;
    original.result->outputText = "";
    original.result->errorText = "Process killed";
    original.result->signalNumber = 9;
    original.result->timedOut = false;
    original.result->executionTime = 1000000;

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    SessionStatus *result = SessionStatus::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->sessionId, "status-789");
    EXPECT_EQ(result->status, "failed");
    EXPECT_TRUE(result->result != nullptr);
    EXPECT_EQ(result->result->exitCode, 137);
    EXPECT_EQ(result->result->signalNumber, 9);

    delete result;

    GTEST_LOG_(INFO) << "SessionStatus_Unmarshalling_0100 end";
}

/**
 * @tc.name: SessionStatus_Unmarshalling_0200
 * @tc.desc: Test SessionStatus Unmarshalling without result
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, SessionStatus_Unmarshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionStatus_Unmarshalling_0200 start";

    SessionStatus original;
    original.sessionId = "status-running";
    original.toolName = "async_tool";
    original.status = "running";
    original.startTime = 7000000;
    original.endTime = 0;
    original.result = nullptr;

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    SessionStatus *result = SessionStatus::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->sessionId, "status-running");
    EXPECT_TRUE(result->result == nullptr);

    delete result;

    GTEST_LOG_(INFO) << "SessionStatus_Unmarshalling_0200 end";
}

/**
 * @tc.name: SessionStatus_Unmarshalling_0300
 * @tc.desc: Test SessionStatus Unmarshalling fail with empty parcel
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, SessionStatus_Unmarshalling_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionStatus_Unmarshalling_0300 start";

    Parcel parcel;
    SessionStatus *result = SessionStatus::Unmarshalling(parcel);

    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "SessionStatus_Unmarshalling_0300 end";
}

// ==================== ToolsRawData Tests ====================

/**
 * @tc.name: ToolsRawData_Marshalling_0100
 * @tc.desc: Test ToolsRawData Marshalling success
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolsRawData_Marshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolsRawData_Marshalling_0100 start";

    ToolsRawData rawData;
    rawData.data = {1, 2, 3, 4, 5};

    Parcel parcel;
    bool ret = rawData.Marshalling(parcel);

    EXPECT_TRUE(ret);

    GTEST_LOG_(INFO) << "ToolsRawData_Marshalling_0100 end";
}

/**
 * @tc.name: ToolsRawData_Marshalling_0200
 * @tc.desc: Test ToolsRawData Marshalling with empty data
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolsRawData_Marshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolsRawData_Marshalling_0200 start";

    ToolsRawData rawData;
    rawData.data = {};

    Parcel parcel;
    bool ret = rawData.Marshalling(parcel);

    EXPECT_TRUE(ret);

    GTEST_LOG_(INFO) << "ToolsRawData_Marshalling_0200 end";
}

/**
 * @tc.name: ToolsRawData_Unmarshalling_0100
 * @tc.desc: Test ToolsRawData Unmarshalling success
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolsRawData_Unmarshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolsRawData_Unmarshalling_0100 start";

    ToolsRawData original;
    original.data = {10, 20, 30, 40, 50, 60};

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    ToolsRawData *result = ToolsRawData::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->data.size(), 6u);
    EXPECT_EQ(result->data[0], 10u);
    EXPECT_EQ(result->data[5], 60u);

    delete result;

    GTEST_LOG_(INFO) << "ToolsRawData_Unmarshalling_0100 end";
}

/**
 * @tc.name: ToolsRawData_Unmarshalling_0200
 * @tc.desc: Test ToolsRawData Unmarshalling with empty data
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolsRawData_Unmarshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolsRawData_Unmarshalling_0200 start";

    ToolsRawData original;
    original.data = {};

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    ToolsRawData *result = ToolsRawData::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->data.size(), 0u);

    delete result;

    GTEST_LOG_(INFO) << "ToolsRawData_Unmarshalling_0200 end";
}

} // namespace CliTool
} // namespace OHOS
