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

#include "ets_dialog_request_callback.h"

#include "hilog_tag_wrapper.h"
#include "sts_context_utils.h"
#include "sts_error_utils.h"
#include "js_runtime_utils.h"
#include "ani_common_want.h"
#include "ani_common_util.h"
#include "ani_enum_convert.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
class EtsDialogRequestCallback {
public:
    explicit EtsDialogRequestCallback() = default;

    virtual ~EtsDialogRequestCallback() = default;

    static EtsDialogRequestCallback &GetInstance()
    {
        static EtsDialogRequestCallback instance;
        return instance;
    }

    static void SetRequestResult(ani_env *env,ani_object param, ani_object result)
    {
        if (env == nullptr || result == nullptr) {
            TAG_LOGE(AAFwkTag::DIALOG, "null %{public}s", ((env == nullptr) ? "env" : "result"));
            return;
        }
        GetInstance().OnSetRequestResult(env, param, result);
    }

private:
    void OnSetRequestResult(ani_env *env, ani_object param, ani_object result)
    {
        TAG_LOGI(AAFwkTag::DIALOG, "call");
        ani_boolean isResultCodeUndefined = true;
        ani_ref resultCodeRef = nullptr;
        if (AppExecFwk::GetPropertyRef(env, result, "resultCode", resultCodeRef, isResultCodeUndefined) && isResultCodeUndefined) {
            TAG_LOGE(AAFwkTag::DIALOG, " resultCode is undefined");
            return;
        }
        int32_t resultCode = 0;
        ani_boolean isConvertSucess = false;
        isConvertSucess = AAFwk::AniEnumConvertUtil::EnumConvertStsToNative(
            env, reinterpret_cast<ani_enum_item>(resultCodeRef), resultCode);
        if (!isConvertSucess) {
            TAG_LOGE(AAFwkTag::DIALOG, " EnumConvertStsToNative is failed");
            return;
        }
        TAG_LOGD(AAFwkTag::DIALOG, "processMode: %{public}d", resultCode);

        AAFwk::Want wantValue;
        ani_boolean isWantUndefined = true;
        ani_ref wantObject = nullptr;
        if (AppExecFwk::GetPropertyRef(env, result, "want", wantObject, isWantUndefined) && !isWantUndefined) {
            AppExecFwk::UnwrapWant(env, static_cast<ani_object>(wantObject), wantValue);
        } else {
            TAG_LOGW(AAFwkTag::DIALOG, "want is undefined");
        }

        sptr<IDialogRequestCallback> callback = GetDialogRequestCallback(env, param);

        if (callback == nullptr) {
            TAG_LOGE(AAFwkTag::DIALOG, "callback is null");
            return;
        }

        callback->SendResult(resultCode, wantValue);
        return;
    }

    sptr<IDialogRequestCallback> GetDialogRequestCallback(ani_env *env, ani_object object)
    {
        ani_long nativeDialogRequest = 0;
        ani_class cls = nullptr;
        ani_field nativeField = nullptr;
        ani_status status = ANI_ERROR;
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::DIALOG, "null env");
            return nullptr;
        }

        status = env->Object_GetFieldByName_Long(object, "nativeRequestCallback", &nativeDialogRequest);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::DIALOG, "Object_SetField_Long failed, status = %{public}d", status);
            return nullptr;
        }
        
        auto pNativeDialogReq = reinterpret_cast<IDialogRequestCallback*>(nativeDialogRequest);
        auto nativeDialogReq = sptr<IDialogRequestCallback>(pNativeDialogReq);
        return nativeDialogReq;
    }
};
}

ani_object CreateEtsDialogRequestCallback(ani_env *env, const sptr<IDialogRequestCallback> &remoteObj)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DIALOG, "null env");
        return nullptr;
    }
    ani_object object = nullptr;
    ani_method method = nullptr;
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    status = env->FindClass("L@ohos/app/ability/dialogRequest/dialogRequest/RequestCallbackInner;", &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DIALOG, "FindClass failed, status = %{public}d", status);
        return nullptr;
    }
    status = env->Class_FindMethod(cls, "<ctor>", ":V", &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DIALOG, "FindMethod failed, status = %{public}d", status);
        return nullptr;
    }
    status = env->Object_New(cls, method, &object);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DIALOG, "ObjectNew failed, status = %{public}d", status);
        return nullptr;
    }

    std::array methods = {
        ani_native_function {"setRequestResult", nullptr, reinterpret_cast<void *>(EtsDialogRequestCallback::SetRequestResult)},
    };
    
    status = env->Class_BindNativeMethods(cls, methods.data(), methods.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DIALOG, "Class_BindNativeMethod failed, status = %{public}d", status);
        return nullptr;
    }

    status = env->Object_SetFieldByName_Long(object, "nativeRequestCallback", reinterpret_cast<ani_long>(remoteObj.GetRefPtr()));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DIALOG, "Object_SetField_Long failed, status = %{public}d", status);
        return nullptr;
    }
    return object;
}
} // AbilityRuntime
} // OHOS
