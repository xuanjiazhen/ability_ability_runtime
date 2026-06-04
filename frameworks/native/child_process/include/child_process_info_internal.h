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

#ifndef ABILITY_RUNTIME_CHILD_PROCESS_INFO_INTERNAL_H
#define ABILITY_RUNTIME_CHILD_PROCESS_INFO_INTERNAL_H

#include <cstdlib>
#include <new>
#include <vector>
#include "child_process_info.h"            // public C API header (opaque type + accessors)
#include "appmgr/child_process_info.h"     // inner API (ChildProcessInfo struct)

struct ChildProcessInfoItem {
    int32_t pid;
    int32_t hostPid;
    char* processName;
};

struct Ability_ChildProcessInfos {
    ChildProcessInfoItem* items;
    uint32_t count;
};

bool CreateAndFillChildProcessInfos(
    const std::vector<OHOS::AppExecFwk::ChildProcessInfo>& childInfos,
    Ability_ChildProcessInfos** infos, uint32_t* count)
{
    *count = static_cast<uint32_t>(childInfos.size());
    if (childInfos.empty()) {
        *infos = nullptr;
        return true;
    }
    auto* result = new (std::nothrow) Ability_ChildProcessInfos();
    if (result == nullptr) {
        return false;
    }
    result->count = *count;
    result->items = new (std::nothrow) ChildProcessInfoItem[result->count];
    if (result->items == nullptr) {
        delete result;
        *infos = nullptr;
        *count = 0;
        return false;
    }
    for (uint32_t i = 0; i < result->count; i++) {
        result->items[i].pid = childInfos[i].pid;
        result->items[i].hostPid = childInfos[i].hostPid;
        result->items[i].processName = strdup(childInfos[i].processName.c_str());
        if (result->items[i].processName == nullptr) {
            for (uint32_t j = 0; j < i; j++) {
                free(result->items[j].processName);
            }
            delete[] result->items;
            delete result;
            *infos = nullptr;
            *count = 0;
            return false;
        }
    }
    *infos = result;
    return true;
}

#endif // ABILITY_RUNTIME_CHILD_PROCESS_INFO_INTERNAL_H
