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

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mutex>
#include <thread>
#include <unistd.h>

#include "io_monitor.h"

using namespace testing::ext;
using namespace OHOS::CliTool;

namespace OHOS {
namespace CliTool {

class IOMonitorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<IOMonitor> monitor_;
    int testPipe_[2] = {-1, -1};
};

void IOMonitorTest::SetUpTestCase(void)
{
}

void IOMonitorTest::TearDownTestCase(void)
{
}

void IOMonitorTest::SetUp()
{
    monitor_ = IOMonitor::Create();
    if (pipe(testPipe_) == 0) {
    }
}

void IOMonitorTest::TearDown()
{
    if (monitor_) {
        monitor_->Stop();
        monitor_.reset();
    }
    if (testPipe_[0] >= 0) {
        close(testPipe_[0]);
        testPipe_[0] = -1;
    }
    if (testPipe_[1] >= 0) {
        close(testPipe_[1]);
        testPipe_[1] = -1;
    }
}

HWTEST_F(IOMonitorTest, Create_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_Create_0100 start";

    auto monitor = IOMonitor::Create();
    EXPECT_NE(monitor, nullptr);

    GTEST_LOG_(INFO) << "IOMonitor_Create_0100 end";
}

HWTEST_F(IOMonitorTest, Create_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_Create_0200 start";

    auto monitor1 = IOMonitor::Create();
    auto monitor2 = IOMonitor::Create();
    EXPECT_NE(monitor1, nullptr);
    EXPECT_NE(monitor2, nullptr);
    EXPECT_NE(monitor1, monitor2);

    monitor1->Stop();
    monitor2->Stop();

    GTEST_LOG_(INFO) << "IOMonitor_Create_0200 end";
}

HWTEST_F(IOMonitorTest, Start_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_Start_0100 start";

    EXPECT_TRUE(monitor_->Start());

    GTEST_LOG_(INFO) << "IOMonitor_Start_0100 end";
}

HWTEST_F(IOMonitorTest, Start_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_Start_0200 start";

    EXPECT_TRUE(monitor_->Start());
    EXPECT_TRUE(monitor_->Start());

    GTEST_LOG_(INFO) << "IOMonitor_Start_0200 end";
}

HWTEST_F(IOMonitorTest, Stop_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_Stop_0100 start";

    monitor_->Start();
    monitor_->Stop();

    GTEST_LOG_(INFO) << "IOMonitor_Stop_0100 end";
}

HWTEST_F(IOMonitorTest, Stop_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_Stop_0200 start";

    monitor_->Stop();
    monitor_->Stop();

    GTEST_LOG_(INFO) << "IOMonitor_Stop_0200 end";
}

HWTEST_F(IOMonitorTest, RegisterSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_RegisterSession_0100 start - Test stdout/stderr/stdin pipe registration";

    ASSERT_TRUE(monitor_->Start());

    int stdoutPipe[2] = {-1, -1};
    int stderrPipe[2] = {-1, -1};
    int stdinPipe[2] = {-1, -1};

    ASSERT_EQ(pipe(stdoutPipe), 0);
    ASSERT_EQ(pipe(stderrPipe), 0);
    ASSERT_EQ(pipe(stdinPipe), 0);

    EXPECT_TRUE(monitor_->RegisterSession("test_session", stdoutPipe[0], stderrPipe[0], stdinPipe[1]));

    monitor_->UnregisterSession("test_session");
    close(stdoutPipe[1]);
    close(stderrPipe[1]);
    close(stdinPipe[0]);

    GTEST_LOG_(INFO) << "IOMonitor_RegisterSession_0100 end";
}

HWTEST_F(IOMonitorTest, RegisterSession_Stderr_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_RegisterSession_Stderr_0100 start - Test stderr data reception";

    ASSERT_TRUE(monitor_->Start());

    int stdoutPipe[2] = {-1, -1};
    int stderrPipe[2] = {-1, -1};
    ASSERT_EQ(pipe(stdoutPipe), 0);
    ASSERT_EQ(pipe(stderrPipe), 0);

    std::string receivedData;
    bool isStdoutFlag = true;
    std::mutex dataMutex;
    std::condition_variable dataCv;
    monitor_->SetOutputCallback([&](const std::string &sessionId, bool isStdout, const std::string &data) {
        std::lock_guard<std::mutex> lock(dataMutex);
        if (!isStdout) {
            receivedData = data;
            isStdoutFlag = isStdout;
            dataCv.notify_one();
        }
    });

    EXPECT_TRUE(monitor_->RegisterSession("stderr_test", stdoutPipe[0], stderrPipe[0], -1));

    const std::string testError = "test_stderr_data";
    write(stderrPipe[1], testError.c_str(), testError.size());
    close(stderrPipe[1]);
    close(stdoutPipe[1]);

    std::unique_lock<std::mutex> lock(dataMutex);
    EXPECT_TRUE(dataCv.wait_for(lock, std::chrono::seconds(2), [&]() {
        return !receivedData.empty();
    }));
    EXPECT_FALSE(isStdoutFlag);
    EXPECT_EQ(receivedData, testError);

    monitor_->UnregisterSession("stderr_test");

    GTEST_LOG_(INFO) << "IOMonitor_RegisterSession_Stderr_0100 end";
}

HWTEST_F(IOMonitorTest, RegisterSession_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_RegisterSession_0200 start";

    ASSERT_TRUE(monitor_->Start());

    int stdoutPipe[2] = {-1, -1};
    ASSERT_EQ(pipe(stdoutPipe), 0);

    EXPECT_TRUE(monitor_->RegisterSession("test_session_stdout_only", stdoutPipe[0], -1, -1));

    monitor_->UnregisterSession("test_session_stdout_only");
    close(stdoutPipe[1]);

    GTEST_LOG_(INFO) << "IOMonitor_RegisterSession_0200 end";
}

HWTEST_F(IOMonitorTest, RegisterSession_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_RegisterSession_0300 start";

    ASSERT_TRUE(monitor_->Start());

    EXPECT_TRUE(monitor_->RegisterSession("test_session_no_fds", -1, -1, -1));

    monitor_->UnregisterSession("test_session_no_fds");

    GTEST_LOG_(INFO) << "IOMonitor_RegisterSession_0300 end";
}

HWTEST_F(IOMonitorTest, UnregisterSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_UnregisterSession_0100 start";

    ASSERT_TRUE(monitor_->Start());

    int stdoutPipe[2] = {-1, -1};
    ASSERT_EQ(pipe(stdoutPipe), 0);

    EXPECT_TRUE(monitor_->RegisterSession("test_unregister", stdoutPipe[0], -1, -1));
    monitor_->UnregisterSession("test_unregister");

    close(stdoutPipe[1]);

    GTEST_LOG_(INFO) << "IOMonitor_UnregisterSession_0100 end";
}

HWTEST_F(IOMonitorTest, UnregisterSession_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_UnregisterSession_0200 start";

    monitor_->UnregisterSession("nonexistent_session");

    GTEST_LOG_(INFO) << "IOMonitor_UnregisterSession_0200 end";
}

HWTEST_F(IOMonitorTest, SetOutputCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_SetOutputCallback_0100 start";

    bool callbackCalled = false;
    monitor_->SetOutputCallback([&](const std::string &, bool, const std::string &) {
        callbackCalled = true;
    });

    GTEST_LOG_(INFO) << "IOMonitor_SetOutputCallback_0100 end";
}

HWTEST_F(IOMonitorTest, SetInputReplyCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_SetInputReplyCallback_0100 start";

    bool callbackCalled = false;
    monitor_->SetInputReplyCallback([&](const std::string &, const std::string &, bool) {
        callbackCalled = true;
    });

    GTEST_LOG_(INFO) << "IOMonitor_SetInputReplyCallback_0100 end";
}

HWTEST_F(IOMonitorTest, SetSessionClosedCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_SetSessionClosedCallback_0100 start";

    bool callbackCalled = false;
    monitor_->SetSessionClosedCallback([&](const std::string &, bool) {
        callbackCalled = true;
    });

    GTEST_LOG_(INFO) << "IOMonitor_SetSessionClosedCallback_0100 end";
}

HWTEST_F(IOMonitorTest, SetSessionDrainedCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_SetSessionDrainedCallback_0100 start";

    bool callbackCalled = false;
    monitor_->SetSessionDrainedCallback([&](const std::string &) {
        callbackCalled = true;
    });

    GTEST_LOG_(INFO) << "IOMonitor_SetSessionDrainedCallback_0100 end";
}

HWTEST_F(IOMonitorTest, SendMessage_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_SendMessage_0100 start";

    ASSERT_TRUE(monitor_->Start());

    int stdinPipe[2] = {-1, -1};
    ASSERT_EQ(pipe(stdinPipe), 0);

    EXPECT_TRUE(monitor_->RegisterSession("send_test", -1, -1, stdinPipe[1]));

    std::atomic<int32_t> replyCount = 0;
    std::mutex replyMutex;
    std::condition_variable replyCv;
    monitor_->SetInputReplyCallback([&](const std::string &, const std::string &, bool) {
        if (replyCount.fetch_add(1) + 1 == 1) {
            std::lock_guard<std::mutex> lock(replyMutex);
            replyCv.notify_one();
        }
    });

    std::atomic<size_t> readBytes = 0;
    std::thread reader([&]() {
        char buffer[256] = {};
        while (readBytes.load() < 10) {
            ssize_t readResult = read(stdinPipe[0], buffer, sizeof(buffer));
            if (readResult > 0) {
                readBytes.fetch_add(static_cast<size_t>(readResult));
            } else {
                break;
            }
        }
    });

    monitor_->SendMessage("send_test", "test_message", "event_1");

    std::unique_lock<std::mutex> lock(replyMutex);
    EXPECT_TRUE(replyCv.wait_for(lock, std::chrono::seconds(2), [&]() {
        return replyCount.load() >= 1;
    }));

    monitor_->UnregisterSession("send_test");
    if (reader.joinable()) {
        reader.join();
    }
    close(stdinPipe[0]);

    GTEST_LOG_(INFO) << "IOMonitor_SendMessage_0100 end";
}

HWTEST_F(IOMonitorTest, SendMessage_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_SendMessage_0200 start - Test send to nonexistent session";

    std::atomic<bool> callbackCalled = false;
    std::atomic<bool> resultValue = true;
    std::mutex callbackMutex;
    std::condition_variable callbackCv;

    monitor_->SetInputReplyCallback([&](const std::string &, const std::string &, bool result) {
        resultValue.store(result);
        callbackCalled.store(true);
        std::lock_guard<std::mutex> lock(callbackMutex);
        callbackCv.notify_one();
    });

    monitor_->SendMessage("nonexistent_session", "test_message", "event_1");

    std::unique_lock<std::mutex> lock(callbackMutex);
    bool received = callbackCv.wait_for(lock, std::chrono::seconds(2), [&]() {
        return callbackCalled.load();
    });

    EXPECT_TRUE(received);
    EXPECT_TRUE(callbackCalled.load());
    EXPECT_FALSE(resultValue.load());

    GTEST_LOG_(INFO) << "IOMonitor_SendMessage_0200 end";
}

HWTEST_F(IOMonitorTest, SendMessage_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_SendMessage_0300 start - Test empty message handling";

    ASSERT_TRUE(monitor_->Start());

    int stdinPipe[2] = {-1, -1};
    ASSERT_EQ(pipe(stdinPipe), 0);

    EXPECT_TRUE(monitor_->RegisterSession("empty_test", -1, -1, stdinPipe[1]));

    std::atomic<int32_t> replyCount = 0;
    std::atomic<bool> lastResult = false;
    std::mutex replyMutex;
    std::condition_variable replyCv;
    monitor_->SetInputReplyCallback([&](const std::string &, const std::string &, bool result) {
        lastResult.store(result);
        if (replyCount.fetch_add(1) + 1 == 1) {
            std::lock_guard<std::mutex> lock(replyMutex);
            replyCv.notify_one();
        }
    });

    monitor_->SendMessage("empty_test", "", "event_empty");

    std::unique_lock<std::mutex> lock(replyMutex);
    bool received = replyCv.wait_for(lock, std::chrono::seconds(2), [&]() {
        return replyCount.load() >= 1;
    });

    EXPECT_TRUE(received);
    EXPECT_GE(replyCount.load(), 1);
    EXPECT_TRUE(lastResult.load());

    monitor_->UnregisterSession("empty_test");
    close(stdinPipe[0]);

    GTEST_LOG_(INFO) << "IOMonitor_SendMessage_0300 end";
}

HWTEST_F(IOMonitorTest, HandleReadableFd_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_HandleReadableFd_0100 start";

    ASSERT_TRUE(monitor_->Start());

    int stdoutPipe[2] = {-1, -1};
    ASSERT_EQ(pipe(stdoutPipe), 0);

    std::string receivedData;
    bool isStdoutFlag = false;
    std::mutex dataMutex;
    std::condition_variable dataCv;
    monitor_->SetOutputCallback([&](const std::string &sessionId, bool isStdout, const std::string &data) {
        std::lock_guard<std::mutex> lock(dataMutex);
        receivedData = data;
        isStdoutFlag = isStdout;
        dataCv.notify_one();
    });

    EXPECT_TRUE(monitor_->RegisterSession("handle_read_test", stdoutPipe[0], -1, -1));

    const std::string testData = "test_output_data";
    write(stdoutPipe[1], testData.c_str(), testData.size());
    close(stdoutPipe[1]);

    std::unique_lock<std::mutex> lock(dataMutex);
    EXPECT_TRUE(dataCv.wait_for(lock, std::chrono::seconds(2), [&]() {
        return receivedData == testData;
    }));
    EXPECT_TRUE(isStdoutFlag);

    monitor_->UnregisterSession("handle_read_test");

    GTEST_LOG_(INFO) << "IOMonitor_HandleReadableFd_0100 end";
}

HWTEST_F(IOMonitorTest, CloseFdLocked_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_CloseFdLocked_0100 start";

    ASSERT_TRUE(monitor_->Start());

    int stdoutPipe[2] = {-1, -1};
    int stderrPipe[2] = {-1, -1};
    ASSERT_EQ(pipe(stdoutPipe), 0);
    ASSERT_EQ(pipe(stderrPipe), 0);

    std::string closedSessionId;
    bool closedIsStdout = false;
    std::string drainedSessionId;
    std::mutex closeMutex;
    std::condition_variable closeCv;
    monitor_->SetSessionClosedCallback([&](const std::string &sessionId, bool isStdout) {
        std::lock_guard<std::mutex> lock(closeMutex);
        closedSessionId = sessionId;
        closedIsStdout = isStdout;
    });
    monitor_->SetSessionDrainedCallback([&](const std::string &sessionId) {
        std::lock_guard<std::mutex> lock(closeMutex);
        drainedSessionId = sessionId;
        closeCv.notify_one();
    });

    EXPECT_TRUE(monitor_->RegisterSession("close_test", stdoutPipe[0], stderrPipe[0], -1));

    close(stdoutPipe[1]);
    close(stderrPipe[1]);

    std::unique_lock<std::mutex> lock(closeMutex);
    EXPECT_TRUE(closeCv.wait_for(lock, std::chrono::seconds(3), [&]() {
        return !drainedSessionId.empty();
    }));
    EXPECT_EQ(drainedSessionId, "close_test");

    monitor_->UnregisterSession("close_test");

    GTEST_LOG_(INFO) << "IOMonitor_CloseFdLocked_0100 end";
}

HWTEST_F(IOMonitorTest, MonitorLoop_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_MonitorLoop_0100 start - Test data reception and content validation";

    ASSERT_TRUE(monitor_->Start());

    int stdoutPipe[2] = {-1, -1};
    ASSERT_EQ(pipe(stdoutPipe), 0);

    std::atomic<int32_t> eventCount = 0;
    std::string receivedData;
    std::mutex dataMutex;
    std::condition_variable dataCv;
    monitor_->SetOutputCallback([&](const std::string &sessionId, bool isStdout, const std::string &data) {
        std::lock_guard<std::mutex> lock(dataMutex);
        receivedData.append(data);
        eventCount.fetch_add(1);
        if (receivedData.find("data1") != std::string::npos &&
            receivedData.find("data2") != std::string::npos) {
            dataCv.notify_one();
        }
    });

    EXPECT_TRUE(monitor_->RegisterSession("loop_test", stdoutPipe[0], -1, -1));

    write(stdoutPipe[1], "data1", 5);
    write(stdoutPipe[1], "data2", 5);
    close(stdoutPipe[1]);

    std::unique_lock<std::mutex> lock(dataMutex);
    bool dataReceived = dataCv.wait_for(lock, std::chrono::seconds(3), [&]() {
        return receivedData.find("data1") != std::string::npos &&
               receivedData.find("data2") != std::string::npos;
    });

    EXPECT_TRUE(dataReceived);
    EXPECT_GE(eventCount.load(), 1);
    EXPECT_TRUE(receivedData.find("data1") != std::string::npos);
    EXPECT_TRUE(receivedData.find("data2") != std::string::npos);

    monitor_->UnregisterSession("loop_test");

    GTEST_LOG_(INFO) << "IOMonitor_MonitorLoop_0100 end";
}

HWTEST_F(IOMonitorTest, ProcessWriteQueue_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_ProcessWriteQueue_0100 start";

    ASSERT_TRUE(monitor_->Start());

    int stdinPipe[2] = {-1, -1};
    ASSERT_EQ(pipe(stdinPipe), 0);

    EXPECT_TRUE(monitor_->RegisterSession("queue_test", -1, -1, stdinPipe[1]));

    constexpr int32_t sendCount = 5;
    std::atomic<int32_t> replyCount = 0;
    std::atomic<int32_t> successCount = 0;
    std::mutex replyMutex;
    std::condition_variable replyCv;
    monitor_->SetInputReplyCallback([&](const std::string &, const std::string &, bool result) {
        if (result) {
            successCount.fetch_add(1);
        }
        if (replyCount.fetch_add(1) + 1 == sendCount) {
            std::lock_guard<std::mutex> lock(replyMutex);
            replyCv.notify_one();
        }
    });

    std::atomic<size_t> readBytes = 0;
    std::thread reader([&]() {
        char buffer[256] = {};
        const size_t expectedBytes = sendCount * 10;
        while (readBytes.load() < expectedBytes) {
            ssize_t readResult = read(stdinPipe[0], buffer, sizeof(buffer));
            if (readResult > 0) {
                readBytes.fetch_add(static_cast<size_t>(readResult));
            } else {
                break;
            }
        }
    });

    for (int32_t i = 0; i < sendCount; ++i) {
        monitor_->SendMessage("queue_test", std::string(10, 'x'), "event_" + std::to_string(i));
    }

    std::unique_lock<std::mutex> lock(replyMutex);
    EXPECT_TRUE(replyCv.wait_for(lock, std::chrono::seconds(3), [&]() {
        return replyCount.load() == sendCount;
    }));
    EXPECT_EQ(successCount.load(), sendCount);

    monitor_->UnregisterSession("queue_test");
    if (reader.joinable()) {
        reader.join();
    }
    close(stdinPipe[0]);

    GTEST_LOG_(INFO) << "IOMonitor_ProcessWriteQueue_0100 end";
}

HWTEST_F(IOMonitorTest, Destructor_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_Destructor_0100 start";

    {
        auto monitor = IOMonitor::Create();
        EXPECT_NE(monitor, nullptr);
        EXPECT_TRUE(monitor->Start());
    }

    GTEST_LOG_(INFO) << "IOMonitor_Destructor_0100 end";
}

HWTEST_F(IOMonitorTest, MultipleSessions_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IOMonitor_MultipleSessions_0100 start";

    ASSERT_TRUE(monitor_->Start());

    int pipes[3][2] = {{-1, -1}, {-1, -1}, {-1, -1}};
    for (int i = 0; i < 3; ++i) {
        ASSERT_EQ(pipe(pipes[i]), 0);
    }

    EXPECT_TRUE(monitor_->RegisterSession("session1", pipes[0][0], -1, pipes[1][1]));
    EXPECT_TRUE(monitor_->RegisterSession("session2", pipes[1][0], -1, pipes[2][1]));

    monitor_->UnregisterSession("session1");
    monitor_->UnregisterSession("session2");

    for (int i = 0; i < 3; ++i) {
        close(pipes[i][1]);
    }

    GTEST_LOG_(INFO) << "IOMonitor_MultipleSessions_0100 end";
}

} // namespace CliTool
} // namespace OHOS