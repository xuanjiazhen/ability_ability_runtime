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

#include "remote_exec_callback_manager.h"

#include <chrono>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

IMPLEMENT_SINGLE_INSTANCE(RemoteExecCallbackManager);

void RemoteExecCallbackManager::AddCallback(const std::string &sessionId, const std::string &eventId,
    const sptr<ICliToolManagerScheduler> &scheduler)
{
    if (sessionId.empty() || scheduler == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "AddCallback: invalid params");
        return;
    }

    auto info = std::make_shared<RemoteExecCallbackInfo>();
    info->sessionId = sessionId;
    info->eventId = eventId;
    info->scheduler = scheduler;
    info->createTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_[sessionId] = info;
    TAG_LOGI(AAFwkTag::CLI_TOOL, "AddCallback: sessionId=%{public}s, eventId=%{public}s",
        sessionId.c_str(), eventId.c_str());
}

std::shared_ptr<RemoteExecCallbackInfo> RemoteExecCallbackManager::GetCallback(const std::string &sessionId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = callbacks_.find(sessionId);
    if (it != callbacks_.end()) {
        return it->second;
    }
    return nullptr;
}

void RemoteExecCallbackManager::RemoveCallback(const std::string &sessionId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.erase(sessionId);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "RemoveCallback: sessionId=%{public}s", sessionId.c_str());
}

void RemoteExecCallbackManager::HandleRemoteExecResult(const std::string &sessionId, int32_t resultCode,
    const CliSessionInfo &session)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "HandleRemoteExecResult: sessionId=%{public}s, resultCode=%{public}d",
        sessionId.c_str(), resultCode);

    std::shared_ptr<RemoteExecCallbackInfo> callbackInfo;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = callbacks_.find(sessionId);
        if (it != callbacks_.end()) {
            callbackInfo = it->second;
            callbacks_.erase(it);
        }
    }

    if (callbackInfo == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "HandleRemoteExecResult: callback not found for sessionId=%{public}s",
            sessionId.c_str());
        return;
    }

    if (callbackInfo->scheduler == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "HandleRemoteExecResult: scheduler is null");
        return;
    }

    // 回调发起端
    callbackInfo->scheduler->SchedulerExecToolReplyEvent(callbackInfo->eventId, resultCode, session);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "HandleRemoteExecResult: callback sent, eventId=%{public}s",
        callbackInfo->eventId.c_str());
}

} // namespace CliTool
} // namespace OHOS