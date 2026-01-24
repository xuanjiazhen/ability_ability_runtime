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

#ifndef OHOS_AGENT_RUNTIME_AGENT_EXTENSION_SERVICEHOST_STUB_IMPL_H
#define OHOS_AGENT_RUNTIME_AGENT_EXTENSION_SERVICEHOST_STUB_IMPL_H

#include <string>
#include "agent_extension_host_stub.h"

namespace OHOS {
namespace AbilityRuntime {

// Forward declaration
class JSAgentExtensionConnection;

/**
 * @class AgentExtensionServiceHostStubImpl
 * Implementation of AgentExtensionHostStub that forwards IPC calls to JSAgentExtensionConnection.
 * Maintains a weak pointer to the connection to avoid circular references.
 */
class AgentExtensionServiceHostStubImpl : public AAFwk::AgentExtensionHostStub {
public:
    /**
     * Constructor.
     *
     * @param conn Weak pointer to the JSAgentExtensionConnection.
     */
    explicit AgentExtensionServiceHostStubImpl(wptr<JSAgentExtensionConnection> conn);
    ~AgentExtensionServiceHostStubImpl() = default;

    /**
     * Handle SendData request from agent extension.
     * Forwards the call to JSAgentExtensionConnection::OnSendData.
     *
     * @param data The string data received from agent extension.
     * @return Returns 0 on success, error code otherwise.
     */
    int32_t SendData(const std::string &data) override;

    /**
     * Handle Authorize request from agent extension.
     * Forwards the call to JSAgentExtensionConnection::OnAuthorize.
     *
     * @param data The authorization string received from agent extension.
     * @return Returns 0 on success, error code otherwise.
     */
    int32_t Authorize(const std::string &data) override;

protected:
    /**
     * Weak pointer to the JSAgentExtensionConnection.
     * Used weak pointer to avoid circular reference between stub and connection.
     */
    wptr<JSAgentExtensionConnection> conn_;
};

} // namespace AgentRuntime
} // namespace OHOS

#endif // OHOS_AGENT_RUNTIME_AGENT_EXTENSION_SERVICEHOST_STUB_IMPL_H
