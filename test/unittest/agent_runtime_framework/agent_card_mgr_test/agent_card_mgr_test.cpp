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
#define protected public
#include "ability_manager_errors.h"
#include "agent_card_mgr.h"
#include "mock_my_flag.h"
#undef private
#undef protected

using namespace OHOS;
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AgentRuntime {
namespace {
AgentCard BuildCard(const std::string &agentId, const std::string &version,
    const std::string &bundleName = "test.bundle", const std::string &abilityName = "TestAgent")
{
    AgentCard card;
    card.agentId = agentId;
    card.name = agentId;
    card.description = "desc";
    card.version = version;
    card.category = "productivity";
    card.defaultInputModes = {"text/plain"};
    card.defaultOutputModes = {"text/plain"};
    auto skill = std::make_shared<AgentSkill>();
    skill->id = agentId + "_skill";
    skill->name = "skill";
    skill->description = "skill desc";
    skill->tags = {"tag"};
    card.skills = {skill};
    card.appInfo = std::make_shared<AgentAppInfo>();
    card.appInfo->bundleName = bundleName;
    card.appInfo->moduleName = "testModule";
    card.appInfo->abilityName = abilityName;
    return card;
}

AppExecFwk::ExtensionAbilityInfo BuildAgentExtensionInfo(const std::string &bundleName = "test.bundle",
    const std::string &moduleName = "testModule", const std::string &abilityName = "TestAgent")
{
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.bundleName = bundleName;
    extensionInfo.moduleName = moduleName;
    extensionInfo.name = abilityName;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::AGENT;
    return extensionInfo;
}
}

class AgentCardMgrTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void AgentCardMgrTest::SetUpTestCase(void)
{}

void AgentCardMgrTest::TearDownTestCase(void)
{}

void AgentCardMgrTest::SetUp(void)
{
    MyFlag::retInsertData = 0;
    MyFlag::retDeleteData = 0;
    MyFlag::retQueryData = 0;
    MyFlag::retQueryAllData = 0;
    MyFlag::insertedCards.clear();
    MyFlag::queryDataCards.clear();
    MyFlag::queryAllDataCards.clear();
    MyFlag::retGetBundleInfo = true;
    MyFlag::retGetResConfigFile = true;
    MyFlag::retFromJson = true;
    MyFlag::mockExtensionInfos.clear();
    MyFlag::mockHapModuleInfos.clear();
    MyFlag::mockProfileInfos.clear();
    MyFlag::mockProfileInfoContent.clear();
}

void AgentCardMgrTest::TearDown(void)
{}

/**
 * @tc.name: HandleBundleRemoveTest_001
 * @tc.desc: HandleBundleRemoveTest_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleRemoveTest_001, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    int ret = agentCardMgr.HandleBundleRemove("", 100);
    EXPECT_TRUE(ret == -1);

    MyFlag::retDeleteData = 0;
    ret = agentCardMgr.HandleBundleRemove("test", 100);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: HandleBundleRemoveTest_002
 * @tc.desc: HandleBundleRemove when DeleteData fails
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleRemoveTest_002, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retDeleteData = -1;
    int ret = agentCardMgr.HandleBundleRemove("test", 100);
    EXPECT_TRUE(ret == -1);
}

/**
 * @tc.name: HandleBundleInstallTest_001
 * @tc.desc: HandleBundleInstall with empty bundleName
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleInstallTest_001, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    int ret = agentCardMgr.HandleBundleInstall("", 100);
    EXPECT_TRUE(ret == -1);
}

/**
 * @tc.name: HandleBundleInstallTest_002
 * @tc.desc: HandleBundleInstall when GetBundleInfo fails
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleInstallTest_002, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retGetBundleInfo = false;
    int ret = agentCardMgr.HandleBundleInstall("test", 100);
    EXPECT_TRUE(ret == -1);
}

/**
 * @tc.name: HandleBundleInstallTest_003
 * @tc.desc: HandleBundleInstall success
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleInstallTest_003, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retInsertData = 0;
    int ret = agentCardMgr.HandleBundleInstall("test", 100);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: HandleBundleInstallTest_004
 * @tc.desc: HandleBundleInstall when InsertData fails
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleInstallTest_004, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retInsertData = -1;
    int ret = agentCardMgr.HandleBundleInstall("test", 100);
    EXPECT_TRUE(ret == -1);
}

/**
 * @tc.name: HandleBundleInstallTest_005
 * @tc.desc: HandleBundleInstall with non-AGENT extension type (SERVICE)
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleInstallTest_005, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;

    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.bundleName = "test.bundle";
    extensionInfo.moduleName = "testModule";
    extensionInfo.name = "TestExtension";
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    MyFlag::mockExtensionInfos.push_back(extensionInfo);

    MyFlag::retInsertData = 0;
    int ret = agentCardMgr.HandleBundleInstall("test.bundle", 100);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: HandleBundleInstallTest_006
 * @tc.desc: HandleBundleInstall with non-AGENT extension type (DATASHARE)
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleInstallTest_006, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;

    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.bundleName = "test.bundle";
    extensionInfo.moduleName = "testModule";
    extensionInfo.name = "TestExtension";
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::DATASHARE;
    MyFlag::mockExtensionInfos.push_back(extensionInfo);

    MyFlag::retInsertData = 0;
    int ret = agentCardMgr.HandleBundleInstall("test.bundle", 100);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: HandleBundleInstallTest_007
 * @tc.desc: HandleBundleInstall when GetResConfigFile fails
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleInstallTest_007, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;

    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.bundleName = "test.bundle";
    extensionInfo.moduleName = "testModule";
    extensionInfo.name = "TestAgent";
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::AGENT;
    MyFlag::mockExtensionInfos.push_back(extensionInfo);

    MyFlag::retGetResConfigFile = false;
    int ret = agentCardMgr.HandleBundleInstall("test.bundle", 100);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: HandleBundleInstallTest_008
 * @tc.desc: HandleBundleInstall with invalid JSON in profile
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleInstallTest_008, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;

    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.bundleName = "test.bundle";
    extensionInfo.moduleName = "testModule";
    extensionInfo.name = "TestAgent";
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::AGENT;
    MyFlag::mockExtensionInfos.push_back(extensionInfo);

    MyFlag::mockProfileInfoContent = "invalid json content";
    int ret = agentCardMgr.HandleBundleInstall("test.bundle", 100);
    EXPECT_TRUE(ret == -1);
}

/**
 * @tc.name: HandleBundleInstallTest_009
 * @tc.desc: HandleBundleInstall with valid JSON but missing agentCards field
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleInstallTest_009, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;

    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.bundleName = "test.bundle";
    extensionInfo.moduleName = "testModule";
    extensionInfo.name = "TestAgent";
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::AGENT;
    MyFlag::mockExtensionInfos.push_back(extensionInfo);

    MyFlag::mockProfileInfoContent = R"({"otherField": "value"})";
    int ret = agentCardMgr.HandleBundleInstall("test.bundle", 100);
    EXPECT_TRUE(ret == -1);
}

/**
 * @tc.name: HandleBundleInstallTest_010
 * @tc.desc: HandleBundleInstall with empty agentCards array
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleInstallTest_010, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;

    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.bundleName = "test.bundle";
    extensionInfo.moduleName = "testModule";
    extensionInfo.name = "TestAgent";
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::AGENT;
    MyFlag::mockExtensionInfos.push_back(extensionInfo);

    MyFlag::mockProfileInfoContent = R"({"agentCards": []})";
    MyFlag::retInsertData = 0;
    int ret = agentCardMgr.HandleBundleInstall("test.bundle", 100);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: HandleBundleInstallTest_011
 * @tc.desc: HandleBundleInstall when FromJson fails (continue branch)
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleInstallTest_011, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;

    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.bundleName = "test.bundle";
    extensionInfo.moduleName = "testModule";
    extensionInfo.name = "TestAgent";
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::AGENT;
    MyFlag::mockExtensionInfos.push_back(extensionInfo);

    MyFlag::mockProfileInfoContent = R"({"agentCards": [{}]})";
    MyFlag::retFromJson = false;
    MyFlag::retInsertData = 0;
    int ret = agentCardMgr.HandleBundleInstall("test.bundle", 100);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: HandleBundleInstallTest_012
 * @tc.desc: HandleBundleInstall with deviceTypes in card (non-empty hapModuleInfo)
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleInstallTest_012, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;

    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.bundleName = "test.bundle";
    extensionInfo.moduleName = "testModule";
    extensionInfo.name = "TestAgent";
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::AGENT;
    MyFlag::mockExtensionInfos.push_back(extensionInfo);

    AppExecFwk::HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "testModule";
    hapModuleInfo.deviceTypes = {"phone", "tablet"};
    MyFlag::mockHapModuleInfos.push_back(hapModuleInfo);

    MyFlag::mockProfileInfoContent = R"({
        "agentCards": [{
            "agentId": "testAgent",
            "name": "Test Agent",
            "description": "Test Description",
            "version": "1.0.0",
            "category": "productivity",
            "defaultInputModes": ["text"],
            "defaultOutputModes": ["text"],
            "appInfo": {
                "deviceTypes": ["phone", "tablet", "watch"]
            }
        }]
    })";
    MyFlag::retFromJson = true;
    MyFlag::retInsertData = 0;
    int ret = agentCardMgr.HandleBundleInstall("test.bundle", 100);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: HandleBundleInstallTest_013
 * @tc.desc: HandleBundleInstall with deviceTypes filtering (intersection)
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleInstallTest_013, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;

    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.bundleName = "test.bundle";
    extensionInfo.moduleName = "testModule";
    extensionInfo.name = "TestAgent";
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::AGENT;
    MyFlag::mockExtensionInfos.push_back(extensionInfo);

    AppExecFwk::HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "testModule";
    hapModuleInfo.deviceTypes = {"phone", "tablet"};
    MyFlag::mockHapModuleInfos.push_back(hapModuleInfo);

    // Card has phone, tablet, watch - hap only has phone, tablet - should filter to phone, tablet
    MyFlag::mockProfileInfoContent = R"({
        "agentCards": [{
            "agentId": "testAgent",
            "name": "Test Agent",
            "description": "Test Description",
            "version": "1.0.0",
            "category": "productivity",
            "defaultInputModes": ["text"],
            "defaultOutputModes": ["text"],
            "appInfo": {
                "deviceTypes": ["phone", "tablet", "watch"]
            }
        }]
    })";
    MyFlag::retFromJson = true;
    MyFlag::retInsertData = 0;
    int ret = agentCardMgr.HandleBundleInstall("test.bundle", 100);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: HandleBundleInstallTest_014
 * @tc.desc: HandleBundleInstall when deviceTypes is empty (fallback to hapModuleInfo)
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleInstallTest_014, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;

    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.bundleName = "test.bundle";
    extensionInfo.moduleName = "testModule";
    extensionInfo.name = "TestAgent";
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::AGENT;
    MyFlag::mockExtensionInfos.push_back(extensionInfo);

    AppExecFwk::HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "testModule";
    hapModuleInfo.deviceTypes = {"phone", "tablet"};
    MyFlag::mockHapModuleInfos.push_back(hapModuleInfo);

    // Card has no deviceTypes - should use hapModuleInfo's deviceTypes
    MyFlag::mockProfileInfoContent = R"({
        "agentCards": [{
            "agentId": "testAgent",
            "name": "Test Agent",
            "description": "Test Description",
            "version": "1.0.0",
            "category": "productivity",
            "defaultInputModes": ["text"],
            "defaultOutputModes": ["text"],
            "appInfo": {}
        }]
    })";
    MyFlag::retFromJson = true;
    MyFlag::retInsertData = 0;
    int ret = agentCardMgr.HandleBundleInstall("test.bundle", 100);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: HandleBundleInstallTest_015
 * @tc.desc: HandleBundleInstall with no matching hapModuleInfo
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleInstallTest_015, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;

    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.bundleName = "test.bundle";
    extensionInfo.moduleName = "testModule";
    extensionInfo.name = "TestAgent";
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::AGENT;
    MyFlag::mockExtensionInfos.push_back(extensionInfo);

    // Empty hapModuleInfos - no matching module
    MyFlag::mockHapModuleInfos.clear();

    MyFlag::mockProfileInfoContent = R"({
        "agentCards": [{
            "agentId": "testAgent",
            "name": "Test Agent",
            "description": "Test Description",
            "version": "1.0.0",
            "category": "productivity",
            "defaultInputModes": ["text"],
            "defaultOutputModes": ["text"],
            "appInfo": {}
        }]
    })";
    MyFlag::retFromJson = true;
    MyFlag::retInsertData = 0;
    int ret = agentCardMgr.HandleBundleInstall("test.bundle", 100);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: HandleBundleInstallTest_016
 * @tc.desc: HandleBundleInstall deduplicates duplicate agentId entries with last-write-wins
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleInstallTest_016, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;

    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.bundleName = "test.bundle";
    extensionInfo.moduleName = "testModule";
    extensionInfo.name = "TestAgent";
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::AGENT;
    MyFlag::mockExtensionInfos.push_back(extensionInfo);

    AppExecFwk::HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "testModule";
    hapModuleInfo.deviceTypes = {"phone", "tablet"};
    MyFlag::mockHapModuleInfos.push_back(hapModuleInfo);

    MyFlag::mockProfileInfoContent = R"({
        "agentCards": [{
            "agentId": "dupAgent",
            "name": "First Agent",
            "description": "First Description",
            "version": "1.0.0",
            "category": "productivity",
            "defaultInputModes": ["text"],
            "defaultOutputModes": ["text"],
            "appInfo": {
                "deviceTypes": ["phone"]
            }
        }, {
            "agentId": "dupAgent",
            "name": "Second Agent",
            "description": "Second Description",
            "version": "2.0.0",
            "category": "assistant",
            "defaultInputModes": ["voice"],
            "defaultOutputModes": ["text"],
            "appInfo": {
                "deviceTypes": ["tablet"]
            }
        }]
    })";

    int ret = agentCardMgr.HandleBundleInstall("test.bundle", 100);
    EXPECT_EQ(ret, 0);
    ASSERT_EQ(MyFlag::insertedCards.size(), 1);
    EXPECT_EQ(MyFlag::insertedCards[0].agentId, "dupAgent");
    EXPECT_EQ(MyFlag::insertedCards[0].name, "Second Agent");
    EXPECT_EQ(MyFlag::insertedCards[0].description, "Second Description");
    ASSERT_NE(MyFlag::insertedCards[0].appInfo, nullptr);
    ASSERT_EQ(MyFlag::insertedCards[0].appInfo->deviceTypes.size(), 1);
    EXPECT_EQ(MyFlag::insertedCards[0].appInfo->deviceTypes[0], "tablet");
}

/**
 * @tc.name: HandleBundleUpdateTest_001
 * @tc.desc: HandleBundleUpdate
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleUpdateTest_001, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retInsertData = 0;
    int ret = agentCardMgr.HandleBundleUpdate("test", 100);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: GetAllAgentCardsTest_001
 * @tc.desc: GetAllAgentCards success
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, GetAllAgentCardsTest_001, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retQueryAllData = 0;
    AgentCardsRawData cards;
    int ret = agentCardMgr.GetAllAgentCards(cards);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: GetAllAgentCardsTest_002
 * @tc.desc: GetAllAgentCards when QueryAllData fails
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, GetAllAgentCardsTest_002, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retQueryAllData = -1;
    AgentCardsRawData cards;
    int ret = agentCardMgr.GetAllAgentCards(cards);
    EXPECT_TRUE(ret == -1);
}

/**
 * @tc.name: GetAgentCardsByBundleNameTest_001
 * @tc.desc: GetAgentCardsByBundleName success
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, GetAgentCardsByBundleNameTest_001, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retQueryData = 0;
    std::vector<AgentCard> cards;
    int ret = agentCardMgr.GetAgentCardsByBundleName("test", cards);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: GetAgentCardsByBundleNameTest_002
 * @tc.desc: GetAgentCardsByBundleName when QueryData fails
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, GetAgentCardsByBundleNameTest_002, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retQueryData = -1;
    std::vector<AgentCard> cards;
    int ret = agentCardMgr.GetAgentCardsByBundleName("test", cards);
    EXPECT_TRUE(ret == -1);
}

/**
 * @tc.name: GetAgentCardByAgentId_001
 * @tc.desc: GetAgentCardByAgentId when QueryData fails
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, GetAgentCardByAgentId_001, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retQueryData = -1;
    AgentCard card;
    int ret = agentCardMgr.GetAgentCardByAgentId("test", "test", card);
    EXPECT_TRUE(ret != 0);
}

/**
 * @tc.name: GetAgentCardByAgentId_002
 * @tc.desc: GetAgentCardByAgentId when agentId not found
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, GetAgentCardByAgentId_002, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retQueryData = 0;
    MyFlag::queryDataCards.clear();
    AgentCard card;
    int ret = agentCardMgr.GetAgentCardByAgentId("test", "test", card);
    EXPECT_TRUE(ret == ERR_NAME_NOT_FOUND);
}

/**
 * @tc.name: GetAgentCardByAgentId_003
 * @tc.desc: GetAgentCardByAgentId success
 * @tc.type:: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, GetAgentCardByAgentId_003, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retQueryData = 0;
    AgentCard testCard;
    testCard.agentId = "test";
    MyFlag::queryDataCards.push_back(testCard);
    AgentCard card;
    int ret = agentCardMgr.GetAgentCardByAgentId("test", "test", card);
    EXPECT_TRUE(ret == 0);
    EXPECT_TRUE(card.agentId == "test");
}

/**
 * @tc.name: GetAgentCardByAgentId_004
 * @tc.desc: GetAgentCardByAgentId when card exists but agentId doesn't match
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, GetAgentCardByAgentId_004, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retQueryData = 0;
    AgentCard testCard;
    testCard.agentId = "otherAgent";
    MyFlag::queryDataCards.push_back(testCard);
    AgentCard card;
    int ret = agentCardMgr.GetAgentCardByAgentId("test", "test", card);
    EXPECT_TRUE(ret == ERR_NAME_NOT_FOUND);
}

/**
 * @tc.name: HandleBundleInstallTest_017
 * @tc.desc: HandleBundleInstall keeps stored card when incoming version is older
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, HandleBundleInstallTest_017, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::mockExtensionInfos.push_back(BuildAgentExtensionInfo());
    MyFlag::mockProfileInfoContent = R"({
        "agentCards": [{
            "agentId": "testAgent",
            "name": "Incoming Agent",
            "description": "Incoming Description",
            "version": "1.0.0",
            "category": "productivity",
            "defaultInputModes": ["text"],
            "defaultOutputModes": ["text"],
            "appInfo": {}
        }]
    })";
    MyFlag::retQueryData = ERR_OK;
    MyFlag::queryDataCards = { BuildCard("testAgent", "2.0.0") };

    int ret = agentCardMgr.HandleBundleInstall("test.bundle", 100);
    EXPECT_EQ(ret, ERR_OK);
    ASSERT_EQ(MyFlag::insertedCards.size(), 1);
    EXPECT_EQ(MyFlag::insertedCards[0].version, "2.0.0");
}

/**
 * @tc.name: HandleBundleInstallTest_018
 * @tc.desc: HandleBundleInstall removes cards absent from the new package
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, HandleBundleInstallTest_018, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::mockExtensionInfos.push_back(BuildAgentExtensionInfo());
    MyFlag::mockProfileInfoContent = R"({
        "agentCards": [{
            "agentId": "incomingOnly",
            "name": "Incoming Agent",
            "description": "Incoming Description",
            "version": "1.0.0",
            "category": "productivity",
            "defaultInputModes": ["text"],
            "defaultOutputModes": ["text"],
            "appInfo": {}
        }]
    })";
    MyFlag::retQueryData = ERR_OK;
    MyFlag::queryDataCards = { BuildCard("storedOnly", "2.0.0") };

    int ret = agentCardMgr.HandleBundleInstall("test.bundle", 100);
    EXPECT_EQ(ret, ERR_OK);
    ASSERT_EQ(MyFlag::insertedCards.size(), 1);
    EXPECT_EQ(MyFlag::insertedCards[0].agentId, "incomingOnly");
}

/**
 * @tc.name: UpdateAgentCard_001
 * @tc.desc: UpdateAgentCard returns INVALID_PARAMETERS_ERR when ids mismatch
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, UpdateAgentCard_001, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    AgentCard card = BuildCard("cardInBody", "1.0.0");
    EXPECT_EQ(agentCardMgr.UpdateAgentCard("cardInArg", card), AAFwk::INVALID_PARAMETERS_ERR);
}

/**
 * @tc.name: RegisterAgentCard_001
 * @tc.desc: RegisterAgentCard returns INVALID_PARAMETERS_ERR when required fields are missing
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, RegisterAgentCard_001, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    AgentCard card;
    EXPECT_EQ(agentCardMgr.RegisterAgentCard(card), AAFwk::INVALID_PARAMETERS_ERR);
}

/**
 * @tc.name: RegisterAgentCard_002
 * @tc.desc: RegisterAgentCard returns invalid version error when semver is invalid
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, RegisterAgentCard_002, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    AgentCard card = BuildCard("testAgent", "invalid-version");
    EXPECT_EQ(agentCardMgr.RegisterAgentCard(card), AAFwk::ERR_INVALID_AGENT_CARD_VERSION);
}

/**
 * @tc.name: RegisterAgentCard_003
 * @tc.desc: RegisterAgentCard returns ERR_BUNDLE_NOT_EXIST when bundle does not exist
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, RegisterAgentCard_003, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retGetBundleInfo = false;
    AgentCard card = BuildCard("testAgent", "1.0.0");
    EXPECT_EQ(agentCardMgr.RegisterAgentCard(card), AAFwk::ERR_BUNDLE_NOT_EXIST);
}

/**
 * @tc.name: RegisterAgentCard_004
 * @tc.desc: RegisterAgentCard returns duplicate error when agentId already exists
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, RegisterAgentCard_004, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::mockExtensionInfos.push_back(BuildAgentExtensionInfo());
    MyFlag::retQueryData = ERR_OK;
    MyFlag::queryDataCards = { BuildCard("testAgent", "1.0.0") };
    AgentCard card = BuildCard("testAgent", "1.0.0");
    EXPECT_EQ(agentCardMgr.RegisterAgentCard(card), AAFwk::ERR_AGENT_CARD_DUPLICATE_REGISTER);
}

/**
 * @tc.name: RegisterAgentCard_005
 * @tc.desc: RegisterAgentCard persists a new card successfully
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, RegisterAgentCard_005, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::mockExtensionInfos.push_back(BuildAgentExtensionInfo());
    MyFlag::retQueryData = ERR_NAME_NOT_FOUND;
    AgentCard card = BuildCard("testAgent", "1.0.0");

    int ret = agentCardMgr.RegisterAgentCard(card);
    EXPECT_EQ(ret, ERR_OK);
    ASSERT_EQ(MyFlag::insertedCards.size(), 1);
    EXPECT_EQ(MyFlag::insertedCards[0].agentId, "testAgent");
}

/**
 * @tc.name: RegisterAgentCard_006
 * @tc.desc: RegisterAgentCard returns RESOLVE_ABILITY_ERR when ability does not exist
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, RegisterAgentCard_006, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    AgentCard card = BuildCard("testAgent", "1.0.0");
    EXPECT_EQ(agentCardMgr.RegisterAgentCard(card), AAFwk::RESOLVE_ABILITY_ERR);
}

/**
 * @tc.name: RegisterAgentCard_007
 * @tc.desc: RegisterAgentCard returns ERR_WRONG_INTERFACE_CALL when ability type is incorrect
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, RegisterAgentCard_007, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    auto extensionInfo = BuildAgentExtensionInfo();
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    MyFlag::mockExtensionInfos.push_back(extensionInfo);
    AgentCard card = BuildCard("testAgent", "1.0.0");
    EXPECT_EQ(agentCardMgr.RegisterAgentCard(card), AAFwk::ERR_WRONG_INTERFACE_CALL);
}

/**
 * @tc.name: UpdateAgentCard_002
 * @tc.desc: UpdateAgentCard returns ERR_INVALID_AGENT_CARD_VERSION when semver is invalid
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, UpdateAgentCard_002, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    AgentCard card = BuildCard("testAgent", "1.0");
    EXPECT_EQ(agentCardMgr.UpdateAgentCard("testAgent", card), AAFwk::ERR_INVALID_AGENT_CARD_VERSION);
}

/**
 * @tc.name: UpdateAgentCard_003
 * @tc.desc: UpdateAgentCard returns ERR_BUNDLE_NOT_EXIST when bundle does not exist
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, UpdateAgentCard_003, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retGetBundleInfo = false;
    AgentCard card = BuildCard("testAgent", "1.0.0");
    EXPECT_EQ(agentCardMgr.UpdateAgentCard("testAgent", card), AAFwk::ERR_BUNDLE_NOT_EXIST);
}

/**
 * @tc.name: UpdateAgentCard_004
 * @tc.desc: UpdateAgentCard returns ERR_INVALID_AGENT_CARD_ID when stored card is missing
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, UpdateAgentCard_004, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::mockExtensionInfos.push_back(BuildAgentExtensionInfo());
    MyFlag::retQueryData = ERR_NAME_NOT_FOUND;
    AgentCard card = BuildCard("testAgent", "1.0.0");
    EXPECT_EQ(agentCardMgr.UpdateAgentCard("testAgent", card), AAFwk::ERR_INVALID_AGENT_CARD_ID);
}

/**
 * @tc.name: UpdateAgentCard_0041
 * @tc.desc: UpdateAgentCard returns RESOLVE_ABILITY_ERR when ability does not exist
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, UpdateAgentCard_0041, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retQueryData = ERR_OK;
    MyFlag::queryDataCards = { BuildCard("testAgent", "1.0.0") };
    AgentCard card = BuildCard("testAgent", "1.0.0");
    EXPECT_EQ(agentCardMgr.UpdateAgentCard("testAgent", card), AAFwk::RESOLVE_ABILITY_ERR);
}

/**
 * @tc.name: UpdateAgentCard_0042
 * @tc.desc: UpdateAgentCard returns ERR_WRONG_INTERFACE_CALL when ability type is incorrect
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, UpdateAgentCard_0042, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    auto storedCard = BuildCard("testAgent", "1.0.0");
    MyFlag::retQueryData = ERR_OK;
    MyFlag::queryDataCards = { storedCard };
    auto extensionInfo = BuildAgentExtensionInfo();
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    MyFlag::mockExtensionInfos.push_back(extensionInfo);
    AgentCard card = BuildCard("testAgent", "1.0.0");
    EXPECT_EQ(agentCardMgr.UpdateAgentCard("testAgent", card), AAFwk::ERR_WRONG_INTERFACE_CALL);
}

/**
 * @tc.name: UpdateAgentCard_005
 * @tc.desc: UpdateAgentCard returns downgrade error when version is older than stored
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, UpdateAgentCard_005, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::mockExtensionInfos.push_back(BuildAgentExtensionInfo());
    MyFlag::retQueryData = ERR_OK;
    MyFlag::queryDataCards = { BuildCard("testAgent", "2.0.0") };
    AgentCard card = BuildCard("testAgent", "1.0.0");
    EXPECT_EQ(agentCardMgr.UpdateAgentCard("testAgent", card), AAFwk::ERR_AGENT_CARD_VERSION_TOO_OLD);
}

/**
 * @tc.name: UpdateAgentCard_0051
 * @tc.desc: UpdateAgentCard returns invalid version error when incoming version is invalid
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, UpdateAgentCard_0051, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    AgentCard card = BuildCard("testAgent", "invalid-version");
    EXPECT_EQ(agentCardMgr.UpdateAgentCard("testAgent", card), AAFwk::ERR_INVALID_AGENT_CARD_VERSION);
}

/**
 * @tc.name: UpdateAgentCard_0052
 * @tc.desc: UpdateAgentCard allows overwrite when stored version is invalid and incoming version is valid
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, UpdateAgentCard_0052, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::mockExtensionInfos.push_back(BuildAgentExtensionInfo());
    MyFlag::retQueryData = ERR_OK;
    MyFlag::queryDataCards = { BuildCard("testAgent", "invalid-version") };
    AgentCard card = BuildCard("testAgent", "1.0.0");
    card.description = "updated from valid version";

    int ret = agentCardMgr.UpdateAgentCard("testAgent", card);
    EXPECT_EQ(ret, ERR_OK);
    ASSERT_EQ(MyFlag::insertedCards.size(), 1);
    EXPECT_EQ(MyFlag::insertedCards[0].version, "1.0.0");
    EXPECT_EQ(MyFlag::insertedCards[0].description, "updated from valid version");
}

/**
 * @tc.name: UpdateAgentCard_006
 * @tc.desc: UpdateAgentCard succeeds for equal version and persists the replacement
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, UpdateAgentCard_006, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::mockExtensionInfos.push_back(BuildAgentExtensionInfo());
    MyFlag::retQueryData = ERR_OK;
    MyFlag::queryDataCards = { BuildCard("testAgent", "2.0.0") };
    AgentCard card = BuildCard("testAgent", "2.0.0");
    card.description = "updated";

    int ret = agentCardMgr.UpdateAgentCard("testAgent", card);
    EXPECT_EQ(ret, ERR_OK);
    ASSERT_EQ(MyFlag::insertedCards.size(), 1);
    EXPECT_EQ(MyFlag::insertedCards[0].description, "updated");
}

/**
 * @tc.name: DeleteAgentCard_001
 * @tc.desc: DeleteAgentCard returns INVALID_PARAMETERS_ERR when id is empty
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, DeleteAgentCard_001, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    EXPECT_EQ(agentCardMgr.DeleteAgentCard("", "testAgent"), AAFwk::INVALID_PARAMETERS_ERR);
}

/**
 * @tc.name: DeleteAgentCard_002
 * @tc.desc: DeleteAgentCard returns query error when QueryAllData fails
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, DeleteAgentCard_002, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retQueryData = ERR_INVALID_OPERATION;
    EXPECT_EQ(agentCardMgr.DeleteAgentCard("test.bundle", "testAgent"), ERR_INVALID_OPERATION);
}

/**
 * @tc.name: DeleteAgentCard_003
 * @tc.desc: DeleteAgentCard returns ERR_INVALID_AGENT_CARD_ID when the target is missing
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, DeleteAgentCard_003, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retQueryData = ERR_OK;
    MyFlag::queryDataCards = { BuildCard("anotherAgent", "1.0.0") };
    EXPECT_EQ(agentCardMgr.DeleteAgentCard("test.bundle", "testAgent"), AAFwk::ERR_INVALID_AGENT_CARD_ID);
}

/**
 * @tc.name: DeleteAgentCard_004
 * @tc.desc: DeleteAgentCard removes only the matching card and persists remaining bundle cards
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, DeleteAgentCard_004, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retQueryData = ERR_OK;
    MyFlag::queryDataCards = { BuildCard("testAgent", "1.0.0"), BuildCard("remainAgent", "1.0.0") };

    int ret = agentCardMgr.DeleteAgentCard("test.bundle", "testAgent");
    EXPECT_EQ(ret, ERR_OK);
    ASSERT_EQ(MyFlag::insertedCards.size(), 1);
    EXPECT_EQ(MyFlag::insertedCards[0].agentId, "remainAgent");
}

/**
 * @tc.name: DeleteAgentCard_005
 * @tc.desc: DeleteAgentCard deletes the bundle entry when the removed card is the last one
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardMgrTest, DeleteAgentCard_005, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retQueryData = ERR_OK;
    MyFlag::queryDataCards = { BuildCard("testAgent", "1.0.0") };
    MyFlag::retDeleteData = ERR_OK;

    int ret = agentCardMgr.DeleteAgentCard("test.bundle", "testAgent");
    EXPECT_EQ(ret, ERR_OK);
}
} // namespace AgentRuntime
} // namespace OHOS
