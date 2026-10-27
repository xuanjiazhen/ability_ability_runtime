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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_AUTHORIZATION_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_AUTHORIZATION_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

namespace OHOS {
namespace AbilityRuntime {

/**
 * DRAFT(SPEC-20260816098661): 三方 Agent 执行意图的授权编排骨架。
 *
 * 设计约束（见 design.md ADR-1 / ADR-2 / ADR-4）：
 * - 编排逻辑落在 ability_runtime 侧，授权记录（写入/查询/生效周期）与弹框 UI 由安全子系统
 *   （ATM + 安全隐私中心）实现，本层只做调用与判定编排；
 * - 任何非确定状态（身份校验失败、ATM 查询异常、弹框未确认）一律拒绝，即 fail-closed；
 * - 外部依赖（O-1 ATM 接口 / O-2 弹框接口 / O-5 篡改校验接口）尚未定义，因此以可注入
 *   backend 的形式解耦：接口就绪后只需替换 backend 实现，编排与测试不受影响；
 * - 骨架阶段默认 backend 全部拒绝，保证「未接入即不可用」，不产生任何放行。
 */
enum class AuthorizationResult {
    DENIED = 0,
    APPROVED,
    PENDING,
};

struct CallerIdentity {
    int32_t callerUid = -1;
    uint32_t callerTokenId = 0;
    std::string callerBundleName;
    std::string targetBundleName;
};

/**
 * 外部依赖的抽象入口，对应 design.md「接口参数规约」中的三个接口。
 */
class IInsightIntentAuthorizationBackend {
public:
    virtual ~IInsightIntentAuthorizationBackend() = default;

    // O-5 篡改校验：弹框拟展示的调用方/被调用方身份需与实际 IPC 对端、目标提供方一致
    virtual bool VerifyIdentity(const CallerIdentity &identity) = 0;

    // O-1 ATM 授权查询：生效周期内是否已有「调用方 × 意图提供方」的有效授权
    virtual bool QueryAuthorization(const CallerIdentity &identity) = 0;

    // O-2 弹框触发：返回用户决定；异步语义（O-3）未定期间按「未确认」处理
    virtual AuthorizationResult RequestUserAuthorization(const CallerIdentity &identity) = 0;

    /**
     * DRAFT(SPEC-20260816098661): 设备形态门槛由 backend 提供，编排层不直接依赖设备参数。
     * 默认不支持（fail-closed）；真实实现按 GetDeviceType() 判 2in1/tablet（AC-1.3）。
     */
    virtual bool IsPopupSupported() const
    {
        return false;
    }
};

class InsightIntentAuthorization {
public:
    static InsightIntentAuthorization &GetInstance();

    /**
     * 设备形态门槛：弹框授权仅在 PC/平板生效，手机维持 ERR_NOT_SYSTEM_APP（AC-1.3）。
     */
    static bool IsPopupAuthorizationSupported();

    /**
     * 三方调用方的授权编排入口，调用顺序固定为：身份校验 → 授权查询 → 弹框。
     * 返回 APPROVED 才允许执行；其余一律拒绝（fail-closed）。
     */
    AuthorizationResult AuthorizeThirdPartyCall(const CallerIdentity &identity);

    /**
     * 仅用于测试注入 mock backend。
     */
    void SetBackendForTest(std::shared_ptr<IInsightIntentAuthorizationBackend> backend);

private:
    InsightIntentAuthorization();

    bool IsPopupSupportedByBackend();

    std::mutex mutex_;
    std::shared_ptr<IInsightIntentAuthorizationBackend> backend_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_AUTHORIZATION_H
