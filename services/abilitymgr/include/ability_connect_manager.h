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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_CONNECT_MANAGER_H
#define OHOS_ABILITY_RUNTIME_ABILITY_CONNECT_MANAGER_H

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
#include "ability_running_info.h"
#include "base_extension_record.h"
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

struct ConnectConfig {
    std::function<int(std::shared_ptr<BaseExtensionRecord>&, bool&)> getServiceFunc;
    std::function<void(std::shared_ptr<BaseExtensionRecord>&)> loadAbilityFunc;
};

/**
 * @class AbilityConnectManager
 * AbilityConnectManager provides a facility for managing service ability connection.
 */
class AbilityConnectManager : public std::enable_shared_from_this<AbilityConnectManager> {
public:
    using ConnectMapType = std::map<sptr<IRemoteObject>, std::list<std::shared_ptr<ConnectionRecord>>>;
    using ServiceMapType = std::map<std::string, std::shared_ptr<BaseExtensionRecord>>;
    using ConnectListType = std::list<std::shared_ptr<ConnectionRecord>>;
    using RecipientMapType = std::map<sptr<IRemoteObject>, sptr<IRemoteObject::DeathRecipient>>;
    using WindowExtMapValType = std::pair<uint32_t, sptr<SessionInfo>>;
    using WindowExtensionMapType = std::map<sptr<IRemoteObject>, WindowExtMapValType>;

    explicit AbilityConnectManager(int userId);
    virtual ~AbilityConnectManager();

    /**
     * StartAbility with request.
     *
     * @param abilityRequest, the request of the service ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t StartAbility(const AbilityRequest &abilityRequest);

    /**
     * TerminateAbility with token and result want.
     *
     * @param token, the token of service type's ability to terminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    int TerminateAbility(const sptr<IRemoteObject> &token);

    /**
     * StopServiceAbility with request.
     *
     * @param abilityRequest, request.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StopServiceAbility(const AbilityRequest &abilityRequest);

    /**
     * ConnectAbilityLocked, connect session with service ability.
     *
     * @param abilityRequest, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param callerToken, caller ability token.
     * @param sessionInfo the extension session info of the ability to connect.
     * @param connectInfo the connect info.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t ConnectAbilityLocked(const AbilityRequest &abilityRequest, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, sptr<SessionInfo> sessionInfo = nullptr);
    int32_t ConnectUIExtensionAbilityLocked(const AbilityRequest &abilityRequest,
        const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken,
        sptr<SessionInfo> sessionInfo = nullptr, sptr<UIExtensionAbilityConnectInfo> connectInfo = nullptr);
    int32_t ConnectAbilityCommon(const AbilityRequest &abilityRequest, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, sptr<SessionInfo> sessionInfo, const ConnectConfig &config);

    /**
     * PreloadUIExtensionAbilityInner, preload uiextension ability.
     *
     * @param abilityRequest, Special want for service type's ability.
     * @param hostBundleName, the caller application bundle name.
     * @param hostPid, the caller hostPid.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int PreloadUIExtensionAbilityInner(const AbilityRequest &abilityRequest, std::string &hostBundleName,
        int32_t hostPid = AAFwk::DEFAULT_INVAL_VALUE)
    {
            return 0;
    }

    /**
     * PreloadUIExtensionAbilityLocked, preload uiextension ability.
     *
     * @param abilityRequest, Special want for service type's ability.
     * @param hostBundleName, the caller application bundle name.
     * @param hostPid, the caller hostPid.
     * @return Returns ERR_OK on success, others on failure.
     */
    int PreloadUIExtensionAbilityLocked(const AbilityRequest &abilityRequest, std::string &hostBundleName,
        int32_t hostPid = AAFwk::DEFAULT_INVAL_VALUE);

    /**
     * Query preload uiextension record.
     *
     * @param element, The uiextension ElementName.
     * @param moduleName, The uiextension moduleName.
     * @param hostBundleName, The uiextension caller hostBundleName.
     * @param recordNum, The returned count of uiextension.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int QueryPreLoadUIExtensionRecordInner(const AppExecFwk::ElementName &element,
                                           const std::string &moduleName,
                                           const int32_t hostPid,
                                           int32_t &recordNum)
    {
        return 0;
    }

    /**
     * UnloadUIExtensionAbility, unload uiextension ability.
     *
     * @param abilityRecord, uiextension ability record.
     * @param hostBundleName, the caller application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UnloadUIExtensionAbility(const std::shared_ptr<AAFwk::BaseExtensionRecord> &abilityRecord, pid_t &hostPid)
    {
        return 0;
    }

    /**
     * ClearPreloadUIExtensionRecord, clear preload uiextension record.
     *
     * @param abilityRecord, uiextension ability record.
     */
    virtual void ClearPreloadUIExtensionRecord(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) {}

    /**
     * DisconnectAbilityLocked, disconnect session with callback.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DisconnectAbilityLocked(const sptr<IAbilityConnection> &connect);

    /**
     * SuspendExtensionAbilityLocked, suspend session with callback.
     *
     * @param connect, Callback used to notify caller the result of suspend.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t SuspendExtensionAbilityLocked(const sptr<IAbilityConnection> &connect);

    /**
     * ResumeExtensionAbilityLocked, resume session with callback.
     *
     * @param connect, Callback used to notify caller the result of resume.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t ResumeExtensionAbilityLocked(const sptr<IAbilityConnection> &connect);

    /**
     * AttachAbilityThreadLocked, ability call this interface after loaded.
     *
     * @param scheduler, the interface handler of kit ability.
     * @param token, ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int AttachAbilityThreadLocked(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token);

    virtual int AttachAbilityThreadInner(const sptr<IAbilityScheduler> &scheduler,
        const sptr<IRemoteObject> &token, const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
    {
        return 0;
    }

    virtual void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state) {}

    void OnAppStateChanged(const AppInfo &info);//暂不修改

    /**
     * AbilityTransitionDone, ability call this interface after lift cycle was changed.
     *
     * @param token, ability's token.
     * @param state, the state of ability lift cycle.
     * @return Returns ERR_OK on success, others on failure.
     */
   int AbilityTransitionDone(const sptr<IRemoteObject> &token, int state);//暂不修改

    /**
     * @brief execute after the ability schedule the lifecycle
     *
     * @param token the ability token
     * @param windowConfig the windowconfig
     * @return execute error code
     */
    int AbilityWindowConfigTransactionDone(
        const sptr<IRemoteObject> &token, const WindowConfig &windowConfig);

    /**
     * ScheduleConnectAbilityDoneLocked, service ability call this interface while session was connected.
     *
     * @param token, service ability's token.
     * @param remoteObject, the session proxy of service ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ScheduleConnectAbilityDoneLocked(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject);

    /**
     * ScheduleDisconnectAbilityDone, service ability call this interface while session was disconnected.
     *
     * @param token,service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ScheduleDisconnectAbilityDoneLocked(const sptr<IRemoteObject> &token);//抽个函数
    /**
     * ScheduleCommandAbilityDoneLocked, service ability call this interface while session was onCommanded.
     *
     * @param token,service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ScheduleCommandAbilityDoneLocked(const sptr<IRemoteObject> &token);

    int ScheduleCommandAbilityWindowDone(
        const sptr<IRemoteObject> &token,
        const sptr<SessionInfo> &sessionInfo,
        WindowCommand winCmd,
        AbilityCommand abilityCmd);

    /**
     * GetUIExtensionBySessionInfo.
     *
     * @param sessionToken, service ability's session token.
     * @return Returns AbilityRecord shared_ptr.
     */
    virtual std::shared_ptr<BaseExtensionRecord> GetUIExtensionBySessionInfo(const sptr<SessionInfo> &sessionInfo)
    {
        return nullptr;
    }

    std::shared_ptr<BaseExtensionRecord> GetExtensionByTokenFromServiceMap(const sptr<IRemoteObject> &token);
    std::shared_ptr<BaseExtensionRecord> GetExtensionByTokenFromTerminatingMap(const sptr<IRemoteObject> &token);
    std::shared_ptr<BaseExtensionRecord> GetExtensionByIdFromServiceMap(const int64_t &abilityRecordId);
    std::shared_ptr<BaseExtensionRecord> GetExtensionByIdFromTerminatingMap(const int64_t &abilityRecordId);
    std::shared_ptr<BaseExtensionRecord> GetUIExtensionBySessionFromServiceMap(const sptr<SessionInfo> &sessionInfo);//暂不修改
    ConnectListType GetConnectRecordListByCallback(sptr<IAbilityConnection> callback);

    void GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info,
        const int32_t userId, bool isPerm);//外层需要查对应userId两个manager，待修改

    void GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm);//外层需要查对应userId两个manager，待修改

    void GetExtensionRunningInfo(std::shared_ptr<BaseExtensionRecord> &abilityRecord, const int32_t userId,
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

    /**
     * @brief Get extensionList by pid.
     * @param pid Process id.
     * @param extensionList UIExtensionAbility name list.
     */
    virtual int32_t GetActiveUIExtensionList(const int32_t pid, std::vector<std::string> &extensionList)
    {
        return 0;
    }

    /**
     * @brief Get extensionList by bundleName.
     * @param bundleName The application bundle name.
     * @param extensionList UIExtensionAbility name list.
     */
    virtual int32_t GetActiveUIExtensionList(const std::string &bundleName, std::vector<std::string> &extensionList)
    {
        return 0;
    }

    void OnLoadAbilityFailed(std::shared_ptr<BaseExtensionRecord> abilityRecord);

    /**
     * OnAbilityDied.
     *
     * @param abilityRecord, service ability record.
     */
    void OnAbilityDied(const std::shared_ptr<BaseExtensionRecord> abilityRecord);

    /**
     * DisconnectBeforeCleanup.
     *
     */
    void DisconnectBeforeCleanup();

    void DumpState(std::vector<std::string> &info, bool isClient, const std::string &args = "");//dump 需修改

    void DumpStateByUri(std::vector<std::string> &info, bool isClient, const std::string &args,
        std::vector<std::string> &params);//dump 需修改

    void PauseExtensions();

    void OnTimeOut(uint32_t msgId, int64_t abilityRecordId, bool isHalf = false);

    /**
     * @brief schedule to background
     *
     * @param abilityRecord the ability to move
     */
    void MoveToBackground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//基类保留，待确认

    void CommandAbilityWindow(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd);

    virtual bool IsUIExtensionFocused(uint32_t uiExtensionTokenId, const sptr<IRemoteObject>& focusToken)    
    {
        return true;
    }

    virtual sptr<IRemoteObject> GetUIExtensionSourceToken(const sptr<IRemoteObject> &token)
    {
        return nullptr;
    }

    bool IsWindowExtensionFocused(uint32_t extensionTokenId, const sptr<IRemoteObject>& focusToken);

    virtual void HandleProcessFrozen(const std::vector<int32_t> &pidList, int32_t uid) {}

    void BackgroundAbilityWindowLocked(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);

    void TerminateAbilityWindowLocked(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);

    void RemoveLauncherDeathRecipient();

    /**
     * @brief Get ui extension session info
     *
     * @param token The ability token.
     * @param uiExtensionSessionInfo The ui extension session info.
     * @param userId The user id.
     * @return int32_t Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetUIExtensionSessionInfo(const sptr<IRemoteObject> token, UIExtensionSessionInfo &uiExtensionSessionInfo)
    {
        return 0;
    }

    virtual void GetUIExtensionCallerTokenList(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        std::list<sptr<IRemoteObject>> &callerList) {}

    void CloseAssertDialog(const std::string &assertSessionId);

    void SignRestartAppFlag(int32_t uid, const std::string &instanceKey);
    void SignRestartProcess(int32_t pid);

    virtual std::shared_ptr<AAFwk::AbilityRecord> GetUIExtensionRootHostInfo(const sptr<IRemoteObject> token)
    {
        return nullptr;
    }
    void UninstallApp(const std::string &bundleName, int32_t uid);

    int32_t UpdateKeepAliveEnableState(const std::string &bundleName, const std::string &moduleName,
        const std::string &mainElement, bool updateEnable);

    /**
     * Handle extension disconnect task.
     */
    void HandleExtensionDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);

    /**
     * GetServiceRecordByAbilityRequest.
     *
     * @param abilityRequest, the request of the service ability to start.
     * @return Returns BaseExtensionRecord shared_ptr.
     */
    std::shared_ptr<BaseExtensionRecord> GetServiceRecordByAbilityRequest(const AbilityRequest &abilityRequest);

    bool HasRequestIdInLoadAbilityQueue(int32_t requestId);//special额外确认下，uiExtension
    void OnStartSpecifiedProcessResponse(const std::string &flag, int32_t requestId);//special额外确认下，uiExtension
    void OnStartSpecifiedProcessTimeoutResponse(int32_t requestId);//special额外确认下，uiExtension
    void StartSpecifiedProcess(const LoadAbilityContext &context,
        const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//special额外确认下，uiExtension
    int UnPreloadUIExtensionAbilityLocked(int32_t extensionAbilityId);
    virtual int UnPreloadUIExtensionAbilityInner(int32_t extensionAbilityId)
    {
        return 0;
    }
    int ClearAllPreloadUIExtensionAbilityLocked();
    virtual int ClearAllPreloadUIExtensionAbilityInner()
    {
        return 0;
    }
    virtual int32_t RegisterPreloadUIExtensionHostClient(const sptr<IRemoteObject> &callerToken)
    {
        return 0;
    }
    virtual int32_t UnRegisterPreloadUIExtensionHostClient(int32_t callerPid)
    {
        return 0;
    }

protected:
    bool AddToServiceMap(const std::string &key, std::shared_ptr<BaseExtensionRecord> abilityRecord);       

    /**
     * LoadAbility.
     *
     * @param abilityRecord, the ptr of the ability to load.
     */
    void LoadAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        std::function<void(const std::shared_ptr<BaseExtensionRecord> &)> updateRecordCallback = nullptr);
    int32_t ReportXiaoYiToRSSIfNeeded(const AppExecFwk::AbilityInfo &abilityInfo);
    int32_t ReportAbilityStartInfoToRSS(const AppExecFwk::AbilityInfo &abilityInfo);
    void ReportEventToRSS(const AppExecFwk::AbilityInfo &abilityInfo,
        const std::shared_ptr<BaseExtensionRecord> abilityRecord, sptr<IRemoteObject> callerToken);
    /**
     * When a service is under starting, enque the request and handle it after the service starting completes
     */
    void EnqueueStartServiceReq(const AbilityRequest &abilityRequest, const std::string &serviceUri = "");
private:
    /**
     * StartAbilityLocked with request.
     *
     * @param abilityRequest, the request of the service ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t StartAbilityLocked(const AbilityRequest &abilityRequest)
    {
        return 0;
    }

    /**
     * TerminateAbilityLocked with token and result want.
     *
     * @param token, the token of service type's ability to terminate.
     * @param resultCode, the result code of service type's ability to terminate.
     * @param resultWant, the result want for service type's ability to terminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    int TerminateAbilityLocked(const sptr<IRemoteObject> &token);//局部下沉子类

    /**
     * StopAbilityLocked with request.
     *
     * @param abilityRequest, the request of the service ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StopServiceAbilityLocked(const AbilityRequest &abilityRequest);

    /**
     * DisconnectAbilityLocked, disconnect session with callback.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param callerDied, bool Indicates if it is caused by the caller's death.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DisconnectAbilityLocked(const sptr<IAbilityConnection> &connect, bool callerDied);

    void HandleLoadAbilityOrStartSpecifiedProcess(
        const AbilityRuntime::LoadParam &loadParam, const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//子类重写
    /**
     * ConnectAbility.Schedule connect ability
     *
     * @param abilityRecord, the ptr of the ability to connect.
     */
    void ConnectAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    /**
     * ConnectAbility.Schedule connect ability
     *
     * @param abilityRecord, the ptr of the ability to connect.
     */
    void ConnectUIServiceExtAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        int connectRecordId, const Want &want);

    /**
     * ConnectAbility.Schedule Resume Connect ability
     *
     * @param abilityRecord, the ptr of the ability to connect.
     */
    void ResumeConnectAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    /**
     * CommandAbility. Schedule command ability
     *
     * @param abilityRecord, the ptr of the ability to command.
     */
    void CommandAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    /**
     * CompleteCommandAbility. complete command ability
     *
     * @param abilityRecord, the ptr of the ability to command.
     */
    void CompleteCommandAbility(std::shared_ptr<BaseExtensionRecord> abilityRecord);

    /**
     * TerminateDone.
     *
     * @param abilityRecord, the ptr of the ability to terminate.
     */
    void TerminateDone(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//局部抽个函数下沉

    /**
     * GetServiceRecordByElementName.
     *
     * @param element, service ability's element.
     * @return Returns BaseExtensionRecord shared_ptr.
     */
    std::shared_ptr<BaseExtensionRecord> GetServiceRecordByElementName(const std::string &element);

    /**
     * GetAbilityConnectedRecordFromRecordList.
     *
     * @param targetService, the ptr of the connected ability.
     * @param connectRecordList, connect record list.
     * @return Returns ConnectionRecord shared_ptr.
     */
    std::shared_ptr<ConnectionRecord> GetAbilityConnectedRecordFromRecordList(
        const std::shared_ptr<BaseExtensionRecord> &targetService,
        std::list<std::shared_ptr<ConnectionRecord>> &connectRecordList);

    /**
     * dispatch service ability life cycle .
     *
     * @param abilityRecord.
     * @param state.
     */
    int DispatchInactive(const std::shared_ptr<BaseExtensionRecord> &abilityRecord, int state);//部分子类
    int DispatchForeground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//uiExtension
    int DispatchBackground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//uiExtension
    int DispatchTerminate(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//基类

    void HandleStartTimeoutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//uiExtension部分子类
    void HandleStopTimeoutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//uiExtension部分子类
    void HandleTerminateDisconnectTask(const ConnectListType& connectlist);
    void HandleCommandTimeoutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);
    void HandleCommandWindowTimeoutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd);
    void HandleForegroundTimeoutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//uiExtension
    void HandleConnectTimeoutTask(std::shared_ptr<BaseExtensionRecord> abilityRecord);
    void HandleRestartResidentTask(const AbilityRequest &abilityRequest);
    void HandleActiveAbility(std::shared_ptr<BaseExtensionRecord> &targetService,
        std::shared_ptr<ConnectionRecord> &connectRecord);
    void HandleCommandDestroy(const sptr<SessionInfo> &sessionInfo);//uiExtension部分子类
    void TerminateOrCacheAbility(std::shared_ptr<BaseExtensionRecord> abilityRecord);

    /**
     * RemoveConnectionRecordFromMap.
     *
     * @param connect, the ptr of the connect record.
     */
    void RemoveConnectionRecordFromMap(std::shared_ptr<ConnectionRecord> connect);

    /**
     * RemoveServiceAbility.
     *
     * @param service, the ptr of the ability record.
     */
    void RemoveServiceAbility(const std::shared_ptr<BaseExtensionRecord> &service);

    /**
     * GetOrCreateServiceRecord.
     *
     * @param abilityRequest, Special want for service type's ability.
     * @param isCreatedByConnect, whether is created by connect ability mode.
     * @param targetAbilityRecord, the target service ability record.
     * @param isLoadedAbility, whether the target ability has been loaded.
     */
    void GetOrCreateServiceRecord(const AbilityRequest &abilityRequest, const bool isCreatedByConnect,
        std::shared_ptr<BaseExtensionRecord> &targetAbilityRecord, bool &isLoadedAbility);//非uiExtension
    void SetServiceAfterNewCreate(const AbilityRequest &abilityRequest, BaseExtensionRecord &targetService);//service独有

    void RemoveServiceFromMapSafe(const std::string &serviceKey);

    /**
     * GetConnectRecordListFromMap.
     *
     * @param connect, callback object.
     * @param isCreatedByConnect, whether is created by connect ability mode.
     * @param connectRecordList, the target connectRecordList.
     * @param isCallbackConnected, whether the callback has been connected.
     */
    void GetConnectRecordListFromMap(
        const sptr<IAbilityConnection> &connect, std::list<std::shared_ptr<ConnectionRecord>> &connectRecordList);

    /**
     * AddConnectDeathRecipient.
     *
     * @param connect, callback object.
     */
    void AddConnectDeathRecipient(sptr<IRemoteObject> connectObject);

    /**
     * RemoteConnectDeathRecipient.
     *
     * @param connect, callback object.
     */
    void RemoveConnectDeathRecipient(sptr<IRemoteObject> connectObject);

    /**
     * RemoteConnectDeathRecipient.
     *
     * @param remote, callback object.
     */
    void OnCallBackDied(const wptr<IRemoteObject> &remote);

    /**
     * HandleOnCallBackDied.
     *
     * @param connect, callback object.
     */
    void HandleCallBackDiedTask(const sptr<IRemoteObject> &connect);

    /**
     * HandleOnCallBackDied.
     *
     * @param abilityRecord, died ability.
     */
    void HandleAbilityDiedTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//uiExtension部分下沉

    virtual void HandleUIExtensionDied(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) {}//uiExtension

    void RestartAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord, int32_t currentUserId);

    /**
     * PostTimeOutTask.
     *
     * @param abilityRecord, ability.
     * @param messageId, message id.
     */
    void PostTimeOutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord, uint32_t messageId);
    void PostTimeOutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord, int connectRecordId,
        uint32_t messageId);

    void CompleteForeground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//uiExtension
    void CompleteBackground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//uiExtension
    void PrintTimeOutLog(const std::shared_ptr<BaseExtensionRecord> &ability, uint32_t msgId, bool isHalf = false);//暂时不改

    void PostRestartResidentTask(const AbilityRequest &abilityRequest);

    bool IsAbilityNeedKeepAlive(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    void ProcessPreload(const std::shared_ptr<BaseExtensionRecord> &record) const;

    void HandleInactiveTimeout(const std::shared_ptr<BaseExtensionRecord> &ability);
    void CleanActivatingTimeoutAbility(std::shared_ptr<BaseExtensionRecord> abilityRecord);//uiExtension部分下沉
    void MoveToTerminatingMap(const std::shared_ptr<BaseExtensionRecord>& abilityRecord);

    void DoForegroundUIExtension(std::shared_ptr<BaseExtensionRecord> abilityRecord,
        const AbilityRequest &abilityRequest);//uiExtension
    virtual void DoBackgroundAbilityWindow(const std::shared_ptr<BaseExtensionRecord> &abilityRecord, 
        const sptr<SessionInfo> &sessionInfo) {}
    /**
     * After the service starting completes, complete the request list
     */
    void CompleteStartServiceReq(const std::string &serviceUri);

    virtual void AddUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session) {}//uiExtension
    virtual void RemoveUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session) {}//uiExtension
    virtual void OnUIExtWindowDied(const wptr<IRemoteObject> &remote) {}//uiExtension
    virtual void HandleUIExtWindowDiedTask(const sptr<IRemoteObject> &remote) {}//uiExtension

    /**
     * Post an extension's disconnect task, auto disconnect when extension connected timeout.
     */
    void PostExtensionDelayDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);

    /**
     * Remove the extension's disconnect task.
     */
    void RemoveExtensionDelayDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);

private:
    bool IsNeedToRestart(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const std::string &bundleName, const std::string &abilityName);
    void TerminateRecord(std::shared_ptr<BaseExtensionRecord> abilityRecord);
    int DisconnectRecordNormal(ConnectListType &list, std::shared_ptr<ConnectionRecord> connectRecord,
        bool callerDied) const;
    void DisconnectRecordForce(ConnectListType &list, std::shared_ptr<ConnectionRecord> connectRecord);
    int TerminateAbilityInner(const sptr<IRemoteObject> &token);//uiExtension部分重写
    bool IsLauncher(std::shared_ptr<BaseExtensionRecord> serviceExtension) const;
    void KillProcessesByUserId() const;
    virtual void SetLastExitReason(const AbilityRequest &abilityRequest, std::shared_ptr<BaseExtensionRecord> &targetService) {}//uiExtension
    virtual bool IsUIExtensionAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
    {
        return true;
    }//uiExtension
    inline bool IsCacheExtensionAbilityByInfo(const AppExecFwk::AbilityInfo &abilityInfo);
    inline bool IsCacheExtensionAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);
    virtual bool CheckUIExtensionAbilitySessionExist(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
    {
        return true;
    }//uiExtension
    virtual void RemoveUIExtensionAbilityRecord(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) {}//uiExtension
    virtual void AddUIExtensionAbilityRecordToTerminatedList(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) {}//uiExtension
    virtual bool IsCallerValid(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
    {
        return true;
    }//uiExtension
    virtual int32_t GetOrCreateExtensionRecord(const AbilityRequest &abilityRequest, bool isCreatedByConnect,
        const std::string &hostBundleName, std::shared_ptr<BaseExtensionRecord> &extensionRecord, bool &isLoaded)
    {
        return 0;
    }//uiExtension部分下沉
    virtual int32_t GetOrCreateTargetServiceRecord(
        const AbilityRequest &abilityRequest, const sptr<UIExtensionAbilityConnectInfo> &connectInfo,
        std::shared_ptr<BaseExtensionRecord> &targetService, bool &isLoadedAbility)
    {
        return 0;
    }//重写，跟随connectAbilityLocked分离
    void HandleNotifyAssertFaultDialogDied(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);
    EventInfo BuildEventInfo(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);
    virtual void UpdateUIExtensionInfo(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        int32_t hostPid = AAFwk::DEFAULT_INVAL_VALUE) {} //uiExtension
    std::string GenerateBundleName(const AbilityRequest &abilityRequest) const;

    ServiceMapType GetServiceMap();

    void AddConnectObjectToMap(sptr<IRemoteObject> connectObject, const ConnectListType &connectRecordList,
        bool updateOnly);

    void KeepAbilityAlive(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);
    void ProcessEliminateAbilityRecord(std::shared_ptr<BaseExtensionRecord> eliminateRecord);
    std::string GetServiceKey(const std::shared_ptr<BaseExtensionRecord> &service);

    void SetExtensionLoadParam(AbilityRuntime::LoadParam &loadParam,
        std::shared_ptr<BaseExtensionRecord> abilityRecord);
    bool IsStrictMode(std::shared_ptr<BaseExtensionRecord> abilityRecord);
    bool NeedExtensionControl(std::shared_ptr<BaseExtensionRecord> abilityRecord);
    bool GetTimeoutMsgContent(uint32_t msgId, std::string &msgContent, int &typeId);

    virtual void UpdateUIExtensionBindInfo(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        std::string callerBundleName, int32_t notifyProcessBind) {}//uiExtension

    void HandleConnectionCountIncrement(int32_t pid, const std::string &callerBundleName,
        const std::string &targetName);
    void DecrementConnectionCountAndCleanup(int32_t pid);
private:
    const std::string TASK_ON_CALLBACK_DIED = "OnCallbackDiedTask";
    const std::string TASK_ON_ABILITY_DIED = "OnAbilityDiedTask";

    int userId_;
    std::atomic_uint32_t sceneBoardTokenId_ = 0;

    ConnectMapType connectMap_;
    ServiceMapType serviceMap_;
    RecipientMapType recipientMap_;
    WindowExtensionMapType windowExtensionMap_;
    std::map<int32_t, int32_t> callerPidConnectionCountMap_;

    std::list<std::shared_ptr<BaseExtensionRecord>> terminatingExtensionList_;
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
    std::mutex callerPidConnectionCountMapMutex_;
    std::vector<int32_t> thresholds_ = {50, 100, 200, 500};

    DISALLOW_COPY_AND_MOVE(AbilityConnectManager);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_CONNECT_MANAGER_H
