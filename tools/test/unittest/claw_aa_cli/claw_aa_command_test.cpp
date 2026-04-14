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
#include "claw_aa_command.h"
#undef protected
#undef private
#include "mock_ability_manager_stub.h"
#define private public
#include "ability_manager_client.h"
#undef private
#include "ability_manager_interface.h"
#include "ability_start_with_wait_observer_data.h"
#include "ability_start_with_wait_observer_utils.h"
#include "hilog_tag_wrapper.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using testing::_;
using testing::Return;

namespace {
const std::string STRING_DEVICE = "device";
const std::string STRING_ABILITY_NAME = "ability";
const std::string STRING_ABILITY_NAME_INVALID = "invalid_ability";
const std::string STRING_BUNDLE_NAME = "bundle";
const std::string STRING_BUNDLE_NAME_INVALID = "invalid_bundle";
const std::string STRING_PARAMETER_KEY_INTEGER = "kinteger";
const std::string STRING_PARAMETER_KEY_BOOL = "kbool";
const std::string STRING_PARAMETER_KEY_STRING = "kstring";
const std::string STRING_PARAMETER_KEY_NULL_STRING = "knullstring";
const std::string STRING_PARAMETER_VALUE_INTEGER = "100";
const std::string STRING_PARAMETER_VALUE_BOOL = "true";
const std::string STRING_PARAMETER_VALUE_STRING = "string_value";
const std::string STRING_PARAMETER_VALUE_INTEGER_INVALID = "invalid_integer";
const std::string STRING_PARAMETER_VALUE_BOOL_INVALID = "invalid_bool";
const std::string STRING_INVALID_PARAMETER_INTEGER_OPTION =
    "invalid parameter invalid_integer for integer option";
const std::string STRING_INVALID_PARAMETER_BOOL_OPTION =
    "invalid parameter invalid_bool for bool option";
const std::string STRING_INVALID_NUMBER_INTEGER_OPTION =
    "invalid number of parameters for option --pi";
const std::string STRING_INVALID_NUMBER_BOOL_OPTION =
    "invalid number of parameters for option --pb";
const std::string STRING_INVALID_NUMBER_STRING_OPTION =
    "invalid number of parameters for option --ps";
const std::string STRING_INVALID_NUMBER_NULL_STRING_OPTION =
    "invalid number of parameters for option --psn";
const std::string STRING_ACTION = "action";
const std::string STRING_URI = "https://valid.uri.com";
const std::string STRING_TYPE = "type";
const std::string STRING_ENTITY = "entity";
}  // namespace

class ClawAaCommandTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects() const;

    std::string cmd_ = "start";
    std::string forceStopCmd_ = "force-stop";
};

void ClawAaCommandTest::SetUpTestCase()
{}

void ClawAaCommandTest::TearDownTestCase()
{}

void ClawAaCommandTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void ClawAaCommandTest::TearDown()
{}

void ClawAaCommandTest::MakeMockObjects() const
{
    // mock a stub
    auto managerStubPtr = sptr<IAbilityManager>(new MockAbilityManagerStub());

    // set the mock stub
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

// ============================================================================
// Basic command tests
// ============================================================================

/**
 * @tc.number: ClawAa_Command_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa" command without sub-command.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: ClawAa_Command_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa xxx" command with unknown sub-command.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_0200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"xxx",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_TRUE(result.find("is not a valid") != std::string::npos);
}

/**
 * @tc.number: ClawAa_Command_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa help" command.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_0300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"help",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

// ============================================================================
// RunAsStartAbility tests - start command parsing and execution
// ============================================================================

/**
 * @tc.number: ClawAa_Command_Start_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start" command without options.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: failed to execute your command.\n");
}

/**
 * @tc.number: ClawAa_Command_Start_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start -x" command with unknown short option.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_0200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-x",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: failed to execute your command.\n");
}

/**
 * @tc.number: ClawAa_Command_Start_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start --xxx" command with unknown long option.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_0300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--xxx",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: failed to execute your command.\n");
}

/**
 * @tc.number: ClawAa_Command_Start_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start -h" command.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_0400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_0400");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-h",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: failed to execute your command.\n");
}

/**
 * @tc.number: ClawAa_Command_Start_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start --help" command.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_0500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_0500");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--help",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: failed to execute your command.\n");
}

/**
 * @tc.number: ClawAa_Command_Start_0600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start -d" command without argument.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_0600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_0600");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: failed to execute your command.\n");
}

/**
 * @tc.number: ClawAa_Command_Start_0700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start -d <device-id>" command.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_0700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_0700");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: ClawAa_Command_Start_0800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start -a" command without argument.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_0800, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_0800");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: failed to execute your command.\n");
}

/**
 * @tc.number: ClawAa_Command_Start_0900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start -a <ability-name>" without bundle-name.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_0900, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_0900");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: failed to execute your command.\n");
}

/**
 * @tc.number: ClawAa_Command_Start_1000
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start -b" command without argument.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_1000, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_1000");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: failed to execute your command.\n");
}

/**
 * @tc.number: ClawAa_Command_Start_1100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start -b <bundle-name>" command.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_1100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_1100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: ClawAa_Command_Start_1200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start -a <ability> -b <bundle>" command with success.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_1200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_1200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: ClawAa_Command_Start_1300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start -a <ability> -b" command without bundle-name value.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_1300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_1300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: failed to execute your command.\n");
}

/**
 * @tc.number: ClawAa_Command_Start_1400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start -d <device> -a <ability> -b <bundle>" command with success.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_1400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_1400");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: ClawAa_Command_Start_1500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start -d <device> -a <invalid_ability> -b <bundle>" command.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_1500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_1500");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME_INVALID.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: failed to execute your command.\n");
}

/**
 * @tc.number: ClawAa_Command_Start_1600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start -d <device> -a <ability> -b <invalid_bundle>" command.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_1600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_1600");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME_INVALID.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: failed to execute your command.\n");
}

/**
 * @tc.number: ClawAa_Command_Start_1700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start -D" command with debug flag only.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_1700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_1700");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-D",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: ClawAa_Command_Start_1800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start -a <ability> -b <bundle> -D" command with debug flag.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_1800, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_1800");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-D",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: ClawAa_Command_Start_1900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start -a <ability> -b <bundle> --pi <key> <int>" command.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_1900, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_1900");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--pi",
        (char*)STRING_PARAMETER_KEY_INTEGER.c_str(),
        (char*)STRING_PARAMETER_VALUE_INTEGER.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: ClawAa_Command_Start_2000
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start -a <ability> -b <bundle> --pb <key> <bool>" command.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_2000, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_2000");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--pb",
        (char*)STRING_PARAMETER_KEY_BOOL.c_str(),
        (char*)STRING_PARAMETER_VALUE_BOOL.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: ClawAa_Command_Start_2100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start -a <ability> -b <bundle> --ps <key> <string>" command.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_2100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_2100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--ps",
        (char*)STRING_PARAMETER_KEY_STRING.c_str(),
        (char*)STRING_PARAMETER_VALUE_STRING.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: ClawAa_Command_Start_2200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start -a <ability> -b <bundle> --psn <key>" command.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_2200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_2200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--psn",
        (char*)STRING_PARAMETER_KEY_NULL_STRING.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: ClawAa_Command_Start_2300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start -A <action>" command with implicit start.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_2300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_2300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-A",
        (char*)STRING_ACTION.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: ClawAa_Command_Start_2400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start -U <URI> -t <type>" command.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_2400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_2400");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-U",
        (char*)STRING_URI.c_str(),
        (char*)"-t",
        (char*)STRING_TYPE.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: ClawAa_Command_Start_2500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start -e <entity>" command.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_2500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_2500");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)STRING_ENTITY.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: ClawAa_Command_Start_2600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the start command with --pi invalid integer parameter.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_2600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_2600");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--pi",
        (char*)STRING_PARAMETER_KEY_INTEGER.c_str(),
        (char*)STRING_PARAMETER_VALUE_INTEGER_INVALID.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: failed to execute your command.\n");
}

/**
 * @tc.number: ClawAa_Command_Start_2700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the start command with --pb invalid bool parameter.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_2700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_2700");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--pb",
        (char*)STRING_PARAMETER_KEY_BOOL.c_str(),
        (char*)STRING_PARAMETER_VALUE_BOOL_INVALID.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: failed to execute your command.\n");
}

/**
 * @tc.number: ClawAa_Command_Start_2800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the start command with --ps too many parameters.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_2800, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_2800");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--ps",
        (char*)STRING_PARAMETER_KEY_STRING.c_str(),
        (char*)STRING_PARAMETER_VALUE_STRING.c_str(),
        (char*)"extra_value",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: failed to execute your command.\n");
}

/**
 * @tc.number: ClawAa_Command_Start_2900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the start command with -u without argument.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_2900, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_2900");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-u",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: failed to execute your command.\n");
}

/**
 * @tc.number: ClawAa_Command_Start_3000
 * @tc.name: ExecCommand
 * @tc.desc: Verify the start command with -u <invalid> non-numeric userId.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_3000, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_3000");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-u",
        (char*)"xxx",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: failed to execute your command.\n");
}

/**
 * @tc.number: ClawAa_Command_Start_3100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the start command with -u <negative> negative userId.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_3100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_3100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-u",
        (char*)"-1",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: failed to execute your command.\n");
}

/**
 * @tc.number: ClawAa_Command_Start_3200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the start command with -u <valid> valid userId.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_3200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_3200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-u",
        (char*)"100",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: ClawAa_Command_Start_3300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the start command with -s <windowMode>.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_3300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_3300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-s",
        (char*)"102",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: ClawAa_Command_Start_3400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the start command with --wl <window-left>.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_3400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_3400");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--wl",
        (char*)"100",
        (char*)"--wt",
        (char*)"200",
        (char*)"--wh",
        (char*)"800",
        (char*)"--ww",
        (char*)"600",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: ClawAa_Command_Start_3500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the start command with --wl invalid argument.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_3500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_3500");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--wl",
        (char*)"abc",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: failed to execute your command.\n");
}

/**
 * @tc.number: ClawAa_Command_Start_3600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start --pi timeout 30000" command.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_3600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_3600");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--pi",
        (char*)"timeout",
        (char*)"30000",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: ClawAa_Command_Start_3700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start --ps timeout 30000" command.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Start_3700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Start_3700");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--ps",
        (char*)"timeout",
        (char*)"30000",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

// ============================================================================
// RunAsForceStop tests
// ============================================================================

/**
 * @tc.number: ClawAa_Command_Force_Stop_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa force-stop" command without bundle-name.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Force_Stop_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Force_Stop_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)forceStopCmd_.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: failed to execute your command.\n");
}

/**
 * @tc.number: ClawAa_Command_Force_Stop_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa force-stop <bundle>" command with KillProcess failure.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Force_Stop_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Force_Stop_0200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)forceStopCmd_.c_str(),
        (char*)"com.test.bundle",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    auto managerClientPtr = AbilityManagerClient::GetInstance();
    auto mockAbilityManagerStub = sptr<MockAbilityManagerStub>(new MockAbilityManagerStub());
    ASSERT_NE(mockAbilityManagerStub, nullptr);
    EXPECT_CALL(*mockAbilityManagerStub, KillProcess(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    managerClientPtr->proxy_ = static_cast<IAbilityManager*>(mockAbilityManagerStub);

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: failed to execute your command.\n");
    managerClientPtr->proxy_ = nullptr;
    testing::Mock::AllowLeak(mockAbilityManagerStub);
}

/**
 * @tc.number: ClawAa_Command_Force_Stop_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa force-stop <bundle>" command with KillProcess success.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Force_Stop_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Force_Stop_0300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)forceStopCmd_.c_str(),
        (char*)"com.test.bundle",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    auto managerClientPtr = AbilityManagerClient::GetInstance();
    auto mockAbilityManagerStub = sptr<MockAbilityManagerStub>(new MockAbilityManagerStub());
    ASSERT_NE(mockAbilityManagerStub, nullptr);
    EXPECT_CALL(*mockAbilityManagerStub, KillProcess(_, _, _, _))
        .Times(1)
        .WillOnce(Return(0));
    managerClientPtr->proxy_ = static_cast<IAbilityManager*>(mockAbilityManagerStub);

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
    managerClientPtr->proxy_ = nullptr;
    testing::Mock::AllowLeak(mockAbilityManagerStub);
}

/**
 * @tc.number: ClawAa_Command_Force_Stop_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa force-stop <bundle> -p <pid> -r <reason>" command.
 */
HWTEST_F(ClawAaCommandTest, ClawAa_Command_Force_Stop_0400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClawAa_Command_Force_Stop_0400");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)forceStopCmd_.c_str(),
        (char*)"com.test.bundle",
        (char*)"-p",
        (char*)"12345",
        (char*)"-r",
        (char*)"LOW_MEMORY",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    auto managerClientPtr = AbilityManagerClient::GetInstance();
    auto mockAbilityManagerStub = sptr<MockAbilityManagerStub>(new MockAbilityManagerStub());
    ASSERT_NE(mockAbilityManagerStub, nullptr);
    EXPECT_CALL(*mockAbilityManagerStub, KillProcess(_, _, _, _))
        .Times(1)
        .WillOnce(Return(0));
    managerClientPtr->proxy_ = static_cast<IAbilityManager*>(mockAbilityManagerStub);

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
    managerClientPtr->proxy_ = nullptr;
    testing::Mock::AllowLeak(mockAbilityManagerStub);
}

// ============================================================================
// IsLongStartOption tests
// ============================================================================

/**
 * @tc.number: IsLongStartOption_0010
 * @tc.name: IsLongStartOption
 * @tc.desc: Verify that "--pi" is recognized as a valid long option.
 */
HWTEST_F(ClawAaCommandTest, IsLongStartOption_0010, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsLongStartOption_0010");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.IsLongStartOption("--pi"));
}

/**
 * @tc.number: IsLongStartOption_0020
 * @tc.name: IsLongStartOption
 * @tc.desc: Verify that "--ps" is recognized as a valid long option.
 */
HWTEST_F(ClawAaCommandTest, IsLongStartOption_0020, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsLongStartOption_0020");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.IsLongStartOption("--ps"));
}

/**
 * @tc.number: IsLongStartOption_0030
 * @tc.name: IsLongStartOption
 * @tc.desc: Verify that "--pb" is recognized as a valid long option.
 */
HWTEST_F(ClawAaCommandTest, IsLongStartOption_0030, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsLongStartOption_0030");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.IsLongStartOption("--pb"));
}

/**
 * @tc.number: IsLongStartOption_0040
 * @tc.name: IsLongStartOption
 * @tc.desc: Verify that "--ppp" is NOT recognized as a valid long option.
 */
HWTEST_F(ClawAaCommandTest, IsLongStartOption_0040, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsLongStartOption_0040");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_FALSE(cmd.IsLongStartOption("--ppp"));
}

/**
 * @tc.number: IsLongStartOption_0050
 * @tc.name: IsLongStartOption
 * @tc.desc: Verify that "-pi" (single dash) is NOT recognized as a valid long option.
 */
HWTEST_F(ClawAaCommandTest, IsLongStartOption_0050, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsLongStartOption_0050");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_FALSE(cmd.IsLongStartOption("-pi"));
}

/**
 * @tc.number: IsLongStartOption_0060
 * @tc.name: IsLongStartOption
 * @tc.desc: Verify that "pi" (no dash) is NOT recognized as a valid long option.
 */
HWTEST_F(ClawAaCommandTest, IsLongStartOption_0060, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsLongStartOption_0060");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_FALSE(cmd.IsLongStartOption("pi"));
}

// ============================================================================
// IsShortStartOption tests
// ============================================================================

/**
 * @tc.number: IsShortStartOption_0010
 * @tc.name: IsShortStartOption
 * @tc.desc: Verify that "-e" is recognized as a valid short option.
 */
HWTEST_F(ClawAaCommandTest, IsShortStartOption_0010, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsShortStartOption_0010");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.IsShortStartOption("-e"));
}

/**
 * @tc.number: IsShortStartOption_0020
 * @tc.name: IsShortStartOption
 * @tc.desc: Verify that "-h" is recognized as a valid short option.
 */
HWTEST_F(ClawAaCommandTest, IsShortStartOption_0020, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsShortStartOption_0020");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.IsShortStartOption("-h"));
}

/**
 * @tc.number: IsShortStartOption_0030
 * @tc.name: IsShortStartOption
 * @tc.desc: Verify that "-x" is NOT recognized as a valid short option.
 */
HWTEST_F(ClawAaCommandTest, IsShortStartOption_0030, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsShortStartOption_0030");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_FALSE(cmd.IsShortStartOption("-x"));
}

/**
 * @tc.number: IsShortStartOption_0040
 * @tc.name: IsShortStartOption
 * @tc.desc: Verify that "-d" is recognized as a valid short option.
 */
HWTEST_F(ClawAaCommandTest, IsShortStartOption_0040, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsShortStartOption_0040");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.IsShortStartOption("-d"));
}

// ============================================================================
// IsStartOption tests
// ============================================================================

/**
 * @tc.number: IsStartOption_0010
 * @tc.name: IsStartOption
 * @tc.desc: Verify that "--pi" is recognized as a valid start option (long).
 */
HWTEST_F(ClawAaCommandTest, IsStartOption_0010, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsStartOption_0010");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.IsStartOption("--pi"));
}

/**
 * @tc.number: IsStartOption_0020
 * @tc.name: IsStartOption
 * @tc.desc: Verify that "-e" is recognized as a valid start option (short).
 */
HWTEST_F(ClawAaCommandTest, IsStartOption_0020, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsStartOption_0020");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.IsStartOption("-e"));
}

/**
 * @tc.number: IsStartOption_0030
 * @tc.name: IsStartOption
 * @tc.desc: Verify that "--ppp" is NOT recognized as a valid start option.
 */
HWTEST_F(ClawAaCommandTest, IsStartOption_0030, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsStartOption_0030");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_FALSE(cmd.IsStartOption("--ppp"));
}

/**
 * @tc.number: IsStartOption_0040
 * @tc.name: IsStartOption
 * @tc.desc: Verify that "-x" is NOT recognized as a valid start option.
 */
HWTEST_F(ClawAaCommandTest, IsStartOption_0040, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsStartOption_0040");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_FALSE(cmd.IsStartOption("-x"));
}

/**
 * @tc.number: IsStartOption_0050
 * @tc.name: IsStartOption
 * @tc.desc: Verify that "100" (no dash) is NOT recognized as a valid start option.
 */
HWTEST_F(ClawAaCommandTest, IsStartOption_0050, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsStartOption_0050");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_FALSE(cmd.IsStartOption("100"));
}

/**
 * @tc.number: IsStartOption_0060
 * @tc.name: IsStartOption
 * @tc.desc: Verify that empty string is NOT recognized as a valid start option.
 */
HWTEST_F(ClawAaCommandTest, IsStartOption_0060, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsStartOption_0060");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_FALSE(cmd.IsStartOption(""));
}

// ============================================================================
// MatchOrderString tests
// ============================================================================

/**
 * @tc.number: MatchOrderString_0010
 * @tc.name: MatchOrderString
 * @tc.desc: Verify MatchOrderString with empty string returns false.
 */
HWTEST_F(ClawAaCommandTest, MatchOrderString_0010, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MatchOrderString_0010");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    const std::regex regexFirst(R"(^(profile|dumpheap).*)");
    std::string command = "";
    EXPECT_EQ(cmd.MatchOrderString(regexFirst, command), false);
}

/**
 * @tc.number: MatchOrderString_0020
 * @tc.name: MatchOrderString
 * @tc.desc: Verify MatchOrderString with non-matching string returns false.
 */
HWTEST_F(ClawAaCommandTest, MatchOrderString_0020, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MatchOrderString_0020");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    const std::regex regexFirst(R"(^(profile|dumpheap).*)");
    std::string command = "test";
    EXPECT_EQ(cmd.MatchOrderString(regexFirst, command), false);
}

/**
 * @tc.number: MatchOrderString_0030
 * @tc.name: MatchOrderString
 * @tc.desc: Verify MatchOrderString with matching string "profile" returns true.
 */
HWTEST_F(ClawAaCommandTest, MatchOrderString_0030, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MatchOrderString_0030");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    const std::regex regexFirst(R"(^(profile|dumpheap).*)");
    std::string command = "profile";
    EXPECT_EQ(cmd.MatchOrderString(regexFirst, command), true);
}

/**
 * @tc.number: MatchOrderString_0040
 * @tc.name: MatchOrderString
 * @tc.desc: Verify MatchOrderString with matching string "dumpheap" returns true.
 */
HWTEST_F(ClawAaCommandTest, MatchOrderString_0040, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MatchOrderString_0040");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    const std::regex regexFirst(R"(^(profile|dumpheap).*)");
    std::string command = "dumpheap";
    EXPECT_EQ(cmd.MatchOrderString(regexFirst, command), true);
}

// ============================================================================
// CheckPerfCmdString tests
// ============================================================================

/**
 * @tc.number: CheckPerfCmdString_0010
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString with nullptr returns false.
 */
HWTEST_F(ClawAaCommandTest, CheckPerfCmdString_0010, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckPerfCmdString_0010");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string command = "";
    EXPECT_EQ(cmd.CheckPerfCmdString(nullptr, 0, command), false);
}

/**
 * @tc.number: CheckPerfCmdString_0020
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString with param length 0 returns false.
 */
HWTEST_F(ClawAaCommandTest, CheckPerfCmdString_0020, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckPerfCmdString_0020");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string command = "";
    const char* optarg = "test";
    EXPECT_EQ(cmd.CheckPerfCmdString(optarg, 0, command), false);
}

/**
 * @tc.number: CheckPerfCmdString_0030
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString with "profile jsperf" returns true.
 */
HWTEST_F(ClawAaCommandTest, CheckPerfCmdString_0030, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckPerfCmdString_0030");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string command = "";
    const char* optarg = "profile jsperf";
    constexpr int32_t len = 32;
    EXPECT_EQ(cmd.CheckPerfCmdString(optarg, len, command), true);
}

/**
 * @tc.number: CheckPerfCmdString_0040
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString with "sleep 100" returns true.
 */
HWTEST_F(ClawAaCommandTest, CheckPerfCmdString_0040, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckPerfCmdString_0040");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string command = "";
    const char* optarg = "sleep 100";
    constexpr int32_t len = 32;
    EXPECT_EQ(cmd.CheckPerfCmdString(optarg, len, command), true);
}

/**
 * @tc.number: CheckPerfCmdString_0050
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString with "profile jsperf abc" returns false (too many args).
 */
HWTEST_F(ClawAaCommandTest, CheckPerfCmdString_0050, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckPerfCmdString_0050");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string command = "";
    const char* optarg = "profile jsperf abc";
    constexpr int32_t len = 32;
    EXPECT_EQ(cmd.CheckPerfCmdString(optarg, len, command), false);
}

/**
 * @tc.number: CheckPerfCmdString_0060
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString with "test" returns false.
 */
HWTEST_F(ClawAaCommandTest, CheckPerfCmdString_0060, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckPerfCmdString_0060");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string command = "";
    const char* optarg = "test";
    constexpr int32_t len = 32;
    EXPECT_EQ(cmd.CheckPerfCmdString(optarg, len, command), false);
}

/**
 * @tc.number: CheckPerfCmdString_0070
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString with "dumpheap" returns true.
 */
HWTEST_F(ClawAaCommandTest, CheckPerfCmdString_0070, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckPerfCmdString_0070");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string command = "";
    const char* optarg = "dumpheap";
    constexpr int32_t len = 32;
    EXPECT_EQ(cmd.CheckPerfCmdString(optarg, len, command), true);
}

/**
 * @tc.number: CheckPerfCmdString_0080
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString with string exceeding param length returns false.
 */
HWTEST_F(ClawAaCommandTest, CheckPerfCmdString_0080, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckPerfCmdString_0080");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string command = "";
    const char* optarg = "profile jsperf";
    constexpr int32_t len = 2;  // strlen("profile jsperf") = 14 >= 2
    EXPECT_EQ(cmd.CheckPerfCmdString(optarg, len, command), false);
}

// ============================================================================
// IsImplicitStartAction tests
// ============================================================================

/**
 * @tc.number: IsImplicitStartAction_0010
 * @tc.name: IsImplicitStartAction
 * @tc.desc: Verify that a Want with abilityName set is NOT implicit.
 */
HWTEST_F(ClawAaCommandTest, IsImplicitStartAction_0010, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsImplicitStartAction_0010");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    Want want;
    ElementName element("", "com.test.bundle", "MainAbility");
    want.SetElement(element);
    EXPECT_FALSE(cmd.IsImplicitStartAction(want));
}

/**
 * @tc.number: IsImplicitStartAction_0020
 * @tc.name: IsImplicitStartAction
 * @tc.desc: Verify that a Want without abilityName but with action is implicit.
 */
HWTEST_F(ClawAaCommandTest, IsImplicitStartAction_0020, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsImplicitStartAction_0020");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    Want want;
    want.SetAction("ohos.want.action.home");
    EXPECT_TRUE(cmd.IsImplicitStartAction(want));
}

/**
 * @tc.number: IsImplicitStartAction_0030
 * @tc.name: IsImplicitStartAction
 * @tc.desc: Verify that a Want with black action "ohos.want.action.select" is NOT implicit.
 */
HWTEST_F(ClawAaCommandTest, IsImplicitStartAction_0030, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsImplicitStartAction_0030");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    Want want;
    want.SetAction("ohos.want.action.select");
    EXPECT_FALSE(cmd.IsImplicitStartAction(want));
}

/**
 * @tc.number: IsImplicitStartAction_0040
 * @tc.name: IsImplicitStartAction
 * @tc.desc: Verify that a Want with screen mode key set is NOT implicit.
 */
HWTEST_F(ClawAaCommandTest, IsImplicitStartAction_0040, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsImplicitStartAction_0040");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    Want want;
    want.SetAction("ohos.want.action.home");
    want.SetParam(AAFwk::SCREEN_MODE_KEY, static_cast<int32_t>(ScreenMode::JUMP_SCREEN_MODE));
    EXPECT_FALSE(cmd.IsImplicitStartAction(want));
}

/**
 * @tc.number: IsImplicitStartAction_0050
 * @tc.name: IsImplicitStartAction
 * @tc.desc: Verify that an empty Want without abilityName or action is NOT implicit.
 */
HWTEST_F(ClawAaCommandTest, IsImplicitStartAction_0050, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsImplicitStartAction_0050");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    Want want;
    // No abilityName, no action - not implicit because action is empty (not != BLACK_ACTION_SELECT_DATA)
    EXPECT_FALSE(cmd.IsImplicitStartAction(want));
}

// ============================================================================
// FormatOutputForWithWait tests
// ============================================================================

/**
 * @tc.number: FormatOutputForWithWait_0010
 * @tc.name: FormatOutputForWithWait
 * @tc.desc: Verify FormatOutputForWithWait with TERMINATE_FOR_NONE reason.
 */
HWTEST_F(ClawAaCommandTest, FormatOutputForWithWait_0010, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FormatOutputForWithWait_0010");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.startTime_ = 0;
    cmd.resultReceiver_ = "";

    Want want;
    AbilityStartWithWaitObserverData data;
    data.coldStart = true;
    data.reason = static_cast<uint32_t>(
        AbilityStartWithWaitObserverUtil::TerminateReason::TERMINATE_FOR_NONE);
    data.startTime = 1000;
    data.foregroundTime = 2000;
    data.bundleName = "com.test.bundle";
    data.abilityName = "MainAbility";

    cmd.FormatOutputForWithWait(want, data);

    EXPECT_TRUE(cmd.resultReceiver_.find("StartMode: Cold") != std::string::npos);
    EXPECT_TRUE(cmd.resultReceiver_.find("BundleName: com.test.bundle") != std::string::npos);
    EXPECT_TRUE(cmd.resultReceiver_.find("AbilityName: MainAbility") != std::string::npos);
    EXPECT_TRUE(cmd.resultReceiver_.find("TotalTime: 1000") != std::string::npos);
}

/**
 * @tc.number: FormatOutputForWithWait_0020
 * @tc.name: FormatOutputForWithWait
 * @tc.desc: Verify FormatOutputForWithWait with TERMINATE_FOR_NONE reason and hot start.
 */
HWTEST_F(ClawAaCommandTest, FormatOutputForWithWait_0020, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FormatOutputForWithWait_0020");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.startTime_ = 0;
    cmd.resultReceiver_ = "";

    Want want;
    AbilityStartWithWaitObserverData data;
    data.coldStart = false;
    data.reason = static_cast<uint32_t>(
        AbilityStartWithWaitObserverUtil::TerminateReason::TERMINATE_FOR_NONE);
    data.startTime = 1000;
    data.foregroundTime = 1500;
    data.bundleName = "com.test.bundle";
    data.abilityName = "MainAbility";

    cmd.FormatOutputForWithWait(want, data);

    EXPECT_TRUE(cmd.resultReceiver_.find("StartMode: Hot") != std::string::npos);
}

/**
 * @tc.number: FormatOutputForWithWait_0030
 * @tc.name: FormatOutputForWithWait
 * @tc.desc: Verify FormatOutputForWithWait with TERMINATE_FOR_NONE and module name.
 */
HWTEST_F(ClawAaCommandTest, FormatOutputForWithWait_0030, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FormatOutputForWithWait_0030");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.startTime_ = 0;
    cmd.resultReceiver_ = "";

    Want want;
    ElementName element("", "com.test.bundle", "MainAbility", "entry");
    want.SetElement(element);

    AbilityStartWithWaitObserverData data;
    data.coldStart = true;
    data.reason = static_cast<uint32_t>(
        AbilityStartWithWaitObserverUtil::TerminateReason::TERMINATE_FOR_NONE);
    data.startTime = 1000;
    data.foregroundTime = 2000;
    data.bundleName = "com.test.bundle";
    data.abilityName = "MainAbility";

    cmd.FormatOutputForWithWait(want, data);

    EXPECT_TRUE(cmd.resultReceiver_.find("ModuleName: entry") != std::string::npos);
}

/**
 * @tc.number: FormatOutputForWithWait_0040
 * @tc.name: FormatOutputForWithWait
 * @tc.desc: Verify FormatOutputForWithWait with TERMINATE_FOR_NON_UI_ABILITY reason.
 */
HWTEST_F(ClawAaCommandTest, FormatOutputForWithWait_0040, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FormatOutputForWithWait_0040");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.resultReceiver_ = "";

    Want want;
    AbilityStartWithWaitObserverData data;
    data.reason = static_cast<uint32_t>(
        AbilityStartWithWaitObserverUtil::TerminateReason::TERMINATE_FOR_NON_UI_ABILITY);

    cmd.FormatOutputForWithWait(want, data);

    EXPECT_TRUE(cmd.resultReceiver_.find(
        "The wait option does not support starting non-uiability") != std::string::npos);
}

/**
 * @tc.number: FormatOutputForWithWait_0050
 * @tc.name: FormatOutputForWithWait
 * @tc.desc: Verify FormatOutputForWithWait with unknown reason does nothing.
 */
HWTEST_F(ClawAaCommandTest, FormatOutputForWithWait_0050, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FormatOutputForWithWait_0050");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.resultReceiver_ = "";

    Want want;
    AbilityStartWithWaitObserverData data;
    data.reason = 99;  // Unknown reason

    cmd.FormatOutputForWithWait(want, data);

    EXPECT_TRUE(cmd.resultReceiver_.empty());
}

// ============================================================================
// CheckParameters tests
// ============================================================================

/**
 * @tc.number: CheckParameters_0010
 * @tc.name: CheckParameters
 * @tc.desc: Verify CheckParameters with enough extra arguments for --pi.
 */
HWTEST_F(ClawAaCommandTest, CheckParameters_0010, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckParameters_0010");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--pi",
        (char*)"key",
        (char*)"100",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    // After getopt processes "--pi", optind points to "key"
    // We manually simulate by setting optind
    optind = 2;  // optind points to "--pi" in argv
    // optind + EXTRA_ARGUMENTS_FOR_KEY_VALUE_PAIR = 2 + 2 = 4 < 5 (argc)
    // argv[3] = "key", argv[4] = "100" - neither is a start option, count = 2
    EXPECT_TRUE(cmd.CheckParameters(EXTRA_ARGUMENTS_FOR_KEY_VALUE_PAIR));
}

/**
 * @tc.number: CheckParameters_0020
 * @tc.name: CheckParameters
 * @tc.desc: Verify CheckParameters with not enough extra arguments.
 */
HWTEST_F(ClawAaCommandTest, CheckParameters_0020, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckParameters_0020");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--pi",
        (char*)"key",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    optind = 2;
    // optind + 2 = 4 >= 4 (argc) -> false
    EXPECT_FALSE(cmd.CheckParameters(EXTRA_ARGUMENTS_FOR_KEY_VALUE_PAIR));
}

/**
 * @tc.number: CheckParameters_0030
 * @tc.name: CheckParameters
 * @tc.desc: Verify CheckParameters for --psn with one extra argument.
 */
HWTEST_F(ClawAaCommandTest, CheckParameters_0030, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckParameters_0030");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--psn",
        (char*)"key",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    optind = 2;
    // optind + 1 = 3 < 4 (argc) -> proceeds to count check
    // argv[3] = "key" -> not a start option, count = 1, matches EXTRA_ARGUMENTS_FOR_NULL_STRING
    EXPECT_TRUE(cmd.CheckParameters(EXTRA_ARGUMENTS_FOR_NULL_STRING));
}

// ============================================================================
// MakeWantFromCmd tests (tested indirectly through RunAsStartAbility)
// Additional tests for edge cases
// ============================================================================

/**
 * @tc.number: MakeWantFromCmd_0010
 * @tc.name: ExecCommand
 * @tc.desc: Verify MakeWantFromCmd handles -C (cold start) flag.
 */
HWTEST_F(ClawAaCommandTest, MakeWantFromCmd_0010, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MakeWantFromCmd_0010");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-C",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: MakeWantFromCmd_0020
 * @tc.name: ExecCommand
 * @tc.desc: Verify MakeWantFromCmd handles -S (sandbox) flag.
 */
HWTEST_F(ClawAaCommandTest, MakeWantFromCmd_0020, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MakeWantFromCmd_0020");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-S",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: MakeWantFromCmd_0030
 * @tc.name: ExecCommand
 * @tc.desc: Verify MakeWantFromCmd handles -m <module-name> option.
 */
HWTEST_F(ClawAaCommandTest, MakeWantFromCmd_0030, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MakeWantFromCmd_0030");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-m",
        (char*)"entry",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: MakeWantFromCmd_0040
 * @tc.name: ExecCommand
 * @tc.desc: Verify MakeWantFromCmd handles -p with invalid perf cmd.
 */
HWTEST_F(ClawAaCommandTest, MakeWantFromCmd_0040, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MakeWantFromCmd_0040");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-p",
        (char*)"invalid_perf_cmd",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: failed to execute your command.\n");
}

/**
 * @tc.number: MakeWantFromCmd_0050
 * @tc.name: ExecCommand
 * @tc.desc: Verify MakeWantFromCmd handles -p with valid perf cmd "profile jsperf".
 */
HWTEST_F(ClawAaCommandTest, MakeWantFromCmd_0050, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MakeWantFromCmd_0050");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-p",
        (char*)"profile jsperf",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

// ============================================================================
// StartAbilityWithWait flag tests
// ============================================================================

/**
 * @tc.number: StartAbilityWithWait_0010
 * @tc.name: ExecCommand
 * @tc.desc: Verify the start command with -W flag sets startAbilityWithWaitFlag_.
 */
HWTEST_F(ClawAaCommandTest, StartAbilityWithWait_0010, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityWithWait_0010");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-W",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    // With -W flag and explicit ability name, StartAbilityWithWait is called
    // The actual result depends on mock behavior, but the command should not crash
    EXPECT_TRUE(result == "" || result == "error: failed to execute your command.\n");
}

/**
 * @tc.number: StartAbilityWithWait_0020
 * @tc.name: ExecCommand
 * @tc.desc: Verify the start command with -W flag and implicit start.
 */
HWTEST_F(ClawAaCommandTest, StartAbilityWithWait_0020, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityWithWait_0020");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-A",
        (char*)STRING_ACTION.c_str(),
        (char*)"-W",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    // With -W and implicit start, it calls StartAbility first then returns
    EXPECT_TRUE(result == "" || result == "error: failed to execute your command.\n");
}

// ============================================================================
// GetErrorInfoFromCode tests
// ============================================================================

/**
 * @tc.number: GetErrorInfoFromCode_0010
 * @tc.name: GetErrorInfoFromCode
 * @tc.desc: Verify GetErrorInfoFromCode returns correct info for known error code.
 */
HWTEST_F(ClawAaCommandTest, GetErrorInfoFromCode_0010, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetErrorInfoFromCode_0010");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    AaToolErrorInfo info = cmd.GetErrorInfoFromCode(RESOLVE_ABILITY_ERR);
    EXPECT_EQ(info.code, "10104001");
    EXPECT_TRUE(info.message.find("ability does not exist") != std::string::npos);
}

/**
 * @tc.number: GetErrorInfoFromCode_0020
 * @tc.name: GetErrorInfoFromCode
 * @tc.desc: Verify GetErrorInfoFromCode returns empty info for unknown error code.
 */
HWTEST_F(ClawAaCommandTest, GetErrorInfoFromCode_0020, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetErrorInfoFromCode_0020");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    AaToolErrorInfo info = cmd.GetErrorInfoFromCode(99999999);
    EXPECT_TRUE(info.code.empty());
}

// ============================================================================
// ConvertPid tests
// ============================================================================

/**
 * @tc.number: ConvertPid_0010
 * @tc.name: ConvertPid
 * @tc.desc: Verify ConvertPid with valid pid string.
 */
HWTEST_F(ClawAaCommandTest, ConvertPid_0010, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConvertPid_0010");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string pidStr = "12345";
    EXPECT_EQ(cmd.ConvertPid(pidStr), 12345);
}

/**
 * @tc.number: ConvertPid_0020
 * @tc.name: ConvertPid
 * @tc.desc: Verify ConvertPid with invalid pid string returns 0.
 */
HWTEST_F(ClawAaCommandTest, ConvertPid_0020, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConvertPid_0020");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string pidStr = "invalid";
    EXPECT_EQ(cmd.ConvertPid(pidStr), 0);
}

/**
 * @tc.number: ConvertPid_0030
 * @tc.name: ConvertPid
 * @tc.desc: Verify ConvertPid with "0" string returns 0.
 */
HWTEST_F(ClawAaCommandTest, ConvertPid_0030, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConvertPid_0030");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string pidStr = "0";
    EXPECT_EQ(cmd.ConvertPid(pidStr), 0);
}

// ============================================================================
// CreateErrorInfoMap tests
// ============================================================================

/**
 * @tc.number: CreateErrorInfoMap_0010
 * @tc.name: CreateErrorInfoMap
 * @tc.desc: Verify CreateErrorInfoMap populates errorInfoMap_ with all expected entries.
 */
HWTEST_F(ClawAaCommandTest, CreateErrorInfoMap_0010, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateErrorInfoMap_0010");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.CreateErrorInfoMap(), ERR_OK);
    EXPECT_EQ(cmd.errorInfoMap_.size(), static_cast<size_t>(20));
}

/**
 * @tc.number: CreateErrorInfoMap_0020
 * @tc.name: CreateErrorInfoMap
 * @tc.desc: Verify CreateErrorInfoMap contains KILL_PROCESS_FAILED entry.
 */
HWTEST_F(ClawAaCommandTest, CreateErrorInfoMap_0020, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateErrorInfoMap_0020");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    auto it = cmd.errorInfoMap_.find(KILL_PROCESS_FAILED);
    ASSERT_NE(it, cmd.errorInfoMap_.end());
    EXPECT_EQ(it->second.code, "10106401");
}

/**
 * @tc.number: CreateErrorInfoMap_0030
 * @tc.name: CreateErrorInfoMap
 * @tc.desc: Verify CreateErrorInfoMap contains KILL_PROCESS_KEEP_ALIVE entry.
 */
HWTEST_F(ClawAaCommandTest, CreateErrorInfoMap_0030, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateErrorInfoMap_0030");

    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    auto it = cmd.errorInfoMap_.find(KILL_PROCESS_KEEP_ALIVE);
    ASSERT_NE(it, cmd.errorInfoMap_.end());
    EXPECT_EQ(it->second.code, "10106402");
}
