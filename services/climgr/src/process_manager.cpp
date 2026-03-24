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

#include "process_manager.h"
#include "io_monitor.h"
#include "hilog_tag_wrapper.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <cstring>
#include <algorithm>

namespace OHOS {
namespace CliMgr {

ProcessManager::ProcessManager(IOMonitor* ioMonitor)
    : ioMonitor_(ioMonitor)
{
}

ProcessManager::~ProcessManager()
{
    // 清理所有进程
    std::lock_guard<std::mutex> lock(processMutex_);
    for (const auto& [pid, taskId] : pidToTaskId_) {
        TerminateProcess(pid, SIGKILL);
    }
}

std::shared_ptr<ExecutionContext> ProcessManager::CreateExecutionContext(
    const std::string& toolName,
    ExecutionMode mode)
{
    auto context = std::make_shared<ExecutionContext>();
    context->toolName = toolName;
    context->mode = mode;
    context->startTime = std::chrono::steady_clock::now();

    return context;
}

bool ProcessManager::SpawnProcess(const ToolInfo& toolInfo,
                                 std::shared_ptr<ExecutionContext> context)
{
    if (!context) {
        TAG_LOGE(TAG, "Invalid execution context");
        return false;
    }

    // 创建管道
    if (!CreatePipes(*context)) {
        TAG_LOGE(TAG, "Failed to create pipes for tool %{public}s", toolInfo.name.c_str());
        return false;
    }

    // Fork 进程
    pid_t pid = fork();

    if (pid < 0) {
        TAG_LOGE(TAG, "Failed to fork process: %{public}d", errno);
        context->CloseAllPipes();
        return false;
    }

    if (pid == 0) {
        // === 子进程 ===
        PrepareChildProcess(*context);

        // 构建参数数组
        std::vector<char*> argv = BuildArgv(toolInfo, *context);

        // 设置环境变量
        char* env[] = { nullptr };

        // 执行工具
        execve(toolInfo.executablePath.c_str(), argv.data(), env);

        // 如果 execve 返回，说明执行失败
        _exit(127);  // exec 失败退出码
    }

    // === 父进程 ===
    context->processId = pid;
    context->isRunning.store(true);

    // 注册到 I/O Monitor
    if (ioMonitor_) {
        ioMonitor_->RegisterTask(context);
    }

    // 记录映射关系
    {
        std::lock_guard<std::mutex> lock(processMutex_);
        pidToTaskId_[pid] = context->taskId;
        taskIdToPid_[context->taskId] = pid;
    }

    TAG_LOGI(TAG, "Tool process spawned: pid=%{public}d, tool=%{public}s",
             pid, toolInfo.name.c_str());

    return true;
}

int ProcessManager::WaitForProcess(std::shared_ptr<ExecutionContext> context,
                                   uint32_t timeoutMs)
{
    if (!context || context->processId < 0) {
        TAG_LOGE(TAG, "Invalid context or process");
        return -1;
    }

    pid_t pid = context->processId;
    int status;
    int options = 0;
    int result = -1;

    if (timeoutMs > 0) {
        // 带超时的等待（需要在单独的线程中实现）
        // 这里简化为不阻塞等待
        options = WNOHANG;
    }

    pid_t ret = waitpid(pid, &status, options);

    if (ret == pid) {
        if (WIFEXITED(status)) {
            result = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            context->signal = WTERMSIG(status);
            result = -1;
        }

        context->isRunning.store(false);
        context->isCompleted.store(true);
        context->exitCode = result;
        context->endTime = std::chrono::steady_clock::now();
    } else if (ret == 0) {
        // 进程仍在运行（WNOHANG 模式）
        result = 0;  // 表示超时但进程仍在运行
    } else {
        TAG_LOGE(TAG, "waitpid failed: %{public}d", errno);
        result = -1;
    }

    return result;
}

bool ProcessManager::TerminateProcess(pid_t pid, int signal)
{
    if (pid <= 0) {
        return false;
    }

    int ret = kill(pid, signal);

    if (ret != 0) {
        TAG_LOGE(TAG, "Failed to send signal %{public}d to pid %{public}d: %{public}d",
                 signal, pid, errno);
        return false;
    }

    TAG_LOGI(TAG, "Sent signal %{public}d to pid %{public}d", signal, pid);
    return true;
}

void ProcessManager::SetProcessExitCallback(ProcessExitCallback callback)
{
    exitCallback_ = std::move(callback);
}

bool ProcessManager::CreatePipes(ExecutionContext& context)
{
    // 创建 stdout 管道
    if (pipe(context.stdoutPipe) != 0) {
        TAG_LOGE(TAG, "Failed to create stdout pipe: %{public}d", errno);
        return false;
    }

    // 创建 stderr 管道
    if (pipe(context.stderrPipe) != 0) {
        TAG_LOGE(TAG, "Failed to create stderr pipe: %{public}d", errno);
        close(context.stdoutPipe[0]);
        close(context.stdoutPipe[1]);
        return false;
    }

    // 创建 stdin 管道（可选）
    if (pipe(context.stdinPipe) != 0) {
        TAG_LOGW(TAG, "Failed to create stdin pipe: %{public}d (optional)", errno);
        context.stdinPipe[0] = -1;
        context.stdinPipe[1] = -1;
    }

    // 设置读端为非阻塞模式
    SetNonBlocking(context.stdoutPipe[0]);
    SetNonBlocking(context.stderrPipe[0]);
    if (context.stdinPipe[0] >= 0) {
        SetNonBlocking(context.stdinPipe[0]);
    }

    TAG_LOGI(TAG, "Pipes created for task %{public}s", context.taskId.c_str());
    return true;
}

void ProcessManager::SetNonBlocking(int fd)
{
    if (fd < 0) {
        return;
    }

    int flags = fcntl(fd, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
}

void ProcessManager::PrepareChildProcess(ExecutionContext& context)
{
    // 重定向 stdout
    if (context.stdoutPipe[1] != STDOUT_FILENO) {
        dup2(context.stdoutPipe[1], STDOUT_FILENO);
    }

    // 重定向 stderr
    if (context.stderrPipe[1] != STDERR_FILENO) {
        dup2(context.stderrPipe[1], STDERR_FILENO);
    }

    // 重定向 stdin（如果需要）
    if (context.stdinPipe[0] >= 0) {
        if (context.stdinPipe[0] != STDIN_FILENO) {
            dup2(context.stdinPipe[0], STDIN_FILENO);
        }
        close(context.stdinPipe[1]);  // 关闭 stdin 写端
    } else {
        // 如果不需要 stdin，重定向到 /dev/null
        int nullFd = open("/dev/null", O_RDONLY);
        if (nullFd >= 0) {
            dup2(nullFd, STDIN_FILENO);
            close(nullFd);
        }
    }

    // 关闭所有不需要的文件描述符
    // 关闭管道的读端（子进程只需要写端）
    close(context.stdoutPipe[0]);
    close(context.stderrPipe[0]);

    // 设置进程组 ID（便于进程管理）
    setsid();
}

std::vector<char*> ProcessManager::BuildArgv(
    const ToolInfo& toolInfo,
    const ExecutionContext& context)
{
    std::vector<char*> argv;

    // 第一个参数是程序名称
    argv.push_back(const_cast<char*>(toolInfo.executablePath.c_str()));

    // 根据 JSON 参数构建命令行参数
    if (context.arguments.isObject()) {
        for (const auto& key : context.arguments.getMemberNames()) {
            Json::Value value = context.arguments[key];

            if (value.isString()) {
                std::string arg = "--" + key + "=" + value.asString();
                argv.push_back(strdup(arg.c_str()));
            } else if (value.isInt()) {
                std::string arg = "--" + key + "=" + std::to_string(value.asInt());
                argv.push_back(strdup(arg.c_str()));
            } else if (value.isUInt()) {
                std::string arg = "--" + key + "=" + std::to_string(value.asUInt());
                argv.push_back(strdup(arg.c_str()));
            } else if (value.isBool()) {
                std::string arg = value.asBool() ? ("--" + key) : ("--no-" + key);
                argv.push_back(strdup(arg.c_str()));
            }
        }
    }

    argv.push_back(nullptr);  // argv 必须以 nullptr 结尾

    return argv;
}

void ProcessManager::CleanupProcess(const std::string& taskId)
{
    std::lock_guard<std::mutex> lock(processMutex_);

    // 从映射中移除
    auto pidIt = taskIdToPid_.find(taskId);
    if (pidIt != taskIdToPid_.end()) {
        pid_t pid = pidIt->second;
        pidToTaskId_.erase(pid);
        taskIdToPid_.erase(pidIt);
    }
}

} // namespace CliMgr
} // namespace OHOS
