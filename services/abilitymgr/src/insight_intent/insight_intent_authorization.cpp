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

#include "insight_intent_authorization.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
/**
 * DRAFT(SPEC-20260816098661): 骨架阶段的默认 backend，所有入口一律拒绝（fail-closed）。
 * O-1 / O-2 / O-5 接口定义关闭后，用真实对接实现替换本类即可，编排层无需改动。
 */
class DenyAllAuthorizationBackend : public IInsightIntentAuthorizationBackend {
public:
    ~DenyAllAuthorizationBackend() override = default;

    bool VerifyIdentity(const CallerIdentity &identity) override
    {
        TAG_LOGW(AAFwkTag::INTENT, "authorization backend not ready, identity check denied, "
            "callerUid: %{public}d, target: %{public}s",
            identity.callerUid, identity.targetBundleName.c_str());
        return false;
    }

    bool QueryAuthorization(const CallerIdentity &identity) override
    {
        TAG_LOGW(AAFwkTag::INTENT, "authorization backend not ready, query denied, caller: %{public}s, "
            "target: %{public}s", identity.callerBundleName.c_str(), identity.targetBundleName.c_str());
        return false;
    }

    AuthorizationResult RequestUserAuthorization(const CallerIdentity &identity) override
    {
        TAG_LOGW(AAFwkTag::INTENT, "authorization backend not ready, popup skipped, caller: %{public}s, "
            "target: %{public}s", identity.callerBundleName.c_str(), identity.targetBundleName.c_str());
        return AuthorizationResult::DENIED;
    }

    bool IsPopupSupported() const override
    {
        // DRAFT(SPEC-20260816098661): 真实实现按 GetDeviceType() 判 2in1/tablet 后才返回 true。
        return false;
    }
};
} // namespace

InsightIntentAuthorization::InsightIntentAuthorization()
    : backend_(std::make_shared<DenyAllAuthorizationBackend>())
{}

InsightIntentAuthorization &InsightIntentAuthorization::GetInstance()
{
    static InsightIntentAuthorization instance;
    return instance;
}

bool InsightIntentAuthorization::IsPopupAuthorizationSupported()
{
    // SPEC-20260816098661 R-4：设备形态判断在权限检查入口完成，手机短路返回 ERR_NOT_SYSTEM_APP。
    // 智慧屏等其他形态不在本需求范围，默认不启用弹框授权（维持拒绝）。
    bool supported = GetInstance().IsPopupSupportedByBackend();
    TAG_LOGD(AAFwkTag::INTENT, "popup authorization supported: %{public}d", static_cast<int32_t>(supported));
    return supported;
}

bool InsightIntentAuthorization::IsPopupSupportedByBackend()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return (backend_ != nullptr) && backend_->IsPopupSupported();
}

AuthorizationResult InsightIntentAuthorization::AuthorizeThirdPartyCall(const CallerIdentity &identity)
{
    std::shared_ptr<IInsightIntentAuthorizationBackend> backend = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        backend = backend_;
    }
    if (backend == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "authorization backend is null, deny by default");
        return AuthorizationResult::DENIED;
    }

    // 步骤 2：篡改校验（O-5）。弹框展示的身份必须与实际调用方/提供方一致，不一致即中止（AC-3.1）。
    if (!backend->VerifyIdentity(identity)) {
        TAG_LOGE(AAFwkTag::INTENT, "identity verification failed, abort authorization");
        return AuthorizationResult::DENIED;
    }

    // 步骤 3：ATM 授权查询（O-1）。生效周期内命中有效授权则免弹框直接执行（AC-2.1）。
    if (backend->QueryAuthorization(identity)) {
        TAG_LOGI(AAFwkTag::INTENT, "valid authorization found, skip popup, caller: %{public}s, target: %{public}s",
            identity.callerBundleName.c_str(), identity.targetBundleName.c_str());
        return AuthorizationResult::APPROVED;
    }

    // 步骤 4：无有效授权，触发弹框（O-2）。用户拒绝/未确认一律拒绝（AC-1.2）。
    AuthorizationResult result = backend->RequestUserAuthorization(identity);
    if (result != AuthorizationResult::APPROVED) {
        TAG_LOGE(AAFwkTag::INTENT, "user authorization not granted, caller: %{public}s, target: %{public}s",
            identity.callerBundleName.c_str(), identity.targetBundleName.c_str());
        return AuthorizationResult::DENIED;
    }
    TAG_LOGI(AAFwkTag::INTENT, "user authorized, caller: %{public}s, target: %{public}s",
        identity.callerBundleName.c_str(), identity.targetBundleName.c_str());
    return AuthorizationResult::APPROVED;
}

void InsightIntentAuthorization::SetBackendForTest(
    std::shared_ptr<IInsightIntentAuthorizationBackend> backend)
{
    std::lock_guard<std::mutex> lock(mutex_);
    backend_ = backend;
}
} // namespace AbilityRuntime
} // namespace OHOS
