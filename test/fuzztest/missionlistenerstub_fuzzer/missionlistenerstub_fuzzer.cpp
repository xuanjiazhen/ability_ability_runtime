/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
#include "attack_vectors.h"
#include "fuzz_util.h"
#include "mission_listener_stub.h"

#include "missionlistenerstub_fuzzer.h"

using namespace OHOS::AAFwk;

namespace OHOS {
class MissionListenerStubFuzz : public MissionListenerStub {
public:
    MissionListenerStubFuzz() = default;
    ~MissionListenerStubFuzz() = default;
    void OnMissionCreated(int32_t missionId) override {}
    void OnMissionDestroyed(int32_t missionId) override {}
    void OnMissionSnapshotChanged(int32_t missionId) override {}
    void OnMissionMovedToFront(int32_t missionId) override {}
    void OnMissionMovedToBackground(int32_t missionId) override {}
    void OnMissionIconUpdated(int32_t missionId, const std::shared_ptr<OHOS::Media::PixelMap> &icon) override {}
    void OnMissionClosed(int32_t missionId) override {}
    void OnMissionLabelUpdated(int32_t missionId) override {}
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 10) {
        case 0: {
            actualCode = static_cast<uint32_t>(IMissionListener::ON_MISSION_CREATED);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(IMissionListener::ON_MISSION_DESTROYED);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(IMissionListener::ON_MISSION_SNAPSHOT_CHANGED);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(IMissionListener::ON_MISSION_MOVED_TO_FRONT);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(IMissionListener::ON_MISSION_ICON_UPDATED);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 5: {
            actualCode = static_cast<uint32_t>(IMissionListener::ON_MISSION_CLOSED);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 6: {
            actualCode = static_cast<uint32_t>(IMissionListener::ON_MISSION_LABEL_UPDATED);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 7: {
            actualCode = static_cast<uint32_t>(IMissionListener::ON_MISSION_FOCUSED);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 8: {
            actualCode = static_cast<uint32_t>(IMissionListener::ON_MISSION_UNFOCUSED);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 9: {
            actualCode = static_cast<uint32_t>(IMissionListener::ON_MISSION_MOVED_TO_BACKGROUND);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(MissionListenerStubFuzz, FuzzUtil::Tokens::MISSION_LISTENER)
} // namespace OHOS
