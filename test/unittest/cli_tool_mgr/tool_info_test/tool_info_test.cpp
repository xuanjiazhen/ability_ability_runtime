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
#include <nlohmann/json.hpp>
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
    tool.hasSubCommand = false;

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
    tool.hasSubCommand = true;
    SubCommandInfo subCmd;
    subCmd.description = "sub1";
    tool.subcommands["sub1"] = subCmd;

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
    original.hasSubCommand = true;
    SubCommandInfo buildSubCmd;
    buildSubCmd.description = "Build subcommand";
    original.subcommands["build"] = buildSubCmd;

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
    EXPECT_TRUE(result->hasSubCommand);
    EXPECT_EQ(result->subcommands.size(), 1u);

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
    original.hasSubCommand = false;

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    ToolInfo *result = ToolInfo::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->name, "simple_tool");
    EXPECT_TRUE(result->argMapping == nullptr);
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

// ==================== ToolInfo ParseToJson Tests ====================

/**
 * @tc.name: ToolInfo_ParseToJson_0100
 * @tc.desc: Test ToolInfo ParseToJson with full data
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseToJson_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0100 start";

    ToolInfo tool;
    tool.name = "json_tool";
    tool.version = "1.0.0";
    tool.description = "JSON test tool";
    tool.executablePath = "/bin/json";
    tool.requirePermissions = {"ohos.permission.INTERNET"};
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.argMapping->type = ArgMappingType::FLAG;
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 30000;
    tool.eventTypes = {"stdout", "stderr"};
    tool.hasSubCommand = false;

    nlohmann::json json = tool.ParseToJson();

    EXPECT_EQ(json["name"], "json_tool");
    EXPECT_EQ(json["version"], "1.0.0");
    EXPECT_EQ(json["description"], "JSON test tool");
    EXPECT_EQ(json["executablePath"], "/bin/json");
    EXPECT_EQ(json["timeout"], 30000);
    EXPECT_TRUE(json.contains("argMapping"));

    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0100 end";
}

/**
 * @tc.name: ToolInfo_ParseToJson_0200
 * @tc.desc: Test ToolInfo ParseToJson without argMapping
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseToJson_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0200 start";

    ToolInfo tool;
    tool.name = "no_arg_tool";
    tool.argMapping = nullptr;

    nlohmann::json json = tool.ParseToJson();

    EXPECT_EQ(json["name"], "no_arg_tool");
    EXPECT_FALSE(json.contains("argMapping"));

    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0200 end";
}

/**
 * @tc.name: ToolInfo_ParseToJson_0300
 * @tc.desc: Test ToolInfo ParseToJson with subcommands
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseToJson_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0300 start";

    ToolInfo tool;
    tool.name = "tool_with_subs";
    tool.hasSubCommand = true;

    SubCommandInfo subCmd;
    subCmd.description = "Test subcommand";
    subCmd.inputSchema = R"({"type": "object"})";
    tool.subcommands["sub1"] = subCmd;

    nlohmann::json json = tool.ParseToJson();

    EXPECT_TRUE(json.contains("subcommands"));
    EXPECT_TRUE(json["subcommands"].contains("sub1"));
    EXPECT_EQ(json["subcommands"]["sub1"]["description"], "Test subcommand");

    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0300 end";
}

// ==================== ToolInfo ParseFromJson Tests ====================

/**
 * @tc.name: ToolInfo_ParseFromJson_0100
 * @tc.desc: Test ToolInfo ParseFromJson with full data
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0100 start";

    nlohmann::json json = R"({
        "name": "parsed_tool",
        "version": "2.0.0",
        "description": "Parsed from JSON",
        "executablePath": "/bin/parsed",
        "requirePermissions": ["ohos.permission.CAMERA"],
        "inputSchema": {"type": "object"},
        "outputSchema": {"type": "array"},
        "argMapping": {"type": "positional", "order": "arg1,arg2"},
        "eventSchemas": {"exit": {"type": "number"}},
        "timeout": 60000,
        "eventTypes": ["stdout", "exit"],
        "hasSubCommand": false
    })"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.name, "parsed_tool");
    EXPECT_EQ(tool.version, "2.0.0");
    EXPECT_EQ(tool.description, "Parsed from JSON");
    EXPECT_EQ(tool.executablePath, "/bin/parsed");
    EXPECT_EQ(tool.requirePermissions.size(), 1u);
    EXPECT_EQ(tool.timeout, 60000);
    EXPECT_FALSE(tool.hasSubCommand);
    ASSERT_NE(tool.argMapping, nullptr);
    EXPECT_EQ(tool.argMapping->type, ArgMappingType::POSITIONAL);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0100 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_0200
 * @tc.desc: Test ToolInfo ParseFromJson with subcommands
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0200 start";

    nlohmann::json json = R"({
        "name": "tool_with_subcommands",
        "version": "1.0.0",
        "hasSubCommand": true,
        "subcommands": {
            "build": {
                "description": "Build the project",
                "inputSchema": {"type": "object"},
                "outputSchema": {"type": "string"}
            },
            "run": {
                "description": "Run the project"
            }
        }
    })"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.name, "tool_with_subcommands");
    EXPECT_TRUE(tool.hasSubCommand);
    EXPECT_EQ(tool.subcommands.size(), 2u);
    EXPECT_EQ(tool.subcommands["build"].description, "Build the project");

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0200 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_0300
 * @tc.desc: Test ToolInfo ParseFromJson with empty JSON
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0300 start";

    nlohmann::json json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_TRUE(tool.name.empty());
    EXPECT_TRUE(tool.version.empty());
    EXPECT_EQ(tool.argMapping, nullptr);
    EXPECT_FALSE(tool.hasSubCommand);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0300 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_0400
 * @tc.desc: Test ToolInfo ParseFromJson with non-string array elements
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0400 start";

    nlohmann::json json = R"({
        "name": "test_tool",
        "requirePermissions": ["ohos.permission.INTERNET", 123, true, {"key": "value"}],
        "eventTypes": ["stdout", 456, false, null]
    })"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.name, "test_tool");
    EXPECT_EQ(tool.requirePermissions.size(), 1u);
    EXPECT_EQ(tool.requirePermissions[0], "ohos.permission.INTERNET");
    EXPECT_EQ(tool.eventTypes.size(), 1u);
    EXPECT_EQ(tool.eventTypes[0], "stdout");

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0400 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_0500
 * @tc.desc: Test ParseFromJson with name field only
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0500 start";

    nlohmann::json json = R"({"name": "only_name"})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.name, "only_name");
    EXPECT_TRUE(tool.version.empty());
    EXPECT_TRUE(tool.description.empty());
    EXPECT_TRUE(tool.executablePath.empty());
    EXPECT_TRUE(tool.requirePermissions.empty());
    EXPECT_TRUE(tool.inputSchema.empty());
    EXPECT_TRUE(tool.outputSchema.empty());
    EXPECT_EQ(tool.argMapping, nullptr);
    EXPECT_TRUE(tool.eventSchemas.empty());
    EXPECT_EQ(tool.timeout, 30);  // default value
    EXPECT_TRUE(tool.eventTypes.empty());
    EXPECT_FALSE(tool.hasSubCommand);
    EXPECT_TRUE(tool.subcommands.empty());

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0500 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_0600
 * @tc.desc: Test ParseFromJson with name as non-string
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0600 start";

    nlohmann::json json = R"({"name": 12345})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_TRUE(tool.name.empty());  // should not be set

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0600 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_0700
 * @tc.desc: Test ParseFromJson with version as non-string
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0700 start";

    nlohmann::json json = R"({"version": ["1.0.0"]})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_TRUE(tool.version.empty());

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0700 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_0800
 * @tc.desc: Test ParseFromJson with description as non-string
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0800 start";

    nlohmann::json json = R"({"description": {"text": "description"}})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_TRUE(tool.description.empty());

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0800 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_0900
 * @tc.desc: Test ParseFromJson with executablePath as non-string
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0900 start";

    nlohmann::json json = R"({"executablePath": null})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_TRUE(tool.executablePath.empty());

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0900 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_1000
 * @tc.desc: Test ParseFromJson with requirePermissions as non-array
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_1000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_1000 start";

    nlohmann::json json = R"({"requirePermissions": "ohos.permission.INTERNET"})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_TRUE(tool.requirePermissions.empty());

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_1000 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_1100
 * @tc.desc: Test ParseFromJson with inputSchema as non-object
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_1100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_1100 start";

    nlohmann::json json = R"({"inputSchema": "not_an_object"})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_TRUE(tool.inputSchema.empty());

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_1100 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_1200
 * @tc.desc: Test ParseFromJson with outputSchema as non-object
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_1200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_1200 start";

    nlohmann::json json = R"({"outputSchema": 123})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_TRUE(tool.outputSchema.empty());

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_1200 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_1300
 * @tc.desc: Test ParseFromJson with argMapping as non-object
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_1300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_1300 start";

    nlohmann::json json = R"({"argMapping": "not_an_object"})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.argMapping, nullptr);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_1300 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_1400
 * @tc.desc: Test ParseFromJson with eventSchemas as non-object
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_1400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_1400 start";

    nlohmann::json json = R"({"eventSchemas": "not_an_object"})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_TRUE(tool.eventSchemas.empty());

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_1400 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_1500
 * @tc.desc: Test ParseFromJson with timeout as non-number
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_1500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_1500 start";

    nlohmann::json json = R"({"timeout": "30"})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.timeout, 30);  // default value

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_1500 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_1600
 * @tc.desc: Test ParseFromJson with eventTypes as non-array
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_1600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_1600 start";

    nlohmann::json json = R"({"eventTypes": "stdout"})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_TRUE(tool.eventTypes.empty());

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_1600 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_1700
 * @tc.desc: Test ParseFromJson with hasSubCommand as non-boolean
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_1700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_1700 start";

    nlohmann::json json = R"({"hasSubCommand": "true"})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_FALSE(tool.hasSubCommand);  // default value

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_1700 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_1800
 * @tc.desc: Test ParseFromJson with subcommands as non-object
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_1800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_1800 start";

    nlohmann::json json = R"({"subcommands": "build"})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_TRUE(tool.subcommands.empty());

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_1800 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_1850
 * @tc.desc: Test ParseFromJson with hasSubCommand false but subcommands present
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_1850, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_1850 start";

    nlohmann::json json = R"({
        "name": "test_tool",
        "hasSubCommand": false,
        "subcommands": {
            "build": {
                "description": "Build subcommand"
            }
        }
    })"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.name, "test_tool");
    EXPECT_FALSE(tool.hasSubCommand);
    EXPECT_TRUE(tool.subcommands.empty());  // should not parse subcommands when hasSubCommand is false

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_1850 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_1900
 * @tc.desc: Test ParseFromJson with empty array for requirePermissions
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_1900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_1900 start";

    nlohmann::json json = R"({"requirePermissions": []})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.requirePermissions.size(), 0u);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_1900 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_2000
 * @tc.desc: Test ParseFromJson with empty array for eventTypes
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_2000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_2000 start";

    nlohmann::json json = R"({"eventTypes": []})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.eventTypes.size(), 0u);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_2000 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_2100
 * @tc.desc: Test ParseFromJson with timeout boundary value (0)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_2100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_2100 start";

    nlohmann::json json = R"({"timeout": 0})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.timeout, 0);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_2100 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_2200
 * @tc.desc: Test ParseFromJson with timeout boundary value (max int32)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_2200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_2200 start";

    nlohmann::json json = R"({"timeout": 2147483647})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.timeout, 2147483647);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_2200 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_2300
 * @tc.desc: Test ParseFromJson with timeout negative value
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_2300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_2300 start";

    nlohmann::json json = R"({"timeout": -1})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.timeout, -1);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_2300 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_2400
 * @tc.desc: Test ParseFromJson with float timeout
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_2400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_2400 start";

    nlohmann::json json = R"({"timeout": 30.5})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.timeout, 30);  // truncated to int

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_2400 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_2500
 * @tc.desc: Test ParseFromJson with inputSchema as empty object
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_2500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_2500 start";

    nlohmann::json json = R"({"inputSchema": {}})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.inputSchema, "{}");

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_2500 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_2600
 * @tc.desc: Test ParseFromJson with outputSchema as empty object
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_2600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_2600 start";

    nlohmann::json json = R"({"outputSchema": {}})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.outputSchema, "{}");

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_2600 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_2700
 * @tc.desc: Test ParseFromJson with eventSchemas as empty object
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_2700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_2700 start";

    nlohmann::json json = R"({"eventSchemas": {}})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.eventSchemas, "{}");

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_2700 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_2800
 * @tc.desc: Test ParseFromJson with complex nested inputSchema
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_2800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_2800 start";

    nlohmann::json json = R"({
        "inputSchema": {
            "type": "object",
            "properties": {
                "name": {"type": "string"},
                "age": {"type": "number"},
                "enabled": {"type": "boolean"}
            },
            "required": ["name"]
        }
    })"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    nlohmann::json parsed = nlohmann::json::parse(tool.inputSchema);
    EXPECT_EQ(parsed["type"], "object");
    EXPECT_TRUE(parsed.contains("properties"));

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_2800 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_2900
 * @tc.desc: Test ParseFromJson with multiple subcommands
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_2900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_2900 start";

    nlohmann::json json = R"({
        "name": "multi_sub_tool",
        "hasSubCommand": true,
        "subcommands": {
            "build": {
                "description": "Build the project",
                "requirePermissions": ["p1"]
            },
            "clean": {
                "description": "Clean build artifacts",
                "requirePermissions": []
            },
            "install": {
                "description": "Install the app"
            }
        }
    })"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.name, "multi_sub_tool");
    EXPECT_TRUE(tool.hasSubCommand);
    EXPECT_EQ(tool.subcommands.size(), 3u);
    EXPECT_EQ(tool.subcommands["build"].description, "Build the project");
    EXPECT_EQ(tool.subcommands["clean"].description, "Clean build artifacts");
    EXPECT_EQ(tool.subcommands["install"].description, "Install the app");
    EXPECT_EQ(tool.subcommands["build"].requirePermissions.size(), 1u);
    EXPECT_EQ(tool.subcommands["clean"].requirePermissions.size(), 0u);
    EXPECT_EQ(tool.subcommands["install"].requirePermissions.size(), 0u);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_2900 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_3000
 * @tc.desc: Test ParseFromJson with all fields
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_3000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_3000 start";

    nlohmann::json json = R"({
        "name": "full_tool",
        "version": "1.0.0",
        "description": "A complete tool description",
        "executablePath": "/usr/bin/full_tool",
        "requirePermissions": ["perm1", "perm2", "perm3"],
        "inputSchema": {"type": "object"},
        "outputSchema": {"type": "string"},
        "argMapping": {"type": "flag", "separator": "--"},
        "eventSchemas": {"stdout": {"type": "string"}},
        "timeout": 60000,
        "eventTypes": ["stdout", "stderr", "exit"],
        "hasSubCommand": true,
        "subcommands": {
            "run": {"description": "Run the tool"}
        }
    })"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.name, "full_tool");
    EXPECT_EQ(tool.version, "1.0.0");
    EXPECT_EQ(tool.description, "A complete tool description");
    EXPECT_EQ(tool.executablePath, "/usr/bin/full_tool");
    EXPECT_EQ(tool.requirePermissions.size(), 3u);
    EXPECT_EQ(tool.requirePermissions[0], "perm1");
    EXPECT_EQ(tool.requirePermissions[1], "perm2");
    EXPECT_EQ(tool.requirePermissions[2], "perm3");
    EXPECT_FALSE(tool.inputSchema.empty());
    EXPECT_FALSE(tool.outputSchema.empty());
    ASSERT_NE(tool.argMapping, nullptr);
    EXPECT_EQ(tool.argMapping->type, ArgMappingType::FLAG);
    EXPECT_EQ(tool.argMapping->separator, "--");
    EXPECT_FALSE(tool.eventSchemas.empty());
    EXPECT_EQ(tool.timeout, 60000);
    EXPECT_EQ(tool.eventTypes.size(), 3u);
    EXPECT_TRUE(tool.hasSubCommand);
    EXPECT_EQ(tool.subcommands.size(), 1u);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_3000 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_3100
 * @tc.desc: Test ParseFromJson with subcommand containing all fields
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_3100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_3100 start";

    nlohmann::json json = R"({
        "subcommands": {
            "fullSub": {
                "description": "A complete subcommand",
                "requirePermissions": ["sub_perm1"],
                "inputSchema": {"type": "array"},
                "outputSchema": {"type": "boolean"},
                "argMapping": {"type": "positional"},
                "eventTypes": ["event1", "event2"],
                "eventSchemas": {"event1": {"type": "object"}}
            }
        }
    })"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.subcommands.size(), 1u);
    auto& subCmd = tool.subcommands["fullSub"];
    EXPECT_EQ(subCmd.description, "A complete subcommand");
    EXPECT_EQ(subCmd.requirePermissions.size(), 1u);
    EXPECT_EQ(subCmd.requirePermissions[0], "sub_perm1");
    EXPECT_FALSE(subCmd.inputSchema.empty());
    EXPECT_FALSE(subCmd.outputSchema.empty());
    ASSERT_NE(subCmd.argMapping, nullptr);
    EXPECT_EQ(subCmd.argMapping->type, ArgMappingType::POSITIONAL);
    EXPECT_EQ(subCmd.eventTypes.size(), 2u);
    EXPECT_FALSE(subCmd.eventSchemas.empty());

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_3100 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_3200
 * @tc.desc: Test ParseFromJson name contains special characters
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_3200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_3200 start";

    nlohmann::json json = R"({"name": "tool-with_underscore.and.dots"})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.name, "tool-with_underscore.and.dots");

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_3200 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_3300
 * @tc.desc: Test ParseFromJson version with pre-release suffix
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_3300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_3300 start";

    nlohmann::json json = R"({"version": "1.0.0-beta.1"})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.version, "1.0.0-beta.1");

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_3300 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_3400
 * @tc.desc: Test ParseFromJson with boolean true for hasSubCommand
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_3400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_3400 start";

    nlohmann::json json = R"({"hasSubCommand": true})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_TRUE(tool.hasSubCommand);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_3400 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_3500
 * @tc.desc: Test ParseFromJson with boolean false for hasSubCommand
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_3500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_3500 start";

    nlohmann::json json = R"({"hasSubCommand": false})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_FALSE(tool.hasSubCommand);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_3500 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_3600
 * @tc.desc: Test ParseFromJson with inputSchema having nested array
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_3600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_3600 start";

    nlohmann::json json = R"({
        "inputSchema": {
            "type": "array",
            "items": [
                {"type": "string"},
                {"type": "number"}
            ]
        }
    })"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    nlohmann::json parsed = nlohmann::json::parse(tool.inputSchema);
    EXPECT_EQ(parsed["type"], "array");
    EXPECT_TRUE(parsed.contains("items"));

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_3600 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_3700
 * @tc.desc: Test ParseFromJson with empty subcommands object
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_3700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_3700 start";

    nlohmann::json json = R"({"subcommands": {}})"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.subcommands.size(), 0u);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_3700 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_3800
 * @tc.desc: Test ParseFromJson - all string fields empty string values
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_3800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_3800 start";

    nlohmann::json json = R"({
        "name": "",
        "version": "",
        "description": "",
        "executablePath": ""
    })"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.name, "");
    EXPECT_EQ(tool.version, "");
    EXPECT_EQ(tool.description, "");
    EXPECT_EQ(tool.executablePath, "");

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_3800 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_3900
 * @tc.desc: Test ParseFromJson - subcommand with empty inputSchema and outputSchema
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_3900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_3900 start";

    nlohmann::json json = R"({
        "subcommands": {
            "test": {
                "description": "test",
                "inputSchema": {},
                "outputSchema": {}
            }
        }
    })"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(json);

    EXPECT_EQ(tool.subcommands["test"].inputSchema, "{}");
    EXPECT_EQ(tool.subcommands["test"].outputSchema, "{}");

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_3900 end";
}

// ==================== ToolInfo ParseFromJson/ParseToJson Round Trip Tests ====================

/**
 * @tc.name: ToolInfo_ParseFromJson_ParseToJson_RoundTrip_0100
 * @tc.desc: Test ToolInfo ParseFromJson and ParseToJson round trip
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_ParseToJson_RoundTrip_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_ParseToJson_RoundTrip_0100 start";

    nlohmann::json originalJson = R"({
        "name": "roundtrip_tool",
        "version": "3.0.0",
        "description": "Round trip test",
        "executablePath": "/bin/roundtrip",
        "requirePermissions": ["ohos.permission.INTERNET", "ohos.permission.CAMERA"],
        "inputSchema": {"type": "object", "properties": {"input": {"type": "string"}}},
        "outputSchema": {"type": "array"},
        "argMapping": {"type": "mixed", "separator": ",", "order": "a,b"},
        "eventSchemas": {"stdout": {"type": "string"}},
        "timeout": 45000,
        "eventTypes": ["stdout", "stderr"],
        "hasSubCommand": true,
        "subcommands": {
            "sub1": {
                "description": "Sub 1",
                "inputSchema": {"type": "object"},
                "outputSchema": {"type": "string"}
            }
        }
    })"_json;

    ToolInfo tool = ToolInfo::ParseFromJson(originalJson);
    nlohmann::json resultJson = tool.ParseToJson();

    EXPECT_EQ(resultJson["name"], originalJson["name"]);
    EXPECT_EQ(resultJson["version"], originalJson["version"]);
    EXPECT_EQ(resultJson["description"], originalJson["description"]);
    EXPECT_EQ(resultJson["executablePath"], originalJson["executablePath"]);
    EXPECT_EQ(resultJson["timeout"], originalJson["timeout"]);
    EXPECT_EQ(resultJson["hasSubCommand"], originalJson["hasSubCommand"]);
    EXPECT_EQ(resultJson["eventTypes"], originalJson["eventTypes"]);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_ParseToJson_RoundTrip_0100 end";
}

// ==================== ToolInfo Validate Tests ====================

/**
 * @tc.name: ToolInfo_Validate_0100
 * @tc.desc: Test ToolInfo Validate with valid data
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_0100 start";

    ToolInfo tool;
    tool.name = "ohos-testtool";
    tool.version = "1.0.0";
    tool.description = "Test tool";
    tool.executablePath = "/usr/bin/test";
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.argMapping->type = ArgMappingType::FLAG;
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 30;
    tool.hasSubCommand = false;

    EXPECT_TRUE(tool.Validate());

    GTEST_LOG_(INFO) << "ToolInfo_Validate_0100 end";
}

/**
 * @tc.name: ToolInfo_Validate_0200
 * @tc.desc: Test ToolInfo Validate with hms- prefix
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_0200 start";

    ToolInfo tool;
    tool.name = "hms-testtool";
    tool.version = "1.0.0";
    tool.description = "Test tool";
    tool.executablePath = "/usr/bin/test";
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.argMapping->type = ArgMappingType::FLAG;
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 30;
    tool.hasSubCommand = false;

    EXPECT_TRUE(tool.Validate());

    GTEST_LOG_(INFO) << "ToolInfo_Validate_0200 end";
}

/**
 * @tc.name: ToolInfo_Validate_0201
 * @tc.desc: Test ToolInfo Validate with hms- prefix short name
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_0201, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_0201 start";

    ToolInfo tool;
    tool.name = "hms-l";  // hms- prefix with single char suffix
    tool.version = "1.0.0";
    tool.description = "Test tool";
    tool.executablePath = "/usr/bin/test";
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.argMapping->type = ArgMappingType::FLAG;
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 30;
    tool.hasSubCommand = false;

    EXPECT_TRUE(tool.Validate());

    GTEST_LOG_(INFO) << "ToolInfo_Validate_0201 end";
}

/**
 * @tc.name: ToolInfo_Validate_0202
 * @tc.desc: Test ToolInfo Validate with ohos- prefix short name
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_0202, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_0202 start";

    ToolInfo tool;
    tool.name = "ohos-a";  // ohos- prefix with single char suffix
    tool.version = "1.0.0";
    tool.description = "Test tool";
    tool.executablePath = "/usr/bin/test";
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.argMapping->type = ArgMappingType::FLAG;
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 30;
    tool.hasSubCommand = false;

    EXPECT_TRUE(tool.Validate());

    GTEST_LOG_(INFO) << "ToolInfo_Validate_0202 end";
}

/**
 * @tc.name: ToolInfo_Validate_0203
 * @tc.desc: Test ToolInfo Validate with hms- prefix only (no suffix)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_0203, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_0203 start";

    ToolInfo tool;
    tool.name = "hms-";  // prefix only, no suffix
    tool.version = "1.0.0";
    tool.description = "Test tool";
    tool.executablePath = "/usr/bin/test";
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 30;
    tool.hasSubCommand = false;

    EXPECT_FALSE(tool.Validate());

    GTEST_LOG_(INFO) << "ToolInfo_Validate_0203 end";
}

/**
 * @tc.name: ToolInfo_Validate_0204
 * @tc.desc: Test ToolInfo Validate with ohos- prefix only (no suffix)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_0204, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_0204 start";

    ToolInfo tool;
    tool.name = "ohos-";  // prefix only, no suffix
    tool.version = "1.0.0";
    tool.description = "Test tool";
    tool.executablePath = "/usr/bin/test";
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 30;
    tool.hasSubCommand = false;

    EXPECT_FALSE(tool.Validate());

    GTEST_LOG_(INFO) << "ToolInfo_Validate_0204 end";
}

/**
 * @tc.name: ToolInfo_Validate_0300
 * @tc.desc: Test ToolInfo Validate with empty name
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_0300 start";

    ToolInfo tool;
    tool.name = "";
    tool.version = "1.0.0";
    tool.description = "Test tool";
    tool.executablePath = "/usr/bin/test";
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 30;

    EXPECT_FALSE(tool.Validate());

    GTEST_LOG_(INFO) << "ToolInfo_Validate_0300 end";
}

/**
 * @tc.name: ToolInfo_Validate_0400
 * @tc.desc: Test ToolInfo Validate with invalid name prefix
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_0400 start";

    ToolInfo tool;
    tool.name = "invalid-tool";
    tool.version = "1.0.0";
    tool.description = "Test tool";
    tool.executablePath = "/usr/bin/test";
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 30;

    EXPECT_FALSE(tool.Validate());

    GTEST_LOG_(INFO) << "ToolInfo_Validate_0400 end";
}

/**
 * @tc.name: ToolInfo_Validate_0500
 * @tc.desc: Test ToolInfo Validate with name suffix too long
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_0500 start";

    ToolInfo tool;
    tool.name = "ohos-abcdefghijklmnopq";  // suffix > 16 chars
    tool.version = "1.0.0";
    tool.description = "Test tool";
    tool.executablePath = "/usr/bin/test";
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 30;

    EXPECT_FALSE(tool.Validate());

    GTEST_LOG_(INFO) << "ToolInfo_Validate_0500 end";
}

/**
 * @tc.name: ToolInfo_Validate_0600
 * @tc.desc: Test ToolInfo Validate with empty version
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_0600 start";

    ToolInfo tool;
    tool.name = "ohos-testtool";
    tool.version = "";
    tool.description = "Test tool";
    tool.executablePath = "/usr/bin/test";
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 30;

    EXPECT_FALSE(tool.Validate());

    GTEST_LOG_(INFO) << "ToolInfo_Validate_0600 end";
}

/**
 * @tc.name: ToolInfo_Validate_0700
 * @tc.desc: Test ToolInfo Validate with empty description
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_0700 start";

    ToolInfo tool;
    tool.name = "ohos-testtool";
    tool.version = "1.0.0";
    tool.description = "";
    tool.executablePath = "/usr/bin/test";
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 30;

    EXPECT_FALSE(tool.Validate());

    GTEST_LOG_(INFO) << "ToolInfo_Validate_0700 end";
}

/**
 * @tc.name: ToolInfo_Validate_0800
 * @tc.desc: Test ToolInfo Validate with relative path
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_0800 start";

    ToolInfo tool;
    tool.name = "ohos-testtool";
    tool.version = "1.0.0";
    tool.description = "Test tool";
    tool.executablePath = "bin/test";  // relative path
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 30;

    EXPECT_FALSE(tool.Validate());

    GTEST_LOG_(INFO) << "ToolInfo_Validate_0800 end";
}

/**
 * @tc.name: ToolInfo_Validate_0900
 * @tc.desc: Test ToolInfo Validate with invalid inputSchema
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_0900 start";

    ToolInfo tool;
    tool.name = "ohos-testtool";
    tool.version = "1.0.0";
    tool.description = "Test tool";
    tool.executablePath = "/usr/bin/test";
    tool.inputSchema = "not a json";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 30;

    EXPECT_FALSE(tool.Validate());

    GTEST_LOG_(INFO) << "ToolInfo_Validate_0900 end";
}

/**
 * @tc.name: ToolInfo_Validate_1000
 * @tc.desc: Test ToolInfo Validate with invalid timeout (0)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_1000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_1000 start";

    ToolInfo tool;
    tool.name = "ohos-testtool";
    tool.version = "1.0.0";
    tool.description = "Test tool";
    tool.executablePath = "/usr/bin/test";
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 0;

    EXPECT_FALSE(tool.Validate());

    GTEST_LOG_(INFO) << "ToolInfo_Validate_1000 end";
}

/**
 * @tc.name: ToolInfo_Validate_1100
 * @tc.desc: Test ToolInfo Validate with invalid timeout (>300)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_1100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_1100 start";

    ToolInfo tool;
    tool.name = "ohos-testtool";
    tool.version = "1.0.0";
    tool.description = "Test tool";
    tool.executablePath = "/usr/bin/test";
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 301;

    EXPECT_FALSE(tool.Validate());

    GTEST_LOG_(INFO) << "ToolInfo_Validate_1100 end";
}

/**
 * @tc.name: ToolInfo_Validate_1200
 * @tc.desc: Test ToolInfo Validate with null argMapping
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_1200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_1200 start";

    ToolInfo tool;
    tool.name = "ohos-testtool";
    tool.version = "1.0.0";
    tool.description = "Test tool";
    tool.executablePath = "/usr/bin/test";
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = nullptr;
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 30;

    EXPECT_FALSE(tool.Validate());

    GTEST_LOG_(INFO) << "ToolInfo_Validate_1200 end";
}

/**
 * @tc.name: ToolInfo_Validate_1300
 * @tc.desc: Test ToolInfo Validate with hasSubCommand true but no subcommands
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_1300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_1300 start";

    ToolInfo tool;
    tool.name = "ohos-testtool";
    tool.version = "1.0.0";
    tool.description = "Test tool";
    tool.executablePath = "/usr/bin/test";
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 30;
    tool.hasSubCommand = true;
    // subcommands is empty

    EXPECT_FALSE(tool.Validate());

    GTEST_LOG_(INFO) << "ToolInfo_Validate_1300 end";
}

/**
 * @tc.name: ToolInfo_Validate_1400
 * @tc.desc: Test ToolInfo Validate with valid subcommands
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_1400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_1400 start";

    ToolInfo tool;
    tool.name = "ohos-testtool";
    tool.version = "1.0.0";
    tool.description = "Test tool";
    tool.executablePath = "/usr/bin/test";
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 30;
    tool.hasSubCommand = true;

    SubCommandInfo subCmd;
    subCmd.description = "Test subcommand";
    subCmd.inputSchema = R"({"type": "object"})";
    subCmd.outputSchema = R"({"type": "string"})";
    subCmd.argMapping = std::make_shared<ArgMapping>();
    subCmd.eventSchemas = R"({"event": {"type": "string"}})";
    tool.subcommands["sub1"] = subCmd;

    EXPECT_TRUE(tool.Validate());

    GTEST_LOG_(INFO) << "ToolInfo_Validate_1400 end";
}

/**
 * @tc.name: ToolInfo_Validate_1500
 * @tc.desc: Test ToolInfo Validate with invalid subcommand
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_1500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_1500 start";

    ToolInfo tool;
    tool.name = "ohos-testtool";
    tool.version = "1.0.0";
    tool.description = "Test tool";
    tool.executablePath = "/usr/bin/test";
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 30;
    tool.hasSubCommand = true;

    SubCommandInfo subCmd;
    subCmd.description = "";  // empty description
    subCmd.inputSchema = R"({"type": "object"})";
    subCmd.outputSchema = R"({"type": "string"})";
    subCmd.argMapping = std::make_shared<ArgMapping>();
    subCmd.eventSchemas = R"({"event": {"type": "string"}})";
    tool.subcommands["sub1"] = subCmd;

    EXPECT_FALSE(tool.Validate());

    GTEST_LOG_(INFO) << "ToolInfo_Validate_1500 end";
}

/**
 * @tc.name: ToolInfo_Validate_1600
 * @tc.desc: Test ToolInfo Validate with timeout boundary (1)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_1600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_1600 start";

    ToolInfo tool;
    tool.name = "ohos-testtool";
    tool.version = "1.0.0";
    tool.description = "Test tool";
    tool.executablePath = "/usr/bin/test";
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 1;

    EXPECT_TRUE(tool.Validate());

    GTEST_LOG_(INFO) << "ToolInfo_Validate_1600 end";
}

/**
 * @tc.name: ToolInfo_Validate_1700
 * @tc.desc: Test ToolInfo Validate with timeout boundary (300)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_1700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_1700 start";

    ToolInfo tool;
    tool.name = "ohos-testtool";
    tool.version = "1.0.0";
    tool.description = "Test tool";
    tool.executablePath = "/usr/bin/test";
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 300;

    EXPECT_TRUE(tool.Validate());

    GTEST_LOG_(INFO) << "ToolInfo_Validate_1700 end";
}

/**
 * @tc.name: ToolInfo_Validate_1800
 * @tc.desc: Test ToolInfo Validate with duplicate requirePermissions
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_1800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_1800 start";

    ToolInfo tool;
    tool.name = "ohos-testtool";
    tool.version = "1.0.0";
    tool.description = "Test tool";
    tool.executablePath = "/usr/bin/test";
    tool.requirePermissions = {"ohos.permission.INTERNET", "ohos.permission.INTERNET"};  // duplicate
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 30;

    EXPECT_FALSE(tool.Validate());

    GTEST_LOG_(INFO) << "ToolInfo_Validate_1800 end";
}

/**
 * @tc.name: ToolInfo_Validate_1900
 * @tc.desc: Test ToolInfo Validate with unique requirePermissions
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_1900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_1900 start";

    ToolInfo tool;
    tool.name = "ohos-testtool";
    tool.version = "1.0.0";
    tool.description = "Test tool";
    tool.executablePath = "/usr/bin/test";
    tool.requirePermissions = {"ohos.permission.INTERNET", "ohos.permission.CAMERA"};  // unique
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.argMapping = std::make_shared<ArgMapping>();
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.timeout = 30;

    EXPECT_TRUE(tool.Validate());

    GTEST_LOG_(INFO) << "ToolInfo_Validate_1900 end";
}

} // namespace CliTool
} // namespace OHOS