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

#ifndef UNITTEST_OHOS_AGENT_RUNTIME_AGENT_RECEIVER_PROXY_TEST_MOCK_H
#define UNITTEST_OHOS_AGENT_RUNTIME_AGENT_RECEIVER_PROXY_TEST_MOCK_H

#include <gmock/gmock.h>
#include <iremote_object.h>
#include <iremote_proxy.h>
#include "ipc_types.h"

namespace OHOS {
namespace AgentRuntime {

class MockRemoteObject : public IRemoteObject {
public:
    MockRemoteObject() : IRemoteObject(u"mock_descriptor") {}
    virtual ~MockRemoteObject() = default;

    MOCK_METHOD4(SendRequest, int(uint32_t code, MessageParcel& data,
        MessageParcel& reply, MessageOption& option));

    int32_t GetObjectRefCount() override
    {
        return 0;
    }

    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
        MessageOption& option) override
    {
        return SendRequest(code, data, reply, option);
    }

    bool IsProxyObject() const override
    {
        return true;
    }

    bool IsObjectDead() const override
    {
        return false;
    }

    std::u16string GetObjectDescriptor() const
    {
        return u"mock_descriptor";
    }
};

} // namespace AgentRuntime
} // namespace OHOS

#endif // UNITTEST_OHOS_AGENT_RUNTIME_AGENT_RECEIVER_PROXY_TEST_MOCK_H
