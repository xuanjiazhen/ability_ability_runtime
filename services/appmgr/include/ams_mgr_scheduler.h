/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_AMS_MGR_SCHEDULER_H
#define OHOS_ABILITY_RUNTIME_AMS_MGR_SCHEDULER_H

#include "ability_info.h"
#include "ability_running_record.h"
#include "ams_mgr_stub.h"
#include "iapp_debug_listener.h"
#include "app_mgr_constants.h"
#include "app_mgr_service_inner.h"
#include "app_record_id.h"
#include "app_running_record.h"
#include "app_scheduler_proxy.h"
#include "appexecfwk_errors.h"
#include "application_info.h"
#include "if_system_ability_manager.h"
#include "nocopyable.h"
#include "system_ability.h"
#include "task_handler_wrap.h"

namespace OHOS {
namespace AppExecFwk {
class AmsMgrScheduler : public AmsMgrStub {
public:
    AmsMgrScheduler(
        const std::shared_ptr<AppMgrServiceInner> &MgrServiceInner_,
        const std::shared_ptr<AAFwk::TaskHandlerWrap> &Handler_);
    virtual ~AmsMgrScheduler() override;

    /**
     * LoadAbility, call LoadAbility() through proxy project, load the ability that needed to be started.
     *
     * @param token, the unique identification to start the ability.
     * @param preToken, the unique identification to call the ability.
     * @param abilityInfo, the ability information.
     * @param appInfo, the app information.
     * @param want, the starting information.
     */
    virtual ErrCode LoadAbility(const std::shared_ptr<AbilityInfo> &abilityInfo,
        const std::shared_ptr<ApplicationInfo> &appInfo,
        const std::shared_ptr<AAFwk::Want> &want, const std::shared_ptr<AbilityRuntime::LoadParam>& loadParam) override;

    /**
     * TerminateAbility, call TerminateAbility() through the proxy object, terminate the token ability.
     *
     * @param token, token, he unique identification to terminate the ability.
     * @param clearMissionFlag, indicates whether terminate the ability when clearMission.
     * @return
     */
    virtual ErrCode TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag) override;

    /**
     * UpdateAbilityState, call UpdateAbilityState() through the proxy object, update the ability status.
     *
     * @param token, the unique identification to update the ability.
     * @param state, ability status that needs to be updated.
     * @return
     */
    virtual ErrCode UpdateAbilityState(const sptr<IRemoteObject> &token, int32_t state) override;

    /**
     * UpdateExtensionState, call UpdateExtensionState() through the proxy object, update the extension status.
     *
     * @param token, the unique identification to update the extension.
     * @param state, extension status that needs to be updated.
     * @return
     */
    virtual ErrCode UpdateExtensionState(const sptr<IRemoteObject> &token, ExtensionState state) override;

    /**
     * RegisterAppStateCallback, call RegisterAppStateCallback() through the proxy object, register the callback.
     *
     * @param callback, Ams register the callback.
     * @return
     */
    virtual ErrCode RegisterAppStateCallback(const sptr<IAppStateCallback> &callback) override;

    /**
     * KillProcessByAbilityToken, call KillProcessByAbilityToken() through proxy object,
     * kill the process by ability token.
     *
     * @param token, the unique identification to the ability.
     * @return
     */
    virtual ErrCode KillProcessByAbilityToken(const sptr<IRemoteObject> &token) override;

    /**
     * KillProcessesByUserId, call KillProcessesByUserId() through proxy object,
     * kill the processes by userId.
     * Send appSpawn uninstall debug hap message.
     *
     * @param userId, the user id.
     * @param isNeedSendAppSpawnMsg, true send appSpawn message otherwise not send.
     * @return
     */
    virtual ErrCode KillProcessesByUserId(int32_t userId, bool isNeedSendAppSpawnMsg,
        const sptr<AAFwk::IUserCallback>& callback) override;

    /**
     * KillProcessesByPids, only in process call is allowed,
     * kill the processes by pid list given.
     *
     * @param pids, the pid list of processes are going to be killed.
     * @param reason, the reason to kill the processes.
     * @param subProcess, kill SubProcess or not.
     * @return ERR_OK, return back success, others fail.
     */
    virtual ErrCode KillProcessesByPids(const std::vector<int32_t> &pids,
        const std::string &reason, bool subProcess) override;

    /**
     * Set child and parent relationship
     * @param token child process
     * @param callerToken parent process
     */
    virtual ErrCode AttachPidToParent(const sptr<IRemoteObject> &token,
        const sptr<IRemoteObject> &callerToken) override;

    /**
     * KillProcessWithAccount, call KillProcessWithAccount() through proxy object, kill the
     * process.
     *
     * @param bundleName, bundle name in Application record.
     * @param accountId, account ID.
     * @return ERR_OK, return back success, others fail.
     */
    virtual ErrCode KillProcessWithAccount(const std::string &bundleName, int32_t accountId,
        bool clearPageStack, int32_t appIndex) override;

    /**
     * KillProcessesInBatch, kill processes in batch, call KillProcessesInBatch() through proxy object;
     * the killed bundle won't be started by the watcher.
     *
     * @param pids, the pid list of processes are going to be killed.
     * @return ERR_OK, return back success, others fail.
     */
    virtual ErrCode KillProcessesInBatch(const std::vector<int32_t> &pids) override;

    /**
     * UpdateApplicationInfoInstalled, call UpdateApplicationInfoInstalled() through proxy object,
     * update the application info after new module installed.
     *
     * @param bundleName, bundle name in Application record.
     * @param  uid, uid.
     * @return ERR_OK, return back success, others fail.
     */
    virtual ErrCode UpdateApplicationInfoInstalled(const std::string &bundleName, int32_t uid,
        const std::string &moduleName, bool isPlugin) override;

    /**
     * KillApplication, call KillApplication() through proxy object, kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @return ERR_OK, return back success, others fail.
     */
    virtual ErrCode KillApplication(const std::string &bundleName, bool clearPageStack = false,
        int32_t appIndex = 0) override;

    /**
     * ForceKillApplication, force kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @param  userId, userId.
     * @param  appIndex, appIndex.
     * @return ERR_OK, return back success, others fail.
     */
    virtual ErrCode ForceKillApplication(const std::string &bundleName, int32_t userId,
        int32_t appIndex) override;

    /**
     * KillProcessesByAccessTokenId.
     *
     * @param  accessTokenId, accessTokenId.
     * @return ERR_OK, return back success, others fail.
     */
    virtual ErrCode KillProcessesByAccessTokenId(uint32_t accessTokenId) override;

    /**
     * KillApplicationByUid, call KillApplicationByUid() through proxy object, kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @param  uid, uid.
     * @param  reason, caller function name.
     * @return ERR_OK, return back success, others fail.
     */
    virtual ErrCode KillApplicationByUid(const std::string &bundleName, int32_t uid,
        const std::string& reason) override;

    /**
     * KillApplicationSelf, this allows app to terminate itself.
     *
     * @param clearPageStack, the flag indicates if ClearPageStack lifecycle should be scheduled.
     * @return ERR_OK for success call, others for failure.
     */
    virtual ErrCode KillApplicationSelf(bool clearPageStack,
        const std::string& reason) override;

    /**
     * Get application info by process id.
     *
     * @param pid Process id.
     * @param application Application information got.
     * @param debug Whether IsDebugApp.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetApplicationInfoByProcessID(int32_t pid, AppExecFwk::ApplicationInfo &application, bool &debug) override;

    /**
     * Record process exit reason to appRunningRecord
     * @param pid pid
     * @param reason reason enum
     * @param exitMsg exitMsg
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual ErrCode NotifyAppMgrRecordExitReason(int32_t pid, int32_t reason, const std::string &exitMsg) override;

    /**
     * AbilityAttachTimeOut, called by ability manager service when an ability is loaded timeout.
     *
     * @param token indicates the token of the ability which is timeout.
     */
    virtual ErrCode AbilityAttachTimeOut(const sptr<IRemoteObject> &token) override;

    /**
     * PrepareTerminate, called before terminating one ability by ability manager service to notify application.
     *
     * @param token indicates the token of the ability to be terminated.
     * @param clearMissionFlag indicates whether it is caused by cleaning mission.
     */
    virtual ErrCode PrepareTerminate(const sptr<IRemoteObject> &token, bool clearMissionFlag) override;

    /**
     * GetRunningProcessInfoByToken, get process info for one ability.
     *
     * @param token indicates the token of the ability requested.
     * @param info output of the information.
     */
    virtual ErrCode GetRunningProcessInfoByToken(
        const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info) override;

    /**
     * Set AbilityForegroundingFlag of an app-record to true.
     *
     * @param pid, pid.
     *
     */
    ErrCode SetAbilityForegroundingFlagToAppRecord(int32_t pid) override;

    /**
     * Start specified ability.
     *
     * @param want Want contains information of the ability to start.
     * @param abilityInfo Ability information.
     * @param requestId request id to callback
     */
    virtual ErrCode StartSpecifiedAbility(
        const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo, int32_t requestId) override;

    /**
     * Register response of start specified ability.
     *
     * @param response Response of start specified ability.
     */
    virtual ErrCode RegisterStartSpecifiedAbilityResponse(
        const sptr<IStartSpecifiedAbilityResponse> &response) override;

    virtual ErrCode PrepareTerminateApp(int32_t pid, const std::string &moduleName) override;

    /**
     * Start specified process.
     *
     * @param want Want contains information wish to start.
     * @param abilityInfo Ability information.
     * @param requestId for callback
     */
    virtual ErrCode StartSpecifiedProcess(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t requestId = 0) override;

    /**
     * SetCurrentUserId, set the userid to app mgr by ability mgr when switching to another user.
     *
     * @param userId the new user.
     */
    virtual ErrCode SetCurrentUserId(int32_t userId) override;

    virtual ErrCode SetEnableStartProcessFlagByUserId(int32_t userId, bool enableStartProcess) override;

    /**
     * GetBundleNameByPid, get bundleName and uid of a process.
     *
     * @param pid input param indicates the pid of the process.
     * @param bundleName output for bundleName.
     * @param uid output for uid.
     * @return ERR_OK for success call, others for failure.
     */
    virtual ErrCode GetBundleNameByPid(int32_t pid, std::string &bundleName, int32_t &uid) override;

    /**
     * @brief Register app debug listener.
     * @param listener App debug listener.
     * @return ERR_OK, return back success, others fail.
     */
    ErrCode RegisterAppDebugListener(const sptr<IAppDebugListener> &listener) override;

    /**
     * @brief Unregister app debug listener.
     * @param listener App debug listener.
     * @return ERR_OK, return back success, others fail.
     */
    ErrCode UnregisterAppDebugListener(const sptr<IAppDebugListener> &listener) override;

    /**
     * @brief Attach app debug.
     * @param bundleName The application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode AttachAppDebug(const std::string &bundleName, bool isDebugFromLocal) override;

    /**
     * @brief Detach app debug.
     * @param bundleName The application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode DetachAppDebug(const std::string &bundleName) override;

    /**
     * @brief Set app waiting debug mode.
     * @param bundleName The application bundle name.
     * @param isPersist The persist flag.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode SetAppWaitingDebug(const std::string &bundleName, bool isPersist) override;

    /**
     * @brief Cancel app waiting debug mode.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode CancelAppWaitingDebug() override;

    /**
     * @brief Get waiting debug mode application.
     * @param debugInfoList The debug info list, including bundle name and persist flag.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetWaitingDebugApp(std::vector<std::string> &debugInfoList) override;

    /**
     * @brief Determine whether it is a waiting debug application based on the bundle name.
     * @return Returns true if it is a waiting debug application, otherwise it returns false.
     */
    ErrCode IsWaitingDebugApp(const std::string &bundleName, bool& funcResult) override;

    /**
     * @brief Clear non persist waiting debug flag.
     */
    ErrCode ClearNonPersistWaitingDebugFlag() override;

    /**
     * @brief Registering ability debug mode response.
     * @param response Response for ability debug object.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RegisterAbilityDebugResponse(const sptr<IAbilityDebugResponse> &response) override;

    /**
     * @brief Determine whether it is an attachment debug application based on the bundle name.
     * @param bundleName The application bundle name.
     * @return Returns true if it is an attach debug application, otherwise it returns false.
     */
    ErrCode IsAttachDebug(const std::string &bundleName, bool& funcResult) override;

    /**
     * @brief Set resident process enable status.
     * @param bundleName The application bundle name.
     * @param enable The current updated enable status.
     * @param uid indicates user, 0 for all users
     */
    ErrCode SetKeepAliveEnableState(const std::string &bundleName, bool enable, int32_t uid) override;

    /**
     * @brief Set non-resident keep-alive process status.
     * @param bundleName The application bundle name.
     * @param enable The current updated enable status.
     * @param uid indicates user, 0 for all users
     */
    ErrCode SetKeepAliveDkv(const std::string &bundleName, bool enable, int32_t uid) override;

    /**
     * To clear the process by ability token.
     *
     * @param token the unique identification to the ability.
     */
    virtual ErrCode ClearProcessByToken(const sptr<IRemoteObject>& token) override;

    /**
     * whether memory size is sufficient.
     * @return Returns true is sufficient memory size, others return false.
     */
    virtual ErrCode IsMemorySizeSufficent(bool& funcResult) override;

    /**
     * whether or not requier a big memory
     * @return Returens true is no big memory, others return false.
     */
    virtual ErrCode IsNoRequireBigMemory(bool& funcResult) override;

    /**
     * Notifies that one ability is attached to status bar.
     *
     * @param token the token of the abilityRecord that is attached to status bar.
     */
    ErrCode AttachedToStatusBar(const sptr<IRemoteObject> &token) override;

    virtual ErrCode BlockProcessCacheByPids(const std::vector<int32_t> &pids) override;

    /**
     * whether killed for upgrade web.
     *
     * @param bundleName the bundle name is killed for upgrade web.
     * @return Returns true is killed for upgrade web, others return false.
     */
    virtual ErrCode IsKilledForUpgradeWeb(const std::string &bundleName, bool& funcResult) override;

    /**
     * Request to clean uiability from user.
     *
     * @param token the token of ability.
     * @return Returns true if clean success, others return false.
     */
    virtual ErrCode CleanAbilityByUserRequest(const sptr<IRemoteObject> &token, bool& funcResult) override;

    /**
     * whether the abilities of process specified by pid type only UIAbility.
     * @return Returns true is only UIAbility, otherwise return false
     */
    virtual ErrCode IsProcessContainsOnlyUIAbility(int32_t pid, bool& funcResult) override;

    /**
     * Whether a process is attached, refer to AttachApplication
     */
    virtual ErrCode IsProcessAttached(const sptr<IRemoteObject>& token, bool& funcResult) override;

    virtual ErrCode IsCallerKilling(const std::string& callerKey, bool& funcResult) override;

private:
    /**
     * @brief Judge whether the application service is ready.
     *
     * @return Returns true means service is ready, otherwise service is not ready.
     */
    bool IsReady() const;

private:
    std::shared_ptr<AppMgrServiceInner> amsMgrServiceInner_;
    std::shared_ptr<AAFwk::TaskHandlerWrap> amsHandler_;
    sptr<ISystemAbilityManager> systemAbilityMgr_;

    DISALLOW_COPY_AND_MOVE(AmsMgrScheduler);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_AMS_MGR_SCHEDULER_H
