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
 * @brief Provide the C interface for the modular object extension ability.
 *
 * @since 26.0.0
 */

/**
 * @file modular_object_extension_ability.h
 *
 * @brief Declare the modular object extension ability.
 *
 * @library libmodular_object_extension.so
 * @kit AbilityKit
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 26.0.0
 */

#ifndef Ability_RUNTIME_MODULAR_OBJECT_EXTENSION_ABILITY_H
#define Ability_RUNTIME_MODULAR_OBJECT_EXTENSION_ABILITY_H

#include "ability_runtime_common.h"
#include "extension_ability.h"
#include "ipc_cparcel.h"
#include "modular_object_extension_context.h"
#include "want.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Define the OH_AbilityRuntime_ModularObject_ExtensionInstance structure type.
 *
 * @since 26.0.0
 */
struct OH_AbilityRuntime_ModularObject_ExtensionInstance;

/**
 * @brief Define the OH_AbilityRuntime_MoeInstance structure type.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_ModularObject_ExtensionInstance OH_AbilityRuntime_MoeInstance;

/**
 * @brief Defines the pointer to OH_AbilityRuntime_MoeInstance.
 *
 * @since 26.0.0
 */
typedef OH_AbilityRuntime_MoeInstance* OH_AbilityRuntime_MoeInstanceHandle;

/**
 * @brief Called back when a modular object extension is started for initialization.
 *
 * @param instance Represents a pointer to an {@link OH_AbilityRuntime_MoeInstance} instance.
 * @param want Indicates the want of created modular object extension.
 *
 * @since 26.0.0
 */
typedef void (*OH_AbilityRuntime_MoeAbility_OnCreateFunc)(
    OH_AbilityRuntime_MoeInstanceHandle instance, AbilityBase_Want *want);

/**
 * @brief Called back before a modular object extension is destroyed.
 *
 * @param instance Represents a pointer to an {@link OH_AbilityRuntime_MoeInstance} instance.
 *
 * @since 26.0.0
 */
typedef void (*OH_AbilityRuntime_MoeAbility_OnDestroyFunc)(OH_AbilityRuntime_MoeInstanceHandle instance);

/**
 * @brief Called back when a modular object extension is connected to an ability.
 *
 * @param instance Represents a pointer to an {@link OH_AbilityRuntime_MoeInstance} instance.
 * @param want Indicates the want of created modular object extension.
 *
 * @since 26.0.0
 */
typedef OHIPCRemoteStub* (*OH_AbilityRuntime_MoeAbility_OnConnectFunc)(
    OH_AbilityRuntime_MoeInstanceHandle instance, AbilityBase_Want *want);

/**
 * @brief Called back when all abilities connected to a modular object extension are disconnected.
 *
 * @param instance Represents a pointer to an {@link OH_AbilityRuntime_MoeInstance} instance.
 *
 * @since 26.0.0
 */
typedef void (*OH_AbilityRuntime_MoeAbility_OnDisconnectFunc)(OH_AbilityRuntime_MoeInstanceHandle instance);

/**
 * @brief Register function {@link OH_AbilityRuntime_MoeAbility_OnCreateFunc} into
 * {@link OH_AbilityRuntime_MoeInstance}.
 *
 * @param instance Represents a pointer to an {@link OH_AbilityRuntime_MoeInstance} instance.
 * @param onCreateFunc Represents callback function.
 * @return Returns a specific error code.
 * @since 26.0.0
 */
AbilityRuntime_Errcode OH_AbilityRuntime_MoeAbility_RegisterOnCreateFunc(
    OH_AbilityRuntime_MoeInstanceHandle instance, OH_AbilityRuntime_MoeAbility_OnCreateFunc onCreateFunc);

/**
 * @brief Register function {@link OH_AbilityRuntime_MoeAbility_OnDestroyFunc} into
 * {@link OH_AbilityRuntime_MoeInstance}.
 *
 * @param instance Represents a pointer to an {@link OH_AbilityRuntime_MoeInstance} instance.
 * @param onDestroyFunc Represents callback function.
 * @return Returns a specific error code.
 * @since 26.0.0
 */
AbilityRuntime_Errcode OH_AbilityRuntime_MoeAbility_RegisterOnDestroyFunc(
    OH_AbilityRuntime_MoeInstanceHandle instance, OH_AbilityRuntime_MoeAbility_OnDestroyFunc onDestroyFunc);

/**
 * @brief Register function {@link OH_AbilityRuntime_MoeAbility_OnConnectFunc} into
 * {@link OH_AbilityRuntime_MoeInstance}.
 *
 * @param instance Represents a pointer to an {@link OH_AbilityRuntime_MoeInstance} instance.
 * @param onConnectFunc Represents callback function.
 * @return Returns a specific error code.
 * @since 26.0.0
 */
AbilityRuntime_Errcode OH_AbilityRuntime_MoeAbility_RegisterOnConnectFunc(
    OH_AbilityRuntime_MoeInstanceHandle instance, OH_AbilityRuntime_MoeAbility_OnConnectFunc onConnectFunc);

/**
 * @brief Register function {@link OH_AbilityRuntime_MoeAbility_OnDisconnectFunc} into
 * {@link OH_AbilityRuntime_MoeInstance}.
 *
 * @param instance Represents a pointer to an {@link OH_AbilityRuntime_MoeInstance} instance.
 * @param onDisconnectFunc Represents callback function.
 * @return Returns a specific error code.
 * @since 26.0.0
 */
AbilityRuntime_Errcode OH_AbilityRuntime_MoeAbility_RegisterOnDisconnectFunc(
    OH_AbilityRuntime_MoeInstanceHandle instance, OH_AbilityRuntime_MoeAbility_OnDisconnectFunc onDisconnectFunc);

/**
 * @brief Get the extension context from the modular object extension instance.
 *
 * @param instance Represents a pointer to an {@link OH_AbilityRuntime_MoeInstance} instance.
 * @param context Represents output modular object extension context.
 * @returns Returns a specific error code.
 * @since 26.0.0
 */
AbilityRuntime_Errcode OH_AbilityRuntime_MoeAbility_GetContextFromInstance(
    OH_AbilityRuntime_MoeInstanceHandle instance, OH_AbilityRuntime_MoeContextHandle* context);

/**
 * @brief Get the modular object extension instance from base extension instance.
 *
 * @param baseExtensionInstance Represents a pointer to a base extension instance.
 * @param moExtensionInstance Represents output modular object extension instance.
 * @returns Returns a specific error code.
 * @since 26.0.0
 */
AbilityRuntime_Errcode OH_AbilityRuntime_MoeAbility_GetInstanceFromBase(
    AbilityRuntime_ExtensionInstanceHandle baseExtensionInstance,
    OH_AbilityRuntime_MoeInstanceHandle* moExtensionInstance);

#ifdef __cplusplus
}
#endif

/** @} */
#endif // Ability_RUNTIME_MODULAR_OBJECT_EXTENSION_ABILITY_H
