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

std::unique_ptr<EtsSaveRequestCallback> EtsSaveRequestCallback::etsSession_ = nullptr;

EtsSaveRequestCallback::EtsSaveRequestCallback(
    const sptr<AAFwk::SessionInfo> &sessionInfo, const sptr<Rosen::Window> &uiWindow)
    : sessionInfo_(sessionInfo), uiWindow_(uiWindow)
{}

void EtsSaveRequestCallback::SaveRequestSuccess(ani_env *env)
{
    etsSession_->OnSaveRequestSuccess(env);
}

void EtsSaveRequestCallback::SaveRequestFailed(ani_env *env)
{
    etsSession_->OnSaveRequestFailed(env);
}

void EtsSaveRequestCallback::OnSaveRequestSuccess(ani_env *env)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        return;
    }
    SendResultCodeAndViewData(EtsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_SUCESS);
}

void EtsSaveRequestCallback::OnSaveRequestFailed(ani_env *env)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        return;
    }
    SendResultCodeAndViewData(EtsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_FAILED);
}

void EtsSaveRequestCallback::SendResultCodeAndViewData(const EtsAutoFillExtensionUtil::AutoFillResultCode &resultCode)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    if (uiWindow_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null uiWindow_");
        return;
    }

    AAFwk::Want want;
    auto ret = uiWindow_->TransferAbilityResult(resultCode, want);
    if (ret != Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "TransferAbilityResult failed");
        return;
    }

    auto errorCode = AAFwk::AbilityManagerClient::GetInstance()->TerminateUIExtensionAbility(sessionInfo_);
    if (errorCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "TerminateUIExtensionAbility error: %{public}d", errorCode);
    }
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
    etsSession_ = std::make_unique<EtsSaveRequestCallback>(sessionInfo, uiWindow);
    if (etsSession_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null etsSession_");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object object = nullptr;
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
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":V", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "find method status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "new object status: %{public}d", status);
        return nullptr;
    }
    return object;
}
} // namespace AbilityRuntime
} // namespace OHOS