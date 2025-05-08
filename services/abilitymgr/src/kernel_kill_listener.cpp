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

#include "kernel_kill_listener.h"

#include "hilog_tag_wrapper.h"
#include "exit_reason.h"

namespace OHOS {
namespace AAFwk {
void KernelKillListener::OnEvent(std::shared_ptr<HiviewDFX::HiSysEventRecord> record)
{
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "record is null");
        return;
    }
    int32_t pid = static_cast<int32_t>(record->GetPid());
    int32_t uid = static_cast<int32_t>(record->GetUid());
    ExitReason exitReason = {Reason::REASON_KERNEL_KILL, "Kill Reason: kernel kill"};
    std::string packageName;
    record->GetParamValue("PACKAGE_NAME", packageName);
    int32_t ret = AbilityManagerService::GetPubInstance()->RecordProcessExitReason(pid, exitReason);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "record kernel kill reason, pid=%{public}d, uid=%{public}d, packageName="
        "%{public}s, ret=%{public}d", pid, uid, packageName.c_str(), ret);
}

void KernelKillListener::OnServiceDied()
{
    TAG_LOGE(AAFwkTag::ABILITYMGR, "service disconnect, exit");
}
}
}