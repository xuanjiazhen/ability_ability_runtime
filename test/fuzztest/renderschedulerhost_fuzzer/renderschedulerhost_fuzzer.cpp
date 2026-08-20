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

#include "renderschedulerhost_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>

#include "parcel.h"
#include "render_scheduler_host.h"
#include "securec.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
}

class RenderSchedulerHostFuzz : public RenderSchedulerHost {
public:
    RenderSchedulerHostFuzz() = default;
    ~RenderSchedulerHostFuzz() = default;

    void NotifyBrowserFd(int32_t ipcFd, int32_t sharedFd, int32_t crashFd, sptr<IRemoteObject> browser) override {}
};

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    FUZZ_EXTRACT_CODE(data, size);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(IRenderScheduler::Message::NOTIFY_BROWSER_FD);

    parcel.WriteInterfaceToken(FuzzUtil::Tokens::RENDER_SCHEDULER);

    switch (code % 1) {
        case 0:
            actualCode = static_cast<uint32_t>(IRenderScheduler::Message::NOTIFY_BROWSER_FD);
            parcel.WriteFileDescriptor(fdp.ConsumeIntegral<int32_t>());
            parcel.WriteFileDescriptor(fdp.ConsumeIntegral<int32_t>());
            parcel.WriteFileDescriptor(fdp.ConsumeIntegral<int32_t>());
            parcel.WriteRemoteObject(nullptr);
            break;
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<RenderSchedulerHost> stub = std::make_shared<RenderSchedulerHostFuzz>();
    stub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
}

FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
