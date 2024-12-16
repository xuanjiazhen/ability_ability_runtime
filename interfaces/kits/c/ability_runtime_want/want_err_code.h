/*
* Copyright (C) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
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
 * @brief 
 *
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 
 */

/**
 * @file want_err_code.h
 *
 * @brief
 *
 * 
 * @kit AbilityKit
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 
 */

#ifndef ABILITY_RUNTIME_COMMON_H
#define ABILITY_RUNTIME_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumerates the error codes.
 *
 * @since 13
 */
typedef enum {
    /** @error No error. */
    WANT_ERROR_CODE_NO_ERROR = 0,
    /** @error Invalid parameters. */
    WANT_ERROR_CODE_PARAM_INVALID = 401,
    WANT_ERROR_CODE_CLOSE_FD_ERROR = 16000154, //TODO
    WANT_ERROR_CODE_KEY_NOT_FOUND = 16000155,
    WANT_ERROR_CODE_BUFFER_TOO_SMALL = 16000156,

} Want_ErrorCode;

#ifdef __cplusplus
}
#endif

/** @} */
#endif // ABILITY_RUNTIME_COMMON_H
