/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APP_CONFIGURATION_POLICY_MANAGER_H
#define OHOS_ABILITY_RUNTIME_APP_CONFIGURATION_POLICY_MANAGER_H

#include <string>
#include <shared_mutex>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "event_handler.h"

namespace OHOS {
namespace AppExecFwk {

class AppConfigurationPolicyManager : public std::enable_shared_from_this<AppConfigurationPolicyManager> {
public:
    AppConfigurationPolicyManager();
    ~AppConfigurationPolicyManager();

    void Init();
    void UnInit();
    void test();
private:
    void StartEvent();
    

private:
    std::thread eventThread_;
    std::condition_variable eventCon_;
    std::mutex eventMutex_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> eventHandler_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif //OHOS_ABILITY_RUNTIME_APP_CONFIGURATION_POLICY_MANAGER_H