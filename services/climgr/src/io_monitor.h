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

#ifndef IO_MONITOR_H
#define IO_MONITOR_H

#include <string>
#include <unordered_map>
#include <atomic>
#include <thread>
#include <mutex>
#include <memory>
#include <sys/epoll.h>
#include "cli_execution_context.h"

namespace OHOS {
namespace CliMgr {

// 前向声明
class EventDispatcher;

/**
 * @brief I/O 事件类型
 */
enum class IOEventType : uint8_t {
    STDOUT = 0,
    STDERR = 1,
    PROCESS_EXIT = 2,
    TIMEOUT = 3,
};

/**
 * @brief I/O 事件数据
 */
struct IOEvent {
    IOEventType type;
    std::string taskId;
    std::string data;
    int exitCode;
};

/**
 * @brief I/O 监控器
 *
 * 使用 epoll 监控所有工具进程的管道输出
 */
class IOMonitor {
public:
    explicit IOMonitor(EventDispatcher* dispatcher);
    ~IOMonitor();

    /**
     * @brief 启动监控线程
     * @return 是否成功
     */
    bool Start();

    /**
     * @brief 停止监控线程
     */
    void Stop();

    /**
     * @brief 注册任务的管道进行监控
     * @param context 执行上下文
     * @return 是否注册成功
     */
    bool RegisterTask(const std::shared_ptr<ExecutionContext>& context);

    /**
     * @brief 取消任务监控
     * @param taskId 任务 ID
     */
    void UnregisterTask(const std::string& taskId);

    /**
     * @brief 向工具进程写入数据（通过 stdin）
     * @param taskId 任务 ID
     * @param data 要写入的数据
     * @return 写入的字节数，失败返回 -1
     */
    ssize_t WriteToStdin(const std::string& taskId, const std::string& data);

private:
    /**
     * @brief 监控线程主循环
     */
    void MonitorThread();

    /**
     * @brief 处理管道可读事件
     * @param fd 文件描述符
     * @param taskId 任务 ID
     * @param type 事件类型
     */
    void HandleReadable(int fd, const std::string& taskId, IOEventType type);

    /**
     * @brief 从管道读取数据
     * @param fd 文件描述符
     * @return 读取的数据
     */
    std::string ReadFromPipe(int fd);

    /**
     * @brief 关闭任务的所有管道
     * @param taskId 任务 ID
     */
    void CloseTaskPipes(const std::string& taskId);

private:
    EventDispatcher* dispatcher_;
    int epollFd_ = -1;
    std::atomic<bool> running_{false};
    std::thread monitorThread_;

    // taskId -> ExecutionContext
    std::unordered_map<std::string, std::shared_ptr<ExecutionContext>> tasks_;
    std::mutex tasksMutex_;

    // fd -> taskId
    std::unordered_map<int, std::string> fdToTaskId_;
    std::mutex fdMutex_;

    // taskId -> fd (stdin write end)
    std::unordered_map<std::string, int> stdinWriteFds_;
    std::mutex stdinMutex_;

    // 缓冲区大小
    static constexpr size_t MAX_BUFFER_SIZE = 4096;
    static constexpr int MAX_EPOLL_EVENTS = 64;

    static constexpr const char* TAG = "IOMonitor";
};

} // namespace CliMgr
} // namespace OHOS

#endif // IO_MONITOR_H
