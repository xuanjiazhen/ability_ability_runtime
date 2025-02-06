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

#ifndef OHOS_ABILITY_RUNTIME_CJ_CHILD_PROCESS_MANAGER_H
#define OHOS_ABILITY_RUNTIME_CJ_CHILD_PROCESS_MANAGER_H

#include <cstdalign>
#include <unistd.h>

#include "child_process_manager.h"
#include "child_process_manager_error_utils.h"
#include "cj_common_ffi.h"
#include "hilog_tag_wrapper.h"

#ifdef WINDOWS_PLATFORM
#define CJ_EXPORT __declspec(dllexport)
#else
#define CJ_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
enum {
    MODE_SELF_FORK = 0,
    MODE_APP_SPAWN_FORK = 1,
};
struct CJChildProcessParam {
    std::string srcEntry;
    AppExecFwk::ChildProcessArgs args;
    AppExecFwk::ChildProcessOptions options;
    int32_t childProcessType;
};
struct CArrStrInt32 {
    CArrString keys;
    CArrI32 values;
}
} // namespace

extern "C" {
CJ_EXPORT int32_t FFICJStartChildProcess(const char* srcEntry, const int32_t startMode, pid_t* result);
CJ_EXPORT int32_t FFICJStartArkChildProcess(
    const char* srcEntry, const char* entryParams, const CArrStrInt32 fds, const bool isolationMode, pid_t* result);
};

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
}
} // namespace AbilityRuntime
} // namespace OHOS

#endif