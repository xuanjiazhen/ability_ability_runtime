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

#include "app_scheduler.h"
#include "mock_my_status.h"

namespace OHOS {
namespace AAFwk {

void AppScheduler::GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info) const
{
    info = MyStatus::GetInstance().processInfo_;
}

// 链接补齐桩：本 mock 替换真实 app_scheduler.cpp 时，需要覆盖被测源码引用到的全部 AppScheduler 对外方法。
AppScheduler::AppScheduler()
{

}

int32_t AppScheduler::AttachAppDebug(const std::string &bundleName, bool isDebugFromLocal)
{
    return 0;
}

void AppScheduler::AttachPidToParent(const sptr<IRemoteObject> &token,
    const sptr<IRemoteObject> &callerToken)
{

}

void AppScheduler::AttachTimeOut(const sptr<IRemoteObject> &token)
{

}

void AppScheduler::AttachedToStatusBar(const sptr<IRemoteObject> &token)
{

}

void AppScheduler::BlockProcessCacheByPids(const std::vector<int32_t> &pids)
{

}

int32_t AppScheduler::CheckPreloadAppRecordExist(const std::string &bundleName, int32_t userId,
    int32_t appIndex, bool &isExist)
{
    return 0;
}

bool AppScheduler::CleanAbilityByUserRequest(const sptr<IRemoteObject> &token)
{
    return false;
}

void AppScheduler::ClearProcessByToken(sptr<IRemoteObject> token) const
{

}

std::string AppScheduler::ConvertAppState(const AppState &state)
{
    return "";
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

int32_t AppScheduler::DetachAppDebug(const std::string &bundleName)
{
    return 0;
}

int AppScheduler::FinishUserTest(const std::string &msg, const int64_t &resultCode,
    const std::string &bundleName)
{
    return 0;
}

int AppScheduler::ForceKillApplication(const std::string &bundleName, const int userId,
    const int appIndex)
{
    return 0;
}

int AppScheduler::GetAbilityRecordsByProcessID(const int pid,
    std::vector<sptr<IRemoteObject>> &tokens)
{
    return 0;
}

AppAbilityState AppScheduler::GetAbilityState() const
{
    return appAbilityState_;
}

int AppScheduler::GetApplicationInfoByProcessID(const int pid,
    AppExecFwk::ApplicationInfo &application, bool &debug)
{
    return 0;
}

int32_t AppScheduler::GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid)
{
    return 0;
}

int AppScheduler::GetConfiguration(AppExecFwk::Configuration &config)
{
    return 0;
}

int AppScheduler::GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info)
{
    return 0;
}

int AppScheduler::GetProcessRunningInfosByAccessTokenId(uint32_t accessTokenId,
    std::vector<AppExecFwk::RunningProcessInfo> &info)
{
    return 0;
}

int AppScheduler::GetProcessRunningInfosByUserId(std::vector<AppExecFwk::RunningProcessInfo> &info,
    int32_t userId)
{
    return 0;
}

void AppScheduler::GetRunningProcessInfoByChildProcessPid(const pid_t childPid,
    OHOS::AppExecFwk::RunningProcessInfo &info) const
{

}

void AppScheduler::GetRunningProcessInfoByToken(const sptr<IRemoteObject> &token,
    AppExecFwk::RunningProcessInfo &info)
{

}

bool AppScheduler::Init(const std::weak_ptr<AppStateCallback> &callback)
{
    return false;
}

bool AppScheduler::IsAttachDebug(const std::string &bundleName)
{
    return false;
}

bool AppScheduler::IsCallerKilling(const std::string& callerKey) const
{
    return false;
}

bool AppScheduler::IsCorrespondingProcessAttachDebug(const AppExecFwk::AbilityInfo &abilityInfo)
{
    return false;
}

bool AppScheduler::IsKilledForUpgradeWeb(const std::string &bundleName)
{
    return false;
}

bool AppScheduler::IsMainProcessDebug(int32_t uid)
{
    return false;
}

bool AppScheduler::IsMemorySizeSufficient() const
{
    return false;
}

bool AppScheduler::IsNoRequireBigMemory() const
{
    return false;
}

bool AppScheduler::IsProcessAttached(sptr<IRemoteObject> token) const
{
    return false;
}

bool AppScheduler::IsProcessContainsOnlyUIAbility(const pid_t pid)
{
    return false;
}

int AppScheduler::KillApplication(const std::string &bundleName, bool clearPageStack,
    int32_t appIndex, const std::string &reason)
{
    return 0;
}

int AppScheduler::KillApplicationByUid(const std::string &bundleName, int32_t uid,
    const std::string& reason)
{
    return 0;
}

int AppScheduler::KillApplicationWithUserId(const std::string &bundleName, const int userId,
    const int appIndex)
{
    return 0;
}

void AppScheduler::KillProcessByAbilityToken(const sptr<IRemoteObject> &token)
{

}

int AppScheduler::KillProcessesByAccessTokenId(const uint32_t accessTokenId)
{
    return 0;
}

int32_t AppScheduler::KillProcessesByPids(const std::vector<int32_t> &pids,
    const std::string &reason, bool subProcess, bool isKillPrecedeStart)
{
    return 0;
}

void AppScheduler::KillProcessesByUserId(int32_t userId, bool isNeedSendAppSpawnMsg,
    sptr<AAFwk::IUserCallback> callback)
{

}

int AppScheduler::LoadAbility(const AbilityRuntime::LoadParam &loadParam,
    const AppExecFwk::AbilityInfo &abilityInfo, const AppExecFwk::ApplicationInfo &applicationInfo,
    const Want &want)
{
    return 0;
}

void AppScheduler::MoveToBackground(const sptr<IRemoteObject> &token, bool isFromScreenOffBackground)
{

}

void AppScheduler::MoveToForeground(const sptr<IRemoteObject> &token)
{

}

int32_t AppScheduler::NotifyAppMgrRecordExitReason(int32_t pid, int32_t reason,
    const std::string &exitMsg)
{
    return 0;
}

int32_t AppScheduler::NotifyAppMgrRecordExitReasonCompability(int32_t pid, int32_t killId,
    const std::string &killMsg, const std::string &innerMsg, int32_t reason)
{
    return 0;
}

void AppScheduler::NotifyAppPreCache(int32_t pid, int32_t userId)
{

}

void AppScheduler::NotifyConfigurationChange(const AppExecFwk::Configuration &config, int32_t userId)
{

}

int32_t AppScheduler::NotifyFault(const AppExecFwk::FaultData &faultData)
{
    return 0;
}

void AppScheduler::NotifyLoadAbilityFinished(pid_t callingPid, pid_t targetPid, uint64_t callbackId)
{

}

int32_t AppScheduler::NotifyPreloadAbilityStateChanged(sptr<IRemoteObject> token,
    bool isPreForeground)
{
    return 0;
}

void AppScheduler::NotifyStartKeepAliveProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos,
    pid_t diedPid)
{

}

void AppScheduler::NotifyStartResidentProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{

}

void AppScheduler::NotifyTerminateAbility(const sptr<IRemoteObject> &token)
{

}

int AppScheduler::NotifyUninstallOrUpgradeApp(const std::string &bundleName, int32_t uid,
    bool isUpgrade)
{
    return 0;
}

void AppScheduler::OnAbilityRequestDone(const sptr<IRemoteObject> &token,
    const AppExecFwk::AbilityState state)
{

}

void AppScheduler::OnAppRemoteDied(const std::vector<sptr<IRemoteObject>> &abilityTokens)
{

}

void AppScheduler::OnAppStateChanged(const AppExecFwk::AppProcessData &appData)
{

}

void AppScheduler::OnCacheExitInfo(uint32_t accessTokenId,
    const AppExecFwk::RunningProcessInfo &exitInfo, const std::string &bundleName,
    const std::vector<std::string> &abilityNames, const std::vector<std::string> &uiExtensionNames)
{

}

void AppScheduler::OnStartProcessFailed(const std::vector<sptr<IRemoteObject>> &abilityTokens)
{

}

int32_t AppScheduler::PreloadApplicationByPhase(const std::string &bundleName, int32_t userId,
    int32_t appIndex, AppExecFwk::PreloadPhase preloadPhase)
{
    return 0;
}

void AppScheduler::PrepareTerminate(const sptr<IRemoteObject> &token, bool clearMissionFlag)
{

}

void AppScheduler::PrepareTerminateApp(const pid_t pid, const std::string &moduleName)
{

}

void AppScheduler::RecordAppExitSignalReason(int32_t pid, int32_t uid, int32_t signal,
    std::string &bundleName)
{

}

int32_t AppScheduler::RegisterAbilityDebugResponse(
    const sptr<AppExecFwk::IAbilityDebugResponse> &response)
{
    return 0;
}

int32_t AppScheduler::RegisterAppDebugListener(const sptr<AppExecFwk::IAppDebugListener> &listener)
{
    return 0;
}

void AppScheduler::SetAbilityForegroundingFlagToAppRecord(const pid_t pid) const
{

}

void AppScheduler::SetEnableStartProcessFlagByUserId(int32_t userId, bool enableStartProcess)
{

}

int32_t AppScheduler::SetGameSAPrelaunch(const sptr<IRemoteObject> &token, bool isGameSAPrelaunch)
{
    return 0;
}

void AppScheduler::SetProcessCacheStatus(int32_t pid, bool isSupport)
{

}

void AppScheduler::SetSpecifiedProcessRequestId(int32_t recordId, int32_t requestId)
{

}

void AppScheduler::StartSpecifiedAbility(const AAFwk::Want &want,
    const AppExecFwk::AbilityInfo &abilityInfo, const AbilityRuntime::StartSpecifiedParam &param)
{

}

void AppScheduler::StartSpecifiedProcess(const AAFwk::Want &want,
    const AppExecFwk::AbilityInfo &abilityInfo, int32_t requestId, const std::string &customProcess)
{

}

int AppScheduler::StartUserTest(const Want &want, const sptr<IRemoteObject> &observer,
    const AppExecFwk::BundleInfo &bundleInfo, int32_t userId)
{
    return 0;
}

void AppScheduler::StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{

}

int AppScheduler::TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag)
{
    return 0;
}

int32_t AppScheduler::UnregisterAppDebugListener(const sptr<AppExecFwk::IAppDebugListener> &listener)
{
    return 0;
}

void AppScheduler::UpdateAbilityState(const sptr<IRemoteObject> &token,
    const AppExecFwk::AbilityState state)
{

}

int AppScheduler::UpdateApplicationInfoInstalled(const std::string &bundleName, const int32_t uid,
    const std::string &moduleName, bool isPlugin)
{
    return 0;
}

int AppScheduler::UpdateConfiguration(const AppExecFwk::Configuration &config)
{
    return 0;
}

void AppScheduler::UpdateExtensionState(const sptr<IRemoteObject> &token,
    const AppExecFwk::ExtensionState state)
{

}

int32_t AppScheduler::VerifyKillProcessPermission(const std::string &bundleName) const
{
    return 0;
}

AppScheduler::~AppScheduler()
{}
}  // namespace AAFwk
}  // namespace OHOS
