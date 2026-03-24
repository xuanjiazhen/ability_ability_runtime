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

#include "io_monitor.h"
#include "event_dispatcher.h"
#include "hilog_tag_wrapper.h"
#include <json/json.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <algorithm>
#include <sstream>

namespace OHOS {
namespace CliMgr {

IOMonitor::IOMonitor(EventDispatcher* dispatcher)
    : dispatcher_(dispatcher), epollFd_(-1)
{
}

IOMonitor::~IOMonitor()
{
    Stop();
}

bool IOMonitor::Start()
{
    if (running_.load()) {
        TAG_LOGW(TAG, "IOMonitor already running");
        return true;
    }

    // 创建 epoll 实例
    epollFd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epollFd_ < 0) {
        TAG_LOGE(TAG, "Failed to create epoll: %{public}d", errno);
        return false;
    }

    running_.store(true);
    monitorThread_ = std::thread(&IOMonitor::MonitorThread, this);

    TAG_LOGI(TAG, "IOMonitor started");
    return true;
}

void IOMonitor::Stop()
{
    if (!running_.load()) {
        return;
    }

    running_.store(false);

    // 关闭 epoll fd 以唤醒 epoll_wait
    if (epollFd_ >= 0) {
        close(epollFd_);
        epollFd_ = -1;
    }

    if (monitorThread_.joinable()) {
        monitorThread_.join();
    }

    TAG_LOGI(TAG, "IOMonitor stopped");
}

bool IOMonitor::RegisterTask(const std::shared_ptr<ExecutionContext>& context)
{
    if (!context || !context->taskId.empty()) {
        TAG_LOGE(TAG, "Invalid context or already registered");
        return false;
    }

    std::lock_guard<std::mutex> lock(tasksMutex_);
    tasks_[context->taskId] = context;

    // 注册 stdout 读端
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;  // 监听可读事件
    ev.data.fd = context->stdoutPipe[0];

    if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, context->stdoutPipe[0], &ev) < 0) {
        TAG_LOGE(TAG, "Failed to register stdout fd: %{public}d", errno);
        return false;
    }

    {
        std::lock_guard<std::mutex> fdLock(fdMutex_);
        fdToTaskId_[context->stdoutPipe[0]] = context->taskId;
    }

    // 注册 stderr 读端
    ev.data.fd = context->stderrPipe[0];
    if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, context->stderrPipe[0], &ev) < 0) {
        TAG_LOGE(TAG, "Failed to register stderr fd: %{public}d", errno);
        // 清理已注册的 stdout
        epoll_ctl(epollFd_, EPOLL_CTL_DEL, context->stdoutPipe[0], nullptr);
        {
            std::lock_guard<std::mutex> fdLock(fdMutex_);
            fdToTaskId_.erase(context->stdoutPipe[0]);
        }
        return false;
    }

    {
        std::lock_guard<std::mutex> fdLock(fdMutex_);
        fdToTaskId_[context->stderrPipe[0]] = context->taskId;
    }

    // 保存 stdin 写端（如果有）
    if (context->stdinPipe[1] >= 0) {
        std::lock_guard<std::mutex> stdinLock(stdinMutex_);
        stdinWriteFds_[context->taskId] = context->stdinPipe[1];
    }

    TAG_LOGI(TAG, "Task registered for monitoring: %{public}s", context->taskId.c_str());
    return true;
}

void IOMonitor::UnregisterTask(const std::string& taskId)
{
    std::shared_ptr<ExecutionContext> context;
    {
        std::lock_guard<std::mutex> lock(tasksMutex_);
        auto it = tasks_.find(taskId);
        if (it != tasks_.end()) {
            context = it->second;
            tasks_.erase(it);
        }
    }

    if (!context) {
        return;
    }

    CloseTaskPipes(taskId);

    TAG_LOGI(TAG, "Task unregistered: %{public}s", taskId.c_str());
}

void IOMonitor::MonitorThread()
{
    TAG_LOGI(TAG, "IOMonitor thread started");

    while (running_.load()) {
        struct epoll_event events[MAX_EPOLL_EVENTS];
        int nfds = epoll_wait(epollFd_, events, MAX_EPOLL_EVENTS, 100);  // 100ms timeout

        if (nfds < 0) {
            if (errno == EINTR) {
                continue;  // 被信号中断，继续
            }
            TAG_LOGE(TAG, "epoll_wait error: %{public}d", errno);
            break;
        }

        for (int i = 0; i < nfds; ++i) {
            int fd = events[i].data.fd;

            // 查找对应的 taskId
            std::string taskId;
            {
                std::lock_guard<std::mutex> lock(fdMutex_);
                auto it = fdToTaskId_.find(fd);
                if (it != fdToTaskId_.end()) {
                    taskId = it->second;
                }
            }

            if (taskId.empty()) {
                continue;
            }

            // 查找 context 以确定是 stdout 还是 stderr
            std::shared_ptr<ExecutionContext> context;
            {
                std::lock_guard<std::mutex> lock(tasksMutex_);
                auto it = tasks_.find(taskId);
                if (it != tasks_.end()) {
                    context = it->second;
                }
            }

            if (!context) {
                continue;
            }

            // 确定事件类型
            IOEventType type = (fd == context->stdoutPipe[0]) ?
                               IOEventType::STDOUT : IOEventType::STDERR;

            // 处理可读事件
            HandleReadable(fd, taskId, type);
        }
    }

    TAG_LOGI(TAG, "IOMonitor thread exited");
}

void IOMonitor::HandleReadable(int fd, const std::string& taskId, IOEventType type)
{
    std::string data = ReadFromPipe(fd);

    if (data.empty() && errno != EAGAIN) {
        // 管道关闭（EOF）或错误
        TAG_LOGI(TAG, "Pipe closed for task %{public}s", taskId.c_str());
        return;
    }

    if (!dispatcher_ || data.empty()) {
        return;
    }

    // 按行分割数据
    std::vector<std::string> lines = SplitIntoLines(data);

    for (const auto& line : lines) {
        // 检查是否为 JSON 事件行
        if (IsJsonLine(line)) {
            // 尝试解析 JSON 事件
            Json::Value root;
            Json::CharReaderBuilder builder;
            std::istringstream iss(line);
            std::string errs;

            if (Json::parseFromStream(builder, iss, &root, &errs)) {
                // 检查是否包含 event 字段
                if (root.isMember("event") && root["event"].isString()) {
                    std::string eventType = root["event"].asString();

                    // 构造自定义事件
                    IOEvent event;
                    event.type = type;
                    event.taskId = taskId;

                    // 将 event 和 data 序列化为 JSON 字符串
                    Json::Value eventWrapper;
                    eventWrapper["type"] = eventType;
                    if (root.isMember("data")) {
                        eventWrapper["data"] = root["data"];
                    }
                    if (root.isMember("timestamp")) {
                        eventWrapper["timestamp"] = root["timestamp"];
                    }

                    Json::StreamWriterBuilder jsonBuilder;
                    jsonBuilder["indentation"] = "";
                    event.data = Json::writeString(jsonBuilder, eventWrapper);

                    dispatcher_->DispatchIOEvent(event);

                    TAG_LOGD(TAG, "Custom event dispatched: task=%{public}s, type=%{public}s",
                             taskId.c_str(), eventType.c_str());
                } else {
                    // 没有 event 字段，作为普通 stdout 处理
                    DispatchStdoutLine(taskId, line);
                }
            } else {
                // JSON 解析失败，作为普通 stdout 处理
                DispatchStdoutLine(taskId, line);
            }
        } else {
            // 非 JSON 行，作为普通 stdout 处理
            DispatchStdoutLine(taskId, line);
        }
    }
}

void IOMonitor::DispatchStdoutLine(const std::string& taskId, const std::string& line)
{
    IOEvent event;
    event.type = IOEventType::STDOUT;
    event.taskId = taskId;
    event.data = line;

    dispatcher_->DispatchIOEvent(event);
}

std::string IOMonitor::ReadFromPipe(int fd)
{
    std::string result;
    char buffer[MAX_BUFFER_SIZE];

    while (true) {
        ssize_t n = read(fd, buffer, sizeof(buffer));

        if (n > 0) {
            result.append(buffer, n);
        } else if (n == 0) {
            // EOF
            break;
        } else {  // n < 0
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 非阻塞模式下没有更多数据
                break;
            } else {
                // 真正的错误
                TAG_LOGE(TAG, "Read error: %{public}d", errno);
                break;
            }
        }
    }

    return result;
}

ssize_t IOMonitor::WriteToStdin(const std::string& taskId, const std::string& data)
{
    int stdinFd = -1;
    {
        std::lock_guard<std::mutex> lock(stdinMutex_);
        auto it = stdinWriteFds_.find(taskId);
        if (it != stdinWriteFds_.end()) {
            stdinFd = it->second;
        }
    }

    if (stdinFd < 0) {
        TAG_LOGE(TAG, "No stdin fd for task %{public}s", taskId.c_str());
        return -1;
    }

    ssize_t written = write(stdinFd, data.c_str(), data.size());
    if (written < 0) {
        TAG_LOGE(TAG, "Write to stdin failed: %{public}d", errno);
    }

    return written;
}

void IOMonitor::CloseTaskPipes(const std::string& taskId)
{
    std::shared_ptr<ExecutionContext> context;
    {
        std::lock_guard<std::mutex> lock(tasksMutex_);
        auto it = tasks_.find(taskId);
        if (it != tasks_.end()) {
            context = it->second;
        }
    }

    if (!context) {
        return;
    }

    // 从 epoll 移除并关闭管道
    if (context->stdoutPipe[0] >= 0) {
        epoll_ctl(epollFd_, EPOLL_CTL_DEL, context->stdoutPipe[0], nullptr);
        close(context->stdoutPipe[0]);
        context->stdoutPipe[0] = -1;
    }

    if (context->stderrPipe[0] >= 0) {
        epoll_ctl(epollFd_, EPOLL_CTL_DEL, context->stderrPipe[0], nullptr);
        close(context->stderrPipe[0]);
        context->stderrPipe[0] = -1;
    }

    if (context->stdinPipe[1] >= 0) {
        close(context->stdinPipe[1]);
        context->stdinPipe[1] = -1;
    }

    // 清理映射
    {
        std::lock_guard<std::mutex> lock(fdMutex_);
        fdToTaskId_.erase(context->stdoutPipe[0]);
        fdToTaskId_.erase(context->stderrPipe[0]);
    }

    {
        std::lock_guard<std::mutex> lock(stdinMutex_);
        stdinWriteFds_.erase(taskId);
    }
}

std::vector<std::string> IOMonitor::SplitIntoLines(const std::string& data)
{
    std::vector<std::string> lines;
    std::stringstream ss(data);
    std::string line;

    while (std::getline(ss, line)) {
        // 保留换行符前的内容，去掉回车符
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }

    return lines;
}

bool IOMonitor::IsJsonLine(const std::string& line)
{
    if (line.empty()) {
        return false;
    }

    // JSON 行必须以 { 开头，以 } 结尾
    if (line.front() != '{' || line.back() != '}') {
        return false;
    }

    // 尝试解析为 JSON
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::istringstream iss(line);
    std::string errs;

    return Json::parseFromStream(builder, iss, &root, &errs);
}

} // namespace CliMgr
} // namespace OHOS
