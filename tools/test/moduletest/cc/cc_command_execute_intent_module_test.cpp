/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#define protected public
#include "cc_command.h"
#undef protected
#include "mock_ability_manager_stub.h"
#define private public
#include "ability_manager_client.h"
#undef private
#include "ability_manager_interface.h"
#include "insight_intent_info_for_query.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace {
const std::string TOOL_NAME = "ohos-claw-cc";
const std::string CMD_EXECUTE_INTENT = "execute-intent";
const std::string BUNDLE_NAME = "com.example.test";
const std::string MODULE_NAME = "entry";
const std::string INTENT_NAME = "TestIntent";
const std::string ABILITY_NAME = "MainAbility";
}  // namespace

class CcCommandExecuteIntentModuleTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects() const;

    std::string cmd_ = CMD_EXECUTE_INTENT;
};

void CcCommandExecuteIntentModuleTest::SetUpTestCase()
{}

void CcCommandExecuteIntentModuleTest::TearDownTestCase()
{}

void CcCommandExecuteIntentModuleTest::SetUp()
{
    optind = 0;
    MakeMockObjects();
}

void CcCommandExecuteIntentModuleTest::TearDown()
{}

void CcCommandExecuteIntentModuleTest::MakeMockObjects() const
{
    auto managerStubPtr = sptr<IAbilityManager>(new MockAbilityManagerStub());
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

/**
 * @tc.number: Cc_Command_Execute_Intent_ModuleTest_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-claw-cc execute-intent -h" command shows help.
 */
HWTEST_F(CcCommandExecuteIntentModuleTest, Cc_Command_Execute_Intent_ModuleTest_0100, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-h",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("usage: ohos-claw-cc execute-intent"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Execute_Intent_ModuleTest_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify execute-intent without required parameters returns error event.
 */
HWTEST_F(CcCommandExecuteIntentModuleTest, Cc_Command_Execute_Intent_ModuleTest_0200, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"event\":\"error\""), std::string::npos);
    EXPECT_NE(result.find("missing required parameters"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Execute_Intent_ModuleTest_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify execute-intent with missing -i parameter returns error event.
 */
HWTEST_F(CcCommandExecuteIntentModuleTest, Cc_Command_Execute_Intent_ModuleTest_0300, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)BUNDLE_NAME.c_str(),
        (char*)"-m",
        (char*)MODULE_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"event\":\"error\""), std::string::npos);
    EXPECT_NE(result.find("missing required parameters"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Execute_Intent_ModuleTest_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify execute-intent with missing -m parameter returns error event.
 */
HWTEST_F(CcCommandExecuteIntentModuleTest, Cc_Command_Execute_Intent_ModuleTest_0400, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)BUNDLE_NAME.c_str(),
        (char*)"-i",
        (char*)INTENT_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"event\":\"error\""), std::string::npos);
    EXPECT_NE(result.find("missing required parameters"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Execute_Intent_ModuleTest_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify execute-intent with missing -b parameter returns error event.
 */
HWTEST_F(CcCommandExecuteIntentModuleTest, Cc_Command_Execute_Intent_ModuleTest_0500, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-m",
        (char*)MODULE_NAME.c_str(),
        (char*)"-i",
        (char*)INTENT_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"event\":\"error\""), std::string::npos);
    EXPECT_NE(result.find("missing required parameters"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Execute_Intent_ModuleTest_0600
 * @tc.name: ExecCommand
 * @tc.desc: Verify execute-intent with invalid execute mode value returns error event.
 */
HWTEST_F(CcCommandExecuteIntentModuleTest, Cc_Command_Execute_Intent_ModuleTest_0600, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)BUNDLE_NAME.c_str(),
        (char*)"-m",
        (char*)MODULE_NAME.c_str(),
        (char*)"-i",
        (char*)INTENT_NAME.c_str(),
        (char*)"-e",
        (char*)"abc",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"event\":\"error\""), std::string::npos);
    EXPECT_NE(result.find("invalid execute mode value"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Execute_Intent_ModuleTest_0700
 * @tc.name: ExecCommand
 * @tc.desc: Verify execute-intent with unknown option returns error event.
 */
HWTEST_F(CcCommandExecuteIntentModuleTest, Cc_Command_Execute_Intent_ModuleTest_0700, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-z",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"event\":\"error\""), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Execute_Intent_ModuleTest_0800
 * @tc.name: ExecCommand
 * @tc.desc: Verify execute-intent auto-resolve fails when query returns empty intent.
 */
HWTEST_F(CcCommandExecuteIntentModuleTest, Cc_Command_Execute_Intent_ModuleTest_0800, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)BUNDLE_NAME.c_str(),
        (char*)"-m",
        (char*)MODULE_NAME.c_str(),
        (char*)"-i",
        (char*)INTENT_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    auto mockStub = new MockAbilityManagerStub();
    EXPECT_CALL(*mockStub, GetInsightIntentInfoByIntentName(_, _, _, _, _, _))
        .WillOnce(Return(ERR_OK));

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"event\":\"error\""), std::string::npos);
    EXPECT_NE(result.find("invalid intentName"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Execute_Intent_ModuleTest_0900
 * @tc.name: ExecCommand
 * @tc.desc: Verify execute-intent auto-resolve fails when abilityName not found.
 */
HWTEST_F(CcCommandExecuteIntentModuleTest, Cc_Command_Execute_Intent_ModuleTest_0900, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)BUNDLE_NAME.c_str(),
        (char*)"-m",
        (char*)MODULE_NAME.c_str(),
        (char*)"-i",
        (char*)INTENT_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    auto mockStub = new MockAbilityManagerStub();
    EXPECT_CALL(*mockStub, GetInsightIntentInfoByIntentName(_, _, _, _, _, _))
        .WillOnce([](auto, auto, auto, auto, auto &info, auto) {
            info.intentType = "test";
            return ERR_OK;
        });

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"event\":\"error\""), std::string::npos);
    EXPECT_NE(result.find("abilityName not found"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Execute_Intent_ModuleTest_1000
 * @tc.name: ExecCommand
 * @tc.desc: Verify execute-intent with all explicit parameters and valid execute mode.
 */
HWTEST_F(CcCommandExecuteIntentModuleTest, Cc_Command_Execute_Intent_ModuleTest_1000, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)BUNDLE_NAME.c_str(),
        (char*)"-m",
        (char*)MODULE_NAME.c_str(),
        (char*)"-a",
        (char*)ABILITY_NAME.c_str(),
        (char*)"-i",
        (char*)INTENT_NAME.c_str(),
        (char*)"-e",
        (char*)"0",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    auto mockStub = new MockAbilityManagerStub();
    EXPECT_CALL(*mockStub, ExecuteIntent(_, _, _))
        .WillOnce(Return(ERR_OK));

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_EQ(result.find("\"event\":\"error\""), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Execute_Intent_ModuleTest_1100
 * @tc.name: ExecCommand
 * @tc.desc: Verify execute-intent with multiple execute modes reports error event.
 */
HWTEST_F(CcCommandExecuteIntentModuleTest, Cc_Command_Execute_Intent_ModuleTest_1100, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)BUNDLE_NAME.c_str(),
        (char*)"-m",
        (char*)MODULE_NAME.c_str(),
        (char*)"-i",
        (char*)INTENT_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    auto mockStub = new MockAbilityManagerStub();
    EXPECT_CALL(*mockStub, GetInsightIntentInfoByIntentName(_, _, _, _, _, _))
        .WillOnce([](auto, auto, auto, auto, auto &info, auto) {
            info.intentType = "test";
            info.uiAbilityIntentInfo.abilityName = ABILITY_NAME;
            info.entryInfo.executeMode = {
                ExecuteMode::UI_ABILITY_FOREGROUND,
                ExecuteMode::UI_ABILITY_BACKGROUND
            };
            return ERR_OK;
        });

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"event\":\"error\""), std::string::npos);
    EXPECT_NE(result.find("multiple execute modes"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Execute_Intent_ModuleTest_1200
 * @tc.name: ExecCommand
 * @tc.desc: Verify execute-intent with no execute mode found reports error event.
 */
HWTEST_F(CcCommandExecuteIntentModuleTest, Cc_Command_Execute_Intent_ModuleTest_1200, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)BUNDLE_NAME.c_str(),
        (char*)"-m",
        (char*)MODULE_NAME.c_str(),
        (char*)"-i",
        (char*)INTENT_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    auto mockStub = new MockAbilityManagerStub();
    EXPECT_CALL(*mockStub, GetInsightIntentInfoByIntentName(_, _, _, _, _, _))
        .WillOnce([](auto, auto, auto, auto, auto &info, auto) {
            info.intentType = "test";
            info.uiAbilityIntentInfo.abilityName = ABILITY_NAME;
            return ERR_OK;
        });

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"event\":\"error\""), std::string::npos);
    EXPECT_NE(result.find("no execute mode found"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Execute_Intent_ModuleTest_1300
 * @tc.name: ExecCommand
 * @tc.desc: Verify execute-intent with JSON param passes validation.
 */
HWTEST_F(CcCommandExecuteIntentModuleTest, Cc_Command_Execute_Intent_ModuleTest_1300, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)BUNDLE_NAME.c_str(),
        (char*)"-m",
        (char*)MODULE_NAME.c_str(),
        (char*)"-a",
        (char*)ABILITY_NAME.c_str(),
        (char*)"-i",
        (char*)INTENT_NAME.c_str(),
        (char*)"-e",
        (char*)"0",
        (char*)"-p",
        (char*)"{\"city\":\"Beijing\",\"days\":3}",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    auto mockStub = new MockAbilityManagerStub();
    EXPECT_CALL(*mockStub, ExecuteIntent(_, _, _))
        .WillOnce(Return(ERR_OK));

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_EQ(result.find("\"event\":\"error\""), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Execute_Intent_ModuleTest_1400
 * @tc.name: ExecCommand
 * @tc.desc: Verify execute-intent abilityName auto-resolve falls back to pageInfo.
 */
HWTEST_F(CcCommandExecuteIntentModuleTest, Cc_Command_Execute_Intent_ModuleTest_1400, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)BUNDLE_NAME.c_str(),
        (char*)"-m",
        (char*)MODULE_NAME.c_str(),
        (char*)"-i",
        (char*)INTENT_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    auto mockStub = new MockAbilityManagerStub();
    EXPECT_CALL(*mockStub, GetInsightIntentInfoByIntentName(_, _, _, _, _, _))
        .WillOnce([](auto, auto, auto, auto, auto &info, auto) {
            info.intentType = "test";
            info.pageInfo.uiAbility = ABILITY_NAME;
            info.entryInfo.executeMode = {
                ExecuteMode::UI_ABILITY_FOREGROUND
            };
            return ERR_OK;
        });
    EXPECT_CALL(*mockStub, ExecuteIntent(_, _, _))
        .WillOnce(Return(ERR_OK));

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_EQ(result.find("abilityName not found"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Execute_Intent_ModuleTest_1500
 * @tc.name: ExecCommand
 * @tc.desc: Verify execute-intent abilityName auto-resolve falls back to entryInfo.
 */
HWTEST_F(CcCommandExecuteIntentModuleTest, Cc_Command_Execute_Intent_ModuleTest_1500, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)BUNDLE_NAME.c_str(),
        (char*)"-m",
        (char*)MODULE_NAME.c_str(),
        (char*)"-i",
        (char*)INTENT_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    auto mockStub = new MockAbilityManagerStub();
    EXPECT_CALL(*mockStub, GetInsightIntentInfoByIntentName(_, _, _, _, _, _))
        .WillOnce([](auto, auto, auto, auto, auto &info, auto) {
            info.intentType = "test";
            info.entryInfo.abilityName = ABILITY_NAME;
            info.entryInfo.executeMode = {
                ExecuteMode::UI_ABILITY_FOREGROUND
            };
            return ERR_OK;
        });
    EXPECT_CALL(*mockStub, ExecuteIntent(_, _, _))
        .WillOnce(Return(ERR_OK));

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_EQ(result.find("abilityName not found"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Execute_Intent_ModuleTest_1600
 * @tc.name: ExecCommand
 * @tc.desc: Verify execute-intent abilityName auto-resolve falls back to formInfo.
 */
HWTEST_F(CcCommandExecuteIntentModuleTest, Cc_Command_Execute_Intent_ModuleTest_1600, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)BUNDLE_NAME.c_str(),
        (char*)"-m",
        (char*)MODULE_NAME.c_str(),
        (char*)"-i",
        (char*)INTENT_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    auto mockStub = new MockAbilityManagerStub();
    EXPECT_CALL(*mockStub, GetInsightIntentInfoByIntentName(_, _, _, _, _, _))
        .WillOnce([](auto, auto, auto, auto, auto &info, auto) {
            info.intentType = "test";
            info.formInfo.abilityName = ABILITY_NAME;
            info.entryInfo.executeMode = {
                ExecuteMode::UI_ABILITY_FOREGROUND
            };
            return ERR_OK;
        });
    EXPECT_CALL(*mockStub, ExecuteIntent(_, _, _))
        .WillOnce(Return(ERR_OK));

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_EQ(result.find("abilityName not found"), std::string::npos);
}
