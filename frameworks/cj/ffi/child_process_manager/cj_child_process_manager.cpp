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

#include <unistd.h>
#include "cj_child_process_manager.h"

namespace OHOS {
namespace AbilityRuntime {
int32_t CJChildProcessManager::OnStartChildProcess(const std::string& srcEntry, const int32_t startMode, pid_t& result)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "called");
    if (ChildProcessManager::GetInstance().IsChildProcess()) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Already in child process");
        return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_OPERATION_NOT_SUPPORTED);
    }

    TAG_LOGD(AAFwkTag::PROCESSMGR, "StartMode: %{public}d", startMode);
    if (startMode != MODE_SELF_FORK && startMode != MODE_APP_SPAWN_FORK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Invalid StartMode");
        return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
    }

    if (startMode == MODE_SELF_FORK) {
        return StartChildProcessSelfForkTask(result, srcEntry);
    } else {
        return StartChildProcessAppSpawnForkTask(result, srcEntry);
    }
    return 0;
}

int32_t CJChildProcessManager::StartChildProcessSelfForkTask(pid_t &result, const std::string &srcEntry)
{
        ChildProcessManagerErrorCode errorCode =
            ChildProcessManager::GetInstance().StartChildProcessBySelfFork(srcEntry, pid);
        if (errorCode != ChildProcessManagerErrorCode::ERR_OK) {
            return static_cast<int32_t>(ChildProcessManagerErrorUtil::GetAbilityErrorCode(errorCode));
        }
        return 0;
}

int32_t CJChildProcessManager::StartChildProcessAppSpawnForkTask(pid_t &result, const std::string &srcEntry)
{
    auto innerErrorCode = std::make_shared<ChildProcessManagerErrorCode>(ChildProcessManagerErrorCode::ERR_OK);
    *innerErrorCode = ChildProcessManager::GetInstance().StartCJChildProcessByAppSpawnFork(srcEntry,result);
    if (*innerErrorCode != ChildProcessManagerErrorCode::ERR_OK) {
        return static_cast<int32_t>(ChildProcessManagerErrorUtil::GetAbilityErrorCode(*innerErrorCode));
    }
    return 0;
}

int32_t CJChildProcessManager::OnStartArkChildProcess(const std::string& srcEntry, const std::string& entryParams, const std::map<std::string, int32_t>& fds, const bool isolationMode, pid_t& result)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "called");
    if (ChildProcessManager::GetInstance().IsChildProcessBySelfFork()) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Already in child process");
        return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_OPERATION_NOT_SUPPORTED);
    }

    AppExecFwk::ChildProcessArgs args;
    AppExecFwk::ChildProcessOptions options;
    args.entryParams = entryParams;
    args.fds = fds;
    options.isolationMode = isolationMode;

    auto innerErrorCode = std::make_shared<ChildProcessManagerErrorCode>(ChildProcessManagerErrorCode::ERR_OK);
    *innerErrorCode = ChildProcessManager::GetInstance().StartChildProcessWithArgs(srcEntry, result,
            AppExecFwk::CHILD_PROCESS_TYPE_ARK, args, options);
    if (*innerErrorCode != ChildProcessManagerErrorCode::ERR_OK) {
        return static_cast<int32_t>(ChildProcessManagerErrorUtil::GetAbilityErrorCode(*innerErrorCode));
    }
    return 0;
}

extern "C" {
int32_t FFICJStartChildProcess(const char* srcEntry, const int32_t startMode, pid_t* result)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "called");
    return CJChildProcessManager::OnStartChildProcess(srcEntry, startMode, *result);
}

int32_t FFICJStartArkChildProcess(const char* srcEntry, const char* entryParams, const CArrStrInt32 fds, const bool isolationMode, pid_t* result)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "called");
    std::map<std::string, int32_t> fdsMap;
    for(int64_t i = 0; i < fds.keys.size; i++) {
        fdsMap[fds.keys.head[i]] = fds.values.head[i];
    }
    return CJChildProcessManager::OnStartArkChildProcess(srcEntry, entryParams, fdsMap, isolationMode, *result);
}
}
}
}