/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "call_common_util.h"
#include "cj_common_ffi.h"
#include "cj_lambda.h"
#include "cj_utils_ffi.h"
#include "cj_ability_context.h"
#include "cj_want_ffi.h"
#include "ffi_remote_data.h"
#include "hilog_tag_wrapper.h"
#include "remote_object_impl.h"
#include "ability_business_error.h"
#include "ability_runtime/cj_caller_complex.h"

using namespace OHOS::FFI;

namespace OHOS {
namespace AbilityRuntime {
namespace {
int32_t StartAbilityByCallComplete(std::shared_ptr<AbilityContext> abilityContext,
    std::shared_ptr<StartAbilityByCallData> calldata, std::shared_ptr<CallerCallBack> callerCallBack,
    int64_t* callerId, int64_t* remoteId)
{
    if (calldata == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null calldata");
        return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
    }
    if (calldata->remoteCallee == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "remoteCallee null");
        if (abilityContext != nullptr) {
            abilityContext->ClearFailedCallConnection(callerCallBack);
        }
        return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
    }
    auto releaseCallAbilityFunc = [abilityContext] (const std::shared_ptr<CallerCallBack> &callback) -> ErrCode {
        if (abilityContext == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "null releaseCallAbilityFunction");
            return -1;
        }
        return abilityContext->ReleaseCall(callback);
    };
    return CreateCjCallerComplex(releaseCallAbilityFunc, calldata->remoteCallee,
        callerCallBack, callerId, remoteId);
}
} // namespace

extern "C" {
CJ_EXPORT int32_t FFIAbilityContextStartAbilityByCall(int64_t id, WantHandle wantHandle,
    int64_t* callerId, int64_t* remoteId)
{
    if (callerId == nullptr) {
        return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
    }
    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null CJAbilityContext");
        return ERR_INVALID_INSTANCE_CODE;
    }
    int32_t userId = DEFAULT_INVAL_VALUE;
    AAFwk::Want* want = reinterpret_cast<AAFwk::Want*>(wantHandle);
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null wantHandle");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto callData = std::make_shared<StartAbilityByCallData>();
    auto callerCallBack = std::make_shared<CallerCallBack>();
    CallCommonUtil::GenerateCallerCallBack(callData, callerCallBack);
    auto ret = context->StartAbilityByCall(*want, callerCallBack, userId);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::CONTEXT, "startAbility failed");
        return static_cast<int32_t>(GetJsErrorCodeByNativeError(ret));
    }
    CallCommonUtil::WaitForCalleeObj(callData);
    return StartAbilityByCallComplete(context->GetAbilityContext(), callData, callerCallBack, callerId, remoteId);
}
}
} // namespace AbilityRuntime
} // namespace OHOS
