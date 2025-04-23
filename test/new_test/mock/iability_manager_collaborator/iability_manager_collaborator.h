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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_IABILITY_MANAGER_COLLABORATOR_H
#define MOCK_OHOS_ABILITY_RUNTIME_IABILITY_MANAGER_COLLABORATOR_H

#include "ability_info.h"
#include "iremote_broker.h"
#include "session_info.h"
#include "want.h"
#include "mock_common.h"

namespace OHOS {
namespace AAFwk {
class SessionInfo;
class IAbilityManagerCollaborator : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.IAbilityManagerCollaborator");

    virtual int32_t NotifyStartAbility(const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t userId, Want &want, uint64_t accessTokenIDEx) = 0;

    virtual int32_t NotifyPreloadAbility(const std::string &bundleName)
    {
        return 0;
    }

    virtual int32_t NotifyMissionCreated(int32_t missionId, const Want &want) = 0;

    virtual int32_t NotifyMissionCreated(const sptr<SessionInfo> &sessionInfo) = 0;

    virtual int32_t NotifyLoadAbility(
        const AppExecFwk::AbilityInfo &abilityInfo, int32_t missionId, const Want &want) = 0;

    virtual int32_t NotifyLoadAbility(
        const AppExecFwk::AbilityInfo &abilityInfo, const sptr<SessionInfo> &sessionInfo) = 0;

    virtual int32_t NotifyMoveMissionToBackground(int32_t missionId) = 0;

    virtual int32_t NotifyMoveMissionToForeground(int32_t missionId) = 0;

    virtual int32_t NotifyTerminateMission(int32_t missionId) = 0;

    virtual int32_t NotifyClearMission(int32_t missionId) = 0;

    virtual int32_t NotifyRemoveShellProcess(int32_t pid, int32_t type, const std::string &reason) = 0;

    virtual void UpdateMissionInfo(sptr<SessionInfo> &sessionInfo) = 0;

    virtual int32_t CheckCallAbilityPermission(const Want &want)
    {
        return -1;
    }

    virtual bool UpdateConfiguration(const AppExecFwk::Configuration &config, int32_t userId)
    {
        return true;
    }

    virtual int32_t OpenFile(const Uri& uri, uint32_t flag)
    {
        return -1;
    }

    virtual void NotifyMissionBindPid(int32_t missionId, int32_t pid)
    {
        return;
    }

    virtual int32_t CheckStaticCfgPermission(const Want &want, bool isImplicit)
    {
        return 0;
    }

    virtual int32_t UpdateCallerIfNeed(Want &want)
    {
        return 0;
    }

    virtual int32_t UpdateTargetIfNeed(Want &want)
    {
        return 0;
    }

    virtual int32_t NotifyKillProcesses(const std::string &bundleName, int32_t userId)
    {
        return 0;
    }

    enum {
        NOTIFY_START_ABILITY = 1,
        NOTIFY_MISSION_CREATED,
        NOTIFY_LOAD_ABILITY,
        NOTIFY_MOVE_MISSION_TO_BACKGROUND,
        NOTIFY_MOVE_MISSION_TO_FOREGROUND,
        NOTIFY_TERMINATE_MISSION,
        NOTIFY_CLEAR_MISSION,
        NOTIFY_REMOVE_SHELL_PROCESS,
        NOTIFY_MISSION_CREATED_BY_SCB = 10,
        NOTIFY_LOAD_ABILITY_BY_SCB,
        UPDATE_MISSION_INFO_BY_SCB,
        NOTIFY_PRELOAD_ABILITY,
        CHECK_CALL_ABILITY_PERMISSION,
        UPDATE_CONFIGURATION,
        OPEN_FILE,
        NOTIFY_MISSION_BIND_PID,
        CHECK_STATIC_CFG_PERMISSION,
        UPDATE_CALLER_IF_NEED,
        UPDATE_TARGET_IF_NEED,
    };
};
}  // namespace AAFwk
}  // namespace OHOS
#endif