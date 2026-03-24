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

#ifndef CLI_EXECUTION_CONTEXT_H
#define CLI_EXECUTION_CONTEXT_H

#include <string>
#include <atomic>
#include <chrono>
#include <json/json.h>
#include "cli_tool_info.h"

namespace OHOS {
namespace CliMgr {

/**
 * @brief 执行模式（已废弃，使用 ExecOptions.background 代替）
 */
enum class ExecutionMode : uint8_t {
    SYNC = 0,      // 同步执行，等待结果
    ASYNC = 1,     // 异步执行，事件回调
};

/**
 * @brief 调用者信息
 */
struct CallerInfo {
    int32_t pid = -1;                // 调用者进程 ID
    int32_t uid = -1;                // 调用者用户 ID
    std::string bundleName;          // 调用者包名
    std::string accessTokenId;       // 访问令牌 ID
};

/**
 * @brief 工具事件
 *
 * 支持的事件类型：
 * - 标准事件: "stdout", "stderr", "exit", "error"
 * - 自定义事件: "progress", "result" 等（由工具描述文件的 eventSchemas 定义）
 */
struct ToolEvent {
    std::string type;      // 事件类型："stdout", "stderr", "exit", "error", "progress", "result" 等
    std::string data;      // 事件数据（JSON 字符串格式，包含 event 和 data 字段）
    int exitCode = 0;      // 退出码（仅 exit 事件）
    int64_t timestamp;     // 时间戳
};

/**
 * @brief 执行上下文
 */
class ExecutionContext {
public:
    ExecutionContext() = default;
    ~ExecutionContext();

    // 禁止拷贝
    ExecutionContext(const ExecutionContext&) = delete;
    ExecutionContext& operator=(const ExecutionContext&) = delete;

    // 支持移动
    ExecutionContext(ExecutionContext&& other) noexcept;
    ExecutionContext& operator=(ExecutionContext&& other) noexcept;

    // 基本信息
    std::string taskId;                      // 任务唯一 ID
    std::string toolName;                    // 工具名称
    ExecutionMode mode;                      // 执行模式

    // 进程信息
    pid_t processId = -1;                    // 工具进程 ID
    std::chrono::time_point<std::chrono::steady_clock> startTime;
    std::chrono::time_point<std::chrono::steady_clock> endTime;

    // 调用者信息
    CallerInfo caller;

    // 管道文件描述符
    int stdoutPipe[2] = {-1, -1};            // [0]=read, [1]=write
    int stderrPipe[2] = {-1, -1};
    int stdinPipe[2] = {-1, -1};

    // 输出缓冲区
    std::string stdoutBuffer;                // 累积的标准输出
    std::string stderrBuffer;                // 累积的标准错误

    // 执行参数
    Json::Value arguments;                   // 执行参数

    // 执行选项
    uint32_t timeoutMs = 30000;              // 超时时间
    std::string workingDir;                  // 工作目录

    // 回调对象（用于异步模式）- 前向声明
    class ICLICallback* callback = nullptr;  // IPC 回调代理

    // 状态标志
    std::atomic<bool> isRunning{false};      // 是否正在运行
    std::atomic<bool> isCompleted{false};    // 是否已完成
    int exitCode = -1;                       // 退出码
    int signal = 0;                          // 终止信号

    /**
     * @brief 关闭所有管道
     */
    void CloseAllPipes();

    /**
     * @brief 关闭写端（通知工具进程 EOF）
     */
    void CloseWriteEnds();

    /**
     * @brief 检查是否超时
     */
    bool IsTimedOut() const;

private:
    void ClosePipe(int pipe[2]);
};

/**
 * @brief 执行结果
 */
struct ExecutionResult {
    std::string taskId;                  // 任务 ID
    int exitCode;                        // 退出码
    std::string stdout;                  // 标准输出
    std::string stderr;                  // 标准错误
    int signal;                          // 信号 (如果被信号终止)
    bool timedOut;                       // 是否超时
    uint64_t executionTimeMs;            // 执行时长

    ExecutionResult()
        : exitCode(-1),
          signal(0),
          timedOut(false),
          executionTimeMs(0)
    {}
};

/**
 * @brief 会话信息
 */
struct SessionInfo {
    std::string sessionId;              // 会话/任务 ID
    std::string toolName;                // 工具名称
    std::string status;                  // "running" | "completed" | "failed"
    uint64_t startTime;                  // 开始时间戳
    uint64_t endTime = 0;                // 结束时间戳（仅 completed/failed 时有值）
    ExecutionResult* result = nullptr;   // 执行结果（仅 completed 时有值）

    SessionInfo() : startTime(0), endTime(0) {}

    ~SessionInfo() {
        delete result;
    }

    // 禁止拷贝
    SessionInfo(const SessionInfo&) = delete;
    SessionInfo& operator=(const SessionInfo&) = delete;

    // 支持移动
    SessionInfo(SessionInfo&& other) noexcept
        : sessionId(std::move(other.sessionId)),
          toolName(std::move(other.toolName)),
          status(std::move(other.status)),
          startTime(other.startTime),
          endTime(other.endTime),
          result(other.result) {
        other.result = nullptr;
    }

    SessionInfo& operator=(SessionInfo&& other) noexcept {
        if (this != &other) {
            delete result;
            sessionId = std::move(other.sessionId);
            toolName = std::move(other.toolName);
            status = std::move(other.status);
            startTime = other.startTime;
            endTime = other.endTime;
            result = other.result;
            other.result = nullptr;
        }
        return *this;
    }
};

/**
 * @brief 任务状态（保留以兼容旧代码）
 */
struct TaskStatus {
    std::string taskId;                  // 任务 ID
    std::string toolName;                // 工具名称
    std::string status;                  // "running", "completed", "failed", "timeout", "cancelled"
    uint64_t startTime;                  // 开始时间戳
    uint64_t endTime;                    // 结束时间戳
    int exitCode;                        // 退出码

    TaskStatus() : startTime(0), endTime(0), exitCode(-1) {}
};

/**
 * @brief 执行选项
 */
struct ExecOptions {
    /**
     * 是否后台执行
     * - false: 前台执行，在 yieldMs 内等待结果返回，超时后自动转入后台
     * - true: 立即进入后台执行，马上返回 sessionId
     *
     * 默认值: false
     */
    bool background = false;

    /**
     * 前台等待超时时间（毫秒）
     * 仅当 background=false 时有效
     *
     * 默认值: 30000 (30秒)
     */
    uint32_t yieldMs = 30000;

    /**
     * 总超时时间（毫秒）
     * 进程运行超过此时间会被强制终止
     * 0 表示使用工具描述文件中的默认值
     */
    uint32_t timeoutMs = 0;

    /**
     * 工作目录
     */
    std::string workingDir;

    /**
     * 环境变量
     */
    Json::Value env;

    // 保留字段以兼容旧代码
    ExecutionMode mode = ExecutionMode::SYNC;  // 已废弃
};

} // namespace CliMgr
} // namespace OHOS

#endif // CLI_EXECUTION_CONTEXT_H
