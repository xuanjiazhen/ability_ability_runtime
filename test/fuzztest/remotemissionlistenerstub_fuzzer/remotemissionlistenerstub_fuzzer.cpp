/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "fuzz_util.h"

#include <cstddef>
#include <cstdint>

#include "remote_mission_listener_stub.h"
#include "message_parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;

namespace OHOS {
class RemoteMissionListenerStubFuzzTest : public RemoteMissionListenerStub {
public:
    RemoteMissionListenerStubFuzzTest() = default;
    virtual ~RemoteMissionListenerStubFuzzTest()
    {};
    void NotifyMissionsChanged(const std::string& deviceId) override
    {}
    void NotifySnapshot(const std::string& deviceId, int32_t missionId) override
    {}
    void NotifyNetDisconnect(const std::string& deviceId, int32_t state) override
    {}
};

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    uint32_t code = FuzzUtil::GetU32Data(data);

    MessageParcel parcel;
    parcel.WriteInterfaceToken(std::u16string(FuzzUtil::Tokens::REMOTE_MISSION_LISTENER));
    parcel.WriteBuffer(data, size);
    parcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    std::shared_ptr<RemoteMissionListenerStub> remotemissionlistenerstub
        = std::make_shared<RemoteMissionListenerStubFuzzTest>();

    remotemissionlistenerstub->OnRemoteRequest(code, parcel, reply, option);

    return true;
}
}

FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
