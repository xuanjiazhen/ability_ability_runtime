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

#include "business_manager.h"

#include <cinttypes>
#include <chrono>
#include <functional>
#include <string>
#include <thread>

#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "app_mgr_service_inner.h"
#include "app_running_manager.h"
#include "app_running_record.h"
#include "appexecfwk_errors.h"
#include "appfreeze_manager.h"
#include "event_report.h"
#include "freeze_util.h"
#include "hilog_tag_wrapper.h"
#include "hisysevent.h"
#include "hisysevent_report.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "time_util.h"
#ifdef APP_NO_RESPONSE_DIALOG
#include "modal_system_app_freeze_uiextension.h"
#endif
#include "xcollie/process_kill_reason.h"
#ifdef APP_MGR_SERVICE_HICOLLIE_ENABLE
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
#endif

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr const int32_t DEFAULT_APPFREEZE_KILL_WAIT_TIME = 3500; // 3.5s
constexpr const int32_t BETA_APPFREEZE_KILL_WAIT_TIME = 6000; // 6s
constexpr int32_t APPRECOVERY_NOTIFYAPP_OVER_LIMIT_ID = 3041;
constexpr const char *REASON_APPRECOVERY_NOTIFYAPP_OVER_LIMIT = "AppRecoveryNotifyAppOverLimit";
constexpr int32_t PROCESS_KILL_PARAM = 25; // PROCESS_KILL params

constexpr const char *EVENT_KEY_PID = "PID";
constexpr const char *EVENT_KEY_UID = "UID";
constexpr const char *EVENT_KEY_PROCESS_NAME = "PROCESS_NAME";
constexpr const char *EVENT_KEY_BUNDLE_NAME = "BUNDLE_NAME";
constexpr const char *EVENT_KEY_MESSAGE = "MSG";
constexpr const char *EVENT_KEY_REASON = "REASON";
constexpr const char *EVENT_KEY_FOREGROUND = "FOREGROUND";
constexpr const char *EVENT_KEY_APP_RUNNING_UNIQUE_ID = "APP_RUNNING_UNIQUE_ID";
constexpr const char *EVENT_KEY_VERSIONCODE = "VERSIONCODE";
constexpr const char *EVENT_KEY_VERSIONNAME = "VERSIONNAME";
constexpr const char *EVENT_KEY_INNER_MSG = "INNER_MSG";
constexpr const char *EVENT_KEY_PROCESS_KILL_ID = "PROCESS_KILL_ID";
constexpr const char *EVENT_KEY_ADJ = "ADJ";
constexpr const char *EVENT_KEY_TIMESTAMP = "TIMESTAMP";
constexpr const char *EVENT_KEY_RSS = "RSS";
constexpr const char *EVENT_KEY_PSS = "PSS";
constexpr const char *EVENT_KEY_PROCESS_STATE = "PROCESS_STATE";
constexpr const char *EVENT_KEY_QUICK_PARAM_FIRST = "EVENT_PARAM_FIRST";
constexpr const char *EVENT_KEY_QUICK_PARAM_SECOND = "EVENT_PARAM_SECOND";
constexpr const char *EVENT_KEY_QUICK_PARAM_THIRD = "EVENT_PARAM_THIRD";
constexpr const char *EVENT_KEY_QUICK_PARAM_FOURTH = "EVENT_PARAM_FOURTH";
constexpr const char *EVENT_KEY_QUICK_PARAM_FIFTH = "EVENT_PARAM_FIFTH";
constexpr const char *EVENT_KEY_QUICK_PARAM_SIXTH = "EVENT_PARAM_SIXTH";
constexpr const char *EVENT_KEY_QUICK_PARAM_SEVENTH = "EVENT_PARAM_SEVENTH";

struct AppFaultCtx {
    int32_t pid;
    int32_t uid;
    int32_t recordId;
    std::string bundleName;
    std::string processName;
    std::string eventName;
};

#define CHECK_POINTER_AND_RETURN_LOG(object, log)      \
    if (!object) {                                     \
        TAG_LOGE(AAFwkTag::APPMGR, "%{public}s", log); \
        return;                                        \
    }

#define CHECK_POINTER_AND_RETURN_VALUE(object, value) \
    if (!object) {                              \
        TAG_LOGE(AAFwkTag::APPMGR, "nullptr");  \
        return value;                           \
    }
}  // namespace

using namespace OHOS::AbilityRuntime;

BusinessManager &BusinessManager::GetInstance()
{
    static BusinessManager instance;
    return instance;
}

int32_t BusinessManager::NotifyAppFault(const std::shared_ptr<AppMgrServiceInner> &inner, const FaultData &faultData)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    auto appRecord = inner->GetAppRunningRecordByPid(pid);
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appRecord, pid:%{public}d", pid);
        return ERR_INVALID_VALUE;
    }
    std::string eventName = faultData.errorObject.name;
    if (!AppExecFwk::AppfreezeManager::GetInstance()->IsValidFreezeTypeName(eventName)) {
        TAG_LOGE(AAFwkTag::APPDFR, "called failed, invalid freeze name: %{public}s", eventName.c_str());
        return ERR_INVALID_VALUE;
    }
    if (appRecord->GetState() == ApplicationState::APP_STATE_TERMINATED ||
        appRecord->GetState() == ApplicationState::APP_STATE_END) {
        TAG_LOGE(AAFwkTag::APPMGR, "appfreeze detect end");
        return ERR_OK;
    }
    int32_t recordId = appRecord->GetRecordId();
    std::string bundleName = appRecord->GetBundleName();
    TAG_LOGW(AAFwkTag::APPDFR, "called, eventName:%{public}s, pid:%{public}d, bundleName:%{public}s, "
        "currentTime:%{public}s", eventName.c_str(), pid, bundleName.c_str(),
        AbilityRuntime::TimeUtil::DefaultCurrentTimeStr().c_str());
    if (IsAsanEnabled(appRecord)) {
        TAG_LOGI(AAFwkTag::APPMGR, "no freeze detect cause asan enable, bundleName:%{public}s, pid:%{public}d",
            bundleName.c_str(), pid);
        return ERR_OK;
    }
    if (AppExecFwk::AppfreezeManager::GetInstance()->IsSkipDetect(pid, uid, bundleName,
        eventName) || AppExecFwk::AppfreezeManager::GetInstance()->IsFreezeExcludedPid(pid)) {
        return ERR_OK;
    }

    FaultData &nonConstFaultData = const_cast<FaultData &>(faultData);
    nonConstFaultData.isInForeground = UpdateForeground(appRecord, eventName);
    std::string processName = appRecord->GetProcessName();
    bool isExit = AppExecFwk::AppfreezeManager::GetInstance()->CheckProcessExit(processName,
        eventName, nonConstFaultData.isInForeground);
    if (faultData.faultType == FaultDataType::APP_FREEZE) {
        if (CheckAppFault(inner, appRecord, faultData)) {
            return ERR_OK;
        }

        if (isExit && faultData.waitSaveState) {
            AppRecoveryNotifyApp(inner, pid, bundleName, FaultDataType::APP_FREEZE,
                "recoveryTimeout", recordId);
        }
    }

    std::string key = std::to_string(pid) + "_" + std::to_string(uid) + "_" + bundleName;
    if (AppExecFwk::AppfreezeManager::GetInstance()->CheckAppfreezeHappend(key, eventName)) {
        return ERR_OK;
    }

    if (inner->appRunningManager_ &&
        AppExecFwk::AppfreezeManager::GetInstance()->CheckNeedRecordAppRunningUnquieId(eventName)) {
        std::string appRunningUniqueId;
        int32_t ret = inner->appRunningManager_->GetAppRunningUniqueIdByPid(pid, appRunningUniqueId);
        TAG_LOGI(AAFwkTag::APPDFR, "ret=%{public}d, appRunningUniqueId=%{public}s", ret,
            appRunningUniqueId.c_str());
        nonConstFaultData.appRunningUniqueId = appRunningUniqueId;
    }

    if (SubmitDfxFaultTask(inner, nonConstFaultData, bundleName, appRecord, pid) != ERR_OK) {
        return ERR_INVALID_VALUE;
    }

#ifdef APP_NO_RESPONSE_DIALOG
    auto killFaultApp = [inner, pid, bundleName, faultData, recordId]() {
        KillFaultApp(inner, pid, bundleName, faultData, false, recordId);
    };
    ModalSystemAppFreezeUIExtension::GetInstance().ProcessAppFreeze(appRecord->GetFocusFlag(), faultData,
        std::to_string(pid), bundleName, killFaultApp);
#else
    if (isExit) {
        KillFaultApp(inner, pid, bundleName, faultData, false, recordId);
    } else {
        TAG_LOGW(AAFwkTag::APPMGR, "Process does not exit, pid:%{public}d, isExit:%{public}d, "
            "eventName:%{public}s, processName:%{public}s", pid, isExit, eventName.c_str(),
            processName.c_str());
    }
#endif

    return ERR_OK;
}

FaultData BusinessManager::ConvertDataTypes(const std::shared_ptr<AppMgrServiceInner> &inner,
    const AppFaultDataBySA &faultData)
{
    FaultData newfaultData;
    newfaultData.faultType = faultData.faultType;
    newfaultData.errorObject.message += faultData.errorObject.message;
    newfaultData.errorObject.name = faultData.errorObject.name;
    newfaultData.errorObject.stack = faultData.errorObject.stack;
    newfaultData.timeoutMarkers = faultData.timeoutMarkers;
    newfaultData.waitSaveState = faultData.waitSaveState;
    newfaultData.notifyApp = faultData.notifyApp;
    newfaultData.forceExit = faultData.forceExit;
    newfaultData.token = faultData.token;
    newfaultData.state = faultData.state;
    newfaultData.eventId = faultData.eventId;
    newfaultData.pid = faultData.pid;
    newfaultData.schedTime = faultData.schedTime;
    newfaultData.detectTime = faultData.detectTime;
    newfaultData.needKillProcess = faultData.needKillProcess;
    newfaultData.appfreezeInfo = faultData.appfreezeInfo;
    newfaultData.procStatm = faultData.procStatm;
    newfaultData.isInForeground = faultData.isInForeground;
    newfaultData.isEnableMainThreadSample = faultData.isEnableMainThreadSample;
    newfaultData.reportLifecycleToFreeze = faultData.reportLifecycleToFreeze;
    newfaultData.applicationHeapInfo = faultData.applicationHeapInfo;
    newfaultData.applicationGCInfo = faultData.applicationGCInfo;
    newfaultData.applicationIOInfo = faultData.applicationIOInfo;
    newfaultData.isBlockInGc = faultData.isBlockInGc;
    newfaultData.processLifeTime = faultData.processLifeTime;
    newfaultData.leakObject.leakType = faultData.leakObject.leakType;
    newfaultData.leakObject.leakSize = faultData.leakObject.leakSize;
    newfaultData.leakObject.detailInfo.arktsSize = faultData.leakObject.detailInfo.arktsSize;
    newfaultData.leakObject.detailInfo.nativeSize = faultData.leakObject.detailInfo.nativeSize;
    newfaultData.leakObject.detailInfo.ionSize = faultData.leakObject.detailInfo.ionSize;
    newfaultData.leakObject.detailInfo.gpuSize = faultData.leakObject.detailInfo.gpuSize;
    newfaultData.leakObject.detailInfo.ashmemSize = faultData.leakObject.detailInfo.ashmemSize;
    newfaultData.leakObject.detailInfo.otherSize = faultData.leakObject.detailInfo.otherSize;
    newfaultData.atLeakType = faultData.atLeakType;
    newfaultData.gpuHookSize = faultData.gpuHookSize;
    if (inner->appRunningManager_) {
        std::string appRunningUniqueId;
        int32_t ret = inner->appRunningManager_->GetAppRunningUniqueIdByPid(faultData.pid, appRunningUniqueId);
        TAG_LOGI(AAFwkTag::APPDFR, "ret=%{public}d, appRunningUniqueId=%{public}s", ret,
            appRunningUniqueId.c_str());
        newfaultData.appRunningUniqueId = appRunningUniqueId;
    }
    return newfaultData;
}

void BusinessManager::TimeoutNotifyApp(const std::shared_ptr<AppMgrServiceInner> &inner, int32_t pid,
    int32_t uid, const std::string &bundleName, const std::string &processName, const FaultData &faultData,
    int32_t recordId)
{
    bool isNeedExit = (faultData.errorObject.name == AppFreezeType::APP_INPUT_BLOCK) ||
        (faultData.errorObject.name == AppFreezeType::LIFECYCLE_TIMEOUT);
#ifdef APP_NO_RESPONSE_DIALOG
    auto killFaultApp = [inner, pid, bundleName, faultData, isNeedExit, recordId]() {
        KillFaultApp(inner, pid, bundleName, faultData, isNeedExit, recordId);
    };
    ModalSystemAppFreezeUIExtension::GetInstance().ProcessAppFreeze(true, faultData, std::to_string(pid),
        bundleName, killFaultApp);
#else
    KillFaultApp(inner, pid, bundleName, faultData, isNeedExit, recordId);
#endif
    if (faultData.faultType == FaultDataType::APP_FREEZE) {
        bool isFrozen = AbilityRuntime::FreezeUtil::GetInstance().IsFrozen(pid);
        AppfreezeManager::AppInfo info = {
            .isFrozen = isFrozen,
            .pid = pid,
            .uid = uid,
            .bundleName = bundleName,
            .processName = processName,
        };
        AppExecFwk::AppfreezeManager::GetInstance()->AppfreezeHandleWithStack(faultData, info);
    }
}

int32_t BusinessManager::TransformedNotifyAppFault(const std::shared_ptr<AppMgrServiceInner> &inner,
    const AppFaultDataBySA &faultData)
{
    auto record = inner->GetAppRunningRecordByPid(faultData.pid);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "no such AppRunningRecord");
        return ERR_INVALID_VALUE;
    }
    FaultData transformedFaultData = ConvertDataTypes(inner, faultData);
    AppFaultCtx ctx = {faultData.pid, record->GetUid(), record->GetRecordId(), record->GetBundleName(),
        record->GetProcessName(), transformedFaultData.errorObject.name};
    transformedFaultData.isInForeground = UpdateForeground(record, ctx.eventName);
    if (IsAsanEnabled(record)) {
        TAG_LOGI(AAFwkTag::APPMGR, "no freeze detect cause asan enable, bundleName:%{public}s, pid:%{public}d",
            ctx.bundleName.c_str(), ctx.pid);
        return ERR_OK;
    }
    if (AppExecFwk::AppfreezeManager::GetInstance()->IsSkipDetect(ctx.pid, ctx.uid, ctx.bundleName, ctx.eventName) ||
        AppExecFwk::AppfreezeManager::GetInstance()->IsFreezeExcludedPid(ctx.pid)) {
        return ERR_OK;
    }
    if (ctx.eventName == "appRecovery") {
        AppRecoveryNotifyApp(inner, ctx.pid, ctx.bundleName, faultData.faultType,
            "appRecovery", ctx.recordId);
        return ERR_OK;
    }
    if (transformedFaultData.timeoutMarkers.empty()) {
        transformedFaultData.timeoutMarkers = "notifyFault:" + ctx.eventName +
            std::to_string(ctx.pid) + "-" + std::to_string(inner->SystemTimeMillisecond());
    }
    const int64_t timeout = 3000; // ipc timeout 3000ms
    if (faultData.faultType == FaultDataType::APP_FREEZE) {
        if (!AppExecFwk::AppfreezeManager::GetInstance()->IsHandleAppfreeze(ctx.bundleName) || record->IsDebugging()) {
            return ERR_OK;
        }
        auto timeoutNotifyApp = [inner, ctx, transformedFaultData]() {
            std::string key = std::to_string(ctx.pid) + "_" + std::to_string(ctx.uid) + "_" + ctx.bundleName;
            std::string message = transformedFaultData.errorObject.message;
            if (AppExecFwk::AppfreezeManager::GetInstance()->CheckPreloadUIExtension(message, ctx.bundleName, ctx.pid,
                ctx.eventName) || AppExecFwk::AppfreezeManager::GetInstance()->CheckAppfreezeHappend(key,
                ctx.eventName)) {
                return;
            }
            TimeoutNotifyApp(inner, ctx.pid, ctx.uid, ctx.bundleName,
                ctx.processName, transformedFaultData, ctx.recordId);
        };
        if (inner->dfxTaskHandler_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "dfxTaskHandler_ null");
            return ERR_NO_INIT;
        }
        inner->dfxTaskHandler_->SubmitTask(timeoutNotifyApp, transformedFaultData.timeoutMarkers, timeout);
    }
    record->NotifyAppFault(transformedFaultData);
    TAG_LOGW(AAFwkTag::APPMGR, "FaultDataBySA is: name: %{public}s, faultType: %{public}s, uid: %{public}d,"
        "pid: %{public}d, bundleName: %{public}s, eventId: %{public}d, foreground: %{public}d",
        ctx.eventName.c_str(), FaultTypeToString(faultData.faultType).c_str(),
        ctx.uid, ctx.pid, ctx.bundleName.c_str(), faultData.eventId, transformedFaultData.isInForeground);
    return ERR_OK;
}

int32_t BusinessManager::SubmitDfxFaultTask(const std::shared_ptr<AppMgrServiceInner> &inner,
    const FaultData &faultData, const std::string &bundleName,
    const std::shared_ptr<AppRunningRecord> &appRecord, const int32_t pid)
{
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return ERR_INVALID_VALUE;
    }
    if (AppExecFwk::AppfreezeManager::GetInstance()->CheckPreloadUIExtension(faultData.errorObject.message,
        bundleName, pid, faultData.errorObject.name)) {
        return ERR_OK;
    }
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    std::string processName = appRecord->GetProcessName();
    int exceptionId = GetExceptionTimerId(inner, faultData, bundleName, appRecord, pid, callerUid);
    auto notifyAppTask = [appRecord, pid, callerUid, bundleName, processName, faultData, exceptionId,
        innerServiceWeak = inner->weak_from_this()]() {
#ifdef APP_MGR_SERVICE_HICOLLIE_ENABLE
        HiviewDFX::XCollie::GetInstance().CancelTimer(exceptionId);
#endif
        auto innerService = innerServiceWeak.lock();
        CHECK_POINTER_AND_RETURN_LOG(innerService, "get appMgrServiceInner fail");
        ParseInfoToAppfreeze(faultData, pid, callerUid, bundleName, processName);
    };

    if (!inner->dfxTaskHandler_) {
        TAG_LOGW(AAFwkTag::APPMGR, "get dfx handler fail");
        return ERR_INVALID_VALUE;
    }
    int64_t startTime = AbilityRuntime::TimeUtil::CurrentTimeMillis();
    TAG_LOGI(AAFwkTag::APPDFR, "init cpuInfo, eventName:%{public}s, bundleName:%{public}s, "
        "pid:%{public}d", faultData.errorObject.name.c_str(), bundleName.c_str(), pid);
    bool isFrozen = AbilityRuntime::FreezeUtil::GetInstance().IsFrozen(pid);
    AppfreezeManager::AppInfo info = {
        .isFrozen = isFrozen,
        .pid = pid,
        .uid = callerUid,
        .bundleName = bundleName,
        .processName = processName,
    };
    AppExecFwk::AppfreezeManager::GetInstance()->InitWarningCpuInfo(faultData, info);
    inner->dfxTaskHandler_->SubmitTask(notifyAppTask, "NotifyAppFaultTask");
    TAG_LOGW(AAFwkTag::APPDFR, "submit NotifyAppFaultTask, eventName:%{public}s, bundleName:%{public}s, "
        "endTime:%{public}s, interval:%{public}" PRId64 " ms", faultData.errorObject.name.c_str(),
        bundleName.c_str(), AbilityRuntime::TimeUtil::DefaultCurrentTimeStr().c_str(),
        AbilityRuntime::TimeUtil::CurrentTimeMillis() - startTime);

    constexpr int delayTime = 15 * 1000; // 15s
    auto task = [pid]() {
        AppExecFwk::AppfreezeManager::GetInstance()->DeleteStack(pid);
    };
    inner->dfxTaskHandler_->SubmitTaskJust(task, "DeleteStack", delayTime);

    return ERR_OK;
}

void BusinessManager::ParseInfoToAppfreeze(const FaultData &faultData, int32_t pid, int32_t uid,
    const std::string &bundleName, const std::string &processName, const bool isOccurException)
{
    if (faultData.faultType == FaultDataType::APP_FREEZE) {
        bool isFrozen = AbilityRuntime::FreezeUtil::GetInstance().IsFrozen(pid);
        AppfreezeManager::AppInfo info = {
            .isOccurException = isOccurException,
            .isFrozen = isFrozen,
            .pid = pid,
            .uid = uid,
            .bundleName = bundleName,
            .processName = processName,
        };
        AppExecFwk::AppfreezeManager::GetInstance()->AppfreezeHandleWithStack(faultData, info);
    }

    TAG_LOGW(AAFwkTag::APPMGR,
        "name: %{public}s, faultType: %{public}d, uid: %{public}d, pid: %{public}d, bundleName: %{public}s,"
        " processName: %{public}s, faultData.forceExit:%{public}d, faultData.waitSaveState:%{public}d,"
        " isOccurException:%{public}d, stuckTimeout:%{public}u",
        faultData.errorObject.name.c_str(), faultData.faultType, uid, pid, bundleName.c_str(),
        processName.c_str(), faultData.forceExit, faultData.waitSaveState, isOccurException, faultData.stuckTimeout);
}

int BusinessManager::GetExceptionTimerId(const std::shared_ptr<AppMgrServiceInner> &inner,
    const FaultData &faultData, const std::string &bundleName,
    const std::shared_ptr<AppRunningRecord> &appRecord, const int32_t pid, const int32_t callerUid)
{
    auto exceptionCallback = [faultData, bundleName, appRecord, pid, callerUid,
        innerServiceWeak = inner->weak_from_this()](void *) {
        auto threadTask = [faultData, bundleName, appRecord, pid, callerUid, innerServiceWeak]() {
            auto innerService = innerServiceWeak.lock();
            CHECK_POINTER_AND_RETURN_LOG(innerService, "get appMgrServiceInner fail");
            if (CheckAppFault(innerService, appRecord, faultData)) {
                TAG_LOGI(AAFwkTag::APPMGR, "Ffrt Exception current dfx task is working.");
                return;
            }
            bool isOccurException = true;
            ParseInfoToAppfreeze(faultData, pid, callerUid, bundleName,
                appRecord->GetProcessName(), isOccurException);
            if (faultData.errorObject.name != AppFreezeType::THREAD_BLOCK_3S &&
                faultData.errorObject.name != AppFreezeType::LIFECYCLE_HALF_TIMEOUT &&
                faultData.errorObject.name != AppFreezeType::LIFECYCLE_HALF_TIMEOUT_WARNING) {
                TAG_LOGI(AAFwkTag::APPMGR, "Ffrt Exception faultData: %{public}s,pid: %{public}d "
                    "will exit because"" %{public}s", bundleName.c_str(), pid,
                    FaultTypeToString(faultData.faultType).c_str());
#ifdef APP_MGR_KILL_REASON_TAG
                RecordAppfreezeKillReason(pid, faultData);
#endif
                return;
            }
        };
        std::thread dfxThread(threadTask);
        if (dfxThread.joinable()) {
            dfxThread.join();
        }
    };
    constexpr uint32_t timeout = 30; // 30s
    int exceptionId = -1;
#ifdef APP_MGR_SERVICE_HICOLLIE_ENABLE
    exceptionId = HiviewDFX::XCollie::GetInstance().SetTimer("DfxFault::Exception", timeout,
        exceptionCallback, nullptr, HiviewDFX::XCOLLIE_FLAG_NOOP);
#endif
    return exceptionId;
}

bool BusinessManager::UpdateForeground(const std::shared_ptr<AppRunningRecord> &appRecord,
    const std::string &eventName)
{
    bool isStateForeground = appRecord->GetState() == ApplicationState::APP_STATE_FOREGROUND ||
        appRecord->GetState() == ApplicationState::APP_STATE_FOCUS;
    if (eventName == AppFreezeType::THREAD_BLOCK_3S || eventName == AppFreezeType::THREAD_BLOCK_6S ||
        eventName == AppFreezeType::BUSSINESS_THREAD_BLOCK_3S ||
        eventName == AppFreezeType::BUSSINESS_THREAD_BLOCK_6S) {
        return isStateForeground;
    }
    bool isTransitioningToForeground = appRecord->GetApplicationPendingState() ==
        ApplicationPendingState::FOREGROUNDING;
    return (isStateForeground || isTransitioningToForeground);
}

bool BusinessManager::IsAsanEnabled(const std::shared_ptr<AppRunningRecord> &record)
{
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "no such AppRunningRecord");
        return false;
    }
    const auto &applicationInfo = record->GetApplicationInfo();
    if (applicationInfo == nullptr) {
        return false;
    }
    return applicationInfo->asanEnabled || applicationInfo->hwasanEnabled || applicationInfo->tsanEnabled;
}

bool BusinessManager::CheckAppFault(const std::shared_ptr<AppMgrServiceInner> &inner,
    const std::shared_ptr<AppRunningRecord> &appRecord, const FaultData &faultData)
{
    if (faultData.timeoutMarkers != "" && !inner->dfxTaskHandler_->CancelTask(faultData.timeoutMarkers)) {
        return true;
    }

    if (appRecord->IsDebugging()) {
        return true;
    }
    return false;
}

#ifdef APP_MGR_KILL_REASON_TAG
void BusinessManager::RecordAppfreezeKillReason(int32_t pid, const FaultData &faultData)
{
    bool isInBackGround = AppExecFwk::AppfreezeManager::GetInstance()->CheckInBackGround(faultData);
    std::string reason = isInBackGround ? AppFreezeType::BG_FREEZE_WARNING : faultData.errorObject.name;
    AAFwk::ExitReasonCompability exitReason = { REASON_APP_FREEZE, "Reason:" + reason };
    exitReason.killId =
        AppExecFwk::AppfreezeManager::GetInstance()->GetFreezeExitReason(faultData.errorObject.name);
    exitReason.killMsg = reason;
    exitReason.innerMsg = reason;
    AbilityManagerClient::GetInstance()->KillAppWithReason(pid, exitReason);
}

void BusinessManager::RecordAppWithReasonByUserId(int32_t userId, int32_t killId)
{
    AAFwk::ExitReasonCompability exitReasonCompability(killId);
    IN_PROCESS_CALL(AbilityManagerClient::GetInstance()->RecordAppWithReasonByUserId(userId, exitReasonCompability));
}
#endif

int32_t BusinessManager::KillFaultApp(const std::shared_ptr<AppMgrServiceInner> &inner, int32_t pid,
    const std::string &bundleName, const FaultData &faultData, bool isNeedExit, int32_t recordId)
{
    if (isNeedExit || (faultData.forceExit && !faultData.waitSaveState)) {
        auto killAppTask = [pid, bundleName, faultData, isNeedExit,
            innerServiceWeak = inner->weak_from_this(), recordId]() {
            auto innerService = innerServiceWeak.lock();
            CHECK_POINTER_AND_RETURN_LOG(innerService, "get appMgrServiceInner fail");
            TAG_LOGW(AAFwkTag::APPMGR, "faultData: %{public}s,pid: %{public}d will exit because %{public}s",
                bundleName.c_str(), pid, FaultTypeToString(faultData.faultType).c_str());
            uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            AppExecFwk::AppfreezeManager::GetInstance()->RegisterAppKillTime(pid, now);
#ifdef APP_MGR_KILL_REASON_TAG
            RecordAppfreezeKillReason(pid, faultData);
#endif
            return;
        };
        int32_t waitTime = AppExecFwk::AppfreezeManager::GetInstance()->IsBetaVersion() ?
            BETA_APPFREEZE_KILL_WAIT_TIME : DEFAULT_APPFREEZE_KILL_WAIT_TIME;
        // wait 3.5s or 6s before kill application
        auto taskHandler = inner->GetTaskHandler();
        CHECK_POINTER_AND_RETURN_VALUE(taskHandler, ERR_NO_INIT);
        taskHandler->SubmitTaskJust(killAppTask, "killAppTask", waitTime);
    }

    return ERR_OK;
}

void BusinessManager::AppRecoveryNotifyApp(const std::shared_ptr<AppMgrServiceInner> &inner, int32_t pid,
    const std::string &bundleName, FaultDataType faultType, const std::string &markers, int32_t recordId)
{
    if (faultType != FaultDataType::APP_FREEZE) {
        TAG_LOGI(AAFwkTag::APPMGR,
            "kill appRecovery NotifyApp bundleName: %{public}s, faultType: "
            "%{public}d, pid: %{public}d", bundleName.c_str(), faultType, pid);
        RecordAppRecoveryNotifyAppReason(inner, pid);
        inner->KillProcessByPid(pid, "AppRecoveryNotifyApp");
        return;
    }

    std::string timeOutName = "waitSaveTask" + std::to_string(pid) + bundleName;
    if (markers == "appRecovery") {
        TAG_LOGI(AAFwkTag::APPMGR, "waitSaveTask finish, but not kill process "
            "immediately, wait for dump stack util 2s timeout");
        return;
    }

    if (markers != "recoveryTimeout") {
        return;
    }
    auto waitSaveTask = [pid, bundleName, innerServiceWeak = inner->weak_from_this(), recordId]() {
        auto innerService = innerServiceWeak.lock();
        CHECK_POINTER_AND_RETURN_LOG(innerService, "get appMgrServiceInner fail");
        auto appRecord = innerService->GetAppRunningRecordByPid(pid);
        if (appRecord == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "no appRecord");
            return;
        }
        std::string name = appRecord->GetBundleName();
        if (bundleName == name) {
            TAG_LOGI(AAFwkTag::APPMGR,
                "waitSaveTask timeout %{public}s,pid: %{public}d will exit",
                bundleName.c_str(), pid);
            RecordAppRecoveryNotifyAppReason(innerService, pid);
            innerService->KillProcessByPid(pid, "AppRecoveryNotifyApp", false, recordId);
        }
    };
    constexpr int32_t timeOut = 2000;
    auto taskHandler = inner->GetTaskHandler();
    CHECK_POINTER_AND_RETURN_LOG(taskHandler, "no taskHandler_");
    taskHandler->SubmitTaskJust(waitSaveTask, timeOutName, timeOut);
}

void BusinessManager::RecordAppRecoveryNotifyAppReason(const std::shared_ptr<AppMgrServiceInner> &inner,
    int32_t pid)
{
    auto appRecord = inner->GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appRecord for pid:%{public}d", pid);
        return;
    }
    appRecord->SetKillId(APPRECOVERY_NOTIFYAPP_OVER_LIMIT_ID);
    appRecord->SetKillMsg(REASON_APPRECOVERY_NOTIFYAPP_OVER_LIMIT);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "AppRecoveryNotifyApp Reason pid=%{public}d, killId=%{public}d, innerMsg=%{public}s",
        pid, APPRECOVERY_NOTIFYAPP_OVER_LIMIT_ID, REASON_APPRECOVERY_NOTIFYAPP_OVER_LIMIT);
}

std::string BusinessManager::FaultTypeToString(AppExecFwk::FaultDataType type)
{
    std::string typeStr = "UNKNOWN";
    switch (type) {
        case AppExecFwk::FaultDataType::CPP_CRASH:
            typeStr = "CPP_CRASH";
            break;
        case AppExecFwk::FaultDataType::JS_ERROR:
            typeStr = "JS_ERROR";
            break;
        case AppExecFwk::FaultDataType::APP_FREEZE:
            typeStr = "APP_FREEZE";
            break;
        case AppExecFwk::FaultDataType::PERFORMANCE_CONTROL:
            typeStr = "PERFORMANCE_CONTROL";
            break;
        case AppExecFwk::FaultDataType::RESOURCE_CONTROL:
            typeStr = "RESOURCE_CONTROL";
            break;
        default:
            break;
    }
    return typeStr;
}

bool BusinessManager::SetAppFreezeFilter(const std::shared_ptr<AppMgrServiceInner> &inner, int32_t pid)
{
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    auto callerRecord = inner->GetAppRunningRecordByPid(pid);
    if (callerRecord == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "callerRecord null");
        return false;
    }
    std::string bundleName = callerRecord->GetBundleName();
    if (callingPid == pid && AppExecFwk::AppfreezeManager::GetInstance()->IsValidFreezeFilter(pid, bundleName)) {
        bool result = AppExecFwk::AppfreezeManager::GetInstance()->CancelAppFreezeDetect(pid, bundleName);
        if (result) {
            auto resetAppfreezeTask = [pid, bundleName]() {
                AppExecFwk::AppfreezeManager::GetInstance()->ResetAppfreezeState(pid, bundleName);
            };
            constexpr int32_t waitTime = 120000; // wait 2min
            CHECK_POINTER_AND_RETURN_VALUE(inner->dfxTaskHandler_, false);
            inner->dfxTaskHandler_->SubmitTaskJust(resetAppfreezeTask, "resetAppfreezeTask", waitTime);
        }
        TAG_LOGW(AAFwkTag::APPDFR, "SetAppFreezeFilter: %{public}d, bundleName=%{public}s, pid:%{public}d, ",
            result, bundleName.c_str(), pid);
        return result;
    }
    TAG_LOGE(AAFwkTag::APPDFR, "SetAppFreezeFilter: failed, pid %{public}d calling pid %{public}d "
        "bundleName %{public}s", pid, callingPid, bundleName.c_str());
    return false;
}

void BusinessManager::InsertKillThread(int32_t state, int32_t pid, int32_t uid, const std::string &bundleName)
{
    AppExecFwk::AppfreezeManager::GetInstance()->InsertKillThread(state, pid, uid, bundleName);
}

void BusinessManager::ReportAppFreezeSysEvents(int32_t pid, const std::string &bundleName)
{
    AppExecFwk::AppfreezeManager::GetInstance()->ReportAppFreezeSysEvents(pid, bundleName);
}

void BusinessManager::RemoveDeathProcess(const std::string &bundleName)
{
    AppExecFwk::AppfreezeManager::GetInstance()->RemoveDeathProcess(bundleName);
}

void BusinessManager::SendProcessKillEvent(std::shared_ptr<AppRunningRecord> appRecord)
{
    CHECK_POINTER_AND_RETURN_LOG(appRecord, "no appRecord");

    auto appInfo = appRecord->GetApplicationInfo();
    if (appInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appInfo");
        return;
    }
    int32_t pid = appRecord->GetPid();
    bool foreground = appRecord->GetState() == ApplicationState::APP_STATE_FOREGROUND ||
        appRecord->GetState() == ApplicationState::APP_STATE_FOCUS;
    AppfreezeManager::ProcessKillInfo killInfo = AppExecFwk::AppfreezeManager::GetInstance()->GetProcessKillReason(
        appRecord->GetKillId(), pid, appRecord->GetKillMsg(), foreground);
    std::string appRunningUniqueId = std::to_string(appRecord->GetAppRunningUniqueId());
    uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    AAFwk::EventInfo eventInfo;
    eventInfo.bundleName = appInfo->name;
    eventInfo.versionName = appInfo->versionName;
    eventInfo.versionCode = appInfo->versionCode;
    eventInfo.pid = appRecord->GetPid();
    eventInfo.processName = appRecord->GetProcessName();
    AAFwk::EventReport::SendAppEvent(AAFwk::EventName::APP_TERMINATE, HISYSEVENT_BEHAVIOR, eventInfo);
    auto hisyseventReport = std::make_shared<AAFwk::HisyseventReport>(PROCESS_KILL_PARAM);
    hisyseventReport->InsertParam(EVENT_KEY_PID, pid);
    hisyseventReport->InsertParam(EVENT_KEY_UID, appRecord->GetUid());
    hisyseventReport->InsertParam(EVENT_KEY_PROCESS_NAME, eventInfo.processName);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, appRecord->GetBundleName());
    hisyseventReport->InsertParam(EVENT_KEY_MESSAGE, killInfo.killMsg);
    hisyseventReport->InsertParam(EVENT_KEY_REASON, killInfo.killReason);
    hisyseventReport->InsertParam(EVENT_KEY_FOREGROUND, killInfo.foreground);
    hisyseventReport->InsertParam(EVENT_KEY_APP_RUNNING_UNIQUE_ID, appRunningUniqueId);
    hisyseventReport->InsertParam(EVENT_KEY_VERSIONCODE, std::to_string(appInfo->versionCode));
    hisyseventReport->InsertParam(EVENT_KEY_VERSIONNAME, appInfo->versionName);
    hisyseventReport->InsertParam(EVENT_KEY_INNER_MSG, appRecord->GetInnerMsg());
    hisyseventReport->InsertParam(EVENT_KEY_PROCESS_KILL_ID, killInfo.killId);
    hisyseventReport->InsertParam(EVENT_KEY_ADJ, killInfo.adj);
    hisyseventReport->InsertParam(EVENT_KEY_TIMESTAMP, timestamp);
    hisyseventReport->InsertParam(EVENT_KEY_RSS, appRecord->GetRssValue());
    hisyseventReport->InsertParam(EVENT_KEY_PSS, appRecord->GetPssValue());
    hisyseventReport->InsertParam(EVENT_KEY_PROCESS_STATE, static_cast<int32_t>(appRecord->GetState()));
    hisyseventReport->InsertParam(EVENT_KEY_QUICK_PARAM_FIRST, killInfo.eventParamFirst);
    hisyseventReport->InsertParam(EVENT_KEY_QUICK_PARAM_SECOND, killInfo.eventParamSecond);
    hisyseventReport->InsertParam(EVENT_KEY_QUICK_PARAM_THIRD, killInfo.eventParamThird);
    hisyseventReport->InsertParam(EVENT_KEY_QUICK_PARAM_FOURTH, killInfo.eventParamFourth);
    hisyseventReport->InsertParam(EVENT_KEY_QUICK_PARAM_FIFTH, killInfo.eventParamFifth);
    hisyseventReport->InsertParam(EVENT_KEY_QUICK_PARAM_SIXTH, killInfo.eventParamSixth);
    hisyseventReport->InsertParam(EVENT_KEY_QUICK_PARAM_SEVENTH, killInfo.eventParamSeventh);
    int result = hisyseventReport->Report("FRAMEWORK", "PROCESS_KILL", HISYSEVENT_FAULT);
    TAG_LOGW(AAFwkTag::APPMGR, "hisysevent write result=%{public}d, send event [FRAMEWORK,PROCESS_KILL], pid="
        "%{public}d, processName=%{public}s, msg=%{public}s, reason=%{public}s, FOREGROUND=%{public}d,"
        " appRunningUniqueId=%{public}s, killId=%{public}d", result, pid, eventInfo.processName.c_str(),
        killInfo.killMsg.c_str(), killInfo.killReason.c_str(), foreground, appRunningUniqueId.c_str(),
        killInfo.killId);
}
}  // namespace AppExecFwk
}  // namespace OHOS
