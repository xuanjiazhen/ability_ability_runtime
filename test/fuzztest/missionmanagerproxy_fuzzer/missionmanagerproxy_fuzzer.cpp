/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "missionmanagerproxy_fuzzer.h"
#include "fuzz_util.h"

#include <fuzzer/FuzzedDataProvider.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "mission_info.h"
#include "mission_manager_proxy.h"
#include "mission_snapshot.h"
#include "start_options.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {
namespace {
struct MissionFuzzParams {
    std::string srcDeviceId;
    std::string dstDeviceId;
    std::string deviceId;
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
    std::string startTime;
    std::string srcBundleName;
    std::string continueType;
    int32_t missionId = 0;
    int32_t numMax = 0;
    int32_t topMissionId = 0;
    bool fixConflict = false;
    bool isLowResolution = false;
    int64_t tag = 0;
    ContinueState continueState = ContinueState::CONTINUESTATE_UNKNOWN;
};

// Consume every fuzz input up front so the helper call order stays data-independent.
MissionFuzzParams ConsumeMissionParams(FuzzedDataProvider &fdp)
{
    MissionFuzzParams p;
    p.srcDeviceId = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    p.dstDeviceId = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    p.deviceId = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    p.bundleName = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    p.moduleName = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    p.abilityName = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    p.startTime = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    p.srcBundleName = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    p.continueType = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    p.missionId = fdp.ConsumeIntegral<int32_t>();
    p.numMax = fdp.ConsumeIntegral<int32_t>();
    p.topMissionId = fdp.ConsumeIntegral<int32_t>();
    p.fixConflict = fdp.ConsumeBool();
    p.isLowResolution = fdp.ConsumeBool();
    p.tag = fdp.ConsumeIntegral<int64_t>();
    uint8_t continueStateRaw = fdp.ConsumeIntegral<uint8_t>() % static_cast<uint8_t>(CONTINUESTATE_MAX);
    p.continueState = static_cast<ContinueState>(continueStateRaw);
    return p;
}

void RunMissionLifecycleCalls(const sptr<MissionManagerProxy> &proxy, const MissionFuzzParams &p,
    WantParams &wantParams)
{
    ContinueMissionInfo continueMissionInfo;
    continueMissionInfo.srcDeviceId = p.srcDeviceId;
    continueMissionInfo.dstDeviceId = p.dstDeviceId;
    continueMissionInfo.bundleName = p.bundleName;
    continueMissionInfo.srcBundleName = p.srcBundleName;
    continueMissionInfo.continueType = p.continueType;
    continueMissionInfo.wantParams = wantParams;

    // 1. ContinueMission(string, string, int32_t, sptr<IRemoteObject>, WantParams&)
    proxy->ContinueMission(p.srcDeviceId, p.dstDeviceId, p.missionId, nullptr, wantParams);
    // 2. ContinueMission(ContinueMissionInfo, sptr<IRemoteObject>)
    proxy->ContinueMission(continueMissionInfo, nullptr);
    // 3. LockMissionForCleanup(int32_t)
    proxy->LockMissionForCleanup(p.missionId);
    // 4. UnlockMissionForCleanup(int32_t)
    proxy->UnlockMissionForCleanup(p.missionId);
    // 5. RegisterMissionListener(sptr<IMissionListener>)
    proxy->RegisterMissionListener(nullptr);
    // 6. UnRegisterMissionListener(sptr<IMissionListener>)
    proxy->UnRegisterMissionListener(nullptr);
    // 7. RegisterMissionListener(string, sptr<IRemoteMissionListener>)
    proxy->RegisterMissionListener(p.deviceId, nullptr);
    // 8. UnRegisterMissionListener(string, sptr<IRemoteMissionListener>)
    proxy->UnRegisterMissionListener(p.deviceId, nullptr);
    // 9. GetMissionInfos(string, int32_t, vector<MissionInfo>&)
    std::vector<MissionInfo> missionInfos;
    proxy->GetMissionInfos(p.deviceId, p.numMax, missionInfos);
    // 10. GetMissionInfo(string, int32_t, MissionInfo&)
    MissionInfo missionInfo;
    proxy->GetMissionInfo(p.deviceId, p.missionId, missionInfo);
    // 11. GetMissionSnapshot(string, int32_t, MissionSnapshot&, bool)
    MissionSnapshot snapshot;
    proxy->GetMissionSnapshot(p.deviceId, p.missionId, snapshot, p.isLowResolution);
    // 12. CleanMission(int32_t)
    proxy->CleanMission(p.missionId);
    // 13. CleanAllMissions()
    proxy->CleanAllMissions();
}

void RunMissionCommandCalls(const sptr<MissionManagerProxy> &proxy, const MissionFuzzParams &p,
    std::vector<int32_t> &missionIds)
{
    StartOptions startOptions;
    // 14. MoveMissionToFront(int32_t)
    proxy->MoveMissionToFront(p.missionId);
    // 15. MoveMissionToFront(int32_t, StartOptions&)
    proxy->MoveMissionToFront(p.missionId, startOptions);
    // 16. MoveMissionsToForeground(vector<int32_t>&, int32_t)
    proxy->MoveMissionsToForeground(missionIds, p.topMissionId);
    // 17. MoveMissionsToBackground(vector<int32_t>&, vector<int32_t>&)
    std::vector<int32_t> moveResult;
    proxy->MoveMissionsToBackground(missionIds, moveResult);
    // 18. GetMissionIdByToken(sptr<IRemoteObject>)
    proxy->GetMissionIdByToken(nullptr);
    // 19. StartSyncRemoteMissions(string, bool, int64_t)
    proxy->StartSyncRemoteMissions(p.deviceId, p.fixConflict, p.tag);
    // 20. StopSyncRemoteMissions(string)
    proxy->StopSyncRemoteMissions(p.deviceId);
    // 21. SetMissionContinueState(sptr<IRemoteObject>, ContinueState)
    proxy->SetMissionContinueState(nullptr, p.continueState);

#ifdef SUPPORT_SCREEN
    // 21a. SetMissionLabel(sptr<IRemoteObject>, string)
    proxy->SetMissionLabel(nullptr, p.bundleName);
    // 21b. SetMissionIcon(sptr<IRemoteObject>, shared_ptr<PixelMap>)
    std::shared_ptr<OHOS::Media::PixelMap> icon = nullptr;
    proxy->SetMissionIcon(nullptr, icon);
    // 21c. UpdateMissionSnapShot(sptr<IRemoteObject>, shared_ptr<PixelMap>)
    proxy->UpdateMissionSnapShot(nullptr, icon);
#endif

    // 22. IsValidMissionIds(vector<int32_t>&, vector<MissionValidResult>&)
    std::vector<MissionValidResult> results;
    proxy->IsValidMissionIds(missionIds, results);
    // 23. PreStartMission(string, string, string, string)
    proxy->PreStartMission(p.bundleName, p.moduleName, p.abilityName, p.startTime);
    // 24. TerminateMission(int32_t)
    proxy->TerminateMission(p.missionId);
}
} // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return false;
    }

    FuzzedDataProvider fdp(data, size);

    MissionFuzzParams p = ConsumeMissionParams(fdp);

    WantParams wantParams;
    std::vector<int32_t> missionIds = OHOS::FuzzUtil::BuildInt32Vector(fdp);

    sptr<IRemoteObject> remoteObj = nullptr;
    sptr<MissionManagerProxy> proxy = new MissionManagerProxy(remoteObj);

    RunMissionLifecycleCalls(proxy, p, wantParams);
    RunMissionCommandCalls(proxy, p, missionIds);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    // Run your code on data.
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
