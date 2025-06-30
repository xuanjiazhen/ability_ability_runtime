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

#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include "semaphore_ex.h"
#include "app_scheduler_interface.h"
#include "app_mgr_stub.h"
#include "hilog_tag_wrapper.h"
#include "app_malloc_info.h"
#include "app_jsheap_mem_info.h"
#include "app_cjheap_mem_info.h"

namespace OHOS {
namespace AppExecFwk {
class MockAppMgrService : public AppMgrStub {
public:
    MOCK_METHOD6(LoadAbility,
        void(const sptr<IRemoteObject>& token, const sptr<IRemoteObject>& preToken,
            const std::shared_ptr<AbilityInfo>& abilityInfo, const std::shared_ptr<ApplicationInfo>& appInfo,
            const std::shared_ptr<AAFwk::Want>& want, int32_t abilityRecordId));
    MOCK_METHOD2(TerminateAbility, void(const sptr<IRemoteObject>& token, bool isClearMissionFlag));
    MOCK_METHOD2(UpdateAbilityState, void(const sptr<IRemoteObject>& token, const AbilityState state));
    MOCK_METHOD1(SetAppFreezingTime, void(int time));
    MOCK_METHOD1(GetAppFreezingTime, void(int& time));
    MOCK_METHOD1(AddAbilityStageDone, void(const int32_t recordId));
    MOCK_METHOD1(StartupResidentProcess, void(const std::vector<AppExecFwk::BundleInfo>& bundleInfos));
    MOCK_METHOD1(NotifyMemoryLevel, int(int32_t level));
    MOCK_METHOD1(NotifyProcMemoryLevel, int32_t(const std::map<pid_t, MemoryLevel> &procLevelMap));
    MOCK_METHOD2(DumpHeapMemory, int(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo));
    MOCK_METHOD2(GetProcessRunningInfosByUserId, int(std::vector<RunningProcessInfo>& info, int32_t userId));
    MOCK_METHOD4(StartUserTestProcess, int(const AAFwk::Want& want, const sptr<IRemoteObject>& observer,
        const BundleInfo& bundleInfo, int32_t userId));
    MOCK_METHOD3(FinishUserTest, ErrCode(const std::string& msg, int64_t resultCode,
        const std::string& bundleName));
    MOCK_METHOD3(ScheduleAcceptWantDone, void(const int32_t recordId, const AAFwk::Want& want,
        const std::string& flag));
    MOCK_METHOD2(GetAbilityRecordsByProcessID, int(const int pid, std::vector<sptr<IRemoteObject>>& tokens));
    MOCK_METHOD0(PreStartNWebSpawnProcess, int());
    MOCK_METHOD6(StartRenderProcess,
                 int(const std::string &renderParam, int32_t ipcFd,
                     int32_t sharedFd, int32_t crashFd, pid_t &renderPid, bool isGPU));
    MOCK_METHOD1(AttachRenderProcess, ErrCode(const sptr<IRemoteObject>& renderScheduler));
    MOCK_METHOD1(SaveBrowserChannel, ErrCode(const sptr<IRemoteObject>& browser));
    MOCK_METHOD2(GetRenderProcessTerminationStatus, ErrCode(pid_t renderPid, int& status));
    MOCK_METHOD1(GetConfiguration, int32_t(Configuration& config));
    MOCK_METHOD2(UpdateConfiguration, int32_t(const Configuration& config, const int32_t userId));
    MOCK_METHOD1(RegisterConfigurationObserver, int32_t(const sptr<IConfigurationObserver>& observer));
    MOCK_METHOD1(UnregisterConfigurationObserver, int32_t(const sptr<IConfigurationObserver>& observer));
    MOCK_METHOD1(GetAppRunningStateByBundleName, bool(const std::string& bundleName));
    MOCK_METHOD2(NotifyLoadRepairPatch, int32_t(const std::string& bundleName,
        const sptr<IQuickFixCallback>& callback));
    MOCK_METHOD2(NotifyHotReloadPage, int32_t(const std::string& bundleName, const sptr<IQuickFixCallback>& callback));
    MOCK_METHOD2(NotifyUnLoadRepairPatch, int32_t(const std::string& bundleName,
        const sptr<IQuickFixCallback>& callback));
    MOCK_METHOD2(IsSharedBundleRunning, bool(const std::string &bundleName, uint32_t versionCode));
    MOCK_METHOD1(NotifyAppFault, int32_t(const FaultData &faultData));
    MOCK_METHOD1(NotifyAppFaultBySA, int32_t(const AppFaultDataBySA &faultData));
    MOCK_METHOD2(GetProcessMemoryByPid, int32_t(const int32_t pid, int32_t & memorySize));
    MOCK_METHOD3(GetRunningProcessInformation, int32_t(const std::string & bundleName, int32_t userId,
        std::vector<RunningProcessInfo> &info));
#ifdef SUPPORT_CHILD_PROCESS
    MOCK_METHOD2(StartChildProcess, int32_t(pid_t &childPid, const ChildProcessRequest &request));
    MOCK_METHOD1(GetChildProcessInfoForSelf, int32_t(ChildProcessInfo &info));
    MOCK_METHOD1(AttachChildProcess, void(const sptr<IRemoteObject> &childScheduler));
    MOCK_METHOD0(ExitChildProcessSafely, void());
#endif // SUPPORT_CHILD_PROCESS
    MOCK_METHOD1(RegisterRenderStateObserver, int32_t(const sptr<IRenderStateObserver> &observer));
    MOCK_METHOD1(UnregisterRenderStateObserver, int32_t(const sptr<IRenderStateObserver> &observer));
    MOCK_METHOD1(RegisterNativeChildExitNotify, int32_t(const sptr<INativeChildNotify> notify));
    MOCK_METHOD1(UnregisterNativeChildExitNotify, int32_t(const sptr<INativeChildNotify> notify));
    MOCK_METHOD2(UpdateRenderState, int32_t(pid_t renderPid, int32_t state));
    MOCK_METHOD2(GetRunningMultiAppInfoByBundleName, int32_t(const std::string &bundleName,
        RunningMultiAppInfo &info));
    MOCK_METHOD1(GetAllRunningInstanceKeysBySelf, int32_t(std::vector<std::string> &instanceKeys));
    MOCK_METHOD3(GetAllRunningInstanceKeysByBundleName, int32_t(const std::string &bundleName,
        std::vector<std::string> &instanceKeys, int32_t userId));
    MOCK_METHOD1(SetSupportedProcessCacheSelf, int32_t(bool isSupported));
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

    ErrCode AttachApplication(const sptr<IRemoteObject>& app)
    {
        GTEST_LOG_(INFO) << "MockAppMgrService::AttachApplication called";
        Attached_ = true;
        EXPECT_TRUE(Attached_);
        Appthread_ = iface_cast<IAppScheduler>(app);
        return 0;
    }

    virtual ErrCode ApplicationForegrounded(int32_t recordId)
    {
        GTEST_LOG_(INFO) << "MockAppMgrService::ApplicationForegrounded called";
        Foregrounded_ = true;
        EXPECT_TRUE(Foregrounded_);
        return 0;
    }

    virtual ErrCode ApplicationBackgrounded(int32_t recordId)
    {
        GTEST_LOG_(INFO) << "MockAppMgrService::ApplicationBackgrounded called";
        Backgrounded_ = true;
        EXPECT_TRUE(Backgrounded_);
        return 0;
    }

    virtual ErrCode ApplicationTerminated(int32_t recordId)
    {
        GTEST_LOG_(INFO) << "MockAppMgrService::ApplicationTerminated called";
        Terminated_ = true;
        EXPECT_TRUE(Terminated_);
        return 0;
    }

    virtual void AbilityCleaned(const sptr<IRemoteObject>& token)
    {
        GTEST_LOG_(INFO) << "MockAppMgrService::AbilityCleaned called";
        Cleaned_ = true;
        EXPECT_TRUE(Cleaned_);
    }

    MOCK_METHOD4(UpdateApplicationInfoInstalled, int(const std::string&, const int uid, const std::string&, bool));

    MOCK_METHOD3(ForceKillApplication, int(const std::string& appName, const int userId, const int appIndex));
    MOCK_METHOD1(KillProcessesByAccessTokenId, int32_t(const uint32_t accessTokenId));
    MOCK_METHOD3(KillApplication, int(const std::string& appName, const bool clearPageStack, int32_t appIndex));
    MOCK_METHOD3(KillApplicationByUid, int(const std::string&, const int uid, const std::string&));
    MOCK_METHOD1(IsFinalAppProcess, ErrCode(bool& funcResult));

    virtual int32_t ClearUpApplicationData(const std::string& appName, int32_t appCloneIndex) override
    {
        return 0;
    }

    virtual int GetProcessRunningInformation(RunningProcessInfo &info)
    {
        return 0;
    }

    int IsBackgroundRunningRestricted(const std::string& appName)
    {
        return 0;
    }

    virtual int GetAllRunningProcesses(std::vector<RunningProcessInfo>& info) override
    {
        return 0;
    }

    virtual int GetRunningProcessesByBundleType(const BundleType bundleType,
        std::vector<RunningProcessInfo>& info) override
    {
        return 0;
    }

    virtual int GetAllRenderProcesses(std::vector<RenderProcessInfo>& info) override
    {
        return 0;
    }

    virtual int GetAllChildrenProcesses(std::vector<ChildProcessInfo> &info) override
    {
        return 0;
    }

    virtual int32_t StartNativeProcessForDebugger(const AAFwk::Want &want) override
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

    void ScheduleTerminateApplication(bool isLastProcess = false)
    {
        if (Appthread_ != nullptr) {
            Appthread_->ScheduleTerminateApplication(isLastProcess);
        }
    }

    void ScheduleLaunchApplication(const AppLaunchData& lanchdata, const Configuration& config)
    {
        if (Appthread_ != nullptr) {
            Appthread_->ScheduleLaunchApplication(lanchdata, config);
        }
    }

    void ScheduleForegroundApplication()
    {
        if (Appthread_ != nullptr) {
            Appthread_->ScheduleForegroundApplication();
        }
    }

    void ScheduleBackgroundApplication()
    {
        if (Appthread_ != nullptr) {
            Appthread_->ScheduleBackgroundApplication();
        }
    }

    void ScheduleShrinkMemory(const int32_t level)
    {
        if (Appthread_ != nullptr) {
            Appthread_->ScheduleShrinkMemory(level);
        }
    }

    void ScheduleLowMemory()
    {
        if (Appthread_ != nullptr) {
            Appthread_->ScheduleLowMemory();
        }
    }

    void ScheduleLaunchAbility(const AbilityInfo& abilityinf, const sptr<IRemoteObject>& token,
        const std::shared_ptr<AAFwk::Want>& want)
    {
        if (Appthread_ != nullptr) {
            Appthread_->ScheduleLaunchAbility(abilityinf, token, want, 0);
        }
    }

    void ScheduleCleanAbility(const sptr<IRemoteObject>& token)
    {
        if (Appthread_ != nullptr) {
            Appthread_->ScheduleCleanAbility(token);
        }
    }

    void ScheduleProfileChanged(const Profile& profile)
    {
        if (Appthread_ != nullptr) {
            Appthread_->ScheduleProfileChanged(profile);
        }
    }

    void ScheduleConfigurationUpdated(const Configuration& config)
    {
        if (Appthread_ != nullptr) {
            Appthread_->ScheduleConfigurationUpdated(config);
        }
    }

    sptr<IAppScheduler> GetAppthread()
    {
        return Appthread_;
    }

    bool IsAttached()
    {
        TAG_LOGI(AAFwkTag::TEST, "MockAppMgrService::IsAttached Attached_ = %{public}d", Attached_);
        return Attached_;
    }

    bool IsForegrounded()
    {
        TAG_LOGI(AAFwkTag::TEST, "MockAppMgrService::IsForegrounded Foregrounded_ = %{public}d", Foregrounded_);
        return Foregrounded_;
    }

    bool IsBackgrounded()
    {
        TAG_LOGI(AAFwkTag::TEST, "MockAppMgrService::IsBackgrounded Backgrounded_ = %{public}d", Backgrounded_);
        return Backgrounded_;
    }

    bool IsTerminated()
    {
        TAG_LOGI(AAFwkTag::TEST, "MockAppMgrService::IsTerminated Terminated_ = %{public}d", Terminated_);
        return Terminated_;
    }

    void init()
    {
        TAG_LOGI(AAFwkTag::TEST, "MockAppMgrService::init called");
        Attached_ = false;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient>& recipient)
    {
        return true;
    }

    int32_t GetRunningMultiAppInfoByBundleName(const std::string &bundleName,
        RunningMultiAppInfo &info)
    {
        return 0;
    }

    ErrCode PreloadApplication(
        const std::string& bundleName, int32_t userId, PreloadMode preloadMode, int32_t appIndex)
    {
        return 0;
    }

    ErrCode GetAmsMgr(sptr<IAmsMgr>& funcResult)
    {
        funcResult = nullptr;
        return 0;
    }

    ErrCode IsTerminatingByPid(int32_t pid, bool& isTerminating)
    {
        return 0;
    }

    ErrCode HasDumpPermission(bool& funcResult)
    {
        return 0;
    }

    ErrCode DumpJsHeapMemory(const JsHeapDumpInfo& info)
    {
        return 0;
    }

    ErrCode DumpCjHeapMemory(const CjHeapDumpInfo& info)
    {
        return 0;
    }

    ErrCode GetForegroundApplications(
        std::vector<AppStateData>& list)
    {
        return 0;
    }

    ErrCode FinishUserTest(
        const std::string& msg,
        int64_t resultCode,
        const std::string& bundleName)
    {
        return 0;
    }

    ErrCode SetAppFreezeFilter(
        int32_t pid,
        bool& funcResult)
    {
        funcResult = false;
        return 0;
    }

    ErrCode SetContinuousTaskProcess(
        int32_t pid,
        bool isContinuousTask)
    {
        return 0;
    }
    ErrCode GetRunningProcessInfoByPid(
        int32_t pid,
        RunningProcessInfo& info)
    {
        return 0;
    }

    ErrCode GetRunningProcessInfoByChildProcessPid(
        int32_t childPid,
        RunningProcessInfo& info)
    {
        return 0;
    }

    ErrCode NotifyPageShow(
        const sptr<IRemoteObject>& token,
        const PageStateData& pageStateData)
    {
        return 0;
    }

    ErrCode NotifyPageHide(
        const sptr<IRemoteObject>& token,
        const PageStateData& pageStateData)
    {
        return 0;
    }

    ErrCode SignRestartAppFlag(
        int32_t uid,
        const std::string& instanceKey)
    {
        return 0;
    }

    ErrCode GetAppRunningUniqueIdByPid(
        int32_t pid,
        std::string& appRunningUniqueId)
    {
        return 0;
    }

    ErrCode GetAllUIExtensionRootHostPid(
        int32_t pid,
        std::vector<int32_t>& hostPids)
    {
        return 0;
    }

    ErrCode GetAllUIExtensionProviderPid(
        int32_t hostPid,
        std::vector<int32_t>& providerPids)
    {
        return 0;
    }

    ErrCode NotifyMemorySizeStateChanged(
        int32_t memorySizeState)
    {
        return 0;
    }

    ErrCode SaveBrowserChannel(
        const sptr<IRemoteObject>& browser)
    {
        return 0;
    }

    ErrCode CheckCallingIsUserTestMode(
        int32_t pid,
        bool& isUserTest)
    {
        return 0;
    }

    ErrCode SetAppAssertionPauseState(
        bool flag)
    {
        return 0;
    }

    ErrCode NotifyProcessDependedOnWeb()
    {
        return 0;
    }

    ErrCode KillProcessDependedOnWeb()
    {
        return 0;
    }

    ErrCode RestartResidentProcessDependedOnWeb()
    {
        return 0;
    }

    ErrCode KillAppSelfWithInstanceKey(
        const std::string& instanceKey,
        bool clearPageStack,
        const std::string& reason)
    {
        return 0;
    }

    ErrCode UpdateInstanceKeyBySpecifiedId(
        int32_t specifiedId,
        const std::string& instanceKey)
    {
        return 0;
    }

    ErrCode IsSpecifiedModuleLoaded(
        const Want& want,
        const AbilityInfo& abilityInfo,
        bool& funcResult)
    {
        return 0;
    }

    ErrCode UpdateProcessMemoryState(
        const std::vector<ProcessMemoryState>& procMemState)
    {
        return 0;
    }

    ErrCode GetKilledProcessInfo(
        int32_t pid,
        int32_t uid,
        KilledProcessInfo& info)
    {
        return 0;
    }

    ErrCode LaunchAbility(
        const sptr<IRemoteObject>& token)
    {
        return 0;
    }

    ErrCode UpdateConfigurationForBackgroundApp(
        const std::vector<BackgroundAppInfo>& appInfos,
        const ConfigurationPolicy& policy,
        int32_t userId)
    {
        return 0;
    }

    ErrCode RegisterNativeChildExitNotify(
        const sptr<INativeChildNotify>& notify)
    {
        return 0;
    }

    ErrCode UnregisterNativeChildExitNotify(
        const sptr<INativeChildNotify>& notify)
    {
        return 0;
    }
    ErrCode PromoteCurrentToCandidateMasterProcess(
        bool isInsertToHead)
    {
        return 0;
    }

    ErrCode DemoteCurrentFromCandidateMasterProcess()
    {
        return 0;
    }
private:
    bool Attached_ = false;
    bool Foregrounded_ = false;
    bool Backgrounded_ = false;
    bool Terminated_ = false;
    bool Cleaned_ = false;
    sptr<IAppScheduler> Appthread_ = nullptr;
    Semaphore sem_;
    std::string data_;
    sptr<IAppStateCallback> callback_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_MGR_SERVICE_H
