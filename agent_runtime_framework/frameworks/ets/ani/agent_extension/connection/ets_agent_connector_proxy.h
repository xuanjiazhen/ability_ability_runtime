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

#ifndef OHOS_AGENT_RUNTIME_ETS_AGENT_CONNECTOR_PROXY_H
#define OHOS_AGENT_RUNTIME_ETS_AGENT_CONNECTOR_PROXY_H

#include <memory>

#include "ani.h"
#include "ets_runtime.h"
#include "agent_connector_proxy.h"

namespace OHOS {
namespace AgentRuntime {
using namespace AbilityRuntime;

/**
 * @class EtsAgentConnectorProxy
 * ETS/ArkTS layer proxy for IAgentConnector interface.
 * Provides bridge between ArkTS code and native IPC implementation.
 * This proxy is used in the Extension process to send data to the Host.
 *
 * Corresponds to the IAgentConnector interface (Host calls Extension).
 */
class EtsAgentConnectorProxy {
public:
    /**
     * Create an ETS proxy object for IAgentConnector.
     *
     * @param env The ANI environment pointer.
     * @param impl The remote object implementation of IAgentConnector.
     * @return The created ArkTS object, or nullptr on failure.
     */
    static ani_object CreateEtsAgentConnectorProxy(ani_env *env, const sptr<IRemoteObject> &impl);

    /**
     * Create an empty proxy object for error handling.
     *
     * @param env The ANI environment pointer.
     * @return The created empty ArkTS object, or nullptr on failure.
     */
    static ani_object CreateEmptyProxyObject(ani_env *env);

    /**
     * Constructor.
     *
     * @param impl The remote object implementation of IAgentConnector.
     */
    explicit EtsAgentConnectorProxy(const sptr<IRemoteObject> &impl);

    /**
     * Destructor.
     */
    virtual ~EtsAgentConnectorProxy();

    /**
     * Get the C++ proxy object from ArkTS object field.
     *
     * @param env The ANI environment pointer.
     * @param obj The ArkTS object containing the native proxy pointer.
     * @return The C++ proxy object pointer, or nullptr on failure.
     */
    static EtsAgentConnectorProxy* GetEtsAgentConnectorProxy(ani_env *env, ani_object obj);

    /**
     * Native method binding for SendData.
     * Called from ArkTS when agent sends data to host.
     *
     * @param env The ANI environment pointer.
     * @param obj The ArkTS object (this).
     * @param data The string data to send (as ani_string).
     */
    static void SendData(ani_env *env, ani_object obj, ani_string data);

    /**
     * Native method binding for Authorize.
     * Called from ArkTS when agent sends authorization to host.
     *
     * @param env The ANI environment pointer.
     * @param obj The ArkTS object (this).
     * @param data The authorization string data (as ani_string).
     */
    static void Authorize(ani_env *env, ani_object obj, ani_string data);

private:
    /**
     * Internal implementation of SendData.
     *
     * @param env The ANI environment pointer.
     * @param data The string data to send.
     */
    void OnSendData(ani_env *env, const std::string &data);

    /**
     * Internal implementation of Authorize.
     *
     * @param env The ANI environment pointer.
     * @param data The authorization string data.
     */
    void OnAuthorize(ani_env *env, const std::string &data);

protected:
    sptr<OHOS::AgentRuntime::IAgentConnector> proxy_ = nullptr;
};

} // namespace AgentRuntime
} // namespace OHOS
#endif // OHOS_AGENT_RUNTIME_ETS_AGENT_CONNECTOR_PROXY_H
