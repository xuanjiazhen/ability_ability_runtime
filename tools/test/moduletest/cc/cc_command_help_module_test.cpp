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

#define protected public
#include "cc_command.h"
#undef protected
#include "mock_ability_manager_stub.h"
#define private public
#include "ability_manager_client.h"
#undef private

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace {
const std::string TOOL_NAME = "claw_cc";
}  // namespace

class CcCommandHelpModuleTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects() const;
};

void CcCommandHelpModuleTest::SetUpTestCase()
{}

void CcCommandHelpModuleTest::TearDownTestCase()
{}

void CcCommandHelpModuleTest::SetUp()
{
    optind = 0;
    MakeMockObjects();
}

void CcCommandHelpModuleTest::TearDown()
{}

void CcCommandHelpModuleTest::MakeMockObjects() const
{
    auto managerStubPtr = sptr<IAbilityManager>(new MockAbilityManagerStub());
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

/**
 * @tc.number: Cc_Command_Help_ModuleTest_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "claw_cc help" command shows all available commands.
 */
HWTEST_F(CcCommandHelpModuleTest, Cc_Command_Help_ModuleTest_0100, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"help",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("usage: claw_cc <command> <options>"), std::string::npos);
    EXPECT_NE(result.find("help"), std::string::npos);
    EXPECT_NE(result.find("execute-intent"), std::string::npos);
    EXPECT_NE(result.find("get-intent"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Help_ModuleTest_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify invalid command shows error and help message.
 */
HWTEST_F(CcCommandHelpModuleTest, Cc_Command_Help_ModuleTest_0200, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"invalid_cmd",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("is not a valid claw_cc command"), std::string::npos);
    EXPECT_NE(result.find("usage: claw_cc <command> <options>"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Help_ModuleTest_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify "claw_cc" without subcommand shows help message.
 */
HWTEST_F(CcCommandHelpModuleTest, Cc_Command_Help_ModuleTest_0300, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("usage: claw_cc <command> <options>"), std::string::npos);
}
