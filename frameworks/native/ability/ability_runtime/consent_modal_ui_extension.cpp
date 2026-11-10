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

#include "consent_modal_ui_extension.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#ifdef SUPPORT_SCREEN
#include "ui_content.h"
#endif // SUPPORT_SCREEN

namespace OHOS {
namespace AbilityRuntime {
#ifdef SUPPORT_SCREEN
ErrCode CreateConsentModalUIExtension(const std::shared_ptr<AbilityContext> &context,
    const AAFwk::Want &want, const std::shared_ptr<ConsentDialogCallback> &callback)
{
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "consent modal ui extension: null context");
        return ERR_INVALID_VALUE;
    }
    auto uiContent = context->GetUIContent();
    if (uiContent == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "consent modal ui extension: null uiContent");
        return ERR_INVALID_VALUE;
    }
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "consent modal ui extension: null callback");
        return ERR_INVALID_VALUE;
    }

    Ace::ModalUIExtensionCallbacks callbacks;
    callbacks.onResult = [callback](int32_t code, const AAFwk::Want &result) {
        callback->OnResult(code, result);
    };
    callbacks.onError = [callback](int32_t code, const std::string &name, const std::string &message) {
        callback->OnError(code, name, message);
    };
    callbacks.onRelease = [callback](int32_t code) {
        callback->OnRelease(code);
    };

    Ace::ModalUIExtensionConfig config;
    config.isProhibitBack = true;

    int32_t sessionId = uiContent->CreateModalUIExtension(want, callbacks, config);
    if (sessionId == 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "consent modal ui extension: create failed");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}
#endif // SUPPORT_SCREEN
} // namespace AbilityRuntime
} // namespace OHOS