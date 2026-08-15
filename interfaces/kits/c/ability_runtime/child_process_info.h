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
 * @brief Defines the child process info types and accessor APIs.
 *
 * @since 26.0.0
 */

/**
 * @file child_process_info.h
 *
 * @brief Defines the child process info types and accessor functions.
 *
 * @library libability_runtime.so
 * @kit AbilityKit
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 26.0.0
 */

#ifndef ABILITY_RUNTIME_CHILD_PROCESS_INFO_H
#define ABILITY_RUNTIME_CHILD_PROCESS_INFO_H

#include <stdint.h>
#include "ability_runtime_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Defines the pointer to OH_AbilityRuntime_ChildProcessInfos.
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_ChildProcessInfos* OH_AbilityRuntime_ChildProcessInfosHandle;

/**
 * @brief Defines the pointer to OH_AbilityRuntime_ChildProcessInfo.
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_ChildProcessInfo* OH_AbilityRuntime_ChildProcessInfoHandle;

/**
 * @brief Retrieves a specific child process info handle from the collection by its index.
 *
 * @param infos Information about all child processes within the self application.
 * @param index The index of the child process info to retrieve. Must be strictly less than the count.
 * @param info The retrieved single child process info handle for the specified index.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetChildProcessInfoByIndex(
    OH_AbilityRuntime_ChildProcessInfosHandle infos, uint32_t index,
    OH_AbilityRuntime_ChildProcessInfoHandle* info);

/**
 * @brief Gets PID of child process info.
 *
 * @param info Pointer to child process info. Must not be nullptr.
 * @param pid Output parameter, returns child process PID. Must not be nullptr.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ChildProcessInfo_GetPid(
    OH_AbilityRuntime_ChildProcessInfoHandle info, int32_t* pid);

/**
 * @brief Gets parent PID of child process info.
 *
 * @param info Pointer to child process info. Must not be nullptr.
 * @param parentPid Output parameter, returns parent process PID. Must not be nullptr.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ChildProcessInfo_GetParentPid(
    OH_AbilityRuntime_ChildProcessInfoHandle info, int32_t* parentPid);

/**
 * @brief Gets process name of child process info.
 *
 * @param info Pointer to child process info. Must not be nullptr.
 * @param processName Indicates a buffer to receive process name.
 * @param cMaxProcessName Indicates size of buffer in bytes, including the null terminator.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ChildProcessInfo_GetProcessName(
    OH_AbilityRuntime_ChildProcessInfoHandle info, char* processName, uint32_t cMaxProcessName);

/**
 * @brief Releases child process info collection.
 *
 * @param infos The child process infos to be released.
 * @since 26.0.0
 */
void OH_AbilityRuntime_ReleaseChildProcessInfos(OH_AbilityRuntime_ChildProcessInfosHandle* infos);

#ifdef __cplusplus
}
#endif

/** @} */
#endif // ABILITY_RUNTIME_CHILD_PROCESS_INFO_H
