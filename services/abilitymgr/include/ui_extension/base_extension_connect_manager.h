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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_CONNECT_MANAGER_BASE_H
#define OHOS_ABILITY_RUNTIME_ABILITY_CONNECT_MANAGER_BASE_H

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
#include "want.h"
#include "iremote_object.h"
#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
using OHOS::AppExecFwk::AbilityType;

/**
 * @class BaseExtensionConnectManager
 * BaseExtensionConnectManager provides basic facility for managing service ability connection.
 */
class BaseExtensionConnectManager : public std::enable_shared_from_this<BaseExtensionConnectManager> {
public:
    using ConnectMapType = std::map<sptr<IRemoteObject>, std::list<std::shared_ptr<ConnectionRecord>>>;
    using ServiceMapType = std::map<std::string, std::shared_ptr<AbilityRecord>>;
    using ConnectListType = std::list<std::shared_ptr<ConnectionRecord>>;
    using RecipientMapType = std::map<sptr<IRemoteObject>, sptr<IRemoteObject::DeathRecipient>>;
    using WindowExtMapValType = std::pair<uint32_t, sptr<SessionInfo>>;
    using WindowExtensionMapType = std::map<sptr<IRemoteObject>, WindowExtMapValType>;

    explicit BaseExtensionConnectManager(int userId);
    virtual ~BaseExtensionConnectManager();

    /**
     * StartAbility with request.
     *
     * @param abilityRequest, the request of the service ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t StartAbility(const AbilityRequest &abilityRequest);

    /**
     * TerminateAbility with token and result want.
     *
     * @param token, the token of service type's ability to terminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int TerminateAbility(const sptr<IRemoteObject> &token);

    /**
     * StopServiceAbility with request.
     *
     * @param abilityRequest, request.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StopServiceAbility(const AbilityRequest &abilityRequest);

    /**
     * ConnectAbilityLocked, connect session with service ability.
     *
     * @param abilityRequest, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param callerToken, caller ability token.
     * @param sessionInfo the extension session info of the ability to connect.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t ConnectAbilityLocked(const AbilityRequest &abilityRequest, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, sptr<SessionInfo> sessionInfo = nullptr);

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
     * ScheduleConnectAbilityDoneLocked, service ability call this interface while session was connected.
     *
     * @param token, service ability's token.
     * @param remoteObject, the session proxy of service ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ScheduleConnectAbilityDoneLocked(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject);

    /**
     * ScheduleDisconnectAbilityDone, service ability call this interface while session was disconnected.
     *
     * @param token,service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ScheduleDisconnectAbilityDoneLocked(const sptr<IRemoteObject> &token);

    /**
     * ScheduleCommandAbilityDoneLocked, service ability call this interface while session was onCommanded.
     *
     * @param token,service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ScheduleCommandAbilityDoneLocked(const sptr<IRemoteObject> &token);

    virtual int ScheduleCommandAbilityWindowDone(
        const sptr<IRemoteObject> &token,
        const sptr<SessionInfo> &sessionInfo,
        WindowCommand winCmd,
        AbilityCommand abilityCmd);

    virtual std::shared_ptr<AbilityRecord> GetExtensionByTokenFromServiceMap(const sptr<IRemoteObject> &token);
    virtual std::shared_ptr<AbilityRecord> GetExtensionByTokenFromTerminatingMap(const sptr<IRemoteObject> &token);
    virtual std::shared_ptr<AbilityRecord> GetExtensionByIdFromServiceMap(const int64_t &abilityRecordId);
    virtual std::shared_ptr<AbilityRecord> GetExtensionByIdFromTerminatingMap(const int64_t &abilityRecordId);
    virtual ConnectListType GetConnectRecordListByCallback(sptr<IAbilityConnection> callback);

    virtual void GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info,
        const int32_t userId, bool isPerm);

    virtual void GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm);

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
     * @param handler,EventHandler
     */
    inline void SetEventHandler(const std::shared_ptr<EventHandlerWrap> &handler)
    {
        eventHandler_ = handler;
    }

    uint32_t GetSceneBoardTokenId() const
    {
        return sceneBoardTokenId_;
    }

    virtual void OnLoadAbilityFailed(std::shared_ptr<AbilityRecord> abilityRecord);

    /**
     * OnAbilityDied.
     *
     * @param abilityRecord, service ability record.
     */
    virtual void OnAbilityDied(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * DisconnectBeforeCleanup.
     *
     */
    virtual void DisconnectBeforeCleanup();

    virtual void DumpState(std::vector<std::string> &info, bool isClient, const std::string &args = "");

    virtual void DumpStateByUri(std::vector<std::string> &info, bool isClient, const std::string &args,
        std::vector<std::string> &params);

    virtual void PauseExtensions();

    virtual void OnTimeOut(uint32_t msgId, int64_t abilityRecordId, bool isHalf = false);

    /**
     * @brief schedule to background
     *
     * @param abilityRecord the ability to move
     */
    virtual void MoveToBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);

    virtual void CommandAbilityWindow(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd);

    virtual bool IsWindowExtensionFocused(uint32_t extensionTokenId, const sptr<IRemoteObject>& focusToken);

    virtual void HandleProcessFrozen(const std::vector<int32_t> &pidList, int32_t uid);

    virtual void ForegroundAbilityWindowLocked(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);

    virtual void BackgroundAbilityWindowLocked(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);

    virtual void TerminateAbilityWindowLocked(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);

    virtual void RemoveLauncherDeathRecipient();

    virtual void CloseAssertDialog(const std::string &assertSessionId);

    virtual void SignRestartAppFlag(int32_t uid, const std::string &instanceKey);
    virtual void SignRestartProcess(int32_t pid);

    virtual void UninstallApp(const std::string &bundleName, int32_t uid);

    virtual int32_t UpdateKeepAliveEnableState(const std::string &bundleName, const std::string &moduleName,
        const std::string &mainElement, bool updateEnable);

    /**
     * Handle extension disconnect task.
     */
    virtual void HandleExtensionDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);

    /**
     * GetServiceRecordByAbilityRequest.
     *
     * @param abilityRequest, the request of the service ability to start.
     * @return Returns AbilityRecord shared_ptr.
     */
    virtual std::shared_ptr<AbilityRecord> GetServiceRecordByAbilityRequest(const AbilityRequest &abilityRequest);

protected:
    /**
     * StartAbilityLocked with request.
     *
     * @param abilityRequest, the request of the service ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t StartAbilityLocked(const AbilityRequest &abilityRequest);

    /**
     * TerminateAbilityLocked with token and result want.
     *
     * @param token, the token of service type's ability to terminate.
     * @param resultCode, the result code of service type's ability to terminate.
     * @param resultWant, the result want for service type's ability to terminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int TerminateAbilityLocked(const sptr<IRemoteObject> &token);

    /**
     * StopAbilityLocked with request.
     *
     * @param abilityRequest, the request of the service ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StopServiceAbilityLocked(const AbilityRequest &abilityRequest);

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
     */
    virtual void LoadAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
        std::function<void(const std::shared_ptr<AbilityRecord> &)> updateRecordCallback = nullptr);
        
    /**
     * ConnectAbility.Schedule connect ability
     *
     * @param abilityRecord, the ptr of the ability to connect.
     */
    virtual void ConnectAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * ConnectAbility.Schedule Resume Connect ability
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
     * GetServiceRecordByElementName.
     *
     * @param element, service ability's element.
     * @return Returns AbilityRecord shared_ptr.
     */
    virtual std::shared_ptr<AbilityRecord> GetServiceRecordByElementName(const std::string &element);

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
     * dispatch service ability life cycle .
     *
     * @param abilityRecord.
     * @param state.
     */
    virtual int DispatchInactive(const std::shared_ptr<AbilityRecord> &abilityRecord, int state);
    virtual int DispatchForeground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    virtual int DispatchBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    virtual int DispatchTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord);

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
     * RemoveServiceAbility.
     *
     * @param service, the ptr of the ability record.
     */
    virtual void RemoveServiceAbility(const std::shared_ptr<AbilityRecord> &service);

    /**
     * GetOrCreateServiceRecord.
     *
     * @param abilityRequest, Special want for service type's ability.
     * @param isCreatedByConnect, whether is created by connect ability mode.
     * @param targetAbilityRecord, the target service ability record.
     * @param isLoadedAbility, whether the target ability has been loaded.
     */
    virtual void GetOrCreateServiceRecord(const AbilityRequest &abilityRequest, const bool isCreatedByConnect,
        std::shared_ptr<AbilityRecord> &targetAbilityRecord, bool &isLoadedAbility);
        
    virtual void SetServiceAfterNewCreate(const AbilityRequest &abilityRequest, AbilityRecord &targetService);

    virtual void RemoveServiceFromMapSafe(const std::string &serviceKey);

    /**
     * GetConnectRecordListFromMap.
     *
     * @param connect, callback object.
     * @param isCreatedByConnect, whether is created by connect ability mode.
     * @param connectRecordList, the target connectRecordList.
     * @param isCallbackConnected, whether the callback has been connected.
     */
    virtual void GetConnectRecordListFromMap(
        const sptr<IAbilityConnection> &connect, std::list<std::shared_ptr<ConnectionRecord>> &connectRecordList);

    /**
     * AddConnectDeathRecipient.
     *
     * @param connect, callback object.
     */
    virtual void AddConnectDeathRecipient(sptr<IRemoteObject> connectObject);

    /**
     * RemoteConnectDeathRecipient.
     *
     * @param connect, callback object.
     */
    virtual void RemoveConnectDeathRecipient(sptr<IRemoteObject> connectObject);

    /**
     * RemoteConnectDeathRecipient.
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

    virtual void RestartAbility(const std::shared_ptr<AbilityRecord> &abilityRecord, int32_t currentUserId);

    /**
     * PostTimeOutTask.
     *
     * @param abilityRecord, ability.
     * @param messageId, message id.
     */
    virtual void PostTimeOutTask(const std::shared_ptr<AbilityRecord> &abilityRecord, uint32_t messageId);
    virtual void PostTimeOutTask(const std::shared_ptr<AbilityRecord> &abilityRecord, int connectRecordId, uint32_t messageId);

    virtual void CompleteForeground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    virtual void CompleteBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    virtual void PrintTimeOutLog(const std::shared_ptr<AbilityRecord> &ability, uint32_t msgId, bool isHalf = false);

    virtual void PostRestartResidentTask(const AbilityRequest &abilityRequest);

    virtual bool IsAbilityNeedKeepAlive(const std::shared_ptr<AbilityRecord> &abilityRecord);

    virtual void ProcessPreload(const std::shared_ptr<AbilityRecord> &record) const;

    virtual void HandleInactiveTimeout(const std::shared_ptr<AbilityRecord> &ability);
    virtual void CleanActivatingTimeoutAbility(std::shared_ptr<AbilityRecord> abilityRecord);
    virtual void MoveToTerminatingMap(const std::shared_ptr<AbilityRecord>& abilityRecord);

    /**
     * When a service is under starting, enque the request and handle it after the service starting completes
     */
    virtual void EnqueueStartServiceReq(const AbilityRequest &abilityRequest, const std::string &serviceUri = "");
    /**
     * After the service starting completes, complete the request list
     */
    virtual void CompleteStartServiceReq(const std::string &serviceUri);

    /**
     * Post an extension's disconnect task, auto disconnect when extension connected timeout.
     */
    virtual void PostExtensionDelayDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);

    /**
     * Remove the extension's disconnect task.
     */
    virtual void RemoveExtensionDelayDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);

protected:
    virtual bool IsNeedToRestart(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const std::string &bundleName, const std::string &abilityName);
    virtual void TerminateRecord(std::shared_ptr<AbilityRecord> abilityRecord);
    virtual int DisconnectRecordNormal(ConnectListType &list, std::shared_ptr<ConnectionRecord> connectRecord,
        bool callerDied) const;
    virtual void DisconnectRecordForce(ConnectListType &list, std::shared_ptr<ConnectionRecord> connectRecord);
    virtual int TerminateAbilityInner(const sptr<IRemoteObject> &token);
    virtual bool IsLauncher(std::shared_ptr<AbilityRecord> serviceExtension) const;
    virtual void KillProcessesByUserId() const;
    virtual void SetLastExitReason(const AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &targetService);
    virtual bool IsCacheExtensionAbilityByInfo(const AppExecFwk::AbilityInfo &abilityInfo);
    virtual bool IsCacheExtensionAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);
    virtual bool IsCallerValid(const std::shared_ptr<AbilityRecord> &abilityRecord);
    virtual void HandleNotifyAssertFaultDialogDied(const std::shared_ptr<AbilityRecord> &abilityRecord);
    virtual EventInfo BuildEventInfo(const std::shared_ptr<AbilityRecord> &abilityRecord);
    virtual std::string GenerateBundleName(const AbilityRequest &abilityRequest) const;

    virtual bool AddToServiceMap(const std::string &key, std::shared_ptr<AbilityRecord> abilityRecord);
    virtual ServiceMapType GetServiceMap();

    virtual void AddConnectObjectToMap(sptr<IRemoteObject> connectObject, const ConnectListType &connectRecordList,
        bool updateOnly);

    virtual void KeepAbilityAlive(const std::shared_ptr<AbilityRecord> &abilityRecord);
    virtual void ProcessEliminateAbilityRecord(std::shared_ptr<AbilityRecord> eliminateRecord);
    virtual std::string GetServiceKey(const std::shared_ptr<AbilityRecord> &service);

    virtual int32_t ReportXiaoYiToRSSIfNeeded(const AppExecFwk::AbilityInfo &abilityInfo);
    virtual int32_t ReportAbilityStartInfoToRSS(const AppExecFwk::AbilityInfo &abilityInfo);
    virtual void ReportEventToRSS(const AppExecFwk::AbilityInfo &abilityInfo,
        const std::shared_ptr<AbilityRecord> abilityRecord, sptr<IRemoteObject> callerToken);

    virtual void SetExtensionLoadParam(AbilityRuntime::LoadParam &loadParam, std::shared_ptr<AbilityRecord> abilityRecord);
    virtual bool IsStrictMode(std::shared_ptr<AbilityRecord> abilityRecord);
    virtual bool NeedExtensionControl(std::shared_ptr<AbilityRecord> abilityRecord);
    virtual bool GetTimeoutMsgContent(uint32_t msgId, std::string &msgContent, int &typeId);

protected:
    const std::string TASK_ON_CALLBACK_DIED = "OnCallbackDiedTask";
    const std::string TASK_ON_ABILITY_DIED = "OnAbilityDiedTask";

    int userId_;
    std::atomic_uint32_t sceneBoardTokenId_ = 0;

    ConnectMapType connectMap_;
    ServiceMapType serviceMap_;
    RecipientMapType recipientMap_;
    WindowExtensionMapType windowExtensionMap_;

    std::list<std::shared_ptr<AbilityRecord>> terminatingExtensionList_;
    std::shared_ptr<TaskHandlerWrap> taskHandler_;
    std::shared_ptr<EventHandlerWrap> eventHandler_;
    std::vector<AbilityRequest> restartResidentTaskList_;
    std::unordered_map<std::string, std::shared_ptr<std::list<AbilityRequest>>> startServiceReqList_;

    ffrt::mutex serialMutex_;
    std::mutex connectMapMutex_;
    ffrt::mutex serviceMapMutex_;
    std::mutex recipientMapMutex_;
    std::mutex windowExtensionMapMutex_;
    std::mutex startServiceReqListLock_;

    DISALLOW_COPY_AND_MOVE(BaseExtensionConnectManager);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_CONNECT_MANAGER_BASE_H