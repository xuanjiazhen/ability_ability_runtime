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

#ifndef OHOS_ABILITY_RUNTIME_UI_EXTENSION_ABILITY_CONNECT_MANAGER_H
#define OHOS_ABILITY_RUNTIME_UI_EXTENSION_ABILITY_CONNECT_MANAGER_H

#include "base_extension_connect_manager.h"
#include "ui_extension/ui_extension_ability_connect_info.h"

namespace OHOS {
namespace AAFwk {

using UIExtensionAbilityConnectInfo = AbilityRuntime::UIExtensionAbilityConnectInfo;
using UIExtensionAbilityConnectManager = AbilityRuntime::ExtensionRecordManager;
using UIExtensionSessionInfo = AbilityRuntime::UIExtensionSessionInfo;

/**
 * @class UIExtensionAbilityConnectManager
 * UIExtensionAbilityConnectManager provides facility for managing UI extension ability connection.
 */
class UIExtensionAbilityConnectManager : public AbilityConnectManagerBase {
public:
    using UIExtWindowMapValType = std::pair<std::weak_ptr<AbilityRecord>, sptr<SessionInfo>>;
    using UIExtensionMapType = std::map<sptr<IRemoteObject>, UIExtWindowMapValType>;

    struct LoadAbilityContext {
        std::shared_ptr<AbilityRuntime::LoadParam> loadParam;
        std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo;
        std::shared_ptr<AppExecFwk::ApplicationInfo> appInfo;
        std::shared_ptr<AAFwk::Want> want;
    };

    explicit UIExtensionAbilityConnectManager(int userId);
    virtual ~UIExtensionAbilityConnectManager();

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

    void ClearPreloadUIExtensionRecord(const std::shared_ptr<AbilityRecord> &abilityRecord);

    std::shared_ptr<AbilityRecord> GetUIExtensionBySessionInfo(const sptr<SessionInfo> &sessionInfo);

    std::shared_ptr<AbilityRecord> GetUIExtensionBySessionFromServiceMap(const sptr<SessionInfo> &sessionInfo);

    int32_t GetActiveUIExtensionList(const int32_t pid, std::vector<std::string> &extensionList);

    int32_t GetActiveUIExtensionList(const std::string &bundleName, std::vector<std::string> &extensionList);

    bool IsUIExtensionFocused(uint32_t uiExtensionTokenId, const sptr<IRemoteObject>& focusToken);

    sptr<IRemoteObject> GetUIExtensionSourceToken(const sptr<IRemoteObject> &token);

    int32_t GetUIExtensionSessionInfo(const sptr<IRemoteObject> token, UIExtensionSessionInfo &uiExtensionSessionInfo);

    void GetUIExtensionCallerTokenList(const std::shared_ptr<AbilityRecord> &abilityRecord,
        std::list<sptr<IRemoteObject>> &callerList);

    std::shared_ptr<AAFwk::AbilityRecord> GetUIExtensionRootHostInfo(const sptr<IRemoteObject> token);

    bool HasRequestIdInLoadAbilityQueue(int32_t requestId);
    void OnStartSpecifiedProcessResponse(const std::string &flag, int32_t requestId);
    void OnStartSpecifiedProcessTimeoutResponse(int32_t requestId);
    void StartSpecifiedProcess(const LoadAbilityContext &context, const std::shared_ptr<AbilityRecord> &abilityRecord);

protected:
    void LoadAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
        std::function<void(const std::shared_ptr<AbilityRecord> &)> updateRecordCallback = nullptr) override;

    void HandleLoadAbilityOrStartSpecifiedProcess(
        const AbilityRuntime::LoadParam &loadParam, const std::shared_ptr<AbilityRecord> &abilityRecord);

    void ConnectUIServiceExtAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
        int connectRecordId, const Want &want);

    void HandleUIExtensionDied(const std::shared_ptr<AbilityRecord> &abilityRecord);

    void DoForegroundUIExtension(std::shared_ptr<AbilityRecord> abilityRecord, const AbilityRequest &abilityRequest);

    void DoBackgroundAbilityWindow(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);

    void AddUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session);

    void RemoveUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session);

    void OnUIExtWindowDied(const wptr<IRemoteObject> &remote);

    void HandleUIExtWindowDiedTask(const sptr<IRemoteObject> &remote);

    bool IsUIExtensionAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);

    bool CheckUIExtensionAbilityLoaded(const AbilityRequest &abilityRequest);

    bool CheckUIExtensionAbilitySessionExist(const std::shared_ptr<AbilityRecord> &abilityRecord);

    void RemoveUIExtensionAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord);

    void AddUIExtensionAbilityRecordToTerminatedList(const std::shared_ptr<AbilityRecord> &abilityRecord);

    int32_t GetOrCreateExtensionRecord(const AbilityRequest &abilityRequest, bool isCreatedByConnect,
        const std::string &hostBundleName, std::shared_ptr<AbilityRecord> &extensionRecord, bool &isLoaded);

    int32_t GetOrCreateTargetServiceRecord(
        const AbilityRequest &abilityRequest, const sptr<UIExtensionAbilityConnectInfo> &connectInfo,
        std::shared_ptr<AbilityRecord> &targetService, bool &isLoadedAbility);

    void UpdateUIExtensionInfo(const std::shared_ptr<AbilityRecord> &abilityRecord,
        int32_t hostPid = AAFwk::DEFAULT_INVAL_VALUE);

    void UpdateUIExtensionBindInfo(
        const std::shared_ptr<AbilityRecord> &abilityRecord, std::string callerBundleName, int32_t notifyProcessBind);

private:
    RecipientMapType uiExtRecipientMap_;
    UIExtensionMapType uiExtensionMap_;
    std::unique_ptr<UIExtensionAbilityConnectManager> uiExtensionAbilityRecordMgr_ = nullptr;
    std::mutex uiExtRecipientMapMutex_;
    std::mutex uiExtensionMapMutex_;
    std::mutex loadAbilityQueueLock_;
    std::deque<std::map<int32_t, LoadAbilityContext>> loadAbilityQueue_;

    DISALLOW_COPY_AND_MOVE(UIExtensionAbilityConnectManager);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_UI_EXTENSION_ABILITY_CONNECT_MANAGER_H