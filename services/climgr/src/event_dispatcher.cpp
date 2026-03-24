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

#include "event_dispatcher.h"
#include "io_monitor.h"
#include "cli_callback_proxy.h"
#include "hilog_tag_wrapper.h"
#include <chrono>

namespace OHOS {
namespace CliMgr {

void EventDispatcher::RegisterCallback(const std::string& taskId,
                                      const sptr<ICLICallback>& callback)
{
    if (!callback) {
        TAG_LOGE(TAG, "Null callback for task %{public}s", taskId.c_str());
        return;
    }

    std::lock_guard<std::mutex> lock(callbacksMutex_);
    callbacks_[taskId] = callback;

    TAG_LOGI(TAG, "Callback registered for task %{public}s", taskId.c_str());
}

void EventDispatcher::UnregisterCallback(const std::string& taskId)
{
    std::lock_guard<std::mutex> lock(callbacksMutex_);
    callbacks_.erase(taskId);

    TAG_LOGI(TAG, "Callback unregistered for task %{public}s", taskId.c_str());
}

void EventDispatcher::DispatchIOEvent(const IOEvent& event)
{
    sptr<ICLICallback> callback;
    {
        std::lock_guard<std::mutex> lock(callbacksMutex_);
        auto it = callbacks_.find(event.taskId);
        if (it != callbacks_.end()) {
            callback = it->second;
        }
    }

    if (!callback) {
        TAG_LOGW(TAG, "No callback for task %{public}s", event.taskId.c_str());
        return;
    }

    // 构建 ToolEvent
    ToolEvent toolEvent;
    toolEvent.type = (event.type == IOEventType::STDOUT) ?
                     "stdout" : "stderr";
    toolEvent.data = event.data;
    toolEvent.timestamp = std::chrono::system_clock::now()
                              .time_since_epoch()
                              .count();

    // 通过 IPC 发送事件到客户端
    callback->OnOutput(event.taskId, toolEvent);

    TAG_LOGD(TAG, "IO event dispatched: task=%{public}s, type=%{public}s, size=%{public}zu",
             event.taskId.c_str(), toolEvent.type.c_str(), event.data.size());
}

void EventDispatcher::DispatchExitEvent(const std::string& taskId, int exitCode)
{
    sptr<ICLICallback> callback;
    {
        std::lock_guard<std::mutex> lock(callbacksMutex_);
        auto it = callbacks_.find(taskId);
        if (it != callbacks_.end()) {
            callback = it->second;
        }
    }

    if (!callback) {
        TAG_LOGW(TAG, "No callback for task %{public}s", taskId.c_str());
        return;
    }

    // 发送退出事件
    callback->OnExit(taskId, exitCode);

    TAG_LOGI(TAG, "Exit event dispatched: task=%{public}s, code=%{public}d",
             taskId.c_str(), exitCode);

    // 退出后取消回调
    UnregisterCallback(taskId);
}

void EventDispatcher::DispatchErrorEvent(const std::string& taskId,
                                        int32_t errorCode,
                                        const std::string& errorMsg)
{
    sptr<ICLICallback> callback;
    {
        std::lock_guard<std::mutex> lock(callbacksMutex_);
        auto it = callbacks_.find(taskId);
        if (it != callbacks_.end()) {
            callback = it->second;
        }
    }

    if (!callback) {
        TAG_LOGW(TAG, "No callback for task %{public}s", taskId.c_str());
        return;
    }

    // 发送错误事件
    callback->OnError(taskId, errorCode, errorMsg);

    TAG_LOGE(TAG, "Error event dispatched: task=%{public}s, code=%{public}d, msg=%{public}s",
             taskId.c_str(), errorCode, errorMsg.c_str());
}

} // namespace CliMgr
} // namespace OHOS
