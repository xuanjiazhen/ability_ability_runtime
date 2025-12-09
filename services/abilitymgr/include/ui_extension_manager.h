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

#ifndef OHOS_ABILITY_RUNTIME_UI_EXTENSION_MANAGER_H
#define OHOS_ABILITY_RUNTIME_UI_EXTENSION_MANAGER_H

#include "ability_connect_manager.h"
#include "extension_record_manager.h"
#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {

class UIExtensionManager : public AbilityConnectManager {
public:
    explicit UIExtensionManager(int userId);
    virtual ~UIExtensionManager();

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

    // UIExtensionManager 特有方法
    // int PreloadUIExtensionAbilityLocked(const AbilityRequest &abilityRequest, std::string &hostBundleName,
    //     int32_t hostPid = AAFwk::DEFAULT_INVAL_VALUE);
    // int QueryPreLoadUIExtensionRecordInner(const AppExecFwk::ElementName &element,
    //                                        const std::string &moduleName,
    //                                        const int32_t hostPid,
    //                                        int32_t &recordNum);
    // int UnloadUIExtensionAbility(const std::shared_ptr<AAFwk::AbilityRecord> &abilityRecord, pid_t &hostPid);
    // void ClearPreloadUIExtensionRecord(const std::shared_ptr<AbilityRecord> &abilityRecord);
    // int32_t GetActiveUIExtensionList(const int32_t pid, std::vector<std::string> &extensionList);
    // int32_t GetActiveUIExtensionList(const std::string &bundleName, std::vector<std::string> &extensionList);
    // void MoveToBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    // void CommandAbilityWindow(const std::shared_ptr<AbilityRecord> &abilityRecord,
    //     const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd);
    // bool IsUIExtensionFocused(uint32_t uiExtensionTokenId, const sptr<IRemoteObject>& focusToken);
    // sptr<IRemoteObject> GetUIExtensionSourceToken(const sptr<IRemoteObject> &token);
    // bool IsWindowExtensionFocused(uint32_t extensionTokenId, const sptr<IRemoteObject>& focusToken);
    // void ForegroundAbilityWindowLocked(const std::shared_ptr<AbilityRecord> &abilityRecord,
    //     const sptr<SessionInfo> &sessionInfo);
    // void BackgroundAbilityWindowLocked(const std::shared_ptr<AbilityRecord> &abilityRecord,
    //     const sptr<SessionInfo> &sessionInfo);
    // void TerminateAbilityWindowLocked(const std::shared_ptr<AbilityRecord> &abilityRecord,
    //     const sptr<SessionInfo> &sessionInfo);
    // int32_t GetUIExtensionSessionInfo(const sptr<IRemoteObject> token, UIExtensionSessionInfo &uiExtensionSessionInfo);
    // void GetUIExtensionCallerTokenList(const std::shared_ptr<AbilityRecord> &abilityRecord,
    //     std::list<sptr<IRemoteObject>> &callerList);
    // std::shared_ptr<AAFwk::AbilityRecord> GetUIExtensionRootHostInfo(const sptr<IRemoteObject> token);
    // std::shared_ptr<AbilityRecord> GetUIExtensionBySessionInfo(const sptr<SessionInfo> &sessionInfo);
    // std::shared_ptr<AbilityRecord> GetUIExtensionBySessionFromServiceMap(const sptr<SessionInfo> &sessionInfo);
    // int UnPreloadUIExtensionAbilityLocked(int32_t extensionAbilityId);
    // int UnPreloadUIExtensionAbilityInner(int32_t extensionAbilityId);
    // int ClearAllPreloadUIExtensionAbilityLocked();
    // int ClearAllPreloadUIExtensionAbilityInner();
    // int32_t RegisterPreloadUIExtensionHostClient(const sptr<IRemoteObject> &callerToken);
    // int32_t UnRegisterPreloadUIExtensionHostClient(int32_t callerPid);

private:
    // int32_t StartAbilityLocked(const AbilityRequest &abilityRequest);
    // int TerminateAbilityLocked(const sptr<IRemoteObject> &token);
    // int StopServiceAbilityLocked(const AbilityRequest &abilityRequest);
    // int DisconnectAbilityLocked(const sptr<IAbilityConnection> &connect, bool callerDied);
    // int PreloadUIExtensionAbilityInner(const AbilityRequest &abilityRequest, std::string &hostBundleName,
    //     int32_t hostPid = AAFwk::DEFAULT_INVAL_VALUE);
    // void DoForegroundUIExtension(std::shared_ptr<AbilityRecord> abilityRecord, const AbilityRequest &abilityRequest);
    // void DoBackgroundAbilityWindow(const std::shared_ptr<AbilityRecord> &abilityRecord,
    //     const sptr<SessionInfo> &sessionInfo);
    // void HandleCommandWindowTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord,
    //     const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd);
    // void HandleForegroundTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord);
    // void HandleCommandDestroy(const sptr<SessionInfo> &sessionInfo);
    // void SetLastExitReason(const AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &targetService);
    // bool IsUIExtensionAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);
    // bool CheckUIExtensionAbilitySessionExist(const std::shared_ptr<AbilityRecord> &abilityRecord);
    // void RemoveUIExtensionAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord);
    // void AddUIExtensionAbilityRecordToTerminatedList(const std::shared_ptr<AbilityRecord> &abilityRecord);
    // bool IsCallerValid(const std::shared_ptr<AbilityRecord> &abilityRecord);
    // int32_t GetOrCreateExtensionRecord(const AbilityRequest &abilityRequest, bool isCreatedByConnect,
    //     const std::string &hostBundleName, std::shared_ptr<AbilityRecord> &extensionRecord, bool &isLoaded);
    // void UpdateUIExtensionInfo(const std::shared_ptr<AbilityRecord> &abilityRecord,
    //     int32_t hostPid = AAFwk::DEFAULT_INVAL_VALUE);
    // void UpdateUIExtensionBindInfo(
    //     const std::shared_ptr<AbilityRecord> &abilityRecord, std::string callerBundleName, int32_t notifyProcessBind);
    // void AddUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session);
    // void RemoveUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session);
    // void OnUIExtWindowDied(const wptr<IRemoteObject> &remote);
    // void HandleUIExtWindowDiedTask(const sptr<IRemoteObject> &remote);
    // void HandleUIExtensionDied(const std::shared_ptr<AbilityRecord> &abilityRecord);
    // void ConnectUIServiceExtAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
    //     int connectRecordId, const Want &want);
    // void ResumeConnectAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);

    // // UIExtensionManager 特有成员变量
    // std::unique_ptr<UIExtensionAbilityConnectManager> uiExtensionAbilityRecordMgr_ = nullptr;
    // RecipientMapType uiExtRecipientMap_;
    // UIExtensionMapType uiExtensionMap_;
    // WindowExtensionMapType windowExtensionMap_;
    // std::mutex uiExtRecipientMapMutex_;
    // std::mutex uiExtensionMapMutex_;
    // std::mutex windowExtensionMapMutex_;

    // // 内部类
    // class PreloadUIExtensionHostClientDeathRecipient : public IRemoteObject::DeathRecipient {
    // public:
    //     using PreloadUIExtensionHostClientDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
    //     explicit PreloadUIExtensionHostClientDeathRecipient(PreloadUIExtensionHostClientDiedHandler handler);
    //     ~PreloadUIExtensionHostClientDeathRecipient() = default;
    //     void OnRemoteDied(const wptr<IRemoteObject> &remote) final;

    // private:
    //     PreloadUIExtensionHostClientDiedHandler diedHandler_;
    // };

    // std::mutex preloadUIExtRecipientMapMutex_;
    // std::map<int32_t, sptr<IRemoteObject::DeathRecipient>> preloadUIExtensionHostClientDeathRecipients_;

    DISALLOW_COPY_AND_MOVE(UIExtensionManager);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_UI_EXTENSION_MANAGER_H