/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "auto_fill_task_handler_wrap.h"

#include <cinttypes>
#include <mutex>
#include "cpp/mutex.h"
#include "hilog_tag_wrapper.h"
#include "ffrt_task_utils_wrap.h"
#include "queue_task_handler_wrap.h"
#include "auto_fill_ffrt_task_handler_wrap.h"

namespace OHOS {
namespace AAFwk {

constexpr int MILL_TO_MICRO = 1000;
bool AutoFillTaskHandle::Cancel() const
{
    auto handler = handler_.lock();
    if (!handler || !innerTaskHandle_) {
        return false;
    }
    auto ret = handler->CancelTaskInner(innerTaskHandle_);
    return ret;
}

std::atomic_int32_t AutoFillTaskHandlerWrap::g_taskId = 0;
std::shared_ptr<AutoFillTaskHandlerWrap> AutoFillTaskHandlerWrap::GetFfrtHandler()
{
    static auto ffrtHandler = std::make_shared<AutoFillFfrtTaskHandlerWrap>();
    return ffrtHandler;
}

AutoFillTaskHandle AutoFillTaskHandlerWrap::SubmitTask(const std::function<void()> &task,
    const std::string &name, int64_t &delayMillis)
{
    std::lock_guard<ffrt::mutex> guard(*tasksMutex_);
    if (!task) {
        return AutoFillTaskHandle();
    }
    auto it = tasks_.find(name);
    if (it != tasks_.end()) {
        TAG_LOGE(AAFwkTag::DEFAULT, "SubmitTask repeated task: %{public}s", name.c_str());
        return AutoFillTaskHandle();
    }

    AutoFillTaskHandle result(shared_from_this(), nullptr);
    result.taskId_ = ++g_taskId;
    auto taskWrap = [result, task, name]() {
        if (result.PrintTaskLog()) {
            TAG_LOGW(AAFwkTag::DEFAULT, "begin execute task name: %{public}s, taskId: %{public}d",
                name.c_str(), result.GetTaskId());
        }
        task();
    };

    if (printTaskLog_) {
        TAG_LOGW(AAFwkTag::DEFAULT, "submitTask: %{public}s, taskId: %{public}d, count: "
            "%{public}" PRIu64"", taskAttr.taskName_.c_str(), result.taskId_, g_taskId);
    }
    result.innerTaskHandle_ = SubmitTaskInner(std::move(taskWrap), name, delayMillis);
    tasks_.emplace(name, result);

    return result;
}

AutoFillTaskHandlerWrap::AutoFillTaskHandlerWrap()
{
    tasksMutex_ = std::make_unique<ffrt::mutex>();
}

bool AutoFillTaskHandlerWrap::CancelTask(const std::string &name)
{
    TAG_LOGD(AAFwkTag::DEFAULT, "CancelTask task: %{public}s", name.c_str());
    std::lock_guard<ffrt::mutex> guard(*tasksMutex_);
    auto it = tasks_.find(name);
    if (it == tasks_.end()) {
        return false;
    }

    auto taskHandle = it->second;
    tasks_.erase(it);
    if (printTaskLog_) {
        TAG_LOGW(AAFwkTag::DEFAULT, "cancel task name: %{public}s, taskId: %{public}d, queueName: %{public}s",
            name.c_str(), taskHandle.taskId_, queueName_.c_str());
    }
    return taskHandle.Cancel();
}

}  // namespace AAFWK
}  // namespace OHOS