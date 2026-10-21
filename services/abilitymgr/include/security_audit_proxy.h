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

#ifndef OHOS_ABILITY_RUNTIME_SECURITY_AUDIT_PROXY_H
#define OHOS_ABILITY_RUNTIME_SECURITY_AUDIT_PROXY_H

#include "event_report.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AAFwk {
// Security audit module SA id. Aligned by the security audit module requirement.
// Placeholder until the security audit module publishes its SA id.
constexpr int32_t SECURITY_AUDIT_SERVICE_ID = -1;

/**
 * @interface ISecurityAudit
 * Caller-side interface descriptor. The authoritative interface is owned by the
 * security audit module (covered by another requirement). This local descriptor
 * keeps the ability_runtime caller skeleton compilable; ownership transfers to
 * the security audit module shared header once that requirement lands.
 */
class ISecurityAudit : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.ISecurityAudit");
    enum class SecurityAuditInterfaceCode {
        REPORT_INTERCEPT_EVENT = 1,
    };

    /**
     * Async report of an intercepted ability start event. Carries EventInfo
     * (including isPreload). Never throws; failures only logged. Safe to call
     * from the interception hot path. Fire-and-forget: caller does not wait.
     */
    virtual void ReportInterceptEventAsync(const EventInfo &eventInfo) = 0;

    static sptr<ISecurityAudit> GetInstance();
};

/**
 * @class SecurityAuditProxy
 * Caller-side proxy for the security audit module. Reports intercepted ability
 * start events asynchronously, in parallel with the Hiview channel, to bypass
 * the Hiview 20/day throttle (see design DD-1/DD-3, spec AC-1.1/AC-1.2).
 *
 * Async fire-and-forget: never blocks the interception decision, never throws
 * to the caller. Proxy acquisition failure (EX-1) and IPC remote failure (EX-2)
 * are logged via hilog (tag: AAFWK) and swallowed.
 */
class SecurityAuditProxy : public IRemoteProxy<ISecurityAudit> {
public:
    explicit SecurityAuditProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<ISecurityAudit>(impl) {}
    ~SecurityAuditProxy() = default;

    void ReportInterceptEventAsync(const EventInfo &eventInfo) override;

private:
    static inline BrokerDelegator<SecurityAuditProxy> delegator_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SECURITY_AUDIT_PROXY_H
