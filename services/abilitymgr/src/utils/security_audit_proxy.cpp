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

#include "security_audit_proxy.h"

#include "ffrt.h"
#include "hilog_tag_wrapper.h"
#include "iservice_registry.h"
#include "message_parcel.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t FFRT_TASK_TIMEOUT = 5 * 1000 * 1000; // 5s, aligned with ability_event_util.cpp:29

void WriteEventInfoToParcel(MessageParcel &data, const EventInfo &eventInfo)
{
    data.WriteString(eventInfo.bundleName);
    data.WriteString(eventInfo.abilityName);
    data.WriteString(eventInfo.processName);
    data.WriteInt32(eventInfo.userId);
    data.WriteInt32(eventInfo.errCode);
    data.WriteString(eventInfo.errMsg);
    data.WriteBool(eventInfo.isPreload);
    data.WriteInt32(eventInfo.preloadMode);
}

void DoReportInterceptEvent(const sptr<IRemoteObject> &remote, const EventInfo &eventInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    data.WriteInterfaceToken(ISecurityAudit::GetDescriptor());
    WriteEventInfoToParcel(data, eventInfo);
    int32_t error = remote->SendRequest(
        static_cast<uint32_t>(ISecurityAudit::SecurityAuditInterfaceCode::REPORT_INTERCEPT_EVENT),
        data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "security audit report ipc fail, error:%{public}d", error);
    }
}
} // namespace

void SecurityAuditProxy::ReportInterceptEventAsync(const EventInfo &eventInfo)
{
    // Async fire-and-forget: capture sptr<IRemoteObject> to keep the remote alive
    // across the ffrt task; never throws, never blocks the interception decision.
    // EX-1 (service unavailable) and EX-2 (ipc fail) only logged, per spec AC-1.2.
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "security audit remote unavailable, skip report");
        return;
    }
    ffrt::submit([remote, eventInfo]() {
        DoReportInterceptEvent(remote, eventInfo);
        }, ffrt::task_attr().timeout(FFRT_TASK_TIMEOUT));
}

sptr<ISecurityAudit> ISecurityAudit::GetInstance()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "security audit samgr unavailable (EX-1)");
        return nullptr;
    }
    auto remote = samgr->GetSystemAbility(SECURITY_AUDIT_SERVICE_ID);
    if (remote == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "security audit service unavailable (EX-1)");
        return nullptr;
    }
    return iface_cast<ISecurityAudit>(remote);
}
} // namespace AAFwk
} // namespace OHOS
