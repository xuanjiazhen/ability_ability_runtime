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

#ifndef OHOS_ABILITY_RUNTIME_KILLING_PROCESS_MANAGER_H
#define OHOS_ABILITY_RUNTIME_KILLING_PROCESS_MANAGER_H

#include <list>
#include <string>
#include <unordered_set>

#include "app_running_manager.h"
#include "app_running_record.h"
#include "cpp/mutex.h"
#include "event_report.h"
#include "iremote_object.h"
#include "kill_process_config.h"
#include "nocopyable.h"
#include "remote_client_manager.h"
#include "user_callback.h"

namespace OHOS {
namespace AppExecFwk {
class KillingProcessManager {
public:
    static KillingProcessManager& GetInstance();
    ~KillingProcessManager() = default;
    void Init(std::shared_ptr<AppRunningManager> appRunningManager,
        std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler,
        std::shared_ptr<RemoteClientManager> remoteClientManager);
    bool IsCallerKilling(const std::string& callerKey) const;
    void AddKillingCallerKey(const std::string& callerKey);
    void RemoveKillingCallerKey(const std::string& callerKey);

    /**
     * kill all processes of a bundleName
     * @param bundleName bundleName of which to be killed
     * @param clearPageStack should schedule clearPage lifecycle
     * @param reason caller function name
     */
    int32_t KillApplicationByBundleName(const std::string &bundleName, int32_t appIndex, bool clearPageStack = false,
        const std::string& reason = "KillApplicationByBundleName");

    int32_t ForceKillApplicationInner(const std::string &bundleName, const int userId = -1,
        const int appIndex = 0);

    /**
     * KillProcessesByAccessTokenId.
     *
     * @param  accessTokenId, accessTokenId.
     * @return ERR_OK, return back success, others fail.
     */
    int32_t KillProcessesByAccessTokenId(const uint32_t accessTokenId);

    /**
     * KillApplicationByUid, call KillApplicationByUid() through proxy object, kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @param  uid, uid.
     * @param  reason, caller function name.
     * @return ERR_OK, return back success, others fail.
     */
    int32_t KillApplicationByUid(const std::string &bundleName, const int uid,
        const std::string& reason = "KillApplicationByUid");

    /**
     * KillApplicationSelf, this allows app to terminate itself.
     *
     * @param clearPageStack, the flag indicates if ClearPageStack lifecycle should be scheduled.
     * @return ERR_OK for success call, others for failure.
     */
    virtual int32_t KillApplicationSelf(const bool clearPageStack = false,
        const std::string& reason = "KillApplicationSelf");

    int32_t KillAppSelfWithInstanceKey(const std::string &instanceKey, bool clearPageStack, const std::string& reason);

    /**
     * KillApplicationByUserId, kill the application by user ID.
     *
     * @param bundleName, bundle name in Application record.
     * @param appCloneIndex the app clone id.
     * @param userId, user ID.
     * @param  reason, caller function name.
     *
     * @return ERR_OK, return back success, others fail.
     */
    int32_t KillApplicationByUserIdLocked(const std::string &bundleName, int32_t appCloneIndex, int32_t userId,
        const KillProcessConfig &config = {});

        /**
     * KillProcessByAbilityToken, kill the process by ability token.
     *
     * @param token, the unique identification to the ability.
     * @return
     */
    void KillProcessByAbilityToken(const sptr<IRemoteObject> &token);

    /**
     * KillProcessesByUserId, kill the processes by userId.
     * Send appSpawn uninstall debug hap message.
     *
     * @param userId, the user id.
     * @param isNeedSendAppSpawnMsg, true send appSpawn message otherwise not send.
     * @return
     */
    void KillProcessesByUserId(int32_t userId, bool isNeedSendAppSpawnMsg = false,
        sptr<AAFwk::IUserCallback> callback = nullptr);

    /**
     * KillProcessesInBatch, kill processes in batch;
     * the killed bundle won't be started by the watcher.
     *
     * @param pids, the pid list of processes are going to be killed.
     * @return ERR_OK, return back success, others fail.
     */
    int32_t KillProcessesInBatch(const std::vector<int32_t> &pids);

    /**
     * KillProcessesByPids, only in process call is allowed,
     * kill the processes by pid list given.
     *
     * @param pids, the pid list of processes are going to be killed.
     * @param reason, the reason to kill the processes.
     * @param subProcess, kill SubProcess or not.
     * @return ERR_OK, return back success, others fail.
     */
    int32_t KillProcessesByPids(const std::vector<int32_t> &pids,
        const std::string &reason = "KillProcessesByPids", bool subProcess = false, bool isKillPrecedeStart = false);

    void KillApplicationByRecord(const std::shared_ptr<AppRunningRecord> &appRecord);

    int32_t KillProcessByPidForExit(int32_t pid, const std::string &reason);

    /**
     * Verify whether the caller has the permission to kill processes of a bundleName.
     */
    int32_t VerifyKillProcessPermission(const std::string &bundleName) const;

    int32_t VerifyKillProcessPermission(const sptr<IRemoteObject> &token) const;

    int32_t KillProcessByPid(const pid_t pid, const std::string& reason = "foundation",
        bool isKillPrecedeStart = false);

    /**
     * SystemTimeMillisecond, Get system time.
     *
     * @return the system time.
     */
    int64_t SystemTimeMillisecond();

    /**
     * Notify application status.
     *
     * @param bundleName Indicates the name of the bundle.
     * @param eventData Indicates the event defined by CommonEventSupport
     *
     * @return
     */
    void NotifyAppStatus(const std::string &bundleName, int32_t appIndex, const std::string &eventData);

    /**
     * Notify application status by event name.
     *
     * @param bundleName Indicates the name of the bundle.
     * @param eventName Indicates the event defined by CommonEventSupport
     * @param want Want contains information of the event wish to notify.
     *
     * @return Returns 0 if the event is normal, 1 if the event needs to be managed.
     */
    int32_t NotifyAppStatusByCommonEventName(const std::string &bundleName, const std::string &eventName,
        const Want &want);

    /**
     * Notify application status.
     *
     * @param bundleName Indicates the name of the bundle.
     * @param bundleName Indicates the name of the bundle.
     * @param eventData Indicates the event defined by CommonEventSupport
     *
     * @return
     */
    void NotifyAppStatusByCallerUid(const std::string &bundleName, const int32_t tokenId, const int32_t userId,
        const int32_t callerUid, const int32_t targetUid, const std::string &eventData);

    void SendReStartProcessEvent(AAFwk::EventInfo &eventInfo, int32_t appUid);

    /**
     * WaitForRemoteProcessExit, Wait for the process to exit normally.
     *
     * @param pids, process number collection to exit.
     * @param startTime, execution process security exit start time.
     *
     * @return true, return back success，others fail.
     */
    bool WaitForRemoteProcessExit(std::list<pid_t> &pids, const int64_t startTime);

    void SendProcessKillEvent(std::shared_ptr<AppRunningRecord> appRecord, const std::string &defaultReason);

#ifdef SUPPORT_CHILD_PROCESS
    /**
     * kill all child processed of a main process
     * @param appRecord indicates the main process
     */
    void KillChildProcess(const std::shared_ptr<AppRunningRecord> &appRecord);

    void KillAttachedChildProcess(const std::shared_ptr<AppRunningRecord> &appRecord);
#endif // SUPPORT_CHILD_PROCESS

private:
    KillingProcessManager() = default;

    int32_t VerifyKillProcessPermissionCommon() const;

    bool VerifyAPL() const;

    /**
     * WaitProcessesExitAndKill, Wait for the process to exit normally, and kill it if time out.
     *
     * @param pids, process number collection to exit.
     * @param startTime, execution process security exit start time.
     * @param reason caller function name
     *
     * @return ERR_OK, return back success, others fail.
     */
    int32_t WaitProcessesExitAndKill(std::list<pid_t> &pids, const int64_t startTime, const std::string& reason);

    int32_t WaitProcessesExitAndKill(std::list<SimpleProcessInfo> &processInfos, const int64_t startTime,
        const std::string& reason, int32_t userId, sptr<AAFwk::IUserCallback> callback);

    bool WaitForRemoteProcessExit(std::list<SimpleProcessInfo> &processInfos, const int64_t startTime);

    int32_t KillProcessByPidInner(const pid_t pid, const std::string& reason,
        const std::string& killReason, std::shared_ptr<AppRunningRecord> appRecord, bool isKillPrecedeStart);

    bool CheckIsThreadInFoundation(pid_t pid);

    void AddToKillProcessMap(const std::string &processName);

    bool DoAllProcessExitCallback(std::list<SimpleProcessInfo> &processInfos, int32_t userId,
        sptr<AAFwk::IUserCallback> callback, int64_t startTime);

    int32_t GetPidsByAccessTokenId(const uint32_t accessTokenId, std::vector<pid_t> &pids);

    void SendAppSpawnUninstallDebugHapMsg(int32_t userId);

    int32_t KillSubProcessBypidInner(const pid_t pid, const std::string &reason,
        AAFwk::EventInfo &eventInfo);

    int32_t KillSubProcessBypid(const pid_t pid, const std::string &reason);

    void SetKilledEventInfo(std::shared_ptr<AppRunningRecord> appRecord, AAFwk::EventInfo &eventInfo);

    mutable ffrt::mutex mutex_;
    std::unordered_set<std::string> killingCallerKeySet_;
    std::shared_ptr<AppRunningManager> appRunningManager_ = nullptr;
    std::shared_ptr<RemoteClientManager> remoteClientManager_ = nullptr;
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    ffrt::mutex killedProcessMapLock_;
    mutable std::map<int64_t, std::string> killedProcessMap_;
    DISALLOW_COPY_AND_MOVE(KillingProcessManager);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_KILLING_PROCESS_MANAGER_H
