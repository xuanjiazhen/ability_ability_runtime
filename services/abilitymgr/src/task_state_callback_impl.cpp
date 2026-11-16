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

#include "task_state_callback_impl.h"

#include "ability_manager_service.h"
#include "hilog_tag_wrapper.h"
#include "singleton.h"

namespace OHOS {
namespace AAFwk {
void TaskStateCallbackImpl::OnTaskFinished(const std::string &taskId, int32_t userId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "task finished callback, taskId=%{public}s, userId=%{public}d", taskId.c_str(), userId);
    DelayedSingleton<AbilityManagerService>::GetInstance()->OnTaskFinished(taskId, userId);
}
}  // namespace AAFwk
}  // namespace OHOS
