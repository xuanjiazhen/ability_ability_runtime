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

#include "embeddable_privacy_modal_callback.h"
#include "ui_extension_context.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* EMBEDDABLE_PRIVACY_EXIT = "ohos.param.exitEmbeddablePrivacyUIExtension";
}

void EmbeddablePrivacyModalCallback::OnRelease()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "EmbeddablePrivacyModalCallback::OnRelease, sessionId: %{public}d", sessionId_);

    auto context = contextWeak_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Context already destroyed in OnRelease");
        return;
    }

    context->EraseUIExtension(sessionId_);
    TAG_LOGI(AAFwkTag::UI_EXT, "Erased embeddable privacy modal on release: %{public}d", sessionId_);

#ifdef SUPPORT_SCREEN
    if (uiContent_ != nullptr) {
        uiContent_->CloseModalUIExtension(sessionId_);
        TAG_LOGI(AAFwkTag::UI_EXT, "Closed embeddable privacy modal: %{public}d", sessionId_);
    } else {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiContent_ in OnRelease");
    }
#endif // SUPPORT_SCREEN
}

void EmbeddablePrivacyModalCallback::OnError()
{
    TAG_LOGE(AAFwkTag::UI_EXT, "EmbeddablePrivacyModalCallback::OnError, sessionId: %{public}d", sessionId_);

    auto context = contextWeak_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Context already destroyed in OnError");
        return;
    }

    context->EraseUIExtension(sessionId_);

#ifdef SUPPORT_SCREEN
    if (uiContent_ != nullptr) {
        uiContent_->CloseModalUIExtension(sessionId_);
        TAG_LOGI(AAFwkTag::UI_EXT, "Closed embeddable privacy modal on error: %{public}d", sessionId_);
    } else {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiContent_ in OnError");
    }
#endif // SUPPORT_SCREEN
}

void EmbeddablePrivacyModalCallback::OnDestroy()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "EmbeddablePrivacyModalCallback::OnDestroy, sessionId: %{public}d", sessionId_);

    auto context = contextWeak_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Context already destroyed in OnDestroy");
        return;
    }

    context->EraseUIExtension(sessionId_);
    TAG_LOGI(AAFwkTag::UI_EXT, "Erased embeddable privacy modal on destroy: %{public}d", sessionId_);
}

void EmbeddablePrivacyModalCallback::OnReceive(const AAFwk::WantParams& data)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "EmbeddablePrivacyModalCallback::OnReceive, sessionId: %{public}d", sessionId_);

    auto context = contextWeak_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Context already destroyed in OnReceive");
        return;
    }

    if (data.HasParam(EMBEDDABLE_PRIVACY_EXIT)) {
        bool shouldExit = data.GetIntParam(EMBEDDABLE_PRIVACY_EXIT, 0);
        if (shouldExit == 1) {
            TAG_LOGI(AAFwkTag::UI_EXT,
                "Embeddable privacy modal notified exit, sessionId: %{public}d", sessionId_);
            context->TerminateSelfWithAnimation(nullptr);
            return;
        }
    }
}

} // namespace AbilityRuntime
} // namespace OHOS
