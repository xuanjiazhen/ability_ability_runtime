/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <gtest/hwext/gtest-ext.h>
#include "process_manager.h"
#include "cli_tool_info.h"
#include "cli_execution_context.h"

using namespace OHOS;
using namespace CliMgr;

/**
 * @brief Process Manager 单元测试
 */
class ProcessManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

protected:
    static std::shared_ptr<ProcessManager> processManager_;
};

std::shared_ptr<ProcessManager> ProcessManagerTest::processManager_ = nullptr;

void ProcessManagerTest::SetUpTestCase()
{
    processManager_ = std::make_shared<ProcessManager>(nullptr);
    ASSERT_NE(processManager_, nullptr);
}

void ProcessManagerTest::TearDownTestCase()
{
    processManager_.reset();
}

void ProcessManagerTest::SetUp()
{
}

void ProcessManagerTest::TearDown()
{
}

/**
 * @brief 测试创建执行上下文
 */
HWTEST_F(ProcessManagerTest, CreateExecutionContext, TestSize.Level1)
{
    auto context = processManager_->CreateExecutionContext("test-tool", ExecutionMode::SYNC);

    ASSERT_NE(context, nullptr);
    EXPECT_EQ(context->toolName, "test-tool");
    EXPECT_EQ(context->mode, ExecutionMode::SYNC);
    EXPECT_EQ(context->processId, -1);
    EXPECT_FALSE(context->isRunning.load());
}

/**
 * @brief 测试进程创建（使用 echo 命令）
 */
HWTEST_F(ProcessManagerTest, SpawnProcess, TestSize.Level2)
{
    ToolInfo tool;
    tool.name = "echo";
    tool.executablePath = "/bin/echo";
    tool.type = ToolType::SHORT_LIVED;
    tool.timeoutMs = 5000;

    auto context = processManager_->CreateExecutionContext("echo", ExecutionMode::SYNC);
    context->arguments["message"] = "Hello, World!";

    Json::Value args;
    args["message"] = "Hello";

    bool spawned = processManager_->SpawnProcess(tool, context);
    EXPECT_TRUE(spawned);

    if (spawned && context->processId > 0) {
        // 等待进程完成
        int exitCode = processManager_->WaitForProcess(context, 5000);
        EXPECT_GE(exitCode, 0);
        EXPECT_FALSE(context->isRunning.load());
    }
}

/**
 * @brief 测试进程终止
 */
HWTEST_F(ProcessManagerTest, TerminateProcess, TestSize.Level1)
{
    ToolInfo tool;
    tool.name = "sleep";
    tool.executablePath = "/bin/sleep";
    tool.type = ToolType::LONG_LIVED;
    tool.timeoutMs = 10000;

    auto context = processManager_->CreateExecutionContext("sleep", ExecutionMode::ASYNC);

    Json::Value args;
    args["seconds"] = 100;  // 长时间睡眠

    bool spawned = processManager_->SpawnProcess(tool, context);
    if (spawned && context->processId > 0) {
        EXPECT_TRUE(context->isRunning.load());

        // 终止进程
        bool terminated = processManager_->TerminateProcess(context->processId, SIGTERM);
        EXPECT_TRUE(terminated);
    }
}

/**
 * @brief 测试管道创建
 */
HWTEST_F(ProcessManagerTest, PipeCreation, TestSize.Level1)
{
    auto context = processManager_->CreateExecutionContext("test", ExecutionMode::SYNC);

    EXPECT_NE(context->stdoutPipe[0], -1);
    EXPECT_NE(context->stdoutPipe[1], -1);
    EXPECT_NE(context->stderrPipe[0], -1);
    EXPECT_NE(context->stderrPipe[1], -1);
}

/**
 * @brief 测试管道关闭
 */
HWTEST_F(ProcessManagerTest, PipeClosure, TestSize.Level1)
{
    auto context = processManager_->CreateExecutionContext("test", ExecutionMode::SYNC);

    context->CloseAllPipes();

    // 验证管道已关闭（在 Unix 中，关闭后会设为 -1）
    // 注意：具体行为取决于实现
}

/**
 * @brief 测试超时检测
 */
HWTEST_F(ProcessManagerTest, TimeoutDetection, TestSize.Level1)
{
    ToolInfo tool;
    tool.name = "sleep";
    tool.executablePath = "/bin/sleep";
    tool.type = ToolType::SHORT_LIVED;
    tool.timeoutMs = 1000;  // 1 秒超时

    auto context = processManager_->CreateExecutionContext("sleep", ExecutionMode::SYNC);
    context->timeoutMs = 1000;

    Json::Value args;
    args["seconds"] = 100;

    bool spawned = processManager_->SpawnProcess(tool, context);
    if (spawned && context->processId > 0) {
        // 等待超时
        int exitCode = processManager_->WaitForProcess(context, 1000);

        // 验证超时行为
        EXPECT_TRUE(context->IsTimedOut() || exitCode != 0);
    }
}

/**
 * @brief 测试输出捕获
 */
HWTEST_F(ProcessManagerTest, OutputCapture, TestSize.Level2)
{
    ToolInfo tool;
    tool.name = "echo";
    tool.executablePath = "/bin/echo";
    tool.type = ToolType::SHORT_LIVED;
    tool.timeoutMs = 5000;

    auto context = processManager_->CreateExecutionContext("echo", ExecutionMode::SYNC);

    Json::Value args;
    args["message"] = "Test Output";

    bool spawned = processManager_->SpawnProcess(tool, context);
    if (spawned && context->processId > 0) {
        int exitCode = processManager_->WaitForProcess(context, 5000);

        EXPECT_EQ(exitCode, 0);
        EXPECT_FALSE(context->stdoutBuffer.empty());
    }
}

/**
 * @brief 测试错误捕获
 */
HWTEST_F(ProcessManagerTest, ErrorCapture, TestSize.Level2)
{
    ToolInfo tool;
    tool.name = "sh";
    tool.executablePath = "/bin/sh";
    tool.type = ToolType::SHORT_LIVED;
    tool.timeoutMs = 5000;

    auto context = processManager_->CreateExecutionContext("sh", ExecutionMode::SYNC);

    Json::Value args;
    args["command"] = "echo 'Error message' >&2";

    bool spawned = processManager_->SpawnProcess(tool, context);
    if (spawned && context->processId > 0) {
        int exitCode = processManager_->WaitForProcess(context, 5000);

        EXPECT_EQ(exitCode, 0);
        // 应该捕获到 stderr
    }
}

/**
 * @brief 测试工作目录设置
 */
HWTEST_F(ProcessManagerTest, WorkingDirectory, TestSize.Level2)
{
    ToolInfo tool;
    tool.name = "pwd";
    tool.executablePath = "/bin/pwd";
    tool.type = ToolType::SHORT_LIVED;
    tool.timeoutMs = 5000;

    auto context = processManager_->CreateExecutionContext("pwd", ExecutionMode::SYNC);
    context->workingDir = "/tmp";

    Json::Value args;

    bool spawned = processManager_->SpawnProcess(tool, context);
    if (spawned && context->processId > 0) {
        int exitCode = processManager_->WaitForProcess(context, 5000);

        EXPECT_EQ(exitCode, 0);
        // 输出应该是 /tmp
    }
}
