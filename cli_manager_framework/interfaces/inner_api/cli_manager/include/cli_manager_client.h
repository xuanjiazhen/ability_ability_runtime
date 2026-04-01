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
     * @param result Output execution result
     * @return Returns ERR_OK on success, others on failure
     */
    int32_t ExecTool(const std::string &name,
                     const std::vector<std::pair<std::string, std::string>> &args,
                     const ExecOptions &options,
                     ExecResult &result);

    /**
     * @brief Clear proxy when service died
     */
    void ClearProxy();

private:
    sptr<ICliManager> GetCliManagerProxy();
    bool LoadCliManagerService();
    void SetCliManager(const sptr<IRemoteObject> &remoteObject);
    sptr<ICliManager> GetCliManager();

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
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_MANAGER_CLIENT_H
