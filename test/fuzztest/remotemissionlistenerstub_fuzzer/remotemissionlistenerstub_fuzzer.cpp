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
#include "remotemissionlistenerstub_fuzzer.h"
#include "attack_vectors.h"
#include "fuzz_util.h"

#include "remote_mission_listener_stub.h"

using namespace OHOS::AAFwk;

namespace OHOS {
class RemoteMissionListenerStubFuzz : public RemoteMissionListenerStub {
public:
    RemoteMissionListenerStubFuzz() = default;
    ~RemoteMissionListenerStubFuzz() = default;
    void NotifyMissionsChanged(const std::string &deviceId) override {}
    void NotifySnapshot(const std::string &deviceId, int32_t missionId) override {}
    void NotifyNetDisconnect(const std::string &deviceId, int32_t state) override {}
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 3) {
        case 0: {
            actualCode = static_cast<uint32_t>(IRemoteMissionListener::NOTIFY_MISSION_CHANGED);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(IRemoteMissionListener::NOTIFY_SNAPSHOT);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(IRemoteMissionListener::NOTIFY_NET_DISCONNECT);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(RemoteMissionListenerStubFuzz, FuzzUtil::Tokens::REMOTE_MISSION_LISTENER)
} // namespace OHOS
