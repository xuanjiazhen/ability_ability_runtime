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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_DISTRIBUTED_CLIENT_H
#define MOCK_OHOS_ABILITY_RUNTIME_DISTRIBUTED_CLIENT_H

#include <string>
#include "dms_continueInfo.h"
#include "mission_info.h"
#include "want_params.h"
#include "want.h"
#include "iremote_broker.h"
#include "mock_common.h"

namespace OHOS {
namespace AAFwk {
class DistributedClient {
public:
    DistributedClient() = default;
    virtual ~DistributedClient() = default;
    int32_t StartRemoteAbility(const OHOS::AAFwk::Want& want, int32_t callerUid, int32_t requestCode,
        uint32_t accessToken)
    {
        return 0;
    }
    int32_t ContinueMission(const std::string& srcDeviceId, const std::string& dstDeviceId, int32_t missionId,
        const sptr<IRemoteObject>& callback, const OHOS::AAFwk::WantParams& wantParams)
    {
        return 0;
    }
    int32_t ContinueMission(AAFwk::ContinueMissionInfo continueMissionInfo, const sptr<IRemoteObject> &callBack)
    {
        return 0;
    }
    int32_t StartContinuation(const OHOS::AAFwk::Want& want, int32_t missionId, int32_t callerUid,
        int32_t status, uint32_t accessToken)
    {
        return 0;
    }
    int32_t NotifyCompleteContinuation(const std::u16string &devId, int32_t sessionId, bool isSuccess,
        const std::string &callerBundleName)
    {
        return 0;
    }
    int32_t ConnectRemoteAbility(const OHOS::AAFwk::Want& want, const sptr<IRemoteObject>& connect)
    {
        return 0;
    }
    int32_t DisconnectRemoteAbility(const sptr<IRemoteObject>& connect, int32_t callerUid, uint32_t accessToken)
    {
        return 0;
    }
    int32_t StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag, int32_t callingUid)
    {
        return 0;
    }
    int32_t StopSyncRemoteMissions(const std::string& devId, int32_t callingUid)
    {
        return 0;
    }
    int32_t RegisterMissionListener(const std::u16string& devId, const sptr<IRemoteObject>& obj, int32_t callingUid)
    {
        return 0;
    }
    int32_t RegisterOnListener(const std::string& type, const sptr<IRemoteObject>& obj, int32_t callingUid)
    {
        return 0;
    }
    int32_t RegisterOffListener(const std::string& type, const sptr<IRemoteObject>& obj, int32_t callingUid)
    {
        return 0;
    }
    int32_t UnRegisterMissionListener(const std::u16string& devId, const sptr<IRemoteObject>& obj)
    {
        return 0;
    }
    int32_t GetMissionInfos(const std::string& deviceId, int32_t numMissions,
        std::vector<AAFwk::MissionInfo>& missionInfos)
    {
        return 0;
    }
    int32_t GetRemoteMissionSnapshotInfo(const std::string& deviceId, int32_t missionId,
        std::unique_ptr<MissionSnapshot>& missionSnapshot)
    {
        return 0;
    }
    int32_t StartRemoteAbilityByCall(const OHOS::AAFwk::Want& want, const sptr<IRemoteObject>& connect)
    {
        return 0;
    }
    int32_t ReleaseRemoteAbility(const sptr<IRemoteObject>& connect, const AppExecFwk::ElementName &element)
    {
        return 0;
    }
    int32_t StartRemoteFreeInstall(const OHOS::AAFwk::Want& want,
        int32_t callerUid, int32_t requestCode, uint32_t accessToken, const sptr<IRemoteObject>& callback)
    {
        return 0;
    }
    int32_t StopRemoteExtensionAbility(const Want &want, int32_t callerUid,
        uint32_t accessToken, int32_t extensionType)
    {
        return 0;
    }
    int32_t SetMissionContinueState(int32_t missionId, const AAFwk::ContinueState &state, int32_t callingUid)
    {
        return 0;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
