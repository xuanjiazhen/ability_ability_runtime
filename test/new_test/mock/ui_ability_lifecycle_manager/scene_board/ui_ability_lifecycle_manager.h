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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_UI_ABILITY_LIFECYCLE_MANAGER_H
#define MOCK_OHOS_ABILITY_RUNTIME_UI_ABILITY_LIFECYCLE_MANAGER_H

#include <list>
#include <memory>

#include "ability_record.h"
#include "isession_handler_interface.h"

namespace OHOS {
namespace AAFwk {
class UIAbilityLifecycleManager : public std::enable_shared_from_this<UIAbilityLifecycleManager> {
public:
    UIAbilityLifecycleManager() = default;
    explicit UIAbilityLifecycleManager(int32_t userId) {}
    virtual ~UIAbilityLifecycleManager() = default;

    void SignRestartAppFlag(int32_t uid, const std::string &instanceKey, bool isAppRecovery = false) {}

    int StartUIAbility(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo, uint32_t sceneFlag,
        bool &isColdStart)
    {
        return 0;
    }

    int AbilityTransactionDone(const sptr<IRemoteObject> &token, int state, const AppExecFwk::PacMap &saveData)
    {
        return 0;
    }

    int AbilityWindowConfigTransactionDone(
        const sptr<IRemoteObject> &token, const WindowConfig &windowConfig)
    {
        return 0;
    }

    int AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
    {
        return 0;
    }

    void OnAbilityRequestDone(const sptr<IRemoteObject> &token, int32_t state) {}

    int32_t NotifySCBToMinimizeUIAbility(const sptr<IRemoteObject> token)
    {
        return 0;
    }

    int MinimizeUIAbility(const std::shared_ptr<AbilityRecord> &abilityRecord, bool fromUser, uint32_t sceneFlag)
    {
        return 0;
    }

    std::shared_ptr<AbilityRecord> GetUIAbilityRecordBySessionInfo(const sptr<SessionInfo> &sessionInfo)
    {
        return nullptr;
    }

    int32_t BackToCallerAbilityWithResult(std::shared_ptr<AbilityRecord> abilityRecord,
        int resultCode, const Want *resultWant, int64_t callerRequestCode)
    {
        return 0;
    }

    int CloseUIAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
        int resultCode, const Want *resultWant, bool isClearSession)
    {
        return 0;
    }

    void SetRootSceneSession(const sptr<IRemoteObject> &rootSceneSession) {}

    int NotifySCBToStartUIAbility(AbilityRequest &abilityRequest)
    {
        return 0;
    }

    int NotifySCBToPreStartUIAbility(const AbilityRequest &abilityRequest,
        sptr<SessionInfo> &sessionInfo)
    {
        return 0;
    }

    int32_t NotifySCBToRecoveryAfterInterception(const AbilityRequest &abilityRequest)
    {
        return 0;
    }

    void OnTimeOut(uint32_t msgId, int64_t abilityRecordId, bool isHalf = false) {}
    void OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord) {}
    int ResolveLocked(const AbilityRequest &abilityRequest, std::string &errMsg)
    {
        return 0;
    }
    void CallUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool &isColdStart) {}
    void OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag, int32_t requestId) {}
    void OnStartSpecifiedProcessResponse(const std::string &flag, int32_t requestId) {}
    void OnStartSpecifiedAbilityTimeoutResponse(int32_t requestId) {}
    void OnStartSpecifiedProcessTimeoutResponse(int32_t requestId) {}
    void OnStartSpecifiedFailed(int32_t requestId) {}
    void StartSpecifiedAbilityBySCB(const Want &want) {}
    void CallRequestDone(const std::shared_ptr<AbilityRecord> &abilityRecord, const sptr<IRemoteObject> &callStub) {}
    int ReleaseCallLocked(const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element)
    {
        return 0;
    }

    void OnCallConnectDied(const std::shared_ptr<CallRecord> &callRecord) {}

    int32_t GetSessionIdByAbilityToken(const sptr<IRemoteObject> &token)
    {
        return 0;
    }

    bool PrepareTerminateAbility(const std::shared_ptr<AbilityRecord> &abilityRecord, bool isSCBCall)
    {
        return false;
    }
    void SetSessionHandler(const sptr<ISessionHandler> &handler) {}

    std::shared_ptr<AbilityRecord> GetAbilityRecordsById(int32_t sessionId) const
    {
        return nullptr;
    }
    void OnAppStateChanged(const AppInfo &info) {}
    void UninstallApp(const std::string &bundleName, int32_t uid) {}
    void GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm) const {}
    void Dump(std::vector<std::string>& info) {}
    void DumpMissionList(std::vector<std::string> &info, bool isClient, const std::string &args = "") {}
    void DumpMissionListByRecordId(std::vector<std::string>& info, bool isClient, int32_t abilityRecordId,
        const std::vector<std::string>& params) {}
    int MoveMissionToFront(int32_t sessionId, std::shared_ptr<StartOptions> startOptions = nullptr)
    {
        return 0;
    }

    bool IsAbilityStarted(AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &targetRecord)
    {
        return false;
    }

    int32_t UpdateSessionInfoBySCB(std::list<SessionInfo> &sessionInfos, std::vector<int32_t> &sessionIds)
    {
        return 0;
    }

    int32_t RegisterStatusBarDelegate(sptr<AbilityRuntime::IStatusBarDelegate> delegate)
    {
        return 0;
    }

    bool IsCallerInStatusBar(const std::string &instanceKey)
    {
        return false;
    }

    bool IsInStatusBar(uint32_t accessTokenId, bool isMultiInstance)
    {
        return false;
    }

    bool IsSupportStatusBar()
    {
        return false;
    }

    int32_t TryPrepareTerminateByPids(const std::vector<int32_t>& pids)
    {
        return 0;
    }

    int ChangeAbilityVisibility(sptr<IRemoteObject> token, bool isShow)
    {
        return 0;
    }

    int ChangeUIAbilityVisibilityBySCB(sptr<SessionInfo> sessionInfo, bool isShow)
    {
        return 0;
    }

    std::vector<std::shared_ptr<AbilityRecord>> GetAbilityRecordsByName(const AppExecFwk::ElementName &element)
    {
        std::vector<std::shared_ptr<AbilityRecord>> ret;
        return ret;
    }
    std::shared_ptr<AbilityRecord> GetAbilityRecordByToken(const sptr<IRemoteObject> &token) const
    {
        return nullptr;
    }
    void CompleteFirstFrameDrawing(int32_t sessionId) const {}
    int32_t GetAbilityStateByPersistentId(int32_t persistentId, bool &state)
    {
        return 0;
    }
    int32_t CleanUIAbility(const std::shared_ptr<AbilityRecord> &abilityRecord)
    {
        return 0;
    }
    void NotifySCBToHandleAtomicServiceException(sptr<SessionInfo> sessionInfo, int32_t errorCode,
        const std::string& errorReason) {}
    void EnableListForSCBRecovery() {}
    void SetKillForPermissionUpdateFlag(uint32_t accessTokenId) {}
    void PrepareTerminateAbilityDone(std::shared_ptr<AbilityRecord> abilityRecord, bool isTerminate) {}
    void TryPrepareTerminateByPidsDone(const std::string &moduleName, int32_t prepareTermination, bool isExist) {}
    bool IsBundleStarting(pid_t pid)
    {
        return false;
    }
    void RecordPidKilling(pid_t pid, const std::string &reason) {}
    int32_t RevokeDelegator(sptr<IRemoteObject> token)
    {
        return 0;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif