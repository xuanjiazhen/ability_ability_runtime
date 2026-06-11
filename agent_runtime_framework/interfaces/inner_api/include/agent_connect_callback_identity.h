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

#ifndef OHOS_AGENT_RUNTIME_AGENT_CONNECT_CALLBACK_IDENTITY_H
#define OHOS_AGENT_RUNTIME_AGENT_CONNECT_CALLBACK_IDENTITY_H

namespace OHOS {
namespace AgentRuntime {
enum class AgentConnectCallbackKind {
    JS,
    ETS,
};

/**
 * @class AgentConnectCallbackIdentity
 * @brief Exposes the developer callback identity hidden behind JS/ETS native wrappers.
 */
class AgentConnectCallbackIdentity {
public:
    virtual ~AgentConnectCallbackIdentity() = default;

    /**
     * @brief Returns the native wrapper family used for safe no-RTTI comparison.
     */
    virtual AgentConnectCallbackKind GetAgentConnectCallbackKind() const = 0;

    /**
     * @brief Checks whether another identity points to the same developer callback object.
     */
    virtual bool IsSameAgentConnectCallback(const AgentConnectCallbackIdentity &other) const = 0;
};
} // namespace AgentRuntime
} // namespace OHOS

#endif // OHOS_AGENT_RUNTIME_AGENT_CONNECT_CALLBACK_IDENTITY_H
