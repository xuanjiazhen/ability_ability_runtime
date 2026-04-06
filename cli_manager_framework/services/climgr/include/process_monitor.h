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

#ifndef OHOS_ABILITY_RUNTIME_PROCESS_MONITOR_H
#define OHOS_ABILITY_RUNTIME_PROCESS_MONITOR_H

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <sys/types.h>

namespace OHOS {
namespace AbilityRuntime {

class ProcessMonitor {
public:
    using ExitCallback = std::function<void(const std::string &, int, bool)>;
    using TimeoutCallback = std::function<void(const std::string &)>;

    ProcessMonitor() = default;
    ~ProcessMonitor();

    int32_t Start();
    void Stop();

    int32_t RegisterSession(pid_t pid, const std::string &sessionId, int64_t startTimeMs, int32_t timeoutMs);
    void UnregisterSession(const std::string &sessionId);

    void SetExitCallback(ExitCallback callback);
    void SetTimeoutCallback(TimeoutCallback callback);

private:
    struct ProcessEntry {
        std::string sessionId;
        int64_t startTimeMs = 0;
        int32_t timeoutMs = 0;
        bool timedOut = false;
        bool exitObserved = false;
    };

    void MonitorLoop();
    void CheckTimeouts();
    void HandleExit(pid_t pid, int status);

    std::atomic<bool> running_ {false};
    std::thread monitorThread_;
    std::mutex entriesMutex_;
    std::unordered_map<pid_t, ProcessEntry> entries_;
    ExitCallback exitCallback_;
    TimeoutCallback timeoutCallback_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_PROCESS_MONITOR_H
