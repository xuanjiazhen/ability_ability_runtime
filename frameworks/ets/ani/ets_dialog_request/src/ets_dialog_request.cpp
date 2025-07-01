/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "ets_dialog_request.h"

#include "hilog_tag_wrapper.h"
#include "sts_error_utils.h"
#include "ets_dialog_request_callback.h"
#include "js_runtime_utils.h"
#include "ani_common_want.h"
#include "request_constants.h"
#include "ets_request_info.h"

namespace OHOS {
namespace AbilityRuntime {

class EtsDialogRequest final {
public:
    EtsDialogRequest() = default;
    ~EtsDialogRequest() = default;

    static ani_object GetRequestInfo(ani_env *env, ani_object wantEts)
    {
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::DIALOG, "env is nullptr");
            return nullptr;
        }

        OHOS::AAFwk::Want want;
        if (!OHOS::AppExecFwk::UnwrapWant(env, wantEts, want)) {
            TAG_LOGE(AAFwkTag::DIALOG, "UnwrapWant failed");
            return nullptr;
        }

        sptr<IRemoteObject> callerToken = want.GetRemoteObject(RequestConstants::REQUEST_TOKEN_KEY);
        if (!callerToken) {
            TAG_LOGE(AAFwkTag::DIALOG, "get token from target wan failed");
            return nullptr;
        }
        int32_t left = want.GetIntParam(RequestConstants::WINDOW_RECTANGLE_LEFT_KEY, 0);
        int32_t top = want.GetIntParam(RequestConstants::WINDOW_RECTANGLE_TOP_KEY, 0);
        int32_t width = want.GetIntParam(RequestConstants::WINDOW_RECTANGLE_WIDTH_KEY, 0);
        int32_t height = want.GetIntParam(RequestConstants::WINDOW_RECTANGLE_HEIGHT_KEY, 0);

        auto requestInfo = new RequestInfo(callerToken, left, top, width, height);
        ani_object etsRequestInfo = RequestInfo::WrapRequestInfo(env, requestInfo);
        if (etsRequestInfo == nullptr) {
            TAG_LOGE(AAFwkTag::DIALOG, "WrapRequestInfo failed");
            return nullptr;   
        }
        return etsRequestInfo;
    }

    static ani_object GetRequestCallback(ani_env *env, ani_object wantEts)
    {
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::DIALOG, "env is nullptr");
            return nullptr;
        }

        OHOS::AAFwk::Want want;
        if (!OHOS::AppExecFwk::UnwrapWant(env, wantEts, want)) {
            TAG_LOGE(AAFwkTag::DIALOG, "The input want is invalid");
            return nullptr;
        }
        sptr<IRemoteObject> remoteObj = want.GetRemoteObject(RequestConstants::REQUEST_CALLBACK_KEY);
        if (!remoteObj) {
            TAG_LOGE(AAFwkTag::DIALOG, "wrap requestCallback failed");
            return nullptr;
        }

        sptr<IDialogRequestCallback> callback = iface_cast<IDialogRequestCallback>(remoteObj);
        if (!callback) {
            TAG_LOGE(AAFwkTag::DIALOG, "Cast to IDialogRequestCallback failed");
            AbilityRuntime::ThrowStsError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
            return nullptr;
        }
        return CreateEtsDialogRequestCallback(env, callback);
    }
};

void EtsDialogRequestInit(ani_env *env)
{
    TAG_LOGD(AAFwkTag::DIALOG, "EtsDialogRequestInit call");
    ani_status status = ANI_ERROR;
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::DIALOG, "EtsDialogRequestInit ResetError failed");
    }
    
    ani_namespace ns;
    status = env->FindNamespace("L@ohos/app/ability/dialogRequest/dialogRequest;", &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DIALOG, "FindNamespace dialogRequest failed status : %{public}d", status);
        return;
    }
    std::array method = {
        ani_native_function {"getRequestInfo",
            "L@ohos/app/ability/Want/Want;:L@ohos/app/ability/dialogRequest/dialogRequest/RequestInfo;",
            reinterpret_cast<void *>(EtsDialogRequest::GetRequestInfo)
        },
        ani_native_function {"getRequestCallback",
            "L@ohos/app/ability/Want/Want;:L@ohos/app/ability/dialogRequest/dialogRequest/RequestCallback;",
            reinterpret_cast<void *>(EtsDialogRequest::GetRequestCallback)
        },
    };
    status = env->Namespace_BindNativeFunctions(ns, method.data(), method.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DIALOG, "Namespace_BindNativeFunctions failed status : %{public}d", status);
    }
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::DIALOG, "ResetError failed");
    }
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "in EtsDualogRequest.ANI_Constructor");
    if (vm == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null vm or result");
        return ANI_INVALID_ARGS;
    }

    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetEnv failed, status=%{public}d", status);
        return ANI_NOT_FOUND;
    }
    EtsDialogRequestInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "EtsDualogRequest.ANI_Constructor finished");
    return ANI_OK;
}
}
}  // namespace AbilityRuntime
}  // namespace OHOS