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

#ifndef OHOS_ABILITY_RUNTIME_CLI_MANAGER_SERVICE_H
#define OHOS_ABILITY_RUNTIME_CLI_MANAGER_SERVICE_H

#include <atomic>
#include <memory>
#include <mutex>
#include <sys/types.h>
#include <unordered_map>
#include <utility>

#include "cli_manager_data.h"
#include "cli_manager_stub.h"
#include "cli_tool_manager.h"
#include "io_monitor.h"
#include "process_monitor.h"
#include "session_record.h"
#include "system_ability.h"
#include "timeout_manager.h"

namespace OHOS {
namespace AbilityRuntime {

enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };

class CliManagerService : public SystemAbility,
                          public CliManagerStub {
    DECLEAR_SYSTEM_ABILITY(CliManagerService);

public:
    static sptr<CliManagerService> GetInstance();

    CliManagerService();
    ~CliManagerService();

    void OnStart() override;
    void OnStop() override;

    int32_t QueryTools(std::vector<ToolInfo> &tools) override;

    int32_t ExecTool(const std::string &name,
                     const std::vector<std::pair<std::string, std::string>> &args,
                     const ExecOptions &options,
                     const std::string &challenge,
                     SessionInfo &session) override;

    int32_t RegisterScheduler(const sptr<ICliManagerScheduler> &scheduler) override;
    int32_t UnregisterScheduler() override;
    int32_t SubscribeSession(const std::string &sessionId, const std::string &subscriptionId) override;
    int32_t UnsubscribeSession(const std::string &sessionId, const std::string &subscriptionId) override;
    int32_t CancelSession(const std::string &sessionId) override;
    int32_t QuerySessionStatus(const std::string &sessionId, SessionStatus &status) override;

private:
    void Init();
    std::string GenerateSessionId(const std::string &name, int64_t startTimeMs);
    void AddSessionRecord(const std::shared_ptr<SessionRecord> &record);
    std::shared_ptr<SessionRecord> GetSessionRecord(const std::string &sessionId);
    void RemoveSessionRecord(const std::string &sessionId);
    void HandleProcessExit(const std::string &sessionId, int status, bool timedOut);
    void HandleOutputClosed(const std::string &sessionId, bool isStdout);
    void HandleOutputDrained(const std::string &sessionId);
    void FinalizeBackgroundSession(const std::shared_ptr<SessionRecord> &record);
    std::string GenerateSandBoxConfig(const std::string &challenge);
    uint32_t ResolveTimeout(const ExecOptions &options, const std::string &toolName);

    DISALLOW_COPY_AND_MOVE(CliManagerService);

private:
    static constexpr size_t MAX_SESSION_COUNT = 64;

    static sptr<CliManagerService> instance_;
    static std::mutex instanceMutex_;

    std::mutex mutex_;
    std::mutex sessionsMutex_;
    std::unordered_map<std::string, std::shared_ptr<SessionRecord>> sessionRecords_;
    std::atomic<bool> stopping_ {false};
    std::atomic<uint64_t> sessionCounter_ {0};
    ServiceRunningState state_ = ServiceRunningState::STATE_NOT_START;
    std::unique_ptr<CliToolManager> cliToolManager_;
    std::shared_ptr<TimeoutManager> timeoutManager_;
    ProcessMonitor processMonitor_;
    IOMonitor ioMonitor_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_MANAGER_SERVICE_H
