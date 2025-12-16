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

#ifndef OHOS_ABILITY_RUNTIME_UI_EXTENSION_ABILITY_MANAGER_H
#define OHOS_ABILITY_RUNTIME_UI_EXTENSION_ABILITY_MANAGER_H

#include "ability_connect_manager.h"

#include "extension_record_manager.h"
#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {

using UIExtensionAbilityConnectManager = AbilityRuntime::ExtensionRecordManager;
using UIExtensionSessionInfo = AbilityRuntime::UIExtensionSessionInfo;

/**
 * @class UIExtensionAbilityManager
 * UIExtensionAbilityManager provides facility for managing UI extension ability connection.
 */
class UIExtensionAbilityManager : public AbilityConnectManager {
public:
    using UIExtWindowMapValType = std::pair<std::weak_ptr<BaseExtensionRecord>, sptr<SessionInfo>>;
    using UIExtensionMapType = std::map<sptr<IRemoteObject>, UIExtWindowMapValType>;

    struct LoadAbilityContext {
        std::shared_ptr<AbilityRuntime::LoadParam> loadParam;
        std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo;
        std::shared_ptr<AppExecFwk::ApplicationInfo> appInfo;
        std::shared_ptr<AAFwk::Want> want;
    };

    explicit UIExtensionAbilityManager(int userId);
    virtual ~UIExtensionAbilityManager();
    
    int PreloadUIExtensionAbilityInner(const AbilityRequest &abilityRequest, std::string &hostBundleName,
        int32_t hostPid = AAFwk::DEFAULT_INVAL_VALUE) override;

    int QueryPreLoadUIExtensionRecordInner(const AppExecFwk::ElementName &element,
                                           const std::string &moduleName,
                                           const int32_t hostPid,
                                           int32_t &recordNum) override;

    int UnloadUIExtensionAbility(const std::shared_ptr<AAFwk::BaseExtensionRecord> &abilityRecord, pid_t &hostPid) override;

    void ClearPreloadUIExtensionRecord(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;

    int AttachAbilityThreadInner(const sptr<IAbilityScheduler> &scheduler,
        const sptr<IRemoteObject> &token, const std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;

    void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state) override;

    std::shared_ptr<BaseExtensionRecord> GetUIExtensionBySessionInfo(const sptr<SessionInfo> &sessionInfo) override;

    int32_t GetActiveUIExtensionList(const int32_t pid, std::vector<std::string> &extensionList) override;

    int32_t GetActiveUIExtensionList(const std::string &bundleName, std::vector<std::string> &extensionList) override;

    // void CommandAbilityWindow(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
    //     const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd);

    bool IsUIExtensionFocused(uint32_t uiExtensionTokenId, const sptr<IRemoteObject>& focusToken) override;

    sptr<IRemoteObject> GetUIExtensionSourceToken(const sptr<IRemoteObject> &token) override;

    int32_t GetUIExtensionSessionInfo(const sptr<IRemoteObject> token, UIExtensionSessionInfo &uiExtensionSessionInfo) override;

    void GetUIExtensionCallerTokenList(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        std::list<sptr<IRemoteObject>> &callerList) override;

    std::shared_ptr<AAFwk::AbilityRecord> GetUIExtensionRootHostInfo(const sptr<IRemoteObject> token) override;

    bool HasRequestIdInLoadAbilityQueue(int32_t requestId) override;
    void OnStartSpecifiedProcessResponse(const std::string &flag, int32_t requestId) override;
    void OnStartSpecifiedProcessTimeoutResponse(int32_t requestId) override;
    void StartSpecifiedProcess(const LoadAbilityContext &context, const std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;

    int UnPreloadUIExtensionAbilityInner(int32_t extensionAbilityId) override;
    int ClearAllPreloadUIExtensionAbilityInner() override;
    int32_t RegisterPreloadUIExtensionHostClient(const sptr<IRemoteObject> &callerToken) override;
    int32_t UnRegisterPreloadUIExtensionHostClient(int32_t callerPid) override;

private:
    int32_t StartAbilityLocked(const AbilityRequest &abilityRequest) override;

    void SetLastExitReason(const AbilityRequest &abilityRequest, std::shared_ptr<BaseExtensionRecord> &targetService) override;

    bool IsCallerValid(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;
    
    void HandleLoadAbilityOrStartSpecifiedProcess(
        const AbilityRuntime::LoadParam &loadParam, const std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;

    void HandleUIExtensionDied(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;

    void DoForegroundUIExtension(std::shared_ptr<BaseExtensionRecord> abilityRecord, const AbilityRequest &abilityRequest) override;

    void DoBackgroundAbilityWindow(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo) override;

    void AddUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session) override;

    void RemoveUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session) override;

    void OnUIExtWindowDied(const wptr<IRemoteObject> &remote) override;

    void HandleUIExtWindowDiedTask(const sptr<IRemoteObject> &remote) override;

    bool IsUIExtensionAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;

    bool CheckUIExtensionAbilitySessionExist(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;

    void RemoveUIExtensionAbilityRecord(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;

    void AddUIExtensionAbilityRecordToTerminatedList(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;

    int32_t GetOrCreateExtensionRecord(const AbilityRequest &abilityRequest, bool isCreatedByConnect,
        const std::string &hostBundleName, std::shared_ptr<BaseExtensionRecord> &extensionRecord, bool &isLoaded) override;

    int32_t GetOrCreateTargetServiceRecord(
        const AbilityRequest &abilityRequest, const sptr<UIExtensionAbilityConnectInfo> &connectInfo,
        std::shared_ptr<BaseExtensionRecord> &targetService, bool &isLoadedAbility) override;

    void UpdateUIExtensionInfo(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        int32_t hostPid = AAFwk::DEFAULT_INVAL_VALUE) override;

    void UpdateUIExtensionBindInfo(
        const std::shared_ptr<BaseExtensionRecord> &abilityRecord, std::string callerBundleName, int32_t notifyProcessBind) override;
    
    class PreloadUIExtensionHostClientDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        using PreloadUIExtensionHostClientDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
        explicit PreloadUIExtensionHostClientDeathRecipient(PreloadUIExtensionHostClientDiedHandler handler);
        ~PreloadUIExtensionHostClientDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) final;

    private:
        PreloadUIExtensionHostClientDiedHandler diedHandler_;
    };    
private:
    RecipientMapType uiExtRecipientMap_;
    UIExtensionMapType uiExtensionMap_;
    std::unique_ptr<UIExtensionAbilityConnectManager> uiExtensionAbilityRecordMgr_ = nullptr;
    std::mutex uiExtRecipientMapMutex_;
    std::mutex uiExtensionMapMutex_;
    std::mutex loadAbilityQueueLock_;
    std::deque<std::map<int32_t, LoadAbilityContext>> loadAbilityQueue_;
    std::mutex preloadUIExtRecipientMapMutex_;
    std::map<int32_t, sptr<IRemoteObject::DeathRecipient>> preloadUIExtensionHostClientDeathRecipients_;

    DISALLOW_COPY_AND_MOVE(UIExtensionAbilityManager);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_UI_EXTENSION_ABILITY_MANAGER_H