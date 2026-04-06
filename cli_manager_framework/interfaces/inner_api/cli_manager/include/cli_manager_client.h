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

#ifndef OHOS_ABILITY_RUNTIME_CLI_MANAGER_CLIENT_H
#define OHOS_ABILITY_RUNTIME_CLI_MANAGER_CLIENT_H

#include <mutex>
#include <memory>
#include <functional>
#include <utility>

#include "iremote_object.h"
#include "icli_manager.h"

namespace OHOS {
namespace AbilityRuntime {

/**
 * @class CliManagerClient
 * CliManagerClient is used to manage CLI tool operations
 */
class CliManagerClient {
public:
    using SessionEventHandler = std::function<void(const std::string &, const std::string &, const ToolEvent &)>;
    using ServiceDeathHandler = std::function<void()>;

    CliManagerClient() = default;
    virtual ~CliManagerClient() = default;

    static CliManagerClient& GetInstance();

    /**
     * @brief Query all available tools
     * @param tools Output vector of tool information
     * @return Returns ERR_OK on success, others on failure
     */
    int32_t QueryTools(std::vector<ToolInfo> &tools);

    /**
     * @brief Execute tool
     * @param name Tool name
     * @param args Arguments as key-value pairs (preserves insertion order)
     * @param options Execution options
     * @param challenge Challenge string
     * @param session Output session information
     * @return Returns ERR_OK on success, others on failure
     */
    int32_t ExecTool(const std::string &name,
                     const std::vector<std::pair<std::string, std::string>> &args,
                     const ExecOptions &options,
                     const std::string &challenge,
                     SessionInfo &session);

    int32_t SubscribeSession(const std::string &sessionId, const std::string &subscriptionId);

    int32_t UnsubscribeSession(const std::string &sessionId, const std::string &subscriptionId);

    int32_t CancelSession(const std::string &sessionId);

    int32_t QuerySessionStatus(const std::string &sessionId, SessionStatus &status);

    void SetSessionEventHandler(SessionEventHandler handler);

    void SetServiceDeathHandler(ServiceDeathHandler handler);

    /**
     * @brief Clear proxy when service died
     */
    void ClearProxy();

private:
    sptr<ICliManager> GetCliManagerProxy();
    bool LoadCliManagerService();
    void SetCliManager(const sptr<IRemoteObject> &remoteObject);
    sptr<ICliManager> GetCliManager();
    int32_t EnsureSchedulerRegistered(const sptr<ICliManager> &proxy);
    int32_t DispatchSessionEvent(const std::string &sessionId, const std::string &subscriptionId, const ToolEvent &event);

    class CliMgrDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit CliMgrDeathRecipient(const std::function<void(const wptr<IRemoteObject>&)> &proxy)
            : proxy_(proxy) {}
        virtual ~CliMgrDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

    private:
        std::function<void(const wptr<IRemoteObject>&)> proxy_;
    };

private:
    std::mutex mutex_;
    sptr<ICliManager> cliManager_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
    sptr<ICliManagerScheduler> schedulerStub_ = nullptr;
    SessionEventHandler sessionEventHandler_;
    ServiceDeathHandler serviceDeathHandler_;
    bool schedulerRegistered_ = false;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_MANAGER_CLIENT_H
