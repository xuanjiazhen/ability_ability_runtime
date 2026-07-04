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

#ifndef OHOS_ABILITY_RUNTIME_CHILD_EXIT_MONITOR_H
#define OHOS_ABILITY_RUNTIME_CHILD_EXIT_MONITOR_H

#include <atomic>
#include <functional>
#include <memory>
#include <signal.h>
#include <sys/types.h>
#include <thread>
#include <vector>

namespace OHOS {
namespace CliTool {

class ChildExitMonitor {
public:
    using ActivePidProvider = std::function<std::vector<pid_t>()>;
    using ReapedChildHandler = std::function<void(pid_t, int32_t, int32_t)>;

    static std::shared_ptr<ChildExitMonitor> Create();

    ChildExitMonitor() = default;
    ~ChildExitMonitor();

    bool Start(ActivePidProvider activePidProvider, ReapedChildHandler reapedChildHandler);
    void Stop();
    void ProcessExitedChildren(int32_t sig);
    void ReapActiveChildren(const std::vector<pid_t> &pids, int32_t sig);
    void ReapPendingChildExit(pid_t pid, int32_t sig);

    ChildExitMonitor(const ChildExitMonitor &) = delete;
    ChildExitMonitor &operator=(const ChildExitMonitor &) = delete;
    ChildExitMonitor(ChildExitMonitor &&) = delete;
    ChildExitMonitor &operator=(ChildExitMonitor &&) = delete;

private:
    struct MonitorState {
        ActivePidProvider activePidProvider;
        ReapedChildHandler reapedChildHandler;
        int32_t notifyReadFd = -1;
        int32_t notifyWriteFd = -1;
        std::atomic<bool> running = false;
    };

    bool SetNotifyFdFlags(int32_t fd);
    bool InstallSigChldHandler();
    void RestoreSigChldHandler();
    static void ProcessNotifications(const std::shared_ptr<MonitorState> &state);
    static void ProcessExitedChildren(const std::shared_ptr<MonitorState> &state, int32_t sig);
    static bool TryReapChild(const std::shared_ptr<MonitorState> &state, pid_t pid, int32_t sig);

    static void SigChldHandler(int32_t sig, siginfo_t *info, void *context);

    std::shared_ptr<MonitorState> state_ = nullptr;
    std::thread monitorThread_;
    struct sigaction oldSigChldAction_ {};
    bool sigChldActionInstalled_ = false;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CHILD_EXIT_MONITOR_H
