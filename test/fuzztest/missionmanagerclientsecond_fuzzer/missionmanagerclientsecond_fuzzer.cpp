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

#include "missionmanagerclientsecond_fuzzer.h"
#include "fuzz_util.h"

#include <fuzzer/FuzzedDataProvider.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "dms_continueInfo.h"
#include "mission_info.h"
#include "mission_manager_client.h"
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
    std::string srcBundleName = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    std::string continueType = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    int32_t missionId = fdp.ConsumeIntegral<int32_t>();
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    bool isLowResolution = fdp.ConsumeBool();
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

    auto &client = MissionManagerClient::GetInstance();

    // 1. ContinueMission(ContinueMissionInfo, sptr<IRemoteObject>)
    client.ContinueMission(continueMissionInfo, nullptr);

    // 2. RegisterMissionListener(sptr<IMissionListener>, int32_t)
    client.RegisterMissionListener(nullptr, userId);

    // 3. UnRegisterMissionListener(sptr<IMissionListener>, int32_t)
    client.UnRegisterMissionListener(nullptr, userId);

    // 4. RegisterMissionListener(string, sptr<IRemoteMissionListener>)
    client.RegisterMissionListener(deviceId, nullptr);

    // 5. UnRegisterMissionListener(string, sptr<IRemoteMissionListener>)
    client.UnRegisterMissionListener(deviceId, nullptr);

    // 6. GetMissionSnapshot(string, int32_t, MissionSnapshot&, bool, int32_t)
    MissionSnapshot snapshot;
    client.GetMissionSnapshot(deviceId, missionId, snapshot, isLowResolution, userId);

    // 7. MoveMissionToFront(int32_t, StartOptions&)
    client.MoveMissionToFront(missionId, startOptions);

    // 8. SetMissionContinueState(sptr<IRemoteObject>, ContinueState, sptr<IRemoteObject>, int32_t)
    client.SetMissionContinueState(nullptr, continueState, nullptr, userId);

    // 9. CleanAllMissions(int32_t)
    client.CleanAllMissions(userId);

#ifdef SUPPORT_SCREEN
    // 10. SetMissionLabel(sptr<IRemoteObject>, string, int32_t)
    std::string label = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    client.SetMissionLabel(nullptr, label, userId);

    // 11. SetMissionIcon(sptr<IRemoteObject>, shared_ptr<PixelMap>, int32_t)
    client.SetMissionIcon(nullptr, nullptr, userId);

    // 12. UpdateMissionSnapShot(sptr<IRemoteObject>, shared_ptr<PixelMap>)
    client.UpdateMissionSnapShot(nullptr, nullptr);
#endif

    // 13. IsValidMissionIds(vector<int32_t>&, vector<MissionValidResult>&, int32_t)
    std::vector<MissionValidResult> results;
    client.IsValidMissionIds(missionIds, results, userId);

    // 14. Release()
    client.Release();

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