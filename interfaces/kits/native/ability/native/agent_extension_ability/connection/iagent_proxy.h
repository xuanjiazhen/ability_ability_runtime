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

#ifndef OHOS_AGENT_RUNTIME_IAGENT_PROXY_H
#define OHOS_AGENT_RUNTIME_IAGENT_PROXY_H

#include <iremote_broker.h>
#include <string>

namespace OHOS {
namespace AgentRuntime {

/**
 * @interface IAgentProxy
 * Interface for callbacks FROM agent extension to host application.
 * Corresponds to the JS AgentProxy object.
 */
class IAgentProxy : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.agentruntime.IAgentProxy");

    /**
     * Called when agent extension sends data to host.
     *
     * @param data The string data sent from agent extension.
     * @return Returns 0 on success, error code otherwise.
     */
    virtual int32_t OnSendData(const std::string &data) = 0;

    /**
     * Called when agent extension sends authentication to host.
     *
     * @param data The authentication data sent from agent extension.
     * @return Returns 0 on success, error code otherwise.
     */
    virtual int32_t OnAuthorize(const std::string &data) = 0;

    enum {
        /**
         * Command code for OnSendData.
         */
        ON_SEND_DATA = 1,

        /**
         * Command code for OnAuthorize.
         */
        ON_AUTHORIZE = 2,
    };
};

} // namespace AgentRuntime
} // namespace OHOS

#endif // OHOS_AGENT_RUNTIME_IAGENT_PROXY_H
