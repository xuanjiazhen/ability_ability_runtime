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
const std::string TOOL_NAME = "claw_cc";
const std::string CMD_GET_INTENT = "get-intent";
const std::string BUNDLE_NAME = "com.example.test";
const std::string MODULE_NAME = "entry";
const std::string INTENT_NAME = "TestIntent";
}  // namespace

class CcCommandGetIntentModuleTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects() const;

    std::string cmd_ = CMD_GET_INTENT;
};

void CcCommandGetIntentModuleTest::SetUpTestCase()
{}

void CcCommandGetIntentModuleTest::TearDownTestCase()
{}

void CcCommandGetIntentModuleTest::SetUp()
{
    optind = 0;
    MakeMockObjects();
}

void CcCommandGetIntentModuleTest::TearDown()
{}

void CcCommandGetIntentModuleTest::MakeMockObjects() const
{
    auto managerStubPtr = sptr<IAbilityManager>(new MockAbilityManagerStub());
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "claw_cc get-intent -h" command shows help.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_0100, Function | MediumTest | Level1)
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
    EXPECT_NE(result.find("usage: claw_cc get-intent"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent without required -e and -f returns error.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_0200, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("error: missing required parameters -e and -f"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent with missing -f returns error.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_0300, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"1",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("error: missing required parameters -e and -f"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent with invalid execute mode value returns error.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_0400, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"abc",
        (char*)"-f",
        (char*)"1",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("error: invalid execute mode value"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent with invalid flag value returns error.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_0500, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"1",
        (char*)"-f",
        (char*)"xyz",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("error: invalid flag value"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_0600
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent with invalid user id value returns error.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_0600, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"1",
        (char*)"-f",
        (char*)"1",
        (char*)"-u",
        (char*)"notanumber",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("error: invalid user id value"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_0700
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent mode 1 (query all) succeeds.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_0700, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"1",
        (char*)"-f",
        (char*)"1",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    auto mockStub = new MockAbilityManagerStub();
    EXPECT_CALL(*mockStub, GetAllInsightIntentInfo(_, _, _))
        .WillOnce([](auto, auto &infos, auto) {
            AbilityRuntime::InsightIntentInfoForQuery info;
            info.bundleName = BUNDLE_NAME;
            info.moduleName = MODULE_NAME;
            info.intentName = INTENT_NAME;
            info.intentType = "test";
            infos.push_back(info);
            return ERR_OK;
        });

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("get insight intent info successfully (mode: all)"), std::string::npos);
    EXPECT_NE(result.find("Total count: 1"), std::string::npos);
    EXPECT_NE(result.find(BUNDLE_NAME), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_0800
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent mode 1 with empty result.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_0800, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"1",
        (char*)"-f",
        (char*)"1",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    auto mockStub = new MockAbilityManagerStub();
    EXPECT_CALL(*mockStub, GetAllInsightIntentInfo(_, _, _))
        .WillOnce(Return(ERR_OK));

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("get insight intent info successfully (mode: all)"), std::string::npos);
    EXPECT_NE(result.find("Total count: 0"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_0900
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent mode 2 without -b returns error.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_0900, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"2",
        (char*)"-f",
        (char*)"1",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("error: mode 2 requires -b <bundle-name>"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_1000
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent mode 2 with bundle name succeeds.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_1000, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"2",
        (char*)"-f",
        (char*)"1",
        (char*)"-b",
        (char*)BUNDLE_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    auto mockStub = new MockAbilityManagerStub();
    EXPECT_CALL(*mockStub, GetInsightIntentInfoByBundleName(_, _, _, _))
        .WillOnce([](auto, auto, auto &infos, auto) {
            AbilityRuntime::InsightIntentInfoForQuery info;
            info.bundleName = BUNDLE_NAME;
            info.moduleName = MODULE_NAME;
            info.intentName = INTENT_NAME;
            info.intentType = "test";
            infos.push_back(info);
            return ERR_OK;
        });

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("get insight intent info successfully (mode: by bundle)"), std::string::npos);
    EXPECT_NE(result.find(BUNDLE_NAME), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_1100
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent mode 3 without required params returns error.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_1100, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"3",
        (char*)"-f",
        (char*)"1",
        (char*)"-b",
        (char*)BUNDLE_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("error: mode 3 requires -b"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_1200
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent mode 3 with all params succeeds.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_1200, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"3",
        (char*)"-f",
        (char*)"1",
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
            info.bundleName = BUNDLE_NAME;
            info.moduleName = MODULE_NAME;
            info.intentName = INTENT_NAME;
            info.intentType = "test";
            return ERR_OK;
        });

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("get insight intent info successfully (mode: by intent)"), std::string::npos);
    EXPECT_NE(result.find(INTENT_NAME), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_1300
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent with invalid execute mode (4) returns error.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_1300, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"4",
        (char*)"-f",
        (char*)"1",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("error: invalid execute mode. Must be 1, 2, or 3"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_1400
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent mode 1 with service failure returns error.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_1400, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"1",
        (char*)"-f",
        (char*)"1",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    auto mockStub = new MockAbilityManagerStub();
    EXPECT_CALL(*mockStub, GetAllInsightIntentInfo(_, _, _))
        .WillOnce(Return(ERR_INVALID_VALUE));

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("error: failed to get insight intent info"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_1500
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent mode 2 with service failure returns error.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_1500, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"2",
        (char*)"-f",
        (char*)"1",
        (char*)"-b",
        (char*)BUNDLE_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    auto mockStub = new MockAbilityManagerStub();
    EXPECT_CALL(*mockStub, GetInsightIntentInfoByBundleName(_, _, _, _))
        .WillOnce(Return(ERR_INVALID_VALUE));

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("error: failed to get insight intent info by bundle"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_1600
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent mode 3 with service failure returns error.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_1600, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"3",
        (char*)"-f",
        (char*)"1",
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
        .WillOnce(Return(ERR_INVALID_VALUE));

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("error: failed to get insight intent info by intent name"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_1700
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent output includes detail fields for single intent.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_1700, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"3",
        (char*)"-f",
        (char*)"1",
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
            info.bundleName = BUNDLE_NAME;
            info.moduleName = MODULE_NAME;
            info.intentName = INTENT_NAME;
            info.intentType = "test";
            info.displayName = "TestDisplay";
            info.domain = "test_domain";
            info.intentVersion = "1.0";
            info.srcEntry = "./test.ts";
            info.displayDescription = "Test description";
            return ERR_OK;
        });

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("BundleName: " + BUNDLE_NAME), std::string::npos);
    EXPECT_NE(result.find("ModuleName: " + MODULE_NAME), std::string::npos);
    EXPECT_NE(result.find("IntentName: " + INTENT_NAME), std::string::npos);
    EXPECT_NE(result.find("DisplayName: TestDisplay"), std::string::npos);
    EXPECT_NE(result.find("Domain: test_domain"), std::string::npos);
    EXPECT_NE(result.find("IntentVersion: 1.0"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_1800
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent output includes PageInfo when available.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_1800, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"3",
        (char*)"-f",
        (char*)"1",
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
            info.bundleName = BUNDLE_NAME;
            info.moduleName = MODULE_NAME;
            info.intentName = INTENT_NAME;
            info.intentType = "test";
            info.pageInfo.uiAbility = "PageAbility";
            info.pageInfo.pagePath = "pages/Index";
            return ERR_OK;
        });

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("[PageInfo]"), std::string::npos);
    EXPECT_NE(result.find("UIAbility: PageAbility"), std::string::npos);
    EXPECT_NE(result.find("PagePath: pages/Index"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_1900
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent output includes EntryInfo when available.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_1900, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"3",
        (char*)"-f",
        (char*)"1",
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
            info.bundleName = BUNDLE_NAME;
            info.moduleName = MODULE_NAME;
            info.intentName = INTENT_NAME;
            info.intentType = "test";
            info.entryInfo.abilityName = "EntryAbility";
            info.entryInfo.executeMode = {ExecuteMode::UI_ABILITY_FOREGROUND};
            return ERR_OK;
        });

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("[EntryInfo]"), std::string::npos);
    EXPECT_NE(result.find("AbilityName: EntryAbility"), std::string::npos);
    EXPECT_NE(result.find("ExecuteMode: 0"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_2000
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent output includes UIAbilityInfo when available.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_2000, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"3",
        (char*)"-f",
        (char*)"1",
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
            info.bundleName = BUNDLE_NAME;
            info.moduleName = MODULE_NAME;
            info.intentName = INTENT_NAME;
            info.intentType = "test";
            info.uiAbilityIntentInfo.abilityName = "UIAbility";
            info.uiAbilityIntentInfo.supportExecuteMode = {
                ExecuteMode::UI_ABILITY_FOREGROUND,
                ExecuteMode::UI_ABILITY_BACKGROUND
            };
            return ERR_OK;
        });

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("[UIAbilityInfo]"), std::string::npos);
    EXPECT_NE(result.find("AbilityName: UIAbility"), std::string::npos);
    EXPECT_NE(result.find("SupportExecuteMode: 0, 1"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_2100
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent output includes Entities when available.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_2100, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"3",
        (char*)"-f",
        (char*)"1",
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
            info.bundleName = BUNDLE_NAME;
            info.moduleName = MODULE_NAME;
            info.intentName = INTENT_NAME;
            info.intentType = "test";
            AbilityRuntime::EntityInfoForQuery entity;
            entity.className = "TestClass";
            entity.entityId = "entity_001";
            entity.entityCategory = "category_a";
            entity.parameters = "param_desc";
            entity.parentClassName = "ParentClass";
            info.entities.push_back(entity);
            return ERR_OK;
        });

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("[Entities] count: 1"), std::string::npos);
    EXPECT_NE(result.find("ClassName: TestClass"), std::string::npos);
    EXPECT_NE(result.find("EntityId: entity_001"), std::string::npos);
    EXPECT_NE(result.find("EntityCategory: category_a"), std::string::npos);
    EXPECT_NE(result.find("ParentClass: ParentClass"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_2200
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent with unknown option returns error.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_2200, Function | MediumTest | Level1)
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
    EXPECT_NE(result.find("fail: unknown option"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_2300
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent mode 1 with custom user id.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_2300, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"1",
        (char*)"-f",
        (char*)"1",
        (char*)"-u",
        (char*)"200",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    auto mockStub = new MockAbilityManagerStub();
    EXPECT_CALL(*mockStub, GetAllInsightIntentInfo(_, _, _))
        .WillOnce([](auto, auto &infos, auto userId) {
            EXPECT_EQ(userId, 200);
            return ERR_OK;
        });

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("get insight intent info successfully (mode: all)"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_2400
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent output includes FormInfo when available.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_2400, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"3",
        (char*)"-f",
        (char*)"1",
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
            info.bundleName = BUNDLE_NAME;
            info.moduleName = MODULE_NAME;
            info.intentName = INTENT_NAME;
            info.intentType = "test";
            info.formInfo.abilityName = "FormAbility";
            info.formInfo.formName = "widget1";
            return ERR_OK;
        });

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("[FormInfo]"), std::string::npos);
    EXPECT_NE(result.find("AbilityName: FormAbility"), std::string::npos);
    EXPECT_NE(result.find("FormName: widget1"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_2500
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent mode 1 with multiple intents shows count correctly.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_2500, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"1",
        (char*)"-f",
        (char*)"1",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    auto mockStub = new MockAbilityManagerStub();
    EXPECT_CALL(*mockStub, GetAllInsightIntentInfo(_, _, _))
        .WillOnce([](auto, auto &infos, auto) {
            for (int i = 0; i < 3; i++) {
                AbilityRuntime::InsightIntentInfoForQuery info;
                info.bundleName = BUNDLE_NAME;
                info.moduleName = MODULE_NAME;
                info.intentName = "Intent" + std::to_string(i);
                info.intentType = "test";
                infos.push_back(info);
            }
            return ERR_OK;
        });

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("Total count: 3"), std::string::npos);
    EXPECT_NE(result.find("--- Insight Intent 1 ---"), std::string::npos);
    EXPECT_NE(result.find("--- Insight Intent 2 ---"), std::string::npos);
    EXPECT_NE(result.find("--- Insight Intent 3 ---"), std::string::npos);
}
