/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_ABILITY_RUNTIME_SECURITY_AUDIT_REPORT_H
#define OHOS_ABILITY_RUNTIME_SECURITY_AUDIT_REPORT_H

namespace OHOS {
namespace AAFwk {
struct AbilityInterceptorParam;
// 安全审计上报封装（需求 20260928009236 / TASK-1）。
//
// 将 EDM（企业设备管理）拦截应用启动事件经 security_guard 的
// NativeDataCollectKit::ReportSecurityInfoAsync 异步上报，与 Hiview 通道并行，绕开 Hiview 20条/天限流
// （design DD-1 / AC-1.1）。远端接口由 security_guard 仓提供（sg_collect_client.h），本类仅做
// AbilityInterceptorParam → SecurityGuard EventInfo 的转换与上报。
// ReportSecurityInfoAsync 本身为异步 fire-and-forget，失败仅 hilog，不阻塞主流程、不返回错误码
// 给调用方（EX-1/EX-2 / BR-2）。
//
// 特性开关：SECURITY_GUARDE_ENABLE（由 ability_runtime.gni 的 security_guard_enabled 决定）。
// 未启用时 ReportEdmBlockedStart 为空实现。
// 事件 ID：EDM_BLOCKED_START_EVENT_ID（0x18000104）。
class SecurityAuditReport {
public:
    explicit SecurityAuditReport() = default;
    virtual ~SecurityAuditReport() = default;

    // 异步上报 EDM 拦截应用启动事件。
    void ReportEdmBlockedStart(const AbilityInterceptorParam &param);
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SECURITY_AUDIT_REPORT_H
