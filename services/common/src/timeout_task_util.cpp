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

#include "timeout_task_util.h"

#include <memory>
#include <mutex>
#include "ffrt.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
bool TimeoutTaskUtil::PostSyncTaskWithTimeout(std::function<void(void)> &&task, int32_t timeoutMillis,
    const std::string &taskName)
{
    if (!task || timeoutMillis <= 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid param name:%{public}s, %{public}d", taskName.c_str(), timeoutMillis);
        return false;
    }

    auto mutexPtr = std::make_shared<ffrt::mutex>();
    auto conditionPtr = std::make_shared<ffrt::condition_variable>();
    std::unique_lock lock(*mutexPtr);
    ffrt::submit([mutexPtr, conditionPtr, taskCopy = std::move(task)]() {
        std::lock_guard lock(*mutexPtr);
        taskCopy();
        conditionPtr->notify_one();
        }, ffrt::task_attr().name(taskName.c_str()));

    if (conditionPtr->wait_for(lock, std::chrono::milliseconds(timeoutMillis)) == ffrt::cv_status::timeout) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "task timeout name:%{public}s, %{public}d", taskName.c_str(), timeoutMillis);
        return false;
    }
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS