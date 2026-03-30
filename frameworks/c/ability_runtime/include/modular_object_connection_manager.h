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

#ifndef OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_CONNECTION_MANAGER_H
#define OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_CONNECTION_MANAGER_H

#include <atomic>
#include <mutex>
#include <unordered_map>

#include "ability_connect_callback.h"
#include "ability_connection.h"
#include "connect_options_impl.h"
#include "errors.h"
#include "iremote_object.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
class ModularObjectConnectionManager;

/**
 * @brief Connection record for ModularObjectExtension.
 * Each connect call creates a new record - no connection reuse.
 */
struct ModularObjectConnectionRecord {
    sptr<IRemoteObject> token;
    AAFwk::Want want;
    sptr<AbilityConnectCallback> callback;
    sptr<AbilityConnection> connection;
    int64_t connectionId;

    ModularObjectConnectionRecord(const sptr<IRemoteObject> &token, const AAFwk::Want &want,
        const sptr<AbilityConnectCallback> &callback, const sptr<AbilityConnection> &connection, int64_t id)
        : token(token), want(want), callback(callback), connection(connection), connectionId(id)
    {}
};

/**
 * @brief Connection callback for ModularObjectExtension.
 * Handles OnAbilityConnectDone/OnAbilityDisconnectDone and invokes user C callbacks.
 */
class ModularObjectConnectCallback : public AbilityConnectCallback {
public:
    ModularObjectConnectCallback(int64_t connectionId,
        const std::shared_ptr<OH_AbilityRuntime_ConnectOptionsState> &state,
        ModularObjectConnectionManager &manager);
    ~ModularObjectConnectCallback() override = default;

    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;

private:
    int64_t connectionId_ = 0;
    std::weak_ptr<OH_AbilityRuntime_ConnectOptionsState> state_;
    ModularObjectConnectionManager &manager_;
};

/**
 * @brief Manages connections to ModularObjectExtension instances.
 *
 * Key difference from ConnectionManager: NO connection reuse.
 * Each connect call creates a new connection.
 */
class ModularObjectConnectionManager {
public:
    ~ModularObjectConnectionManager() = default;
    ModularObjectConnectionManager(const ModularObjectConnectionManager &) = delete;
    ModularObjectConnectionManager &operator=(const ModularObjectConnectionManager &) = delete;

    static ModularObjectConnectionManager &GetInstance();

    /**
     * @brief Connect to ModularObjectExtension - always creates new connection.
     */
    ErrCode ConnectModularObjectExtension(const sptr<IRemoteObject> &token, const AAFwk::Want &want,
        const std::shared_ptr<OH_AbilityRuntime_ConnectOptionsState> &state, int64_t &connectionId);

    /**
     * @brief Disconnect from ModularObjectExtension by connection ID.
     */
    ErrCode DisconnectModularObjectExtension(const sptr<IRemoteObject> &token, int64_t connectionId);

    /**
     * @brief Remove connection record (called by callback on disconnect/failure).
     */
    void RemoveConnectionRecord(int64_t connectionId);

private:
    ModularObjectConnectionManager() = default;

    int64_t GenerateConnectionId();

    std::atomic<int64_t> nextConnectionId_{1};
    std::mutex connectionMutex_;
    std::unordered_map<int64_t, ModularObjectConnectionRecord> connectionRecords_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_CONNECTION_MANAGER_H
