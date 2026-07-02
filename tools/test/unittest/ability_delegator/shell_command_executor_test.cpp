/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#define private public
#include "shell_command_executor.h"
#include "shell_command_config_loader.h"
#undef private

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace {
const int64_t TIMEOUT = 1;
const std::string CMD = "ls";
}  // namespace

class ShellCommandExecutorTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<ShellCommandExecutor> Command_;
};

void ShellCommandExecutorTest::SetUpTestCase()
{
    ShellCommandConfigLoader::configState_ = false;
    ShellCommandConfigLoader::commands_ = {
        "aa", "bm", "cp", "mkdir", "rm", "uinput", "hilog", "ppwd",
        "echo", "uitest", "acm", "hidumper", "wukong", "pkill", "ps",
        "pidof", "perftest", "ime"
    };
    ShellCommandConfigLoader::configState_ = true;
}

void ShellCommandExecutorTest::TearDownTestCase()
{}

void ShellCommandExecutorTest::SetUp()
{
    Command_ = std::make_shared<ShellCommandExecutor>(CMD, TIMEOUT);
}

void ShellCommandExecutorTest::TearDown()
{}

/**
 * @tc.number: ShellCommandExecutor_DoWork_0100
 * @tc.name: DoWork
 * @tc.desc: Verify cmd_ When it is null, calling DoWork function returns false.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_DoWork_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0100 start";
    Command_->cmd_ = "";
    EXPECT_EQ(Command_->DoWork(), false);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0100 end";
}

/**
 * @tc.number: ShellCommandExecutor_DoWork_0200
 * @tc.name: DoWork
 * @tc.desc: Verify the handler_ When it is nullptr, calling DoWork function returns false.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_DoWork_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0200 start";
    Command_->handler_ = nullptr;
    EXPECT_EQ(Command_->DoWork(), false);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0200 end";
}

/**
 * @tc.number: ShellCommandExecutor_DoWork_0300
 * @tc.name: DoWork
 * @tc.desc: Verify that PostTask conditions are met and cmd_ Not empty, call DoWork function branch to overwrite.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_DoWork_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0300 start";
    std::shared_ptr<AppExecFwk::EventHandler> handler_ = std::make_shared<AppExecFwk::EventHandler>();
    ASSERT_NE(handler_, nullptr);
    auto task = []() { GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0300 task called"; };
    handler_->PostTask(task, 1000);
    Command_->cmd_ = CMD;
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0300 end";
}

/**
 * @tc.number: ShellCommandExecutor_DoWork_0400
 * @tc.name: DoWork
 * @tc.desc: Verify that PostTask conditions are met and cmd_ Exception, calling DoWork function branch
 *           failed to reach coverage.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_DoWork_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0400 start";
    std::shared_ptr<AppExecFwk::EventHandler> handler_ = std::make_shared<AppExecFwk::EventHandler>();
    ASSERT_NE(handler_, nullptr);
    auto task = []() { GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0400 task called"; };
    handler_->PostTask(task, 1000);
    Command_->cmd_ = "CMD12";
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0400 end";
}

/**
 * @tc.number: ShellCommandExecutor_WaitWorkDone_0100
 * @tc.name: WaitWorkDone
 * @tc.desc: Verify that timeoutSec is met_ Condition, call WaitWorkDone function branch to reach coverage.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_WaitWorkDone_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_WaitWorkDone_0100 start";
    ASSERT_NE(Command_, nullptr);
    Command_->timeoutSec_ = -1;
    Command_->cmdResult_.exitCode = 0;
    Command_->WaitWorkDone();
    Command_->DoWork();
    GTEST_LOG_(INFO) << "ShellCommandExecutor_WaitWorkDone_0100 end";
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_0100
 * @tc.name: CheckCommand single valid command
 * @tc.desc: Verify that a single valid command passes CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_0100, TestSize.Level1)
{
    Command_->cmd_ = "hilog";
    EXPECT_TRUE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_0200
 * @tc.name: CheckCommand single invalid command
 * @tc.desc: Verify that a single invalid command fails CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_0200, TestSize.Level1)
{
    Command_->cmd_ = "malicious_cmd";
    EXPECT_FALSE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_0300
 * @tc.name: CheckCommand pipe with both commands valid
 * @tc.desc: Verify that piped commands with both valid pass CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_0300, TestSize.Level1)
{
    Command_->cmd_ = "hilog -b D | hilog -b D";
    EXPECT_TRUE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_0400
 * @tc.name: CheckCommand pipe with invalid second command
 * @tc.desc: Verify that piped commands with invalid second fail CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_0400, TestSize.Level1)
{
    Command_->cmd_ = "hilog -b D | malicious_cmd";
    EXPECT_FALSE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_0500
 * @tc.name: CheckCommand pipe with invalid first command
 * @tc.desc: Verify that piped commands with invalid first fail CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_0500, TestSize.Level1)
{
    Command_->cmd_ = "malicious_cmd | hilog";
    EXPECT_FALSE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_0600
 * @tc.name: CheckCommand semicolon separator with valid commands
 * @tc.desc: Verify that semicolon-separated commands with both valid pass CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_0600, TestSize.Level1)
{
    Command_->cmd_ = "hilog -b D; ps";
    EXPECT_TRUE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_0700
 * @tc.name: CheckCommand semicolon separator with invalid second command
 * @tc.desc: Verify that semicolon-separated commands with invalid second fail CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_0700, TestSize.Level1)
{
    Command_->cmd_ = "hilog; malicious_cmd";
    EXPECT_FALSE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_0800
 * @tc.name: CheckCommand && operator with valid commands
 * @tc.desc: Verify that &&-separated commands with both valid pass CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_0800, TestSize.Level1)
{
    Command_->cmd_ = "hilog && ps";
    EXPECT_TRUE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_0900
 * @tc.name: CheckCommand && operator with invalid second command
 * @tc.desc: Verify that &&-separated commands with invalid second fail CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_0900, TestSize.Level1)
{
    Command_->cmd_ = "hilog && malicious_cmd";
    EXPECT_FALSE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_1000
 * @tc.name: CheckCommand || operator with valid commands
 * @tc.desc: Verify that ||-separated commands with both valid pass CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_1000, TestSize.Level1)
{
    Command_->cmd_ = "hilog || ps";
    EXPECT_TRUE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_1100
 * @tc.name: CheckCommand || operator with invalid second command
 * @tc.desc: Verify that ||-separated commands with invalid second fail CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_1100, TestSize.Level1)
{
    Command_->cmd_ = "hilog || malicious_cmd";
    EXPECT_FALSE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_1200
 * @tc.name: CheckCommand & background operator with valid commands
 * @tc.desc: Verify that &-separated commands with both valid pass CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_1200, TestSize.Level1)
{
    Command_->cmd_ = "hilog & ps";
    EXPECT_TRUE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_1300
 * @tc.name: CheckCommand & background operator with invalid second command
 * @tc.desc: Verify that &-separated commands with invalid second fail CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_1300, TestSize.Level1)
{
    Command_->cmd_ = "hilog & malicious_cmd";
    EXPECT_FALSE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_1400
 * @tc.name: CheckCommand command substitution $()
 * @tc.desc: Verify that command with $() substitution fails CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_1400, TestSize.Level1)
{
    Command_->cmd_ = "hilog $(malicious_cmd)";
    EXPECT_FALSE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_1500
 * @tc.name: CheckCommand backtick command substitution
 * @tc.desc: Verify that command with backtick substitution fails CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_1500, TestSize.Level1)
{
    Command_->cmd_ = "hilog `malicious_cmd`";
    EXPECT_FALSE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_1600
 * @tc.name: CheckCommand newline injection with invalid second command
 * @tc.desc: Verify that newline-separated commands with invalid second fail CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_1600, TestSize.Level1)
{
    Command_->cmd_ = "hilog\nmalicious_cmd";
    EXPECT_FALSE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_1700
 * @tc.name: CheckCommand newline separator with valid commands
 * @tc.desc: Verify that newline-separated commands with both valid pass CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_1700, TestSize.Level1)
{
    Command_->cmd_ = "hilog\nps";
    EXPECT_TRUE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_1800
 * @tc.name: CheckCommand newline separator with arguments
 * @tc.desc: Verify that newline-separated commands with arguments and both valid pass CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_1800, TestSize.Level1)
{
    Command_->cmd_ = "hilog -b D\nps";
    EXPECT_TRUE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_1900
 * @tc.name: CheckCommand multiple pipes all valid
 * @tc.desc: Verify that multiple piped commands all valid pass CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_1900, TestSize.Level1)
{
    Command_->cmd_ = "ps | hilog | echo test";
    EXPECT_TRUE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_2000
 * @tc.name: CheckCommand multiple pipes one invalid
 * @tc.desc: Verify that multiple piped commands with one invalid fail CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_2000, TestSize.Level1)
{
    Command_->cmd_ = "ps | hilog | malicious_cmd";
    EXPECT_FALSE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_2100
 * @tc.name: CheckCommand mixed operators all valid
 * @tc.desc: Verify that mixed operator commands all valid pass CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_2100, TestSize.Level1)
{
    Command_->cmd_ = "hilog && ps; echo test | pidof test";
    EXPECT_TRUE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_2200
 * @tc.name: CheckCommand mixed operators one invalid
 * @tc.desc: Verify that mixed operator commands with one invalid fail CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_2200, TestSize.Level1)
{
    Command_->cmd_ = "hilog && ps; echo test | malicious_cmd";
    EXPECT_FALSE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_2300
 * @tc.name: CheckCommand single command with arguments
 * @tc.desc: Verify that a valid command with arguments passes CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_2300, TestSize.Level1)
{
    Command_->cmd_ = "hilog -b D -t test";
    EXPECT_TRUE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_2400
 * @tc.name: CheckCommand empty string
 * @tc.desc: Verify that an empty command string passes CheckCommand (no sub-commands to validate).
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_2400, TestSize.Level1)
{
    Command_->cmd_ = "";
    EXPECT_TRUE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_2500
 * @tc.name: CheckCommand $() in middle of command
 * @tc.desc: Verify that $() appearing in the middle of a command fails CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_2500, TestSize.Level1)
{
    Command_->cmd_ = "echo $(rm -rf /)";
    EXPECT_FALSE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_2600
 * @tc.name: CheckCommand || is not split as |
 * @tc.desc: Verify that || is treated as a single operator, not as two | operators.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_2600, TestSize.Level1)
{
    Command_->cmd_ = "hilog || ps";
    EXPECT_TRUE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_2700
 * @tc.name: CheckCommand && is not split as &
 * @tc.desc: Verify that && is treated as a single operator, not as & followed by &.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_2700, TestSize.Level1)
{
    Command_->cmd_ = "hilog && ps";
    EXPECT_TRUE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_2800
 * @tc.name: CheckCommand backtick in arguments
 * @tc.desc: Verify that backtick appearing in command arguments fails CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_2800, TestSize.Level1)
{
    Command_->cmd_ = "echo `cat /etc/passwd`";
    EXPECT_FALSE(Command_->CheckCommand());
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_2900
 * @tc.name: CheckCommand pipe with varying whitespace
 * @tc.desc: Verify that piped commands with varying whitespace are correctly parsed.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_2900, TestSize.Level1)
{
    Command_->cmd_ = "hilog   |   ps";
    EXPECT_TRUE(Command_->CheckCommand());
}