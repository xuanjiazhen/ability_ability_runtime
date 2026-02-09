/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "ability_manager_errors.h"
#include "agent_card.h"
#include "securec.h"

using namespace OHOS;
using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AgentRuntime {
class AgentCardTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void AgentCardTest::SetUpTestCase(void)
{}

void AgentCardTest::TearDownTestCase(void)
{}

void AgentCardTest::SetUp(void)
{}

void AgentCardTest::TearDown(void)
{}

/**
 * @tc.name: ProviderMarshallingTest_001
 * @tc.desc: Test AgentProvider Marshalling method with valid data
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderMarshallingTest_001, TestSize.Level1)
{
    AgentProvider provider;
    provider.organization = "test1";
    provider.url = "abc";

    Parcel parcelMock;
    bool result = provider.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: ProviderMarshallingTest_002
 * @tc.desc: Test Provider Marshalling method with empty strings
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderMarshallingTest_002, TestSize.Level1)
{
    AgentProvider provider;
    provider.organization = "";
    provider.url = "";

    Parcel parcelMock;
    bool result = provider.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: ProviderUnmarshalling_001
 * @tc.desc: ProviderUnmarshalling_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderUnmarshalling_001, TestSize.Level1)
{
    Parcel parcelMock;
    AgentProvider *provider = AgentProvider::Unmarshalling(parcelMock);

    EXPECT_TRUE(provider != nullptr);
    delete provider;
}

/**
 * @tc.name: CapabilitiesMarshalling_001
 * @tc.desc: CapabilitiesMarshalling_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, CapabilitiesMarshalling_001, TestSize.Level1)
{
    AgentCapabilities capabilities;
    capabilities.streaming = true;
    capabilities.pushNotifications = true;
    capabilities.stateTransitionHistory = true;
    capabilities.extension = "test";

    Parcel parcelMock;
    bool result = capabilities.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: CapabilitiesMarshalling_002
 * @tc.desc: CapabilitiesMarshalling_002 - test with false values
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, CapabilitiesMarshalling_002, TestSize.Level1)
{
    AgentCapabilities capabilities;
    capabilities.streaming = false;
    capabilities.pushNotifications = false;
    capabilities.stateTransitionHistory = false;
    capabilities.extension = "test";

    Parcel parcelMock;
    bool result = capabilities.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: CapabilitiesMarshalling_003
 * @tc.desc: CapabilitiesMarshalling_003 - test with mixed bool values
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, CapabilitiesMarshalling_003, TestSize.Level1)
{
    AgentCapabilities capabilities;
    capabilities.streaming = true;
    capabilities.pushNotifications = false;
    capabilities.stateTransitionHistory = true;
    capabilities.extension = "test";

    Parcel parcelMock;
    bool result = capabilities.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: CapabilitiesMarshalling_004
 * @tc.desc: CapabilitiesMarshalling_004 - test with empty extension
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, CapabilitiesMarshalling_004, TestSize.Level1)
{
    AgentCapabilities capabilities;
    capabilities.streaming = true;
    capabilities.pushNotifications = true;
    capabilities.stateTransitionHistory = true;
    capabilities.extension = "";

    Parcel parcelMock;
    bool result = capabilities.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: CapabilitiesUnmarshalling_001
 * @tc.desc: CapabilitiesUnmarshalling_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, CapabilitiesUnmarshalling_001, TestSize.Level1)
{
    Parcel parcelMock;
    AgentCapabilities *capabilities = AgentCapabilities::Unmarshalling(parcelMock);

    EXPECT_TRUE(capabilities != nullptr);
    delete capabilities;
}

/**
 * @tc.name: SkillMarshalling_001
 * @tc.desc: SkillMarshalling_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillMarshalling_001, TestSize.Level1)
{
    AgentSkill skill;
    skill.id = "test1";
    skill.name = "test1";
    skill.description = "test1";
    std::vector<std::string> tags;
    tags.push_back("test");
    skill.tags = tags;
    std::vector<std::string> examples;
    examples.push_back("test");
    skill.examples = examples;
    std::vector<std::string> inputModes;
    inputModes.push_back("test");
    skill.inputModes = inputModes;
    std::vector<std::string> outputModes;
    outputModes.push_back("test");
    skill.outputModes = outputModes;
    skill.extension = "test";

    Parcel parcelMock;
    bool result = skill.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: SkillMarshalling_002
 * @tc.desc: SkillMarshalling_002 - test with empty id
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillMarshalling_002, TestSize.Level1)
{
    AgentSkill skill;
    skill.id = "";
    skill.name = "test1";
    skill.description = "test1";
    std::vector<std::string> tags;
    tags.push_back("test");
    skill.tags = tags;
    std::vector<std::string> examples;
    examples.push_back("test");
    skill.examples = examples;
    std::vector<std::string> inputModes;
    inputModes.push_back("test");
    skill.inputModes = inputModes;
    std::vector<std::string> outputModes;
    outputModes.push_back("test");
    skill.outputModes = outputModes;
    skill.extension = "test";

    Parcel parcelMock;
    bool result = skill.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: SkillMarshalling_003
 * @tc.desc: SkillMarshalling_003 - test with empty name
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillMarshalling_003, TestSize.Level1)
{
    AgentSkill skill;
    skill.id = "test1";
    skill.name = "";
    skill.description = "test1";
    std::vector<std::string> tags;
    tags.push_back("test");
    skill.tags = tags;
    std::vector<std::string> examples;
    examples.push_back("test");
    skill.examples = examples;
    std::vector<std::string> inputModes;
    inputModes.push_back("test");
    skill.inputModes = inputModes;
    std::vector<std::string> outputModes;
    outputModes.push_back("test");
    skill.outputModes = outputModes;
    skill.extension = "test";

    Parcel parcelMock;
    bool result = skill.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: SkillMarshalling_004
 * @tc.desc: SkillMarshalling_004 - test with empty description
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillMarshalling_004, TestSize.Level1)
{
    AgentSkill skill;
    skill.id = "test1";
    skill.name = "test1";
    skill.description = "";
    std::vector<std::string> tags;
    tags.push_back("test");
    skill.tags = tags;
    std::vector<std::string> examples;
    examples.push_back("test");
    skill.examples = examples;
    std::vector<std::string> inputModes;
    inputModes.push_back("test");
    skill.inputModes = inputModes;
    std::vector<std::string> outputModes;
    outputModes.push_back("test");
    skill.outputModes = outputModes;
    skill.extension = "test";

    Parcel parcelMock;
    bool result = skill.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: SkillMarshalling_005
 * @tc.desc: SkillMarshalling_005 - test with empty tags
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillMarshalling_005, TestSize.Level1)
{
    AgentSkill skill;
    skill.id = "test1";
    skill.name = "test1";
    skill.description = "test1";
    std::vector<std::string> tags;
    skill.tags = tags;
    std::vector<std::string> examples;
    examples.push_back("test");
    skill.examples = examples;
    std::vector<std::string> inputModes;
    inputModes.push_back("test");
    skill.inputModes = inputModes;
    std::vector<std::string> outputModes;
    outputModes.push_back("test");
    skill.outputModes = outputModes;
    skill.extension = "test";

    Parcel parcelMock;
    bool result = skill.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: SkillMarshalling_006
 * @tc.desc: SkillMarshalling_006 - test with empty examples
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillMarshalling_006, TestSize.Level1)
{
    AgentSkill skill;
    skill.id = "test1";
    skill.name = "test1";
    skill.description = "test1";
    std::vector<std::string> tags;
    tags.push_back("test");
    skill.tags = tags;
    std::vector<std::string> examples;
    skill.examples = examples;
    std::vector<std::string> inputModes;
    inputModes.push_back("test");
    skill.inputModes = inputModes;
    std::vector<std::string> outputModes;
    outputModes.push_back("test");
    skill.outputModes = outputModes;
    skill.extension = "test";

    Parcel parcelMock;
    bool result = skill.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: SkillMarshalling_007
 * @tc.desc: SkillMarshalling_007 - test with empty inputModes
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillMarshalling_007, TestSize.Level1)
{
    AgentSkill skill;
    skill.id = "test1";
    skill.name = "test1";
    skill.description = "test1";
    std::vector<std::string> tags;
    tags.push_back("test");
    skill.tags = tags;
    std::vector<std::string> examples;
    examples.push_back("test");
    skill.examples = examples;
    std::vector<std::string> inputModes;
    skill.inputModes = inputModes;
    std::vector<std::string> outputModes;
    outputModes.push_back("test");
    skill.outputModes = outputModes;
    skill.extension = "test";

    Parcel parcelMock;
    bool result = skill.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: SkillMarshalling_008
 * @tc.desc: SkillMarshalling_008 - test with empty outputModes
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillMarshalling_008, TestSize.Level1)
{
    AgentSkill skill;
    skill.id = "test1";
    skill.name = "test1";
    skill.description = "test1";
    std::vector<std::string> tags;
    tags.push_back("test");
    skill.tags = tags;
    std::vector<std::string> examples;
    examples.push_back("test");
    skill.examples = examples;
    std::vector<std::string> inputModes;
    inputModes.push_back("test");
    skill.inputModes = inputModes;
    std::vector<std::string> outputModes;
    skill.outputModes = outputModes;
    skill.extension = "test";

    Parcel parcelMock;
    bool result = skill.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: SkillUnmarshalling_001
 * @tc.desc: SkillUnmarshalling_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillUnmarshalling_001, TestSize.Level1)
{
    Parcel parcelMock;
    AgentSkill *skill = AgentSkill::Unmarshalling(parcelMock);

    EXPECT_TRUE(skill == nullptr);
    delete skill;
}

/**
 * @tc.name: AgentCardReadFromParcel_001
 * @tc.desc: AgentCardReadFromParcel_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardReadFromParcel_001, TestSize.Level1)
{
    AgentCard agentCard;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    Parcel parcelMock;
    bool result = agentCard.ReadFromParcel(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: AgentCardReadFromParcel_002
 * @tc.desc: AgentCardReadFromParcel_002
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardReadFromParcel_002, TestSize.Level1)
{
    AgentCard agentCard;
    std::vector<std::string> defaultInputModes;
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    Parcel parcelMock;
    bool result = agentCard.ReadFromParcel(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: AgentCardReadFromParcel_003
 * @tc.desc: AgentCardReadFromParcel_003
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardReadFromParcel_003, TestSize.Level1)
{
    AgentCard agentCard;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    agentCard.defaultOutputModes = defaultOutputModes;
    Parcel parcelMock;
    bool result = agentCard.ReadFromParcel(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: AgentCardMarshalling_001
 * @tc.desc: AgentCardMarshalling_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_001, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test1";
    agentCard.description = "test1";
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    agentCard.version = "test1";
    agentCard.documentationUrl = "test1";
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;
    agentCard.category = "test";
    agentCard.extension = "test";
    agentCard.iconUrl = "test";

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: AgentCardMarshalling_002
 * @tc.desc: AgentCardMarshalling_002
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_002, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test";
    agentCard.description = "test1";
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    agentCard.version = "test1";
    agentCard.documentationUrl = "test1";
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;
    agentCard.category = "test";
    agentCard.extension = "test";
    agentCard.iconUrl = "test";

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: AgentCardMarshalling_003
 * @tc.desc: AgentCardMarshalling_003
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_003, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test1";
    agentCard.description = "test";
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    agentCard.version = "test1";
    agentCard.documentationUrl = "test1";
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;
    agentCard.category = "test";
    agentCard.extension = "test";
    agentCard.iconUrl = "test";

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: AgentCardMarshalling_004
 * @tc.desc: AgentCardMarshalling_004
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_004, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test1";
    agentCard.description = "test1";
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    agentCard.version = "test1";
    agentCard.documentationUrl = "test1";
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;
    agentCard.category = "test";
    agentCard.extension = "test";
    agentCard.iconUrl = "test";

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: AgentCardMarshalling_005
 * @tc.desc: AgentCardMarshalling_005
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_005, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test1";
    agentCard.description = "test1";
    std::shared_ptr<AgentProvider> provider = nullptr;
    agentCard.provider = provider;
    agentCard.version = "test1";
    agentCard.documentationUrl = "test1";
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;
    agentCard.category = "test";
    agentCard.extension = "test";
    agentCard.iconUrl = "test";

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: AgentCardMarshalling_006
 * @tc.desc: AgentCardMarshalling_006
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_006, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test1";
    agentCard.description = "test1";
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    agentCard.version = "test";
    agentCard.documentationUrl = "test1";
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;
    agentCard.category = "test";
    agentCard.extension = "test";
    agentCard.iconUrl = "test";

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: AgentCardMarshalling_007
 * @tc.desc: AgentCardMarshalling_007
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_007, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test1";
    agentCard.description = "test1";
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    agentCard.version = "test1";
    agentCard.documentationUrl = "test";
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;
    agentCard.category = "test";
    agentCard.extension = "test";
    agentCard.iconUrl = "test";

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: AgentCardMarshalling_008
 * @tc.desc: AgentCardMarshalling_008
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_008, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test1";
    agentCard.description = "test1";
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    agentCard.version = "test1";
    agentCard.documentationUrl = "test1";
    std::shared_ptr<AgentCapabilities> capabilities = nullptr;
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;
    agentCard.category = "test";
    agentCard.extension = "test";
    agentCard.iconUrl = "test";

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: AgentCardMarshalling_009
 * @tc.desc: AgentCardMarshalling_009
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_009, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test1";
    agentCard.description = "test1";
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    agentCard.version = "test1";
    agentCard.documentationUrl = "test1";
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;
    agentCard.category = "test";
    agentCard.extension = "test";
    agentCard.iconUrl = "test";

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: AgentCardMarshalling_010
 * @tc.desc: AgentCardMarshalling_010
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_010, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test1";
    agentCard.description = "test1";
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    agentCard.version = "test1";
    agentCard.documentationUrl = "test1";
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;
    agentCard.category = "test";
    agentCard.extension = "test";
    agentCard.iconUrl = "test";

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: AgentCardMarshalling_011
 * @tc.desc: AgentCardMarshalling_011
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_011, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test1";
    agentCard.description = "test1";
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    agentCard.version = "test1";
    agentCard.documentationUrl = "test1";
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;
    agentCard.category = "test";
    agentCard.extension = "test";
    agentCard.iconUrl = "test";

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: AgentCardMarshalling_012
 * @tc.desc: AgentCardMarshalling_012
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_012, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test1";
    agentCard.description = "test1";
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    agentCard.version = "test1";
    agentCard.documentationUrl = "test1";
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;
    agentCard.category = "test";
    agentCard.extension = "test";
    agentCard.iconUrl = "test";

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: AgentCardUnmarshalling_001
 * @tc.desc: AgentCardUnmarshalling_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardUnmarshalling_001, TestSize.Level1)
{
    Parcel parcelMock;
    AgentCard *agentCard = AgentCard::Unmarshalling(parcelMock);

    EXPECT_TRUE(agentCard == nullptr);
    delete agentCard;
}

/**
 * @tc.name: AgentCardToJson_001
 * @tc.desc: AgentCardToJson_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardToJson_001, TestSize.Level1)
{
    AgentCard agentCard;
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;
    nlohmann::json jsonObject = agentCard.ToJson();
    EXPECT_TRUE(jsonObject.contains("provider"));

    agentCard.provider = nullptr;
    jsonObject = agentCard.ToJson();
    EXPECT_FALSE(jsonObject.contains("provider"));

    agentCard.capabilities = nullptr;
    jsonObject = agentCard.ToJson();
    EXPECT_FALSE(jsonObject.contains("capabilities"));

    std::vector<std::shared_ptr<AgentSkill>> skills1;
    agentCard.skills = skills1;
    jsonObject = agentCard.ToJson();
    EXPECT_FALSE(jsonObject.contains("skills"));
}

/**
 * @tc.name: AgentCardFromJson_001
 * @tc.desc: AgentCardFromJson_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_001, TestSize.Level1)
{
    AgentProvider provider;
    AgentCapabilities capabilities;
    std::vector<std::string> defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    AgentSkill skill;
    nlohmann::json jsonArray = { skill.ToJson() };
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test" },
        { "version", "1.0" },
        { "documentationUrl", "http://example.com/docs" },
        { "url", "test" },
        { "provider", provider.ToJson() },
        { "capabilities", capabilities.ToJson() },
        { "defaultInputModes", defaultInputModes },
        { "defaultOutputModes", defaultOutputModes },
        { "skills", jsonArray },
        { "category", "test" },
    };
    AgentCard agentCard;
    ASSERT_TRUE(AgentCard::FromJson(jsonObject, agentCard));

    EXPECT_TRUE(agentCard.agentId == "1");
    EXPECT_TRUE(agentCard.name == "test");
    EXPECT_TRUE(agentCard.description == "test");
    EXPECT_TRUE(agentCard.version == "1.0");
    EXPECT_TRUE(agentCard.documentationUrl == "http://example.com/docs");
    EXPECT_TRUE(agentCard.category == "test");
}

/**
 * @tc.name: ProviderToJson_001
 * @tc.desc: ProviderToJson_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderToJson_001, TestSize.Level1)
{
    AgentProvider provider;
    provider.organization = "test";
    nlohmann::json jsonObject = provider.ToJson();

    EXPECT_TRUE(jsonObject["organization"] == "test");
}

/**
 * @tc.name: ProviderFromJson_001
 * @tc.desc: ProviderFromJson_001 - valid input
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderFromJson_001, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "organization", "test" },
        { "url", "test" },
    };
    AgentProvider provider;
    EXPECT_TRUE(AgentProvider::FromJson(jsonObject, provider));
    EXPECT_TRUE(jsonObject["organization"] == provider.organization);
    EXPECT_TRUE(provider.url == "test");
}

/**
 * @tc.name: ProviderFromJson_002
 * @tc.desc: Test FromJson returns false when organization is missing
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderFromJson_002, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "url", "test" },
    };
    AgentProvider provider;
    EXPECT_FALSE(AgentProvider::FromJson(jsonObject, provider));
}

/**
 * @tc.name: ProviderFromJson_003
 * @tc.desc: Test FromJson returns false when organization is not a string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderFromJson_003, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "organization", 123 },
        { "url", "test" },
    };
    AgentProvider provider;
    EXPECT_FALSE(AgentProvider::FromJson(jsonObject, provider));
}

/**
 * @tc.name: ProviderFromJson_004
 * @tc.desc: Test FromJson returns false when organization length exceeds 128
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderFromJson_004, TestSize.Level1)
{
    std::string longOrg(129, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "organization", longOrg },
        { "url", "test" },
    };
    AgentProvider provider;
    EXPECT_FALSE(AgentProvider::FromJson(jsonObject, provider));
}

/**
 * @tc.name: ProviderFromJson_005
 * @tc.desc: Test FromJson returns false when organization is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderFromJson_005, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "organization", "" },
        { "url", "test" },
    };
    AgentProvider provider;
    EXPECT_FALSE(AgentProvider::FromJson(jsonObject, provider));
}

/**
 * @tc.name: ProviderFromJson_006
 * @tc.desc: Test FromJson returns false when url is missing
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderFromJson_006, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "organization", "test" },
    };
    AgentProvider provider;
    EXPECT_FALSE(AgentProvider::FromJson(jsonObject, provider));
}

/**
 * @tc.name: ProviderFromJson_007
 * @tc.desc: Test FromJson returns false when url is not a string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderFromJson_007, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "organization", "test" },
        { "url", 456 },
    };
    AgentProvider provider;
    EXPECT_FALSE(AgentProvider::FromJson(jsonObject, provider));
}

/**
 * @tc.name: ProviderFromJson_008
 * @tc.desc: Test FromJson returns false when url length exceeds 512
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderFromJson_008, TestSize.Level1)
{
    std::string longUrl(513, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "organization", "test" },
        { "url", longUrl },
    };
    AgentProvider provider;
    EXPECT_FALSE(AgentProvider::FromJson(jsonObject, provider));
}

/**
 * @tc.name: ProviderFromJson_009
 * @tc.desc: Test FromJson returns false when url is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderFromJson_009, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "organization", "test" },
        { "url", "" },
    };
    AgentProvider provider;
    EXPECT_FALSE(AgentProvider::FromJson(jsonObject, provider));
}

/**
 * @tc.name: ProviderFromJson_010
 * @tc.desc: Test FromJson returns true with valid boundary values
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderFromJson_010, TestSize.Level1)
{
    std::string validOrg(128, 'a');
    std::string validUrl(512, 'b');
    nlohmann::json jsonObject = nlohmann::json {
        { "organization", validOrg },
        { "url", validUrl },
    };
    AgentProvider provider;
    EXPECT_TRUE(AgentProvider::FromJson(jsonObject, provider));
    EXPECT_EQ(provider.organization.length(), 128);
    EXPECT_EQ(provider.url.length(), 512);
}

/**
 * @tc.name: CapabilitiesToJson_001
 * @tc.desc: CapabilitiesToJson_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, CapabilitiesToJson_001, TestSize.Level1)
{
    AgentCapabilities capabilities;
    capabilities.streaming = true;
    nlohmann::json jsonObject = capabilities.ToJson();

    EXPECT_TRUE(jsonObject["streaming"]);
}

/**
 * @tc.name: CapabilitiesFromJson_001
 * @tc.desc: CapabilitiesFromJson_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, CapabilitiesFromJson_001, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "streaming", true },
        { "pushNotifications", true },
        { "stateTransitionHistory", true },
    };
    AgentCapabilities capabilities = AgentCapabilities::FromJson(jsonObject);
    EXPECT_TRUE(jsonObject["streaming"]);

    jsonObject = nlohmann::json {};
    capabilities = AgentCapabilities::FromJson(jsonObject);
    EXPECT_TRUE(jsonObject.contains("streaming") == false);
}

/**
 * @tc.name: SkillToJson_001
 * @tc.desc: SkillToJson_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillToJson_001, TestSize.Level1)
{
    AgentSkill skill;
    skill.id = "test";
    nlohmann::json jsonObject = skill.ToJson();

    EXPECT_TRUE(jsonObject["id"] == "test");
}

/**
 * @tc.name: SkillFromJson_001
 * @tc.desc: SkillFromJson_001 - valid input
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_001, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test", "test" }) },
        { "examples", nlohmann::json::array({ "test", "test" }) },
        { "inputModes", nlohmann::json::array({ "test", "test" }) },
        { "outputModes", nlohmann::json::array({ "test", "test" }) },
    };
    AgentSkill skill;
    EXPECT_TRUE(AgentSkill::FromJson(jsonObject, skill));
    EXPECT_TRUE(skill.id == "test");
    EXPECT_EQ(skill.tags.size(), 2);
}

/**
 * @tc.name: SkillFromJson_002
 * @tc.desc: Test FromJson returns false when id is missing
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_002, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_003
 * @tc.desc: Test FromJson returns false when id is not a string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_003, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", 123 },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_004
 * @tc.desc: Test FromJson returns false when id length exceeds 64
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_004, TestSize.Level1)
{
    std::string longId(65, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "id", longId },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_005
 * @tc.desc: Test FromJson returns false when id is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_005, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_006
 * @tc.desc: Test FromJson returns false when name is missing
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_006, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_007
 * @tc.desc: Test FromJson returns false when name is not a string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_007, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", 456 },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_008
 * @tc.desc: Test FromJson returns false when name length exceeds 128
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_008, TestSize.Level1)
{
    std::string longName(129, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", longName },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_009
 * @tc.desc: Test FromJson returns false when name is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_009, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_010
 * @tc.desc: Test FromJson returns false when description is missing
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_010, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_011
 * @tc.desc: Test FromJson returns false when description is not a string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_011, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", 789 },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_012
 * @tc.desc: Test FromJson returns false when description length exceeds 512
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_012, TestSize.Level1)
{
    std::string longDescription(513, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", longDescription },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_013
 * @tc.desc: Test FromJson returns false when description is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_013, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "" },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_014
 * @tc.desc: Test FromJson returns false when tags is missing
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_014, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_015
 * @tc.desc: Test FromJson returns false when tags is not an array
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_015, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", "not an array" },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_016
 * @tc.desc: Test FromJson returns false when tags size exceeds 100
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_016, TestSize.Level1)
{
    nlohmann::json tagsArray = nlohmann::json::array();
    for (int i = 0; i < 101; i++) {
        tagsArray.push_back("tag" + std::to_string(i));
    }
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", tagsArray },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_017
 * @tc.desc: Test FromJson returns false when all tags are invalid (too long), resulting in empty tags
 * @tc.type:: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_017, TestSize.Level1)
{
    std::string longTag(33, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ longTag }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_018
 * @tc.desc: Test FromJson returns true with valid boundary values
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_018, TestSize.Level1)
{
    std::string validId(64, 'a');
    std::string validName(128, 'b');
    std::string validDescription(512, 'c');
    std::string validTag(32, 'd');

    nlohmann::json tagsArray = nlohmann::json::array();
    for (int i = 0; i < 100; i++) {
        tagsArray.push_back(validTag);
    }

    nlohmann::json jsonObject = nlohmann::json {
        { "id", validId },
        { "name", validName },
        { "description", validDescription },
        { "tags", tagsArray },
    };
    AgentSkill skill;
    EXPECT_TRUE(AgentSkill::FromJson(jsonObject, skill));
    EXPECT_EQ(skill.id.length(), 64);
    EXPECT_EQ(skill.name.length(), 128);
    EXPECT_EQ(skill.description.length(), 512);
    EXPECT_EQ(skill.tags.size(), 100);
}

/**
 * @tc.name: SkillFromJson_019
 * @tc.desc: Test FromJson skips non-string elements in examples array
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_019, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
        { "examples", nlohmann::json::array({ "example1", 123, true, "example2", nullptr }) },
    };
    AgentSkill skill;
    EXPECT_TRUE(AgentSkill::FromJson(jsonObject, skill));
    EXPECT_EQ(skill.examples.size(), 2);
    EXPECT_EQ(skill.examples[0], "example1");
    EXPECT_EQ(skill.examples[1], "example2");
}

/**
 * @tc.name: SkillFromJson_020
 * @tc.desc: Test FromJson skips non-string elements in inputModes array
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_020, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
        { "inputModes", nlohmann::json::array({ "mode1", 456, false, "mode2", nullptr }) },
    };
    AgentSkill skill;
    EXPECT_TRUE(AgentSkill::FromJson(jsonObject, skill));
    EXPECT_EQ(skill.inputModes.size(), 2);
    EXPECT_EQ(skill.inputModes[0], "mode1");
    EXPECT_EQ(skill.inputModes[1], "mode2");
}

/**
 * @tc.name:: SkillFromJson_021
 * @tc.desc: Test FromJson skips non-string elements in outputModes array
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_021, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
        { "outputModes", nlohmann::json::array({ "output1", 789, true, "output2", nullptr }) },
    };
    AgentSkill skill;
    EXPECT_TRUE(AgentSkill::FromJson(jsonObject, skill));
    EXPECT_EQ(skill.outputModes.size(), 2);
    EXPECT_EQ(skill.outputModes[0], "output1");
    EXPECT_EQ(skill.outputModes[1], "output2");
}

/**
 * @tc.name: SkillFromJson_022
 * @tc.desc: Test FromJson skips non-string elements in tags array
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_022, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "tag1", 123, true, "tag2", nullptr }) },
    };
    AgentSkill skill;
    EXPECT_TRUE(AgentSkill::FromJson(jsonObject, skill));
    EXPECT_EQ(skill.tags.size(), 2);
    EXPECT_EQ(skill.tags[0], "tag1");
    EXPECT_EQ(skill.tags[1], "tag2");
}

/**
 * @tc.name: SkillFromJson_023
 * @tc.desc: Test FromJson returns false when extension length exceeds 1280
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_023, TestSize.Level1)
{
    std::string longExtension(1281, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
        { "extension", longExtension },
    };
    AgentSkill skill;
    EXPECT_TRUE(AgentSkill::FromJson(jsonObject, skill));
    EXPECT_TRUE(skill.extension.empty());
}

/**
 * @tc.name: SkillFromJson_024
 * @tc.desc: Test FromJson returns false when extension length is 0
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_024, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
        { "extension", "" },
    };
    AgentSkill skill;
    EXPECT_TRUE(AgentSkill::FromJson(jsonObject, skill));
    EXPECT_TRUE(skill.extension.empty());
}

/**
 * @tc.name: AgentCardFromJson_002
 * @tc.desc: Test FromJson succeeds with valid iconUrl field (length 1-512)
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_002, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "iconUrl", "http://example.com/icon.png" },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.iconUrl, "http://example.com/icon.png");
}

/**
 * @tc.name: AgentCardFromJson_003
 * @tc.desc: Test FromJson succeeds when iconUrl length is exactly 512
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_003, TestSize.Level1)
{
    std::string validIconUrl(512, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "iconUrl", validIconUrl },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.iconUrl.length(), 512);
}

/**
 * @tc.name: AgentCardFromJson_004
 * @tc.desc: Test FromJson sets iconUrl to empty when length exceeds 512
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_004, TestSize.Level1)
{
    std::string longIconUrl(513, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "iconUrl", longIconUrl },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.iconUrl, "");
}

/**
 * @tc.name: AgentCardFromJson_005
 * @tc.desc: Test FromJson sets iconUrl to empty when iconUrl is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_005, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "iconUrl", "" },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.iconUrl, "");
}

/**
 * @tc.name: AgentCardFromJson_006
 * @tc.desc: Test FromJson ignores iconUrl when it's not a string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_006, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "iconUrl", 12345 },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.iconUrl, "");
}

/**
 * @tc.name: AgentCardFromJson_007
 * @tc.desc: Test FromJson succeeds when category is provided with valid length (1-64)
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_007, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.category, "productivity");
}

/**
 * @tc.name: AgentCardFromJson_008
 * @tc.desc: Test FromJson fails when category is missing (required field)
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_008, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_009
 * @tc.desc: Test FromJson fails when category length exceeds 64
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_009, TestSize.Level1)
{
    std::string longCategory(65, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", longCategory },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_010
 * @tc.desc: Test FromJson fails when category is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_010, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "" },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_011
 * @tc.desc: Test FromJson fails when category is not a string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_011, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", 12345 },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_012
 * @tc.desc: Test FromJson succeeds when category length is exactly 64
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_012, TestSize.Level1)
{
    std::string validCategory(64, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", validCategory },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.category.length(), 64);
}

/**
 * @tc.name: AgentCardFromJson_013
 * @tc.desc: Test FromJson succeeds with valid appInfo
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_013, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "appInfo", nlohmann::json {
            { "bundleName", "com.test.app" },
            { "moduleName", "entry" },
            { "abilityName", "MainAbility" },
            { "deviceTypes", "phone" },
            { "minAppVersion", "1.0.0" },
        }},
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_NE(agentCard.appInfo, nullptr);
    EXPECT_EQ(agentCard.appInfo->bundleName, "com.test.app");
    EXPECT_EQ(agentCard.appInfo->moduleName, "entry");
    EXPECT_EQ(agentCard.appInfo->abilityName, "MainAbility");
    EXPECT_EQ(agentCard.appInfo->deviceTypes, "phone");
    EXPECT_EQ(agentCard.appInfo->minAppVersion, "1.0.0");
}

/**
 * @tc.name: AgentCardFromJson_014
 * @tc.desc: Test FromJson succeeds with empty appInfo
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_014, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "appInfo", nlohmann::json {} },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_NE(agentCard.appInfo, nullptr);
}

/**
 * @tc.name: AgentCardFromJson_015
 * @tc.desc: Test FromJson succeeds when appInfo deviceTypes length is 1-128
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_015, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "appInfo", nlohmann::json {
            { "bundleName", "com.test.app" },
            { "moduleName", "entry" },
            { "abilityName", "MainAbility" },
            { "deviceTypes", "p" },
            { "minAppVersion", "1.0.0" },
        }},
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.appInfo->deviceTypes, "p");
}

/**
 * @tc.name: AgentCardFromJson_016
 * @tc.desc: Test FromJson fails when appInfo deviceTypes length exceeds 128
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_016, TestSize.Level1)
{
    std::string longDeviceTypes(129, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "appInfo", nlohmann::json {
            { "bundleName", "com.test.app" },
            { "moduleName", "entry" },
            { "abilityName", "MainAbility" },
            { "deviceTypes", longDeviceTypes },
            { "minAppVersion", "1.0.0" },
        }},
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_017
 * @tc.desc: Test FromJson fails when appInfo deviceTypes is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_017, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "appInfo", nlohmann::json {
            { "bundleName", "com.test.app" },
            { "moduleName", "entry" },
            { "abilityName", "MainAbility" },
            { "deviceTypes", "" },
            { "minAppVersion", "1.0.0" },
        }},
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_018
 * @tc.desc: Test FromJson succeeds when appInfo minAppVersion length is 1-32
 * @tc.type: FUNC
 * @tc.require:AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_018, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "appInfo", nlohmann::json {
            { "bundleName", "com.test.app" },
            { "moduleName", "entry" },
            { "abilityName", "MainAbility" },
            { "deviceTypes", "phone" },
            { "minAppVersion", "1.0" },
        }},
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.appInfo->minAppVersion, "1.0");
}

/**
 * @tc.name: AgentCardFromJson_019
 * @tc.desc: Test FromJson fails when appInfo minAppVersion length exceeds 32
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_019, TestSize.Level1)
{
    std::string longMinAppVersion(33, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "appInfo", nlohmann::json {
            { "bundleName", "com.test.app" },
            { "moduleName", "entry" },
            { "abilityName", "MainAbility" },
            { "deviceTypes", "phone" },
            { "minAppVersion", longMinAppVersion },
        }},
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_020
 * @tc.desc: Test FromJson fails when appInfo minAppVersion is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_020, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "appInfo", nlohmann::json {
            { "bundleName", "com.test.app" },
            { "moduleName", "entry" },
            { "abilityName", "MainAbility" },
            { "deviceTypes", "phone" },
            { "minAppVersion", "" },
        }},
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_021
 * @tc.desc: Test FromJson succeeds when extension length is 1-51200
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_021, TestSize.Level1)
{
    std::string validExtension(51200, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "extension", validExtension },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.extension.length(), 51200);
}

/**
 * @tc.name: AgentCardFromJson_022
 * @tc.desc: Test FromJson sets extension to empty when length exceeds 51200
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_022, TestSize.Level1)
{
    std::string longExtension(51201, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "extension", longExtension },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_TRUE(agentCard.extension.empty());
}

/**
 * @tc.name: AgentCardFromJson_023
 * @tc.desc: Test FromJson sets extension to empty when extension is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_023, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "extension", "" },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_TRUE(agentCard.extension.empty());
}

/**
 * @tc.name: AgentCardFromJson_024
 * @tc.desc: Test FromJson ignores extension when it's not a string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_024, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "extension", 12345 },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_TRUE(agentCard.extension.empty());
}

/**
 * @tc.name: AgentCardFromJson_025
 * @tc.desc: Test FromJson succeeds with capabilities.extendedAgentCard
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_025, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "capabilities", nlohmann::json {
            { "extendedAgentCard", true },
        }},
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_NE(agentCard.capabilities, nullptr);
    EXPECT_TRUE(agentCard.capabilities->extendedAgentCard);
}

/**
 * @tc.name: AgentCardFromJson_026
 * @tc.desc: Test FromJson succeeds with skills array
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_026, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "skills", nlohmann::json::array({
            nlohmann::json {
                { "id", "skill001" },
                { "name", "Test Skill" },
                { "description", "Test Skill Description" },
                { "tags", nlohmann::json::array({ "tag1", "tag2" }) },
            },
        })},
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.skills.size(), 1);
    EXPECT_EQ(agentCard.skills[0]->id, "skill001");
    EXPECT_EQ(agentCard.skills[0]->name, "Test Skill");
}

/**
 * @tc.name: AgentCardFromJson_027
 * @tc.desc: Test FromJson succeeds with defaultInputModes and defaultOutputModes
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_027, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "defaultInputModes", nlohmann::json::array({ "text", "voice" }) },
        { "defaultOutputModes", nlohmann::json::array({ "text" }) },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.defaultInputModes.size(), 2);
    EXPECT_EQ(agentCard.defaultOutputModes.size(), 1);
}

/**
 * @tc.name: AgentCardFromJson_028
 * @tc.desc: Test FromJson succeeds with provider
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_028, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "provider", nlohmann::json {
            { "organization", "Test Org" },
            { "url", "http://example.com" },
        }},
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_NE(agentCard.provider, nullptr);
    EXPECT_EQ(agentCard.provider->organization, "Test Org");
    EXPECT_EQ(agentCard.provider->url, "http://example.com");
}

/**
 * @tc.name: AgentCardFromJson_029
 * @tc.desc: Test FromJson succeeds with documentationUrl
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_029, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "documentationUrl", "http://example.com/docs" },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.documentationUrl, "http://example.com/docs");
}

/**
 * @tc.name: AgentCardFromJson_030
 * @tc.desc: Test FromJson succeeds when documentationUrl length is exactly 512
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_030, TestSize.Level1)
{
    std::string validDocUrl(512, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "documentationUrl", validDocUrl },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.documentationUrl.length(), 512);
}

/**
 * @tc.name: AgentCardFromJson_031
 * @tc.desc: Test FromJson sets documentationUrl to empty when length exceeds 512
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_031, TestSize.Level1)
{
    std::string longDocUrl(513, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "documentationUrl", longDocUrl },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_TRUE(agentCard.documentationUrl.empty());
}

/**
 * @tc.name: AgentCardFromJson_032
 * @tc.desc: Test FromJson succeeds when documentationUrl is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_032, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "documentationUrl", "" },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_TRUE(agentCard.documentationUrl.empty());
}

/**
 * @tc.name: AgentCardFromJson_033
 * @tc.desc: Test FromJson ignores documentationUrl when it's not a string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_033, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "agent001" },
        { "name", "Test Agent" },
        { "description", "Test Description" },
        { "version", "1.0.0" },
        { "category", "productivity" },
        { "documentationUrl", 12345 },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_TRUE(agentCard.documentationUrl.empty());
}

/**
 * @tc.name: AgentAppInfoParcel_001
 * @tc.desc: Test AgentAppInfo Marshalling and Unmarshalling
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentAppInfoParcel_001, TestSize.Level1)
{
    AgentAppInfo originalAppInfo;
    originalAppInfo.bundleName = "test.bundle";
    originalAppInfo.moduleName = "test";
    originalAppInfo.abilityName = "TestAbility";
    originalAppInfo.deviceTypes = "phone";
    originalAppInfo.minAppVersion = "1.0.0";

    Parcel parcel;
    EXPECT_TRUE(originalAppInfo.Marshalling(parcel));

    AgentAppInfo *unmarshalledAppInfo = AgentAppInfo::Unmarshalling(parcel);
    EXPECT_NE(unmarshalledAppInfo, nullptr);
    if (unmarshalledAppInfo != nullptr) {
        EXPECT_EQ(unmarshalledAppInfo->bundleName, originalAppInfo.bundleName);
        EXPECT_EQ(unmarshalledAppInfo->moduleName, originalAppInfo.moduleName);
        EXPECT_EQ(unmarshalledAppInfo->abilityName, originalAppInfo.abilityName);
        EXPECT_EQ(unmarshalledAppInfo->deviceTypes, originalAppInfo.deviceTypes);
        EXPECT_EQ(unmarshalledAppInfo->minAppVersion, originalAppInfo.minAppVersion);
        delete unmarshalledAppInfo;
    }
}

/**
 * @tc.name: AgentCardParcel_001
 * @tc.desc: Test AgentCard Marshalling and Unmarshalling round-trip with category field
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardParcel_001, TestSize.Level1)
{
    AgentCard originalCard;
    originalCard.agentId = "agent_001";
    originalCard.name = "Test Agent";
    originalCard.description = "Test Description";
    originalCard.provider = std::make_shared<AgentProvider>();
    originalCard.provider->organization = "Test Org";
    originalCard.provider->url = "http://example.com";
    originalCard.version = "1.0.0";
    originalCard.documentationUrl = "http://docs.example.com";
    originalCard.capabilities = std::make_shared<AgentCapabilities>();
    originalCard.capabilities->streaming = true;
    originalCard.capabilities->extendedAgentCard = true;
    originalCard.defaultInputModes = {"text", "voice"};
    originalCard.defaultOutputModes = {"text"};
    originalCard.skills = {std::make_shared<AgentSkill>()};
    originalCard.skills[0]->id = "skill_001";
    originalCard.skills[0]->name = "Test Skill";
    originalCard.skills[0]->description = "Test Skill Description";
    originalCard.skills[0]->tags = {"tag1", "tag2"};
    originalCard.extension = "customExtension";
    originalCard.category = "productivity";
    originalCard.iconUrl = "http://example.com/icon.png";
    originalCard.appInfo = std::make_shared<AgentAppInfo>();
    originalCard.appInfo->bundleName = "test.bundle";
    originalCard.appInfo->moduleName = "test";
    originalCard.appInfo->abilityName = "TestAbility";

    Parcel parcel;
    EXPECT_TRUE(originalCard.Marshalling(parcel));

    AgentCard *unmarshalledCard = AgentCard::Unmarshalling(parcel);
    EXPECT_NE(unmarshalledCard, nullptr);
    if (unmarshalledCard != nullptr) {
        EXPECT_EQ(unmarshalledCard->agentId, originalCard.agentId);
        EXPECT_EQ(unmarshalledCard->name, originalCard.name);
        EXPECT_EQ(unmarshalledCard->description, originalCard.description);
        EXPECT_EQ(unmarshalledCard->version, originalCard.version);
        EXPECT_EQ(unmarshalledCard->documentationUrl, originalCard.documentationUrl);
        EXPECT_EQ(unmarshalledCard->extension, originalCard.extension);
        EXPECT_EQ(unmarshalledCard->category, originalCard.category);
        EXPECT_EQ(unmarshalledCard->iconUrl, originalCard.iconUrl);
        EXPECT_EQ(unmarshalledCard->defaultInputModes, originalCard.defaultInputModes);
        EXPECT_EQ(unmarshalledCard->defaultOutputModes, originalCard.defaultOutputModes);
        EXPECT_NE(unmarshalledCard->provider, nullptr);
        if (unmarshalledCard->provider != nullptr) {
            EXPECT_EQ(unmarshalledCard->provider->organization, originalCard.provider->organization);
            EXPECT_EQ(unmarshalledCard->provider->url, originalCard.provider->url);
        }
        EXPECT_NE(unmarshalledCard->capabilities, nullptr);
        if (unmarshalledCard->capabilities != nullptr) {
            EXPECT_EQ(unmarshalledCard->capabilities->streaming, originalCard.capabilities->streaming);
            EXPECT_EQ(unmarshalledCard->capabilities->extendedAgentCard, originalCard.capabilities->extendedAgentCard);
        }
        EXPECT_EQ(unmarshalledCard->skills.size(), originalCard.skills.size());
        if (unmarshalledCard->skills.size() > 0) {
            EXPECT_EQ(unmarshalledCard->skills[0]->id, originalCard.skills[0]->id);
            EXPECT_EQ(unmarshalledCard->skills[0]->name, originalCard.skills[0]->name);
        }
        EXPECT_NE(unmarshalledCard->appInfo, nullptr);
        if (unmarshalledCard->appInfo != nullptr) {
            EXPECT_EQ(unmarshalledCard->appInfo->bundleName, originalCard.appInfo->bundleName);
            EXPECT_EQ(unmarshalledCard->appInfo->moduleName, originalCard.appInfo->moduleName);
        }
        delete unmarshalledCard;
    }
}
} // namespace AgentRuntime
} // namespace OHOS
