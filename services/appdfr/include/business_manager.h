/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_BUSINESS_MANAGER_H
#define OHOS_ABILITY_RUNTIME_BUSINESS_MANAGER_H

#include <memory>
#include <string>

#include "fault_data.h"

namespace OHOS {
namespace AppExecFwk {

class AppMgrServiceInner;
class AppRunningRecord;

/**
 * @brief BusinessManager is a stateless static facade that owns the DFR business
 * logic backing AppMgrServiceInner::NotifyAppFault / NotifyAppFaultBySA.
 *
 * All methods are static: a stateless singleton's logic does not depend on any
 * instance state, so callers (AppMgrServiceInner forwarders and unit tests) invoke
 * them directly as BusinessManager::XXX(...). The Meyer singleton GetInstance() is
 * retained only for API compatibility; static calls do not require it.
 *
 * Methods that need AppMgrServiceInner state receive it via an explicit `inner`
 * handle and access private members through `friend class BusinessManager`.
 */
class BusinessManager {
public:
    static BusinessManager &GetInstance();

    static int32_t NotifyAppFault(const std::shared_ptr<AppMgrServiceInner> &inner, const FaultData &faultData);

    static int32_t TransformedNotifyAppFault(const std::shared_ptr<AppMgrServiceInner> &inner,
        const AppFaultDataBySA &faultData);

    static FaultData ConvertDataTypes(const std::shared_ptr<AppMgrServiceInner> &inner,
        const AppFaultDataBySA &faultData);

    static void TimeoutNotifyApp(const std::shared_ptr<AppMgrServiceInner> &inner, int32_t pid, int32_t uid,
        const std::string &bundleName, const std::string &processName, const FaultData &faultData,
        int32_t recordId);

    static int32_t SubmitDfxFaultTask(const std::shared_ptr<AppMgrServiceInner> &inner, const FaultData &faultData,
        const std::string &bundleName, const std::shared_ptr<AppRunningRecord> &appRecord, const int32_t pid);

    static void ParseInfoToAppfreeze(const FaultData &faultData, int32_t pid, int32_t uid,
        const std::string &bundleName, const std::string &processName, const bool isOccurException = false);

    static int GetExceptionTimerId(const std::shared_ptr<AppMgrServiceInner> &inner, const FaultData &faultData,
        const std::string &bundleName, const std::shared_ptr<AppRunningRecord> &appRecord,
        const int32_t pid, const int32_t callerUid);

    static bool UpdateForeground(const std::shared_ptr<AppRunningRecord> &appRecord, const std::string &eventName);

    static bool IsAsanEnabled(const std::shared_ptr<AppRunningRecord> &record);

    static bool CheckAppFault(const std::shared_ptr<AppMgrServiceInner> &inner,
        const std::shared_ptr<AppRunningRecord> &appRecord, const FaultData &faultData);

    static int32_t KillFaultApp(const std::shared_ptr<AppMgrServiceInner> &inner, int32_t pid,
        const std::string &bundleName, const FaultData &faultData, bool isNeedExit = false,
        int32_t recordId = -1);

    static void AppRecoveryNotifyApp(const std::shared_ptr<AppMgrServiceInner> &inner, int32_t pid,
        const std::string &bundleName, FaultDataType faultType, const std::string &markers,
        int32_t recordId);

    static void RecordAppRecoveryNotifyAppReason(const std::shared_ptr<AppMgrServiceInner> &inner, int32_t pid);

    static bool SetAppFreezeFilter(const std::shared_ptr<AppMgrServiceInner> &inner, int32_t pid);

    static void InsertKillThread(int32_t state, int32_t pid, int32_t uid, const std::string &bundleName);
    static void ReportAppFreezeSysEvents(int32_t pid, const std::string &bundleName);
    static void RemoveDeathProcess(const std::string &bundleName);
    static void SendProcessKillEvent(std::shared_ptr<AppRunningRecord> appRecord);

    static std::string FaultTypeToString(FaultDataType type);

#ifdef APP_MGR_KILL_REASON_TAG
    static void RecordAppfreezeKillReason(int32_t pid, const FaultData &faultData);
    static void RecordAppWithReasonByUserId(int32_t userId, int32_t killId);
#endif

private:
    BusinessManager() = default;
    ~BusinessManager() = default;
    BusinessManager(const BusinessManager &) = delete;
    BusinessManager &operator=(const BusinessManager &) = delete;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_BUSINESS_MANAGER_H
