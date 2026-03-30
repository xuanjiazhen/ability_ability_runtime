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

#include "modular_object_connection_manager.h"

#include <cinttypes>

#include "ability_manager_client.h"
#include "connect_options_impl.h"
#include "hilog_tag_wrapper.h"
#include "ipc_inner_object.h"
#include "modular_object_utils.h"
#include "modular_object_extension_types.h"
#include "want_manager.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
void NotifyFailed(std::shared_ptr<OH_AbilityRuntime_ConnectOptionsState> state, int32_t businessErrorCode)
{
    if (state == nullptr) {
        return;
    }
    OH_AbilityRuntime_ConnectOptions_OnFailedCallback callback = nullptr;
    OH_AbilityRuntime_ConnectOptions *owner = nullptr;
    {
        std::lock_guard<std::mutex> guard(state->mutex);
        if (!state->alive) {
            return;
        }
        callback = state->onFailedCallback;
        owner = state->owner;
    }
    if (callback != nullptr) {
        callback(owner, businessErrorCode);
    }
}

} // namespace

// ========== ModularObjectConnectCallback ==========

ModularObjectConnectCallback::ModularObjectConnectCallback(int64_t connectionId,
    const std::shared_ptr<OH_AbilityRuntime_ConnectOptionsState> &state,
    ModularObjectConnectionManager &manager)
    : connectionId_(connectionId), state_(state), manager_(manager)
{}

void ModularObjectConnectCallback::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    if (resultCode != ERR_OK || remoteObject == nullptr) {
        auto businessErrorCode = (resultCode == ERR_OK) ?
            ABILITY_RUNTIME_ERROR_CODE_INTERNAL :
            ModularObjectUtils::ConvertConnectBusinessErrorCode(resultCode);
        NotifyFailed(state_.lock(), businessErrorCode);
        manager_.RemoveConnectionRecord(connectionId_);
        return;
    }

    auto state = state_.lock();
    if (state == nullptr) {
        return;
    }

    OH_AbilityRuntime_ConnectOptions_OnConnectCallback callback = nullptr;
    OH_AbilityRuntime_ConnectOptions *owner = nullptr;
    {
        std::lock_guard<std::mutex> guard(state->mutex);
        if (!state->alive) {
            return;
        }
        callback = state->onConnectCallback;
        owner = state->owner;
    }
    if (callback == nullptr) {
        return;
    }

    AbilityBase_Element cElement;
    if (!ModularObjectUtils::BuildElement(element, cElement)) {
        NotifyFailed(state, ABILITY_RUNTIME_ERROR_CODE_INTERNAL);
        return;
    }
    sptr<IRemoteObject> remoteObjectCopy = remoteObject;
    OHIPCRemoteProxy *proxy = CreateIPCRemoteProxy(remoteObjectCopy);
    if (proxy == nullptr) {
        ModularObjectUtils::DestroyElement(cElement);
        NotifyFailed(state, ABILITY_RUNTIME_ERROR_CODE_INTERNAL);
        return;
    }
    callback(owner, &cElement, proxy);
    ModularObjectUtils::DestroyElement(cElement);
}

void ModularObjectConnectCallback::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    auto state = state_.lock();
    if (resultCode != ERR_OK) {
        NotifyFailed(state, ModularObjectUtils::ConvertConnectBusinessErrorCode(resultCode));
        manager_.RemoveConnectionRecord(connectionId_);
        return;
    }

    if (state != nullptr) {
        OH_AbilityRuntime_ConnectOptions_OnDisconnectCallback callback = nullptr;
        OH_AbilityRuntime_ConnectOptions *owner = nullptr;
        {
            std::lock_guard<std::mutex> guard(state->mutex);
            if (state->alive) {
                callback = state->onDisconnectCallback;
                owner = state->owner;
            }
        }

        if (callback != nullptr) {
            AbilityBase_Element cElement;
            if (ModularObjectUtils::BuildElement(element, cElement)) {
                callback(owner, &cElement);
                ModularObjectUtils::DestroyElement(cElement);
            } else {
                NotifyFailed(state, ABILITY_RUNTIME_ERROR_CODE_INTERNAL);
            }
        }
    }
    manager_.RemoveConnectionRecord(connectionId_);
}

// ========== ModularObjectConnectionManager ==========

ModularObjectConnectionManager &ModularObjectConnectionManager::GetInstance()
{
    static ModularObjectConnectionManager instance;
    return instance;
}

int64_t ModularObjectConnectionManager::GenerateConnectionId()
{
    return nextConnectionId_.fetch_add(1);
}

ErrCode ModularObjectConnectionManager::ConnectModularObjectExtension(const sptr<IRemoteObject> &token,
    const AAFwk::Want &want, const std::shared_ptr<OH_AbilityRuntime_ConnectOptionsState> &state,
    int64_t &connectionId)
{
    if (token == nullptr || state == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Invalid token or state");
        return AAFwk::ERR_INVALID_CALLER;
    }

    // KEY: No connection search - always create new connection
    int64_t newId = GenerateConnectionId();

    // Create callback for this connection
    auto connectCallback = sptr<ModularObjectConnectCallback>::MakeSptr(newId, state, *this);
    if (connectCallback == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to create connect callback");
        return AAFwk::ERR_INVALID_CALLER;
    }

    // Create AbilityConnection and register callback
    sptr<AbilityConnection> abilityConnection = sptr<AbilityConnection>::MakeSptr();
    if (abilityConnection == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to create ability connection");
        return AAFwk::ERR_INVALID_CALLER;
    }
    abilityConnection->AddConnectCallback(connectCallback);
    abilityConnection->SetConnectionState(CONNECTION_STATE_CONNECTING);

    // Connect directly via AbilityManagerClient (bypass ConnectionManager)
    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbilityWithExtensionType(
        want, abilityConnection, token, AAFwk::DEFAULT_INVAL_VALUE, GetModularObjectExtensionType());

    if (ret == ERR_OK) {
        std::lock_guard<std::mutex> guard(connectionMutex_);
        connectionRecords_.emplace(std::piecewise_construct,
            std::forward_as_tuple(newId),
            std::forward_as_tuple(token, want, connectCallback, abilityConnection, newId));
        TAG_LOGI(AAFwkTag::APPKIT, "Connection created, id: %{public}" PRId64, newId);
    } else {
        TAG_LOGE(AAFwkTag::APPKIT, "Connect failed: %{public}d", ret);
    }

    connectionId = newId;
    return ret;
}

ErrCode ModularObjectConnectionManager::DisconnectModularObjectExtension(
    const sptr<IRemoteObject> &token, int64_t connectionId)
{
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Null token");
        return AAFwk::ERR_INVALID_CALLER;
    }

    sptr<AbilityConnection> abilityConnection;
    {
        std::lock_guard<std::mutex> guard(connectionMutex_);
        auto iter = connectionRecords_.find(connectionId);
        if (iter == connectionRecords_.end()) {
            TAG_LOGE(AAFwkTag::APPKIT, "Connection id not found: %{public}" PRId64, connectionId);
            return AAFwk::CONNECTION_NOT_EXIST;
        }
        if (iter->second.token != token) {
            TAG_LOGE(AAFwkTag::APPKIT, "Connection owner mismatched");
            return AAFwk::ERR_INVALID_CALLER;
        }
        abilityConnection = iter->second.connection;
    }

    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(abilityConnection);
    if (ret == ERR_OK || ret == AAFwk::CONNECTION_NOT_EXIST) {
        RemoveConnectionRecord(connectionId);
    }
    return ret;
}

void ModularObjectConnectionManager::RemoveConnectionRecord(int64_t connectionId)
{
    std::lock_guard<std::mutex> guard(connectionMutex_);
    connectionRecords_.erase(connectionId);
}

} // namespace AbilityRuntime
} // namespace OHOS
