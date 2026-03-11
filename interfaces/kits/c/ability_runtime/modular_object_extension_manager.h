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

/**
 * @addtogroup AbilityRuntime
 * @{
 *
 * @brief Provide the C interface for the modular object extension manager.
 *
 * @since 26.0.0
 */

/**
 * @file modular_object_extension_manager.h
 *
 * @brief Declare the modular object extension manager.
 *
 * @library libmodular_object_extension.so
 * @kit AbilityKit
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 26.0.0
 */

#ifndef Ability_RUNTIME_MODULAR_OBJECT_MANAGER_H
#define Ability_RUNTIME_MODULAR_OBJECT_MANAGER_H

#include <stddef.h>

#include "ability_runtime_common.h"
#include "connect_options.h"
#include "context.h"
#include "want.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Connect modular object extension ability.
 *
 * @param context Represents a pointer to a UI ability context.
 * @param want Indicates the modular object extension to connect.
 * @param connectOptions Indicates the callback of connection.
 * @param connectionId Indicates output connection id.
 * @returns Returns a specific error code.
 * @since 26.0.0
 */
AbilityRuntime_Errcode OH_AbilityRuntime_ConnectModularObjectExtensionAbility(
    AbilityRuntime_ContextHandle context, AbilityBase_Want *want,
    OH_AbilityRuntime_ConnectOptions *connectOptions, int64_t *connectionId);

/**
 * @brief Disconnect modular object extension ability.
 *
 * @param context Represents a pointer to a UI ability context.
 * @param connectionId Indicates the connection id.
 * @returns Returns a specific error code.
 * @since 26.0.0
 */
AbilityRuntime_Errcode OH_AbilityRuntime_DisconnectModularObjectExtensionAbility(
    AbilityRuntime_ContextHandle context, int64_t connectionId);

/**
 * @brief Define the OH_AbilityRuntime_ModularObject_ExtensionInfo structure type.
 *
 * @since 26.0.0
 */
struct OH_AbilityRuntime_ModularObject_ExtensionInfo;

/**
 * @brief Define the OH_AbilityRuntime_MoeInfo structure type.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_ModularObject_ExtensionInfo OH_AbilityRuntime_MoeInfo;

/**
 * @brief Defines the pointer to OH_AbilityRuntime_MoeInfo.
 *
 * @since 26.0.0
 */
typedef OH_AbilityRuntime_MoeInfo* OH_AbilityRuntime_MoeInfoHandle;

/**
 * @brief Destroy input modular object extension info.
 *
 * @param extensionInfo The modular object extension info to be deleted.
 * @returns Returns a specific error code.
 * @since 26.0.0
 */
AbilityRuntime_Errcode OH_AbilityRuntime_DestroyExtensionInfo(OH_AbilityRuntime_MoeInfoHandle extensionInfo);

/**
 * @brief The launch mode of modular object extension.
 * @since 26.0.0
 */
typedef enum OH_AbilityRuntime_LaunchMode {
    OH_AbilityRuntime_LaunchMode_InProccess = 0,
    OH_AbilityRuntime_LaunchMode_CrossProcess = 1,
} OH_AbilityRuntime_LaunchMode;

/**
 * @brief The process mode of modular object extension.
 * @since 26.0.0
 */
typedef enum OH_AbilityRuntime_ProcessMode {
    OH_AbilityRuntime_ProcessMode_Bundle = 0,
    OH_AbilityRuntime_ProcessMode_Type = 1,
    OH_AbilityRuntime_ProcessMode_Instance = 2,
} OH_AbilityRuntime_ProcessMode;

/**
 * @brief The thread mode of modular object extension.
 * @since 26.0.0
 */
typedef enum OH_AbilityRuntime_ThreadMode {
    OH_AbilityRuntime_ThreadMode_Bundle = 0,
    OH_AbilityRuntime_ThreadMode_Type = 1,
    OH_AbilityRuntime_ThreadMode_Instance = 2,
} OH_AbilityRuntime_ThreadMode;

/**
 * @brief Get launch mode from modular object extension info.
 *
 * @param extensionInfo The modular object extension info.
 * @param launchMode The obtained launch mode.
 * @returns Returns a specific error code.
 * @since 26.0.0
 */
AbilityRuntime_Errcode OH_AbilityRuntime_GetModularObjectExtensionInfoLaunchMode(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, OH_AbilityRuntime_LaunchMode *launchMode);

/**
 * @brief Get process mode from modular object extension info.
 *
 * @param extensionInfo The modular object extension info.
 * @param processMode The obtained process mode.
 * @returns Returns a specific error code.
 * @since 26.0.0
 */
AbilityRuntime_Errcode OH_AbilityRuntime_GetModularObjectExtensionInfoProcessMode(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, OH_AbilityRuntime_ProcessMode *processMode);

/**
 * @brief Get thread mode from modular object extension info.
 *
 * @param extensionInfo The modular object extension info.
 * @param threadMode The obtained thread mode.
 * @returns Returns a specific error code.
 * @since 26.0.0
 */
AbilityRuntime_Errcode OH_AbilityRuntime_GetModularObjectExtensionInfoThreadMode(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, OH_AbilityRuntime_ThreadMode *threadMode);

/**
 * @brief Get element from modular object extension info.
 *
 * @param extensionInfo The modular object extension info.
 * @param element The obtained element.
 * @returns Returns a specific error code.
 * @since 26.0.0
 */
AbilityRuntime_Errcode OH_AbilityRuntime_GetModularObjectExtensionInfoElementName(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, AbilityBase_Element *element);

/**
 * @brief Get disable state from modular object extension info.
 *
 * @param extensionInfo The modular object extension info.
 * @param isSelfDisabled Whether extension is disabled by itself.
 * @param isSystemDisabled Whether extension is disabled by system.
 * @returns Returns a specific error code.
 * @since 26.0.0
 */
AbilityRuntime_Errcode OH_AbilityRuntime_GetModularObjectExtensionInfoDisableState(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, bool *isSelfDisabled, bool *isSystemDisabled);

/**
 * @brief Query modular object extension infos within self application.
 *
 * @param extensionInfos Output extension info array.
 * @param count Output extension info count.
 * @returns Returns a specific error code.
 * @since 26.0.0
 */
AbilityRuntime_Errcode OH_AbilityRuntime_QuerySelfModularObjectExtensionInfo(
    OH_AbilityRuntime_MoeInfoHandle *extensionInfos, size_t *count);

/**
 * @brief Set disable state of modular object extension within self application.
 *
 * @param abilityName The ability name.
 * @param isDisable The disable state.
 * @returns Returns a specific error code.
 * @since 26.0.0
 */
AbilityRuntime_Errcode OH_AbilityRuntime_SetSelfModularObjectDisableState(char *abilityName, bool isDisable);

#ifdef __cplusplus
}
#endif

/** @} */
#endif // Ability_RUNTIME_MODULAR_OBJECT_MANAGER_H
