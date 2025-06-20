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

#include "ets_auto_fill_extension_context.h"

#include "ani_common_util.h"
#include "ani_common_want.h"
#include "ets_extension_context.h"
#include "hilog_tag_wrapper.h"
#include "sts_error_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* AUTO_FILL_EXTENSION_CONTEXT_CLASS_NAME =
    "Lapplication/AutoFillExtensionContext/AutoFillExtensionContext;";
constexpr const char* CUSTOMDATA_DATA = "data";
}

std::unique_ptr<EtsAutoFillExtensionContext> EtsAutoFillExtensionContext::etsContext_ = nullptr;

void EtsAutoFillExtensionContext::ReloadInModal(ani_env *env, ani_object object, ani_object callback)
{
    etsContext_->OnReloadInModal(env, object, callback);
}

void EtsAutoFillExtensionContext::OnReloadInModal(ani_env *env, ani_object object, ani_object callback)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        return;
    }
    ani_object etsCustomData = AppExecFwk::GetPropertyValueByName(env, object, CUSTOMDATA_DATA);
    CustomData customData;
    if (etsCustomData == nullptr || !AppExecFwk::UnwrapWantParams(env, etsCustomData, customData.data)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Parse custom data failed");
        ThrowStsError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null context");
        ThrowStsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    auto ret = context->ReloadInModal(customData);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "error is %{public}d", ret);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::CreateStsErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
}

ani_object EtsAutoFillExtensionContext::CreateEtsAutoFillExtensionContext(ani_env *env,
    const std::shared_ptr<AutoFillExtensionContext> &context)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        return nullptr;
    }
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null context");
        return nullptr;
    }
    etsContext_ = std::make_unique<EtsAutoFillExtensionContext>(context);
    if (etsContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null etsContext_");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object contextObj = nullptr;
    if ((status = env->FindClass(AUTO_FILL_EXTENSION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "find class status: %{public}d", status);
        return nullptr;
    }
    std::array functions = {
        ani_native_function { "nativeReloadInModal",
            "Lapplication/CustomData/CustomData;Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void*>(EtsAutoFillExtensionContext::ReloadInModal) },
    };
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "bind method status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":V", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "find method status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &contextObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "new object status: %{public}d", status);
        return nullptr;
    }
    OHOS::AbilityRuntime::CreatEtsExtensionContext(env, cls, contextObj, context, context->GetAbilityInfo());
    return contextObj;
}
}
}