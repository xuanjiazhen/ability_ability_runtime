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

#ifndef OHOS_ABILITY_RUNTIME_EMBEDDABLE_PRIVACY_MODAL_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_EMBEDDABLE_PRIVACY_MODAL_CALLBACK_H

#include "hilog_tag_wrapper.h"
#include <memory>

#ifdef SUPPORT_SCREEN
#include "ui_content.h"
#endif // SUPPORT_SCREEN

namespace OHOS {
namespace AbilityRuntime {

class UIExtensionContext;

/**
 * @brief Callback class for embeddable privacy modal UIExtension lifecycle management.
 *
 * Independent from UIExtensionModalCallback. Handles the new exit key
 * "ohos.param.exitEmbeddablePrivacyUIExtension" for the new embeddable privacy modal path
 * (EmbeddableUIAbility privacy dialog modal-app experience).
 */
class EmbeddablePrivacyModalCallback {
public:
    EmbeddablePrivacyModalCallback() = default;
    ~EmbeddablePrivacyModalCallback() = default;

    /**
     * @brief Set the session ID for this embeddable privacy modal UIExtension.
     * @param sessionId The session ID.
     */
    void SetSessionId(int32_t sessionId) { sessionId_ = sessionId; }

#ifdef SUPPORT_SCREEN
    /**
     * @brief Set the UIContent for this embeddable privacy modal UIExtension.
     * @param uiContent Pointer to the UIContent.
     */
    void SetUIContent(Ace::UIContent* uiContent) { uiContent_ = uiContent; }
#endif // SUPPORT_SCREEN

    /**
     * @brief Set the UIExtensionContext for this embeddable privacy modal UIExtension.
     * @param context Weak pointer to the UIExtensionContext.
     */
    void SetUIExtensionContext(const std::weak_ptr<UIExtensionContext>& context) { contextWeak_ = context; }

    /**
     * @brief Called when the embeddable privacy modal UIExtension is released.
     */
    void OnRelease();

    /**
     * @brief Called when an error occurs in the embeddable privacy modal UIExtension.
     */
    void OnError();

    /**
     * @brief Called when the embeddable privacy modal UIExtension is destroyed.
     */
    void OnDestroy();

    /**
     * @brief Called when the embeddable privacy modal UIExtension receives data.
     * @param data The received WantParams data.
     */
    void OnReceive(const AAFwk::WantParams& data);

private:
    int32_t sessionId_ = 0;
#ifdef SUPPORT_SCREEN
    Ace::UIContent* uiContent_ = nullptr;
#endif // SUPPORT_SCREEN
    std::weak_ptr<UIExtensionContext> contextWeak_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_EMBEDDABLE_PRIVACY_MODAL_CALLBACK_H
