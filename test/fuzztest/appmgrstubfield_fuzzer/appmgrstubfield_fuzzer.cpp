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

#include "appmgrstubfield_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "app_mgr_stub.h"
#include "bundle_info.h"
#include "configuration.h"
#include "message_parcel.h"
#include "securec.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string APPMGR_INTERFACE_TOKEN = u"ohos.appexecfwk.AppMgr";
}

class AppMgrStubFieldFuzz : public AppMgrStub {
public:
    AppMgrStubFieldFuzz() = default;
    ~AppMgrStubFieldFuzz() = default;

    void AttachApplication(const sptr<IRemoteObject> &app) override {}
    void ApplicationForegrounded(const int32_t recordId) override {}
    void ApplicationBackgrounded(const int32_t recordId) override {}
    void ApplicationTerminated(const int32_t recordId) override {}
    void AbilityCleaned(const sptr<IRemoteObject> &token) override {}
    sptr<IAmsMgr> GetAmsMgr() override { return nullptr; }
    int32_t ClearUpApplicationData(const std::string &bundleName, int32_t appCloneIndex, int32_t userId) override
    {
        return 0;
    }
    int32_t ClearUpApplicationDataBySelf(int32_t userId) override { return 0; }
    int GetAllRunningProcesses(std::vector<RunningProcessInfo> &info) override { return 0; }
    int32_t GetRunningMultiAppInfoByBundleName(const std::string &bundleName, RunningMultiAppInfo &info) override
    {
        return 0;
    }
    int32_t GetAllRunningInstanceKeysBySelf(std::vector<std::string> &instanceKeys) override { return 0; }
    int32_t GetAllRunningInstanceKeysByBundleName(const std::string &bundleName,
        std::vector<std::string> &instanceKeys, int32_t userId) override { return 0; }
    int GetRunningProcessesByBundleType(const BundleType bundleType,
        std::vector<RunningProcessInfo> &info) override { return 0; }
    int GetAllRenderProcesses(std::vector<RenderProcessInfo> &info) override { return 0; }
    int GetAllChildrenProcesses(std::vector<ChildProcessInfo> &info) override { return 0; }
    int GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId) override { return 0; }
    int32_t GetProcessRunningInfosByAccessTokenId(uint32_t accessTokenId,
        std::vector<RunningProcessInfo> &info) override { return 0; }
    int32_t GetProcessRunningInformation(RunningProcessInfo &info) override { return 0; }
    int NotifyMemoryLevel(int32_t level) override { return 0; }
    int32_t NotifyProcMemoryLevel(const std::map<pid_t, MemoryLevel> &procLevelMap) override { return 0; }
    int DumpHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo) override { return 0; }
    void AddAbilityStageDone(const int32_t recordId) override {}
    int DumpJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info) override { return 0; }
    int DumpCjHeapMemory(OHOS::AppExecFwk::CjHeapDumpInfo &info) override { return 0; }
    int DumpMem(OHOS::AppExecFwk::MemDumpInfo &info, sptr<IMemDumpCallback> callback) override { return 0; }
    int ReportDumpMemResult(sptr<IMemDumpCallback> callback, const std::string &dumpResult) override { return 0; }
    void StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos) override {}
    int32_t RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer,
        const std::vector<std::string> &bundleNameList) override { return 0; }
    int32_t UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer) override
    {
        return 0;
    }
    int32_t RegisterAbilityForegroundStateObserver(
        const sptr<IAbilityForegroundStateObserver> &observer) override { return 0; }
    int32_t UnregisterAbilityForegroundStateObserver(
        const sptr<IAbilityForegroundStateObserver> &observer) override { return 0; }
    int32_t GetForegroundApplications(std::vector<AppStateData> &list) override { return 0; }
    int StartUserTestProcess(const AAFwk::Want &want, const sptr<IRemoteObject> &observer,
        const BundleInfo &bundleInfo, int32_t userId) override { return 0; }
    int FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName) override
    {
        return 0;
    }
    void ScheduleAcceptWantDone(const int32_t recordId, const AAFwk::Want &want, const std::string &flag) override {}
    void ScheduleNewProcessRequestDone(const int32_t recordId, const AAFwk::Want &want,
        const std::string &flag) override {}
    int GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens) override { return 0; }
    int PreStartNWebSpawnProcess() override { return 0; }
    int StartRenderProcess(const std::string &renderParam, int32_t ipcFd, int32_t sharedFd, int32_t crashFd,
        pid_t &renderPid, bool isGPU) override { return 0; }
    void AttachRenderProcess(const sptr<IRemoteObject> &renderScheduler) override {}
    int GetRenderProcessTerminationStatus(pid_t renderPid, int &status) override { return 0; }
    int32_t GetConfiguration(Configuration &config) override { return 0; }
    int32_t GetConfiguration(Configuration &config, int32_t userid) override { return 0; }
    int32_t UpdateConfiguration(const Configuration &config, const int32_t userId) override { return 0; }
    int32_t UpdateConfigurationByUserIds(const Configuration &config,
        const std::vector<int32_t> userIds) override { return 0; }
    int32_t UpdateConfigurationByBundleName(const Configuration &config, const std::string &name,
        int32_t appIndex) override { return 0; }
    int32_t RegisterConfigurationObserver(const sptr<IConfigurationObserver> &observer,
        const int32_t userId) override { return 0; }
    int32_t UnregisterConfigurationObserver(const sptr<IConfigurationObserver> &observer) override { return 0; }
    bool GetAppRunningStateByBundleName(const std::string &bundleName) override { return false; }
    int32_t NotifyLoadRepairPatch(const std::string &bundleName,
        const sptr<IQuickFixCallback> &callback) override { return 0; }
    int32_t NotifyHotReloadPage(const std::string &bundleName,
        const sptr<IQuickFixCallback> &callback) override { return 0; }
    int32_t NotifyUnLoadRepairPatch(const std::string &bundleName,
        const sptr<IQuickFixCallback> &callback) override { return 0; }
    int32_t NotifyAppFault(const FaultData &faultData) override { return 0; }
    int32_t NotifyAppFaultBySA(const AppFaultDataBySA &faultData) override { return 0; }
    bool SetAppFreezeFilter(int32_t pid) override { return false; }
    void UpdateFreezeExcludedPid(bool isAdd, int32_t targetPid, int32_t profilerPid) override {}
    bool IsSharedBundleRunning(const std::string &bundleName, uint32_t versionCode) override { return false; }
    int32_t StartNativeProcessForDebugger(const AAFwk::Want &want) override { return 0; }
    int32_t GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid) override { return 0; }
    int32_t GetProcessMemoryByPid(const int32_t pid, int32_t &memorySize) override { return 0; }
    int32_t GetRunningProcessInformation(const std::string &bundleName, int32_t userId,
        std::vector<RunningProcessInfo> &info) override { return 0; }
    int32_t ChangeAppGcState(pid_t pid, int32_t state, uint64_t tid) override { return 0; }
    int32_t RegisterAppRunningStatusListener(const sptr<IRemoteObject> &listener) override { return 0; }
    int32_t UnregisterAppRunningStatusListener(const sptr<IRemoteObject> &listener) override { return 0; }
    int32_t RegisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer) override
    {
        return 0;
    }
    int32_t UnregisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer) override
    {
        return 0;
    }
    int32_t IsApplicationRunning(const std::string &bundleName, bool &isRunning) override { return 0; }
    int32_t IsAppRunning(const std::string &bundleName, int32_t appCloneIndex, bool &isRunning) override { return 0; }
    int32_t IsAppRunningByBundleNameAndUserId(const std::string &bundleName, int32_t userId,
        bool &isRunning) override { return 0; }
#ifdef SUPPORT_CHILD_PROCESS
    int32_t StartChildProcess(pid_t &childPid, const ChildProcessRequest &request) override { return 0; }
    int32_t GetChildProcessInfoForSelf(ChildProcessInfo &info) override { return 0; }
    void AttachChildProcess(const sptr<IRemoteObject> &childScheduler) override {}
    void ExitChildProcessSafely() override {}
#endif // SUPPORT_CHILD_PROCESS
    bool IsFinalAppProcess() override { return false; }
    int32_t RegisterRenderStateObserver(const sptr<IRenderStateObserver> &observer) override { return 0; }
    int32_t UnregisterRenderStateObserver(const sptr<IRenderStateObserver> &observer) override { return 0; }
    int32_t RegisterKiaInterceptor(const sptr<IKiaInterceptor> &interceptor) override { return 0; }
    int32_t CheckIsKiaProcess(pid_t pid, bool &isKia) override { return 0; }
    int32_t UpdateRenderState(pid_t renderPid, int32_t state) override { return 0; }
    int32_t SetSupportedProcessCacheSelf(bool isSupport) override { return 0; }
    int32_t SetSupportedProcessCache(int32_t pid, bool isSupport) override { return 0; }
    int32_t IsProcessCacheSupported(int32_t pid, bool &isSupported) override { return 0; }
    int32_t SetProcessCacheEnable(int32_t pid, bool enable) override { return 0; }
    void SaveBrowserChannel(sptr<IRemoteObject> browser) override {}
    int32_t GetSupportedProcessCachePids(const std::string &bundleName,
        std::vector<int32_t> &pidList) override { return 0; }
};

bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    uint32_t code = FuzzUtil::GetU32Data(data);
    FuzzedDataProvider fdp(data + FuzzUtil::U32_AT_SIZE, size - FuzzUtil::U32_AT_SIZE);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::APP_ATTACH_APPLICATION);

    parcel.WriteInterfaceToken(APPMGR_INTERFACE_TOKEN);

    switch (code % 10) {
        case 0:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::APP_ATTACH_APPLICATION);
            parcel.WriteRemoteObject(nullptr);
            break;
        case 1:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::APP_APPLICATION_TERMINATED);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 2:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::APP_ABILITY_CLEANED);
            parcel.WriteRemoteObject(nullptr);
            break;
        case 3:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::APP_CLEAR_UP_APPLICATION_DATA);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 4:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::APP_ADD_ABILITY_STAGE_INFO_DONE);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 5:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::APP_NOTIFY_MEMORY_LEVEL);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 6: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::STARTUP_RESIDENT_PROCESS);
            int32_t infoSize = static_cast<int32_t>(fdp.ConsumeIntegral<uint8_t>() % 4);
            parcel.WriteInt32(infoSize);
            for (int32_t i = 0; i < infoSize; i++) {
                AppExecFwk::BundleInfo bundleInfo;
                parcel.WriteParcelable(&bundleInfo);
            }
            break;
        }
        case 7: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::UPDATE_CONFIGURATION);
            Configuration config;
            parcel.WriteParcelable(&config);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 8:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::SCHEDULE_ACCEPT_WANT_DONE);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            break;
        case 9: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::MAKE_IMAGE);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            bool flag = fdp.ConsumeBool();
            parcel.WriteBool(flag);
            if (flag) {
                parcel.WriteRemoteObject(nullptr);
            }
            break;
        }
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<AppMgrStub> appMgrStub = std::make_shared<AppMgrStubFieldFuzz>();
    appMgrStub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        std::cout << "invalid data" << std::endl;
        return 0;
    }

    if (size < OHOS::FuzzUtil::U32_AT_SIZE) {
        return 0;
    }

    char *ch = static_cast<char *>(malloc(size + 1));
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size + 1, data, size) != EOK) {
        std::cout << "copy failed." << std::endl;
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}
