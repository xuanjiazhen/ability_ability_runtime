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

#ifndef UNITTEST_OHOS_AGENT_RUNTIME_JS_AGENT_CONNECTOR_STUB_IMPL_TEST_MOCK_H
#define UNITTEST_OHOS_AGENT_RUNTIME_JS_AGENT_CONNECTOR_STUB_IMPL_TEST_MOCK_H

#define private public
#include "js_agent_connection.h"
#undef private

#include <gmock/gmock.h>
#include <iremote_object.h>
#include "ability_business_error.h"

namespace OHOS {
namespace AgentRuntime {

class JSAgentConnectionTestMockImpl : public JSAgentConnection {
public:
    explicit JSAgentConnectionTestMockImpl(napi_env env) : JSAgentConnection(env) {}
    virtual ~JSAgentConnectionTestMockImpl() = default;

    int32_t OnSendData(const std::string &data) override
    {
        return 0;
    }

    int32_t OnAuthorize(const std::string &data) override
    {
        return 0;
    }

    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject,
        int resultCode) override
    {
        // Mock implementation
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override
    {
        // Mock implementation
    }
};

} // namespace AgentRuntime
} // namespace OHOS

#endif // UNITTEST_OHOS_AGENT_RUNTIME_JS_AGENT_CONNECTOR_STUB_IMPL_TEST_MOCK_H
