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

#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <memory>
#include <functional>
#include "cli_execution_context.h"
#include "cli_tool_info.h"

namespace OHOS {
namespace CliMgr {

// 前向声明
class IOMonitor;

/**
 * @brief 进程退出回调
 */
using ProcessExitCallback = std::function<void(const std::string&, int, int)>;

/**
 * @brief 进程管理器
 *
 * 负责创建、监控、终止 CLI 工具进程
 */
class ProcessManager {
public:
    explicit ProcessManager(IOMonitor* ioMonitor);
    ~ProcessManager();

    /**
     * @brief 创建执行上下文
     * @param toolName 工具名称
     * @param mode 执行模式
     * @return 执行上下文
     */
    std::shared_ptr<ExecutionContext> CreateExecutionContext(
        const std::string& toolName,
        ExecutionMode mode);

    /**
     * @brief 启动工具进程
     * @param toolInfo 工具信息
     * @param context 执行上下文
     * @return 是否成功
     */
    bool SpawnProcess(const ToolInfo& toolInfo,
                     std::shared_ptr<ExecutionContext> context);

    /**
     * @brief 等待进程退出（同步模式）
     * @param context 执行上下文
     * @param timeoutMs 超时时间
     * @return 退出码
     */
    int WaitForProcess(std::shared_ptr<ExecutionContext> context,
                      uint32_t timeoutMs);

    /**
     * @brief 终止进程
     * @param pid 进程 ID
     * @param signal 信号（SIGTERM 或 SIGKILL）
     * @return 是否成功
     */
    bool TerminateProcess(pid_t pid, int signal = SIGTERM);

    /**
     * @brief 设置进程退出回调
     * @param callback 回调函数
     */
    void SetProcessExitCallback(ProcessExitCallback callback);

private:
    /**
     * @brief 创建管道
     * @param context 执行上下文
     * @return 是否成功
     */
    bool CreatePipes(ExecutionContext& context);

    /**
     * @brief 设置非阻塞模式
     * @param fd 文件描述符
     */
    void SetNonBlocking(int fd);

    /**
     * @brief 准备子进程
     * @param context 执行上下文
     */
    void PrepareChildProcess(ExecutionContext& context);

    /**
     * @brief 构建参数数组
     * @param toolInfo 工具信息
     * @param context 执行上下文
     * @return 参数数组
     */
    std::vector<char*> BuildArgv(const ToolInfo& toolInfo,
                                  const ExecutionContext& context);

    /**
     * @brief 清理进程资源
     * @param taskId 任务 ID
     */
    void CleanupProcess(const std::string& taskId);

private:
    IOMonitor* ioMonitor_;
    ProcessExitCallback exitCallback_;

    // 进程监控线程
    std::unordered_map<pid_t, std::string> pidToTaskId_;
    std::unordered_map<std::string, pid_t> taskIdToPid_;
    std::mutex processMutex_;

    static constexpr const char* TAG = "ProcessManager";
};

} // namespace CliMgr
} // namespace OHOS

#endif // PROCESS_MANAGER_H
