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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_APP_SCHEDULER_H
#define MOCK_OHOS_ABILITY_RUNTIME_APP_SCHEDULER_H

#include <memory>
#include <unordered_set>

#include "ability_info.h"
#include "app_debug_listener_interface.h"
#include "application_info.h"
#include "app_mgr_client.h"
#include "bundle_info.h"
#include "iremote_object.h"
#include "refbase.h"
#include "running_process_info.h"
#include "singleton.h"
#include "want.h"
#include "last_exit_detail_info.h"
#include "mock_common.h"

namespace OHOS {
namespace AbilityRuntime {
struct LoadParam;
}
namespace AAFwk {
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
    std::vector<AppData> appData;
    std::string processName;
    AppState state;
    pid_t pid = 0;
    int32_t appIndex = 0;
    std::string instanceKey = "";
    std::string bundleName = "";
};

class AppStateCallback {
public:
    AppStateCallback()
    {}
    virtual ~AppStateCallback()
    {}

    virtual void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state) = 0;

    virtual void OnAppStateChanged(const AppInfo &info) = 0;

    virtual void NotifyConfigurationChange(const AppExecFwk::Configuration &config, int32_t userId) {}

    virtual void NotifyStartResidentProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos) {}

    virtual void NotifyStartKeepAliveProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos) {}

    virtual void NotifyAppPreCache(int32_t pid, int32_t userId) {}

    virtual void OnAppRemoteDied(const std::vector<sptr<IRemoteObject>> &abilityTokens) {}
    
    virtual void OnStartProcessFailed(sptr<IRemoteObject> token) {}

    virtual void OnCacheExitInfo(uint32_t accessTokenId, const AAFwk::LastExitDetailInfo &exitInfo,
        const std::string &bundleName, const std::vector<std::string> &abilityNames,
        const std::vector<std::string> &uiExtensionNames) {}
};

class AppScheduler : virtual RefBase {
    DECLARE_DELAYED_SINGLETON(AppScheduler)
public:
    bool Init(const std::weak_ptr<AppStateCallback> &callback)
    {
        return false;
    }

    int TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag)
    {
        return 0;
    }

    void KillProcessesByPids(std::vector<int32_t> &pids, const std::string &reason = "KillProcessesByPids") {}

    AppAbilityState GetAbilityState() const
    {
        return AppAbilityState::ABILITY_STATE_UNDEFINED;
    }

    int KillApplication(const std::string &bundleName, bool clearPageStack = false, int32_t appIndex = 0)
    {
        return 0;
    }

    int KillProcessesByAccessTokenId(const uint32_t accessTokenId)
    {
        return 0;
    }

    int KillApplicationByUid(const std::string &bundleName, int32_t uid,
        const std::string& reason = "KillApplicationByUid")
    {
        return 0;
    }

    void GetRunningProcessInfoByToken(const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info) {}

    void GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info) const {}

    void GetRunningProcessInfoByChildProcessPid(const pid_t childPid,
        OHOS::AppExecFwk::RunningProcessInfo &info) const {}

    int GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info)
    {
        return 0;
    }

    int StartUserTest(const Want &want, const sptr<IRemoteObject> &observer, const AppExecFwk::BundleInfo &bundleInfo,
        int32_t userId)
    {
        return 0;
    }

    int FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName)
    {
        return 0;
    }

    int GetProcessRunningInfosByUserId(std::vector<AppExecFwk::RunningProcessInfo> &info, int32_t userId)
    {
        return 0;
    }

    std::string ConvertAppState(const AppState &state)
    {
        return "";
    }

    int UpdateConfiguration(const AppExecFwk::Configuration &config)
    {
        return 0;
    }

    int GetConfiguration(AppExecFwk::Configuration &config)
    {
        return 0;
    }

    int GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens)
    {
        return 0;
    }

    int GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application, bool &debug)
    {
        return 0;
    }

    int32_t GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid)
    {
        return 0;
    }

    int32_t RegisterAppDebugListener(const sptr<AppExecFwk::IAppDebugListener> &listener)
    {
        return 0;
    }

    int32_t UnregisterAppDebugListener(const sptr<AppExecFwk::IAppDebugListener> &listener)
    {
        return 0;
    }

    int32_t AttachAppDebug(const std::string &bundleName, bool isDebugFromLocal)
    {
        return 0;
    }

    int32_t DetachAppDebug(const std::string &bundleName)
    {
        return 0;
    }

    virtual bool IsMemorySizeSufficent() const
    {
        return false;
    }

    bool IsProcessContainsOnlyUIAbility(const pid_t pid)
    {
        return false;
    }

};
}  // namespace AAFwk
}  // namespace OHOS
#endif
