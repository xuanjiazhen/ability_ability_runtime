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

#ifndef OHOS_ABILITY_RUNTIME_CJ_CHILD_PROCESS_MANAGER_H
#define OHOS_ABILITY_RUNTIME_CJ_CHILD_PROCESS_MANAGER_H

#include <unistd.h>

#include "child_process_manager.h"
#include "child_process_manager_error_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

class CJChildProcessManager {
public:
    CJChildProcessManager() = default;
    ~CJChildProcessManager() = default;

    static int32_t OnStartChildProcess(const std::string& srcEntry, const int32_t startMode, pid_t& result);

    static int32_t OnStartArkChildProcess(const std::string& srcEntry, const std::string& entryParams,
        const std::map<std::string, int32_t>& fds, const bool isolationMode, pid_t& result);

private:
    static int32_t StartChildProcessSelfForkTask(pid_t& result, const std::string& srcEntry);
    static int32_t StartChildProcessAppSpawnForkTask(pid_t& result, const std::string& srcEntry);
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif