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

#ifndef OHOS_ABILITY_RUNTIME_BASE_EXTENSION_CONNECT_MANAGER_H
#define OHOS_ABILITY_RUNTIME_BASE_EXTENSION_CONNECT_MANAGER_H

#include <atomic>
#include <deque>
#include <list>
#include <map>
#include <string>
#include <unordered_map>
#include "cpp/mutex.h"

#include "ability_cache_manager.h"
#include "ability_connect_callback_interface.h"
#include "task_handler_wrap.h"
#include "event_handler_wrap.h"
#include "ability_record.h"
#include "ability_running_info.h"
#include "event_report.h"
#include "extension_config.h"
#include "extension_running_info.h"
#include "connection_record.h"
#include "element_name.h"
#include "ui_extension/ui_extension_ability_connect_info.h"
#include "want.h"
#include "iremote_object.h"
#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
using OHOS::AppExecFwk::AbilityType;
using UIExtensionAbilityConnectInfo = AbilityRuntime::UIExtensionAbilityConnectInfo;
using UIExtensionSessionInfo = AbilityRuntime::UIExtensionSessionInfo;

/**
 * @class BaseExtensionConnectManager
 * BaseExtensionConnectManager provides common facility for managing extension connection.
 */
class BaseExtensionConnectManager : public std::enable_shared_from_this<BaseExtensionConnectManager> {
public:
    using ConnectMapType = std::map<sptr<IRemoteObject>, std::list<std::shared_ptr<ConnectionRecord>>>;
    using ServiceMapType = std::map<std::string, std::shared_ptr<AbilityRecord>>;
    using ConnectListType = std::list<std::shared_ptr<ConnectionRecord>>;
    using RecipientMapType = std::map<sptr<IRemoteObject>, sptr<IRemoteObject::DeathRecipient>>;
    using UIExtWindowMapValType = std::pair<std::weak_ptr<AbilityRecord>, sptr<SessionInfo>>;
    using UIExtensionMapType = std::map<sptr<IRemoteObject>, UIExtWindowMapValType>;
    using WindowExtMapValType = std::pair<uint32_t, sptr<SessionInfo>>;
    using WindowExtensionMapType = std::map<sptr<IRemoteObject>, WindowExtMapValType>;

    struct LoadAbilityContext {
        std::shared_ptr<AbilityRuntime::LoadParam> loadParam;
        std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo;
        std::shared_ptr<AppExecFwk::ApplicationInfo> appInfo;
        std::shared_ptr<AAFwk::Want> want;
    };

    explicit BaseExtensionConnectManager(int userId);
    virtual ~BaseExtensionConnectManager();

    /**
     * ConnectAbilityLocked, connect session with extension.
     *
     * @param abilityRequest, Special want for extension.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param callerToken, caller ability token.
     * @param sessionInfo the extension session info of the ability to connect.
     * @param connectInfo the connect info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t ConnectAbilityLocked(const AbilityRequest &abilityRequest, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, sptr<SessionInfo> sessionInfo = nullptr,
        sptr<UIExtensionAbilityConnectInfo> connectInfo = nullptr);

    /**
     * DisconnectAbilityLocked, disconnect session with callback.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DisconnectAbilityLocked(const sptr<IAbilityConnection> &connect);

    /**
     * SuspendExtensionAbilityLocked, suspend session with callback.
     *
     * @param connect, Callback used to notify caller the result of suspend.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t SuspendExtensionAbilityLocked(const sptr<IAbilityConnection> &connect);

    /**
     * ResumeExtensionAbilityLocked, resume session with callback.
     *
     * @param connect, Callback used to notify caller the result of resume.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t ResumeExtensionAbilityLocked(const sptr<IAbilityConnection> &connect);

    /**
     * AttachAbilityThreadLocked, ability call this interface after loaded.
     *
     * @param scheduler, the interface handler of kit ability.
     * @param token, ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AttachAbilityThreadLocked(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token);

    virtual void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state);

    virtual void OnAppStateChanged(const AppInfo &info);

    /**
     * AbilityTransitionDone, ability call this interface after lift cycle was changed.
     *
     * @param token, ability's token.
     * @param state, the state of ability lift cycle.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AbilityTransitionDone(const sptr<IRemoteObject> &token, int state);

    /**
     * @brief execute after the ability schedule the lifecycle
     *
     * @param token the ability token
     * @param windowConfig the windowconfig
     * @return execute error code
     */
    virtual int AbilityWindowConfigTransactionDone(
        const sptr<IRemoteObject> &token, const WindowConfig &windowConfig);

    /**
     * ScheduleConnectAbilityDoneLocked, extension call this interface while session was connected.
     *
     * @param token, extension's token.
     * @param remoteObject, the session proxy of extension.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ScheduleConnectAbilityDoneLocked(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject);

    /**
     * ScheduleDisconnectAbilityDone, extension call this interface while session was disconnected.
     *
     * @param token, extension's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ScheduleDisconnectAbilityDoneLocked(const sptr<IRemoteObject> &token);

    /**
     * Get extension by token from service map.
     *
     * @param token, extension's token.
     * @return Returns AbilityRecord shared_ptr.
     */
    virtual std::shared_ptr<AbilityRecord> GetExtensionByTokenFromServiceMap(const sptr<IRemoteObject> &token);

    /**
     * Get extension by token from terminating map.
     *
     * @param token, extension's token.
     * @return Returns AbilityRecord shared_ptr.
     */
    virtual std::shared_ptr<AbilityRecord> GetExtensionByTokenFromTerminatingMap(const sptr<IRemoteObject> &token);

    /**
     * Get extension by id from service map.
     *
     * @param abilityRecordId, extension's id.
     * @return Returns AbilityRecord shared_ptr.
     */
    virtual std::shared_ptr<AbilityRecord> GetExtensionByIdFromServiceMap(const int64_t &abilityRecordId);

    /**
     * Get extension by id from terminating map.
     *
     * @param abilityRecordId, extension's id.
     * @return Returns AbilityRecord shared_ptr.
     */
    virtual std::shared_ptr<AbilityRecord> GetExtensionByIdFromTerminatingMap(const int64_t &abilityRecordId);

    /**
     * Get connect record list by callback.
     *
     * @param callback, connect callback.
     * @return Returns ConnectListType.
     */
    virtual ConnectListType GetConnectRecordListByCallback(sptr<IAbilityConnection> callback);

    /**
     * Get extension running infos.
     *
     * @param upperLimit, max count of info.
     * @param info, output extension running infos.
     * @param userId, user id.
     * @param isPerm, whether has permission.
     */
    virtual void GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info,
        const int32_t userId, bool isPerm);

    /**
     * Get ability running infos.
     *
     * @param info, output ability running infos.
     * @param isPerm, whether has permission.
     */
    virtual void GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm);

    /**
     * Get extension running info.
     *
     * @param abilityRecord, extension record.
     * @param userId, user id.
     * @param info, output extension running info.
     */
    virtual void GetExtensionRunningInfo(std::shared_ptr<AbilityRecord> &abilityRecord, const int32_t userId,
        std::vector<ExtensionRunningInfo> &info);

    /**
     * set from ability manager service for sequenced task
     */
    inline void SetTaskHandler(const std::shared_ptr<TaskHandlerWrap> &taskHandler)
    {
        taskHandler_ = taskHandler;
    }

    /**
     * SetEventHandler.
     *
     * @param handler, EventHandler.
     */
    inline void SetEventHandler(const std::shared_ptr<EventHandlerWrap> &handler)
    {
        eventHandler_ = handler;
    }

    /**
     * Get scene board token id.
     *
     * @return Returns scene board token id.
     */
    uint32_t GetSceneBoardTokenId() const
    {
        return sceneBoardTokenId_;
    }

    /**
     * OnLoadAbilityFailed.
     *
     * @param abilityRecord, extension record.
     */
    virtual void OnLoadAbilityFailed(std::shared_ptr<AbilityRecord> abilityRecord);

    /**
     * OnAbilityDied.
     *
     * @param abilityRecord, extension record.
     */
    virtual void OnAbilityDied(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * DisconnectBeforeCleanup.
     */
    virtual void DisconnectBeforeCleanup();

    /**
     * DumpState.
     *
     * @param info, output dump info.
     * @param isClient, whether is client.
     * @param args, dump args.
     */
    virtual void DumpState(std::vector<std::string> &info, bool isClient, const std::string &args = "");

    /**
     * DumpStateByUri.
     *
     * @param info, output dump info.
     * @param isClient, whether is client.
     * @param args, dump args.
     * @param params, dump params.
     */
    virtual void DumpStateByUri(std::vector<std::string> &info, bool isClient, const std::string &args,
        std::vector<std::string> &params);

    /**
     * PauseExtensions.
     */
    virtual void PauseExtensions();

    /**
     * OnTimeOut.
     *
     * @param msgId, message id.
     * @param abilityRecordId, extension record id.
     * @param isHalf, whether half timeout.
     */
    virtual void OnTimeOut(uint32_t msgId, int64_t abilityRecordId, bool isHalf = false);

    /**
     * @brief schedule to background
     *
     * @param abilityRecord the ability to move
     */
    virtual void MoveToBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * CommandAbilityWindow.
     *
     * @param abilityRecord, extension record.
     * @param sessionInfo, session info.
     * @param winCmd, window command.
     */
    virtual void CommandAbilityWindow(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd);

    /**
     * HandleProcessFrozen.
     *
     * @param pidList, process id list.
     * @param uid, user id.
     */
    virtual void HandleProcessFrozen(const std::vector<int32_t> &pidList, int32_t uid);

    /**
     * RemoveLauncherDeathRecipient.
     */
    virtual void RemoveLauncherDeathRecipient();

    /**
     * Get ui extension caller token list.
     *
     * @param abilityRecord, extension record.
     * @param callerList, output caller token list.
     */
    virtual void GetUIExtensionCallerTokenList(const std::shared_ptr<AbilityRecord> &abilityRecord,
        std::list<sptr<IRemoteObject>> &callerList);

    /**
     * CloseAssertDialog.
     *
     * @param assertSessionId, assert session id.
     */
    virtual void CloseAssertDialog(const std::string &assertSessionId);

    /**
     * SignRestartAppFlag.
     *
     * @param uid, user id.
     * @param instanceKey, instance key.
     */
    virtual void SignRestartAppFlag(int32_t uid, const std::string &instanceKey);

    /**
     * SignRestartProcess.
     *
     * @param pid, process id.
     */
    virtual void SignRestartProcess(int32_t pid);

    /**
     * Get ui extension root host info.
     *
     * @param token, extension token.
     * @return Returns AbilityRecord shared_ptr.
     */
    virtual std::shared_ptr<AAFwk::AbilityRecord> GetUIExtensionRootHostInfo(const sptr<IRemoteObject> token);

    /**
     * UninstallApp.
     *
     * @param bundleName, bundle name.
     * @param uid, user id.
     */
    virtual void UninstallApp(const std::string &bundleName, int32_t uid);

    /**
     * UpdateKeepAliveEnableState.
     *
     * @param bundleName, bundle name.
     * @param moduleName, module name.
     * @param mainElement, main element.
     * @param updateEnable, whether enable keep alive.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UpdateKeepAliveEnableState(const std::string &bundleName, const std::string &moduleName,
        const std::string &mainElement, bool updateEnable);

    /**
     * Handle extension disconnect task.
     *
     * @param connectRecord, connect record.
     */
    virtual void HandleExtensionDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);

    /**
     * Check if has request id in load ability queue.
     *
     * @param requestId, request id.
     * @return Returns true if exists, false otherwise.
     */
    virtual bool HasRequestIdInLoadAbilityQueue(int32_t requestId);

    /**
     * OnStartSpecifiedProcessResponse.
     *
     * @param flag, process flag.
     * @param requestId, request id.
     */
    virtual void OnStartSpecifiedProcessResponse(const std::string &flag, int32_t requestId);

    /**
     * OnStartSpecifiedProcessTimeoutResponse.
     *
     * @param requestId, request id.
     */
    virtual void OnStartSpecifiedProcessTimeoutResponse(int32_t requestId);

protected:
    /**
     * DisconnectAbilityLocked, disconnect session with callback.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param callerDied, bool Indicates if it is caused by the caller's death.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DisconnectAbilityLocked(const sptr<IAbilityConnection> &connect, bool callerDied);

    /**
     * LoadAbility.
     *
     * @param abilityRecord, the ptr of the ability to load.
     * @param updateRecordCallback, callback for updating record.
     */
    virtual void LoadAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
        std::function<void(const std::shared_ptr<AbilityRecord> &)> updateRecordCallback = nullptr);

    /**
     * HandleLoadAbilityOrStartSpecifiedProcess.
     *
     * @param loadParam, load parameter.
     * @param abilityRecord, extension record.
     */
    virtual void HandleLoadAbilityOrStartSpecifiedProcess(
        const AbilityRuntime::LoadParam &loadParam, const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * ConnectAbility.Schedule connect ability
     *
     * @param abilityRecord, the ptr of the ability to connect.
     */
    virtual void ConnectAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * ResumeConnectAbility.Schedule Resume Connect ability
     *
     * @param abilityRecord, the ptr of the ability to connect.
     */
    virtual void ResumeConnectAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * CommandAbility. Schedule command ability
     *
     * @param abilityRecord, the ptr of the ability to command.
     */
    virtual void CommandAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * CompleteCommandAbility. complete command ability
     *
     * @param abilityRecord, the ptr of the ability to command.
     */
    virtual void CompleteCommandAbility(std::shared_ptr<AbilityRecord> abilityRecord);

    /**
     * TerminateDone.
     *
     * @param abilityRecord, the ptr of the ability to terminate.
     */
    virtual void TerminateDone(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * GetAbilityConnectedRecordFromRecordList.
     *
     * @param targetService, the ptr of the connected ability.
     * @param connectRecordList, connect record list.
     * @return Returns ConnectionRecord shared_ptr.
     */
    virtual std::shared_ptr<ConnectionRecord> GetAbilityConnectedRecordFromRecordList(
        const std::shared_ptr<AbilityRecord> &targetService,
        std::list<std::shared_ptr<ConnectionRecord>> &connectRecordList);

    /**
     * dispatch extension life cycle .
     *
     * @param abilityRecord.
     * @param state.
     */
    virtual int DispatchInactive(const std::shared_ptr<AbilityRecord> &abilityRecord, int state);
    virtual int DispatchForeground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    virtual int DispatchBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    virtual int DispatchTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * Handle timeout tasks.
     */
    virtual void HandleStartTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord);
    virtual void HandleStopTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord);
    virtual void HandleTerminateDisconnectTask(const ConnectListType& connectlist);
    virtual void HandleCommandTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord);
    virtual void HandleCommandWindowTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd);
    virtual void HandleForegroundTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord);
    virtual void HandleConnectTimeoutTask(std::shared_ptr<AbilityRecord> abilityRecord);
    virtual void HandleRestartResidentTask(const AbilityRequest &abilityRequest);
    virtual void HandleActiveAbility(std::shared_ptr<AbilityRecord> &targetService,
        std::shared_ptr<ConnectionRecord> &connectRecord);
    virtual void HandleCommandDestroy(const sptr<SessionInfo> &sessionInfo);
    virtual void TerminateOrCacheAbility(std::shared_ptr<AbilityRecord> abilityRecord);

    /**
     * RemoveConnectionRecordFromMap.
     *
     * @param connect, the ptr of the connect record.
     */
    virtual void RemoveConnectionRecordFromMap(std::shared_ptr<ConnectionRecord> connect);

    /**
     * GetConnectRecordListFromMap.
     *
     * @param connect, callback object.
     * @param connectRecordList, the target connectRecordList.
     */
    virtual void GetConnectRecordListFromMap(
        const sptr<IAbilityConnection> &connect, std::list<std::shared_ptr<ConnectionRecord>> &connectRecordList);

    /**
     * AddConnectDeathRecipient.
     *
     * @param connectObject, callback object.
     */
    virtual void AddConnectDeathRecipient(sptr<IRemoteObject> connectObject);

    /**
     * RemoveConnectDeathRecipient.
     *
     * @param connectObject, callback object.
     */
    virtual void RemoveConnectDeathRecipient(sptr<IRemoteObject> connectObject);

    /**
     * OnCallBackDied.
     *
     * @param remote, callback object.
     */
    virtual void OnCallBackDied(const wptr<IRemoteObject> &remote);

    /**
     * HandleOnCallBackDied.
     *
     * @param connect, callback object.
     */
    virtual void HandleCallBackDiedTask(const sptr<IRemoteObject> &connect);

    /**
     * HandleOnCallBackDied.
     *
     * @param abilityRecord, died ability.
     */
    virtual void HandleAbilityDiedTask(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * RestartAbility.
     *
     * @param abilityRecord, extension record.
     * @param currentUserId, current user id.
     */
    virtual void RestartAbility(const std::shared_ptr<AbilityRecord> &abilityRecord, int32_t currentUserId);

    /**
     * PostTimeOutTask.
     *
     * @param abilityRecord, ability.
     * @param messageId, message id.
     */
    virtual void PostTimeOutTask(const std::shared_ptr<AbilityRecord> &abilityRecord, uint32_t messageId);
    virtual void PostTimeOutTask(const std::shared_ptr<AbilityRecord> &abilityRecord, int connectRecordId, uint32_t messageId);

    /**
     * CompleteForeground.
     *
     * @param abilityRecord, extension record.
     */
    virtual void CompleteForeground(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * CompleteBackground.
     *
     * @param abilityRecord, extension record.
     */
    virtual void CompleteBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * PrintTimeOutLog.
     *
     * @param ability, extension record.
     * @param msgId, message id.
     * @param isHalf, whether half timeout.
     */
    virtual void PrintTimeOutLog(const std::shared_ptr<AbilityRecord> &ability, uint32_t msgId, bool isHalf = false);

    /**
     * PostRestartResidentTask.
     *
     * @param abilityRequest, ability request.
     */
    virtual void PostRestartResidentTask(const AbilityRequest &abilityRequest);

    /**
     * IsAbilityNeedKeepAlive.
     *
     * @param abilityRecord, extension record.
     * @return Returns true if need keep alive, false otherwise.
     */
    virtual bool IsAbilityNeedKeepAlive(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * ProcessPreload.
     *
     * @param record, extension record.
     */
    virtual void ProcessPreload(const std::shared_ptr<AbilityRecord> &record) const;

    /**
     * HandleInactiveTimeout.
     *
     * @param ability, extension record.
     */
    virtual void HandleInactiveTimeout(const std::shared_ptr<AbilityRecord> &ability);

    /**
     * CleanActivatingTimeoutAbility.
     *
     * @param abilityRecord, extension record.
     */
    virtual void CleanActivatingTimeoutAbility(std::shared_ptr<AbilityRecord> abilityRecord);

    /**
     * MoveToTerminatingMap.
     *
     * @param abilityRecord, extension record.
     */
    virtual void MoveToTerminatingMap(const std::shared_ptr<AbilityRecord>& abilityRecord);

    /**
     * DoBackgroundAbilityWindow.
     *
     * @param abilityRecord, extension record.
     * @param sessionInfo, session info.
     */
    virtual void DoBackgroundAbilityWindow(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);

    /**
     * Post an extension's disconnect task, auto disconnect when extension connected timeout.
     */
    virtual void PostExtensionDelayDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);

    /**
     * Remove the extension's disconnect task.
     */
    virtual void RemoveExtensionDelayDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);

    /**
     * IsNeedToRestart.
     *
     * @param abilityRecord, extension record.
     * @param bundleName, bundle name.
     * @param abilityName, ability name.
     * @return Returns true if need restart, false otherwise.
     */
    virtual bool IsNeedToRestart(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const std::string &bundleName, const std::string &abilityName);

    /**
     * TerminateRecord.
     *
     * @param abilityRecord, extension record.
     */
    virtual void TerminateRecord(std::shared_ptr<AbilityRecord> abilityRecord);

    /**
     * DisconnectRecordNormal.
     *
     * @param list, connect record list.
     * @param connectRecord, connect record.
     * @param callerDied, whether caller died.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DisconnectRecordNormal(ConnectListType &list, std::shared_ptr<ConnectionRecord> connectRecord,
        bool callerDied) const;

    /**
     * DisconnectRecordForce.
     *
     * @param list, connect record list.
     * @param connectRecord, connect record.
     */
    virtual void DisconnectRecordForce(ConnectListType &list, std::shared_ptr<ConnectionRecord> connectRecord);

    /**
     * TerminateAbilityInner.
     *
     * @param token, extension token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int TerminateAbilityInner(const sptr<IRemoteObject> &token);

    /**
     * IsLauncher.
     *
     * @param serviceExtension, extension record.
     * @return Returns true if it's launcher, false otherwise.
     */
    virtual bool IsLauncher(std::shared_ptr<AbilityRecord> serviceExtension) const;

    /**
     * KillProcessesByUserId.
     */
    virtual void KillProcessesByUserId() const;

    /**
     * SetLastExitReason.
     *
     * @param abilityRequest, ability request.
     * @param targetService, extension record.
     */
    virtual void SetLastExitReason(const AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &targetService);

    /**
     * IsCallerValid.
     *
     * @param abilityRecord, extension record.
     * @return Returns true if caller is valid, false otherwise.
     */
    virtual bool IsCallerValid(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * BuildEventInfo.
     *
     * @param abilityRecord, extension record.
     * @return Returns EventInfo.
     */
    virtual EventInfo BuildEventInfo(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * ReportXiaoYiToRSSIfNeeded.
     *
     * @param abilityInfo, ability info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t ReportXiaoYiToRSSIfNeeded(const AppExecFwk::AbilityInfo &abilityInfo);

    /**
     * ReportAbilityStartInfoToRSS.
     *
     * @param abilityInfo, ability info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t ReportAbilityStartInfoToRSS(const AppExecFwk::AbilityInfo &abilityInfo);

    /**
     * ReportEventToRSS.
     *
     * @param abilityInfo, ability info.
     * @param abilityRecord, extension record.
     * @param callerToken, caller token.
     */
    virtual void ReportEventToRSS(const AppExecFwk::AbilityInfo &abilityInfo,
        const std::shared_ptr<AbilityRecord> abilityRecord, sptr<IRemoteObject> callerToken);

    /**
     * SetExtensionLoadParam.
     *
     * @param loadParam, load parameter.
     * @param abilityRecord, extension record.
     */
    virtual void SetExtensionLoadParam(AbilityRuntime::LoadParam &loadParam, std::shared_ptr<AbilityRecord> abilityRecord);

    /**
     * IsStrictMode.
     *
     * @param abilityRecord, extension record.
     * @return Returns true if strict mode, false otherwise.
     */
    virtual bool IsStrictMode(std::shared_ptr<AbilityRecord> abilityRecord);

    /**
     * NeedExtensionControl.
     *
     * @param abilityRecord, extension record.
     * @return Returns true if need extension control, false otherwise.
     */
    virtual bool NeedExtensionControl(std::shared_ptr<AbilityRecord> abilityRecord);

    /**
     * GetTimeoutMsgContent.
     *
     * @param msgId, message id.
     * @param msgContent, output message content.
     * @param typeId, output type id.
     * @return Returns true if success, false otherwise.
     */
    virtual bool GetTimeoutMsgContent(uint32_t msgId, std::string &msgContent, int &typeId);

    /**
     * StartSpecifiedProcess.
     *
     * @param context, load ability context.
     * @param abilityRecord, extension record.
     */
    virtual void StartSpecifiedProcess(const LoadAbilityContext &context, const std::shared_ptr<AbilityRecord> &abilityRecord);

protected:
    const std::string TASK_ON_CALLBACK_DIED = "OnCallbackDiedTask";
    const std::string TASK_ON_ABILITY_DIED = "OnAbilityDiedTask";

    int userId_;
    std::atomic_uint32_t sceneBoardTokenId_ = 0;
    ConnectMapType connectMap_;
    RecipientMapType recipientMap_;
    std::list<std::shared_ptr<AbilityRecord>> terminatingExtensionList_;
    std::shared_ptr<TaskHandlerWrap> taskHandler_;
    std::shared_ptr<EventHandlerWrap> eventHandler_;
    std::vector<AbilityRequest> restartResidentTaskList_;

    ffrt::mutex serialMutex_;
    std::mutex connectMapMutex_;
    std::mutex recipientMapMutex_;
    std::mutex loadAbilityQueueLock_;
    std::deque<std::map<int32_t, LoadAbilityContext>> loadAbilityQueue_;

    DISALLOW_COPY_AND_MOVE(BaseExtensionConnectManager);
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_BASE_EXTENSION_CONNECT_MANAGER_H