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

#include "appschedulerhost_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <securec.h>
#include <fuzzer/FuzzedDataProvider.h>

#include "app_scheduler_host.h"
#include "message_parcel.h"
#include "attack_vectors.h"
#include "fuzz_util.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;
using namespace OHOS::FuzzUtil;

namespace OHOS {
namespace {
const std::u16string APP_SCHEDULER_INTERFACE_TOKEN = u"ohos.appexecfwk.AppScheduler";
constexpr uint8_t SCHEDULER_HANDLE_COUNT = 40;
constexpr size_t TARGET_BUF_SIZE = 256;

class AppSchedulerHostFUZZ : public AppSchedulerHost {
public:
    AppSchedulerHostFUZZ() = default;
    ~AppSchedulerHostFUZZ() = default;

    bool ScheduleForegroundApplication() override { return false; }
    void ScheduleBackgroundApplication() override {}
    void ScheduleTerminateApplication(bool isLastProcess = false) override {}
    void ScheduleShrinkMemory(const int level) override {}
    void ScheduleLowMemory() override {}
    void ScheduleMemoryLevel(int32_t level, bool isShellCall = false) override {}
    void ScheduleHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo) override {}
    void ScheduleJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info) override {}
    void ScheduleCjHeapMemory(OHOS::AppExecFwk::CjHeapDumpInfo &info) override {}
    void ScheduleMem(OHOS::AppExecFwk::MemDumpInfo &info, sptr<IMemDumpCallback> callback) override {}
    void ScheduleLaunchApplication(const AppLaunchData &launchData, const Configuration &config) override {}
    void ScheduleUpdateApplicationInfoInstalled(const ApplicationInfo &appInfo, const std::string &moduleName) override {}
    void ScheduleAbilityStage(const HapModuleInfo &abilityStage) override {}
    void ScheduleLaunchAbility(const AbilityInfo &abilityInfo, const sptr<IRemoteObject> &token,
        const std::shared_ptr<AAFwk::Want> &want, int32_t abilityRecordId,
        std::shared_ptr<AppUpdateInfo> updateInfo = nullptr) override {}
    void ScheduleCleanAbility(const sptr<IRemoteObject> &token, bool isCacheProcess = false) override {}
    void ScheduleProfileChanged(const Profile &profile) override {}
    void ScheduleConfigurationUpdated(const Configuration &config,
        ConfigUpdateReason reason = ConfigUpdateReason::CONFIG_UPDATE_REASON_DEFAULT) override {}
    void ScheduleProcessSecurityExit() override {}
    void ScheduleClearPageStack() override {}
    void ScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName) override {}
    void SchedulePrepareTerminate(const std::string &moduleName) override {}
    void ScheduleNewProcessRequest(const AAFwk::Want &want, const std::string &moduleName) override {}
    int32_t ScheduleNotifyLoadRepairPatch(const std::string &bundleName,
        const sptr<IQuickFixCallback> &callback, const int32_t recordId) override { return 0; }
    int32_t ScheduleNotifyHotReloadPage(const sptr<IQuickFixCallback> &callback, const int32_t recordId) override
    {
        return 0;
    }
    int32_t ScheduleNotifyUnLoadRepairPatch(const std::string &bundleName,
        const sptr<IQuickFixCallback> &callback, const int32_t recordId) override { return 0; }
    int32_t ScheduleNotifyAppFault(const FaultData &faultData) override { return 0; }
    int32_t ScheduleChangeAppGcState(int32_t state, uint64_t tid = 0) override { return 0; }
    void AttachAppDebug(bool isDebugFromLocal) override {}
    void DetachAppDebug() override {}
    int32_t ScheduleDumpIpcStart(std::string &result) override { return 0; }
    int32_t ScheduleDumpIpcStop(std::string &result) override { return 0; }
    int32_t ScheduleDumpIpcStat(std::string &result) override { return 0; }
    void ScheduleCacheProcess() override {}
    int32_t ScheduleDumpFfrt(std::string &result) override { return 0; }
    int32_t ScheduleDumpArkWeb(const std::string &customArgs, std::string &result) override { return 0; }
    void SetWatchdogBackgroundStatus(bool status) override {}
    void OnLoadAbilityFinished(uint64_t callbackId, int32_t pid) override {}
};

sptr<AppSchedulerHostFUZZ> g_stub = nullptr;

void InitAppSchedulerStub()
{
    if (g_stub == nullptr) {
        g_stub = new AppSchedulerHostFUZZ();
    }
}

bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    InitAppSchedulerStub();
    if (g_stub == nullptr) {
        return false;
    }
    FUZZ_EXTRACT_CODE(data, size);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = 0;
    uint8_t tarPos = static_cast<uint8_t>(code % SCHEDULER_HANDLE_COUNT);
    switch (tarPos) {
        case 0: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_FOREGROUND_APPLICATION_TRANSACTION);
            break;
        }
        case 1: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_BACKGROUND_APPLICATION_TRANSACTION);
            break;
        }
        case 2: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            parcel.WriteBool(fdp.ConsumeBool());
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_TERMINATE_APPLICATION_TRANSACTION);
            break;
        }
        case 3: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            parcel.WriteInt32(BuildIntegerOverflow(fdp));
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_SHRINK_MEMORY_APPLICATION_TRANSACTION);
            break;
        }
        case 4: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_LOWMEMORY_APPLICATION_TRANSACTION);
            break;
        }
        case 5: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            parcel.WriteInt32(BuildIntegerOverflow(fdp));
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_MEMORYLEVEL_APPLICATION_TRANSACTION);
            break;
        }
        case 6: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            parcel.WriteInt32(BuildIntegerOverflow(fdp));
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_HEAPMEMORY_APPLICATION_TRANSACTION);
            break;
        }
        case 7: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            JsHeapDumpInfo jsHeapDumpInfo;
            parcel.WriteParcelable(&jsHeapDumpInfo);
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_JSHEAP_MEMORY_APPLICATION_TRANSACTION);
            break;
        }
        case 8: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            CjHeapDumpInfo cjHeapDumpInfo;
            parcel.WriteParcelable(&cjHeapDumpInfo);
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_CJHEAP_MEMORY_APPLICATION_TRANSACTION);
            break;
        }
        case 9: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            MemDumpInfo memDumpInfo;
            parcel.WriteParcelable(&memDumpInfo);
            bool hasCallback = fdp.ConsumeBool();
            parcel.WriteBool(hasCallback);
            if (hasCallback) {
                parcel.WriteRemoteObject(nullptr);
            }
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_MEM_APPLICATION_TRANSACTION);
            break;
        }
        case 10: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            AppLaunchData appLaunchData;
            parcel.WriteParcelable(&appLaunchData);
            Configuration configuration;
            parcel.WriteParcelable(&configuration);
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_LAUNCH_APPLICATION_TRANSACTION);
            break;
        }
        case 11: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            AbilityInfo abilityInfo;
            parcel.WriteParcelable(&abilityInfo);
            bool hasToken = fdp.ConsumeBool();
            parcel.WriteBool(hasToken);
            if (hasToken) {
                parcel.WriteRemoteObject(nullptr);
            }
            WriteMaliciousWant(parcel, fdp);
            parcel.WriteInt32(BuildIntegerOverflow(fdp));
            bool hasUpdateInfo = fdp.ConsumeBool();
            parcel.WriteBool(hasUpdateInfo);
            if (hasUpdateInfo) {
                AppUpdateInfo updateInfo;
                parcel.WriteParcelable(&updateInfo);
            }
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_LAUNCH_ABILITY_TRANSACTION);
            break;
        }
        case 12: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteBool(fdp.ConsumeBool());
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_CLEAN_ABILITY_TRANSACTION);
            break;
        }
        case 13: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            Profile profile;
            parcel.WriteParcelable(&profile);
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_PROFILE_CHANGED_TRANSACTION);
            break;
        }
        case 14: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            Configuration configuration;
            parcel.WriteParcelable(&configuration);
            parcel.WriteUint8(fdp.ConsumeIntegral<uint8_t>());
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_CONFIGURATION_UPDATED);
            break;
        }
        case 15: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_PROCESS_SECURITY_EXIT_TRANSACTION);
            break;
        }
        case 16: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            HapModuleInfo hapModuleInfo;
            parcel.WriteParcelable(&hapModuleInfo);
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_ABILITY_STAGE_INFO);
            break;
        }
        case 17: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_CLEAR_PAGE_STACK);
            break;
        }
        case 18: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            WriteMaliciousWant(parcel, fdp);
            parcel.WriteString(BuildSpecialCharString(fdp));
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_ACCEPT_WANT);
            break;
        }
        case 19: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            parcel.WriteString(BuildSpecialCharString(fdp));
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_PREPARE_TERMINATE);
            break;
        }
        case 20: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            WriteMaliciousWant(parcel, fdp);
            parcel.WriteString(BuildSpecialCharString(fdp));
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NEW_PROCESS_REQUEST);
            break;
        }
        case 21: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            parcel.WriteString(BuildMaliciousBundleName(fdp));
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(BuildIntegerOverflow(fdp));
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NOTIFY_LOAD_REPAIR_PATCH);
            break;
        }
        case 22: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(BuildIntegerOverflow(fdp));
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NOTIFY_HOT_RELOAD_PAGE);
            break;
        }
        case 23: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            parcel.WriteString(BuildMaliciousBundleName(fdp));
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(BuildIntegerOverflow(fdp));
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NOTIFY_UNLOAD_REPAIR_PATCH);
            break;
        }
        case 24: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            ApplicationInfo appInfo;
            parcel.WriteParcelable(&appInfo);
            parcel.WriteString(BuildSpecialCharString(fdp));
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_UPDATE_APPLICATION_INFO_INSTALLED);
            break;
        }
        case 25: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            FaultData faultData;
            parcel.WriteParcelable(&faultData);
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NOTIFY_FAULT);
            break;
        }
        case 26: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            parcel.WriteInt32(BuildIntegerOverflow(fdp));
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::APP_GC_STATE_CHANGE);
            break;
        }
        case 27: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            parcel.WriteBool(fdp.ConsumeBool());
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_ATTACH_APP_DEBUG);
            break;
        }
        case 28: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_DETACH_APP_DEBUG);
            break;
        }
        case 29: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            JsHandleMapInfo jsHandleMapInfo;
            parcel.WriteParcelable(&jsHandleMapInfo);
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_JSHANDLE_MAP_APPLICATION_TRANSACTION);
            break;
        }
        case 30: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_DUMP_IPC_START);
            break;
        }
        case 31: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_DUMP_IPC_STOP);
            break;
        }
        case 32: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_DUMP_IPC_STAT);
            break;
        }
        case 33: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_DUMP_FFRT);
            break;
        }
        case 34: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            parcel.WriteString(BuildSpecialCharString(fdp));
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_DUMP_ARKWEB);
            break;
        }
        case 35: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_CACHE_PROCESS);
            break;
        }
        case 36: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            parcel.WriteBool(fdp.ConsumeBool());
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::WATCHDOG_BACKGROUND_STATUS_TRANSACTION);
            break;
        }
        case 37: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteInt32(BuildIntegerOverflow(fdp));
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::ON_LOAD_ABILITY_FINISHED);
            break;
        }
        case 38: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            AppUpdateInfo updateInfo;
            parcel.WriteParcelable(&updateInfo);
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_UPDATE_WORK_PROCESS_INFO);
            break;
        }
        case 39: {
            parcel.WriteInterfaceToken(APP_SCHEDULER_INTERFACE_TOKEN);
            actualCode = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_PRE_TEMPLATE_PROCESS_DEEP_FROZEN);
            break;
        }
        default:
            return false;
    }
    parcel.RewindRead(0);
    g_stub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
}

FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
