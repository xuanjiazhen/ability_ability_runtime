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

#ifndef UNITTEST_OHOS_AGENT_RUNTIME_JS_AGENT_RECEIVER_PROXY_TEST_MOCK_H
#define UNITTEST_OHOS_AGENT_RUNTIME_JS_AGENT_RECEIVER_PROXY_TEST_MOCK_H

#include <gmock/gmock.h>
#include <iremote_object.h>
#include <iremote_broker.h>
#include "iagent_receiver.h"
#include "ability_business_error.h"

namespace OHOS {
namespace AgentRuntime {

class MockRemoteObjectForJsReceiver : public IRemoteObject {
public:
    MockRemoteObjectForJsReceiver() : IRemoteObject(u"mock_descriptor") {}
    virtual ~MockRemoteObjectForJsReceiver() = default;

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
        return ERR_OK;
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

class MockIAgentReceiver : public IAgentReceiver {
public:
    MockIAgentReceiver() = default;
    virtual ~MockIAgentReceiver() = default;

    MOCK_METHOD2(SendData, int32_t(const sptr<IRemoteObject> &connectorProxy, const std::string &data));
    MOCK_METHOD2(Authorize, int32_t(const sptr<IRemoteObject> &connectorProxy, const std::string &data));

    sptr<IRemoteObject> AsObject()
    {
        return remoteObject_;
    }

    int32_t SendData(const sptr<IRemoteObject> &connectorProxy, const std::string &data) override
    {
        return SendData(connectorProxy, data);
    }

    int32_t Authorize(const sptr<IRemoteObject> &connectorProxy, const std::string &data) override
    {
        return Authorize(connectorProxy, data);
    }

private:
    sptr<IRemoteObject> remoteObject_ = new MockRemoteObjectForJsReceiver();
};

} // namespace AgentRuntime
} // namespace OHOS

#endif // UNITTEST_OHOS_AGENT_RUNTIME_JS_AGENT_RECEIVER_PROXY_TEST_MOCK_H
