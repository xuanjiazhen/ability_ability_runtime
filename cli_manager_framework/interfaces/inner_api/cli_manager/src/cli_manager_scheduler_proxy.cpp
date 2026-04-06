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

#include "cli_manager_scheduler_proxy.h"

#include "message_parcel.h"

namespace OHOS {
namespace AbilityRuntime {

namespace {
constexpr int32_t ERROR_CODE = -1;
}

CliManagerSchedulerProxy::CliManagerSchedulerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<ICliManagerScheduler>(impl)
{}

int32_t CliManagerSchedulerProxy::ScheduleSessionEvent(const std::string &sessionId, const std::string &subscriptionId,
    const ToolEvent &event)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor()) || !data.WriteString(sessionId) ||
        !data.WriteString(subscriptionId) || !data.WriteParcelable(&event)) {
        return ERROR_CODE;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return ERROR_CODE;
    }

    int32_t ret = remote->SendRequest(static_cast<uint32_t>(CliManagerSchedulerCmd::SCHEDULE_SESSION_EVENT), data,
        reply, option);
    if (ret != 0) {
        return ret;
    }

    int32_t finalResult = ERROR_CODE;
    if (!reply.ReadInt32(finalResult)) {
        return ERROR_CODE;
    }
    return finalResult;
}

} // namespace AbilityRuntime
} // namespace OHOS
