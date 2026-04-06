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

#ifndef OHOS_ABILITY_RUNTIME_JS_CLI_SUBSCRIPTION_MANAGER_H
#define OHOS_ABILITY_RUNTIME_JS_CLI_SUBSCRIPTION_MANAGER_H

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "cli_manager_data.h"
#include "js_cli_callback.h"

namespace OHOS {
namespace AbilityRuntime {

class CliSessionSubscriptionManager final {
public:
    static CliSessionSubscriptionManager &GetInstance();

    std::string AddProvisionalSubscription(napi_env env, const std::string &sessionId, napi_value jsCallback);
    void ActivateSubscription(const std::string &subscriptionId);
    void RemoveSubscription(const std::string &subscriptionId);
    void ClearAllSubscriptions();
    int32_t HandleSessionEvent(const std::string &sessionId, const std::string &subscriptionId, const ToolEvent &event);

private:
    struct SubscriptionRecord {
        std::string sessionId;
        std::shared_ptr<CliCallbackNapi> callback;
        bool isActive = false;
        std::vector<ToolEvent> pendingEvents;
    };

    CliSessionSubscriptionManager() = default;
    ~CliSessionSubscriptionManager() = default;

    void EnsureInitialized();
    std::string GenerateSubscriptionId(const std::string &sessionId);

    std::mutex mutex_;
    std::unordered_map<std::string, SubscriptionRecord> subscriptions_;
    std::atomic<uint64_t> nextSubscriptionId_ {0};
    bool initialized_ = false;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_JS_CLI_SUBSCRIPTION_MANAGER_H
