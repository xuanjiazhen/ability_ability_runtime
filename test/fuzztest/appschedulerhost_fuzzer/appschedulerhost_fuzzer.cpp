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
constexpr uint8_t SCHEDULER_HANDLE_COUNT = 13;
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

bool DoSomethingInterestingWithMyAPI(const char *rawData, FuzzedDataProvider &fdp)
{
    if (g_stub == nullptr) {
        return false;
    }
    uint32_t code = OHOS::FuzzUtil::GetU32Data(rawData);
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
        default:
            return false;
    }
    parcel.RewindRead(0);
    g_stub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::FuzzUtil::U32_AT_SIZE) {
        return 0;
    }
    InitAppSchedulerStub();
    char *buf = static_cast<char *>(malloc(size));
    if (buf == nullptr) {
        return 0;
    }
    if (memset_s(buf, size, 0, size) != EOK) {
        free(buf);
        return 0;
    }
    if (memcpy_s(buf, size, data, size) != EOK) {
        free(buf);
        return 0;
    }
    FuzzedDataProvider fdp(reinterpret_cast<const uint8_t *>(buf) + OHOS::FuzzUtil::U32_AT_SIZE,
        size - OHOS::FuzzUtil::U32_AT_SIZE);
    OHOS::DoSomethingInterestingWithMyAPI(buf, fdp);
    free(buf);
    return 0;
}
