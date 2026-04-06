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

#include "js_cli_subscription_manager.h"

#include "cli_manager_client.h"

namespace OHOS {
namespace AbilityRuntime {

namespace {
constexpr int32_t ERROR_CODE = -1;
constexpr int32_t ERR_OK = 0;
}

CliSessionSubscriptionManager &CliSessionSubscriptionManager::GetInstance()
{
    static CliSessionSubscriptionManager instance;
    return instance;
}

std::string CliSessionSubscriptionManager::AddProvisionalSubscription(napi_env env, const std::string &sessionId,
    napi_value jsCallback)
{
    if (env == nullptr || jsCallback == nullptr || sessionId.empty()) {
        return "";
    }

    auto callback = std::make_shared<CliCallbackNapi>(env, jsCallback);
    if (callback == nullptr || !callback->IsValid()) {
        return "";
    }

    EnsureInitialized();
    std::string subscriptionId = GenerateSubscriptionId(sessionId);

    std::lock_guard<std::mutex> lock(mutex_);
    subscriptions_[subscriptionId] = SubscriptionRecord {sessionId, callback, false};
    return subscriptionId;
}

void CliSessionSubscriptionManager::ActivateSubscription(const std::string &subscriptionId)
{
    std::shared_ptr<CliCallbackNapi> callback;
    std::vector<ToolEvent> pendingEvents;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subscriptions_.find(subscriptionId);
        if (it == subscriptions_.end()) {
            return;
        }
        it->second.isActive = true;
        callback = it->second.callback;
        pendingEvents.swap(it->second.pendingEvents);
    }

    if (callback == nullptr) {
        RemoveSubscription(subscriptionId);
        return;
    }

    bool sawExit = false;
    for (const auto &event : pendingEvents) {
        callback->DispatchEvent(event);
        if (event.type == "exit") {
            sawExit = true;
        }
    }

    if (sawExit) {
        RemoveSubscription(subscriptionId);
    }
}

void CliSessionSubscriptionManager::RemoveSubscription(const std::string &subscriptionId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    subscriptions_.erase(subscriptionId);
}

void CliSessionSubscriptionManager::ClearAllSubscriptions()
{
    std::lock_guard<std::mutex> lock(mutex_);
    subscriptions_.clear();
}

int32_t CliSessionSubscriptionManager::HandleSessionEvent(const std::string &sessionId, const std::string &subscriptionId,
    const ToolEvent &event)
{
    std::shared_ptr<CliCallbackNapi> callback;
    bool shouldDispatch = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subscriptions_.find(subscriptionId);
        if (it == subscriptions_.end() || it->second.sessionId != sessionId) {
            return ERROR_CODE;
        }
        if (!it->second.isActive) {
            it->second.pendingEvents.push_back(event);
            return ERR_OK;
        }
        callback = it->second.callback;
        shouldDispatch = true;
        if (event.type == "exit") {
            subscriptions_.erase(it);
        }
    }

    if (!shouldDispatch || callback == nullptr) {
        return ERROR_CODE;
    }
    return callback->DispatchEvent(event);
}

void CliSessionSubscriptionManager::EnsureInitialized()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        return;
    }

    CliManagerClient::GetInstance().SetSessionEventHandler(
        [this](const std::string &sessionId, const std::string &subscriptionId, const ToolEvent &event) {
            HandleSessionEvent(sessionId, subscriptionId, event);
        });
    CliManagerClient::GetInstance().SetServiceDeathHandler([this]() {
        ClearAllSubscriptions();
    });
    initialized_ = true;
}

std::string CliSessionSubscriptionManager::GenerateSubscriptionId(const std::string &sessionId)
{
    uint64_t suffix = nextSubscriptionId_.fetch_add(1, std::memory_order_relaxed) + 1;
    return sessionId + "-" + std::to_string(suffix);
}

} // namespace AbilityRuntime
} // namespace OHOS
