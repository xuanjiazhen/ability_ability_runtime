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

#include "app_scheduler.h"

#include "hilog_tag_wrapper.h"
#include "ability_util.h"
#include "ability_manager_errors.h"
#include "appmgr/app_mgr_constants.h"

namespace OHOS {
namespace AAFwk {
AppScheduler::AppScheduler()
{
    TAG_LOGI(AAFwkTag::TEST, " Test AppScheduler::AppScheduler()");
}

AppScheduler::~AppScheduler()
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::~AppScheduler()");
}

bool AppScheduler::Init(const std::weak_ptr<AppStateCallback>& callback)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::Init()");
    if (!callback.lock()) {
        return false;
    }
    return true;
}

int AppScheduler::LoadAbility(const AbilityRuntime::LoadParam &loadParam, const AppExecFwk::AbilityInfo& abilityInfo,
    const AppExecFwk::ApplicationInfo& applicationInfo, const AAFwk::Want& want)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::LoadAbility()");
    if (applicationInfo.bundleName.find("com.ix.First.Test") != std::string::npos) {
        return INNER_ERR;
    }
    return ERR_OK;
}

int AppScheduler::TerminateAbility(const sptr<IRemoteObject>& token, bool isClearMissionFlag)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::TerminateAbility()");
    return ERR_OK;
}

void AppScheduler::MoveToForeground(const sptr<IRemoteObject>& token)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::MoveToForeground()");
}

void AppScheduler::MoveToBackground(const sptr<IRemoteObject>& token, bool isFromScreenOffBackground)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::MoveToBackground()");
}

void AppScheduler::KillProcessByAbilityToken(const sptr<IRemoteObject>& token)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::KillProcessByAbilityToken()");
}

void AppScheduler::KillProcessesByUserId(int32_t userId, bool isNeedSendAppSpawnMsg,
    sptr<AAFwk::IUserCallback> callback)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::KillProcessesByUserId()");
}

AppAbilityState AppScheduler::ConvertToAppAbilityState(const int32_t state)
{
    AppExecFwk::AbilityState abilityState = static_cast<AppExecFwk::AbilityState>(state);
    switch (abilityState) {
        case AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND: {
            return AppAbilityState::ABILITY_STATE_FOREGROUND;
        }
        case AppExecFwk::AbilityState::ABILITY_STATE_BACKGROUND: {
            return AppAbilityState::ABILITY_STATE_BACKGROUND;
        }
        default:
            return AppAbilityState::ABILITY_STATE_UNDEFINED;
    }
}

AppAbilityState AppScheduler::GetAbilityState() const
{
    return appAbilityState_;
}

void AppScheduler::OnAbilityRequestDone(const sptr<IRemoteObject>& token, const AppExecFwk::AbilityState state)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::OnAbilityRequestDone()");
}

int AppScheduler::KillApplication(const std::string& bundleName, bool clearPageStack, int32_t appIndex,
    const std::string& reason)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::KillApplication()");
    return ERR_OK;
}

void AppScheduler::AttachTimeOut(const sptr<IRemoteObject>& token)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::AttachTimeOut()");
}

void AppScheduler::PrepareTerminate(const sptr<IRemoteObject>& token, bool clearMissionFlag)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::PrepareTerminate()");
}

void AppScheduler::OnAppStateChanged(const AppExecFwk::AppProcessData& appData)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::OnAppStateChanged()");
}

void AppScheduler::NotifyConfigurationChange(const AppExecFwk::Configuration &config, int32_t userId)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::NotifyConfigurationChange()");
}

void AppScheduler::NotifyStartResidentProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::NotifyStartResidentProcess()");
}

void AppScheduler::NotifyStartKeepAliveProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos,
    pid_t diedPid)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::NotifyStartKeepAliveProcess()");
}

void AppScheduler::OnAppRemoteDied(const std::vector<sptr<IRemoteObject>> &abilityTokens)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::OnAppRemoteDied()");
}

void AppScheduler::OnStartProcessFailed(const std::vector<sptr<IRemoteObject>> &abilityTokens)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::OnStartProcessFailed()");
}

void AppScheduler::NotifyTerminateAbility(const sptr<IRemoteObject> &token)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::NotifyTerminateAbility()");
}

void AppScheduler::NotifyAppPreCache(int32_t pid, int32_t userId)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::NotifyAppPreCache()");
}

void AppScheduler::RecordAppExitSignalReason(int32_t pid, int32_t uid, int32_t signal, std::string &bundleName)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::RecordAppExitSignalReason()");
}

void AppScheduler::UpdateAbilityState(const sptr<IRemoteObject>& token, const AppExecFwk::AbilityState state)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::UpdateAbilityState()");
}

void AppScheduler::UpdateExtensionState(const sptr<IRemoteObject>& token, const AppExecFwk::ExtensionState state)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::UpdateExtensionState()");
}

void AppScheduler::StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo>& bundleInfos)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::StartupResidentProcess()");
}

int AppScheduler::GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo>& info)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::GetProcessRunningInfos()");
    return 0;
}

void AppScheduler::GetRunningProcessInfoByToken(const sptr<IRemoteObject>& token, AppExecFwk::RunningProcessInfo& info)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::GetRunningProcessInfoByToken()");
}

void AppScheduler::GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo& info) const
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::GetRunningProcessInfoByPid()");
}

bool AppScheduler::IsMemorySizeSufficient() const
{
    return true;
}

int32_t AppScheduler::VerifyKillProcessPermission(const std::string &bundleName) const
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::VerifyKillProcessPermission()");
    return ERR_OK;
}

void AppScheduler::StartSpecifiedAbility(const AAFwk::Want&, const AppExecFwk::AbilityInfo&,
    const AbilityRuntime::StartSpecifiedParam&)
{}

int AppScheduler::StartUserTest(
    const Want& want, const sptr<IRemoteObject>& observer, const AppExecFwk::BundleInfo& bundleInfo, int32_t userId)
{
    return 0;
}

int AppScheduler::GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo& application, bool& debug)
{
    if (pid < 0) {
        return -1;
    }

    return 0;
}

int32_t AppScheduler::NotifyAppMgrRecordExitReason(int32_t pid, int32_t reason, const std::string &exitMsg)
{
    return 0;
}

int32_t AppScheduler::NotifyAppMgrRecordExitReasonCompability(
    int32_t pid, int32_t killId, const std::string &killMsg, const std::string &innerMsg, int32_t reason)
{
    return 0;
}

int AppScheduler::GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>>& tokens)
{
    return 0;
}

int32_t AppScheduler::RegisterAppDebugListener(const sptr<AppExecFwk::IAppDebugListener> &listener)
{
    return 0;
}

int32_t AppScheduler::UnregisterAppDebugListener(const sptr<AppExecFwk::IAppDebugListener> &listener)
{
    return 0;
}

int32_t AppScheduler::AttachAppDebug(const std::string &bundleName, bool isDebugFromLocal)
{
    return 0;
}

int32_t AppScheduler::DetachAppDebug(const std::string &bundleName)
{
    return 0;
}

int32_t AppScheduler::RegisterAbilityDebugResponse(const sptr<AppExecFwk::IAbilityDebugResponse> &response)
{
    return 0;
}

// 链接补齐桩：本 mock 替换真实 app_scheduler.cpp 时，需要覆盖被测源码引用到的全部 AppScheduler 对外方法。
void AppScheduler::AttachPidToParent(const sptr<IRemoteObject> &token,
    const sptr<IRemoteObject> &callerToken)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::AttachPidToParent()");

}

void AppScheduler::AttachedToStatusBar(const sptr<IRemoteObject> &token)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::AttachedToStatusBar()");

}

void AppScheduler::BlockProcessCacheByPids(const std::vector<int32_t> &pids)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::BlockProcessCacheByPids()");

}

int32_t AppScheduler::CheckPreloadAppRecordExist(const std::string &bundleName, int32_t userId,
    int32_t appIndex, bool &isExist)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::CheckPreloadAppRecordExist()");
    return 0;
}

bool AppScheduler::CleanAbilityByUserRequest(const sptr<IRemoteObject> &token)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::CleanAbilityByUserRequest()");
    return false;
}

void AppScheduler::ClearProcessByToken(sptr<IRemoteObject> token) const
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::ClearProcessByToken()");

}

std::string AppScheduler::ConvertAppState(const AppState &state)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::ConvertAppState()");
    return "";
}

int AppScheduler::FinishUserTest(const std::string &msg, const int64_t &resultCode,
    const std::string &bundleName)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::FinishUserTest()");
    return 0;
}

int AppScheduler::ForceKillApplication(const std::string &bundleName, const int userId,
    const int appIndex)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::ForceKillApplication()");
    return 0;
}

int32_t AppScheduler::GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::GetBundleNameByPid()");
    return 0;
}

int AppScheduler::GetConfiguration(AppExecFwk::Configuration &config)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::GetConfiguration()");
    return 0;
}

int AppScheduler::GetProcessRunningInfosByAccessTokenId(uint32_t accessTokenId,
    std::vector<AppExecFwk::RunningProcessInfo> &info)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::GetProcessRunningInfosByAccessTokenId()");
    return 0;
}

int AppScheduler::GetProcessRunningInfosByUserId(std::vector<AppExecFwk::RunningProcessInfo> &info,
    int32_t userId)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::GetProcessRunningInfosByUserId()");
    return 0;
}

void AppScheduler::GetRunningProcessInfoByChildProcessPid(const pid_t childPid,
    OHOS::AppExecFwk::RunningProcessInfo &info) const
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::GetRunningProcessInfoByChildProcessPid()");

}

bool AppScheduler::IsAttachDebug(const std::string &bundleName)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::IsAttachDebug()");
    return false;
}

bool AppScheduler::IsCallerKilling(const std::string& callerKey) const
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::IsCallerKilling()");
    return false;
}

bool AppScheduler::IsCorrespondingProcessAttachDebug(const AppExecFwk::AbilityInfo &abilityInfo)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::IsCorrespondingProcessAttachDebug()");
    return false;
}

bool AppScheduler::IsKilledForUpgradeWeb(const std::string &bundleName)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::IsKilledForUpgradeWeb()");
    return false;
}

bool AppScheduler::IsMainProcessDebug(int32_t uid)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::IsMainProcessDebug()");
    return false;
}

bool AppScheduler::IsNoRequireBigMemory() const
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::IsNoRequireBigMemory()");
    return false;
}

bool AppScheduler::IsProcessAttached(sptr<IRemoteObject> token) const
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::IsProcessAttached()");
    return false;
}

bool AppScheduler::IsProcessContainsOnlyUIAbility(const pid_t pid)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::IsProcessContainsOnlyUIAbility()");
    return false;
}

int AppScheduler::KillApplicationByUid(const std::string &bundleName, int32_t uid,
    const std::string& reason)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::KillApplicationByUid()");
    return 0;
}

int AppScheduler::KillApplicationWithUserId(const std::string &bundleName, const int userId,
    const int appIndex)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::KillApplicationWithUserId()");
    return 0;
}

int AppScheduler::KillProcessesByAccessTokenId(const uint32_t accessTokenId)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::KillProcessesByAccessTokenId()");
    return 0;
}

int32_t AppScheduler::KillProcessesByPids(const std::vector<int32_t> &pids,
    const std::string &reason, bool subProcess, bool isKillPrecedeStart)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::KillProcessesByPids()");
    return 0;
}

int32_t AppScheduler::NotifyFault(const AppExecFwk::FaultData &faultData)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::NotifyFault()");
    return 0;
}

void AppScheduler::NotifyLoadAbilityFinished(pid_t callingPid, pid_t targetPid, uint64_t callbackId)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::NotifyLoadAbilityFinished()");

}

int32_t AppScheduler::NotifyPreloadAbilityStateChanged(sptr<IRemoteObject> token,
    bool isPreForeground)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::NotifyPreloadAbilityStateChanged()");
    return 0;
}

int AppScheduler::NotifyUninstallOrUpgradeApp(const std::string &bundleName, int32_t uid,
    bool isUpgrade)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::NotifyUninstallOrUpgradeApp()");
    return 0;
}

void AppScheduler::OnCacheExitInfo(uint32_t accessTokenId,
    const AppExecFwk::RunningProcessInfo &exitInfo, const std::string &bundleName,
    const std::vector<std::string> &abilityNames, const std::vector<std::string> &uiExtensionNames)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::OnCacheExitInfo()");

}

int32_t AppScheduler::PreloadApplicationByPhase(const std::string &bundleName, int32_t userId,
    int32_t appIndex, AppExecFwk::PreloadPhase preloadPhase)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::PreloadApplicationByPhase()");
    return 0;
}

void AppScheduler::PrepareTerminateApp(const pid_t pid, const std::string &moduleName)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::PrepareTerminateApp()");

}

void AppScheduler::SetAbilityForegroundingFlagToAppRecord(const pid_t pid) const
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::SetAbilityForegroundingFlagToAppRecord()");

}

void AppScheduler::SetEnableStartProcessFlagByUserId(int32_t userId, bool enableStartProcess)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::SetEnableStartProcessFlagByUserId()");

}

int32_t AppScheduler::SetGameSAPrelaunch(const sptr<IRemoteObject> &token, bool isGameSAPrelaunch)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::SetGameSAPrelaunch()");
    return 0;
}

void AppScheduler::SetProcessCacheStatus(int32_t pid, bool isSupport)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::SetProcessCacheStatus()");

}

void AppScheduler::SetSpecifiedProcessRequestId(int32_t recordId, int32_t requestId)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::SetSpecifiedProcessRequestId()");

}

void AppScheduler::StartSpecifiedProcess(const AAFwk::Want &want,
    const AppExecFwk::AbilityInfo &abilityInfo, int32_t requestId, const std::string &customProcess)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::StartSpecifiedProcess()");

}

int AppScheduler::UpdateApplicationInfoInstalled(const std::string &bundleName, const int32_t uid,
    const std::string &moduleName, bool isPlugin)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::UpdateApplicationInfoInstalled()");
    return 0;
}

int AppScheduler::UpdateConfiguration(const AppExecFwk::Configuration &config)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::UpdateConfiguration()");
    return 0;
}
}  // namespace AAFwk
}  // namespace OHOS
