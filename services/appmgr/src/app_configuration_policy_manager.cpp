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

#include "app_configuration_policy_manager.h"

#include <sys/prctl.h>

#include "single_instance.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string SOCK_NAME = "AppConfigurationPolicyManager";
}

AppConfigurationPolicyManager::AppConfigurationPolicyManager()
{
}

AppConfigurationPolicyManager::~AppConfigurationPolicyManager()
{
}

void AppConfigurationPolicyManager::Init()
{
    TAG_LOGI(AAFwkTag::APPMGR, "Init AppConfigurationPolicyManager start");
    if (eventHandler_ != nullptr) {
        TAG_LOGI(AAFwkTag::APPMGR, "AppConfigurationPolicyManager already inited, end.");
        return;
    }
    eventThread_ = std::thread(&AppConfigurationPolicyManager::StartEvent, this);
    std::unique_lock<std::mutex> lock(eventMutex_);
    eventCon_.wait(lock, [this] {
        return eventHandler_ != nullptr;
    });
    TAG_LOGI(AAFwkTag::APPMGR, "Init AppConfigurationPolicyManager end");
}

void AppConfigurationPolicyManager::StartEvent()
{
    TAG_LOGI(AAFwkTag::APPMGR, "StartEvent start");
    prctl(PR_SET_NAME, SOCK_NAME.c_str());
    auto runner = AppExecFwk::EventRunner::Create(false);
    {
        std::lock_guard<std::mutex> lock(eventMutex_);
        eventHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    }
    eventCon_.notify_one();
    std::thread::id threadId = eventThread_.get_id();
    std::thread::id threadId1 = std::this_thread::get_id();
    runner->Run();
    TAG_LOGI(AAFwkTag::APPMGR, "StartEvent end");
}

void AppConfigurationPolicyManager::test()
{
    TAG_LOGI(AAFwkTag::APPMGR, "test start........");
    auto func = [this]() {
        TAG_LOGI(AAFwkTag::APPMGR, "func start........");
    };
    eventHandler_->PostTask(func, AppExecFwk::EventQueue::Priority::LOW);
}

void AppConfigurationPolicyManager::UnInit()
{
    TAG_LOGI(AAFwkTag::APPMGR, "UnInit start");
    if (eventHandler_ != nullptr) {
        eventHandler_->GetEventRunner()->Stop();
        if (eventThread_.joinable()) {
            eventThread_.join();
        }
        eventHandler_ = nullptr;
    } else {
        TAG_LOGE(AAFwkTag::APPMGR, "eventHandler_ is nullptr");
    }
    TAG_LOGI(AAFwkTag::APPMGR, "UnInit end");
}

}  // namespace DistributedCollab
}  // namespace OHOS