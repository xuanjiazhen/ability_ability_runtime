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
bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return false;
    }

    FuzzedDataProvider fdp(data, size);

    // Consume all fuzz input parameters first to avoid inter-method data dependency.
    std::string srcDeviceId = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    std::string dstDeviceId = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    std::string deviceId = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    std::string bundleName = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    std::string moduleName = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    std::string abilityName = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    std::string startTime = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    std::string srcBundleName = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    std::string continueType = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    int32_t missionId = fdp.ConsumeIntegral<int32_t>();
    int32_t numMax = fdp.ConsumeIntegral<int32_t>();
    int32_t topMissionId = fdp.ConsumeIntegral<int32_t>();
    bool fixConflict = fdp.ConsumeBool();
    bool isLowResolution = fdp.ConsumeBool();
    int64_t tag = fdp.ConsumeIntegral<int64_t>();
    uint8_t continueStateRaw = fdp.ConsumeIntegral<uint8_t>() % static_cast<uint8_t>(CONTINUESTATE_MAX);
    ContinueState continueState = static_cast<ContinueState>(continueStateRaw);

    WantParams wantParams;
    std::vector<int32_t> missionIds = OHOS::FuzzUtil::BuildInt32Vector(fdp);

    ContinueMissionInfo continueMissionInfo;
    continueMissionInfo.srcDeviceId = srcDeviceId;
    continueMissionInfo.dstDeviceId = dstDeviceId;
    continueMissionInfo.bundleName = bundleName;
    continueMissionInfo.srcBundleName = srcBundleName;
    continueMissionInfo.continueType = continueType;
    continueMissionInfo.wantParams = wantParams;

    StartOptions startOptions;

    sptr<IRemoteObject> remoteObj = nullptr;
    sptr<MissionManagerProxy> proxy = new MissionManagerProxy(remoteObj);

    // 1. ContinueMission(string, string, int32_t, sptr<IRemoteObject>, WantParams&)
    proxy->ContinueMission(srcDeviceId, dstDeviceId, missionId, nullptr, wantParams);

    // 2. ContinueMission(ContinueMissionInfo, sptr<IRemoteObject>)
    proxy->ContinueMission(continueMissionInfo, nullptr);

    // 3. LockMissionForCleanup(int32_t)
    proxy->LockMissionForCleanup(missionId);

    // 4. UnlockMissionForCleanup(int32_t)
    proxy->UnlockMissionForCleanup(missionId);

    // 5. RegisterMissionListener(sptr<IMissionListener>)
    proxy->RegisterMissionListener(nullptr);

    // 6. UnRegisterMissionListener(sptr<IMissionListener>)
    proxy->UnRegisterMissionListener(nullptr);

    // 7. RegisterMissionListener(string, sptr<IRemoteMissionListener>)
    proxy->RegisterMissionListener(deviceId, nullptr);

    // 8. UnRegisterMissionListener(string, sptr<IRemoteMissionListener>)
    proxy->UnRegisterMissionListener(deviceId, nullptr);

    // 9. GetMissionInfos(string, int32_t, vector<MissionInfo>&)
    std::vector<MissionInfo> missionInfos;
    proxy->GetMissionInfos(deviceId, numMax, missionInfos);

    // 10. GetMissionInfo(string, int32_t, MissionInfo&)
    MissionInfo missionInfo;
    proxy->GetMissionInfo(deviceId, missionId, missionInfo);

    // 11. GetMissionSnapshot(string, int32_t, MissionSnapshot&, bool)
    MissionSnapshot snapshot;
    proxy->GetMissionSnapshot(deviceId, missionId, snapshot, isLowResolution);

    // 12. CleanMission(int32_t)
    proxy->CleanMission(missionId);

    // 13. CleanAllMissions()
    proxy->CleanAllMissions();

    // 14. MoveMissionToFront(int32_t)
    proxy->MoveMissionToFront(missionId);

    // 15. MoveMissionToFront(int32_t, StartOptions&)
    proxy->MoveMissionToFront(missionId, startOptions);

    // 16. MoveMissionsToForeground(vector<int32_t>&, int32_t)
    proxy->MoveMissionsToForeground(missionIds, topMissionId);

    // 17. MoveMissionsToBackground(vector<int32_t>&, vector<int32_t>&)
    std::vector<int32_t> moveResult;
    proxy->MoveMissionsToBackground(missionIds, moveResult);

    // 18. GetMissionIdByToken(sptr<IRemoteObject>)
    proxy->GetMissionIdByToken(nullptr);

    // 19. StartSyncRemoteMissions(string, bool, int64_t)
    proxy->StartSyncRemoteMissions(deviceId, fixConflict, tag);

    // 20. StopSyncRemoteMissions(string)
    proxy->StopSyncRemoteMissions(deviceId);

    // 21. SetMissionContinueState(sptr<IRemoteObject>, ContinueState)
    proxy->SetMissionContinueState(nullptr, continueState);

#ifdef SUPPORT_SCREEN
    // 21a. SetMissionLabel(sptr<IRemoteObject>, string)
    proxy->SetMissionLabel(nullptr, bundleName);

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
    proxy->PreStartMission(bundleName, moduleName, abilityName, startTime);

    // 24. TerminateMission(int32_t)
    proxy->TerminateMission(missionId);

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
