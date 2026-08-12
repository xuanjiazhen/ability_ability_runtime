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

#include "sessionhandlerstub_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "session_handler_stub.h"
#include "message_parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string SESSION_HANDLER_INTERFACE_TOKEN = u"ohos.aafwk.SessionHandler";
}

class SessionHandlerStubFuzz : public SessionHandlerStub {};

bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    FUZZ_EXTRACT_CODE(data, size);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(ISessionHandler::ON_SESSION_MOVED_TO_FRONT);

    parcel.WriteInterfaceToken(SESSION_HANDLER_INTERFACE_TOKEN);

    switch (code % 1) {
        case 0:
            actualCode = static_cast<uint32_t>(ISessionHandler::ON_SESSION_MOVED_TO_FRONT);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<SessionHandlerStub> stub = std::make_shared<SessionHandlerStubFuzz>();
    stub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
} // namespace OHOS

FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
