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

#include "ability_runtime/modular_object_extension_context.h"

#include "ability_business_error_utils.h"
#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "modular_object_extension_common.h"
#include "modular_object_extension_context_impl.h"
#include "start_options_impl.h"
#include "want_manager.h"
#include "want_utils.h"

using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;

namespace {
AbilityRuntime_ErrorCode CheckMoeContext(OH_AbilityRuntime_MoeContextHandle context,
    std::shared_ptr<OHOS::AbilityRuntime::Context> &contextPtr)
{
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (context->type != GetModularObjectExtensionType()) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid extension type");
        return ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE;
    }
    contextPtr = context->context.lock();
    if (contextPtr == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "context not exist");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode TransformWant(AbilityBase_Want *want, Want &abilityWant)
{
    auto ret = CheckWant(want);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid want");
        return ret;
    }
    auto errCode = CWantManager::TransformToWant(*want, false, abilityWant);
    if (errCode != ABILITY_BASE_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPKIT, "transform want failed: %{public}d", errCode);
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}
} // namespace

#ifdef __cplusplus
extern "C" {
#endif

AbilityRuntime_Errcode OH_AbilityRuntime_MoeContext_GetBaseContext(
    OH_AbilityRuntime_MoeContextHandle moeContext, AbilityRuntime_ContextHandle *baseContext)
{
    if (moeContext == nullptr || baseContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid params");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (moeContext->type != GetModularObjectExtensionType()) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid extension type");
        return ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE;
    }
    *baseContext = static_cast<AbilityRuntime_ContextHandle>(moeContext);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_Errcode OH_AbilityRuntime_MoeContext_StartSelfUIAbility(
    OH_AbilityRuntime_MoeContextHandle context, AbilityBase_Want *want)
{
    std::shared_ptr<OHOS::AbilityRuntime::Context> contextPtr;
    auto ret = CheckMoeContext(context, contextPtr);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    (void)contextPtr;
    Want abilityWant;
    ret = TransformWant(want, abilityWant);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    auto err = AbilityManagerClient::GetInstance()->StartSelfUIAbility(abilityWant);
    return ConvertToCommonBusinessErrorCode(err);
}

AbilityRuntime_Errcode OH_AbilityRuntime_MoeContext_StartSelfUIAbilityWithStartOptions(
    OH_AbilityRuntime_MoeContextHandle context, AbilityBase_Want *want, AbilityRuntime_StartOptions *options)
{
    if (options == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null options");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::shared_ptr<OHOS::AbilityRuntime::Context> contextPtr;
    auto ret = CheckMoeContext(context, contextPtr);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    (void)contextPtr;
    Want abilityWant;
    ret = TransformWant(want, abilityWant);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    auto startOptions = options->GetInnerStartOptions();
    auto err = AbilityManagerClient::GetInstance()->StartSelfUIAbilityWithStartOptions(abilityWant, startOptions);
    return ConvertToAPI17BusinessErrorCode(err);
}

AbilityRuntime_Errcode OH_AbilityRuntime_MoeContext_TerminateSelf(OH_AbilityRuntime_MoeContextHandle context)
{
    std::shared_ptr<OHOS::AbilityRuntime::Context> contextPtr;
    auto ret = CheckMoeContext(context, contextPtr);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    auto token = contextPtr->GetToken();
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null token");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    auto err = AbilityManagerClient::GetInstance()->TerminateAbility(token, -1, nullptr);
    return ConvertToCommonBusinessErrorCode(err);
}

#ifdef __cplusplus
} // extern "C"
#endif
