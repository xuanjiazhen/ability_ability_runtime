/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "auto_fill_ffrt_task_handler_wrap.h"

namespace OHOS {
namespace AAFwk {

std::shared_ptr<AutoFillInnerTaskHandle> AutoFillFfrtTaskHandlerWrap::SubmitTaskInner(std::function<void()> &&task,
    const std::string &taskName, const int64_t &delayMillis)
{
    ffrt::task_attr ffrtTaskAttr;
    if (delayMillis > 0) {
        ffrtTaskAttr.delay(delayMillis * MILL_TO_MICRO);
    }
    if (!taskName.empty()) {
        ffrtTaskAttr.name(taskName.c_str());
    }

    return std::make_shared<AutoFillInnerTaskHandle>(ffrt::submit_h(std::move(task),
        {}, {}, ffrtTaskAttr));
}

bool AutoFillFfrtTaskHandlerWrap::CancelTaskInner(const std::shared_ptr<AutoFillInnerTaskHandle> &taskHandle)
{
    if (!taskHandle) {
        return false;
    }
    return ffrt::skip(taskHandle->GetFfrtHandle()) == 0;
}

void AutoFillFfrtTaskHandlerWrap::WaitTaskInner(const std::shared_ptr<AutoFillInnerTaskHandle> &taskHandle)
{
    if (!taskHandle) {
        return;
    }
    ffrt::wait({taskHandle->GetFfrtHandle()});
}
} // namespace AAFwk
} // namespace OHOS