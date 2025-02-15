/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APPEXECFWK_PROCESS_UTIL_H
#define OHOS_ABILITY_RUNTIME_APPEXECFWK_PROCESS_UTIL_H

#include <list>
#include <sys/stat.h>

#include "file_ex.h"
#include "hilog_tag_wrapper.h"
#include "securec.h"
#include "simple_process_info.h"

namespace OHOS {
namespace AppExecFwk {
namespace ProcessUtil {

static bool ProcessExist(pid_t pid)
{
    char pid_path[128] = {0};
    struct stat stat_buf;
    if (!pid) {
        return false;
    }
    if (snprintf_s(pid_path, sizeof(pid_path), sizeof(pid_path) - 1, "/proc/%d/status", pid) < 0) {
        return false;
    }
    if (stat(pid_path, &stat_buf) == 0) {
        return true;
    }
    return false;
}

static bool ProcessExist(pid_t pid, const std::string &processName)
{
    char pid_path[128] = {0};
    struct stat stat_buf;
    if (!pid) {
        return false;
    }
    if (snprintf_s(pid_path, sizeof(pid_path), sizeof(pid_path) - 1, "/proc/%d/comm", pid) < 0) {
        return false;
    }
    if (stat(pid_path, &stat_buf) != 0) {
        return false;
    }
    std::string pidProcessName;
    std::string commPath = pid_path;
    OHOS::LoadStringFromFile(commPath, pidProcessName);
    return pidProcessName == processName;
}

static bool CheckAllProcessExit(std::list<SimpleProcessInfo> &processInfos)
{
    for (auto iter = processInfos.begin(); iter != processInfos.end();) {
        if (!ProcessExist((*iter).pid, (*iter).processName)) {
            iter = processInfos.erase(iter);
        } else {
            iter++;
        }
    }
    return processInfos.empty();
}

static bool CheckAllProcessExit(std::list<pid_t> &pids)
{
    for (auto iter = pids.begin(); iter != pids.end();) {
        if (!ProcessExist(*iter)) {
            iter = pids.erase(iter);
        } else {
            iter++;
        }
    }
    return pids.empty();
}

static bool IsAllProcessKilled(std::list<SimpleProcessInfo> &processInfos)
{
    bool processExists = false;
    for (auto &item : processInfos) {
        if (ProcessExist(item.pid, item.processName)) {
            return false;
        }
    }
    return true;
}
}  // namespace ProcessUtil
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_APPEXECFWK_PROCESS_UTIL_H
