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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MISSION_LIST_MANAGER_INTERFACE_H
#define MOCK_OHOS_ABILITY_RUNTIME_MISSION_LIST_MANAGER_INTERFACE_H

#include <memory>
#include <string>
#include <vector>

#include "ability_running_info.h"
#include "mission_info.h"
#include "start_options.h"
#include "want.h"
#include "window_config.h"
#include "mock_common.h"

namespace OHOS {
namespace AAFwk {
class MissionListManagerInterface {
public:
    virtual ~MissionListManagerInterface() = default;

    virtual void Init() {}
    virtual int StartAbility(AbilityRequest &abilityRequest)
    {
        return 0;
    }
    virtual int MinimizeAbility(const sptr<IRemoteObject> &token, bool fromUser)
    {
        return 0;
    }
    virtual int RegisterMissionListener(const sptr<IMissionListener> &listener)
    {
        return 0;
    }
    virtual int UnRegisterMissionListener(const sptr<IMissionListener> &listener)
    {
        return 0;
    }
    virtual int GetMissionInfos(int32_t numMax, std::vector<MissionInfo> &missionInfos)
    {
        return 0;
    }
    virtual int GetMissionInfo(int32_t missionId, MissionInfo &missionInfo)
    {
        return 0;
    }
    virtual int MoveMissionToFront(int32_t missionId, std::shared_ptr<StartOptions> startOptions = nullptr)
    {
        return 0;
    }
    virtual int MoveMissionToFront(int32_t missionId, bool isCallerFromLauncher, bool isRecent,
        std::shared_ptr<AbilityRecord> callerAbility, std::shared_ptr<StartOptions> startOptions = nullptr)
    {
        return 0;
    }
    virtual void NotifyMissionFocused(int32_t missionId) {}
    virtual void NotifyMissionUnfocused(int32_t missionId) {}
    virtual void OnAbilityRequestDone(const sptr<IRemoteObject> &token, int32_t state) {}
    virtual void OnAppStateChanged(const AppInfo &info) {}
    virtual int AttachAbilityThread(const sptr<AAFwk::IAbilityScheduler> &scheduler,
        const sptr<IRemoteObject> &token)
    {
        return 0;
    }
    virtual std::shared_ptr<AbilityRecord> GetAbilityRecordByToken(const sptr<IRemoteObject> &token)
    {
        return nullptr;
    }
    virtual std::shared_ptr<AbilityRecord> GetAbilityRecordByMissionId(int missionId)
    {
        return nullptr;
    }
    virtual int MoveAbilityToBackground(const std::shared_ptr<AbilityRecord> &abilityRecord)
    {
        return 0;
    }
    virtual int32_t BackToCallerAbilityWithResult(std::shared_ptr<AbilityRecord> abilityRecord,
        int32_t resultCode, const Want *resultWant, int64_t callerRequestCode)
    {
        return 0;
    }
    virtual int TerminateAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
        int resultCode, const Want *resultWant, bool flag)
    {
        return 0;
    }
    virtual int AbilityTransactionDone(const sptr<IRemoteObject> &token, int state, const AppExecFwk::PacMap &saveData)
    {
        return 0;
    }
    virtual std::shared_ptr<AbilityRecord> GetAbilityFromTerminateList(const sptr<IRemoteObject> &token)
    {
        return nullptr;
    }
    virtual int ClearMission(int missionId)
    {
        return 0;
    }
    virtual int ClearAllMissions()
    {
        return 0;
    }

    virtual int SetMissionLockedState(int missionId, bool lockedState)
    {
        return 0;
    }
    virtual void OnTimeOut(uint32_t msgId, int64_t abilityRecordId, bool isHalf = false) {}
    virtual void OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord, int32_t currentUserId) {}
    virtual void OnCallConnectDied(const std::shared_ptr<CallRecord> &callRecord) {}
    virtual int32_t GetMissionIdByAbilityToken(const sptr<IRemoteObject> &token)
    {
        return 0;
    }
    virtual sptr<IRemoteObject> GetAbilityTokenByMissionId(int32_t missionId)
    {
        return nullptr;
    }

    virtual void Dump(std::vector<std::string> &info) {}

    virtual void DumpMissionList(std::vector<std::string> &info, bool isClient, const std::string &args = "") {}

    virtual void DumpMissionListByRecordId(std::vector<std::string> &info, bool isClient, int32_t abilityRecordId,
        const std::vector<std::string> &params) {}
    virtual void DumpMission(int missionId, std::vector<std::string> &info) {}
    virtual void DumpMissionInfos(std::vector<std::string> &info) {}
    virtual void OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag) {}
    virtual void OnStartSpecifiedAbilityTimeoutResponse() {}
    virtual int ResolveLocked(const AbilityRequest &abilityRequest)
    {
        return 0;
    }

    virtual int ReleaseCallLocked(const sptr<IAbilityConnection> &connect,
        const AppExecFwk::ElementName &element)
    {
        return 0;
    }
    virtual void RegisterSnapshotHandler(const sptr<ISnapshotHandler> &handler) {}
    virtual bool GetMissionSnapshot(int32_t missionId, const sptr<IRemoteObject> &abilityToken,
        MissionSnapshot &missionSnapshot, bool isLowResolution)
    {
        return false;
    }
    virtual void GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm) {}

    virtual void UpdateSnapShot(const sptr<IRemoteObject> &token,
        const std::shared_ptr<Media::PixelMap> &pixelMap) {}

    virtual void EnableRecoverAbility(int32_t missionId) {}

    virtual void UninstallApp(const std::string &bundleName, int32_t uid) {}

    virtual bool IsStarted()
    {
        return false;
    }
    virtual void PauseManager() {}
    virtual void ResumeManager() {}
    virtual int32_t IsValidMissionIds(const std::vector<int32_t> &missionIds,
        std::vector<MissionValidResult> &results)
    {
        return 0;
    }
    virtual int DoAbilityForeground(std::shared_ptr<AbilityRecord> &abilityRecord, uint32_t flag)
    {
        return 0;
    }
    virtual void CallRequestDone(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<IRemoteObject> &callStub) {}
    virtual int SetMissionContinueState(const sptr<IRemoteObject> &token, int32_t missionId,
        const AAFwk::ContinueState &state)
    {
        return 0;
    }

    virtual bool IsAbilityStarted(AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &targetRecord)
    {
        return false;
    }
    virtual void SignRestartAppFlag(int32_t uid, const std::string &instanceKey) {}
    virtual int SetMissionLabel(const sptr<IRemoteObject> &abilityToken, const std::string &label)
    {
        return 0;
    }
    virtual int SetMissionIcon(const sptr<IRemoteObject> &token, const std::shared_ptr<Media::PixelMap> &icon)
    {
        return 0;
    }
    virtual void CompleteFirstFrameDrawing(const sptr<IRemoteObject> &abilityToken) {}
};

class MissionListWrap {
public:
    virtual ~MissionListWrap() = default;
    virtual int32_t GetInnerMissionInfoById(int32_t missionId, InnerMissionInfo &innerMissionInfo)
    {
        return 0;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
