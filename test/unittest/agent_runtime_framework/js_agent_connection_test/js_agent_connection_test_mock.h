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

#ifndef UNITTEST_OHOS_AGENT_RUNTIME_JS_AGENT_CONNECTION_TEST_MOCK_H
#define UNITTEST_OHOS_AGENT_RUNTIME_JS_AGENT_CONNECTION_TEST_MOCK_H

#define private public
#include "js_agent_connection.h"
#undef private

#include <gmock/gmock.h>
#include <iremote_object.h>
#include <native_engine/native_reference.h>

namespace OHOS {
namespace AgentRuntime {

class MockRemoteObjectForTest : public IRemoteObject {
public:
    MockRemoteObjectForTest() : IRemoteObject(u"mock_descriptor") {}
    virtual ~MockRemoteObjectForTest() = default;

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

class NapiAsyncTaskTestMock : public OHOS::AbilityRuntime::NapiAsyncTask {
public:
    NapiAsyncTaskTestMock() : NapiAsyncTask(nullptr, nullptr, nullptr) {}
    virtual ~NapiAsyncTaskTestMock() = default;

    void ResolveWithNoError(napi_env env, napi_value result) override
    {
        // Mock implementation - do nothing
    }

    void Reject(napi_env env, napi_value error) override
    {
        // Mock implementation - do nothing
    }
};

class JSAgentConnectionTestMock : public JSAgentConnection {
public:
    explicit JSAgentConnectionTestMock(napi_env env) : JSAgentConnection(env) {}
    virtual ~JSAgentConnectionTestMock() = default;

    // Expose protected methods for testing
    using JSAgentConnection::HandleOnSendData;
    using JSAgentConnection::HandleOnAuthorize;
};

} // namespace AgentRuntime
} // namespace OHOS

#endif // UNITTEST_OHOS_AGENT_RUNTIME_JS_AGENT_CONNECTION_TEST_MOCK_H
