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

#include "ets_save_request_callback.h"

#include "ability_manager_client.h"
#include "ani_common_util.h"
#include "hilog_tag_wrapper.h"
#include "int_wrapper.h"
#include "ipc_skeleton.h"
#include "sts_error_utils.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *SAVE_REQUEST_CALL_BACK_CLASS_NAME = "Lapplication/AutoFillRequest/SaveRequestCallbackInner;";
}

void EtsSaveRequestCallback::SaveRequestSuccess(ani_env *env, ani_object object)
{
    GetInstance().OnSaveRequestSuccess(env, object);
}

void EtsSaveRequestCallback::SaveRequestFailed(ani_env *env, ani_object object)
{
    GetInstance().OnSaveRequestFailed(env, object);
}

void EtsSaveRequestCallback::OnSaveRequestSuccess(ani_env *env, ani_object object)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        SendResultCodeAndViewData(env, object,
            EtsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_FAILED_INVALID_PARAM);
        return;
    }
    SendResultCodeAndViewData(env, object, EtsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_SUCESS);
}

void EtsSaveRequestCallback::OnSaveRequestFailed(ani_env *env, ani_object object)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        SendResultCodeAndViewData(env, object,
            EtsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_FAILED_INVALID_PARAM);
        return;
    }
    SendResultCodeAndViewData(env, object, EtsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_FAILED);
}

void EtsSaveRequestCallback::SendResultCodeAndViewData(ani_env *env, ani_object object,
    const EtsAutoFillExtensionUtil::AutoFillResultCode &resultCode)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    auto uiWindow = GetWindow(env, object);
    if (uiWindow == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null uiWindow");
        return;
    }

    AAFwk::Want want;
    auto ret = uiWindow->TransferAbilityResult(resultCode, want);
    if (ret != Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "TransferAbilityResult failed");
        return;
    }

    auto sessionInfo = GetSessionInfo(env, object);
    auto errorCode = AAFwk::AbilityManagerClient::GetInstance()->TerminateUIExtensionAbility(sessionInfo);
    if (errorCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "TerminateUIExtensionAbility error: %{public}d", errorCode);
    }
}

EtsSaveRequestCallback &EtsSaveRequestCallback::GetInstance()
{
    static EtsSaveRequestCallback instance;
    return instance;
}

ani_object EtsSaveRequestCallback::SetSessionInfoAndWindow(ani_env *env, const sptr<AAFwk::SessionInfo> &sessionInfo,
    const sptr<Rosen::Window> &uiWindow)
{
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object object = nullptr;
    ani_field field = nullptr;
    if (sessionInfo == nullptr || uiWindow == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null sessionInfo or uiWindow");
        return nullptr;
    }
    if ((status = env->FindClass(SAVE_REQUEST_CALL_BACK_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "find class status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":V", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "find method status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "new object status: %{public}d", status);
        return nullptr;
    }
    SetField(env, object, cls, field, sessionInfo, uiWindow);
    return object;
}

void EtsSaveRequestCallback::SetField(ani_env *env, ani_object object, ani_class cls, ani_field field,
    const sptr<AAFwk::SessionInfo> &sessionInfo, const sptr<Rosen::Window> &uiWindow)
{
    ani_status status = ANI_ERROR;
    if ((status = env->Class_FindField(cls, "nativeSessionInfo", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "find nativeSessionInfo: %{public}d", status);
        return;
    }
    auto nativeSessionInfo = new (std::nothrow) sptr<AAFwk::SessionInfo>(sessionInfo);
    if (nativeSessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "nativeSessionInfo nullptr");
        return;
    }
    ani_long nativeSessionInfoLong = (ani_long)nativeSessionInfo;
    if ((status = env->Object_SetField_Long(object, field, nativeSessionInfoLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "nativeSessionInfoLong: %{public}d", status);
        delete nativeSessionInfo;
        nativeSessionInfo = nullptr;
        return;
    }
    if ((status = env->Class_FindField(cls, "nativeUiWindow", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "find nativeUiWindow: %{public}d", status);
        return;
    }
    auto nativeUiWindow = new (std::nothrow) sptr<Rosen::Window>(uiWindow);
    if (nativeUiWindow == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "nativeUiWindow nullptr");
        return;
    }
    ani_long nativeUiWindowLong = (ani_long)nativeUiWindow;
    if ((status = env->Object_SetField_Long(object, field, nativeUiWindowLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "nativeUiWindowLong: %{public}d", status);
        delete nativeUiWindow;
        nativeUiWindow = nullptr;
        return;
    }
}

sptr<AAFwk::SessionInfo> EtsSaveRequestCallback::GetSessionInfo(ani_env *env, ani_object object)
{
    ani_long nativeSessionInfoLong;
    ani_class cls = nullptr;
    ani_field field = nullptr;
    ani_status status = ANI_ERROR;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(SAVE_REQUEST_CALL_BACK_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "find class status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindField(cls, "nativeSessionInfo", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "find field: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_GetField_Long(object, field, &nativeSessionInfoLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "nativeSessionInfoLong: %{public}d", status);
        return nullptr;
    }
    auto nativeSessionInfoPtr = reinterpret_cast<AAFwk::SessionInfo*>(nativeSessionInfoLong);
    auto nativeSessionInfo = sptr<AAFwk::SessionInfo>(nativeSessionInfoPtr);
    return nativeSessionInfo != nullptr ? nativeSessionInfo : nullptr;
}

sptr<Rosen::Window> EtsSaveRequestCallback::GetWindow(ani_env *env, ani_object object)
{
    ani_long nativeUiWindowLong;
    ani_class cls = nullptr;
    ani_field field = nullptr;
    ani_status status = ANI_ERROR;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(SAVE_REQUEST_CALL_BACK_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "find class status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindField(cls, "nativeUiWindow", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "find field: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_GetField_Long(object, field, &nativeUiWindowLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "nativeUiWindowLong: %{public}d", status);
        return nullptr;
    }
    auto nativeUiWindowPtr = reinterpret_cast<Rosen::Window*>(nativeUiWindowLong);
    auto nativeUiWindow = sptr<Rosen::Window>(nativeUiWindowPtr);
    return nativeUiWindow != nullptr ? nativeUiWindow : nullptr;
}

ani_object EtsSaveRequestCallback::CreateEtsSaveRequestCallback(ani_env *env,
    const sptr<AAFwk::SessionInfo> &sessionInfo, const sptr<Rosen::Window> &uiWindow)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        return nullptr;
    }
    if (sessionInfo == nullptr || uiWindow == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null sessionInfo or uiWindow");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass(SAVE_REQUEST_CALL_BACK_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "find class status: %{public}d", status);
        return nullptr;
    }
    std::array functions = {
        ani_native_function { "onSuccess", ":V",
            reinterpret_cast<void*>(EtsSaveRequestCallback::SaveRequestSuccess) },
        ani_native_function { "onFailure", ":V",
            reinterpret_cast<void*>(EtsSaveRequestCallback::SaveRequestFailed) },
    };
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "bind method status : %{public}d", status);
        return nullptr;
    }
    ani_object object = SetSessionInfoAndWindow(env, sessionInfo, uiWindow);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null object");
        return nullptr;
    }
    return object;
}
} // namespace AbilityRuntime
} // namespace OHOS