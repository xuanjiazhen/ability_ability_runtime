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
#include "ohos_aa_command.h"
#undef protected
#undef private
#include "mock_ability_manager_stub.h"
#define private public
#include "ability_manager_client.h"
#undef private
#include "ability_manager_interface.h"
#include "hilog_tag_wrapper.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using testing::_;
using testing::Return;

namespace {
const std::string TOOL_NAME = "ohos-aa";
}  // namespace

class OhosAaCommandForceStopTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects() const;

    std::string cmd_ = "force-stop";
};

void OhosAaCommandForceStopTest::SetUpTestCase()
{}

void OhosAaCommandForceStopTest::TearDownTestCase()
{}

void OhosAaCommandForceStopTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void OhosAaCommandForceStopTest::TearDown()
{}

void OhosAaCommandForceStopTest::MakeMockObjects() const
{
    // mock a stub
    auto managerStubPtr = sptr<IAbilityManager>(new MockAbilityManagerStub());

    // set the mock stub
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

/**
 * @tc.number: Ohos_Aa_Command_Force_Stop_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa force-stop" command with no option.
 */
HWTEST_F(OhosAaCommandForceStopTest, Ohos_Aa_Command_Force_Stop_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Force_Stop_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("error"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Force_Stop_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa force-stop --help" command.
 */
HWTEST_F(OhosAaCommandForceStopTest, Ohos_Aa_Command_Force_Stop_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Force_Stop_0200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--help",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    EXPECT_EQ(cmd.ExecCommand(), "ohos-aa force-stop - Stop an application on the system\n\n"
        "Usage:\n"
        "  ohos-aa force-stop [options]\n\n");
}

/**
 * @tc.number: Ohos_Aa_Command_Force_Stop_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa force-stop --bundlename <bundle>" command with success.
 */
HWTEST_F(OhosAaCommandForceStopTest, Ohos_Aa_Command_Force_Stop_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Force_Stop_0300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--bundlename",
        (char*)"com.example.test",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    auto managerClientPtr = AbilityManagerClient::GetInstance();
    auto mockAbilityManagerStub = sptr<MockAbilityManagerStub>(new MockAbilityManagerStub());
    ASSERT_NE(mockAbilityManagerStub, nullptr);
    EXPECT_CALL(*mockAbilityManagerStub, KillProcess(_, _, _, _))
        .Times(1)
        .WillOnce(Return(0));
    managerClientPtr->proxy_ = static_cast<IAbilityManager*>(mockAbilityManagerStub);

    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("force stop process successfully"), std::string::npos);
    managerClientPtr->proxy_ = nullptr;
    testing::Mock::AllowLeak(mockAbilityManagerStub);
}

/**
 * @tc.number: Ohos_Aa_Command_Force_Stop_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa force-stop --bundlename <bundle>" command with failure.
 */
HWTEST_F(OhosAaCommandForceStopTest, Ohos_Aa_Command_Force_Stop_0400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Force_Stop_0400");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--bundlename",
        (char*)"com.example.test",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    auto managerClientPtr = AbilityManagerClient::GetInstance();
    auto mockAbilityManagerStub = sptr<MockAbilityManagerStub>(new MockAbilityManagerStub());
    ASSERT_NE(mockAbilityManagerStub, nullptr);
    EXPECT_CALL(*mockAbilityManagerStub, KillProcess(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    managerClientPtr->proxy_ = static_cast<IAbilityManager*>(mockAbilityManagerStub);

    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("failed"), std::string::npos);
    managerClientPtr->proxy_ = nullptr;
    testing::Mock::AllowLeak(mockAbilityManagerStub);
}

/**
 * @tc.number: Ohos_Aa_Command_Force_Stop_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa force-stop xxx" command with wrong option.
 */
HWTEST_F(OhosAaCommandForceStopTest, Ohos_Aa_Command_Force_Stop_0500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Force_Stop_0500");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"xxx",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("error"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Force_Stop_0600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa force-stop --bundlename" command with no value.
 */
HWTEST_F(OhosAaCommandForceStopTest, Ohos_Aa_Command_Force_Stop_0600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Force_Stop_0600");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--bundlename",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    // With only --bundlename but no value, argList_ has size 1, not 2.
    EXPECT_NE(result.find("error"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Force_Stop_0700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa force-stop --bundlename <bundle> extra" command with too many parameters.
 */
HWTEST_F(OhosAaCommandForceStopTest, Ohos_Aa_Command_Force_Stop_0700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Force_Stop_0700");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--bundlename",
        (char*)"com.example.test",
        (char*)"extra",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("error"), std::string::npos);
}
