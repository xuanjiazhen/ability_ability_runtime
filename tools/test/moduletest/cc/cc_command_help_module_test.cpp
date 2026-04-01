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

using namespace testing::ext;
using namespace testing;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace {
const std::string TEST_TOOL_NAME = "ohos-claw-cc";
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
 * @tc.desc: Verify the "ohos-claw-cc help" command shows main help message.
 */
HWTEST_F(CcCommandHelpModuleTest, Cc_Command_Help_ModuleTest_0100, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
        (char*)"help",
        (char*)"",
    };
    int argc = 0;
    while (argv[argc] != nullptr) {
        argc++;
    }

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("usage: ohos-claw-cc <command> <options>"), std::string::npos);
    EXPECT_NE(result.find("execute-intent"), std::string::npos);
    EXPECT_NE(result.find("get-intent"), std::string::npos);
}

/**
 * @tc.number: Cc_Command_Help_ModuleTest_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify "ohos-claw-cc" without subcommand shows help.
 */
HWTEST_F(CcCommandHelpModuleTest, Cc_Command_Help_ModuleTest_0200, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TEST_TOOL_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    InsightIntentShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("usage: ohos-claw-cc <command> <options>"), std::string::npos);
}
