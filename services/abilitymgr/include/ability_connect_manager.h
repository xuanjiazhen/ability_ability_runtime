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
#include "extension_record_manager.h"
#include "want.h"
#include "iremote_object.h"
#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
using OHOS::AppExecFwk::AbilityType;
using UIExtensionAbilityConnectInfo = AbilityRuntime::UIExtensionAbilityConnectInfo;//uiExtension
//using UIExtensionAbilityConnectManager = AbilityRuntime::ExtensionRecordManager; //需从基类删除
using UIExtensionSessionInfo = AbilityRuntime::UIExtensionSessionInfo;
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
    //using UIExtWindowMapValType = std::pair<std::weak_ptr<BaseExtensionRecord>, sptr<SessionInfo>>; //需从基类删除
    //using UIExtensionMapType = std::map<sptr<IRemoteObject>, UIExtWindowMapValType>; //需从基类删除
    using WindowExtMapValType = std::pair<uint32_t, sptr<SessionInfo>>;
    using WindowExtensionMapType = std::map<sptr<IRemoteObject>, WindowExtMapValType>;

    explicit AbilityConnectManager(int userId);
    virtual ~AbilityConnectManager();

    // struct LoadAbilityContext {//需从基类删除
    //     std::shared_ptr<AbilityRuntime::LoadParam> loadParam;
    //     std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo;
    //     std::shared_ptr<AppExecFwk::ApplicationInfo> appInfo;
    //     std::shared_ptr<AAFwk::Want> want;
    // };

    /**
     * StartAbility with request.
     *
     * @param abilityRequest, the request of the service ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t StartAbility(const AbilityRequest &abilityRequest);//不需要修改

    /**
     * TerminateAbility with token and result want.
     *
     * @param token, the token of service type's ability to terminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    int TerminateAbility(const sptr<IRemoteObject> &token);//不需要修改

    /**
     * StopServiceAbility with request.
     *
     * @param abilityRequest, request.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StopServiceAbility(const AbilityRequest &abilityRequest);//待与宇航确认

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
    virtual int32_t ConnectAbilityLocked(const AbilityRequest &abilityRequest, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, sptr<SessionInfo> sessionInfo = nullptr,
        sptr<UIExtensionAbilityConnectInfo> connectInfo = nullptr);//uiExtension和serviceExtension分开，uiExtension下沉子类，再新增的函数

    virtual int32_t ConnectUIExtensionAbilityLocked(const AbilityRequest &abilityRequest, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, sptr<SessionInfo> sessionInfo = nullptr,
        sptr<UIExtensionAbilityConnectInfo> connectInfo = nullptr);//uiExtension和serviceExtension分开，uiExtension下沉子类，再新增的函数

    /**
     * PreloadUIExtensionAbilityInner, preload uiextension ability.
     *
     * @param abilityRequest, Special want for service type's ability.
     * @param hostBundleName, the caller application bundle name.
     * @param hostPid, the caller hostPid.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int PreloadUIExtensionAbilityInner(const AbilityRequest &abilityRequest, std::string &hostBundleName,
        int32_t hostPid = AAFwk::DEFAULT_INVAL_VALUE);//UIEXTENSION,普通虚函数

    /**
     * PreloadUIExtensionAbilityLocked, preload uiextension ability.
     *
     * @param abilityRequest, Special want for service type's ability.
     * @param hostBundleName, the caller application bundle name.
     * @param hostPid, the caller hostPid.
     * @return Returns ERR_OK on success, others on failure.
     */
    int PreloadUIExtensionAbilityLocked(const AbilityRequest &abilityRequest, std::string &hostBundleName,
        int32_t hostPid = AAFwk::DEFAULT_INVAL_VALUE);//UIEXTENSION,普通虚函数

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
                                           int32_t &recordNum);//UIEXTENSION,普通虚函数

    /**
     * UnloadUIExtensionAbility, unload uiextension ability.
     *
     * @param abilityRecord, uiextension ability record.
     * @param hostBundleName, the caller application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UnloadUIExtensionAbility(const std::shared_ptr<AAFwk::BaseExtensionRecord> &abilityRecord, pid_t &hostPid);//UIEXTENSION,普通虚函数

    /**
     * ClearPreloadUIExtensionRecord, clear preload uiextension record.
     *
     * @param abilityRecord, uiextension ability record.
     */
    virtual void ClearPreloadUIExtensionRecord(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//UIEXTENSION,普通虚函数

    /**
     * DisconnectAbilityLocked, disconnect session with callback.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DisconnectAbilityLocked(const sptr<IAbilityConnection> &connect);//不需要修改

    /**
     * SuspendExtensionAbilityLocked, suspend session with callback.
     *
     * @param connect, Callback used to notify caller the result of suspend.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t SuspendExtensionAbilityLocked(const sptr<IAbilityConnection> &connect);//不需要修改

    /**
     * ResumeExtensionAbilityLocked, resume session with callback.
     *
     * @param connect, Callback used to notify caller the result of resume.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t ResumeExtensionAbilityLocked(const sptr<IAbilityConnection> &connect);//不需要修改

    /**
     * AttachAbilityThreadLocked, ability call this interface after loaded.
     *
     * @param scheduler, the interface handler of kit ability.
     * @param token, ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AttachAbilityThreadLocked(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token);//麻烦玩意，锁基类，子类重写

    virtual void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state);//UIEXTENSION,普通虚函数

    void OnAppStateChanged(const AppInfo &info);//待确认是否需修改，暂定不改

    /**
     * AbilityTransitionDone, ability call this interface after lift cycle was changed.
     *
     * @param token, ability's token.
     * @param state, the state of ability lift cycle.
     * @return Returns ERR_OK on success, others on failure.
     */
   virtual int AbilityTransitionDone(const sptr<IRemoteObject> &token, int state);//子类manager重写

    /**
     * @brief execute after the ability schedule the lifecycle
     *
     * @param token the ability token
     * @param windowConfig the windowconfig
     * @return execute error code
     */
    int AbilityWindowConfigTransactionDone(
        const sptr<IRemoteObject> &token, const WindowConfig &windowConfig);//windowExtension 忽略不修改

    /**
     * ScheduleConnectAbilityDoneLocked, service ability call this interface while session was connected.
     *
     * @param token, service ability's token.
     * @param remoteObject, the session proxy of service ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ScheduleConnectAbilityDoneLocked(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject);//基类不修改

    /**
     * ScheduleDisconnectAbilityDone, service ability call this interface while session was disconnected.
     *
     * @param token,service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ScheduleDisconnectAbilityDoneLocked(const sptr<IRemoteObject> &token);//基类，暂不修改

    /**
     * ScheduleCommandAbilityDoneLocked, service ability call this interface while session was onCommanded.
     *
     * @param token,service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ScheduleCommandAbilityDoneLocked(const sptr<IRemoteObject> &token);//基类

    int ScheduleCommandAbilityWindowDone(
        const sptr<IRemoteObject> &token,
        const sptr<SessionInfo> &sessionInfo,
        WindowCommand winCmd,
        AbilityCommand abilityCmd);//基类

    /**
     * GetUIExtensionBySessionInfo.
     *
     * @param sessionToken, service ability's session token.
     * @return Returns AbilityRecord shared_ptr.
     */
    virtual std::shared_ptr<BaseExtensionRecord> GetUIExtensionBySessionInfo(const sptr<SessionInfo> &sessionInfo);//uiExtension

    std::shared_ptr<BaseExtensionRecord> GetExtensionByTokenFromServiceMap(const sptr<IRemoteObject> &token);//基类
    std::shared_ptr<BaseExtensionRecord> GetExtensionByTokenFromTerminatingMap(const sptr<IRemoteObject> &token);//基类
    std::shared_ptr<BaseExtensionRecord> GetExtensionByIdFromServiceMap(const int64_t &abilityRecordId);//基类
    std::shared_ptr<BaseExtensionRecord> GetExtensionByIdFromTerminatingMap(const int64_t &abilityRecordId);//基类
    std::shared_ptr<BaseExtensionRecord> GetUIExtensionBySessionFromServiceMap(const sptr<SessionInfo> &sessionInfo);//uiExtension
    ConnectListType GetConnectRecordListByCallback(sptr<IAbilityConnection> callback);//基类

    void GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info,
        const int32_t userId, bool isPerm);//外层需要查对应userId两个manager

    void GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm);//外层需要查对应userId两个manager

    void GetExtensionRunningInfo(std::shared_ptr<BaseExtensionRecord> &abilityRecord, const int32_t userId,
        std::vector<ExtensionRunningInfo> &info);//基类，无需修改

    /**
     * set from ability manager service for sequenced task
     */
    inline void SetTaskHandler(const std::shared_ptr<TaskHandlerWrap> &taskHandler)//基类
    {
        taskHandler_ = taskHandler;
    }
    /**
     * SetEventHandler.
     *
     * @param handler,EventHandler
     */
    inline void SetEventHandler(const std::shared_ptr<EventHandlerWrap> &handler)//基类
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
    virtual int32_t GetActiveUIExtensionList(const int32_t pid, std::vector<std::string> &extensionList);//uiExtension

    /**
     * @brief Get extensionList by bundleName.
     * @param bundleName The application bundle name.
     * @param extensionList UIExtensionAbility name list.
     */
    virtual int32_t GetActiveUIExtensionList(const std::string &bundleName, std::vector<std::string> &extensionList);//uiExtension

    void OnLoadAbilityFailed(std::shared_ptr<BaseExtensionRecord> abilityRecord);//基类

    /**
     * OnAbilityDied.
     *
     * @param abilityRecord, service ability record.
     */
    void OnAbilityDied(const std::shared_ptr<BaseExtensionRecord> abilityRecord);//基类

    /**
     * DisconnectBeforeCleanup.
     *
     */
    void DisconnectBeforeCleanup();//基类

    void DumpSttae(std::vector<std::string> &info, bool isClient, const std::string &args = "");//基类

    void DumpStateByUri(std::vector<std::string> &info, bool isClient, const std::string &args,
        std::vector<std::string> &params);//基类

    void PauseExtensions();//??干啥子的

    void OnTimeOut(uint32_t msgId, int64_t abilityRecordId, bool isHalf = false);//基类

    /**
     * @brief schedule to background
     *
     * @param abilityRecord the ability to move
     */
    virtual void MoveToBackground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//uiExtension

    virtual void CommandAbilityWindow(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd);//uiExtension

    virtual bool IsUIExtensionFocused(uint32_t uiExtensionTokenId, const sptr<IRemoteObject>& focusToken);//uiExtension

    virtual sptr<IRemoteObject> GetUIExtensionSourceToken(const sptr<IRemoteObject> &token);//uiExtension

    bool IsWindowExtensionFocused(uint32_t extensionTokenId, const sptr<IRemoteObject>& focusToken);//基类保留不修改

    void HandleProcessFrozen(const std::vector<int32_t> &pidList, int32_t uid);//待确认功能

    virtual void ForegroundAbilityWindowLocked(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);//uiExtension

    virtual void BackgroundAbilityWindowLocked(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);//uiExtension

    void TerminateAbilityWindowLocked(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);//基类，无需修改

    void RemoveLauncherDeathRecipient();//基类

    /**
     * @brief Get ui extension session info
     *
     * @param token The ability token.
     * @param uiExtensionSessionInfo The ui extension session info.
     * @param userId The user id.
     * @return int32_t Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetUIExtensionSessionInfo(const sptr<IRemoteObject> token, UIExtensionSessionInfo &uiExtensionSessionInfo);//uiExtension

    virtual void GetUIExtensionCallerTokenList(const std::shared_ptr<AbilityRecord> &abilityRecord,
        std::list<sptr<IRemoteObject>> &callerList);//uiExtension

    void CloseAssertDialog(const std::string &assertSessionId);//什么东西？？

    void SignRestartAppFlag(int32_t uid, const std::string &instanceKey);//什么东西？？
    void SignRestartProcess(int32_t pid);//什么东西？？

    virtual std::shared_ptr<AAFwk::AbilityRecord> GetUIExtensionRootHostInfo(const sptr<IRemoteObject> token);//uiExtension
    void UninstallApp(const std::string &bundleName, int32_t uid);//基类，无需修改

    int32_t UpdateKeepAliveEnableState(const std::string &bundleName, const std::string &moduleName,
        const std::string &mainElement, bool updateEnable);//基类，无需修改

    /**
     * Handle extension disconnect task.
     */
    void HandleExtensionDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);//基类，无需修改

    /**
     * GetServiceRecordByAbilityRequest.
     *
     * @param abilityRequest, the request of the service ability to start.
     * @return Returns BaseExtensionRecord shared_ptr.
     */
    std::shared_ptr<BaseExtensionRecord> GetServiceRecordByAbilityRequest(const AbilityRequest &abilityRequest);//基类，无需修改

    bool HasRequestIdInLoadAbilityQueue(int32_t requestId);//uiExtension
    void OnStartSpecifiedProcessResponse(const std::string &flag, int32_t requestId);//uiExtension
    void OnStartSpecifiedProcessTimeoutResponse(int32_t requestId);//uiExtension
    void StartSpecifiedProcess(const LoadAbilityContext &context,
        const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//uiExtension
    virtual int UnPreloadUIExtensionAbilityLocked(int32_t extensionAbilityId);//uiExtension
    virtual int UnPreloadUIExtensionAbilityInner(int32_t extensionAbilityId);//uiExtension
    virtual int ClearAllPreloadUIExtensionAbilityLocked();//uiExtension
    virtual int ClearAllPreloadUIExtensionAbilityInner();//uiExtension
    virtual int32_t RegisterPreloadUIExtensionHostClient(const sptr<IRemoteObject> &callerToken);//uiExtension
    virtual int32_t UnRegisterPreloadUIExtensionHostClient(int32_t callerPid);//uiExtension

private:
    /**
     * StartAbilityLocked with request.
     *
     * @param abilityRequest, the request of the service ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t StartAbilityLocked(const AbilityRequest &abilityRequest);//子类重写

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
    int StopServiceAbilityLocked(const AbilityRequest &abilityRequest);//基类

    /**
     * DisconnectAbilityLocked, disconnect session with callback.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param callerDied, bool Indicates if it is caused by the caller's death.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DisconnectAbilityLocked(const sptr<IAbilityConnection> &connect, bool callerDied);//基类

    /**
     * LoadAbility.
     *
     * @param abilityRecord, the ptr of the ability to load.
     */
    void LoadAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        std::function<void(const std::shared_ptr<BaseExtensionRecord> &)> updateRecordCallback = nullptr);//基类，不改
    void HandleLoadAbilityOrStartSpecifiedProcess(
        const AbilityRuntime::LoadParam &loadParam, const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//子类重写
    /**
     * ConnectAbility.Schedule connect ability
     *
     * @param abilityRecord, the ptr of the ability to connect.
     */
    void ConnectAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//基类

    /**
     * ConnectAbility.Schedule connect ability
     *
     * @param abilityRecord, the ptr of the ability to connect.
     */
    void ConnectUIServiceExtAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        int connectRecordId, const Want &want);//基类

    /**
     * ConnectAbility.Schedule Resume Connect ability
     *
     * @param abilityRecord, the ptr of the ability to connect.
     */
    void ResumeConnectAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//基类

    /**
     * CommandAbility. Schedule command ability
     *
     * @param abilityRecord, the ptr of the ability to command.
     */
    void CommandAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//基类

    /**
     * CompleteCommandAbility. complete command ability
     *
     * @param abilityRecord, the ptr of the ability to command.
     */
    void CompleteCommandAbility(std::shared_ptr<BaseExtensionRecord> abilityRecord);//基类

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
    std::shared_ptr<BaseExtensionRecord> GetServiceRecordByElementName(const std::string &element);//基类

    /**
     * GetAbilityConnectedRecordFromRecordList.
     *
     * @param targetService, the ptr of the connected ability.
     * @param connectRecordList, connect record list.
     * @return Returns ConnectionRecord shared_ptr.
     */
    std::shared_ptr<ConnectionRecord> GetAbilityConnectedRecordFromRecordList(
        const std::shared_ptr<BaseExtensionRecord> &targetService,
        std::list<std::shared_ptr<ConnectionRecord>> &connectRecordList);//基类

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
    void HandleTerminateDisconnectTask(const ConnectListType& connectlist);//基类
    void HandleCommandTimeoutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//基类
    void HandleCommandWindowTimeoutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd);//基类
    void HandleForegroundTimeoutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//uiExtension
    void HandleConnectTimeoutTask(std::shared_ptr<BaseExtensionRecord> abilityRecord);//基类
    void HandleRestartResidentTask(const AbilityRequest &abilityRequest);//基类
    void HandleActiveAbility(std::shared_ptr<BaseExtensionRecord> &targetService,
        std::shared_ptr<ConnectionRecord> &connectRecord);//基类
    void HandleCommandDestroy(const sptr<SessionInfo> &sessionInfo);//uiExtension部分子类
    void TerminateOrCacheAbility(std::shared_ptr<BaseExtensionRecord> abilityRecord);//基类

    /**
     * RemoveConnectionRecordFromMap.
     *
     * @param connect, the ptr of the connect record.
     */
    void RemoveConnectionRecordFromMap(std::shared_ptr<ConnectionRecord> connect);//基类

    /**
     * RemoveServiceAbility.
     *
     * @param service, the ptr of the ability record.
     */
    void RemoveServiceAbility(const std::shared_ptr<BaseExtensionRecord> &service);//基类

    /**
     * GetOrCreateServiceRecord.
     *
     * @param abilityRequest, Special want for service type's ability.
     * @param isCreatedByConnect, whether is created by connect ability mode.
     * @param targetAbilityRecord, the target service ability record.
     * @param isLoadedAbility, whether the target ability has been loaded.
     */
    void GetOrCreateServiceRecord(const AbilityRequest &abilityRequest, const bool isCreatedByConnect,
        std::shared_ptr<BaseExtensionRecord> &targetAbilityRecord, bool &isLoadedAbility);//基类
    void SetServiceAfterNewCreate(const AbilityRequest &abilityRequest, BaseExtensionRecord &targetService);//基类

    void RemoveServiceFromMapSafe(const std::string &serviceKey);//基类

    /**
     * GetConnectRecordListFromMap.
     *
     * @param connect, callback object.
     * @param isCreatedByConnect, whether is created by connect ability mode.
     * @param connectRecordList, the target connectRecordList.
     * @param isCallbackConnected, whether the callback has been connected.
     */
    void GetConnectRecordListFromMap(
        const sptr<IAbilityConnection> &connect, std::list<std::shared_ptr<ConnectionRecord>> &connectRecordList);//基类

    /**
     * AddConnectDeathRecipient.
     *
     * @param connect, callback object.
     */
    void AddConnectDeathRecipient(sptr<IRemoteObject> connectObject);//基类

    /**
     * RemoteConnectDeathRecipient.
     *
     * @param connect, callback object.
     */
    void RemoveConnectDeathRecipient(sptr<IRemoteObject> connectObject);//基类

    /**
     * RemoteConnectDeathRecipient.
     *
     * @param remote, callback object.
     */
    void OnCallBackDied(const wptr<IRemoteObject> &remote);//基类

    /**
     * HandleOnCallBackDied.
     *
     * @param connect, callback object.
     */
    void HandleCallBackDiedTask(const sptr<IRemoteObject> &connect);//基类

    /**
     * HandleOnCallBackDied.
     *
     * @param abilityRecord, died ability.
     */
    void HandleAbilityDiedTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//uiExtension部分下沉

    void HandleUIExtensionDied(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//uiExtension

    void RestartAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord, int32_t currentUserId);//基类

    /**
     * PostTimeOutTask.
     *
     * @param abilityRecord, ability.
     * @param messageId, message id.
     */
    void PostTimeOutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord, uint32_t messageId);//基类
    void PostTimeOutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord, int connectRecordId,
        uint32_t messageId);//基类

    void CompleteForeground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//uiExtension
    void CompleteBackground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//uiExtension
    void PrintTimeOutLog(const std::shared_ptr<BaseExtensionRecord> &ability, uint32_t msgId, bool isHalf = false);//基类，先不动

    void PostRestartResidentTask(const AbilityRequest &abilityRequest);//基类

    bool IsAbilityNeedKeepAlive(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//基类

    void ProcessPreload(const std::shared_ptr<BaseExtensionRecord> &record) const;//基类

    void HandleInactiveTimeout(const std::shared_ptr<BaseExtensionRecord> &ability);//基类
    void CleanActivatingTimeoutAbility(std::shared_ptr<BaseExtensionRecord> abilityRecord);//uiExtension部分下沉
    void MoveToTerminatingMap(const std::shared_ptr<BaseExtensionRecord>& abilityRecord);//基类

    void DoForegroundUIExtension(std::shared_ptr<BaseExtensionRecord> abilityRecord,
        const AbilityRequest &abilityRequest);//uiExtension
    void DoBackgroundAbilityWindow(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);//uiExtension

    /**
     * When a service is under starting, enque the request and handle it after the service starting completes
     */
    void EnqueueStartServiceReq(const AbilityRequest &abilityRequest, const std::string &serviceUri = "");//基类
    /**
     * After the service starting completes, complete the request list
     */
    void CompleteStartServiceReq(const std::string &serviceUri);//基类

    void AddUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session);//uiExtension
    void RemoveUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session);//uiExtension
    void OnUIExtWindowDied(const wptr<IRemoteObject> &remote);//uiExtension
    void HandleUIExtWindowDiedTask(const sptr<IRemoteObject> &remote);//uiExtension

    /**
     * Post an extension's disconnect task, auto disconnect when extension connected timeout.
     */
    void PostExtensionDelayDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);//基类

    /**
     * Remove the extension's disconnect task.
     */
    void RemoveExtensionDelayDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);//基类

private:
    bool IsNeedToRestart(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const std::string &bundleName, const std::string &abilityName);//基类
    void TerminateRecord(std::shared_ptr<BaseExtensionRecord> abilityRecord);//基类
    int DisconnectRecordNormal(ConnectListType &list, std::shared_ptr<ConnectionRecord> connectRecord,
        bool callerDied) const;//基类
    void DisconnectRecordForce(ConnectListType &list, std::shared_ptr<ConnectionRecord> connectRecord);//基类
    int TerminateAbilityInner(const sptr<IRemoteObject> &token);//uiExtension部分重写
    bool IsLauncher(std::shared_ptr<BaseExtensionRecord> serviceExtension) const;//基类
    void KillProcessesByUserId() const;//基类
    void SetLastExitReason(const AbilityRequest &abilityRequest, std::shared_ptr<BaseExtensionRecord> &targetService);//uiExtension
    inline bool IsUIExtensionAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//uiExtension
    inline bool IsCacheExtensionAbilityByInfo(const AppExecFwk::AbilityInfo &abilityInfo);//基类
    inline bool IsCacheExtensionAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//基类
    inline bool CheckUIExtensionAbilityLoaded(const AbilityRequest &abilityRequest);//没有实现
    inline bool CheckUIExtensionAbilitySessionExist(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//uiExtension
    inline void RemoveUIExtensionAbilityRecord(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//uiExtension
    inline void AddUIExtensionAbilityRecordToTerminatedList(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//uiExtension
    inline bool IsCallerValid(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//uiExtension
    int32_t GetOrCreateExtensionRecord(const AbilityRequest &abilityRequest, bool isCreatedByConnect,
        const std::string &hostBundleName, std::shared_ptr<BaseExtensionRecord> &extensionRecord, bool &isLoaded);//uiExtension部分下沉
    int32_t GetOrCreateTargetServiceRecord(
        const AbilityRequest &abilityRequest, const sptr<UIExtensionAbilityConnectInfo> &connectInfo,
        std::shared_ptr<BaseExtensionRecord> &targetService, bool &isLoadedAbility);//重写，跟随connectAbilityLocked分离
    void HandleNotifyAssertFaultDialogDied(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//基类
    EventInfo BuildEventInfo(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//基类
    virtual void UpdateUIExtensionInfo(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        int32_t hostPid = AAFwk::DEFAULT_INVAL_VALUE);//uiExtension
    std::string GenerateBundleName(const AbilityRequest &abilityRequest) const;//基类

    bool AddToServiceMap(const std::string &key, std::shared_ptr<BaseExtensionRecord> abilityRecord);//基类
    ServiceMapType GetServiceMap();//基类

    void AddConnectObjectToMap(sptr<IRemoteObject> connectObject, const ConnectListType &connectRecordList,
        bool updateOnly);//基类

    void KeepAbilityAlive(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);//基类
    void ProcessEliminateAbilityRecord(std::shared_ptr<BaseExtensionRecord> eliminateRecord);//基类
    std::string GetServiceKey(const std::shared_ptr<BaseExtensionRecord> &service);//基类

    int32_t ReportXiaoYiToRSSIfNeeded(const AppExecFwk::AbilityInfo &abilityInfo);//基类
    int32_t ReportAbilityStartInfoToRSS(const AppExecFwk::AbilityInfo &abilityInfo);//基类
    void ReportEventToRSS(const AppExecFwk::AbilityInfo &abilityInfo,
        const std::shared_ptr<BaseExtensionRecord> abilityRecord, sptr<IRemoteObject> callerToken);//基类

    void SetExtensionLoadParam(AbilityRuntime::LoadParam &loadParam,
        std::shared_ptr<BaseExtensionRecord> abilityRecord);//基类
    bool IsStrictMode(std::shared_ptr<BaseExtensionRecord> abilityRecord);//基类
    bool NeedExtensionControl(std::shared_ptr<BaseExtensionRecord> abilityRecord);//基类
    bool GetTimeoutMsgContent(uint32_t msgId, std::string &msgContent, int &typeId);//基类

    virtual void UpdateUIExtensionBindInfo(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        std::string callerBundleName, int32_t notifyProcessBind);//uiExtension

    void HandleConnectionCountIncrement(int32_t pid, const std::string &callerBundleName,
        const std::string &targetName);//基类
    void DecrementConnectionCountAndCleanup(int32_t pid);//基类
    //uiExtension
    // class PreloadUIExtensionHostClientDeathRecipient : public IRemoteObject::DeathRecipient {
    // public:
    //     using PreloadUIExtensionHostClientDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
    //     explicit PreloadUIExtensionHostClientDeathRecipient(PreloadUIExtensionHostClientDiedHandler handler);
    //     ~PreloadUIExtensionHostClientDeathRecipient() = default;
    //     void OnRemoteDied(const wptr<IRemoteObject> &remote) final;

    // private:
    //     PreloadUIExtensionHostClientDiedHandler diedHandler_;
    // };
private:
    const std::string TASK_ON_CALLBACK_DIED = "OnCallbackDiedTask";
    const std::string TASK_ON_ABILITY_DIED = "OnAbilityDiedTask";

    int userId_;
    std::atomic_uint32_t sceneBoardTokenId_ = 0;

    ConnectMapType connectMap_;
    ServiceMapType serviceMap_;
    RecipientMapType recipientMap_;
    //RecipientMapType uiExtRecipientMap_;//需从基类删除
    //UIExtensionMapType uiExtensionMap_;//需从基类删除
    WindowExtensionMapType windowExtensionMap_;
    std::map<int32_t, int32_t> callerPidConnectionCountMap_;

    std::list<std::shared_ptr<BaseExtensionRecord>> terminatingExtensionList_;
    std::shared_ptr<TaskHandlerWrap> taskHandler_;
    std::shared_ptr<EventHandlerWrap> eventHandler_;
    std::vector<AbilityRequest> restartResidentTaskList_;
    std::unordered_map<std::string, std::shared_ptr<std::list<AbilityRequest>>> startServiceReqList_;

    //std::unique_ptr<UIExtensionAbilityConnectManager> uiExtensionAbilityRecordMgr_ = nullptr; //需从基类删除
    ffrt::mutex serialMutex_;
    std::mutex connectMapMutex_;
    ffrt::mutex serviceMapMutex_;
    std::mutex recipientMapMutex_;
    //std::mutex uiExtRecipientMapMutex_;//需从基类删除，uiExtension
    //std::mutex uiExtensionMapMutex_;//需从基类删除，uiExtension
    std::mutex windowExtensionMapMutex_;
    std::mutex startServiceReqListLock_;
    //std::mutex loadAbilityQueueLock_;//uiExtension
    std::mutex callerPidConnectionCountMapMutex_;
    //std::mutex preloadUIExtRecipientMapMutex_;//需从基类删除
    //std::deque<std::map<int32_t, LoadAbilityContext>> loadAbilityQueue_;//需从基类删除
    std::vector<int32_t> thresholds_ = {50, 100, 200, 500};
    //std::map<int32_t, sptr<IRemoteObject::DeathRecipient>> preloadUIExtensionHostClientDeathRecipients_;//uiExtension

    DISALLOW_COPY_AND_MOVE(AbilityConnectManager);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_CONNECT_MANAGER_H
