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

#ifndef OHOS_ABILITY_RUNTIME_UI_EXTENSION_ABILITY_MANAGER_H
#define OHOS_ABILITY_RUNTIME_UI_EXTENSION_ABILITY_MANAGER_H

#include "ability_connect_manager.h"
#include "ui_extension/ui_extension_ability_connect_info.h"

namespace OHOS {
namespace AAFwk {

using UIExtensionAbilityConnectManager = AbilityRuntime::ExtensionRecordManager;

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

    // UIExtension specific methods
    int32_t ConnectAbilityLocked(const AbilityRequest &abilityRequest, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, sptr<SessionInfo> sessionInfo = nullptr,
        sptr<UIExtensionAbilityConnectInfo> connectInfo = nullptr) override;

    int PreloadUIExtensionAbilityInner(const AbilityRequest &abilityRequest, std::string &hostBundleName,
        int32_t hostPid = AAFwk::DEFAULT_INVAL_VALUE);

    int PreloadUIExtensionAbilityLocked(const AbilityRequest &abilityRequest, std::string &hostBundleName,
        int32_t hostPid = AAFwk::DEFAULT_INVAL_VALUE);

    int QueryPreLoadUIExtensionRecordInner(const AppExecFwk::ElementName &element,
                                           const std::string &moduleName,
                                           const std::string &hostBundleName,
                                           int32_t &recordNum);

    int UnloadUIExtensionAbility(const std::shared_ptr<AAFwk::AbilityRecord> &abilityRecord, std::string &bundleName);

    void ClearPreloadUIExtensionRecord(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    std::shared_ptr<BaseExtensionRecord> GetUIExtensionBySessionInfo(const sptr<SessionInfo> &sessionInfo);

    std::shared_ptr<BaseExtensionRecord> GetUIExtensionBySessionFromServiceMap(const sptr<SessionInfo> &sessionInfo);

    int32_t GetActiveUIExtensionList(const int32_t pid, std::vector<std::string> &extensionList);

    int32_t GetActiveUIExtensionList(const std::string &bundleName, std::vector<std::string> &extensionList);

    bool IsUIExtensionFocused(uint32_t uiExtensionTokenId, const sptr<IRemoteObject>& focusToken);

    sptr<IRemoteObject> GetUIExtensionSourceToken(const sptr<IRemoteObject> &token);

    int32_t GetUIExtensionSessionInfo(const sptr<IRemoteObject> token, UIExtensionSessionInfo &uiExtensionSessionInfo);

    void GetUIExtensionCallerTokenList(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        std::list<sptr<IRemoteObject>> &callerList);

    std::shared_ptr<AAFwk::AbilityRecord> GetUIExtensionRootHostInfo(const sptr<IRemoteObject> token);

    bool HasRequestIdInLoadAbilityQueue(int32_t requestId);
    void OnStartSpecifiedProcessResponse(const std::string &flag, int32_t requestId);
    void OnStartSpecifiedProcessTimeoutResponse(int32_t requestId);
    void StartSpecifiedProcess(const LoadAbilityContext &context, const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

private:
    void LoadAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        std::function<void(const std::shared_ptr<BaseExtensionRecord> &)> updateRecordCallback = nullptr) override;

    void HandleLoadAbilityOrStartSpecifiedProcess(
        const AbilityRuntime::LoadParam &loadParam, const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    void ConnectUIServiceExtAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        int connectRecordId, const Want &want);

    void HandleUIExtensionDied(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    void DoForegroundUIExtension(std::shared_ptr<BaseExtensionRecord> abilityRecord, const AbilityRequest &abilityRequest);

    void DoBackgroundAbilityWindow(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);

    void AddUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session);

    void RemoveUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session);

    void OnUIExtWindowDied(const wptr<IRemoteObject> &remote);

    void HandleUIExtWindowDiedTask(const sptr<IRemoteObject> &remote);

    bool IsUIExtensionAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    bool CheckUIExtensionAbilityLoaded(const AbilityRequest &abilityRequest);

    bool CheckUIExtensionAbilitySessionExist(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    void RemoveUIExtensionAbilityRecord(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    void AddUIExtensionAbilityRecordToTerminatedList(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    int32_t GetOrCreateExtensionRecord(const AbilityRequest &abilityRequest, bool isCreatedByConnect,
        const std::string &hostBundleName, std::shared_ptr<BaseExtensionRecord> &extensionRecord, bool &isLoaded);

    int32_t GetOrCreateTargetServiceRecord(
        const AbilityRequest &abilityRequest, const sptr<UIExtensionAbilityConnectInfo> &connectInfo,
        std::shared_ptr<BaseExtensionRecord> &targetService, bool &isLoadedAbility);

    void UpdateUIExtensionInfo(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        int32_t hostPid = AAFwk::DEFAULT_INVAL_VALUE);

    void UpdateUIExtensionBindInfo(
        const std::shared_ptr<BaseExtensionRecord> &abilityRecord, std::string callerBundleName, int32_t notifyProcessBind);
    
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