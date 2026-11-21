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

#ifndef OHOS_ABILITY_RUNTIME_APP_SCHEDULER_H
#define OHOS_ABILITY_RUNTIME_APP_SCHEDULER_H

#include <string>
#include <vector>

#include "ability_info.h"
#include "ability_manager_client.h"
#include "appmgr/app_state_callback_host.h"
#include "bundle_info.h"
#include "configuration.h"
#include "iremote_object.h"
#include "refbase.h"
#include "running_process_info.h"
#include "singleton.h"
#include "start_params_by_SCB.h"
#include "status_bar_delegate/status_bar_delegate_interface.h"
#include "ui_ability_last_caller_info.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
struct LoadParam;
struct StartSpecifiedParam;
}
namespace AAFwk {
/**
 * @enum AppAbilityState
 * AppAbilityState defines the life cycle state of app ability.
 */
enum class AppAbilityState {
    ABILITY_STATE_UNDEFINED = 0,
    ABILITY_STATE_FOREGROUND,
    ABILITY_STATE_BACKGROUND,
    ABILITY_STATE_END,
};

enum class AppState {
    BEGIN = 0,
    READY,
    FOREGROUND,
    FOCUS,
    BACKGROUND,
    TERMINATED,
    END,
    SUSPENDED,
    COLD_START = 99,
};

struct AppData {
    std::string appName;
    int32_t uid;
};

struct AppInfo {
    std::string instanceKey = "";
    std::string bundleName = "";
    std::string processName;
    std::vector<AppData> appData;
    pid_t pid = 0;
    int32_t userId = -1;
    int32_t appIndex = 0;
    AppState state;
};

// Verbatim from the real app_scheduler.h: AbilityManagerService inherits this
// abstract base, and ability_manager_service.h reaches it through the same
// include chain that the shadowed real header used to provide.
class AppStateCallback {
public:
    AppStateCallback() {}
    virtual ~AppStateCallback() {}

    virtual void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state) = 0;
    virtual void OnAppStateChanged(const AppInfo &info) = 0;
    virtual void NotifyConfigurationChange(const AppExecFwk::Configuration &config, int32_t userId) {}
    virtual void NotifyStartResidentProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos) {}
    virtual void NotifyStartKeepAliveProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos,
        pid_t diedPid = AppExecFwk::INVALID_DIED_PID) {}
    virtual void NotifyAppPreCache(int32_t pid, int32_t userId) {}
    virtual void OnAppRemoteDied(const std::vector<sptr<IRemoteObject>> &abilityTokens) {}
    virtual void OnStartProcessFailed(const std::vector<sptr<IRemoteObject>> &abilityTokens) {}
    virtual void NotifyTerminateAbility(const sptr<IRemoteObject> &token) {}
    virtual void OnCacheExitInfo(uint32_t accessTokenId, const AppExecFwk::RunningProcessInfo &exitInfo,
        const std::string &bundleName, const std::vector<std::string> &abilityNames,
        const std::vector<std::string> &uiExtensionNames) {}
    virtual void RecordAppExitSignalReason(int32_t pid, int32_t uid, int32_t signal, std::string &bundleName) {}
};

/**
 * @class AppScheduler
 * Link-time mock for the UIAbility lifecycle-manager UT: shadows the real
 * app_scheduler.h for every translation unit compiled into this test binary
 * (the real class keeps living inside libabilityms for the record code there).
 *
 * Default behaviour mirrors the real class with the appmgr service unreachable:
 * every appmgr call fails, so tests written against the dropped-report semantics
 * keep their meaning. The stub state below makes the registration channel
 * controllable: with attributeSucceeds = true, AttributeProcessForAbility hands
 * out incrementing server handles, which lets the dedup regression tests assert
 * whether a registration was attempted at all (a removed dedup would be seen
 * here).
 */
class AppScheduler {
    DECLARE_DELAYED_SINGLETON(AppScheduler);
public:
    struct StubState {
        // false: registration fails like an unreachable service (default);
        // true: registration succeeds and hands out incrementing handles.
        bool attributeSucceeds = false;
        // number of registration attempts seen by the stub.
        int32_t attributeCalls = 0;
        // first handle handed out when attributeSucceeds is true.
        int64_t nextAttributionId = 4701;
    };
    static StubState &Stub();
    static void ResetStub();

    int32_t AttributeProcessForAbility(const std::string &taskId, int32_t callerUid, int32_t callerPid,
        int32_t targetPid, int64_t &attributionId);

    void AttachTimeOut(const sptr<IRemoteObject> &token);
    void BlockProcessCacheByPids(const std::vector<int32_t> &pids);
    bool CleanAbilityByUserRequest(const sptr<IRemoteObject> &token);
    AppAbilityState ConvertToAppAbilityState(const int32_t state);
    int GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens);
    void GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info) const;
    void GetRunningProcessInfoByToken(const sptr<IRemoteObject> &token,
        OHOS::AppExecFwk::RunningProcessInfo &info);
    bool IsCallerKilling(const std::string &callerKey) const;
    int32_t KillProcessesByPids(const std::vector<int32_t> &pids,
        const std::string &reason = "KillProcessesByPids", bool subProcess = false,
        bool isKillPrecedeStart = false);
    void MoveToBackground(const sptr<IRemoteObject> &token, bool isFromScreenOffBackground = false);
    void MoveToForeground(const sptr<IRemoteObject> &token,
        const AppExecFwk::UiAbilityLastCallerInfo &callerInfo = {});
    void NotifyLoadAbilityFinished(pid_t callingPid, pid_t targetPid, uint64_t callbackId);
    int32_t NotifyPreloadAbilityStateChanged(sptr<IRemoteObject> token, bool isPreForeground);
    void PrepareTerminate(const sptr<IRemoteObject> &token, bool clearMissionFlag = false);
    void PrepareTerminateApp(const pid_t pid, const std::string &moduleName);
    int32_t SetGameSAPrelaunch(const sptr<IRemoteObject> &token, bool isGameSAPrelaunch);
    void SetSpecifiedProcessRequestId(int32_t recordId, int32_t requestId);
    void StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        const AbilityRuntime::StartSpecifiedParam &param);
    void StartSpecifiedProcess(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t requestId = 0, const std::string &customProcess = "");
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_SCHEDULER_H
