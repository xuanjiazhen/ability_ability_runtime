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

#ifndef PROCESS_MONITOR_H
#define PROCESS_MONITOR_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>
#include "cli_execution_context.h"

namespace OHOS {
namespace CliMgr {

// 前向声明
class EventDispatcher;

/**
 * @brief 进程监控器
 *
 * 监控所有工具进程的退出事件
 */
class ProcessMonitor {
public:
    explicit ProcessMonitor(EventDispatcher* eventDispatcher);
    ~ProcessMonitor();

    /**
     * @brief 启动监控
     * @return 是否成功
     */
    bool Start();

    /**
     * @brief 停止监控
     */
    void Stop();

    /**
     * @brief 注册进程监控
     * @param pid 进程 ID
     * @param taskId 任务 ID
     */
    void RegisterProcess(pid_t pid, const std::string& taskId);

    /**
     * @brief 注销进程监控
     * @param pid 进程 ID
     */
    void UnregisterProcess(pid_t pid);

    /**
     * @brief 获取进程对应的任务 ID
     * @param pid 进程 ID
     * @return 任务 ID
     */
    std::string GetTaskId(pid_t pid);

private:
    /**
     * @brief 监控线程主循环
     */
    void MonitorThread();

    /**
     * @brief 处理进程退出
     * @param pid 退出的进程 ID
     * @param status 退出状态
     */
    void HandleProcessExit(pid_t pid, int status);

    /**
     * @brief 等待进程退出
     * @param pid 进程 ID
     * @param status 输出参数，退出状态
     * @return 是否成功
     */
    bool WaitForProcessExit(pid_t pid, int* status);

private:
    EventDispatcher* eventDispatcher_;

    // pid -> taskId 映射
    std::unordered_map<pid_t, std::string> pidToTaskId_;
    std::mutex pidMutex_;

    // 线程控制
    std::atomic<bool> running_{false};
    std::thread monitorThread_;

    static constexpr const char* TAG = "ProcessMonitor";
};

} // namespace CliMgr
} // namespace OHOS

#endif // PROCESS_MONITOR_H
