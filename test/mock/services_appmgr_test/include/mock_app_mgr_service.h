/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_MGR_SERVICE_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_MGR_SERVICE_H

#include "gmock/gmock.h"
#include <cstdint>
#include "semaphore_ex.h"
#include "app_mgr_stub.h"
#include "app_malloc_info.h"
#include "memory_level_info.h"

namespace OHOS {
namespace AppExecFwk {
class MockAppMgrService : public AppMgrStub {
public:
    MOCK_METHOD6(LoadAbility,
        void(const sptr<IRemoteObject>& token, const sptr<IRemoteObject>& preToken,
            const std::shared_ptr<AbilityInfo>& abilityInfo, const std::shared_ptr<ApplicationInfo>& appInfo,
            const std::shared_ptr<AAFwk::Want>& want, int32_t abilityRecordId));
    MOCK_METHOD2(TerminateAbility, ErrCode(const sptr<IRemoteObject>& token, bool clearMissionFlag));
    MOCK_METHOD2(UpdateAbilityState, void(const sptr<IRemoteObject>& token, const AbilityState state));
    MOCK_METHOD1(AttachApplication, ErrCode(const sptr<IRemoteObject>& app));
    MOCK_METHOD1(NotifyMemoryLevel, int(int32_t level));
    MOCK_METHOD1(NotifyProcMemoryLevel, int32_t(const MemoryLevelInfo& procLevelMap));
    MOCK_METHOD2(DumpHeapMemory, int(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo));
    MOCK_METHOD1(ApplicationForegrounded, ErrCode(const int32_t recordId));
    MOCK_METHOD1(ApplicationBackgrounded, ErrCode(const int32_t recordId));
    MOCK_METHOD1(ApplicationTerminated, ErrCode(const int32_t recordId));
    MOCK_METHOD1(AbilityCleaned, ErrCode(const sptr<IRemoteObject>& token));
    MOCK_METHOD4(UpdateApplicationInfoInstalled, int(const std::string&, const int uid, const std::string&, bool));
    MOCK_METHOD3(ForceKillApplication, int32_t(const std::string& appName, const int userId, const int appIndex));
    MOCK_METHOD1(KillProcessesByAccessTokenId, int32_t(const uint32_t accessTokenId));
    MOCK_METHOD3(KillApplication, int32_t(const std::string& appName, const bool clearPageStack, int32_t appIndex));
    MOCK_METHOD3(KillApplicationByUid, int(const std::string&, const int uid, const std::string&));
    MOCK_METHOD1(IsBackgroundRunningRestricted, int(const std::string& bundleName));
    MOCK_METHOD1(GetAllRunningProcesses, int(std::vector<RunningProcessInfo>& info));
    MOCK_METHOD2(GetRunningProcessesByBundleType, int(const BundleType bundleType,
        std::vector<RunningProcessInfo>& info));
    MOCK_METHOD2(GetProcessRunningInfosByUserId, int(std::vector<RunningProcessInfo>& info, int32_t userId));
    MOCK_METHOD1(GetAllRenderProcesses, int(std::vector<RenderProcessInfo>& info));
    MOCK_METHOD1(GetAllChildrenProcesses, int(std::vector<ChildProcessInfo>&));
    MOCK_METHOD1(GetAmsMgr, ErrCode(sptr<IAmsMgr>& funcResult));
    MOCK_METHOD1(GetAppFreezingTime, void(int& time));
    MOCK_METHOD1(SetAppFreezingTime, void(int time));
    MOCK_METHOD3(ClearUpApplicationData, int32_t(const std::string& bundleName, int32_t appCloneIndex, int32_t userId));
    MOCK_METHOD1(ClearUpApplicationDataBySelf, int32_t(int32_t userId));
    MOCK_METHOD1(StartupResidentProcess, ErrCode(const std::vector<AppExecFwk::BundleInfo>& bundleInfos));
    MOCK_METHOD1(AddAbilityStageDone, ErrCode(const int32_t recordId));
    MOCK_METHOD0(PreStartNWebSpawnProcess, int());
    MOCK_METHOD6(StartRenderProcess, int(const std::string&, int32_t, int32_t, int32_t, pid_t&, bool));
    MOCK_METHOD1(AttachRenderProcess, ErrCode(const sptr<IRemoteObject>& renderScheduler));
    MOCK_METHOD1(SaveBrowserChannel, ErrCode(const sptr<IRemoteObject>& browser));
    MOCK_METHOD2(GetRenderProcessTerminationStatus, int(pid_t renderPid, int& status));
    MOCK_METHOD2(RegisterApplicationStateObserver, int32_t(const sptr<IApplicationStateObserver>& observer,
        const std::vector<std::string>& bundleNameList));
    MOCK_METHOD1(UnregisterApplicationStateObserver, int32_t(const sptr<IApplicationStateObserver>& observer));
    MOCK_METHOD3(ScheduleAcceptWantDone,
        ErrCode(const int32_t recordId, const AAFwk::Want& want, const std::string& flag));
    MOCK_METHOD3(ScheduleNewProcessRequestDone,
        ErrCode(const int32_t recordId, const AAFwk::Want& want, const std::string& flag));
    MOCK_METHOD2(GetAbilityRecordsByProcessID, int(const int pid, std::vector<sptr<IRemoteObject>>& tokens));
    MOCK_METHOD1(GetConfiguration, int32_t(Configuration& config));
    MOCK_METHOD2(UpdateConfiguration, int32_t(const Configuration& config, const int32_t userId));
    MOCK_METHOD3(UpdateConfigurationForBackgroundApp, int32_t(const std::vector<BackgroundAppInfo> &appInfos,
        const AppExecFwk::ConfigurationPolicy &policy, const int32_t userId));
    MOCK_METHOD3(UpdateConfigurationByBundleName, int32_t(const Configuration& config, const std::string &name,
        int32_t appIndex));
    MOCK_METHOD1(RegisterConfigurationObserver, int32_t(const sptr<IConfigurationObserver>& observer));
    MOCK_METHOD1(UnregisterConfigurationObserver, int32_t(const sptr<IConfigurationObserver>& observer));
    MOCK_METHOD2(GetAppRunningStateByBundleName, ErrCode(const std::string& bundleName, bool& funcResult));
    MOCK_METHOD2(NotifyLoadRepairPatch, int32_t(const std::string& bundleName,
        const sptr<IQuickFixCallback>& callback));
    MOCK_METHOD2(NotifyHotReloadPage, int32_t(const std::string& bundleName, const sptr<IQuickFixCallback>& callback));
    MOCK_METHOD2(NotifyUnLoadRepairPatch, int32_t(const std::string& bundleName,
        const sptr<IQuickFixCallback>& callback));
    MOCK_METHOD3(IsSharedBundleRunning, ErrCode(const std::string &bundleName, uint32_t versionCode, bool& funcResult));
    MOCK_METHOD3(GetBundleNameByPid, int32_t(const int pid, std::string &bundleName, int32_t &uid));

    MOCK_METHOD1(NotifyAppFault, int32_t(const FaultData &faultData));
    MOCK_METHOD1(NotifyAppFaultBySA, int32_t(const AppFaultDataBySA &faultData));
    MOCK_METHOD2(GetProcessMemoryByPid, int32_t(const int32_t pid, int32_t & memorySize));
    MOCK_METHOD3(GetRunningProcessInformation, int32_t(const std::string & bundleName, int32_t userId,
        std::vector<RunningProcessInfo> &info));
    MOCK_METHOD2(GetRunningMultiAppInfoByBundleName, int32_t(const std::string &bundleName,
        RunningMultiAppInfo &info));
    MOCK_METHOD1(GetAllRunningInstanceKeysBySelf, int32_t(std::vector<std::string> &instanceKeys));
    MOCK_METHOD3(GetAllRunningInstanceKeysByBundleName, int32_t(const std::string &bundleName,
        std::vector<std::string> &instanceKeys, int32_t userId));
    MOCK_METHOD2(IsApplicationRunning, int32_t(const std::string &bundleName, bool &isRunning));
    MOCK_METHOD3(IsAppRunning, int32_t(const std::string &bundleName,
        int32_t appCloneIndex, bool &isRunning));
    MOCK_METHOD3(IsAppRunningByBundleNameAndUserId, int32_t(const std::string &bundleName,
        int32_t userId, bool &isRunning));
#ifdef SUPPORT_CHILD_PROCESS
    MOCK_METHOD2(StartChildProcess, int32_t(pid_t &childPid, const ChildProcessRequest &request));
    MOCK_METHOD1(GetChildProcessInfoForSelf, int32_t(ChildProcessInfo &info));
    MOCK_METHOD1(AttachChildProcess, ErrCode(const sptr<IRemoteObject> &childScheduler));
    MOCK_METHOD0(ExitChildProcessSafely, ErrCode());
#endif // SUPPORT_CHILD_PROCESS
    MOCK_METHOD1(RegisterNativeChildExitNotify, int32_t(const sptr<INativeChildNotify>& notify));
    MOCK_METHOD1(UnregisterNativeChildExitNotify, int32_t(const sptr<INativeChildNotify>& notify));
    MOCK_METHOD1(RegisterRenderStateObserver, int32_t(const sptr<IRenderStateObserver> &observer));
    MOCK_METHOD1(UnregisterRenderStateObserver, int32_t(const sptr<IRenderStateObserver> &observer));
    MOCK_METHOD2(UpdateRenderState, int32_t(pid_t renderPid, int32_t state));

    MOCK_METHOD1(IsFinalAppProcess, ErrCode(bool& funcResult));
    MOCK_METHOD1(SetSupportedProcessCacheSelf, int32_t(bool isSupport));
    MOCK_METHOD2(IsProcessCacheSupported, int32_t(int32_t pid, bool &isSupported));
    MOCK_METHOD2(SetProcessCacheEnable, int32_t(int32_t pid, bool enable));
    MOCK_METHOD2(SetSupportedProcessCache, int32_t(int32_t pid, bool isSupport));
#ifdef SUPPORT_CHILD_PROCESS
    MOCK_METHOD4(StartNativeChildProcess, int32_t(const std::string &libName, int32_t childProcessCount,
        const sptr<IRemoteObject> &callback, const std::string &customProcessName));
#endif // SUPPORT_CHILD_PROCESS
    MOCK_METHOD2(GetSupportedProcessCachePids, int32_t(const std::string &bundleName,
        std::vector<int32_t> &pidList));

    MOCK_METHOD1(RegisterKiaInterceptor, int32_t(const sptr<IKiaInterceptor> &interceptor));
    MOCK_METHOD2(CheckIsKiaProcess, int32_t(pid_t pid, bool &isKia));
    virtual ErrCode PreloadApplication(
        const std::string& bundleName,
        int32_t userId,
        PreloadMode preloadMode,
        int32_t appIndex)
    {
        return 0;
    }

    virtual ErrCode HasDumpPermission(
        bool& funcResult)
    {
        return ERR_OK;
    }
    virtual ErrCode IsTerminatingByPid(
        int32_t pid,
        bool& isTerminating)
    {
        return ERR_OK;
    }
    virtual ErrCode GetForegroundApplications(
        std::vector<AppStateData>& list)
    {
        return ERR_OK;
    }
    virtual ErrCode SetContinuousTaskProcess(
        int32_t pid,
        bool isContinuousTask)
    {
        return ERR_OK;
    }
    virtual ErrCode GetRunningProcessInfoByPid(
        int32_t pid,
        RunningProcessInfo& info)
    {
        return ERR_OK;
    }
    virtual ErrCode GetRunningProcessInfoByChildProcessPid(
        int32_t childPid,
        RunningProcessInfo& info)
    {
        return ERR_OK;
    }
    virtual ErrCode NotifyPageShow(const sptr<IRemoteObject>& token,
                             const PageStateData& pageStateData)
    {
        return ERR_OK;
    }

    virtual ErrCode NotifyPageHide(const sptr<IRemoteObject>& token,
                             const PageStateData& pageStateData)
    {
        return ERR_OK;
    }

    virtual ErrCode SignRestartAppFlag(int32_t uid, const std::string& instanceKey)
    {
        return ERR_OK;
    }

    virtual ErrCode GetAppRunningUniqueIdByPid(int32_t pid, std::string& appRunningUniqueId)
    {
        return ERR_OK;
    }

    virtual ErrCode GetAllUIExtensionRootHostPid(int32_t pid, std::vector<int32_t>& hostPids)
    {
        return ERR_OK;
    }

    virtual ErrCode GetAllUIExtensionProviderPid(int32_t hostPid, std::vector<int32_t>& providerPids)
    {
        return ERR_OK;
    }

    virtual ErrCode NotifyMemorySizeStateChanged(int32_t memorySizeState)
    {
        return ERR_OK;
    }

    virtual ErrCode CheckCallingIsUserTestMode(int32_t pid, bool& isUserTest)
    {
        return ERR_OK;
    }

    virtual ErrCode SetAppAssertionPauseState(bool flag)
    {
        return ERR_OK;
    }

    virtual ErrCode NotifyProcessDependedOnWeb()
    {
        return ERR_OK;
    }

    virtual ErrCode KillProcessDependedOnWeb()
    {
        return ERR_OK;
    }

    virtual ErrCode RestartResidentProcessDependedOnWeb()
    {
        return ERR_OK;
    }

    virtual ErrCode KillAppSelfWithInstanceKey(const std::string& instanceKey,
                                            bool clearPageStack,
                                            const std::string& reason)
    {
        return ERR_OK;
    }

    virtual ErrCode UpdateInstanceKeyBySpecifiedId(int32_t specifiedId,
                                                const std::string& instanceKey)
    {
        return ERR_OK;
    }

    virtual ErrCode IsSpecifiedModuleLoaded(const Want& want,
                                        const AbilityInfo& abilityInfo,
                                        bool& loadedResult)
    {
        return ERR_OK;
    }

    virtual ErrCode UpdateProcessMemoryState(const std::vector<ProcessMemoryState>& procMemState)
    {
        return ERR_OK;
    }

    virtual ErrCode GetKilledProcessInfo(int32_t pid,
                                    int32_t uid,
                                    KilledProcessInfo& info)
    {
        return ERR_OK;
    }

    virtual ErrCode LaunchAbility(const sptr<IRemoteObject>& token)
    {
        return ERR_OK;
    }

    virtual ErrCode PromoteCurrentToCandidateMasterProcess(bool isInsertToHead)
    {
        return ERR_OK;
    }
    virtual ErrCode DemoteCurrentFromCandidateMasterProcess()
    {
        return ERR_OK;
    }

    virtual int StartUserTestProcess(
        const AAFwk::Want &want, const sptr<IRemoteObject> &observer, const BundleInfo &bundleInfo, int32_t userId)
    {
        return 0;
    }

    virtual int32_t RegisterAppForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer)
    {
        return 0;
    }

    virtual int32_t RegisterAbilityForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer)
    {
        return 0;
    }

    virtual int32_t UnregisterAppForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer)
    {
        return 0;
    }

    virtual int32_t UnregisterAbilityForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer)
    {
        return 0;
    }

    virtual int32_t RegisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer)
    {
        return 0;
    }

    virtual int32_t UnregisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer)
    {
        return 0;
    }

    virtual int32_t StartNativeProcessForDebugger(const AAFwk::Want &want) override
    {
        return 0;
    }

    virtual int FinishUserTest(const std::string& msg, int64_t resultCode, const std::string& bundleName)
    {
        return 0;
    }

    virtual int GetProcessRunningInformation(RunningProcessInfo &info)
    {
        return 0;
    }

    virtual void RegisterAppStateCallback(const sptr<IAppStateCallback>& callback)
    {
        callback_ = callback;
    }

    int32_t CheckPermissionImpl([[maybe_unused]] const int32_t recordId, const std::string& data)
    {
        data_ = data;
        return 0;
    }

    virtual int32_t JudgeSandboxByPid(pid_t pid, bool &isSandbox)
    {
        isSandbox = isSandbox_;
        return judgeSandboxByPidRet_;
    }

    void KillApplicationImpl(const std::string& data)
    {
        data_ = data;
    }

    const std::string& GetData() const
    {
        return data_;
    }

    void Wait()
    {
        sem_.Wait();
    }

    void Post()
    {
        sem_.Post();
    }

    void UpdateState() const
    {
        if (!callback_) {
            return;
        }
        AppProcessData processData;
        processData.pid = 1;
        processData.appState = ApplicationState::APP_STATE_CREATE;
        callback_->OnAppStateChanged(processData);
    }

    void Terminate(const sptr<IRemoteObject>& token) const
    {
        if (!callback_) {
            return;
        }
        AbilityState st = AbilityState::ABILITY_STATE_CREATE;
        callback_->OnAbilityRequestDone(token, st);
    }

    MOCK_METHOD4(SendRequest, int(uint32_t, MessageParcel&, MessageParcel&, MessageOption&));

    int InvokeSendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        code_ = code;

        return 0;
    }

    int code_;

    virtual ErrCode SetAppFreezeFilter(int32_t pid, bool& funcResult)
    {
        return ERR_OK;
    }

    virtual int32_t ChangeAppGcState(pid_t pid, int32_t state, uint64_t tid = 0)
    {
        return 0;
    }

    int32_t RegisterAppRunningStatusListener(const sptr<IRemoteObject> &listener)
    {
        return 0;
    }

    int32_t UnregisterAppRunningStatusListener(const sptr<IRemoteObject> &listener)
    {
        return 0;
    }

    int32_t DumpJsHeapMemory(const OHOS::AppExecFwk::JsHeapDumpInfo &info)
    {
        return 0;
    }

    int32_t DumpCjHeapMemory(const OHOS::AppExecFwk::CjHeapDumpInfo &info)
    {
        return 0;
    }

private:
    Semaphore sem_;
    std::string data_;
    sptr<IAppStateCallback> callback_;

public:
    uint32_t judgeSandboxByPidRet_ = 0;
    bool isSandbox_ = false;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_MGR_SERVICE_H
