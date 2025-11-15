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

#ifndef OHOS_ABILITY_RUNTIME_UI_EXTENSION_CONNECT_MANAGER_H
#define OHOS_ABILITY_RUNTIME_UI_EXTENSION_CONNECT_MANAGER_H

#include "base_extension_connect_manager.h"
#include "extension_record_manager.h"

namespace OHOS {
namespace AAFwk {
using UIExtensionAbilityConnectManager = AbilityRuntime::ExtensionRecordManager;

/**
 * @class UIExtensionConnectManager
 * UIExtensionConnectManager provides facility for managing UIExtension connection.
 */
class UIExtensionConnectManager : public BaseExtensionConnectManager {
public:
    explicit UIExtensionConnectManager(int userId);
    ~UIExtensionConnectManager() override;

    /**
     * PreloadUIExtensionAbilityInner, preload uiextension ability.
     *
     * @param abilityRequest, Special want for uiextension ability.
     * @param hostBundleName, the caller application bundle name.
     * @param hostPid, the caller process id.
     * @return Returns ERR_OK on success, others on failure.
     */
    int PreloadUIExtensionAbilityInner(const AbilityRequest &abilityRequest, std::string &hostBundleName,
        int32_t hostPid = AAFwk::DEFAULT_INVAL_VALUE);

    /**
     * PreloadUIExtensionAbilityLocked, preload uiextension ability.
     *
     * @param abilityRequest, Special want for uiextension ability.
     * @param hostBundleName, the caller application bundle name.
     * @param hostPid, the caller process id.
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
    int QueryPreLoadUIExtensionRecordInner(const AppExecFwk::ElementName &element,
                                           const std::string &moduleName,
                                           const std::string &hostBundleName,
                                           int32_t &recordNum);

    /**
     * UnloadUIExtensionAbility, unload uiextension ability.
     *
     * @param abilityRecord, uiextension ability record.
     * @param bundleName, the caller application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    int UnloadUIExtensionAbility(const std::shared_ptr<AAFwk::AbilityRecord> &abilityRecord, std::string &bundleName);

    /**
     * ClearPreloadUIExtensionRecord, clear preload uiextension record.
     *
     * @param abilityRecord, uiextension ability record.
     */
    void ClearPreloadUIExtensionRecord(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * GetUIExtensionBySessionInfo.
     *
     * @param sessionToken, uiextension's session token.
     * @return Returns AbilityRecord shared_ptr.
     */
    std::shared_ptr<AbilityRecord> GetUIExtensionBySessionInfo(const sptr<SessionInfo> &sessionInfo);

    /**
     * GetUIExtensionBySessionFromServiceMap.
     *
     * @param sessionInfo, session info.
     * @return Returns AbilityRecord shared_ptr.
     */
    std::shared_ptr<AbilityRecord> GetUIExtensionBySessionFromServiceMap(const sptr<SessionInfo> &sessionInfo);

    /**
     * @brief Get active uiextension list by pid.
     * @param pid Process id.
     * @param extensionList UIExtensionAbility name list.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t GetActiveUIExtensionList(const int32_t pid, std::vector<std::string> &extensionList);

    /**
     * @brief Get active uiextension list by bundleName.
     * @param bundleName The application bundle name.
     * @param extensionList UIExtensionAbility name list.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t GetActiveUIExtensionList(const std::string &bundleName, std::vector<std::string> &extensionList);

    /**
     * Check if uiextension is focused.
     *
     * @param uiExtensionTokenId, uiextension token id.
     * @param focusToken, focus token.
     * @return Returns true if focused, false otherwise.
     */
    bool IsUIExtensionFocused(uint32_t uiExtensionTokenId, const sptr<IRemoteObject>& focusToken);

    /**
     * Check if window extension is focused.
     *
     * @param extensionTokenId, extension token id.
     * @param focusToken, focus token.
     * @return Returns true if focused, false otherwise.
     */
    bool IsWindowExtensionFocused(uint32_t extensionTokenId, const sptr<IRemoteObject>& focusToken);

    /**
     * Get uiextension source token.
     *
     * @param token, uiextension token.
     * @return Returns IRemoteObject shared_ptr.
     */
    sptr<IRemoteObject> GetUIExtensionSourceToken(const sptr<IRemoteObject> &token);

    /**
     * ForegroundAbilityWindowLocked.
     *
     * @param abilityRecord, uiextension record.
     * @param sessionInfo, session info.
     */
    void ForegroundAbilityWindowLocked(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);

    /**
     * BackgroundAbilityWindowLocked.
     *
     * @param abilityRecord, uiextension record.
     * @param sessionInfo, session info.
     */
    void BackgroundAbilityWindowLocked(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo) override;

    /**
     * TerminateAbilityWindowLocked.
     *
     * @param abilityRecord, uiextension record.
     * @param sessionInfo, session info.
     */
    void TerminateAbilityWindowLocked(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);

    /**
     * @brief Get ui extension session info
     *
     * @param token The ability token.
     * @param uiExtensionSessionInfo The ui extension session info.
     * @param userId The user id.
     * @return int32_t Returns ERR_OK on success, others on failure.
     */
    int32_t GetUIExtensionSessionInfo(const sptr<IRemoteObject> token, UIExtensionSessionInfo &uiExtensionSessionInfo);

    /**
     * UpdateUIExtensionBindInfo.
     *
     * @param abilityRecord, uiextension record.
     * @param callerBundleName, caller bundle name.
     * @param notifyProcessBind, notify process bind.
     */
    void UpdateUIExtensionBindInfo(
        const std::shared_ptr<AbilityRecord> &abilityRecord, std::string callerBundleName, int32_t notifyProcessBind);

    /**
     * ConnectUIServiceExtAbility.
     *
     * @param abilityRecord, uiextension record.
     * @param connectRecordId, connect record id.
     * @param want, want info.
     */
    void ConnectUIServiceExtAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
        int connectRecordId, const Want &want);

    /**
     * HandleUIExtensionDied.
     *
     * @param abilityRecord, uiextension record.
     */
    void HandleUIExtensionDied(const std::shared_ptr<AbilityRecord> &abilityRecord);

private:
    /**
     * AddUIExtWindowDeathRecipient.
     *
     * @param session, session info.
     */
    void AddUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session);

    /**
     * RemoveUIExtWindowDeathRecipient.
     *
     * @param session, session info.
     */
    void RemoveUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session);

    /**
     * OnUIExtWindowDied.
     *
     * @param remote, session remote.
     */
    void OnUIExtWindowDied(const wptr<IRemoteObject> &remote);

    /**
     * HandleUIExtWindowDiedTask.
     *
     * @param remote, session remote.
     */
    void HandleUIExtWindowDiedTask(const sptr<IRemoteObject> &remote);

    /**
     * IsUIExtensionAbility.
     *
     * @param abilityRecord, extension record.
     * @return Returns true if it's uiextension, false otherwise.
     */
    inline bool IsUIExtensionAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * IsCacheExtensionAbilityByInfo.
     *
     * @param abilityInfo, ability info.
     * @return Returns true if need cache, false otherwise.
     */
    inline bool IsCacheExtensionAbilityByInfo(const AppExecFwk::AbilityInfo &abilityInfo);

    /**
     * IsCacheExtensionAbility.
     *
     * @param abilityRecord, extension record.
     * @return Returns true if need cache, false otherwise.
     */
    inline bool IsCacheExtensionAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * CheckUIExtensionAbilityLoaded.
     *
     * @param abilityRequest, ability request.
     * @return Returns true if loaded, false otherwise.
     */
    inline bool CheckUIExtensionAbilityLoaded(const AbilityRequest &abilityRequest);

    /**
     * CheckUIExtensionAbilitySessionExist.
     *
     * @param abilityRecord, extension record.
     * @return Returns true if session exists, false otherwise.
     */
    inline bool CheckUIExtensionAbilitySessionExist(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * RemoveUIExtensionAbilityRecord.
     *
     * @param abilityRecord, extension record.
     */
    inline void RemoveUIExtensionAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * AddUIExtensionAbilityRecordToTerminatedList.
     *
     * @param abilityRecord, extension record.
     */
    inline void AddUIExtensionAbilityRecordToTerminatedList(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * GetOrCreateExtensionRecord.
     *
     * @param abilityRequest, ability request.
     * @param isCreatedByConnect, whether created by connect.
     * @param hostBundleName, host bundle name.
     * @param extensionRecord, output extension record.
     * @param isLoaded, output whether loaded.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t GetOrCreateExtensionRecord(const AbilityRequest &abilityRequest, bool isCreatedByConnect,
        const std::string &hostBundleName, std::shared_ptr<AbilityRecord> &extensionRecord, bool &isLoaded);

    /**
     * GetOrCreateTargetServiceRecord.
     *
     * @param abilityRequest, ability request.
     * @param connectInfo, connect info.
     * @param targetService, output target service.
     * @param isLoadedAbility, output whether loaded.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t GetOrCreateTargetServiceRecord(
        const AbilityRequest &abilityRequest, const sptr<UIExtensionAbilityConnectInfo> &connectInfo,
        std::shared_ptr<AbilityRecord> &targetService, bool &isLoadedAbility);

    /**
     * HandleNotifyAssertFaultDialogDied.
     *
     * @param abilityRecord, extension record.
     */
    void HandleNotifyAssertFaultDialogDied(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * UpdateUIExtensionInfo.
     *
     * @param abilityRecord, extension record.
     * @param hostPid, host process id.
     */
    void UpdateUIExtensionInfo(const std::shared_ptr<AbilityRecord> &abilityRecord,
        int32_t hostPid = AAFwk::DEFAULT_INVAL_VALUE);

    /**
     * GenerateBundleName.
     *
     * @param abilityRequest, ability request.
     * @return Returns bundle name.
     */
    std::string GenerateBundleName(const AbilityRequest &abilityRequest) const;

    /**
     * DoForegroundUIExtension.
     *
     * @param abilityRecord, extension record.
     * @param abilityRequest, ability request.
     */
    void DoForegroundUIExtension(std::shared_ptr<AbilityRecord> abilityRecord, const AbilityRequest &abilityRequest);

private:
    UIExtensionMapType uiExtensionMap_;
    WindowExtensionMapType windowExtensionMap_;
    RecipientMapType uiExtRecipientMap_;
    std::unique_ptr<UIExtensionAbilityConnectManager> uiExtensionAbilityRecordMgr_ = nullptr;

    std::mutex uiExtRecipientMapMutex_;
    std::mutex uiExtensionMapMutex_;
    std::mutex windowExtensionMapMutex_;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_UI_EXTENSION_CONNECT_MANAGER_H