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

#include "child_process_info_internal.h"

#define NCP_ERR_INVALID_PARAM 401

int32_t OH_Ability_ChildProcessInfos_GetPid(
    Ability_ChildProcessInfos* infos, uint32_t index, int32_t* pid)
{
    if (infos == nullptr || pid == nullptr || index >= infos->count) {
        return NCP_ERR_INVALID_PARAM;
    }
    *pid = infos->items[index].pid;
    return 0; // NCP_NO_ERROR
}

int32_t OH_Ability_ChildProcessInfos_GetHostPid(
    Ability_ChildProcessInfos* infos, uint32_t index, int32_t* hostPid)
{
    if (infos == nullptr || hostPid == nullptr || index >= infos->count) {
        return NCP_ERR_INVALID_PARAM;
    }
    *hostPid = infos->items[index].hostPid;
    return 0; // NCP_NO_ERROR
}

int32_t OH_Ability_ChildProcessInfos_GetProcessName(
    Ability_ChildProcessInfos* infos, uint32_t index, const char** processName)
{
    if (infos == nullptr || processName == nullptr || index >= infos->count) {
        return NCP_ERR_INVALID_PARAM;
    }
    *processName = infos->items[index].processName;
    return 0; // NCP_NO_ERROR
}

void OH_Ability_DestroyChildProcessInfos(Ability_ChildProcessInfos* infos)
{
    if (infos == nullptr) {
        return;
    }
    for (uint32_t i = 0; i < infos->count; i++) {
        free(infos->items[i].processName);
    }
    delete[] infos->items;
    delete infos;
}
