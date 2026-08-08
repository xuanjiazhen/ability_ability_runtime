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

#include "missionmanagerclient_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "mission_info.h"
#include "mission_manager_client.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace {
constexpr size_t STRING_MAX_LENGTH = 256;
constexpr size_t MISSION_VEC_MAX_SIZE = 8;

// 通过 FuzzedDataProvider 构造 int32_t 向量，长度受控避免内存爆炸。
std::vector<int32_t> BuildFuzzInt32Vector(FuzzedDataProvider &fdp)
{
    std::vector<int32_t> vec;
    uint8_t count = fdp.ConsumeIntegral<uint8_t>() % MISSION_VEC_MAX_SIZE;
    for (uint8_t i = 0; i < count; i++) {
        vec.push_back(fdp.ConsumeIntegral<int32_t>());
    }
    return vec;
}
} // namespace

namespace OHOS {
bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return false;
    }

    FuzzedDataProvider fdp(data, size);

    // 消费所有 fuzz 输入参数，先一次性消费避免不同方法间数据相互依赖。
    std::string srcDeviceId = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::string dstDeviceId = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::string deviceId = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::string moduleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::string abilityName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::string startTime = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    int32_t missionId = fdp.ConsumeIntegral<int32_t>();
    int32_t numMax = fdp.ConsumeIntegral<int32_t>();
    int32_t topMissionId = fdp.ConsumeIntegral<int32_t>();
    bool fixConflict = fdp.ConsumeBool();
    int64_t tag = fdp.ConsumeIntegral<int64_t>();
    WantParams wantParams;
    std::vector<int32_t> missionIds = BuildFuzzInt32Vector(fdp);

    auto &client = MissionManagerClient::GetInstance();

    // 1. ContinueMission(string, string, int32_t, sptr<IRemoteObject>, WantParams&)
    client.ContinueMission(srcDeviceId, dstDeviceId, missionId, nullptr, wantParams);

    // 2. LockMissionForCleanup(int32_t)
    client.LockMissionForCleanup(missionId);

    // 3. UnlockMissionForCleanup(int32_t)
    client.UnlockMissionForCleanup(missionId);

    // 4. GetMissionInfos(string, int32_t, vector<MissionInfo>&)
    std::vector<MissionInfo> missionInfos;
    client.GetMissionInfos(deviceId, numMax, missionInfos);

    // 5. GetMissionInfo(string, int32_t, MissionInfo&)
    MissionInfo missionInfo;
    client.GetMissionInfo(deviceId, missionId, missionInfo);

    // 6. CleanMission(int32_t)
    client.CleanMission(missionId);

    // 7. MoveMissionToFront(int32_t)
    client.MoveMissionToFront(missionId);

    // 8. MoveMissionsToForeground(vector<int32_t>&, int32_t)
    client.MoveMissionsToForeground(missionIds, topMissionId);

    // 9. MoveMissionsToBackground(vector<int32_t>&, vector<int32_t>&)
    std::vector<int32_t> moveResult;
    client.MoveMissionsToBackground(missionIds, moveResult);

    // 10. GetMissionIdByToken(sptr<IRemoteObject>, int32_t&)
    int32_t outMissionId = 0;
    client.GetMissionIdByToken(nullptr, outMissionId);

    // 11. StartSyncRemoteMissions(string, bool, int64_t)
    client.StartSyncRemoteMissions(deviceId, fixConflict, tag);

    // 12. StopSyncRemoteMissions(string)
    client.StopSyncRemoteMissions(deviceId);

    // 13. PreStartMission(string, string, string, string)
    client.PreStartMission(bundleName, moduleName, abilityName, startTime);

    // 14. TerminateMission(int32_t)
    client.TerminateMission(missionId);

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
