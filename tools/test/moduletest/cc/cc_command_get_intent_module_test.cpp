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
#include "insight_intent/insight_intent_info_for_query.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace {
const std::string TEST_TOOL_NAME = "ohos-claw-cc";
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
 * @tc.desc: Verify the "ohos-claw-cc get-intent -h" command shows help.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_0100, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-h",
        (char*)"",
    };
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("ohos-claw-cc get-intent"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent without required -e and -f returns error event.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_0200, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"",
    };
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"type\":\"result\",\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(result.find("Missing required parameters --executeMode and --flag"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent with missing -f returns error event.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_0300, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"1",
        (char*)"",
    };
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"type\":\"result\",\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(result.find("Missing required parameters --executeMode and --flag"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent with invalid execute mode value returns error event.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_0400, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"abc",
        (char*)"-f",
        (char*)"1",
        (char*)"",
    };
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"type\":\"result\",\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(result.find("Invalid executeMode value"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent with invalid flag value returns error event.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_0500, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"1",
        (char*)"-f",
        (char*)"xyz",
        (char*)"",
    };
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"type\":\"result\",\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(result.find("Invalid flag value"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_0600
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent with invalid user id value returns error event.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_0600, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"1",
        (char*)"-f",
        (char*)"1",
        (char*)"-u",
        (char*)"notanumber",
        (char*)"",
    };
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"type\":\"result\",\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(result.find("Invalid userId value"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_0700
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent mode 1 (query all) returns JSON result event.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_0700, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"1",
        (char*)"-f",
        (char*)"1",
        (char*)"",
    };
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

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
    EXPECT_NE(result.find("\"type\":\"result\",\"status\":\"success\""), std::string::npos);
    EXPECT_NE(result.find("\"mode\":\"all\""), std::string::npos);
    EXPECT_NE(result.find(BUNDLE_NAME), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_0800
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent mode 1 with empty result returns zero total.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_0800, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"1",
        (char*)"-f",
        (char*)"1",
        (char*)"",
    };
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

    auto mockStub = new MockAbilityManagerStub();
    EXPECT_CALL(*mockStub, GetAllInsightIntentInfo(_, _, _))
        .WillOnce(Return(ERR_OK));

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"type\":\"result\",\"status\":\"success\""), std::string::npos);
    EXPECT_NE(result.find("\"total\":0"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_0900
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent mode 2 without -b returns error event.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_0900, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"2",
        (char*)"-f",
        (char*)"1",
        (char*)"",
    };
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"type\":\"result\",\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(result.find("executeMode 2 requires --bundleName"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_1000
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent mode 2 with bundle name returns JSON result event.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_1000, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"2",
        (char*)"-f",
        (char*)"1",
        (char*)"-b",
        (char*)BUNDLE_NAME.c_str(),
        (char*)"",
    };
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

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
    EXPECT_NE(result.find("\"type\":\"result\",\"status\":\"success\""), std::string::npos);
    EXPECT_NE(result.find("\"mode\":\"by bundle\""), std::string::npos);
    EXPECT_NE(result.find(BUNDLE_NAME), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_1100
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent mode 3 without required params returns error event.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_1100, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"3",
        (char*)"-f",
        (char*)"1",
        (char*)"-b",
        (char*)BUNDLE_NAME.c_str(),
        (char*)"",
    };
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"type\":\"result\",\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(result.find("executeMode 3 requires --bundleName"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_1200
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent mode 3 with all params returns JSON result event.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_1200, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
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
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

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
    EXPECT_NE(result.find("\"type\":\"result\",\"status\":\"success\""), std::string::npos);
    EXPECT_NE(result.find("\"mode\":\"by intent\""), std::string::npos);
    EXPECT_NE(result.find(INTENT_NAME), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_1300
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent with invalid execute mode (4) returns error event.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_1300, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"4",
        (char*)"-f",
        (char*)"1",
        (char*)"",
    };
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"type\":\"result\",\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(result.find("Invalid executeMode value: 4"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_1400
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent mode 1 with service failure returns error event.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_1400, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"1",
        (char*)"-f",
        (char*)"1",
        (char*)"",
    };
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

    auto mockStub = new MockAbilityManagerStub();
    EXPECT_CALL(*mockStub, GetAllInsightIntentInfo(_, _, _))
        .WillOnce(Return(ERR_INVALID_VALUE));

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"type\":\"result\",\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(result.find("Failed to get all insight intent info"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_1500
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent mode 2 with service failure returns error event.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_1500, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"2",
        (char*)"-f",
        (char*)"1",
        (char*)"-b",
        (char*)BUNDLE_NAME.c_str(),
        (char*)"",
    };
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

    auto mockStub = new MockAbilityManagerStub();
    EXPECT_CALL(*mockStub, GetInsightIntentInfoByBundleName(_, _, _, _))
        .WillOnce(Return(ERR_INVALID_VALUE));

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"type\":\"result\",\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(result.find("Failed to get insight intent info by bundle"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_1600
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent mode 3 with service failure returns error event.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_1600, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
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
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

    auto mockStub = new MockAbilityManagerStub();
    EXPECT_CALL(*mockStub, GetInsightIntentInfoByIntentName(_, _, _, _, _, _))
        .WillOnce(Return(ERR_INVALID_VALUE));

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"type\":\"result\",\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(result.find("Failed to get insight intent info by intent name"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_1700
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent JSON output includes detail fields.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_1700, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
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
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

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
    EXPECT_NE(result.find("\"bundleName\":\"" + BUNDLE_NAME + "\""), std::string::npos);
    EXPECT_NE(result.find("\"moduleName\":\"" + MODULE_NAME + "\""), std::string::npos);
    EXPECT_NE(result.find("\"intentName\":\"" + INTENT_NAME + "\""), std::string::npos);
    EXPECT_NE(result.find("\"displayName\":\"TestDisplay\""), std::string::npos);
    EXPECT_NE(result.find("\"domain\":\"test_domain\""), std::string::npos);
    EXPECT_NE(result.find("\"intentVersion\":\"1.0\""), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_1800
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent JSON output includes pageInfo when available.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_1800, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
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
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

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
    EXPECT_NE(result.find("\"pageInfo\""), std::string::npos);
    EXPECT_NE(result.find("\"uiAbility\":\"PageAbility\""), std::string::npos);
    EXPECT_NE(result.find("\"pagePath\":\"pages/Index\""), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_1900
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent JSON output includes entryInfo when available.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_1900, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
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
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

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
    EXPECT_NE(result.find("\"entryInfo\""), std::string::npos);
    EXPECT_NE(result.find("\"abilityName\":\"EntryAbility\""), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_2000
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent JSON output includes entities when flag includes GET_ENTITY_INFO.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_2000, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"3",
        (char*)"-f",
        (char*)"5",
        (char*)"-b",
        (char*)BUNDLE_NAME.c_str(),
        (char*)"-m",
        (char*)MODULE_NAME.c_str(),
        (char*)"-i",
        (char*)INTENT_NAME.c_str(),
        (char*)"",
    };
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

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
    EXPECT_NE(result.find("\"entities\""), std::string::npos);
    EXPECT_NE(result.find("\"className\":\"TestClass\""), std::string::npos);
    EXPECT_NE(result.find("\"entityId\":\"entity_001\""), std::string::npos);
    EXPECT_NE(result.find("\"entityCategory\":\"category_a\""), std::string::npos);
    EXPECT_NE(result.find("\"parentClassName\":\"ParentClass\""), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_2100
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent with unknown option returns error event.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_2100, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-z",
        (char*)"",
    };
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"type\":\"result\",\"status\":\"failed\""), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_2200
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent mode 1 with custom user id.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_2200, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"1",
        (char*)"-f",
        (char*)"1",
        (char*)"-u",
        (char*)"200",
        (char*)"",
    };
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

    auto mockStub = new MockAbilityManagerStub();
    EXPECT_CALL(*mockStub, GetAllInsightIntentInfo(_, _, _))
        .WillOnce([](auto, auto &infos, auto userId) {
            EXPECT_EQ(userId, 200);
            return ERR_OK;
        });

    AbilityManagerClient::GetInstance()->proxy_ = sptr<IAbilityManager>(mockStub);

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("\"type\":\"result\",\"status\":\"success\""), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_2300
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent JSON output includes formInfo when available.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_2300, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
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
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

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
    EXPECT_NE(result.find("\"formInfo\""), std::string::npos);
    EXPECT_NE(result.find("\"abilityName\":\"FormAbility\""), std::string::npos);
    EXPECT_NE(result.find("\"formName\":\"widget1\""), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Get_Intent_ModuleTest_2400
 * @tc.name: ExecCommand
 * @tc.desc: Verify get-intent mode 1 with multiple intents shows correct total in JSON.
 */
HWTEST_F(CcCommandGetIntentModuleTest, Cc_Command_Get_Intent_ModuleTest_2400, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)"1",
        (char*)"-f",
        (char*)"1",
        (char*)"",
    };
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

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
    EXPECT_NE(result.find("\"total\":3"), std::string::npos);
    EXPECT_NE(result.find("Intent0"), std::string::npos);
    EXPECT_NE(result.find("Intent1"), std::string::npos);
    EXPECT_NE(result.find("Intent2"), std::string::npos);
}
