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
#include <fstream>

#define private public
#define protected public
#include "cli_tool_data_manager.h"
#include "mock_single_kv_store.h"
#undef private
#undef protected

#include "cli_tool_manager_data.h"
#include "nlohmann/json.hpp"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace CliTool {

namespace {
constexpr int32_t ERR_OK = 0;
constexpr int32_t ERR_FILE_NOT_FOUND = -1;
constexpr int32_t ERR_JSON_PARSE_FAILED = -2;
constexpr int32_t ERR_KVSTORE_NOT_READY = -3;
}

class CliToolDataManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void CliToolDataManagerTest::SetUpTestCase() {}

void CliToolDataManagerTest::TearDownTestCase() {}

void CliToolDataManagerTest::SetUp() {}

void CliToolDataManagerTest::TearDown() {}

/**
 * Feature: CliToolDataManager
 * Function: CheckKvStore
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager CheckKvStore with null kvStorePtr
 */
HWTEST_F(CliToolDataManagerTest, CheckKvStore_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckKvStore_100 start";
    CliToolDataManager dataManager;
    dataManager.kvStorePtr_ = nullptr;
    auto result = dataManager.CheckKvStore();
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "CheckKvStore_100 end";
}

/**
 * Feature: CliToolDataManager
 * Function: CheckKvStore
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager CheckKvStore with valid kvStorePtr
 */
HWTEST_F(CliToolDataManagerTest, CheckKvStore_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckKvStore_200 start";
    CliToolDataManager dataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    dataManager.kvStorePtr_ = kvStorePtr;
    auto result = dataManager.CheckKvStore();
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "CheckKvStore_200 end";
}

/**
 * Feature: CliToolDataManager
 * Function: EnsureToolsLoaded
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager EnsureToolsLoaded when already loaded
 */
HWTEST_F(CliToolDataManagerTest, EnsureToolsLoaded_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnsureToolsLoaded_100 start";
    CliToolDataManager dataManager;
    dataManager.toolsLoaded_ = true;
    auto result = dataManager.EnsureToolsLoaded();
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "EnsureToolsLoaded_100 end";
}

/**
 * Feature: CliToolDataManager
 * Function: EnsureToolsLoaded
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager EnsureToolsLoaded when KVStore not ready
 */
HWTEST_F(CliToolDataManagerTest, EnsureToolsLoaded_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnsureToolsLoaded_200 start";
    CliToolDataManager dataManager;
    dataManager.toolsLoaded_ = false;
    dataManager.kvStorePtr_ = nullptr;
    auto result = dataManager.EnsureToolsLoaded();
    EXPECT_EQ(result, ERR_KVSTORE_NOT_READY);
    GTEST_LOG_(INFO) << "EnsureToolsLoaded_200 end";
}

/**
 * Feature: CliToolDataManager
 * Function: GetAllTools
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager GetAllTools when KVStore not ready
 */
HWTEST_F(CliToolDataManagerTest, GetAllTools_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAllTools_100 start";
    CliToolDataManager dataManager;
    dataManager.kvStorePtr_ = nullptr;
    dataManager.toolsLoaded_ = false;
    std::vector<ToolInfo> tools;
    auto result = dataManager.GetAllTools(tools);
    EXPECT_EQ(result, ERR_KVSTORE_NOT_READY);
    GTEST_LOG_(INFO) << "GetAllTools_100 end";
}

/**
 * Feature: CliToolDataManager
 * Function: GetAllTools
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager GetAllTools when tools list not found
 */
HWTEST_F(CliToolDataManagerTest, GetAllTools_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAllTools_200 start";
    CliToolDataManager dataManager;
    auto mockKvStore = std::make_shared<MockSingleKvStore>();
    dataManager.kvStorePtr_ = mockKvStore;
    dataManager.toolsLoaded_ = true;
    mockKvStore->Get_ = DistributedKv::Status::KEY_NOT_FOUND;

    std::vector<ToolInfo> tools;
    auto result = dataManager.GetAllTools(tools);
    EXPECT_NE(result, ERR_OK);
    mockKvStore->Get_ = DistributedKv::Status::SUCCESS;
    GTEST_LOG_(INFO) << "GetAllTools_200 end";
}

/**
 * Feature: CliToolDataManager
 * Function: GetAllTools
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager GetAllTools with valid data
 */
HWTEST_F(CliToolDataManagerTest, GetAllTools_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAllTools_300 start";
    CliToolDataManager dataManager;
    auto mockKvStore = std::make_shared<MockSingleKvStore>();
    dataManager.kvStorePtr_ = mockKvStore;
    dataManager.toolsLoaded_ = true;

    nlohmann::json toolsJson = nlohmann::json::array();
    nlohmann::json tool1;
    tool1["name"] = "test_tool";
    tool1["version"] = "1.0";
    tool1["description"] = "Test tool";
    toolsJson.push_back(tool1);
    mockKvStore->SetMockData("all_tools_list", toolsJson.dump());

    std::vector<ToolInfo> tools;
    auto result = dataManager.GetAllTools(tools);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(tools.size(), 1);
    GTEST_LOG_(INFO) << "GetAllTools_300 end";
}

/**
 * Feature: CliToolDataManager
 * Function: GetToolByName
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager GetToolByName when tool found
 */
HWTEST_F(CliToolDataManagerTest, GetToolByName_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetToolByName_100 start";
    CliToolDataManager dataManager;
    auto mockKvStore = std::make_shared<MockSingleKvStore>();
    dataManager.kvStorePtr_ = mockKvStore;
    dataManager.toolsLoaded_ = true;

    nlohmann::json toolsJson = nlohmann::json::array();
    nlohmann::json tool1;
    tool1["name"] = "test_tool";
    tool1["version"] = "1.0";
    tool1["description"] = "Test tool";
    toolsJson.push_back(tool1);
    mockKvStore->SetMockData("all_tools_list", toolsJson.dump());

    ToolInfo tool;
    auto result = dataManager.GetToolByName("test_tool", tool);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(tool.name, "test_tool");
    GTEST_LOG_(INFO) << "GetToolByName_100 end";
}

/**
 * Feature: CliToolDataManager
 * Function: GetToolByName
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager GetToolByName when tool not found
 */
HWTEST_F(CliToolDataManagerTest, GetToolByName_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetToolByName_200 start";
    CliToolDataManager dataManager;
    auto mockKvStore = std::make_shared<MockSingleKvStore>();
    dataManager.kvStorePtr_ = mockKvStore;
    dataManager.toolsLoaded_ = true;

    nlohmann::json toolsJson = nlohmann::json::array();
    nlohmann::json tool1;
    tool1["name"] = "test_tool";
    tool1["version"] = "1.0";
    tool1["description"] = "Test tool";
    toolsJson.push_back(tool1);
    mockKvStore->SetMockData("all_tools_list", toolsJson.dump());

    ToolInfo tool;
    auto result = dataManager.GetToolByName("nonexistent_tool", tool);
    EXPECT_NE(result, ERR_OK);
    GTEST_LOG_(INFO) << "GetToolByName_200 end";
}

/**
 * Feature: CliToolDataManager
 * Function: ToolExists
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager ToolExists when tool exists
 */
HWTEST_F(CliToolDataManagerTest, ToolExists_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolExists_100 start";
    CliToolDataManager dataManager;
    auto mockKvStore = std::make_shared<MockSingleKvStore>();
    dataManager.kvStorePtr_ = mockKvStore;
    dataManager.toolsLoaded_ = true;

    nlohmann::json toolsJson = nlohmann::json::array();
    nlohmann::json tool1;
    tool1["name"] = "test_tool";
    tool1["version"] = "1.0";
    tool1["description"] = "Test tool";
    toolsJson.push_back(tool1);
    mockKvStore->SetMockData("all_tools_list", toolsJson.dump());

    auto result = dataManager.ToolExists("test_tool");
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "ToolExists_100 end";
}

/**
 * Feature: CliToolDataManager
 * Function: ToolExists
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager ToolExists when tool not exists
 */
HWTEST_F(CliToolDataManagerTest, ToolExists_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolExists_200 start";
    CliToolDataManager dataManager;
    auto mockKvStore = std::make_shared<MockSingleKvStore>();
    dataManager.kvStorePtr_ = mockKvStore;
    dataManager.toolsLoaded_ = true;

    nlohmann::json toolsJson = nlohmann::json::array();
    nlohmann::json tool1;
    tool1["name"] = "test_tool";
    tool1["version"] = "1.0";
    tool1["description"] = "Test tool";
    toolsJson.push_back(tool1);
    mockKvStore->SetMockData("all_tools_list", toolsJson.dump());

    auto result = dataManager.ToolExists("nonexistent_tool");
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "ToolExists_200 end";
}

/**
 * Feature: CliToolDataManager
 * Function: QueryToolSummaries
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager QueryToolSummaries with valid data
 */
HWTEST_F(CliToolDataManagerTest, QueryToolSummaries_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryToolSummaries_100 start";
    CliToolDataManager dataManager;
    auto mockKvStore = std::make_shared<MockSingleKvStore>();
    dataManager.kvStorePtr_ = mockKvStore;
    dataManager.toolsLoaded_ = true;

    nlohmann::json toolsJson = nlohmann::json::array();
    nlohmann::json tool1;
    tool1["name"] = "test_tool";
    tool1["version"] = "1.0";
    tool1["description"] = "Test tool";
    toolsJson.push_back(tool1);
    mockKvStore->SetMockData("all_tools_list", toolsJson.dump());

    std::vector<ToolSummary> summaries;
    auto result = dataManager.QueryToolSummaries(summaries);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(summaries.size(), 1);
    EXPECT_EQ(summaries[0].name, "test_tool");
    GTEST_LOG_(INFO) << "QueryToolSummaries_100 end";
}

/**
 * Feature: CliToolDataManager
 * Function: QueryToolSummaries
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager QueryToolSummaries when KVStore not ready
 */
HWTEST_F(CliToolDataManagerTest, QueryToolSummaries_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryToolSummaries_200 start";
    CliToolDataManager dataManager;
    dataManager.kvStorePtr_ = nullptr;
    dataManager.toolsLoaded_ = false;

    std::vector<ToolSummary> summaries;
    auto result = dataManager.QueryToolSummaries(summaries);
    EXPECT_EQ(result, ERR_KVSTORE_NOT_READY);
    GTEST_LOG_(INFO) << "QueryToolSummaries_200 end";
}

/**
 * Feature: CliToolDataManager
 * Function: RegisterTool
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager RegisterTool with valid data
 */
HWTEST_F(CliToolDataManagerTest, RegisterTool_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterTool_100 start";
    CliToolDataManager dataManager;
    auto mockKvStore = std::make_shared<MockSingleKvStore>();
    dataManager.kvStorePtr_ = mockKvStore;

    ToolInfo tool;
    tool.name = "new_tool";
    tool.version = "1.0";
    tool.description = "New test tool";

    auto result = dataManager.RegisterTool(tool);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "RegisterTool_100 end";
}

/**
 * Feature: CliToolDataManager
 * Function: RegisterTool
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager RegisterTool when KVStore not ready
 */
HWTEST_F(CliToolDataManagerTest, RegisterTool_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterTool_200 start";
    CliToolDataManager dataManager;
    dataManager.kvStorePtr_ = nullptr;

    ToolInfo tool;
    tool.name = "new_tool";
    tool.version = "1.0";
    tool.description = "New test tool";

    auto result = dataManager.RegisterTool(tool);
    EXPECT_EQ(result, ERR_KVSTORE_NOT_READY);
    GTEST_LOG_(INFO) << "RegisterTool_200 end";
}

/**
 * Feature: CliToolDataManager
 * Function: RegisterTool
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager RegisterTool when Put fails
 */
HWTEST_F(CliToolDataManagerTest, RegisterTool_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterTool_300 start";
    CliToolDataManager dataManager;
    auto mockKvStore = std::make_shared<MockSingleKvStore>();
    dataManager.kvStorePtr_ = mockKvStore;
    mockKvStore->Put_ = DistributedKv::Status::INVALID_FORMAT;

    ToolInfo tool;
    tool.name = "new_tool";
    tool.version = "1.0";
    tool.description = "New test tool";

    auto result = dataManager.RegisterTool(tool);
    EXPECT_NE(result, ERR_OK);
    mockKvStore->Put_ = DistributedKv::Status::SUCCESS;
    GTEST_LOG_(INFO) << "RegisterTool_300 end";
}

/**
 * Feature: CliToolDataManager
 * Function: StoreTool
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager StoreTool with valid data
 */
HWTEST_F(CliToolDataManagerTest, StoreTool_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StoreTool_100 start";
    CliToolDataManager dataManager;
    auto mockKvStore = std::make_shared<MockSingleKvStore>();
    dataManager.kvStorePtr_ = mockKvStore;

    ToolInfo tool;
    tool.name = "test_tool";
    tool.version = "1.0";
    tool.description = "Test tool";

    auto result = dataManager.StoreTool(tool);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "StoreTool_100 end";
}

/**
 * Feature: CliToolDataManager
 * Function: StoreTool
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager StoreTool when Put fails
 */
HWTEST_F(CliToolDataManagerTest, StoreTool_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StoreTool_200 start";
    CliToolDataManager dataManager;
    auto mockKvStore = std::make_shared<MockSingleKvStore>();
    dataManager.kvStorePtr_ = mockKvStore;
    mockKvStore->Put_ = DistributedKv::Status::INVALID_FORMAT;

    ToolInfo tool;
    tool.name = "test_tool";
    tool.version = "1.0";
    tool.description = "Test tool";

    auto result = dataManager.StoreTool(tool);
    EXPECT_NE(result, ERR_OK);
    mockKvStore->Put_ = DistributedKv::Status::SUCCESS;
    GTEST_LOG_(INFO) << "StoreTool_200 end";
}

/**
 * Feature: CliToolDataManager
 * Function: ToolInfoToJson
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager ToolInfoToJson
 */
HWTEST_F(CliToolDataManagerTest, ToolInfoToJson_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfoToJson_100 start";
    CliToolDataManager dataManager;

    ToolInfo tool;
    tool.name = "test_tool";
    tool.version = "1.0";
    tool.description = "Test tool";
    tool.executablePath = "/usr/bin/test_tool";
    tool.timeout = 30000;
    tool.enabled = true;

    auto jsonStr = dataManager.ToolInfoToJson(tool);
    EXPECT_FALSE(jsonStr.empty());

    nlohmann::json j = nlohmann::json::parse(jsonStr, nullptr, false);
    EXPECT_FALSE(j.is_discarded());
    EXPECT_EQ(j["name"], "test_tool");
    EXPECT_EQ(j["version"], "1.0");
    EXPECT_EQ(j["timeout"], 30000);
    GTEST_LOG_(INFO) << "ToolInfoToJson_100 end";
}

/**
 * Feature: CliToolDataManager
 * Function: JsonToToolInfo
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager JsonToToolInfo with valid JSON
 */
HWTEST_F(CliToolDataManagerTest, JsonToToolInfo_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsonToToolInfo_100 start";
    CliToolDataManager dataManager;

    nlohmann::json j;
    j["name"] = "test_tool";
    j["version"] = "1.0";
    j["description"] = "Test tool";
    j["executablePath"] = "/usr/bin/test_tool";
    j["timeout"] = 30000;
    j["enabled"] = true;

    ToolInfo tool;
    auto result = dataManager.JsonToToolInfo(j.dump(), tool);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(tool.name, "test_tool");
    EXPECT_EQ(tool.version, "1.0");
    EXPECT_EQ(tool.timeout, 30000);
    EXPECT_TRUE(tool.enabled);
    GTEST_LOG_(INFO) << "JsonToToolInfo_100 end";
}

/**
 * Feature: CliToolDataManager
 * Function: JsonToToolInfo
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager JsonToToolInfo with invalid JSON
 */
HWTEST_F(CliToolDataManagerTest, JsonToToolInfo_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsonToToolInfo_200 start";
    CliToolDataManager dataManager;

    ToolInfo tool;
    auto result = dataManager.JsonToToolInfo("invalid json", tool);
    EXPECT_EQ(result, ERR_JSON_PARSE_FAILED);
    GTEST_LOG_(INFO) << "JsonToToolInfo_200 end";
}

/**
 * Feature: CliToolDataManager
 * Function: ToolsToJsonArray
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager ToolsToJsonArray
 */
HWTEST_F(CliToolDataManagerTest, ToolsToJsonArray_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolsToJsonArray_100 start";
    CliToolDataManager dataManager;

    std::vector<ToolInfo> tools;
    ToolInfo tool1;
    tool1.name = "tool1";
    tool1.version = "1.0";
    tool1.description = "Tool 1";
    tools.push_back(tool1);

    ToolInfo tool2;
    tool2.name = "tool2";
    tool2.version = "2.0";
    tool2.description = "Tool 2";
    tools.push_back(tool2);

    auto jsonStr = dataManager.ToolsToJsonArray(tools);
    EXPECT_FALSE(jsonStr.empty());

    nlohmann::json j = nlohmann::json::parse(jsonStr, nullptr, false);
    EXPECT_FALSE(j.is_discarded());
    EXPECT_TRUE(j.is_array());
    EXPECT_EQ(j.size(), 2);
    GTEST_LOG_(INFO) << "ToolsToJsonArray_100 end";
}

/**
 * Feature: CliToolDataManager
 * Function: JsonArrayToTools
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager JsonArrayToTools with valid JSON array
 */
HWTEST_F(CliToolDataManagerTest, JsonArrayToTools_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsonArrayToTools_100 start";
    CliToolDataManager dataManager;

    nlohmann::json j = nlohmann::json::array();
    nlohmann::json tool1;
    tool1["name"] = "tool1";
    tool1["version"] = "1.0";
    tool1["description"] = "Tool 1";
    j.push_back(tool1);

    nlohmann::json tool2;
    tool2["name"] = "tool2";
    tool2["version"] = "2.0";
    tool2["description"] = "Tool 2";
    j.push_back(tool2);

    std::vector<ToolInfo> tools;
    auto result = dataManager.JsonArrayToTools(j.dump(), tools);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(tools.size(), 2);
    EXPECT_EQ(tools[0].name, "tool1");
    EXPECT_EQ(tools[1].name, "tool2");
    GTEST_LOG_(INFO) << "JsonArrayToTools_100 end";
}

/**
 * Feature: CliToolDataManager
 * Function: JsonArrayToTools
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager JsonArrayToTools with invalid JSON
 */
HWTEST_F(CliToolDataManagerTest, JsonArrayToTools_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsonArrayToTools_200 start";
    CliToolDataManager dataManager;

    std::vector<ToolInfo> tools;
    auto result = dataManager.JsonArrayToTools("invalid json", tools);
    EXPECT_EQ(result, ERR_JSON_PARSE_FAILED);
    GTEST_LOG_(INFO) << "JsonArrayToTools_200 end";
}

/**
 * Feature: CliToolDataManager
 * Function: JsonArrayToTools
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager JsonArrayToTools with non-array JSON
 */
HWTEST_F(CliToolDataManagerTest, JsonArrayToTools_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsonArrayToTools_300 start";
    CliToolDataManager dataManager;

    nlohmann::json j;
    j["name"] = "tool1";

    std::vector<ToolInfo> tools;
    auto result = dataManager.JsonArrayToTools(j.dump(), tools);
    EXPECT_EQ(result, ERR_JSON_PARSE_FAILED);
    GTEST_LOG_(INFO) << "JsonArrayToTools_300 end";
}

/**
 * Feature: CliToolDataManager
 * Function: ParseJsonFile
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager ParseJsonFile with non-existent file
 */
HWTEST_F(CliToolDataManagerTest, ParseJsonFile_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ParseJsonFile_100 start";
    CliToolDataManager dataManager;

    std::vector<ToolInfo> tools;
    auto result = dataManager.ParseJsonFile("/nonexistent/path/file.json", tools);
    EXPECT_EQ(result, ERR_FILE_NOT_FOUND);
    GTEST_LOG_(INFO) << "ParseJsonFile_100 end";
}

/**
 * Feature: CliToolDataManager
 * Function: ParseJsonFile
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager ParseJsonFile with valid JSON file
 */
HWTEST_F(CliToolDataManagerTest, ParseJsonFile_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ParseJsonFile_200 start";
    CliToolDataManager dataManager;

    std::string testFilePath = "/tmp/test_tools.json";
    nlohmann::json j = nlohmann::json::array();
    nlohmann::json tool1;
    tool1["name"] = "test_tool";
    tool1["version"] = "1.0";
    tool1["description"] = "Test tool";
    tool1["executablePath"] = "/usr/bin/test_tool";
    tool1["timeout"] = 30000;
    j.push_back(tool1);

    std::ofstream file(testFilePath);
    file << j.dump();
    file.close();

    std::vector<ToolInfo> tools;
    auto result = dataManager.ParseJsonFile(testFilePath, tools);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(tools.size(), 1);
    EXPECT_EQ(tools[0].name, "test_tool");
    GTEST_LOG_(INFO) << "ParseJsonFile_200 end";
}

/**
 * Feature: CliToolDataManager
 * Function: ParseJsonFile
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager ParseJsonFile with invalid JSON content
 */
HWTEST_F(CliToolDataManagerTest, ParseJsonFile_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ParseJsonFile_300 start";
    CliToolDataManager dataManager;

    std::string testFilePath = "/tmp/test_invalid_tools.json";
    std::ofstream file(testFilePath);
    file << "invalid json content";
    file.close();

    std::vector<ToolInfo> tools;
    auto result = dataManager.ParseJsonFile(testFilePath, tools);
    EXPECT_EQ(result, ERR_JSON_PARSE_FAILED);
    GTEST_LOG_(INFO) << "ParseJsonFile_300 end";
}

/**
 * Feature: CliToolDataManager
 * Function: ParseJsonFile
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager ParseJsonFile with empty file
 */
HWTEST_F(CliToolDataManagerTest, ParseJsonFile_400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ParseJsonFile_400 start";
    CliToolDataManager dataManager;

    std::string testFilePath = "/tmp/test_empty_tools.json";
    std::ofstream file(testFilePath);
    file << "";
    file.close();

    std::vector<ToolInfo> tools;
    auto result = dataManager.ParseJsonFile(testFilePath, tools);
    EXPECT_EQ(result, ERR_JSON_PARSE_FAILED);
    GTEST_LOG_(INFO) << "ParseJsonFile_400 end";
}

/**
 * Feature: CliToolDataManager
 * Function: ParseJsonFile
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager ParseJsonFile with non-array JSON
 */
HWTEST_F(CliToolDataManagerTest, ParseJsonFile_500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ParseJsonFile_500 start";
    CliToolDataManager dataManager;

    std::string testFilePath = "/tmp/test_object_tools.json";
    nlohmann::json j;
    j["name"] = "test_tool";
    j["version"] = "1.0";

    std::ofstream file(testFilePath);
    file << j.dump();
    file.close();

    std::vector<ToolInfo> tools;
    auto result = dataManager.ParseJsonFile(testFilePath, tools);
    EXPECT_EQ(result, ERR_JSON_PARSE_FAILED);
    GTEST_LOG_(INFO) << "ParseJsonFile_500 end";
}

/**
 * Feature: CliToolDataManager
 * Function: LoadToolsFromFile
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager LoadToolsFromFile with KVStore not ready
 */
HWTEST_F(CliToolDataManagerTest, LoadToolsFromFile_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadToolsFromFile_100 start";
    CliToolDataManager dataManager;
    dataManager.kvStorePtr_ = nullptr;

    auto result = dataManager.LoadToolsFromFile("/nonexistent/path/file.json");
    EXPECT_EQ(result, ERR_FILE_NOT_FOUND);
    GTEST_LOG_(INFO) << "LoadToolsFromFile_100 end";
}

/**
 * Feature: CliToolDataManager
 * Function: LoadToolsFromFile
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager LoadToolsFromFile with valid file
 */
HWTEST_F(CliToolDataManagerTest, LoadToolsFromFile_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadToolsFromFile_200 start";
    CliToolDataManager dataManager;
    auto mockKvStore = std::make_shared<MockSingleKvStore>();
    dataManager.kvStorePtr_ = mockKvStore;

    std::string testFilePath = "/tmp/test_load_tools.json";
    nlohmann::json j = nlohmann::json::array();
    nlohmann::json tool1;
    tool1["name"] = "test_tool";
    tool1["version"] = "1.0";
    tool1["description"] = "Test tool";
    j.push_back(tool1);

    std::ofstream file(testFilePath);
    file << j.dump();
    file.close();

    auto result = dataManager.LoadToolsFromFile(testFilePath);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "LoadToolsFromFile_200 end";
}

/**
 * Feature: CliToolDataManager
 * Function: LoadToolsFromFile
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager LoadToolsFromFile when Put fails
 */
HWTEST_F(CliToolDataManagerTest, LoadToolsFromFile_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadToolsFromFile_300 start";
    CliToolDataManager dataManager;
    auto mockKvStore = std::make_shared<MockSingleKvStore>();
    dataManager.kvStorePtr_ = mockKvStore;
    mockKvStore->Put_ = DistributedKv::Status::INVALID_FORMAT;

    std::string testFilePath = "/tmp/test_load_tools_fail.json";
    nlohmann::json j = nlohmann::json::array();
    nlohmann::json tool1;
    tool1["name"] = "test_tool";
    tool1["version"] = "1.0";
    tool1["description"] = "Test tool";
    j.push_back(tool1);

    std::ofstream file(testFilePath);
    file << j.dump();
    file.close();

    auto result = dataManager.LoadToolsFromFile(testFilePath);
    EXPECT_NE(result, ERR_OK);
    mockKvStore->Put_ = DistributedKv::Status::SUCCESS;
    GTEST_LOG_(INFO) << "LoadToolsFromFile_300 end";
}

/**
 * Feature: CliToolDataManager
 * Function: ParseJsonFile with permissions
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager ParseJsonFile with permissions array
 */
HWTEST_F(CliToolDataManagerTest, ParseJsonFile_WithPermissions_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ParseJsonFile_WithPermissions_100 start";
    CliToolDataManager dataManager;

    std::string testFilePath = "/tmp/test_tools_permissions.json";
    nlohmann::json j = nlohmann::json::array();
    nlohmann::json tool1;
    tool1["name"] = "test_tool";
    tool1["version"] = "1.0";
    tool1["description"] = "Test tool";
    tool1["permissions"] = nlohmann::json::array({"perm1", "perm2", "perm3"});
    j.push_back(tool1);

    std::ofstream file(testFilePath);
    file << j.dump();
    file.close();

    std::vector<ToolInfo> tools;
    auto result = dataManager.ParseJsonFile(testFilePath, tools);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(tools.size(), 1);
    EXPECT_EQ(tools[0].permissions.size(), 3);
    GTEST_LOG_(INFO) << "ParseJsonFile_WithPermissions_100 end";
}

/**
 * Feature: CliToolDataManager
 * Function: ParseJsonFile with eventTypes
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager ParseJsonFile with eventTypes array
 */
HWTEST_F(CliToolDataManagerTest, ParseJsonFile_WithEventTypes_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ParseJsonFile_WithEventTypes_100 start";
    CliToolDataManager dataManager;

    std::string testFilePath = "/tmp/test_tools_events.json";
    nlohmann::json j = nlohmann::json::array();
    nlohmann::json tool1;
    tool1["name"] = "test_tool";
    tool1["version"] = "1.0";
    tool1["description"] = "Test tool";
    tool1["eventTypes"] = nlohmann::json::array({"event1", "event2"});
    j.push_back(tool1);

    std::ofstream file(testFilePath);
    file << j.dump();
    file.close();

    std::vector<ToolInfo> tools;
    auto result = dataManager.ParseJsonFile(testFilePath, tools);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(tools.size(), 1);
    EXPECT_EQ(tools[0].eventTypes.size(), 2);
    GTEST_LOG_(INFO) << "ParseJsonFile_WithEventTypes_100 end";
}

/**
 * Feature: CliToolDataManager
 * Function: ParseJsonFile with subcommands
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager ParseJsonFile with hasSubcommands and subcommands
 */
HWTEST_F(CliToolDataManagerTest, ParseJsonFile_WithSubcommands_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ParseJsonFile_WithSubcommands_100 start";
    CliToolDataManager dataManager;

    std::string testFilePath = "/tmp/test_tools_subcommands.json";
    nlohmann::json j = nlohmann::json::array();
    nlohmann::json tool1;
    tool1["name"] = "test_tool";
    tool1["version"] = "1.0";
    tool1["description"] = "Test tool";
    tool1["hasSubcommands"] = true;
    tool1["subcommands"] = "start,stop,restart";
    j.push_back(tool1);

    std::ofstream file(testFilePath);
    file << j.dump();
    file.close();

    std::vector<ToolInfo> tools;
    auto result = dataManager.ParseJsonFile(testFilePath, tools);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(tools.size(), 1);
    EXPECT_TRUE(tools[0].hasSubcommands);
    EXPECT_EQ(tools[0].subcommands, "start,stop,restart");
    GTEST_LOG_(INFO) << "ParseJsonFile_WithSubcommands_100 end";
}

/**
 * Feature: CliToolDataManager
 * Function: ParseJsonFile with UTF-8 BOM
 * SubFunction: NA
 * FunctionPoints: CliToolDataManager ParseJsonFile handles UTF-8 BOM
 */
HWTEST_F(CliToolDataManagerTest, ParseJsonFile_WithBOM_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ParseJsonFile_WithBOM_100 start";
    CliToolDataManager dataManager;

    std::string testFilePath = "/tmp/test_tools_bom.json";
    nlohmann::json j = nlohmann::json::array();
    nlohmann::json tool1;
    tool1["name"] = "test_tool";
    tool1["version"] = "1.0";
    tool1["description"] = "Test tool";
    j.push_back(tool1);

    std::ofstream file(testFilePath, std::ios::binary);
    file << static_cast<char>(0xEF) << static_cast<char>(0xBB) << static_cast<char>(0xBF);
    file << j.dump();
    file.close();

    std::vector<ToolInfo> tools;
    auto result = dataManager.ParseJsonFile(testFilePath, tools);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(tools.size(), 1);
    GTEST_LOG_(INFO) << "ParseJsonFile_WithBOM_100 end";
}

} // namespace CliTool
} // namespace OHOS
