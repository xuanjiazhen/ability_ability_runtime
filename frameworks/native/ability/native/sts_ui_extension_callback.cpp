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
#include "ani_common/ani_common_want.h"
#include "hilog_tag_wrapper.h"
#include "ui_content.h"

namespace OHOS {
namespace AbilityRuntime {
#ifdef SUPPORT_SCREEN
constexpr const char* ERROR_MSG_INNER = "Inner error.";
#endif // SUPPORT_SCREEN

ani_object CreateAbilityResult(ani_env* aniEnv, int32_t resultCode, const AAFwk::Want &want)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "startAbilityByType CreateAbilityResult");
    if (aniEnv == nullptr) {
        return {};
    }
    ani_class abilityResult = nullptr;
    ani_status status = ANI_ERROR;
    status = aniEnv->FindClass("LabilityResult/AbilityResultInner;", &abilityResult);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "startAbilityByType find LAbilityResultInner failed status : %{public}d", status);
        return {};
    }
    TAG_LOGI(AAFwkTag::UI_EXT, "find AbilityResultInner success");
 
    ani_method method = nullptr;
    status = aniEnv->Class_FindMethod(abilityResult, "<ctor>", ":V", &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "startAbilityByType Class_FindMethod ctor failed status : %{public}d", status);
        return {};
    }
    TAG_LOGI(AAFwkTag::UI_EXT, "startAbilityByType Class_FindMethod ctor success");
 
    ani_object object = nullptr;
    status = aniEnv->Object_New(abilityResult, method, &object);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "startAbilityByType Object_New failed status : %{public}d", status);
        return {};
    }
    TAG_LOGI(AAFwkTag::UI_EXT, "startAbilityByType Object_New AbilityResultInner success");

    ani_field field = nullptr;
    status = aniEnv->Class_FindField(abilityResult, "resultCode", &field);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "startAbilityByType find resultCode failed.status : %{public}d", status);
        return {};
    }

    status = aniEnv->Object_SetField_Int(object, field, resultCode);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "startAbilityByType set resultCode failed.status : %{public}d", status);
        return {};
    }

    status = aniEnv->Class_FindField(abilityResult, "want", &field);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "startAbilityByType find want failed.status : %{public}d", status);
        return {};
    }
    ani_object wantObj = AppExecFwk::WrapWant(aniEnv, want);
    if (wantObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "startAbilityByType WrapWant want failed.status : %{public}d", status);
        return {};
    }

    ani_ref wantObjRef = nullptr;
    if (aniEnv->GlobalReference_Create(wantObj, &wantObjRef) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "startAbilityByType GlobalReference_Create wantObj failed");
        return {};
    }
    if (aniEnv->Object_SetField_Ref(object, field, wantObjRef) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "startAbilityByType Object_SetField_Ref wantObjRef failed");
        return {};
    }
    return object;
}

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

void StsUIExtensionCallback::SetJsCallbackObject(napi_value jsCallbackObject)
{
}

void StsUIExtensionCallback::OnError(int32_t number)
{
    TAG_LOGI(AAFwkTag::APPKIT, "startAbilityByType StsUIExtensionCallback OnError");
    if (aniEnv_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null aniEnv_");
        return;
    }

    ani_class clsCall = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = aniEnv_->FindClass("LAbilityStartCallback/AbilityStartCallback;", &clsCall)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "startAbilityByType, find class failed, status : %{public}d", status);
        return;
    }

    ani_method method = nullptr;
    if ((status = aniEnv_->Class_FindMethod(clsCall, "onError", "ILstd/core/String;Lstd/core/String;:V", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "startAbilityByType find onError failed, status : %{public}d", status);
        return;
    }

    std::string name;
    std::string message;
#ifdef SUPPORT_SCREEN
    if (number != 0) {
        number = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
        name = ERROR_MSG_INNER;
        message = "StartAbilityByType failed.";
    }
#endif // SUPPORT_SCREEN

    ani_string aniName;
    // Get a ani_string from std::string
    status = aniEnv_->String_NewUTF8(name.c_str(), name.length(), &aniName);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, " startAbilityByType String_NewUTF8 failed status : %{public}d", status);
        return;
    }

    ani_string aniMsg;
    // Get a ani_string from std::string
    status = aniEnv_->String_NewUTF8(message.c_str(), message.length(), &aniMsg);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "startAbilityByType String_NewUTF8 failed status : %{public}d", status);
        return;
    }

    ani_int aniCode = number;
    if ((status = aniEnv_->Object_CallMethod_Void(reinterpret_cast<ani_object>(startAbilityAniCallback_), method, aniCode, aniName, aniMsg)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "startAbilityByType status : %{public}d", status);
        return;
    }

    if (aniUIContent_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null aniUIContent_");
        return;
    }
    aniUIContent_->CloseModalUIExtension(aniSessionId_);
}

void StsUIExtensionCallback::OnResult(int32_t resultCode, const AAFwk::Want &want)
{
    TAG_LOGI(AAFwkTag::APPKIT, "startAbilityByType StsUIExtensionCallback OnResult");
    if (aniEnv_ == nullptr || startAbilityAniCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null aniEnv_");
        return;
    }
    //ToDo call sts by lamda. sts do not support yet.

    TAG_LOGI(AAFwkTag::APPKIT, "startAbilityByType StsUIExtensionCallback OnResult2");
    ani_object startCallback = reinterpret_cast<ani_object>(startAbilityAniCallback_);
    ani_ref onResultRef {};
    ani_status status = aniEnv_->Object_GetFieldByName_Ref(startCallback, "onResult", &onResultRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "startAbilityByType get onResult failed : %{public}d", status);
        return;
    }
    ani_fn_object onResultFn = reinterpret_cast<ani_fn_object>(onResultRef);
    ani_object abilityResultObj = CreateAbilityResult(aniEnv_, resultCode, want);
    if (abilityResultObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "startAbilityByType CreateAbilityResult abilityResultObj failed");
        return;
    }

    ani_ref abilityResultObjRef = nullptr;
    if (aniEnv_->GlobalReference_Create(abilityResultObj, &abilityResultObjRef) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "startAbilityByType GlobalReference_Create abilityResultObj failed");
        return;
    }
    ani_ref argv[] = { abilityResultObjRef };
    if (aniEnv_->FunctionalObject_Call(onResultFn, 1, argv, nullptr) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "startAbilityByType call onResult fn failed.");
        return;
    }

    if (aniUIContent_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null aniUIContent_");
        return;
    }
    aniUIContent_->CloseModalUIExtension(aniSessionId_);
}

void StsUIExtensionCallback::CallJsResult(int32_t resultCode, const AAFwk::Want &want)
{
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

void StsUIExtensionCallback::CallJsError(int32_t number)
{
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