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

#ifndef TIMEOUT_MANAGER_H
#define TIMEOUT_MANAGER_H

#include <string>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include "cli_execution_context.h"

namespace OHOS {
namespace CliMgr {

// 前向声明
class ProcessManager;
class EventDispatcher;

/**
 * @brief 超时任务信息
 */
struct TimeoutTask {
    std::string taskId;
    uint32_t timeoutMs;
    std::chrono::time_point<std::chrono::steady_clock> deadline;
    std::shared_ptr<ExecutionContext> context;
};

/**
 * @brief 超时管理器
 *
 * 管理所有任务的超时控制，超时后强制终止进程
 */
class TimeoutManager {
public:
    TimeoutManager(ProcessManager* processMgr,
                  EventDispatcher* eventDispatcher);
    ~TimeoutManager();

    /**
     * @brief 启动超时管理器
     * @return 是否成功
     */
    bool Start();

    /**
     * @brief 停止超时管理器
     */
    void Stop();

    /**
     * @brief 添加超时任务
     * @param taskId 任务 ID
     * @param context 执行上下文
     * @param timeoutMs 超时时间（毫秒）
     */
    void AddTimeoutTask(const std::string& taskId,
                       std::shared_ptr<ExecutionContext> context,
                       uint32_t timeoutMs);

    /**
     * @brief 取消超时任务
     * @param taskId 任务 ID
     */
    void CancelTimeoutTask(const std::string& taskId);

    /**
     * @brief 检查任务是否超时
     * @param taskId 任务 ID
     * @return 是否超时
     */
    bool IsTaskTimedOut(const std::string& taskId);

private:
    /**
     * @brief 超时检查线程主循环
     */
    void TimeoutCheckThread();

    /**
     * @brief 处理超时任务
     * @param task 超时任务
     */
    void HandleTimeout(TimeoutTask& task);

    /**
     * @brief 通知超时错误
     * @param taskId 任务 ID
     */
    void NotifyTimeout(const std::string& taskId);

private:
    ProcessManager* processManager_;
    EventDispatcher* eventDispatcher_;

    // 超时任务列表（按截止时间排序）
    std::unordered_map<std::string, TimeoutTask> timeoutTasks_;
    std::mutex tasksMutex_;

    // 线程控制
    std::atomic<bool> running_{false};
    std::thread timeoutThread_;
    std::condition_variable cv_;
    std::mutex cvMutex_;

    // 检查间隔
    static constexpr uint32_t CHECK_INTERVAL_MS = 100;

    static constexpr const char* TAG = "TimeoutManager";
};

} // namespace CliMgr
} // namespace OHOS

#endif // TIMEOUT_MANAGER_H
