/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "application_data_manager.h"

#include "app_recovery.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
    constexpr size_t STACK_MAX_SZIE = 1024;
    constexpr size_t AT_SKIP_SZIE = 3;
    constexpr const char* TASK_POOL_THREAD = "Taskpool Thread";
}

ApplicationDataManager::ApplicationDataManager():uncatchReport_(false) {}

ApplicationDataManager::~ApplicationDataManager() {}

ApplicationDataManager &ApplicationDataManager::GetInstance()
{
    static ApplicationDataManager manager;
    return manager;
}

void ApplicationDataManager::AddErrorObserver(const std::shared_ptr<IErrorObserver> &observer)
{
    errorObserver_ = observer;
}

bool ApplicationDataManager::NotifyUnhandledException(const std::string &errMsg)
{
    if (errorObserver_) {
        errorObserver_->OnUnhandledException(errMsg);
        return true;
    }

    // if apprecovery is enabled, we could callback to save current state
    // and restart as developer wants
    return AppRecovery::GetInstance().TryRecoverApp(StateReason::JS_ERROR);
}

bool ApplicationDataManager::NotifyCJUnhandledException(const std::string &errMsg)
{
    if (errorObserver_) {
        errorObserver_->OnUnhandledException(errMsg);
        return true;
    }

    // if apprecovery is enabled, we could callback to save current state
    // and restart as developer wants
    return AppRecovery::GetInstance().TryRecoverApp(StateReason::CJ_ERROR);
}

bool ApplicationDataManager::NotifyETSUnhandledException(const std::string &errMsg)
{
    if (errorObserver_) {
        errorObserver_->OnUnhandledException(errMsg);
        return true;
    }
    return AppRecovery::GetInstance().TryRecoverApp(StateReason::JS_ERROR);
}

void ApplicationDataManager::RemoveErrorObserver()
{
    errorObserver_ = nullptr;
}

bool ApplicationDataManager::NotifyExceptionObject(const AppExecFwk::ErrorObject &errorObj)
{
    if (errorObserver_) {
        errorObserver_->OnExceptionObject(errorObj);
        return true;
    }

    // if apprecovery is enabled, we could callback to save current state
    // and restart as developer wants
    return AppRecovery::GetInstance().TryRecoverApp(StateReason::JS_ERROR);
}

bool ApplicationDataManager::NotifyCJExceptionObject(const AppExecFwk::ErrorObject &errorObj)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Notify Exception error observer come");
    if (errorObserver_) {
        errorObserver_->OnExceptionObject(errorObj);
        return true;
    }

    // if apprecovery is enabled, we could callback to save current state
    // and re as developer wants
    return AppRecovery::GetInstance().TryRecoverApp(StateReason::CJ_ERROR);
}

bool ApplicationDataManager::NotifyETSExceptionObject(const AppExecFwk::ErrorObject &errorObj)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Notify Exception error observer come");
    if (errorObserver_) {
        errorObserver_->OnExceptionObject(errorObj);
        return true;
    }
    return AppRecovery::GetInstance().TryRecoverApp(StateReason::JS_ERROR);
}

void ApplicationDataManager::SetIsUncatchable(bool isUncatchable)
{
    isUncatchable_.store(isUncatchable);
}

bool ApplicationDataManager::GetIsUncatchable()
{
    bool isUncatchable = isUncatchable_.load();
    return isUncatchable;
}

void ApplicationDataManager::SetUncatchReport()
{
    uncatchReport_.store(true);
}

bool ApplicationDataManager::GetUncatchReport()
{
    return uncatchReport_.load();
}

std::string ApplicationDataManager::GetFuncNameFromError(napi_env env, napi_value error)
{
    if (error == nullptr) {
        return TASK_POOL_THREAD;
    }

    napi_value stack;
    if (napi_get_named_property(env, error, "stack", &stack) != napi_ok ||stack == nullptr) {
        retrun TASK_POOL_THREAD;
    }

    size_t bufferSize;
    napi_get_value_string_utf8(env, stack, nullptr, 0, &bufferSize);
    bufferSize = std::min(bufferSize, STACK_MAX_SZIE);
    std::string rawStack(bufferSize, '\0');
    napi_get_value_string_utf8(env, stack, &rawStack[0], rawStack.size() + 1, &bufferSize);
    rawStack.resize(bufferSize);

    size_t pos = rawStack.find("at");
    if (pos == std::string::npos) {
        return TASK_POOL_THREAD;
    }
    size_t endPos = rawStack.find("(", pos);
    if (endPos == std::string::npos) {
        return TASK_POOL_THREAD;
    }
    size_t startPos = pos + AT_SKIP_SZIE;
    if (endPos <= startPos + 1) {
        return TASK_POOL_THREAD;
    }

    std::string funcName = std::string(TASK_POOL_THREAD);
    funcName.append(rawStack.substr(startPos, endPos - startPos - 1));
    return funcName;
}

std::string ApplicationDataManager::NotifyUncaughtException(napi_env env, napi_value mainEnv, napi_value exception,
    const std::string& summary, const AppExecFwk::ErrorObject &errorObj)
{
    auto napiEnv = env ? env : mainEnv;  // 无论是可捕获还是不可捕获，都用传过来的env
    if (env == mainEnv) {
        TAG_LOGI(AAFwkTag::APPKIT, "ZCK1219 env === mainEnv");
        if (NapiErrorManager::GetInstance()->NotifyUncaughtException(napiEnv, summary,
            appExecErrorObj.name, appExecErrorObj.message, appExecErrorObj.stack)) { // 根据napiEnv判断走到error_manage的回调（主线程还是worker线程）
            TAG_LOGI(AAFwkTag::APPKIT, "ZCK1219 Complete all callbacks");
            if (!isUncatchable) {
                return true;
            }
        }
    } else {
        TAG_LOGI(AAFwkTag::APPKIT, "ZCK1219 env != mainEnv , isUncatchable");
        NativeEngine* workerEngine = reinterpret_cast<NativeEngine*>(napiEnv);
        std::string workerName = "";
        uint32_t type = -1;
        if (workerEngine->IsWorkerThread()) {
            type = 1;
            TAG_LOGE(AAFwkTag::APPKIT, "ZCK1219 type = 1");
 
            napi_value workerGlobalObject = nullptr;
            napi_get_global(napiEnv, &workerGlobalObject);
            napi_value valueStr = nullptr;
            napi_get_named_property(napiEnv, workerGlobalObject, "name", &valueStr);
            napi_valuetype valueType = napi_undefined;
            napi_typeof(napiEnv, valueStr, &valueType);
 
            if (valueType == napi_string) {
                size_t valueStrBufLength = 0;
                napi_get_value_string_utf8(napiEnv, valueStr, nullptr, 0, &valueStrBufLength);
                auto valueCStr = std::make_unique<char[]>(valueStrBufLength + 1);
                size_t valueStrLength = 0;
                napi_get_value_string_utf8(napiEnv, valueStr, valueCStr.get(), valueStrBufLength + 1, &valueStrLength);
                    workerName = std::string(valueCStr.get(), valueStrLength);
            }
            TAG_LOGE(AAFwkTag::APPKIT, "ZCK1219 workerName = %{public}s", workerName.c_str());
        } else if (workerEngine->IsTaskPoolThread()) {
            type = 2;
            TAG_LOGE(AAFwkTag::APPKIT, "ZCK1219 type = 2");
            workerName = GetFuncNameFromError(napiEnv, exception);
            TAG_LOGE(AAFwkTag::APPKIT, "ZCK1219 workerName = %{public}s", workerName.c_str());
        }
        TAG_LOGI(AAFwkTag::APPKIT, "ZCK1219 env != mainEnv");
        NapiErrorManager::GetInstance()->NotifyUncaughtException(napiEnv, exception, workerName, type);
        }
        return false;
}
}  // namespace AppExecFwk
}  // namespace OHOS
