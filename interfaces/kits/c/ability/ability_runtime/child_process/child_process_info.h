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
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 27
 */

/**
 * @file child_process_info.h
 *
 * @brief Defines the child process info collection type and accessor functions.
 *
 * @library libchild_process.so
 * @kit AbilityKit
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 27
 */

#ifndef ABILITY_RUNTIME_CHILD_PROCESS_INFO_H
#define ABILITY_RUNTIME_CHILD_PROCESS_INFO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Defines a struct for the child process infos (opaque type).
 *        Obtained via {@link OH_Ability_AcquireChildProcesses} or
 *        {@link OH_AbilityRuntime_AcquireUIAbilityChildProcesses}.
 *        Released via {@link OH_Ability_DestroyChildProcessInfos}.
 * @since 27
 */
typedef struct Ability_ChildProcessInfos Ability_ChildProcessInfos;

/**
 * @brief Get PID of child process at specified index.
 *
 * @param infos Pointer to child process info collection. Must not be nullptr.
 * @param index Index of child process, range [0, count).
 * @param pid Output parameter, returns child process PID. Must not be nullptr.
 * @return Returns 0 if the operation is successful.
 *         Returns 401 if parameters are invalid.
 * @since 27
 */
int32_t OH_Ability_ChildProcessInfos_GetPid(
    Ability_ChildProcessInfos* infos, uint32_t index, int32_t* pid);

/**
 * @brief Get host PID of child process at specified index.
 *
 * @param infos Pointer to child process info collection. Must not be nullptr.
 * @param index Index of child process, range [0, count).
 * @param hostPid Output parameter, returns host process PID. Must not be nullptr.
 * @return Returns 0 if the operation is successful.
 *         Returns 401 if parameters are invalid.
 * @since 27
 */
int32_t OH_Ability_ChildProcessInfos_GetHostPid(
    Ability_ChildProcessInfos* infos, uint32_t index, int32_t* hostPid);

/**
 * @brief Get process name of child process at specified index.
 *
 * Returned pointer is valid until {@link OH_Ability_DestroyChildProcessInfos} is called.
 * Caller must NOT free this pointer.
 *
 * @param infos Pointer to child process info collection. Must not be nullptr.
 * @param index Index of child process, range [0, count).
 * @param processName Output parameter, returns process name string. Must not be nullptr.
 * @return Returns 0 if the operation is successful.
 *         Returns 401 if parameters are invalid.
 * @since 27
 */
int32_t OH_Ability_ChildProcessInfos_GetProcessName(
    Ability_ChildProcessInfos* infos, uint32_t index, const char** processName);

/**
 * @brief Destroy child process info collection and release memory.
 *
 * @param infos Pointer allocated by {@link OH_Ability_AcquireChildProcesses} or
 *              {@link OH_AbilityRuntime_AcquireUIAbilityChildProcesses}. Passing nullptr is safe.
 * @since 27
 */
void OH_Ability_DestroyChildProcessInfos(Ability_ChildProcessInfos* infos);

#ifdef __cplusplus
}
#endif

/** @} */
#endif // ABILITY_RUNTIME_CHILD_PROCESS_INFO_H
