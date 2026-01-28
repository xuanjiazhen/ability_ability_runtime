/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "killing_process_manager.h"

#include <csignal>
#include <mutex>
#include <queue>

#include "ability_manager_errors.h"
#include "appfreeze_manager.h"
#include "app_mgr_event.h"
#include "app_state_observer_manager.h"
#include "app_utils.h"
#include "cache_process_manager.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "exit_resident_process_manager.h"
#include "hilog_tag_wrapper.h"
#include "hisysevent.h"
#include "hitrace_meter.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "process_util.h"
#include "record_cost_time_util.h"
#include "task_handler_wrap.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t CLEAR_CALLER_KEY_DELAY_TIME = 5 * 1000; // 5s
// NANOSECONDS mean 10^9 nano second
constexpr int64_t NANOSECONDS = 1000000000;
// MICROSECONDS mean 10^6 milli second
constexpr int64_t MICROSECONDS = 1000000;
constexpr const char* SYSTEM_BASIC = "system_basic";
constexpr const char* SYSTEM_CORE = "system_core";
// Kill process timeout setting
constexpr int KILL_PROCESS_TIMEOUT_MICRO_SECONDS = 1000;
// Kill process delay time setting
constexpr int KILL_PROCESS_DELAYTIME_MICRO_SECONDS = 200;
constexpr const char* KILL_PROCESS_REASON_PREFIX = "Kill Reason:";
constexpr int32_t EXIT_REASON_UNKNOWN = 0;
const int32_t SIGNAL_KILL = 9;
constexpr const char* PROC_SELF_TASK_PATH = "/proc/self/task/";
constexpr int PROCESS_RESTART_MARGIN_MICRO_SECONDS = 2000;
constexpr int32_t DEFAULT_INVAL_VALUE = -1;
constexpr int32_t BASE_USER_RANGE = 200000;
// kill process for logout user time out setting
constexpr int LOGOUT_USER_TIMEOUT_MILLISION_SECONDS = 5000;
constexpr const char* KILL_SUB_PROCESS_REASON_PREFIX = "Kill SubProcess Reason:";
constexpr const char* EVENT_KEY_PID = "PID";
constexpr const char* EVENT_KEY_PROCESS_NAME = "PROCESS_NAME";
constexpr const char* EVENT_KEY_MESSAGE = "MSG";
const std::string TOKEN_ID = "TOKEN_ID";
constexpr const char* TARGET_UID_KEY = "ohos.aafwk.param.targetUid";
constexpr const char* EVENT_KEY_REASON = "REASON";
constexpr const char* EVENT_KEY_FOREGROUND = "FOREGROUND";
}
KillingProcessManager& KillingProcessManager::GetInstance()
{
    static KillingProcessManager instance;
    return instance;
}

void KillingProcessManager::Init(std::shared_ptr<AppRunningManager> appRunningManager,
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler,
    std::shared_ptr<RemoteClientManager> remoteClientManager)
{
    appRunningManager_ = appRunningManager;
    taskHandler_ = taskHandler;
    remoteClientManager_ = remoteClientManager;
}

bool KillingProcessManager::IsCallerKilling(const std::string& callerKey) const
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    return killingCallerKeySet_.find(callerKey) != killingCallerKeySet_.end();
}

void KillingProcessManager::AddKillingCallerKey(const std::string& callerKey)
{
    auto taskHandler = AAFwk::TaskHandlerWrap::GetFfrtHandler();
    if (taskHandler == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "handler is null");
        return;
    }
    if (callerKey.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid callerKey");
        return;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "add caller:%{public}s", callerKey.c_str());
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        auto ret = killingCallerKeySet_.insert(callerKey);
        if (!ret.second) {
            TAG_LOGI(AAFwkTag::APPMGR, "already inserted");
            return;
        }
    }
    auto task = [callerKey] () {
        KillingProcessManager::GetInstance().RemoveKillingCallerKey(callerKey);
    };
    taskHandler->SubmitTaskJust(task, "clearCallerKey", CLEAR_CALLER_KEY_DELAY_TIME);
}

void KillingProcessManager::RemoveKillingCallerKey(const std::string& callerKey)
{
    if (callerKey.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid callerKey");
        return;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "remove caller:%{public}s", callerKey.c_str());
    std::lock_guard<ffrt::mutex> lock(mutex_);
    killingCallerKeySet_.erase(callerKey);
}

int32_t KillingProcessManager::KillApplicationByBundleName(
    const std::string &bundleName, int32_t appIndex, bool clearPageStack, const std::string& reason)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }

    auto result = VerifyKillProcessPermission(bundleName);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verification fail");
        return result;
    }
    int64_t startTime = SystemTimeMillisecond();
    std::list<pid_t> pids;

    if (!appRunningManager_->ProcessExitByBundleNameAndAppIndex(bundleName, appIndex, pids, clearPageStack)) {
        TAG_LOGE(AAFwkTag::APPMGR, "process corresponding to the package name did not start");
        return result;
    }
    result = WaitProcessesExitAndKill(pids, startTime, reason);
    NotifyAppStatus(bundleName, appIndex, EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_RESTARTED);
    return result;
}

int32_t KillingProcessManager::VerifyKillProcessPermission(const std::string &bundleName) const
{
    TAG_LOGI(AAFwkTag::APPMGR, "callerUid:%{public}d, callerPid:%{public}d",
        IPCSkeleton::GetCallingUid(), IPCSkeleton::GetCallingPid());
    int32_t ret = VerifyKillProcessPermissionCommon();
    if (ret != ERR_PERMISSION_DENIED) {
        return ret;
    }
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }

    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_CLEAN_BACKGROUND_PROCESSES);
    if (isCallingPerm) {
        auto callerPid = IPCSkeleton::GetCallingPid();
        auto appRecord = appRunningManager_->GetAppRunningRecordByPid(callerPid);
        if (!appRecord || appRecord->GetBundleName() != bundleName) {
            TAG_LOGE(AAFwkTag::APPMGR, "permission verification fail");
            return ERR_PERMISSION_DENIED;
        }
    } else {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verification fail");
        return ERR_PERMISSION_DENIED;
    }

    return ERR_OK;
}

int32_t KillingProcessManager::VerifyKillProcessPermission(const sptr<IRemoteObject> &token) const
{
    TAG_LOGI(AAFwkTag::APPMGR, "callerUid:%{public}d, callerPid:%{public}d",
        IPCSkeleton::GetCallingUid(), IPCSkeleton::GetCallingPid());
    int32_t ret = VerifyKillProcessPermissionCommon();
    if (ret != ERR_PERMISSION_DENIED) {
        return ret;
    }
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }
    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_CLEAN_BACKGROUND_PROCESSES);
    if (isCallingPerm) {
        auto callerUid = IPCSkeleton::GetCallingUid();
        auto appRecord = appRunningManager_->GetAppRunningRecordByAbilityToken(token);
        if (!appRecord || appRecord->GetUid() != callerUid) {
            TAG_LOGE(AAFwkTag::APPMGR, "permission verification fail");
            return ERR_PERMISSION_DENIED;
        }
    } else {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verification fail");
        return ERR_PERMISSION_DENIED;
    }

    return ERR_OK;
}

int32_t KillingProcessManager::VerifyKillProcessPermissionCommon() const
{
    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_KILL_APP_PROCESSES);
    if (isCallingPerm) {
        return ERR_OK;
    }

    // VerifyAPL and ohos.permission.CLEAN_BACKGROUND_PROCESSES will be removed on API18
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    if (isSaCall || isShellCall) {
        return ERR_OK;
    }

    if (VerifyAPL()) {
        return ERR_OK;
    }

    return ERR_PERMISSION_DENIED;
}

int64_t KillingProcessManager::SystemTimeMillisecond()
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (int64_t)((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS;
}

bool KillingProcessManager::VerifyAPL() const
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return false;
    }

    auto callerPid = IPCSkeleton::GetCallingPid();
    auto appRecord = appRunningManager_->GetAppRunningRecordByPid(callerPid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "get app running record fail, callingPId: %{public}d", callerPid);
        return false;
    }

    auto applicationInfo = appRecord->GetApplicationInfo();
    if (!applicationInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "get app info fail");
        return false;
    }

    auto apl = applicationInfo->appPrivilegeLevel;
    if (apl != SYSTEM_BASIC && apl != SYSTEM_CORE) {
        TAG_LOGE(AAFwkTag::APPMGR, "caller is not system_basic or system_core");
        return false;
    }
    TAG_LOGE(AAFwkTag::APPMGR, "%{public}s without ohos.permission.KILL_APP_PROCESSES",
        applicationInfo->name.c_str());
    return true;
}

int32_t KillingProcessManager::WaitProcessesExitAndKill(std::list<pid_t> &pids, const int64_t startTime,
    const std::string& reason)
{
    int32_t result = ERR_OK;
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        auto appRecord = appRunningManager_->GetAppRunningRecordByPid(*iter);
        if (appRecord) {
            appRecord->SetKillReason(reason); 
        }
    }
    if (WaitForRemoteProcessExit(pids, startTime)) {
        TAG_LOGI(AAFwkTag::APPMGR, "remote process exited successs");
        return result;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        auto singleRet = KillProcessByPid(*iter, reason);
        if (singleRet != 0 && singleRet != AAFwk::ERR_KILL_PROCESS_NOT_EXIST) {
            TAG_LOGE(AAFwkTag::APPMGR, "killApplication fail for pid:%{public}d", *iter);
            result = singleRet;
        }
    }
    return result;
}

int32_t KillingProcessManager::WaitProcessesExitAndKill(std::list<SimpleProcessInfo> &processInfos,
    const int64_t startTime, const std::string& reason, int32_t userId, sptr<AAFwk::IUserCallback> callback)
{
    int32_t result = ERR_OK;
    ProcessUtil::UpdateProcessNameByProcFile(processInfos);
    if (WaitForRemoteProcessExit(processInfos, startTime)) {
        TAG_LOGI(AAFwkTag::APPMGR, "remote process exited successs");
        if (callback) {
            callback->OnLogoutUserDone(userId, ERR_OK);
        }
        return result;
    }
    for (auto iter = processInfos.begin(); iter != processInfos.end(); ++iter) {
        auto singleRet = KillProcessByPid((*iter).pid, reason);
        if (singleRet != 0 && singleRet != AAFwk::ERR_KILL_PROCESS_NOT_EXIST) {
            TAG_LOGE(AAFwkTag::APPMGR, "killApplication fail for pid:%{public}d", (*iter).pid);
            result = singleRet;
        }
    }
    DoAllProcessExitCallback(processInfos, userId, callback, startTime);
    return result;
}

bool KillingProcessManager::WaitForRemoteProcessExit(std::list<pid_t> &pids, const int64_t startTime)
{
    int64_t delayTime = SystemTimeMillisecond() - startTime;
    if (ProcessUtil::CheckAllProcessExit(pids)) {
        return true;
    }
    while (delayTime < KILL_PROCESS_TIMEOUT_MICRO_SECONDS) {
        usleep(KILL_PROCESS_DELAYTIME_MICRO_SECONDS);
        if (ProcessUtil::CheckAllProcessExit(pids)) {
            return true;
        }
        delayTime = SystemTimeMillisecond() - startTime;
    }
    return false;
}

bool KillingProcessManager::WaitForRemoteProcessExit(std::list<SimpleProcessInfo> &processInfos,
    const int64_t startTime)
{
    int64_t delayTime = SystemTimeMillisecond() - startTime;
    if (ProcessUtil::CheckAllProcessExit(processInfos)) {
        return true;
    }
    while (delayTime < KILL_PROCESS_TIMEOUT_MICRO_SECONDS) {
        usleep(KILL_PROCESS_DELAYTIME_MICRO_SECONDS);
        if (ProcessUtil::CheckAllProcessExit(processInfos)) {
            return true;
        }
        delayTime = SystemTimeMillisecond() - startTime;
    }
    return false;
}

int32_t KillingProcessManager::KillProcessByPid(const pid_t pid, const std::string& reason, bool isKillPrecedeStart)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!ProcessUtil::ProcessExist(pid)) {
        TAG_LOGI(AAFwkTag::APPMGR, "null killProcessByPid, pid: %{public}d", pid);
        return AAFwk::ERR_KILL_PROCESS_NOT_EXIST;
    }
    std::string killReason = KILL_PROCESS_REASON_PREFIX + reason + ",callingPid=" +
        std::to_string(IPCSkeleton::GetCallingPid());
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }
    auto appRecord = appRunningManager_->GetAppRunningRecordByPid(pid);
    if (appRecord && appRecord->GetExitReason() == EXIT_REASON_UNKNOWN) {
        appRecord->SetExitMsg(killReason);
    }
    TAG_LOGI(AAFwkTag::APPMGR, "kill reason=%{public}s, pid=%{public}d", reason.c_str(), pid);
    return KillProcessByPidInner(pid, reason, killReason, appRecord, isKillPrecedeStart);
}

int32_t KillingProcessManager::KillProcessByPidInner(const pid_t pid, const std::string& reason,
    const std::string& killReason, std::shared_ptr<AppRunningRecord> appRecord, bool isKillPrecedeStart)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    AAFwk::RecordCostTimeUtil timeRecord("KillProcessByPidInner");
    int32_t ret = -1;
    if (pid > 0) {
        if (CheckIsThreadInFoundation(pid)) {
            TAG_LOGI(AAFwkTag::APPMGR, "don't kill pid %{public}d", pid);
            return AAFwk::ERR_KILL_FOUNDATION_UID;
        }
        ret = kill(pid, SIGNAL_KILL);
        if (ret == -1 && errno == ESRCH) {
            TAG_LOGW(AAFwkTag::APPMGR, "kill process not exist, pid=%{public}d", pid);
            return AAFwk::ERR_KILL_PROCESS_NOT_EXIST;
        }
        if (reason == "OnRemoteDied") {
            TAG_LOGW(AAFwkTag::APPMGR, "application is dead, double check, pid=%{public}d", pid);
        } else {
            TAG_LOGW(AAFwkTag::APPMGR, "kill pid %{public}d, ret:%{public}d, %{public}s",
                pid, ret, killReason.c_str());
        }
    }
    if (!appRecord) {
        TAG_LOGW(AAFwkTag::APPMGR, "nullptr");
        return ret;
    }
    AppExecFwk::AppfreezeManager::GetInstance()->InsertKillThread(ret, pid, appRecord->GetUid(),
        appRecord->GetBundleName());
    appRecord->SetKillReason(reason);
    appRecord->SetIsKillPrecedeStart(isKillPrecedeStart);
    if (ret >= 0) {
        AddToKillProcessMap(appRecord->GetProcessName());
    }
    DelayedSingleton<CacheProcessManager>::GetInstance()->OnProcessKilled(appRecord);
    return ret;
}

bool KillingProcessManager::CheckIsThreadInFoundation(pid_t pid)
{
    std::ostringstream pathBuilder;
    pathBuilder << PROC_SELF_TASK_PATH << pid;
    std::string path = pathBuilder.str();
    TAG_LOGD(AAFwkTag::APPMGR, "CheckIsThreadInFoundation path:%{public}s", path.c_str());
    return access(path.c_str(), F_OK) == 0;
}

void KillingProcessManager::AddToKillProcessMap(const std::string &processName)
{
    std::lock_guard lock(killedProcessMapLock_);
    int64_t killTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count();
    killedProcessMap_.emplace(killTime, processName);
}

void KillingProcessManager::SendReStartProcessEvent(AAFwk::EventInfo &eventInfo, int32_t appUid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::lock_guard lock(killedProcessMapLock_);
    int64_t restartTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count();
    for (auto iter = killedProcessMap_.begin(); iter != killedProcessMap_.end();) {
        int64_t killTime = iter->first;
        if (restartTime - killTime > PROCESS_RESTART_MARGIN_MICRO_SECONDS) {
            iter = killedProcessMap_.erase(iter);
            continue;
        }
        if (eventInfo.bundleName == eventInfo.callerBundleName &&
            eventInfo.processName != eventInfo.callerProcessName) {
            AppMgrEventUtil::SendReStartProcessEvent(eventInfo, appUid, restartTime);
            iter = killedProcessMap_.erase(iter);
            continue;
        }
        ++iter;
    }
}

int32_t KillingProcessManager::ForceKillApplicationInner(const std::string &bundleName,
    const int userId, const int appIndex)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }

    std::list<pid_t> pids;
    int32_t newUserId = userId;
    if (userId == DEFAULT_INVAL_VALUE) {
        newUserId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    }
    int32_t result = ERR_OK;
    if (!appRunningManager_->GetPidsByBundleNameUserIdAndAppIndex(bundleName, newUserId, appIndex, pids)) {
        TAG_LOGI(AAFwkTag::APPMGR, "unstart");
        return result;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        result = KillProcessByPid(*iter, "ForceKillApplicationByBundleName");
        if (result < 0) {
            TAG_LOGE(AAFwkTag::APPMGR,
                "forceKillApplicationByBundleName fail for bundleName:%{public}s pid:%{public}d",
                bundleName.c_str(), *iter);
            return result;
        }
    }
    return result;
}

bool KillingProcessManager::DoAllProcessExitCallback(std::list<SimpleProcessInfo> &processInfos,
    int32_t userId, sptr<AAFwk::IUserCallback> callback, int64_t startTime)
{
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "callback nullptr");
        return false;
    }
    auto checkProcessExistCallback = [processInfos, userId, callback, startTime] () mutable {
        TAG_LOGI(AAFwkTag::APPMGR, "checkProcessExistCallback call");
        if (callback == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "callback nullptr");
            return;
        }
        if (ProcessUtil::IsAllProcessKilled(processInfos)) {
            TAG_LOGI(AAFwkTag::APPMGR, "all process exit");
            callback->OnLogoutUserDone(userId, ERR_OK);
            return;
        }
        int64_t currentTime = KillingProcessManager::GetInstance().SystemTimeMillisecond();
        if (currentTime - startTime > LOGOUT_USER_TIMEOUT_MILLISION_SECONDS) {
            TAG_LOGE(AAFwkTag::APPMGR, "kill process timeout");
            callback->OnLogoutUserDone(userId, AAFwk::ERR_LOGOUT_USER_KILL_PROCESS_TIMEOUT);
            return;
        }
        KillingProcessManager::GetInstance().DoAllProcessExitCallback(processInfos,
            userId, callback, startTime);
    };
    if (taskHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "taskHandler_ nullptr");
        callback->OnLogoutUserDone(userId, AAFwk::ERR_LOGOUT_USER_TASK_HANDLE_NULL);
        return false;
    }
    taskHandler_->SubmitTaskJust(checkProcessExistCallback, "DelayCheckProcessExit",
        AMSEventHandler::DELAY_CHECK_ALL_PROCESSES_EXITED);
    return true;
}

int32_t KillingProcessManager::KillProcessesByAccessTokenId(const uint32_t accessTokenId)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_KILL_APP_PROCESSES);
    if (!isCallingPerm) {
        TAG_LOGE(AAFwkTag::APPMGR, "no permission to kill processes");
        return ERR_PERMISSION_DENIED;
    }

    int32_t result = ERR_OK;
    std::vector<pid_t> pids;
    result = GetPidsByAccessTokenId(accessTokenId, pids);
    if (result != ERR_OK) {
        return result;
    }
    if (pids.empty()) {
        TAG_LOGI(AAFwkTag::APPMGR, "no accessTokenId");
        return ERR_OK;
    }

    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        result = KillProcessByPid(*iter, "KillProcessesByAccessTokenId");
        if (result < 0) {
            TAG_LOGE(AAFwkTag::APPMGR,
                "killProcessesByAccessTokenId fail for accessTokenId:%{public}d,pid:%{public}d",
                accessTokenId, *iter);
            return result;
        }
    }
    return result;
}

int32_t KillingProcessManager::GetPidsByAccessTokenId(const uint32_t accessTokenId, std::vector<pid_t> &pids)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }
    std::vector<pid_t> foregroundPids;
    for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        if (!appRecord || !appRecord->GetSpawned()) {
            continue;
        }
        auto applicationInfo = appRecord->GetApplicationInfo();
        if (!applicationInfo) {
            continue;
        }
        if (accessTokenId == applicationInfo->accessTokenId) {
            pid_t curPid = appRecord->GetPid();
            if (appRecord->GetState() == ApplicationState::APP_STATE_FOREGROUND) {
                foregroundPids.push_back(curPid);
                continue;
            }
            pids.push_back(curPid);
        }
    }
    for (pid_t foregroundPid : foregroundPids) {
        pids.push_back(foregroundPid);
    }
    return ERR_OK;
}

int32_t KillingProcessManager::KillApplicationByUid(const std::string &bundleName, const int uid,
    const std::string& reason)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }

    int32_t result = ERR_OK;
    result = VerifyKillProcessPermission(bundleName);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
        return result;
    }

    int64_t startTime = SystemTimeMillisecond();
    std::list<pid_t> pids;
    TAG_LOGI(AAFwkTag::APPMGR, "uid value: %{public}d", uid);
    KillProcessConfig config{false, false, reason};
    if (!appRunningManager_->ProcessExitByBundleNameAndUid(bundleName, uid, pids, config)) {
        TAG_LOGI(AAFwkTag::APPMGR, "unstart");
        return result;
    }
    return WaitProcessesExitAndKill(pids, startTime, reason);
}

int32_t KillingProcessManager::KillApplicationSelf(const bool clearPageStack, const std::string& reason)
{
    TAG_LOGI(AAFwkTag::APPMGR, "start");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }

    auto callerPid = IPCSkeleton::GetCallingPid();
    auto appRecord = appRunningManager_->GetAppRunningRecordByPid(callerPid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appRecord, callerPid:%{public}d", callerPid);
        return ERR_INVALID_VALUE;
    }
    int64_t startTime = SystemTimeMillisecond();
    auto bundleName = appRecord->GetBundleName();
    auto callingUid = IPCSkeleton::GetCallingUid();
    TAG_LOGI(AAFwkTag::APPMGR, "uid value: %{public}d", callingUid);
    std::list<pid_t> pids;
    KillProcessConfig config{clearPageStack, false, reason};
    if (!appRunningManager_->ProcessExitByBundleNameAndUid(bundleName, callingUid, pids, config)) {
        TAG_LOGI(AAFwkTag::APPMGR, "unstart");
        return ERR_OK;
    }
    return WaitProcessesExitAndKill(pids, startTime, reason);
}

int32_t KillingProcessManager::KillAppSelfWithInstanceKey(const std::string &instanceKey, bool clearPageStack,
    const std::string& reason)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }

    int64_t startTime = SystemTimeMillisecond();
    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    TAG_LOGI(AAFwkTag::APPMGR, "callingTokenId value: %{public}d", callingTokenId);
    std::list<pid_t> pids;
    if (!appRunningManager_->ProcessExitByTokenIdAndInstance(callingTokenId, instanceKey, pids, clearPageStack)) {
        TAG_LOGI(AAFwkTag::APPMGR, "not exist");
        return ERR_OK;
    }
    return WaitProcessesExitAndKill(pids, startTime, reason);
}

int32_t KillingProcessManager::KillApplicationByUserIdLocked(
    const std::string &bundleName, int32_t appCloneIndex, const int userId, const KillProcessConfig &config)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }

    int64_t startTime = SystemTimeMillisecond();
    std::list<pid_t> pids;
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ null");
        return ERR_NO_INIT;
    }
    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundleMgrHelper null");
        return ERR_NO_INIT;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "userId: %{public}d, bundleName: %{public}s, appCloneIndex: %{public}d",
        userId, bundleName.c_str(), appCloneIndex);
    int uid = IN_PROCESS_CALL(bundleMgrHelper->GetUidByBundleName(bundleName, userId, appCloneIndex));
    TAG_LOGI(AAFwkTag::APPMGR, "uID value: %{public}d", uid);
    if (!appRunningManager_->ProcessExitByBundleNameAndUid(bundleName, uid, pids, config)) {
        TAG_LOGI(AAFwkTag::APPMGR, "process corresponding package name unstart");
        return ERR_OK;
    }
    return WaitProcessesExitAndKill(pids, startTime, "KillApplicationByUserId");
}

void KillingProcessManager::KillProcessByAbilityToken(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!token) {
        TAG_LOGE(AAFwkTag::APPMGR, "token null");
        return;
    }
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return;
    }
    auto appRecord = appRunningManager_->GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord unexist");
        return;
    }

    // before exec ScheduleProcessSecurityExit return
    // The resident process won't let him die
    if (appRecord->IsKeepAliveApp() && ExitResidentProcessManager::GetInstance().IsMemorySizeSufficient()) {
        return;
    }

    pid_t pid = appRecord->GetPid();
    if (pid > 0) {
        std::list<pid_t> pids;
        pids.push_back(pid);
        appRecord->ScheduleProcessSecurityExit();
        if (!WaitForRemoteProcessExit(pids, SystemTimeMillisecond())) {
            int32_t result = KillProcessByPid(pid, "KillProcessByAbilityToken");
            if (result < 0) {
                TAG_LOGE(AAFwkTag::APPMGR, "killProcessByAbilityToken kill process fail");
                return;
            }
        }
    }
}

void KillingProcessManager::KillProcessesByUserId(int32_t userId, bool isNeedSendAppSpawnMsg,
    sptr<AAFwk::IUserCallback> callback)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return;
    }
    int32_t result = ERR_OK;
    int64_t startTime = SystemTimeMillisecond();
    if (callback == nullptr) {
        std::list<pid_t> pids;
        if (!appRunningManager_->GetPidsByUserId(userId, pids)) {
            TAG_LOGI(AAFwkTag::APPMGR, "process corresponding uId unstart");
            if (isNeedSendAppSpawnMsg) {
                TAG_LOGI(AAFwkTag::APPMGR, "developer mode, send uninstall debug hap messages");
                SendAppSpawnUninstallDebugHapMsg(userId);
            }
            return;
        }
        result = WaitProcessesExitAndKill(pids, startTime, "KillProcessesByUserId");
    } else {
        // for logout user to callback
        std::list<SimpleProcessInfo> processInfos;
        if (!appRunningManager_->GetProcessInfosByUserId(userId, processInfos)) {
            TAG_LOGI(AAFwkTag::APPMGR, "process corresponding uId unstart");
            callback->OnLogoutUserDone(userId, ERR_OK);
            if (isNeedSendAppSpawnMsg) {
                TAG_LOGI(AAFwkTag::APPMGR, "developer mode, send uninstall debug hap messages");
                SendAppSpawnUninstallDebugHapMsg(userId);
            }
            return;
        }
        result = WaitProcessesExitAndKill(processInfos, startTime, "KillProcessesByUserId", userId, callback);
    }
    if (result == ERR_OK && isNeedSendAppSpawnMsg) {
        TAG_LOGI(AAFwkTag::APPMGR, "developer mode, send uninstall debug hap messages");
        SendAppSpawnUninstallDebugHapMsg(userId);
    }
}

void KillingProcessManager::SendAppSpawnUninstallDebugHapMsg(int32_t userId)
{
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null remoteClientManager_");
        return;
    }
    auto spawnClient = remoteClientManager_->GetSpawnClient();
    if (spawnClient == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null spawnClient");
        return;
    }
    auto errCode = spawnClient->SendAppSpawnUninstallDebugHapMsg(userId);
    if (FAILED(errCode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendAppSpawnUninstallDebugHapMsg failed, errCode %{public}08x", errCode);
    }
}

int32_t KillingProcessManager::KillProcessesInBatch(const std::vector<int32_t> &pids)
{
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_KILL_APP_PROCESSES)) {
        TAG_LOGE(AAFwkTag::APPMGR, "verify permission failed.");
        return ERR_PERMISSION_DENIED;
    }
    if (!AAFwk::AppUtils::GetInstance().IsStartOptionsWithAnimation()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not supported.");
        return AAFwk::ERR_CAPABILITY_NOT_SUPPORT;
    }
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }
    for (const auto& pid: pids) {
        auto appRecord = appRunningManager_->GetAppRunningRecordByPid(pid);
        if (appRecord == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
            continue;
        }
        std::string callerKey = std::to_string(pid) + ":" + std::to_string(appRecord->GetUid());
        AddKillingCallerKey(callerKey);
        (void)KillProcessByPid(pid, "KillProcessesInBatch");
    }
    return ERR_OK;
}

int32_t KillingProcessManager::KillSubProcessBypidInner(const pid_t pid, const std::string &reason,
    AAFwk::EventInfo &eventInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    int32_t ret = -1;
    if (!ProcessUtil::ProcessExist(pid) || pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid pid: %{public}d", pid);
        return AAFwk::ERR_KILL_PROCESS_NOT_EXIST;
    }
    if (CheckIsThreadInFoundation(pid)) {
        TAG_LOGI(AAFwkTag::APPMGR, "don't kill pid %{public}d", pid);
        return AAFwk::ERR_KILL_FOUNDATION_UID;
    }
    ret = kill(pid, SIGNAL_KILL);
    if (ret < 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "Kill pid %{public}d err: %{public}d", pid, ret);
    }

    std::string killReason = KILL_SUB_PROCESS_REASON_PREFIX + reason;
    AAFwk::EventReport::SendAppEvent(AAFwk::EventName::APP_TERMINATE, HiSysEventType::BEHAVIOR, eventInfo);
    int result = HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::FRAMEWORK, "PROCESS_KILL",
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, EVENT_KEY_PID, std::to_string(eventInfo.pid),
        EVENT_KEY_PROCESS_NAME, eventInfo.processName, EVENT_KEY_MESSAGE, killReason);
    TAG_LOGW(AAFwkTag::APPMGR, "hisysevent write result=%{public}d, send event [FRAMEWORK,PROCESS_KILL], pid="
        "%{public}d, processName=%{public}s, msg=%{public}s, ret=%{public}d",
        result, pid, eventInfo.processName.c_str(), killReason.c_str(), ret);
    return ret;
}

int32_t KillingProcessManager::KillSubProcessBypid(const pid_t pid, const std::string &reason)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    int32_t ret = ERR_OK;
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_INVALID_VALUE;
    }
    // get render record
    auto appRecord = appRunningManager_->GetAppRunningRecordByRenderPid(pid);
    if (appRecord) {
        auto renderRecord = appRecord->GetRenderRecordByPid(pid);
        if (renderRecord) {
            AAFwk::EventInfo renderEventInfo;
            renderEventInfo.pid = renderRecord->GetPid();
            renderEventInfo.processName = renderRecord->GetProcessName();
            return KillSubProcessBypidInner(pid, reason, renderEventInfo);
        }
    }
    // get child process
#ifdef SUPPORT_CHILD_PROCESS
    appRecord = appRunningManager_->GetAppRunningRecordByChildProcessPid(pid);
    if (appRecord) {
        auto childRecordMap = appRecord->GetChildProcessRecordMap();
        auto iter = childRecordMap.find(pid);
        if (iter != childRecordMap.end()) {
            auto childProcessRecord = iter->second;
            if (childProcessRecord) {
                AAFwk::EventInfo childEventInfo;
                childEventInfo.pid = childProcessRecord->GetPid();
                childEventInfo.processName = childProcessRecord->GetProcessName();
                return KillSubProcessBypidInner(pid, reason, childEventInfo);
            }
        }
    }
#endif
    // get child record process
    appRecord = appRunningManager_->GetAppRunningRecordByChildRecordPid(pid);
    if (appRecord) {
        auto childAppRecordMap = appRecord->GetChildAppRecordMap();
        auto it = childAppRecordMap.find(pid);
        if (it != childAppRecordMap.end()) {
            auto weakChildAppRecord = it->second;
            auto childAppRecord = weakChildAppRecord.lock();
            if (childAppRecord) {
                AAFwk::EventInfo childRecordEventInfo;
                childRecordEventInfo.pid = childAppRecord->GetPid();
                childRecordEventInfo.processName = childAppRecord->GetProcessName();
                return KillSubProcessBypidInner(pid, reason, childRecordEventInfo);
            }
        }
    }

    return ERR_OK;
}

int32_t KillingProcessManager::KillProcessesByPids(const std::vector<int32_t> &pids, const std::string &reason,
    bool subProcess, bool isKillPrecedeStart)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    int32_t ret = ERR_OK;
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }
    for (const auto& pid: pids) {
        auto appRecord = appRunningManager_->GetAppRunningRecordByPid(pid);
        if (appRecord != nullptr) {
            ret = KillProcessByPid(pid, reason, isKillPrecedeStart);
            if (ret != ERR_OK) {
                TAG_LOGW(AAFwkTag::APPMGR, "fail, pid:%{public}d", pid);
                return ret;
            }
            continue;
        }
        if (subProcess == false) {
            continue;
        }
        ret = KillSubProcessBypid(pid, reason);
        if (ret != ERR_OK) {
            TAG_LOGW(AAFwkTag::APPMGR, "KillSubProcessBypid fail, pid:%{public}d, ret:%{public}d", pid, ret);
            return ret;
        }
    }
    return ret;
}

void KillingProcessManager::KillApplicationByRecord(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Kill application by appRecord.");
    if (!appRecord || !taskHandler_) {
        TAG_LOGW(AAFwkTag::APPMGR, "appRecord or taskHandler_ null");
        return;
    }

    auto pid = appRecord->GetPid();
    appRecord->SetTerminating();
    appRecord->ScheduleProcessSecurityExit();

    auto startTime = SystemTimeMillisecond();
    std::list<pid_t> pids = {pid};
    if (WaitForRemoteProcessExit(pids, startTime)) {
        TAG_LOGI(AAFwkTag::APPMGR, "remote process exited success");
        return;
    }

    auto timeoutTask = [pid]() {
        TAG_LOGI(AAFwkTag::APPMGR, "killProcessByPid: %{public}d", pid);
        int32_t result = KillingProcessManager::GetInstance().KillProcessByPid(pid, "KillApplicationByRecord");
        if (result < 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "kill app fail, pid: %{public}d", pid);
            return;
        }
    };
    taskHandler_->SubmitTaskJust(timeoutTask, "DelayKillProcess", AMSEventHandler::KILL_PROCESS_TIMEOUT);
}

#ifdef SUPPORT_CHILD_PROCESS
void KillingProcessManager::KillChildProcess(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }
    auto childRecordMap = appRecord->GetChildProcessRecordMap();
    if (childRecordMap.empty()) {
        return;
    }
    for (auto iter : childRecordMap) {
        auto childRecord = iter.second;
        if (!childRecord) {
            continue;
        }
        auto childPid = childRecord->GetPid();
        if (childPid > 0) {
            TAG_LOGI(AAFwkTag::APPMGR, "kill child process, childPid:%{public}d, childUid:%{public}d",
                childPid, childRecord->GetUid());
            KillProcessByPid(childPid, "KillChildProcess");
            DelayedSingleton<AppStateObserverManager>::GetInstance()->OnChildProcessDied(childRecord);
        }
    }
}

void KillingProcessManager::KillAttachedChildProcess(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }
    auto parentAppRecord = appRecord->GetParentAppRecord();
    if (parentAppRecord) {
        parentAppRecord->RemoveChildAppRecord(appRecord->GetPid());
    }
    std::vector<pid_t> pids;
    std::queue<std::shared_ptr<AppRunningRecord>> queue;
    queue.push(appRecord);
    while (!queue.empty()) {
        auto front = queue.front();
        queue.pop();
        if (front == nullptr) {
            continue;
        }
        auto childAppRecordMap = front->GetChildAppRecordMap();
        for (const auto& [pid, weakChildAppRecord] : childAppRecordMap) {
            auto childRecord = weakChildAppRecord.lock();
            if (childRecord) {
                queue.push(childRecord);
                pids.push_back(pid);
            }
        }
        front->ClearChildAppRecordMap();
    }
    for (const auto& pid : pids) {
        KillProcessByPid(pid, "KillAttachedChildProcess");
    }
}
#endif // SUPPORT_CHILD_PROCESS

int32_t KillingProcessManager::KillProcessByPidForExit(int32_t pid, const std::string &reason)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }

    TAG_LOGI(AAFwkTag::APPMGR, "KillProcessByPidForExit %{public}d", pid);
    KillProcessConfig config{false, false, reason};
    if (!appRunningManager_->ProcessExitByPid(pid, config)) {
        TAG_LOGI(AAFwkTag::APPMGR, "not exist");
        return ERR_OK;
    }
    int64_t startTime = SystemTimeMillisecond();
    std::list<pid_t> pids { pid };
    return WaitProcessesExitAndKill(pids, startTime, reason);
}

int32_t KillingProcessManager::NotifyAppStatusByCommonEventName(const std::string &bundleName,
    const std::string &eventName, const Want &want)
{
    EventFwk::CommonEventData commonData {want};
    std::set<std::string> permissionControlEvents = {
        EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED,
        EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_RESTARTED
    };
    if (permissionControlEvents.find(eventName) == permissionControlEvents.end()) {
        TAG_LOGI(AAFwkTag::APPMGR, "bundle name: %{public}s, normal event: %{public}s",
            bundleName.c_str(), eventName.c_str());
        EventFwk::CommonEventManager::PublishCommonEvent(commonData);
        return 0;
    }
    EventFwk::CommonEventPublishInfo eventPublishData;
    std::vector<std::string> permissions;
    permissions.emplace_back(AAFwk::PermissionConstants::PERMISSION_LISTEN_BUNDLE_CHANGE);
    eventPublishData.SetBundleName(bundleName);
    eventPublishData.SetSubscriberPermissions(permissions);
    eventPublishData.SetSubscriberType(EventFwk::SubscriberType::SYSTEM_SUBSCRIBER_TYPE);
    eventPublishData.SetValidationRule(EventFwk::ValidationRule::OR);
    EventFwk::CommonEventManager::PublishCommonEvent(commonData, eventPublishData);
    return 1;
}

void KillingProcessManager::NotifyAppStatus(const std::string &bundleName, int32_t appIndex,
    const std::string &eventData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "bundle name is %{public}s, event is %{public}s",
        bundleName.c_str(), eventData.c_str());
    Want want;
    want.SetAction(eventData);
    ElementName element;
    element.SetBundleName(bundleName);
    want.SetElement(element);
    want.SetParam(Constants::USER_ID, 0);
    want.SetParam(Constants::APP_INDEX, appIndex);
    NotifyAppStatusByCommonEventName(bundleName, eventData, want);
}

void KillingProcessManager::NotifyAppStatusByCallerUid(const std::string &bundleName, const int32_t tokenId,
    const int32_t userId, const int32_t callerUid, const int32_t targetUid, const std::string &eventData)
{
    TAG_LOGI(AAFwkTag::APPMGR,
        "%{public}s call, bundle name: %{public}s, userId: %{public}d, event: %{public}s", __func__,
        bundleName.c_str(), userId, eventData.c_str());
    Want want;
    want.SetAction(eventData);
    ElementName element;
    element.SetBundleName(bundleName);
    want.SetElement(element);
    want.SetParam(TOKEN_ID, tokenId);
    want.SetParam(Constants::USER_ID, userId);
    want.SetParam(Constants::UID, callerUid);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, callerUid);
    want.SetParam(TARGET_UID_KEY, targetUid);
    NotifyAppStatusByCommonEventName(bundleName, eventData, want);
}

void KillingProcessManager::SendProcessKillEvent(std::shared_ptr<AppRunningRecord> appRecord,
    const std::string &defaultReason)
{
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appRecord");
        return;
    }
    std::string appRunningUniqueId = std::to_string(appRecord->GetAppStartTime());
    AAFwk::EventInfo eventInfo;
    SetKilledEventInfo(appRecord, eventInfo);
    std::string newReason = appRecord->GetKillReason().empty() ? defaultReason : appRecord->GetKillReason();
    bool foreground = appRecord->GetState() == ApplicationState::APP_STATE_FOREGROUND ||
        appRecord->GetState() == ApplicationState::APP_STATE_FOCUS;
    AAFwk::EventReport::SendAppEvent(AAFwk::EventName::APP_TERMINATE, HiSysEventType::BEHAVIOR, eventInfo);
    int result = HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::FRAMEWORK, "PROCESS_KILL",
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, EVENT_KEY_PID, std::to_string(eventInfo.pid),
        EVENT_KEY_PROCESS_NAME, eventInfo.processName, EVENT_KEY_MESSAGE, newReason, EVENT_KEY_REASON, newReason,
        EVENT_KEY_FOREGROUND, foreground, "APP_RUNNING_UNIQUE_ID", appRunningUniqueId);
    TAG_LOGW(AAFwkTag::APPMGR, "hisysevent write result=%{public}d, send event [FRAMEWORK,PROCESS_KILL], pid="
        "%{public}d, processName=%{public}s, msg=%{public}s, reason=%{public}s, FOREGROUND=%{public}d,"
        " appRunningUniqueId=%{public}s", result, eventInfo.pid, eventInfo.processName.c_str(), newReason.c_str(),
        newReason.c_str(), foreground, appRunningUniqueId.c_str());
}

void KillingProcessManager::SetKilledEventInfo(std::shared_ptr<AppRunningRecord> appRecord, AAFwk::EventInfo &eventInfo)
{
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is null");
        return;
    }
    auto applicationInfo = appRecord->GetApplicationInfo();
    if (!applicationInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "appInfo null");
    } else {
        eventInfo.bundleName = applicationInfo->name;
        eventInfo.versionName = applicationInfo->versionName;
        eventInfo.versionCode = applicationInfo->versionCode;
    }
    eventInfo.pid = appRecord->GetPid();
    eventInfo.processName = appRecord->GetProcessName();
}
}  // namespace AppExecFwk
}  // namespace OHOS