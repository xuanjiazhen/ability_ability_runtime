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

#include "native_child_process.h"
#include <map>
#include <mutex>
#include "hilog_tag_wrapper.h"
#include "native_child_callback.h"
#include "child_process_args_manager.h"
#include "child_process_manager.h"
#include "child_callback_manager.h"
#include "child_process_manager_error_utils.h"

using namespace OHOS;
using namespace OHOS::AbilityRuntime;

namespace {

std::mutex g_mutexCallBackObj;
constexpr size_t MAX_KEY_SIZE = 20;
constexpr size_t MAX_FD_SIZE = 16;

} // Anonymous namespace

int OH_Ability_CreateNativeChildProcess(const char* libName, OH_Ability_OnNativeChildProcessStarted onProcessStarted)
{
    if (libName == nullptr || *libName == '\0' || onProcessStarted == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null libname or callback");
        return NCP_ERR_INVALID_PARAM;
    }

    std::string strLibName(libName);
    if (strLibName.find("../") != std::string::npos) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "relative path not allow");
        return NCP_ERR_INVALID_PARAM;
    }

    sptr<IRemoteObject> callbackStub(new (std::nothrow) NativeChildCallback(onProcessStarted));
    if (!callbackStub) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null callbackStub");
        return NCP_ERR_INTERNAL;
    }

    ChildProcessManager &mgr = ChildProcessManager::GetInstance();
    auto cpmErr = mgr.StartNativeChildProcessByAppSpawnFork(strLibName, callbackStub);
    if (cpmErr != ChildProcessManagerErrorCode::ERR_OK) {
        return ChildProcessManagerErrorUtil::CvtChildProcessManagerErrCode(cpmErr);
    }

    ChildCallbackManager::GetInstance().AddRemoteObject(callbackStub);
    return NCP_NO_ERROR;
}

Ability_NativeChildProcess_ErrCode OH_Ability_StartNativeChildProcess(const char* entry,
    NativeChildProcess_Args args, NativeChildProcess_Options options, int32_t *pid)
{
    if (entry == nullptr || *entry == '\0') {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Invalid entry");
        return NCP_ERR_INVALID_PARAM;
    }
    std::string entryName(entry);
    if (entryName.find(":") == std::string::npos) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "entry point misses a colon");
        return NCP_ERR_INVALID_PARAM;
    }
    if (pid == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "pid null");
        return NCP_ERR_INVALID_PARAM;
    }

    std::map<std::string, int32_t> fds;
    NativeChildProcess_Fd* cur = args.fdList.head;
    while (cur != nullptr) {
        if (!cur->fdName) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "fdName null");
            return NCP_ERR_INVALID_PARAM;
        }
        std::string key(cur->fdName);
        if (key.size() > MAX_KEY_SIZE) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "fd name too long");
            return NCP_ERR_INVALID_PARAM;
        }
        fds.emplace(key, cur->fd);
        cur = cur->next;
    }
    if (fds.size() > MAX_FD_SIZE) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "too many fds");
        return NCP_ERR_INVALID_PARAM;
    }
    AppExecFwk::ChildProcessArgs childArgs;
    childArgs.fds = fds;
    if (args.entryParams != nullptr && *(args.entryParams) != '\0') {
        std::string entryParams(args.entryParams);
        childArgs.entryParams = entryParams;
    }
    AppExecFwk::ChildProcessOptions childProcessOptions;
    childProcessOptions.isolationMode = options.isolationMode == NCP_ISOLATION_MODE_ISOLATED;
    int32_t childProcessType = AppExecFwk::CHILD_PROCESS_TYPE_NATIVE_ARGS;

    ChildProcessManager &mgr = ChildProcessManager::GetInstance();
    auto cpmErr = mgr.StartChildProcessWithArgs(entryName, *pid, childProcessType, childArgs, childProcessOptions);
    if (cpmErr != ChildProcessManagerErrorCode::ERR_OK) {
        return ChildProcessManagerErrorUtil::CvtChildProcessManagerErrCode(cpmErr);
    }
    return NCP_NO_ERROR;
}

NativeChildProcess_Args* OH_Ability_GetCurrentChildProcessArgs()
{
    NativeChildProcess_Args* result = ChildProcessArgsManager::GetInstance().GetChildProcessArgs();
    if (result == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "GetChildProcessArgs null");
    }
    return result;
}


Ability_ChildProcessConfigs* OH_Ability_CreateChildProcessConfigs()
{
    std::unique_ptr<Ability_ChildProcessConfigs> configs = std::make_unique<Ability_ChildProcessConfigs>();
    return configs.release();
}

Ability_NativeChildProcess_ErrCode OH_Ability_DestroyChildProcessConfigs(Ability_ChildProcessConfigs* configs)
{
    if(configs == nullptr){
        TAG_LOGE(AAFwkTag::APPKIT, "null Ability_ChildProcessConfigs");
        return NCP_ERR_INVALID_PARAM;
    }
    delete configs;
    configs = nullptr;
    return NCP_NO_ERROR;
}

Ability_NativeChildProcess_ErrCode OH_Ability_ChildProcessConfigs_SetIsolationMode(
    Ability_ChildProcessConfigs* configs, NativeChildProcess_IsolationMode isolationMode)
{
    if(configs == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null Ability_ChildProcessConfigs");
        return NCP_ERR_INVALID_PARAM;
    } else {
        configs->isolationMode = isolationMode;
        return NCP_NO_ERROR;
    }
}

Ability_NativeChildProcess_ErrCode OH_Ability_ChildProcessConfigs_SetProcessName(Ability_ChildProcessConfigs* configs,
    const char* processName)
{
    if (configs == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null Ability_ChildProcessConfigs");
        return NCP_ERR_INVALID_PARAM;
    }
    if (processName == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null processName");
        return NCP_ERR_INVALID_PARAM;
    }
    int processNameLen = strlen(processName);
    if(processNameLen == 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "processName is 0");
        return NCP_ERR_INVALID_PARAM;
    }
    if (processNameLen > 64) {
        TAG_LOGE(AAFwkTag::APPKIT, "processName is larger than 64");
        return NCP_ERR_INVALID_PARAM; 
    }
    int i = 0;
    while (i < processNameLen) {
        if((processName[i] >= 'a' && processName[i] <= 'z') ||
           (processName[i] >= 'A' && processName[i] <= 'Z') ||
           (processName[i] >= '0' && processName[i] <= '9') ||
           processName[i] == '_'){
                i++;
           }else{
                TAG_LOGE(AAFwkTag::APPKIT, "Wrong processName");
                return NCP_ERR_INVALID_PARAM;
           }
    }
    configs->processName = std::string(processName);
    return NCP_NO_ERROR;
}

Ability_NativeChildProcess_ErrCode OH_Ability_CreateNativeChildProcessWithConfigs(const char* libName,
    Ability_ChildProcessConfigs* configs, OH_Ability_OnNativeChildProcessStarted onProcessStarted)
{
    if (libName == nullptr || *libName == '\0' || onProcessStarted == nullptr || configs == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null libname or callback or configs");
        return NCP_ERR_INVALID_PARAM;
    }

    std::string strLibName(libName);
    if (strLibName.find("../") != std::string::npos) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "relative path not allow");
        return NCP_ERR_INVALID_PARAM;
    }

    sptr<IRemoteObject> callbackStub(new (std::nothrow) NativeChildCallback(onProcessStarted));
    if (!callbackStub) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null callbackStub");
        return NCP_ERR_INTERNAL;
    }

    ChildProcessManager &mgr = ChildProcessManager::GetInstance();
    auto cpmErr = mgr.StartNativeChildProcessByAppSpawnFork(strLibName, callbackStub, configs);
    if (cpmErr != ChildProcessManagerErrorCode::ERR_OK) {
        return ChildProcessManagerErrorUtil::CvtChildProcessManagerErrCode(cpmErr);
    }

    ChildCallbackManager::GetInstance().AddRemoteObject(callbackStub);
    return NCP_NO_ERROR;
}