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

/**
 * @addtogroup AbilityRuntime
 * @{
 *
 * @brief Describe the functions of want.
 *
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 14
 */

/**
 * @file cwant.h
 *
 * @brief Defines the want APIs.
 *
 * @library libability_runtime_want.so
 * @kit AbilityKit
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 14
 */

#ifndef ABILITY_RUNTIME_WANT_H
#define ABILITY_RUNTIME_WANT_H

#include <stddef.h>
#include <stdint.h>
#include "ability_runtime_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Indicates information of element.
 *
 * @since 14
 */
typedef struct AbilityRuntime_Element {
    /** Indicates the name of application. */
    char* bundleName;
    /** Indicates the name of module. */
    char* moduleName;
    /** Indicates the name of ability. */
    char* abilityName;
} AbilityRuntime_Element;

struct AbilityRuntime_Want;
typedef struct AbilityRuntime_Want AbilityRuntime_Want;

/**
 * @brief Creat want.
 *
 * @param element Information of element.
 * @return Returns the newly created AbilityRuntime_Want object.
 *
 * @since 14
 */
AbilityRuntime_Want* OH_AbilityRuntime_NewWant(AbilityRuntime_Element element);

/**
 * @brief Delete input want.
 *
 * @param want The want to be deleted.
 * @return The error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the want is invalid.
 * @since 14
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_DeleteWant(AbilityRuntime_Want* want);

/**
 * @brief Set want element.
 *
 * @param want The want that needs to be set element.
 * @param element Information of element.
 * @return The error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the want is invalid.
 * @since 14
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_SetWantElement(AbilityRuntime_Want* want, AbilityRuntime_Element element);

/**
 * @brief Get want element.
 *
 * @param want The want for the element that has been obtained.
 * @param element The element obtained from want.
 * @return The error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the want or element is invalid.
 * @since 14
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetWantElement(AbilityRuntime_Want* want, AbilityRuntime_Element* element);

/**
 * @brief Set want char param.
 *
 * @param want The want needs to be set char param.
 * @param key The key of char param.
 * @param value The value of char param.
 * @return The error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the input parameters are invalid.
 * @since 14
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_SetWantCharParam(AbilityRuntime_Want* want, const char* key, const char* value);

/**
 * @brief Get want char param.
 *
 * @param want The want for the char param that has been obtained.
 * @param key The key of char param.
 * @param value The value of char param.
 * @param valueSize Size of the value.
 * @return The error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the input parameters are invalid.
 * @since 14
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetWantCharParam(AbilityRuntime_Want* want, const char* key,
    char* value, size_t valueSize);

/**
 * @brief Add fd to want.
 *
 * @param want The want needs to be add fd.
 * @param key The key of the fd.
 * @param fd File Descriptor.
 * @return The error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the input parameters are invalid.
 * @since 14
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_AddWantFd(AbilityRuntime_Want* want, const char* key, int32_t fd);

/**
 * @brief Get fd from want.
 *
 * @param want The want that includes fd.
 * @param key The key of the fd.
 * @param fd File Descriptor.
 * @return The error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the input parameters are invalid.
 * @since 14
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetWantFd(AbilityRuntime_Want* want, const char* key, int32_t* fd);

#ifdef __cplusplus
} // extern "C"
#endif

/** @} */
#endif // ABILITY_RUNTIME_WANT_H
