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

#include "ability_manager_service.h"
#include "hilog_tag_wrapper.h"
#include "exit_reason.h"

namespace OHOS {
namespace AAFwk {
void KernelKillListener::RegisterKernelKillListener()
{
    auto kernelKillListener = std::make_shared<KernelKillListener>();
    HiviewDFX::ListenerRule domainNameRule("KERNEL_VENDOR", "PROCESS_KILL", HiviewDFX::RuleType::WHOLE_WORD);
    std::vector<HiviewDFX::ListenerRule> listenerRules;
    listenerRules.push_back(domainNameRule);
    auto ret = HiviewDFX::HiSysEventManager::AddListener(kernelKillListener, listenerRules);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "add kernelKill listener failed: %{public}d", ret);
    }
}

void KernelKillListener::OnEvent(std::shared_ptr<HiviewDFX::HiSysEventRecord> record)
{
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "record null");
        return;
    }

    std::string eventName = record->GetEventName();
    int32_t pid = record->GetPid();
    int32_t uid = record->GetUid();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "kernel kill, pid: %{public}d, uid: %{public}d, eventName : %{public}s",
        pid, uid, record->GetEventName().c_str());
    ExitReason exitReason = {Reason::REASON_KERNEL_KILL, "Kill Reason: kernel kill"};
    AbilityManagerService::GetPubInstance()->RecordProcessExitReason(pid, exitReason);
}

void KernelKillListener::OnServiceDied()
{
    TAG_LOGW(AAFwkTag::ABILITYMGR, "service disconnect, exit");
}
}
}