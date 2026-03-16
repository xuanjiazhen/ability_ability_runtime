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
 * @brief Provide the C interface for the modular object extension context.
 *
 * @since 26.0.0
 */

/**
 * @file modular_object_extension_context.h
 *
 * @brief Declare the modular object extension context.
 *
 * @library libmodular_object_extension.so
 * @kit AbilityKit
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 26.0.0
 */

#ifndef Ability_RUNTIME_MODULAR_OBJECT_EXTENSION_CONTEXT_H
#define Ability_RUNTIME_MODULAR_OBJECT_EXTENSION_CONTEXT_H

#include "ability_runtime_common.h"
#include "context.h"
#include "start_options.h"
#include "want.h"

#ifdef __cplusplus
extern "C" {
#endif

struct AbilityBase_Want;
typedef struct AbilityBase_Want AbilityBase_Want;

/**
 * @brief Define the OH_AbilityRuntime_ModularObject_ExtensionContext structure type.
 *
 * @since 26.0.0
 */
struct OH_AbilityRuntime_ModularObject_ExtensionContext;

/**
 * @brief Define the OH_AbilityRuntime_MoeContext structure type.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_ModularObject_ExtensionContext OH_AbilityRuntime_MoeContext;

/**
 * @brief Defines the pointer to OH_AbilityRuntime_MoeContext.
 *
 * @since 26.0.0
 */
typedef OH_AbilityRuntime_MoeContext* OH_AbilityRuntime_MoeContextHandle;

/**
 * @brief Get the base context from the modular object extension context.
 *
 * @param moeContext Represents a pointer to modular object extension ability context.
 * @param baseContext Represents output base context handle.
 * @returns Returns a specific error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} - success.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} - parameter check failed.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NOT_SUPPORTED} - device not supported.
 * @since 26.0.0
 */
AbilityRuntime_Errcode OH_AbilityRuntime_MoeContext_GetBaseContext(
    OH_AbilityRuntime_MoeContextHandle moeContext, AbilityRuntime_ContextHandle *baseContext);

/**
 * @brief Starts self UIAbility.
 *
 * @permission {@code ohos.permission.NDK_START_SELF_UI_ABILITY}
 * @param context Represents a pointer to modular object extension ability context.
 * @param want The arguments passed to start self UIAbility.
 * @returns Returns a specific error code.
 * @since 26.0.0
 */
AbilityRuntime_Errcode OH_AbilityRuntime_MoeContext_StartSelfUIAbility(
    OH_AbilityRuntime_MoeContextHandle context, AbilityBase_Want *want);

/**
 * @brief Starts self UIAbility with start options.
 *
 * @permission {@code ohos.permission.NDK_START_SELF_UI_ABILITY}
 * @param context Represents a pointer to modular object extension ability context.
 * @param want The arguments passed to start self UIAbility.
 * @param options The start options passed to start self UIAbility.
 * @returns Returns a specific error code.
 * @since 26.0.0
 */
AbilityRuntime_Errcode OH_AbilityRuntime_MoeContext_StartSelfUIAbilityWithStartOptions(
    OH_AbilityRuntime_MoeContextHandle context, AbilityBase_Want *want, AbilityRuntime_StartOptions *options);

/**
 * @brief Destroys this modular object extension.
 *
 * @param context Represents a pointer to modular object extension ability context.
 * @returns Returns a specific error code.
 * @since 26.0.0
 */
AbilityRuntime_Errcode OH_AbilityRuntime_MoeContext_TerminateSelf(OH_AbilityRuntime_MoeContextHandle context);

#ifdef __cplusplus
}
#endif

/** @} */
#endif // Ability_RUNTIME_MODULAR_OBJECT_EXTENSION_CONTEXT_H
