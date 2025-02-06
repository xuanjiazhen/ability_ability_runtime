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
    std::string domain = record->GetDomain();
    std::string eventName = record->GetEventName();
    int64_t pid = record->GetPid();
    int64_t uid = record->GetUid();
    std::string accessToken;
    std::string packageName;
    if (record->GetParamValue("ACCESS_TOEKN", accessToken) || record->GetParamValue("PACKAGE_NAME", packageName)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get accessToken or packageName failed");
        return;
    }
    if (accessToken.empty() || packageName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "accessToken or packageName is invalid");
        return;
    }
    ExitReason exitReason = {Reason::REASON_APP_FREEZE, "Kill Reason: kernel kill"};
    AbilityManagerService::GetPubInstance()->RecordProcessExitReason(pid, exitReason);
    TAG_LOGE(AAFwkTag::ABILITYMGR, "record kernel kill reason, pid : %lld, uid : %lld, packageName : %{public}s",
        pid, uid, packageName.c_str());
}

void KernelKillListener::OnServiceDied()
{
    TAG_LOGE(AAFwkTag::ABILITYMGR, "service disconnect, exit");
}
}
}