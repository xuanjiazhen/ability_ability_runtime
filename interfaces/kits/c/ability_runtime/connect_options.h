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
 * @brief Provide the C interface for extension connect options.
 *
 * @since 26.0.0
 */

/**
 * @file connect_options.h
 *
 * @brief Declare the options of connect extension ability.
 *
 * @library libability_runtime.so
 * @kit AbilityKit
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 26.0.0
 */

#ifndef ABILITY_RUNTIME_CONNECT_OPTIONS_H
#define ABILITY_RUNTIME_CONNECT_OPTIONS_H

#include <stdint.h>

#include "ability_runtime_common.h"
#include "ipc_cparcel.h"
#include "want.h"

#ifdef __cplusplus
extern "C" {
#endif

struct AbilityBase_Want;
typedef struct AbilityBase_Want AbilityBase_Want;

struct AbilityBase_Element;
typedef struct AbilityBase_Element AbilityBase_Element;

/**
 * @brief Define the OH_AbilityRuntime_ConnectOptions structure type.
 *
 * @since 26.0.0
 */
struct OH_AbilityRuntime_ConnectOptions;

/**
 * @brief Define the OH_AbilityRuntime_ConnectOptions structure type.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_ConnectOptions OH_AbilityRuntime_ConnectOptions;

/**
 * @brief The callback interface for successful connection.
 *
 * @param connectOptions Represents a pointer to an {@link OH_AbilityRuntime_ConnectOptions} instance.
 * @param element Represents the element name of the modular object extension ability.
 * @param proxy Represents the remote object proxy.
 *
 * @since 26.0.0
 */
typedef void (*OH_AbilityRuntime_ConnectOptions_OnConnectCallback)(
    OH_AbilityRuntime_ConnectOptions *connectOptions, AbilityBase_Element *element, OHIPCRemoteProxy *proxy);

/**
 * @brief The callback interface for successful disconnection.
 *
 * @param connectOptions Represents a pointer to an {@link OH_AbilityRuntime_ConnectOptions} instance.
 * @param element Represents the element name of the modular object extension ability.
 *
 * @since 26.0.0
 */
typedef void (*OH_AbilityRuntime_ConnectOptions_OnDisconnectCallback)(
    OH_AbilityRuntime_ConnectOptions *connectOptions, AbilityBase_Element *element);

/**
 * @brief The callback interface for connection failure.
 *
 * @param connectOptions Represents a pointer to an {@link OH_AbilityRuntime_ConnectOptions} instance.
 * @param code Represents the business error code.
 *
 * @since 26.0.0
 */
typedef void (*OH_AbilityRuntime_ConnectOptions_OnFailedCallback)(
    OH_AbilityRuntime_ConnectOptions *connectOptions, int32_t code);

/**
 * @brief Create connect options.
 *
 * @return Returns the newly created OH_AbilityRuntime_ConnectOptions object.
 *
 * @since 26.0.0
 */
OH_AbilityRuntime_ConnectOptions* OH_AbilityRuntime_CreateConnectOptions(void);

/**
 * @brief Destroy input connect options.
 *
 * @param connectOptions The options to be deleted.
 * @return Returns a specific error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the connectOptions is invalid.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_DestroyConnectOptions(OH_AbilityRuntime_ConnectOptions *connectOptions);

/**
 * @brief Register function {@link OH_AbilityRuntime_ConnectOptions_OnConnectCallback} into
 * {@link OH_AbilityRuntime_ConnectOptions}.
 *
 * @param connectOptions Represents a pointer to an {@link OH_AbilityRuntime_ConnectOptions} instance.
 * @param onConnectCallback Represents callback function.
 * @return Returns a specific error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} - success.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} - parameter check failed.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ConnectOptions_RegisterOnConnectCallback(
    OH_AbilityRuntime_ConnectOptions *connectOptions,
    OH_AbilityRuntime_ConnectOptions_OnConnectCallback onConnectCallback);

/**
 * @brief Register function {@link OH_AbilityRuntime_ConnectOptions_OnDisconnectCallback} into
 * {@link OH_AbilityRuntime_ConnectOptions}.
 *
 * @param connectOptions Represents a pointer to an {@link OH_AbilityRuntime_ConnectOptions} instance.
 * @param onDisconnectCallback Represents callback function.
 * @return Returns a specific error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} - success.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} - parameter check failed.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ConnectOptions_RegisterOnDisconnectCallback(
    OH_AbilityRuntime_ConnectOptions *connectOptions,
    OH_AbilityRuntime_ConnectOptions_OnDisconnectCallback onDisconnectCallback);

/**
 * @brief Register function {@link OH_AbilityRuntime_ConnectOptions_OnFailedCallback} into
 * {@link OH_AbilityRuntime_ConnectOptions}.
 *
 * @param connectOptions Represents a pointer to an {@link OH_AbilityRuntime_ConnectOptions} instance.
 * @param onFailedCallback Represents callback function.
 * @return Returns a specific error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} - success.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} - parameter check failed.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ConnectOptions_RegisterOnFailedCallback(
    OH_AbilityRuntime_ConnectOptions *connectOptions,
    OH_AbilityRuntime_ConnectOptions_OnFailedCallback onFailedCallback);

#ifdef __cplusplus
}
#endif

/** @} */
#endif // ABILITY_RUNTIME_CONNECT_OPTIONS_H
