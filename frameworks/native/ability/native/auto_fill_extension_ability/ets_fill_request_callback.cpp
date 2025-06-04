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

#include "ets_fill_request_callback.h"

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
constexpr const char *WANT_PARAMS_VIEW_DATA = "ohos.ability.params.viewData";
constexpr const char *WANT_PARAMS_AUTO_FILL_CMD = "ohos.ability.params.autoFillCmd";
constexpr const char *WANT_PARAMS_AUTO_FILL_CMD_AUTOFILL = "autofill";
constexpr const char *WANT_PARAMS_UPDATE_POPUP_WIDTH = "ohos.ability.params.popupWidth";
constexpr const char *WANT_PARAMS_UPDATE_POPUP_HEIGHT = "ohos.ability.params.popupHeight";
constexpr const char *WANT_PARAMS_UPDATE_POPUP_PLACEMENT = "ohos.ability.params.popupPlacement";
constexpr const char *CONFIG_POPUP_SIZE = "popupSize";
constexpr const char *CONFIG_POPUP_PLACEMENT = "placement";
constexpr const char *WANT_PARAMS_FILL_CONTENT = "ohos.ability.params.fillContent";
constexpr const char *ERROR_MSG_INVALID_EMPTY = "JsonString is empty.";
constexpr const char *FILL_REQUEST_CALL_BACK_CLASS_NAME = "Lapplication/AutoFillRequest/FillRequestCallbackInner;";
}

EtsFillRequestCallback &EtsFillRequestCallback::GetInstance()
{
    static EtsFillRequestCallback instance;
    return instance;
}

ani_object EtsFillRequestCallback::SetSessionInfoAndWindow(ani_env *env, const sptr<AAFwk::SessionInfo> &sessionInfo,
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
    if ((status = env->FindClass(FILL_REQUEST_CALL_BACK_CLASS_NAME, &cls)) != ANI_OK) {
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

void EtsFillRequestCallback::SetField(ani_env *env, ani_object object, ani_class cls, ani_field field,
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

sptr<AAFwk::SessionInfo> EtsFillRequestCallback::GetSessionInfo(ani_env *env, ani_object object)
{
    ani_long nativeSessionInfoLong;
    ani_class cls = nullptr;
    ani_field field = nullptr;
    ani_status status = ANI_ERROR;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(FILL_REQUEST_CALL_BACK_CLASS_NAME, &cls)) != ANI_OK) {
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

sptr<Rosen::Window> EtsFillRequestCallback::GetWindow(ani_env *env, ani_object object)
{
    ani_long nativeUiWindowLong;
    ani_class cls = nullptr;
    ani_field field = nullptr;
    ani_status status = ANI_ERROR;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(FILL_REQUEST_CALL_BACK_CLASS_NAME, &cls)) != ANI_OK) {
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

void EtsFillRequestCallback::FillRequestSuccess(ani_env *env, ani_object object, ani_object responseObj)
{
    GetInstance().OnFillRequestSuccess(env, object, responseObj);
}

void EtsFillRequestCallback::FillRequestFailed(ani_env *env, ani_object object)
{
    GetInstance().OnFillRequestFailed(env, object);
}

void EtsFillRequestCallback::FillRequestCanceled(ani_env *env, ani_object object, ani_object fillContentObj)
{
    GetInstance().OnFillRequestCanceled(env, object, fillContentObj);
}

void EtsFillRequestCallback::FillRequestAutoFillPopupConfig(ani_env *env, ani_object object,
    ani_object autoFillPopupConfigObj)
{
    GetInstance().OnFillRequestAutoFillPopupConfig(env, object, autoFillPopupConfigObj);
}

void EtsFillRequestCallback::OnFillRequestSuccess(ani_env *env, ani_object object, ani_object responseObj)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        SendResultCodeAndViewData(env, object,
            EtsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_FAILED_INVALID_PARAM, "");
        return;
    }
    FillResponse response;
    EtsAutoFillExtensionUtil::UnwrapFillResponse(env, responseObj, response);
    std::string jsonString = response.viewData.ToJsonString();
    if (jsonString.empty()) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "empty jsonString");
        ThrowStsError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        SendResultCodeAndViewData(env, object,
            EtsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_FAILED_INVALID_PARAM, "");
        return;
    }
    SendResultCodeAndViewData(env, object, EtsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_SUCESS, jsonString);
}

void EtsFillRequestCallback::OnFillRequestFailed(ani_env *env, ani_object object)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        SendResultCodeAndViewData(env, object,
            EtsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_FAILED_INVALID_PARAM, "");
        return;
    }
    SendResultCodeAndViewData(env, object, EtsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_FAILED, "");
}

void EtsFillRequestCallback::OnFillRequestCanceled(ani_env *env, ani_object object, ani_object fillContentObj)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        SendResultCodeAndViewData(env, object,
            EtsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_FAILED_INVALID_PARAM, "");
        return;
    }
    std::string jsonString = "";
    ani_status status = ANI_OK;
    ani_boolean isFillContentUndefined;
    if ((status = env->Reference_IsUndefined(fillContentObj, &isFillContentUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Falied to check undefinde status: %{public}d", status);
        SendResultCodeAndViewData(env, object,
            EtsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_FAILED_INVALID_PARAM, "");
        return;
    }
    if (!isFillContentUndefined) {
        if (!AppExecFwk::GetStdString(env, reinterpret_cast<ani_string>(fillContentObj), jsonString)) {
            TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "empty jsonString");
            ThrowStsError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
                ERROR_MSG_INVALID_EMPTY);
            SendResultCodeAndViewData(env, object,
                EtsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_FAILED_INVALID_PARAM, "");
            return;
        }
    }
    SendResultCodeAndViewData(env, object, EtsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_CANCEL, jsonString);
}

void EtsFillRequestCallback::OnFillRequestAutoFillPopupConfig(ani_env *env, ani_object object,
    ani_object autoFillPopupConfigObj)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Non-system app forbidden to call");
        ThrowStsError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }
    auto uiWindow = GetWindow(env, object);
    if (uiWindow == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null uiWindow");
        ThrowStsError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        return;
    }
    AAFwk::WantParams wantParams;
    wantParams.SetParam(WANT_PARAMS_AUTO_FILL_CMD, AAFwk::Integer::Box(AutoFillCommand::RESIZE));
    auto isValueChanged = SetPopupConfigToWantParams(env, autoFillPopupConfigObj, wantParams);
    if (isValueChanged) {
        auto ret = uiWindow->TransferExtensionData(wantParams);
        if (ret != Rosen::WMError::WM_OK) {
            TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Transfer ability result failed");
            ThrowStsError(env, AbilityErrorCode::ERROR_CODE_INNER);
        }
    }
}

bool EtsFillRequestCallback::SetPopupConfigToWantParams(ani_env *env, ani_object autoFillPopupConfigObj,
    AAFwk::WantParams& wantParams)
{
    ani_object etsValue = nullptr;
    bool isValueChanged = false;
    etsValue = AppExecFwk::GetPropertyValueByName(env, autoFillPopupConfigObj, CONFIG_POPUP_SIZE);
    if (etsValue) {
        PopupSize popupSize;
        EtsAutoFillExtensionUtil::UnwrapPopupSize(env, etsValue, popupSize);
        wantParams.SetParam(WANT_PARAMS_UPDATE_POPUP_WIDTH, AAFwk::Integer::Box(popupSize.width));
        wantParams.SetParam(WANT_PARAMS_UPDATE_POPUP_HEIGHT, AAFwk::Integer::Box(popupSize.height));
        isValueChanged = true;
    }
    int popupPlacement = 0;
    if (AppExecFwk::GetIntByName(env, autoFillPopupConfigObj, CONFIG_POPUP_PLACEMENT, popupPlacement)) {
        wantParams.SetParam(WANT_PARAMS_UPDATE_POPUP_PLACEMENT, AAFwk::Integer::Box(popupPlacement));
        isValueChanged = true;
    }
    return isValueChanged;
}

void EtsFillRequestCallback::SendResultCodeAndViewData(ani_env *env, ani_object object,
    const EtsAutoFillExtensionUtil::AutoFillResultCode &resultCode, const std::string &etsString)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    auto uiWindow = GetWindow(env, object);
    if (uiWindow == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null uiWindow");
        return;
    }

    AAFwk::Want want;
    if (resultCode == EtsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_SUCESS) {
        want.SetParam(WANT_PARAMS_VIEW_DATA, etsString);
        want.SetParam(WANT_PARAMS_AUTO_FILL_CMD, WANT_PARAMS_AUTO_FILL_CMD_AUTOFILL);
    }

    if (resultCode == EtsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_CANCEL) {
        want.SetParam(WANT_PARAMS_FILL_CONTENT, etsString);
    }

    auto ret = uiWindow->TransferAbilityResult(resultCode, want);
    if (ret != Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "TransferAbilityResult failed");
        return;
    }

    auto sessionInfo = GetSessionInfo(env, object);
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null sessionInfo");
        return;
    }

    auto errorCode = AAFwk::AbilityManagerClient::GetInstance()->TerminateUIExtensionAbility(sessionInfo);
    if (errorCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "TerminateUIExtensionAbility error: %{public}d", errorCode);
    }
}

ani_object EtsFillRequestCallback::CreateEtsFillRequestCallback(ani_env *env,
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
    if ((status = env->FindClass(FILL_REQUEST_CALL_BACK_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "find class status: %{public}d", status);
        return nullptr;
    }
    std::array functions = {
        ani_native_function { "onSuccess", "Lapplication/AutoFillRequest/FillResponse;:V",
            reinterpret_cast<void*>(EtsFillRequestCallback::FillRequestSuccess) },
        ani_native_function { "onFailure", ":V",
            reinterpret_cast<void*>(EtsFillRequestCallback::FillRequestFailed) },
        ani_native_function { "onCancel", "Lstd/core/String;:V",
            reinterpret_cast<void*>(EtsFillRequestCallback::FillRequestCanceled) },
        ani_native_function { "setAutoFillPopupConfig", "Lapplication/AutoFillRequest/AutoFillPopupConfig;:V",
            reinterpret_cast<void*>(EtsFillRequestCallback::FillRequestAutoFillPopupConfig) },
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