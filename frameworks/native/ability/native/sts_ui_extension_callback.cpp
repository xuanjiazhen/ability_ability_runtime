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

#include "sts_ui_extension_callback.h"

#include "ability_business_error.h"
#include "hilog_tag_wrapper.h"
#include "ui_content.h"

namespace OHOS {
namespace AbilityRuntime {

StsUIExtensionCallback::StsUIExtensionCallback() : JsUIExtensionCallback(nullptr)
{
}

StsUIExtensionCallback::~StsUIExtensionCallback()
{
}

void StsUIExtensionCallback::SetSessionId(int32_t sessionId)
{
    aniSessionId_ = sessionId;
}

void StsUIExtensionCallback::SetUIContent(Ace::UIContent* uiContent)
{
    aniUIContent_ = uiContent;
}

void StsUIExtensionCallback::OnError(int32_t number)
{
    TAG_LOGI(AAFwkTag::APPKIT, "startAbilityByType StsUIExtensionCallback OnError");
    if (aniUIContent_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null aniUIContent_");
        return;
    }
    aniUIContent_->CloseModalUIExtension(aniSessionId_);
}

void StsUIExtensionCallback::OnResult(int32_t resultCode, const AAFwk::Want &want)
{
    TAG_LOGI(AAFwkTag::APPKIT, "startAbilityByType StsUIExtensionCallback OnResult");
    if (aniUIContent_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null aniUIContent_");
        return;
    }
    aniUIContent_->CloseModalUIExtension(aniSessionId_);
}

void StsUIExtensionCallback::OnRelease(int32_t code)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "call, code:%{public}d", code);
    if (aniUIContent_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null aniUIContent_");
        return;
    }
    aniUIContent_->CloseModalUIExtension(aniSessionId_);
}

void StsUIExtensionCallback::SetStsCallbackObject(ani_env *env, ani_object aniObject)
{
    if (env == nullptr) {
        return;
    }

    ani_ref startAbilityCallbackRef = nullptr;
    if (env->GlobalReference_Create(aniObject, &startAbilityCallbackRef) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "GlobalReference_Create failed");
        return;
    }
    aniEnv_ = env;
    startAbilityAniCallback_ = startAbilityCallbackRef;
}
}  // namespace AbilityRuntime
}  // namespace OHOS