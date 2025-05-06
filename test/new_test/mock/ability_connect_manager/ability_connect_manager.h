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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_ABILITY_CONNECT_MANAGER_H
#define MOCK_OHOS_ABILITY_RUNTIME_ABILITY_CONNECT_MANAGER_H

#include <list>
#include <map>
#include <string>
#include <unordered_map>

#include "ability_cache_manager.h"
#include "connection_record.h"
#include "ability_record.h"
#include "iremote_object.h"
#include "ability_connect_callback_interface.h"
#include "task_handler_wrap.h"
#include "ability_running_info.h"
#include "ui_extension_ability_connect_info.h"
#include "extension_running_info.h"
#include "element_name.h"
#include "want.h"
#include "session_info.h"
#include "ability_scheduler_interface.h"
#include "app_scheduler.h"
#include "window_config.h"
#include "ui_extension_window_command.h"
#include "ui_extension_session_info.h"

namespace OHOS {
namespace AAFwk {
class AbilityConnectManager : public std::enable_shared_from_this<AbilityConnectManager> {
public:
    using ConnectListType = std::list<std::shared_ptr<ConnectionRecord>>;

    explicit AbilityConnectManager(int userId) {}
    virtual ~AbilityConnectManager() {}

    int32_t StartAbility(const AbilityRequest &abilityRequest)
    {
        return 0;
    }

    int TerminateAbility(const sptr<IRemoteObject> &token)
    {
        return 0;
    }

    int StopServiceAbility(const AbilityRequest &abilityRequest)
    {
        return 0;
    }

    int32_t ConnectAbilityLocked(const AbilityRequest &abilityRequest, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, sptr<SessionInfo> sessionInfo = nullptr,
        sptr<AbilityRuntime::UIExtensionAbilityConnectInfo> connectInfo = nullptr)
    {
        return 0;
    }

    int PreloadUIExtensionAbilityInner(const AbilityRequest &abilityRequest, std::string &hostBundleName,
        int32_t hostPid = -1)
    {
        return 0;
    }

    int PreloadUIExtensionAbilityLocked(const AbilityRequest &abilityRequest, std::string &hostBundleName,
        int32_t hostPid = -1)
    {
        return 0;
    }

    int QueryPreLoadUIExtensionRecordInner(const AppExecFwk::ElementName &element,
                                           const std::string &moduleName,
                                           const std::string &hostBundleName,
                                           int32_t &recordNum)
    {
        return 0;
    }

    int UnloadUIExtensionAbility(const std::shared_ptr<AAFwk::AbilityRecord> &abilityRecord, std::string &bundleName)
    {
        return 0;
    }

    int DisconnectAbilityLocked(const sptr<IAbilityConnection> &connect)
    {
        return 0;
    }

    int AttachAbilityThreadLocked(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
    {
        return 0;
    }

    void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state) {}

    void OnAppStateChanged(const AppInfo &info) {}

    int AbilityTransitionDone(const sptr<IRemoteObject> &token, int state)
    {
        return 0;
    }

    int AbilityWindowConfigTransactionDone(
        const sptr<IRemoteObject> &token, const WindowConfig &windowConfig)
    {
        return 0;
    }

    int ScheduleConnectAbilityDoneLocked(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject)
    {
        return 0;
    }

    int ScheduleDisconnectAbilityDoneLocked(const sptr<IRemoteObject> &token)
    {
        return 0;
    }

    int ScheduleCommandAbilityDoneLocked(const sptr<IRemoteObject> &token)
    {
        return 0;
    }

    int ScheduleCommandAbilityWindowDone(
        const sptr<IRemoteObject> &token,
        const sptr<SessionInfo> &sessionInfo,
        WindowCommand winCmd,
        AbilityCommand abilityCmd)
    {
        return 0;
    }

    std::shared_ptr<AbilityRecord> GetUIExtensionBySessionInfo(const sptr<SessionInfo> &sessionInfo)
    {
        return nullptr;
    }

    std::shared_ptr<AbilityRecord> GetExtensionByTokenFromServiceMap(const sptr<IRemoteObject> &token)
    {
        return nullptr;
    }

    std::shared_ptr<AbilityRecord> GetExtensionByTokenFromAbilityCache(const sptr<IRemoteObject> &token)
    {
        return nullptr;
    }

    std::shared_ptr<AbilityRecord> GetExtensionByTokenFromTerminatingMap(const sptr<IRemoteObject> &token)
    {
        return nullptr;
    }

    ConnectListType GetConnectRecordListByCallback(sptr<IAbilityConnection> callback)
    {
        ConnectListType list;
        return list;
    }

    void GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info,
        const int32_t userId, bool isPerm) {}

    void GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm) {}

    inline void SetTaskHandler(const std::shared_ptr<TaskHandlerWrap> &taskHandler) {}

    uint32_t GetSceneBoardTokenId() const
    {
        return 0;
    }

    void OnLoadAbilityFailed(std::shared_ptr<AbilityRecord> abilityRecord) {}

    void OnAbilityDied(const std::shared_ptr<AbilityRecord> &abilityRecord, int32_t currentUserId) {}

    void DumpState(std::vector<std::string> &info, bool isClient, const std::string &args = "") {}

    void DumpStateByUri(std::vector<std::string> &info, bool isClient, const std::string &args,
        std::vector<std::string> &params) {}

    void PauseExtensions() {}

    void OnTimeOut(uint32_t msgId, int64_t abilityRecordId, bool isHalf = false) {}

    bool IsUIExtensionFocused(uint32_t uiExtensionTokenId, const sptr<IRemoteObject>& focusToken)
    {
        return false;
    }

    sptr<IRemoteObject> GetUIExtensionSourceToken(const sptr<IRemoteObject> &token)
    {
        return nullptr;
    }

    bool IsWindowExtensionFocused(uint32_t extensionTokenId, const sptr<IRemoteObject>& focusToken)
    {
        return false;
    }

    void HandleProcessFrozen(const std::vector<int32_t> &pidList, int32_t uid) {}

    void BackgroundAbilityWindowLocked(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo) {}

    void TerminateAbilityWindowLocked(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo) {}

    void RemoveLauncherDeathRecipient() {}

    int32_t GetUIExtensionSessionInfo(const sptr<IRemoteObject> token,
        AbilityRuntime::UIExtensionSessionInfo &uiExtensionSessionInfo)
    {
        return 0;
    }

    void GetUIExtensionCallerTokenList(const std::shared_ptr<AbilityRecord> &abilityRecord,
        std::list<sptr<IRemoteObject>> &callerList) {}

    void CloseAssertDialog(const std::string &assertSessionId) {}

    void SignRestartAppFlag(int32_t uid, const std::string &instanceKey) {}

    std::shared_ptr<AAFwk::AbilityRecord> GetUIExtensionRootHostInfo(const sptr<IRemoteObject> token)
    {
        return nullptr;
    }

    void UninstallApp(const std::string &bundleName, int32_t uid) {}

    int32_t UpdateKeepAliveEnableState(const std::string &bundleName, const std::string &moduleName,
        const std::string &mainElement, bool updateEnable)
    {
        return 0;
    }

    void DisconnectBeforeCleanup() {}

    std::shared_ptr<AbilityRecord> GetUIExtensionBySessionFromServiceMap(const sptr<SessionInfo> &sessionInfo)
    {
        return nullptr;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
