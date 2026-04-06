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

#include "cli_manager_scheduler_stub.h"

#include "message_parcel.h"

namespace OHOS {
namespace AbilityRuntime {

namespace {
constexpr int32_t ERROR_CODE = -1;
}

int32_t CliManagerSchedulerStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        return ERROR_CODE;
    }

    switch (static_cast<CliManagerSchedulerCmd>(code)) {
        case CliManagerSchedulerCmd::SCHEDULE_SESSION_EVENT: {
            std::string sessionId;
            std::string subscriptionId;
            if (!data.ReadString(sessionId) || !data.ReadString(subscriptionId)) {
                return ERROR_CODE;
            }
            std::unique_ptr<ToolEvent> event(data.ReadParcelable<ToolEvent>());
            if (!event) {
                return ERROR_CODE;
            }
            int32_t ret = ScheduleSessionEvent(sessionId, subscriptionId, *event);
            reply.WriteInt32(ret);
            return ret;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

} // namespace AbilityRuntime
} // namespace OHOS
