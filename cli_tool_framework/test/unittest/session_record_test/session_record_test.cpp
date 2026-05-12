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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "session_record.h"
#include "cli_session_info.h"

using namespace testing::ext;
using namespace OHOS::CliTool;

namespace OHOS {
namespace CliTool {

class SessionRecordTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<SessionRecord> record_;
};

void SessionRecordTest::SetUpTestCase(void)
{
}

void SessionRecordTest::TearDownTestCase(void)
{
}

void SessionRecordTest::SetUp()
{
    record_ = std::make_shared<SessionRecord>();
    record_->sessionId = "test_session";
    record_->toolName = "test_tool";
    record_->callerPid = 1001;
    record_->processId = -1;
    record_->startTime = 0;
    record_->timeoutMs = 30000;
}

void SessionRecordTest::TearDown()
{
    record_.reset();
}

HWTEST_F(SessionRecordTest, SetState_GetState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_SetState_GetState_0100 start";

    EXPECT_EQ(record_->GetState(), SessionState::SPAWNING);

    record_->SetState(SessionState::RUNNING);
    EXPECT_EQ(record_->GetState(), SessionState::RUNNING);

    GTEST_LOG_(INFO) << "SessionRecord_SetState_GetState_0100 end";
}

HWTEST_F(SessionRecordTest, SetState_GetState_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_SetState_GetState_0200 start";

    record_->SetState(SessionState::CANCELLING);
    EXPECT_EQ(record_->GetState(), SessionState::CANCELLING);

    record_->SetState(SessionState::COMPLETED);
    EXPECT_EQ(record_->GetState(), SessionState::COMPLETED);

    record_->SetState(SessionState::FAILED);
    EXPECT_EQ(record_->GetState(), SessionState::FAILED);

    GTEST_LOG_(INFO) << "SessionRecord_SetState_GetState_0200 end";
}

HWTEST_F(SessionRecordTest, SetTerminalResult_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_SetTerminalResult_0100 start";

    record_->SetTerminalResult(0, 0);

    EXPECT_TRUE(record_->HasProcessExited());
    EXPECT_EQ(record_->GetTerminalStatus(), 0);

    GTEST_LOG_(INFO) << "SessionRecord_SetTerminalResult_0100 end";
}

HWTEST_F(SessionRecordTest, SetTerminalResult_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_SetTerminalResult_0200 start";

    record_->SetTerminalResult(1, 0);

    EXPECT_TRUE(record_->HasProcessExited());
    EXPECT_EQ(record_->GetTerminalStatus(), 1);

    GTEST_LOG_(INFO) << "SessionRecord_SetTerminalResult_0200 end";
}

HWTEST_F(SessionRecordTest, SetTerminalResult_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_SetTerminalResult_0300 start - Test signal termination (SIGKILL=9)";

    record_->SetTerminalResult(0, 9);

    EXPECT_TRUE(record_->HasProcessExited());
    EXPECT_EQ(record_->GetTerminalStatus(), 0);

    GTEST_LOG_(INFO) << "SessionRecord_SetTerminalResult_0300 end";
}

HWTEST_F(SessionRecordTest, SetTerminalResult_Signal_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_SetTerminalResult_Signal_0100 start - Test SIGTERM (15) termination";

    record_->SetTerminalResult(0, 15);

    EXPECT_TRUE(record_->HasProcessExited());
    EXPECT_EQ(record_->GetTerminalStatus(), 0);

    CliSessionInfo session;
    record_->MarkStdoutClosed();
    record_->MarkStderrClosed();
    record_->BuildSessionInfo(session);

    EXPECT_NE(session.result, nullptr);
    EXPECT_EQ(session.result->signalNumber, 15);

    GTEST_LOG_(INFO) << "SessionRecord_SetTerminalResult_Signal_0100 end";
}

HWTEST_F(SessionRecordTest, SetTerminalResult_Signal_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_SetTerminalResult_Signal_0200 start - Test SIGABRT (6) termination";

    record_->SetTerminalResult(0, 6);

    EXPECT_TRUE(record_->HasProcessExited());
    EXPECT_EQ(record_->GetTerminalStatus(), 0);

    CliSessionInfo session;
    record_->MarkStdoutClosed();
    record_->MarkStderrClosed();
    record_->BuildSessionInfo(session);

    EXPECT_NE(session.result, nullptr);
    EXPECT_EQ(session.result->signalNumber, 6);

    GTEST_LOG_(INFO) << "SessionRecord_SetTerminalResult_Signal_0200 end";
}

HWTEST_F(SessionRecordTest, SetTerminalResult_Signal_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_SetTerminalResult_Signal_0300 start - Test exit code with no signal";

    record_->SetTerminalResult(127, 0);

    EXPECT_TRUE(record_->HasProcessExited());
    EXPECT_EQ(record_->GetTerminalStatus(), 127);

    CliSessionInfo session;
    record_->MarkStdoutClosed();
    record_->MarkStderrClosed();
    record_->BuildSessionInfo(session);

    EXPECT_NE(session.result, nullptr);
    EXPECT_EQ(session.result->exitCode, 127);
    EXPECT_EQ(session.result->signalNumber, 0);

    GTEST_LOG_(INFO) << "SessionRecord_SetTerminalResult_Signal_0300 end";
}

HWTEST_F(SessionRecordTest, SetSkillResult_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_SetSkillResult_0100 start";

    record_->sessionType = SessionType::SKILL;
    record_->SetSkillResult(0, "success output");

    EXPECT_TRUE(record_->HasProcessExited());
    EXPECT_TRUE(record_->OutputDrained());
    EXPECT_EQ(record_->GetTerminalStatus(), 0);

    GTEST_LOG_(INFO) << "SessionRecord_SetSkillResult_0100 end";
}

HWTEST_F(SessionRecordTest, SetSkillResult_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_SetSkillResult_0200 start";

    record_->sessionType = SessionType::SKILL;
    record_->SetSkillResult(1, "failed output");

    EXPECT_TRUE(record_->HasProcessExited());
    EXPECT_TRUE(record_->OutputDrained());
    EXPECT_EQ(record_->GetTerminalStatus(), 1);

    GTEST_LOG_(INFO) << "SessionRecord_SetSkillResult_0200 end";
}

HWTEST_F(SessionRecordTest, SetTimedOut_TimedOut_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_SetTimedOut_TimedOut_0100 start";

    EXPECT_FALSE(record_->TimedOut());

    record_->SetTimedOut(true);
    EXPECT_TRUE(record_->TimedOut());

    GTEST_LOG_(INFO) << "SessionRecord_SetTimedOut_TimedOut_0100 end";
}

HWTEST_F(SessionRecordTest, SetTimedOut_TimedOut_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_SetTimedOut_TimedOut_0200 start";

    record_->SetTimedOut(true);
    EXPECT_TRUE(record_->TimedOut());

    record_->SetTimedOut(false);
    EXPECT_FALSE(record_->TimedOut());

    GTEST_LOG_(INFO) << "SessionRecord_SetTimedOut_TimedOut_0200 end";
}

HWTEST_F(SessionRecordTest, GetEndTimeMs_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_GetEndTimeMs_0100 start";

    EXPECT_EQ(record_->GetEndTimeMs(), 0);

    record_->SetTerminalResult(0, 0);
    EXPECT_GT(record_->GetEndTimeMs(), 0);

    GTEST_LOG_(INFO) << "SessionRecord_GetEndTimeMs_0100 end";
}

HWTEST_F(SessionRecordTest, HasProcessExited_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_HasProcessExited_0100 start";

    EXPECT_FALSE(record_->HasProcessExited());

    record_->SetTerminalResult(0, 0);
    EXPECT_TRUE(record_->HasProcessExited());

    GTEST_LOG_(INFO) << "SessionRecord_HasProcessExited_0100 end";
}

HWTEST_F(SessionRecordTest, MarkStdoutClosed_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_MarkStdoutClosed_0100 start";

    EXPECT_FALSE(record_->OutputDrained());

    record_->MarkStdoutClosed();
    EXPECT_FALSE(record_->OutputDrained());

    GTEST_LOG_(INFO) << "SessionRecord_MarkStdoutClosed_0100 end";
}

HWTEST_F(SessionRecordTest, MarkStderrClosed_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_MarkStderrClosed_0100 start";

    EXPECT_FALSE(record_->OutputDrained());

    record_->MarkStderrClosed();
    EXPECT_FALSE(record_->OutputDrained());

    GTEST_LOG_(INFO) << "SessionRecord_MarkStderrClosed_0100 end";
}

HWTEST_F(SessionRecordTest, OutputDrained_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_OutputDrained_0100 start";

    EXPECT_FALSE(record_->OutputDrained());

    record_->MarkStdoutClosed();
    record_->MarkStderrClosed();
    EXPECT_TRUE(record_->OutputDrained());

    GTEST_LOG_(INFO) << "SessionRecord_OutputDrained_0100 end";
}

HWTEST_F(SessionRecordTest, OutputDrained_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_OutputDrained_0200 start";

    record_->MarkStdoutClosed();
    EXPECT_FALSE(record_->OutputDrained());

    record_->MarkStderrClosed();
    EXPECT_TRUE(record_->OutputDrained());

    GTEST_LOG_(INFO) << "SessionRecord_OutputDrained_0200 end";
}

HWTEST_F(SessionRecordTest, BeginCleanup_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_BeginCleanup_0100 start";

    EXPECT_TRUE(record_->BeginCleanup());
    EXPECT_FALSE(record_->BeginCleanup());

    GTEST_LOG_(INFO) << "SessionRecord_BeginCleanup_0100 end";
}

HWTEST_F(SessionRecordTest, BeginCleanup_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_BeginCleanup_0200 start";

    EXPECT_TRUE(record_->BeginCleanup());
    EXPECT_FALSE(record_->BeginCleanup());
    EXPECT_FALSE(record_->BeginCleanup());

    GTEST_LOG_(INFO) << "SessionRecord_BeginCleanup_0200 end";
}

HWTEST_F(SessionRecordTest, AppendOutput_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_AppendOutput_0100 start";

    record_->AppendOutput(true, "stdout_data_1");
    record_->AppendOutput(true, "stdout_data_2");

    CliSessionInfo session;
    record_->BuildSessionInfo(session);

    GTEST_LOG_(INFO) << "SessionRecord_AppendOutput_0100 end";
}

HWTEST_F(SessionRecordTest, AppendOutput_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_AppendOutput_0200 start";

    record_->AppendOutput(false, "stderr_data_1");
    record_->AppendOutput(false, "stderr_data_2");

    CliSessionInfo session;
    record_->BuildSessionInfo(session);

    GTEST_LOG_(INFO) << "SessionRecord_AppendOutput_0200 end";
}

HWTEST_F(SessionRecordTest, AppendOutput_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_AppendOutput_0300 start";

    record_->AppendOutput(true, "stdout");
    record_->AppendOutput(false, "stderr");

    CliSessionInfo session;
    record_->BuildSessionInfo(session);

    GTEST_LOG_(INFO) << "SessionRecord_AppendOutput_0300 end";
}

HWTEST_F(SessionRecordTest, AppendOutput_LargeData_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_AppendOutput_LargeData_0100 start - Test large data truncation";

    std::string largeData(100 * 1024, 'x');
    record_->AppendOutput(true, largeData);
    record_->AppendOutput(true, largeData);

    CliSessionInfo session;
    record_->BuildSessionInfo(session);

    EXPECT_NE(session.result, nullptr);
    EXPECT_LE(session.result->outputText.size(), 64 * 1024);

    GTEST_LOG_(INFO) << "SessionRecord_AppendOutput_LargeData_0100 end";
}

HWTEST_F(SessionRecordTest, AppendOutput_Truncation_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_AppendOutput_Truncation_0100 start - Test truncation at exact limit";

    std::string exactLimitData(64 * 1024, 'a');
    record_->AppendOutput(true, exactLimitData);

    CliSessionInfo session;
    record_->BuildSessionInfo(session);

    EXPECT_NE(session.result, nullptr);
    EXPECT_LE(session.result->outputText.size(), 64 * 1024);

    GTEST_LOG_(INFO) << "SessionRecord_AppendOutput_Truncation_0100 end";
}

HWTEST_F(SessionRecordTest, AppendOutput_Truncation_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_AppendOutput_Truncation_0200 start - Test truncation preserves prefix";

    std::string prefixData = "PREFIX_DATA_";
    std::string suffixData(100 * 1024, 'x');
    record_->AppendOutput(true, prefixData);
    record_->AppendOutput(true, suffixData);

    CliSessionInfo session;
    record_->BuildSessionInfo(session);

    EXPECT_NE(session.result, nullptr);
    EXPECT_LE(session.result->outputText.size(), 64 * 1024);
    EXPECT_TRUE(session.result->outputText.find("PREFIX_DATA_") != std::string::npos);

    GTEST_LOG_(INFO) << "SessionRecord_AppendOutput_Truncation_0200 end";
}

HWTEST_F(SessionRecordTest, SetBackground_Background_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_SetBackground_Background_0100 start";

    EXPECT_TRUE(record_->Background());

    bool oldBackground = record_->SetBackground(false);
    EXPECT_TRUE(oldBackground);
    EXPECT_FALSE(record_->Background());

    GTEST_LOG_(INFO) << "SessionRecord_SetBackground_Background_0100 end";
}

HWTEST_F(SessionRecordTest, SetBackground_Background_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_SetBackground_Background_0200 start";

    record_->SetBackground(false);
    EXPECT_FALSE(record_->Background());

    bool oldBackground = record_->SetBackground(true);
    EXPECT_FALSE(oldBackground);
    EXPECT_TRUE(record_->Background());

    GTEST_LOG_(INFO) << "SessionRecord_SetBackground_Background_0200 end";
}

HWTEST_F(SessionRecordTest, BuildSessionInfo_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_BuildSessionInfo_0100 start";

    record_->SetState(SessionState::RUNNING);

    CliSessionInfo session;
    record_->BuildSessionInfo(session);

    EXPECT_EQ(session.sessionId, "test_session");
    EXPECT_EQ(session.toolName, "test_tool");
    EXPECT_EQ(session.status, "running");

    GTEST_LOG_(INFO) << "SessionRecord_BuildSessionInfo_0100 end";
}

HWTEST_F(SessionRecordTest, BuildSessionInfo_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_BuildSessionInfo_0200 start";

    record_->SetTerminalResult(0, 0);
    record_->MarkStdoutClosed();
    record_->MarkStderrClosed();

    CliSessionInfo session;
    record_->BuildSessionInfo(session);

    EXPECT_EQ(session.sessionId, "test_session");
    EXPECT_EQ(session.toolName, "test_tool");
    EXPECT_EQ(session.status, "completed");
    EXPECT_NE(session.result, nullptr);
    EXPECT_EQ(session.result->exitCode, 0);

    GTEST_LOG_(INFO) << "SessionRecord_BuildSessionInfo_0200 end";
}

HWTEST_F(SessionRecordTest, BuildSessionInfo_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_BuildSessionInfo_0300 start";

    record_->SetTerminalResult(1, 0);
    record_->MarkStdoutClosed();
    record_->MarkStderrClosed();

    CliSessionInfo session;
    record_->BuildSessionInfo(session);

    EXPECT_EQ(session.sessionId, "test_session");
    EXPECT_EQ(session.toolName, "test_tool");
    EXPECT_EQ(session.status, "failed");
    EXPECT_NE(session.result, nullptr);
    EXPECT_EQ(session.result->exitCode, 1);

    GTEST_LOG_(INFO) << "SessionRecord_BuildSessionInfo_0300 end";
}

HWTEST_F(SessionRecordTest, BuildSessionInfo_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_BuildSessionInfo_0400 start";

    record_->SetTimedOut(true);
    record_->SetTerminalResult(0, 0);
    record_->MarkStdoutClosed();
    record_->MarkStderrClosed();

    CliSessionInfo session;
    record_->BuildSessionInfo(session);

    EXPECT_EQ(session.status, "failed");
    EXPECT_NE(session.result, nullptr);
    EXPECT_TRUE(session.result->timedOut);

    GTEST_LOG_(INFO) << "SessionRecord_BuildSessionInfo_0400 end";
}

HWTEST_F(SessionRecordTest, BuildSessionInfo_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_BuildSessionInfo_0500 start";

    record_->SetState(SessionState::SPAWNING);

    CliSessionInfo session;
    record_->BuildSessionInfo(session);

    EXPECT_EQ(session.status, "running");

    GTEST_LOG_(INFO) << "SessionRecord_BuildSessionInfo_0500 end";
}

HWTEST_F(SessionRecordTest, GetCallerPid_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_GetCallerPid_0100 start";

    EXPECT_EQ(record_->GetCallerPid(), 1001);

    GTEST_LOG_(INFO) << "SessionRecord_GetCallerPid_0100 end";
}

HWTEST_F(SessionRecordTest, SessionType_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_SessionType_0100 start";

    EXPECT_EQ(record_->sessionType, SessionType::CLI);

    record_->sessionType = SessionType::SKILL;
    EXPECT_EQ(record_->sessionType, SessionType::SKILL);

    GTEST_LOG_(INFO) << "SessionRecord_SessionType_0100 end";
}

HWTEST_F(SessionRecordTest, SessionState_Enum_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_SessionState_Enum_0100 start";

    EXPECT_EQ(static_cast<int>(SessionState::SPAWNING), 0);
    EXPECT_EQ(static_cast<int>(SessionState::RUNNING), 1);
    EXPECT_EQ(static_cast<int>(SessionState::CANCELLING), 2);
    EXPECT_EQ(static_cast<int>(SessionState::COMPLETED), 3);
    EXPECT_EQ(static_cast<int>(SessionState::FAILED), 4);

    GTEST_LOG_(INFO) << "SessionRecord_SessionState_Enum_0100 end";
}

HWTEST_F(SessionRecordTest, SessionType_Enum_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_SessionType_Enum_0100 start";

    EXPECT_EQ(static_cast<int>(SessionType::CLI), 0);
    EXPECT_EQ(static_cast<int>(SessionType::SKILL), 1);

    GTEST_LOG_(INFO) << "SessionRecord_SessionType_Enum_0100 end";
}

HWTEST_F(SessionRecordTest, DefaultValues_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_DefaultValues_0100 start";

    SessionRecord newRecord;

    EXPECT_EQ(newRecord.callerPid, -1);
    EXPECT_EQ(newRecord.sessionId, "");
    EXPECT_EQ(newRecord.toolName, "");
    EXPECT_EQ(newRecord.processId, -1);
    EXPECT_EQ(newRecord.startTime, 0);
    EXPECT_EQ(newRecord.timeoutMs, 0);
    EXPECT_EQ(newRecord.stdinPipe[0], -1);
    EXPECT_EQ(newRecord.stdinPipe[1], -1);
    EXPECT_EQ(newRecord.stdoutPipe[0], -1);
    EXPECT_EQ(newRecord.stdoutPipe[1], -1);
    EXPECT_EQ(newRecord.stderrPipe[0], -1);
    EXPECT_EQ(newRecord.stderrPipe[1], -1);
    EXPECT_EQ(newRecord.sessionType, SessionType::CLI);
    EXPECT_EQ(newRecord.GetState(), SessionState::SPAWNING);

    GTEST_LOG_(INFO) << "SessionRecord_DefaultValues_0100 end";
}

HWTEST_F(SessionRecordTest, MultipleRecords_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionRecord_MultipleRecords_0100 start";

    auto record1 = std::make_shared<SessionRecord>();
    auto record2 = std::make_shared<SessionRecord>();

    record1->sessionId = "session_1";
    record2->sessionId = "session_2";

    record1->SetState(SessionState::RUNNING);
    record2->SetState(SessionState::COMPLETED);

    EXPECT_EQ(record1->GetState(), SessionState::RUNNING);
    EXPECT_EQ(record2->GetState(), SessionState::COMPLETED);

    GTEST_LOG_(INFO) << "SessionRecord_MultipleRecords_0100 end";
}

} // namespace CliTool
} // namespace OHOS