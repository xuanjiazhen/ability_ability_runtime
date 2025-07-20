/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APPLICATION_DATA_MANAGER_H
#define OHOS_ABILITY_RUNTIME_APPLICATION_DATA_MANAGER_H

#include <string>

#include "ierror_observer.h"
#include "nocopyable.h"


namespace OHOS {
namespace AppExecFwk {

using STSExceptionObjectCallback = std::function<void(const ErrorObject &)>;
using STSUnhandledExceptionCallback = std::function<void(std::string)>;
using GetForgroundInfoFunc = std::function<void(bool &)>;
struct ExpectionInfo {
    std::string bundleName;
    int32_t versionCode;
    std::string hapPath;
    int32_t appRunningId;
    int32_t pid;
    std::string processName;
    GetForgroundInfoFunc GetForgroundInfo;
};

class ApplicationDataManager {
public:
    static ApplicationDataManager &GetInstance();
    void AddErrorObserver(const std::shared_ptr<IErrorObserver> &observer);
    bool NotifyUnhandledException(const std::string &errMsg);
    bool NotifyCJUnhandledException(const std::string &errMsg);
    bool NotifySTSUnhandledException(const std::string &errMsg);
    void RemoveErrorObserver();
    bool NotifyExceptionObject(const AppExecFwk::ErrorObject &errorObj);
    bool NotifyCJExceptionObject(const AppExecFwk::ErrorObject &errorObj);
    bool NotifySTSExceptionObject(const AppExecFwk::ErrorObject &errorObj);
    void SetStsExceptionInfo(const ExpectionInfo &info);
    void RegisterSTSExceptionObjectCallback(const STSExceptionObjectCallback &callback);
    void RegisterSTSUnhandledExceptionCallback(const STSUnhandledExceptionCallback &callback);
    void GetStsExceptionInfo(ExpectionInfo &info);
private:
    ApplicationDataManager();
    ~ApplicationDataManager();
    DISALLOW_COPY_AND_MOVE(ApplicationDataManager);
    std::shared_ptr<IErrorObserver> errorObserver_;
    ExpectionInfo stsExceptionInfo_;
    STSExceptionObjectCallback stsExceptionObjectCallback_;
    STSUnhandledExceptionCallback stsUnhandledExceptionCallback_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APPLICATION_DATA_MANAGER_H
