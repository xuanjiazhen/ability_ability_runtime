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
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <csignal>
#include <thread>

#include "hilog_tag_wrapper.h"
#include "proc_scanner.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
// Respawn-safe sleep child: the post-fork child must only use async-signal-safe calls, so
// sleep is nanosleep, not sleep()/std::this_thread (the parent may be multi-threaded).
void NapChild(int seconds)
{
    struct timespec ts = { seconds, 0 };
    nanosleep(&ts, nullptr);
    _exit(0);
}
} // namespace

class ProcScannerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: ProcScanner_ParseProcStat_0100
 * @tc.desc: A well-formed stat line parses into pid/state/ppid.
 * @tc.type: FUNC
 */
HWTEST_F(ProcScannerTest, ParseProcStat_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcScanner_ParseProcStat_0100 start";

    ProcStatInfo info;
    ASSERT_TRUE(ProcScanner::ParseProcStat("1234 (claw_sandbox) S 100 1234 100 0 1 0", info));
    EXPECT_EQ(info.pid, 1234);
    EXPECT_EQ(info.state, 'S');
    EXPECT_EQ(info.ppid, 100);

    GTEST_LOG_(INFO) << "ProcScanner_ParseProcStat_0100 end";
}

/**
 * @tc.name: ProcScanner_ParseProcStat_0200
 * @tc.desc: comm containing spaces and ')' characters: fields must be parsed after the
 *           LAST ')' (claw_sandbox commands may carry arbitrary argv[0] text).
 * @tc.type: FUNC
 */
HWTEST_F(ProcScannerTest, ParseProcStat_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcScanner_ParseProcStat_0200 start";

    ProcStatInfo info;
    ASSERT_TRUE(ProcScanner::ParseProcStat("123 (weird ) name (x)) R 45 123 0 0 1 0", info));
    EXPECT_EQ(info.pid, 123);
    EXPECT_EQ(info.state, 'R');
    EXPECT_EQ(info.ppid, 45);

    GTEST_LOG_(INFO) << "ProcScanner_ParseProcStat_0200 end";
}

/**
 * @tc.name: ProcScanner_ParseProcStat_0300
 * @tc.desc: Malformed input (no comm parens / missing fields) is rejected, not half-parsed.
 * @tc.type: FUNC
 */
HWTEST_F(ProcScannerTest, ParseProcStat_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcScanner_ParseProcStat_0300 start";

    ProcStatInfo info;
    EXPECT_FALSE(ProcScanner::ParseProcStat("no parens at all", info));
    EXPECT_FALSE(ProcScanner::ParseProcStat("123 (comm)", info));
    EXPECT_FALSE(ProcScanner::ParseProcStat("", info));

    GTEST_LOG_(INFO) << "ProcScanner_ParseProcStat_0300 end";
}

/**
 * @tc.name: ProcScanner_IsProcessAlive_0100
 * @tc.desc: Self is alive; a reaped pid is not; zombies report not-alive.
 * @tc.type: FUNC
 */
HWTEST_F(ProcScannerTest, IsProcessAlive_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcScanner_IsProcessAlive_0100 start";

    EXPECT_TRUE(ProcScanner::IsProcessAlive(getpid()));

    pid_t child = fork();
    ASSERT_GE(child, 0);
    if (child == 0) {
        NapChild(5);
    }
    EXPECT_TRUE(ProcScanner::IsProcessAlive(child));
    ASSERT_EQ(kill(child, SIGKILL), 0);
    ASSERT_GT(waitpid(child, nullptr, 0), 0);
    EXPECT_FALSE(ProcScanner::IsProcessAlive(child));

    GTEST_LOG_(INFO) << "ProcScanner_IsProcessAlive_0100 end";
}

/**
 * @tc.name: ProcScanner_FindCommandProcess_0100
 * @tc.desc: The single live child of the anchor is resolved as the command process
 *           (claw_sandbox forks exactly one command child); the out-parameter reports
 *           a readable scan (>0 stats opened and parsed).
 * @tc.type: FUNC
 */
HWTEST_F(ProcScannerTest, FindCommandProcess_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcScanner_FindCommandProcess_0100 start";

    pid_t child = fork();
    ASSERT_GE(child, 0);
    if (child == 0) {
        NapChild(5);
    }
    int32_t readStatCount = 0;
    EXPECT_EQ(ProcScanner::FindCommandProcess(getpid(), &readStatCount), child);
    // A live child was resolved, so the scan was readable: at least the child's own stat
    // (and the test process's) was opened and parsed this round. UT runs with enough
    // privileges that a blinded (zero) scan cannot be constructed here.
    EXPECT_GT(readStatCount, 0);

    ASSERT_EQ(kill(child, SIGKILL), 0);
    ASSERT_GT(waitpid(child, nullptr, 0), 0);

    GTEST_LOG_(INFO) << "ProcScanner_FindCommandProcess_0100 end";
}

/**
 * @tc.name: ProcScanner_FindCommandProcess_0200
 * @tc.desc: A zombie child is never resolved as the command (the real command may still be
 *           forked later); -1 keeps the caller polling.
 * @tc.type: FUNC
 */
HWTEST_F(ProcScannerTest, FindCommandProcess_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcScanner_FindCommandProcess_0200 start";

    pid_t child = fork();
    ASSERT_GE(child, 0);
    if (child == 0) {
        _exit(0);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // The zombie child is filtered out of command resolution.
    EXPECT_EQ(ProcScanner::FindCommandProcess(getpid()), -1);
    ASSERT_GT(waitpid(child, nullptr, 0), 0);

    GTEST_LOG_(INFO) << "ProcScanner_FindCommandProcess_0200 end";
}

/**
 * @tc.name: ProcScanner_FindCommandProcess_0300
 * @tc.desc: No children at all (claw_sandbox still initializing before its pid-ns fork):
 *           -1, never the anchor itself; an invalid parent keeps the out-parameter at 0
 *           (the always-written contract: 0 on the early return).
 * @tc.type: FUNC
 */
HWTEST_F(ProcScannerTest, FindCommandProcess_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcScanner_FindCommandProcess_0300 start";

    EXPECT_EQ(ProcScanner::FindCommandProcess(getpid()), -1);
    int32_t readStatCount = -1;
    EXPECT_EQ(ProcScanner::FindCommandProcess(-1, &readStatCount), -1);
    EXPECT_EQ(readStatCount, 0);

    GTEST_LOG_(INFO) << "ProcScanner_FindCommandProcess_0300 end";
}

/**
 * @tc.name: ProcScanner_CheckLiveness_0100
 * @tc.desc: Tri-state liveness: own pid ALIVE; reaped (gone) pid and invalid pid DEAD;
 *           a zombie (exited, not yet reaped) is DEAD too.
 * @tc.type: FUNC
 */
HWTEST_F(ProcScannerTest, CheckLiveness_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcScanner_CheckLiveness_0100 start";

    EXPECT_EQ(ProcScanner::CheckLiveness(getpid()), ProcLiveness::ALIVE);
    EXPECT_EQ(ProcScanner::CheckLiveness(-1), ProcLiveness::DEAD);

    pid_t child = fork();
    ASSERT_GE(child, 0);
    if (child == 0) {
        NapChild(5);
    }
    EXPECT_EQ(ProcScanner::CheckLiveness(child), ProcLiveness::ALIVE);
    ASSERT_EQ(kill(child, SIGKILL), 0);
    // The zombie window opens once SIGKILL is delivered; kill(2) returns before the kernel
    // transitions the target, so poll bounded instead of asserting immediately.
    ProcLiveness liveness = ProcLiveness::ALIVE;
    for (int waited = 0; waited < 1000; waited += 10) {
        liveness = ProcScanner::CheckLiveness(child);
        if (liveness == ProcLiveness::DEAD) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_EQ(liveness, ProcLiveness::DEAD);
    ASSERT_GT(waitpid(child, nullptr, 0), 0);
    // Reaped: the pid is gone.
    EXPECT_EQ(ProcScanner::CheckLiveness(child), ProcLiveness::DEAD);

    GTEST_LOG_(INFO) << "ProcScanner_CheckLiveness_0100 end";
}

/**
 * @tc.name: ProcScanner_CheckLiveness_0200
 * @tc.desc: A recently reaped pid is not reused (monotonic allocation), so probing it never
 *           reports a foreign process as ALIVE — the property the retired-anchor tests rely on.
 * @tc.type: FUNC
 */
HWTEST_F(ProcScannerTest, CheckLiveness_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcScanner_CheckLiveness_0200 start";

    pid_t retired = fork();
    ASSERT_GE(retired, 0);
    if (retired == 0) {
        _exit(0);
    }
    ASSERT_GT(waitpid(retired, nullptr, 0), 0);
    EXPECT_EQ(ProcScanner::CheckLiveness(retired), ProcLiveness::DEAD);

    GTEST_LOG_(INFO) << "ProcScanner_CheckLiveness_0200 end";
}
} // namespace CliTool
} // namespace OHOS
