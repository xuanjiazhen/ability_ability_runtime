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

#include "modular_object_extension_ability.h"

#include "hilog_tag_wrapper.h"
#include "modular_object_extension_types.h"

namespace {
using OHOS::AbilityRuntime::GetModularObjectExtensionType;

AbilityRuntime_ErrorCode CheckMoeInstance(OH_AbilityRuntime_MoeInstanceHandle instance,
    OH_AbilityRuntime_ModularObject_ExtensionInstance **moeInstance)
{
    if (instance == nullptr || moeInstance == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid params");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto *inner = reinterpret_cast<OH_AbilityRuntime_ModularObject_ExtensionInstance *>(instance);
    if (inner->type != GetModularObjectExtensionType()) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid extension type");
        return ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE;
    }
    *moeInstance = inner;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}
} // namespace

#ifdef __cplusplus
extern "C" {
#endif

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoeAbility_RegisterOnCreateFunc(
    OH_AbilityRuntime_MoeInstanceHandle instance, OH_AbilityRuntime_MoeAbility_OnCreateFunc onCreateFunc)
{
    if (onCreateFunc == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null onCreateFunc");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    OH_AbilityRuntime_ModularObject_ExtensionInstance *inner = nullptr;
    auto ret = CheckMoeInstance(instance, &inner);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    inner->onCreateFunc = onCreateFunc;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoeAbility_RegisterOnDestroyFunc(
    OH_AbilityRuntime_MoeInstanceHandle instance, OH_AbilityRuntime_MoeAbility_OnDestroyFunc onDestroyFunc)
{
    if (onDestroyFunc == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null onDestroyFunc");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    OH_AbilityRuntime_ModularObject_ExtensionInstance *inner = nullptr;
    auto ret = CheckMoeInstance(instance, &inner);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    inner->onDestroyFunc = onDestroyFunc;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoeAbility_RegisterOnConnectFunc(
    OH_AbilityRuntime_MoeInstanceHandle instance, OH_AbilityRuntime_MoeAbility_OnConnectFunc onConnectFunc)
{
    if (onConnectFunc == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null onConnectFunc");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    OH_AbilityRuntime_ModularObject_ExtensionInstance *inner = nullptr;
    auto ret = CheckMoeInstance(instance, &inner);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    inner->onConnectFunc = onConnectFunc;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoeAbility_RegisterOnDisconnectFunc(
    OH_AbilityRuntime_MoeInstanceHandle instance, OH_AbilityRuntime_MoeAbility_OnDisconnectFunc onDisconnectFunc)
{
    if (onDisconnectFunc == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null onDisconnectFunc");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    OH_AbilityRuntime_ModularObject_ExtensionInstance *inner = nullptr;
    auto ret = CheckMoeInstance(instance, &inner);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    inner->onDisconnectFunc = onDisconnectFunc;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoeAbility_GetContextFromInstance(
    OH_AbilityRuntime_MoeInstanceHandle instance, OH_AbilityRuntime_MoeContextHandle *context)
{
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    OH_AbilityRuntime_ModularObject_ExtensionInstance *inner = nullptr;
    auto ret = CheckMoeInstance(instance, &inner);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    if (inner->context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null inner context");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    *context = inner->context.get();
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoeAbility_GetInstanceFromBase(
    AbilityRuntime_ExtensionInstanceHandle baseExtensionInstance,
    OH_AbilityRuntime_MoeInstanceHandle *moExtensionInstance)
{
    if (baseExtensionInstance == nullptr || moExtensionInstance == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid params");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (baseExtensionInstance->type != GetModularObjectExtensionType()) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid extension type");
        return ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE;
    }
    *moExtensionInstance = reinterpret_cast<OH_AbilityRuntime_MoeInstanceHandle>(baseExtensionInstance);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

#ifdef __cplusplus
} // extern "C"
#endif
