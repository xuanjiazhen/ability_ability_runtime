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

#include "insight_intent_will_execute_callback_mgr.h"

#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
std::mutex InsightIntentWillExecuteCallbackMgr::instanceMutex_;
sptr<InsightIntentWillExecuteCallbackMgr> InsightIntentWillExecuteCallbackMgr::instance_ = nullptr;

sptr<InsightIntentWillExecuteCallbackMgr> InsightIntentWillExecuteCallbackMgr::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(instanceMutex_);
        if (instance_ == nullptr) {
            instance_ = sptr<InsightIntentWillExecuteCallbackMgr>::MakeSptr();
        }
    }
    return instance_;
}

int32_t InsightIntentWillExecuteCallbackMgr::Register(
    const std::function<void(const InsightIntentExecutionInfo &)> &callback)
{
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callback_ = callback;
    }
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->OnWillInsightIntentExecute(AsObject());
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "register will execute callback failed, ret:%{public}d", ret);
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callback_ = nullptr;
        return ret;
    }
    return ERR_OK;
}

int32_t InsightIntentWillExecuteCallbackMgr::Unregister()
{
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callback_ = nullptr;
    }
    return AAFwk::AbilityManagerClient::GetInstance()->OffWillInsightIntentExecute(AsObject());
}

void InsightIntentWillExecuteCallbackMgr::OnWillInsightIntentExecute(const InsightIntentExecutionInfo &info)
{
    std::function<void(const InsightIntentExecutionInfo &)> callback;
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callback = callback_;
    }
    if (callback != nullptr) {
        callback(info);
    }
}
} // namespace AbilityRuntime
} // namespace OHOS