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
 * @brief Provides the definition of the C interface for the modular object extension manager.
 *
 * @since 24
 */

/**
 * @file modular_object_extension_manager.h
 *
 * @brief Declares the modular object extension manager.
 *
 * @library libmodular_object_extension.so
 * @kit AbilityKit
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 24
 */

#ifndef ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_MANAGER_H
#define ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_MANAGER_H

#include <stddef.h>
#include "ability_runtime_common.h"
#include "want_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

struct OH_AbilityRuntime_ModularObject_ExtensionInfo;

/**
 * @brief Defines the pointer to OH_AbilityRuntime_MoeInfo.
 *
 * @since 24
 */
typedef struct OH_AbilityRuntime_ModularObject_ExtensionInfo* OH_AbilityRuntime_MoeInfoHandle;


/**
 * @brief Define the OH_AbilityRuntime_ModularObject_AllExtensionInfos structure type.
 *
 * @since 24
 */
struct OH_AbilityRuntime_ModularObject_AllExtensionInfos;

/**
 * @brief Defines the pointer to OH_AbilityRuntime_AllMoeInfosHandle.
 *
 * @since 24
 */
typedef struct OH_AbilityRuntime_ModularObject_AllExtensionInfos* OH_AbilityRuntime_AllMoeInfosHandle;

/**
 * @brief Destroys the specified modular object extension info.
 *
 * @param extensionInfo The modular object extension info to be deleted.
 * @return Returns a specific error code.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.
 * @since 24
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_DestroyExtensionInfo(OH_AbilityRuntime_MoeInfoHandle *extensionInfo);

AbilityRuntime_ErrorCode OH_AbilityRuntime_DestroyAllExtensionInfos(OH_AbilityRuntime_AllMoeInfosHandle *extensionInfo);

/**
 * @brief The launch mode of a modular object extension.
 * @since 24
 */
typedef enum OH_AbilityRuntime_LaunchMode {
    /**
     * @brief All extensions under the same bundle share a single process.
     * @since 24
     */
    OH_ABILITY_RUNTIME_LAUNCH_MODE_IN_PROCESS = 0,

    /**
     * @brief Allow modular object extension abilities to be started across processes.
     * @since 24
     */
    OH_ABILITY_RUNTIME_LAUNCH_MODE_CROSS_PROCESS = 1,
} OH_AbilityRuntime_LaunchMode;

/**
 * @brief The process mode of a modular object extension.
 * @since 24
 */
typedef enum OH_AbilityRuntime_ProcessMode {
    /**
     * @brief All modular object extensions under the same bundle share a single process.
     * @since 24
     */
    OH_ABILITY_RUNTIME_PROCESS_MODE_BUNDLE = 0,

    /**
     * @brief Modular object extensions with the same name share a single process.
     * @since 24
     */
    OH_ABILITY_RUNTIME_PROCESS_MODE_TYPE = 1,

    /**
     * @brief Each modular object extension instance is a process.
     * @since 24
     */
    OH_ABILITY_RUNTIME_PROCESS_MODE_INSTANCE = 2,
} OH_AbilityRuntime_ProcessMode;

/**
 * @brief The thread mode of a modular object extension.
 * @since 24
 */
typedef enum OH_AbilityRuntime_ThreadMode {
    /**
     * @brief All modular object extensions under the same bundle share a single thread.
     * @since 24
     */
    OH_ABILITY_RUNTIME_THREAD_MODE_BUNDLE = 0,

    /**
     * @brief Modular object extensions with the same name share a single thread.
     * @since 24
     */
    OH_ABILITY_RUNTIME_THREAD_MODE_TYPE = 1,

    /**
     * @brief Each modular object extension instance is a thread.
     * @since 24
     */
    OH_ABILITY_RUNTIME_THREAD_MODE_INSTANCE = 2,
} OH_AbilityRuntime_ThreadMode;

/**
 * @brief Gets the launch mode from modular object extension info.
 *
 * @param extensionInfo The modular object extension info from which to get the launch mode.
 * @param launchMode The launch mode obtained from the extension info.
 * @return Returns a specific error code.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.
 * @since 24
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoLaunchMode(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, OH_AbilityRuntime_LaunchMode *launchMode);

/**
 * @brief Gets the process mode from modular object extension info.
 *
 * @param extensionInfo The modular object extension info to get the process mode from.
 * @param processMode The process mode obtained from the extension info.
 * @return Returns a specific error code.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.
 * @since 24
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoProcessMode(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, OH_AbilityRuntime_ProcessMode *processMode);

/**
 * @brief Gets the thread mode from modular object extension info.
 *
 * @param extensionInfo The modular object extension info to get the thread mode from.
 * @param threadMode The thread mode obtained from the extension info.
 * @return Returns a specific error code.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.
 * @since 24
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoThreadMode(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, OH_AbilityRuntime_ThreadMode *threadMode);

/**
 * @brief Gets elementName from modular object extension info.
 *
 * @param extensionInfo The modular object extension info to get the element name from.
 * @param element The element name obtained from the extension info.
 * @return Returns a specific error code.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.
 * @since 24
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoElementName(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, AbilityBase_Element *element);

/**
 * @brief Gets the disable state of modular object extension.
 *
 * @param extensionInfo The modular object extension info.
 * @param isSelfDisabled Whether the extension is disabled by the application itself.
 * @param isSystemDisabled Whether the extension disabled by system application.
 * @return Returns a specific error code.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.
 * @since 24
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoDisableState(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, bool *isSelfDisabled, bool *isSystemDisabled);

/**
 * @brief Query modular object extension infos within the self application.
 *
 * @param extensionInfos Information about all extensions within the self application.
 * @param count The count of extensions.
 * @return Returns a specific error code.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NOT_SUPPORTED} - if the device does not support.
 * @since 24
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_QuerySelfModularObjectExtensionInfos(
    OH_AbilityRuntime_AllMoeInfosHandle *allExtensionInfos);

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetCountFromAllMoeInfos(
    OH_AbilityRuntime_AllMoeInfosHandle allExtensionInfos, size_t *count);

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetMoeInfoByIndex(
    OH_AbilityRuntime_AllMoeInfosHandle allExtensionInfos, size_t index,
    OH_AbilityRuntime_MoeInfoHandle *extensionInfo);


#ifdef __cplusplus
}
#endif

/** @} */
#endif // ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_MANAGER_H
