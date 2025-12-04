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

#include "ui_extension_manager.h"

namespace OHOS {
namespace AAFwk {

UIExtensionManager::UIExtensionManager(int userId) : AbilityConnectManager(userId)
{
    uiExtensionAbilityRecordMgr_ = std::make_unique<AbilityRuntime::ExtensionRecordManager>(userId);
}

UIExtensionManager::~UIExtensionManager()
{
    // 清理资源
}

int32_t UIExtensionManager::StartAbility(const AbilityRequest &abilityRequest)
{
    // TODO: 实现UIExtension的StartAbility逻辑
    return ERR_OK;
}

int UIExtensionManager::TerminateAbility(const sptr<IRemoteObject> &token)
{
    // TODO: 实现UIExtension的TerminateAbility逻辑
    return ERR_OK;
}

int UIExtensionManager::StopServiceAbility(const AbilityRequest &abilityRequest)
{
    // TODO: 实现UIExtension的StopServiceAbility逻辑
    return ERR_OK;
}

int32_t UIExtensionManager::ConnectAbilityLocked(const AbilityRequest &abilityRequest, 
    const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken, 
    sptr<SessionInfo> sessionInfo, sptr<UIExtensionAbilityConnectInfo> connectInfo)
{
    // TODO: 实现UIExtension的ConnectAbilityLocked逻辑
    return ERR_OK;
}

int UIExtensionManager::DisconnectAbilityLocked(const sptr<IAbilityConnection> &connect)
{
    // TODO: 实现UIExtension的DisconnectAbilityLocked逻辑
    return ERR_OK;
}

int UIExtensionManager::AttachAbilityThreadLocked(const sptr<IAbilityScheduler> &scheduler, 
    const sptr<IRemoteObject> &token)
{
    // TODO: 实现UIExtension的AttachAbilityThreadLocked逻辑
    return ERR_OK;
}

int UIExtensionManager::AbilityTransitionDone(const sptr<IRemoteObject> &token, int state)
{
    // TODO: 实现UIExtension的AbilityTransitionDone逻辑
    return ERR_OK;
}

int UIExtensionManager::ScheduleConnectAbilityDoneLocked(const sptr<IRemoteObject> &token, 
    const sptr<IRemoteObject> &remoteObject)
{
    // TODO: 实现UIExtension的ScheduleConnectAbilityDoneLocked逻辑
    return ERR_OK;
}

int UIExtensionManager::ScheduleDisconnectAbilityDoneLocked(const sptr<IRemoteObject> &token)
{
    // TODO: 实现UIExtension的ScheduleDisconnectAbilityDoneLocked逻辑
    return ERR_OK;
}

int UIExtensionManager::ScheduleCommandAbilityDoneLocked(const sptr<IRemoteObject> &token)
{
    // TODO: 实现UIExtension的ScheduleCommandAbilityDoneLocked逻辑
    return ERR_OK;
}

void UIExtensionManager::OnAbilityDied(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    // TODO: 实现UIExtension的OnAbilityDied逻辑
}

void UIExtensionManager::OnTimeOut(uint32_t msgId, int64_t abilityRecordId, bool isHalf)
{
    // TODO: 实现UIExtension的OnTimeOut逻辑
}

int UIExtensionManager::PreloadUIExtensionAbilityLocked(const AbilityRequest &abilityRequest, 
    std::string &hostBundleName, int32_t hostPid)
{
    // TODO: 实现UIExtension的PreloadUIExtensionAbilityLocked逻辑
    return ERR_OK;
}

int UIExtensionManager::QueryPreLoadUIExtensionRecordInner(const AppExecFwk::ElementName &element,
                                                           const std::string &moduleName,
                                                           const int32_t hostPid,
                                                           int32_t &recordNum)
{
    // TODO: 实现UIExtension的QueryPreLoadUIExtensionRecordInner逻辑
    return ERR_OK;
}

int UIExtensionManager::UnloadUIExtensionAbility(const std::shared_ptr<AAFwk::AbilityRecord> &abilityRecord, 
    pid_t &hostPid)
{
    // TODO: 实现UIExtension的UnloadUIExtensionAbility逻辑
    return ERR_OK;
}

void UIExtensionManager::ClearPreloadUIExtensionRecord(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    // TODO: 实现UIExtension的ClearPreloadUIExtensionRecord逻辑
}

int32_t UIExtensionManager::GetActiveUIExtensionList(const int32_t pid, std::vector<std::string> &extensionList)
{
    // TODO: 实现UIExtension的GetActiveUIExtensionList逻辑
    return ERR_OK;
}

int32_t UIExtensionManager::GetActiveUIExtensionList(const std::string &bundleName, std::vector<std::string> &extensionList)
{
    // TODO: 实现UIExtension的GetActiveUIExtensionList逻辑
    return ERR_OK;
}

void UIExtensionManager::MoveToBackground(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    // TODO: 实现UIExtension的MoveToBackground逻辑
}

void UIExtensionManager::CommandAbilityWindow(const std::shared_ptr<AbilityRecord> &abilityRecord,
    const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd)
{
    // TODO: 实现UIExtension的CommandAbilityWindow逻辑
}

bool UIExtensionManager::IsUIExtensionFocused(uint32_t uiExtensionTokenId, const sptr<IRemoteObject>& focusToken)
{
    // TODO: 实现UIExtension的IsUIExtensionFocused逻辑
    return false;
}

sptr<IRemoteObject> UIExtensionManager::GetUIExtensionSourceToken(const sptr<IRemoteObject> &token)
{
    // TODO: 实现UIExtension的GetUIExtensionSourceToken逻辑
    return nullptr;
}

bool UIExtensionManager::IsWindowExtensionFocused(uint32_t extensionTokenId, const sptr<IRemoteObject>& focusToken)
{
    // TODO: 实现UIExtension的IsWindowExtensionFocused逻辑
    return false;
}

void UIExtensionManager::ForegroundAbilityWindowLocked(const std::shared_ptr<AbilityRecord> &abilityRecord,
    const sptr<SessionInfo> &sessionInfo)
{
    // TODO: 实现UIExtension的ForegroundAbilityWindowLocked逻辑
}

void UIExtensionManager::BackgroundAbilityWindowLocked(const std::shared_ptr<AbilityRecord> &abilityRecord,
    const sptr<SessionInfo> &sessionInfo)
{
    // TODO: 实现UIExtension的BackgroundAbilityWindowLocked逻辑
}

void UIExtensionManager::TerminateAbilityWindowLocked(const std::shared_ptr<AbilityRecord> &abilityRecord,
    const sptr<SessionInfo> &sessionInfo)
{
    // TODO: 实现UIExtension的TerminateAbilityWindowLocked逻辑
}

int32_t UIExtensionManager::GetUIExtensionSessionInfo(const sptr<IRemoteObject> token, 
    UIExtensionSessionInfo &uiExtensionSessionInfo)
{
    // TODO: 实现UIExtension的GetUIExtensionSessionInfo逻辑
    return ERR_OK;
}

void UIExtensionManager::GetUIExtensionCallerTokenList(const std::shared_ptr<AbilityRecord> &abilityRecord,
    std::list<sptr<IRemoteObject>> &callerList)
{
    // TODO: 实现UIExtension的GetUIExtensionCallerTokenList逻辑
}

std::shared_ptr<AAFwk::AbilityRecord> UIExtensionManager::GetUIExtensionRootHostInfo(const sptr<IRemoteObject> token)
{
    // TODO: 实现UIExtension的GetUIExtensionRootHostInfo逻辑
    return nullptr;
}

std::shared_ptr<AbilityRecord> UIExtensionManager::GetUIExtensionBySessionInfo(const sptr<SessionInfo> &sessionInfo)
{
    // TODO: 实现UIExtension的GetUIExtensionBySessionInfo逻辑
    return nullptr;
}

std::shared_ptr<AbilityRecord> UIExtensionManager::GetUIExtensionBySessionFromServiceMap(const sptr<SessionInfo> &sessionInfo)
{
    // TODO: 实现UIExtension的GetUIExtensionBySessionFromServiceMap逻辑
    return nullptr;
}

int UIExtensionManager::UnPreloadUIExtensionAbilityLocked(int32_t extensionAbilityId)
{
    // TODO: 实现UIExtension的UnPreloadUIExtensionAbilityLocked逻辑
    return ERR_OK;
}

int UIExtensionManager::UnPreloadUIExtensionAbilityInner(int32_t extensionAbilityId)
{
    // TODO: 实现UIExtension的UnPreloadUIExtensionAbilityInner逻辑
    return ERR_OK;
}

int UIExtensionManager::ClearAllPreloadUIExtensionAbilityLocked()
{
    // TODO: 实现UIExtension的ClearAllPreloadUIExtensionAbilityLocked逻辑
    return ERR_OK;
}

int UIExtensionManager::ClearAllPreloadUIExtensionAbilityInner()
{
    // TODO: 实现UIExtension的ClearAllPreloadUIExtensionAbilityInner逻辑
    return ERR_OK;
}

int32_t UIExtensionManager::RegisterPreloadUIExtensionHostClient(const sptr<IRemoteObject> &callerToken)
{
    // TODO: 实现UIExtension的RegisterPreloadUIExtensionHostClient逻辑
    return ERR_OK;
}

int32_t UIExtensionManager::UnRegisterPreloadUIExtensionHostClient(int32_t callerPid)
{
    // TODO: 实现UIExtension的UnRegisterPreloadUIExtensionHostClient逻辑
    return ERR_OK;
}

UIExtensionManager::PreloadUIExtensionHostClientDeathRecipient::PreloadUIExtensionHostClientDeathRecipient(
    PreloadUIExtensionHostClientDiedHandler handler)
    : diedHandler_(handler)
{}

void UIExtensionManager::PreloadUIExtensionHostClientDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    // TODO: 实现PreloadUIExtensionHostClientDeathRecipient的OnRemoteDied逻辑
}

}  // namespace AAFwk
}  // namespace OHOS