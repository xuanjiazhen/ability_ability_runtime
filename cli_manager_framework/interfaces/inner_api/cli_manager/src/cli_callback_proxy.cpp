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

#include "cli_callback_proxy.h"

#include "message_parcel.h"

namespace OHOS {
namespace AbilityRuntime {

namespace {
constexpr int32_t ERROR_CODE = -1;
}

CliCallbackProxy::CliCallbackProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<ICliCallback>(impl)
{}

int32_t CliCallbackProxy::OnOutput(const std::string &sessionId, const ToolEvent &event)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor()) || !data.WriteString(sessionId) || !data.WriteParcelable(&event)) {
        return ERROR_CODE;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        return ERROR_CODE;
    }
    return remote->SendRequest(static_cast<uint32_t>(CliCallbackCmd::ON_OUTPUT), data, reply, option);
}

int32_t CliCallbackProxy::OnExit(const std::string &sessionId, int32_t exitCode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor()) || !data.WriteString(sessionId) || !data.WriteInt32(exitCode)) {
        return ERROR_CODE;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        return ERROR_CODE;
    }
    return remote->SendRequest(static_cast<uint32_t>(CliCallbackCmd::ON_EXIT), data, reply, option);
}

} // namespace AbilityRuntime
} // namespace OHOS
