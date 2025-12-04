/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_COMMON_EXTENSION_MANAGER_H
#define OHOS_ABILITY_RUNTIME_COMMON_EXTENSION_MANAGER_H

#include "ability_connect_manager.h"
#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
class CommonExtensionManager : public AbilityConnectManager {
public:
    explicit CommonExtensionManager(int userId);
    virtual ~CommonExtensionManager();

    // 实现基类纯虚函数
    // int32_t StartAbility(const AbilityRequest &abilityRequest) override;
    // int TerminateAbility(const sptr<IRemoteObject> &token) override;
    // int StopServiceAbility(const AbilityRequest &abilityRequest) override;
    // int32_t ConnectAbilityLocked(const AbilityRequest &abilityRequest, const sptr<IAbilityConnection> &connect,
    //     const sptr<IRemoteObject> &callerToken, sptr<SessionInfo> sessionInfo = nullptr,
    //     sptr<UIExtensionAbilityConnectInfo> connectInfo = nullptr) override;
    // int DisconnectAbilityLocked(const sptr<IAbilityConnection> &connect) override;
    // int AttachAbilityThreadLocked(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token) override;
    // int AbilityTransitionDone(const sptr<IRemoteObject> &token, int state) override;
    // int ScheduleConnectAbilityDoneLocked(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject) override;
    // int ScheduleDisconnectAbilityDoneLocked(const sptr<IRemoteObject> &token) override;
    // int ScheduleCommandAbilityDoneLocked(const sptr<IRemoteObject> &token) override;
    // void OnAbilityDied(const std::shared_ptr<AbilityRecord> &abilityRecord) override;
    // void OnTimeOut(uint32_t msgId, int64_t abilityRecordId, bool isHalf = false) override;

    // CommonExtensionManager 特有方法
    // int32_t SuspendExtensionAbilityLocked(const sptr<IAbilityConnection> &connect);
    // int32_t ResumeExtensionAbilityLocked(const sptr<IAbilityConnection> &connect);
    // void HandleExtensionDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);
    // void PostExtensionDelayDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);
    // void RemoveExtensionDelayDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);

private:
    // int32_t StartAbilityLocked(const AbilityRequest &abilityRequest);
    // int TerminateAbilityLocked(const sptr<IRemoteObject> &token);
    // int StopServiceAbilityLocked(const AbilityRequest &abilityRequest);
    // int DisconnectAbilityLocked(const sptr<IAbilityConnection> &connect, bool callerDied);
    // void GetOrCreateServiceRecord(const AbilityRequest &abilityRequest, const bool isCreatedByConnect,
    //     std::shared_ptr<AbilityRecord> &targetAbilityRecord, bool &isLoadedAbility);
    // void SetServiceAfterNewCreate(const AbilityRequest &abilityRequest, AbilityRecord &targetService);
    // void RemoveServiceFromMapSafe(const std::string &serviceKey);
    // void CompleteCommandAbility(std::shared_ptr<AbilityRecord> abilityRecord);
    // int DispatchInactive(const std::shared_ptr<AbilityRecord> &abilityRecord, int state);
    // int DispatchForeground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    // int DispatchBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    // int DispatchTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord);
    // void HandleStartTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord);
    // void HandleStopTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord);
    // void HandleTerminateDisconnectTask(const ConnectListType& connectlist);
    // void HandleCommandTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord);
    // void HandleConnectTimeoutTask(std::shared_ptr<AbilityRecord> abilityRecord);
    // void HandleRestartResidentTask(const AbilityRequest &abilityRequest);
    // void HandleActiveAbility(std::shared_ptr<AbilityRecord> &targetService,
    //     std::shared_ptr<ConnectionRecord> &connectRecord);
    // void TerminateOrCacheAbility(std::shared_ptr<AbilityRecord> abilityRecord);
    // void EnqueueStartServiceReq(const AbilityRequest &abilityRequest, const std::string &serviceUri = "");
    // void CompleteStartServiceReq(const std::string &serviceUri);
    // void HandleInactiveTimeout(const std::shared_ptr<AbilityRecord> &ability);
    // void CleanActivatingTimeoutAbility(std::shared_ptr<AbilityRecord> abilityRecord);
    // void PostRestartResidentTask(const AbilityRequest &abilityRequest);
    // void ProcessPreload(const std::shared_ptr<AbilityRecord> &record) const;
    // void TerminateRecord(std::shared_ptr<AbilityRecord> abilityRecord);
    // int DisconnectRecordNormal(ConnectListType &list, std::shared_ptr<ConnectionRecord> connectRecord,
    //     bool callerDied) const;
    // void DisconnectRecordForce(ConnectListType &list, std::shared_ptr<ConnectionRecord> connectRecord);
    // int TerminateAbilityInner(const sptr<IRemoteObject> &token);
    // std::shared_ptr<AbilityRecord> GetServiceRecordByElementName(const std::string &element);
    // std::shared_ptr<ConnectionRecord> GetAbilityConnectedRecordFromRecordList(
    //     const std::shared_ptr<AbilityRecord> &targetService,
    //     std::list<std::shared_ptr<ConnectionRecord>> &connectRecordList);
    // std::shared_ptr<AbilityRecord> GetExtensionByTokenFromServiceMap(const sptr<IRemoteObject> &token);
    // std::shared_ptr<AbilityRecord> GetExtensionByTokenFromTerminatingMap(const sptr<IRemoteObject> &token);
    // std::shared_ptr<AbilityRecord> GetExtensionByIdFromServiceMap(const int64_t &abilityRecordId);
    // std::shared_ptr<AbilityRecord> GetExtensionByIdFromTerminatingMap(const int64_t &abilityRecordId);
    // ConnectListType GetConnectRecordListByCallback(sptr<IAbilityConnection> callback);
    // std::shared_ptr<AbilityRecord> GetServiceRecordByAbilityRequest(const AbilityRequest &abilityRequest);
    // bool AddToServiceMap(const std::string &key, std::shared_ptr<AbilityRecord> abilityRecord);
    // ServiceMapType GetServiceMap();
    // int32_t GetOrCreateTargetServiceRecord(
    //     const AbilityRequest &abilityRequest, const sptr<UIExtensionAbilityConnectInfo> &connectInfo,
    //     std::shared_ptr<AbilityRecord> &targetService, bool &isLoadedAbility);
    // void SetExtensionLoadParam(AbilityRuntime::LoadParam &loadParam, std::shared_ptr<AbilityRecord> abilityRecord);
    // bool IsStrictMode(std::shared_ptr<AbilityRecord> abilityRecord); 
    // bool NeedExtensionControl(std::shared_ptr<AbilityRecord> abilityRecord);
    // bool GetTimeoutMsgContent(uint32_t msgId, std::string &msgContent, int &typeId);

    DISALLOW_COPY_AND_MOVE(CommonExtensionManager);
};

}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_COMMON_EXTENSION_MANAGER_H
#endif  // OHOS_ABILITY_RUNTIME_COMMON_EXTENSION_MANAGER_H