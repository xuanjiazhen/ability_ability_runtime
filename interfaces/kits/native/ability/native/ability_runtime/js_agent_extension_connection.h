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

#ifndef OHOS_AGENT_RUNTIME_JS_AGENT_EXTENSION_CONNECTION_H
#define OHOS_AGENT_RUNTIME_JS_AGENT_EXTENSION_CONNECTION_H

#include "js_ability_context.h"

namespace OHOS {
namespace AgentRuntime {
class JSAgentExtensionConnection;

namespace AgentExtensionConnection {
/**
 * Remove agent extension connection from registry.
 *
 * @param connectId The connection ID to remove.
 */
void RemoveAgentExtensionConnection(int64_t connectId);

/**
 * Insert agent extension connection into registry.
 *
 * @param connection The connection object to insert.
 * @param want The want information for the connection.
 * @return Returns the connection ID.
 */
int64_t InsertAgentExtensionConnection(sptr<JSAgentExtensionConnection> connection,
    const AAFwk::Want &want);

/**
 * Find agent extension connection by ID.
 *
 * @param connectId The connection ID to find.
 * @param want Output parameter for the want information.
 * @param connection Output parameter for the found connection.
 */
void FindAgentExtensionConnection(const int64_t &connectId, AAFwk::Want &want,
    sptr<JSAgentExtensionConnection> &connection);

/**
 * Find agent extension connection by want and callback.
 *
 * @param env The N-API environment.
 * @param want The want information to match.
 * @param callback The callback object to match.
 * @param connection Output parameter for the found connection.
 */
void FindAgentExtensionConnection(napi_env env, AAFwk::Want &want, napi_value callback,
    sptr<JSAgentExtensionConnection> &connection);
}

class AgentHostProxyStubImpl;

/**
 * @class JSAgentExtensionConnection
 * Connection class for agent extension that bridges JS callbacks and native IPC.
 * Manages the connection lifecycle between JavaScript and Agent Extension.
 */
class JSAgentExtensionConnection : public JSAbilityConnection {
public:
    /**
     * Constructor.
     *
     * @param env The N-API environment.
     */
    JSAgentExtensionConnection(napi_env env);

    /**
     * Destructor.
     */
    ~JSAgentExtensionConnection();

    /**
     * Called when agent extension connection succeeds.
     * Creates the JsAgentProxy and resolves the promise.
     *
     * @param element The element name of the connected extension.
     * @param remoteObject The remote object for IPC communication.
     * @param resultCode The result code of the connection.
     */
    virtual void HandleOnAbilityConnectDone(
        const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject,
        int resultCode) override;

    /**
     * Called when agent extension connection fails or disconnects.
     * Rejects pending tasks and calls onDisconnect callback.
     *
     * @param element The element name of the extension.
     * @param resultCode The result code of disconnection.
     */
    virtual void HandleOnAbilityDisconnectDone(
        const AppExecFwk::ElementName &element,
        int resultCode) override;

    /**
     * Get the service host stub.
     *
     * @return Returns the service host stub.
     */
    sptr<AgentHostProxyStubImpl> GetServiceHostStub() { return serviceHostStub_; }

    /**
     * Set the proxy object returned to JavaScript.
     *
     * @param proxy The N-API value of the proxy object.
     */
    void SetProxyObject(napi_value proxy);

    /**
     * Get the proxy object.
     *
     * @return Returns the N-API value of the proxy object, or nullptr if not set.
     */
    napi_value GetProxyObject();

    /**
     * Set the async task for the connection promise.
     *
     * @param task The shared pointer to the async task.
     */
    void SetNapiAsyncTask(std::shared_ptr<NapiAsyncTask> &task);

    /**
     * Add a duplicated pending task.
     * Used when multiple connection requests are made for the same extension.
     *
     * @param task The unique pointer to the pending async task.
     */
    void AddDuplicatedPendingTask(std::unique_ptr<NapiAsyncTask> &task);

    /**
     * Resolve all duplicated pending tasks with the proxy.
     *
     * @param env The N-API environment.
     * @param proxy The proxy object to resolve with.
     */
    void ResolveDuplicatedPendingTask(napi_env env, napi_value proxy);

    /**
     * Reject all duplicated pending tasks with error.
     *
     * @param env The N-API environment.
     * @param error The error object to reject with.
     */
    void RejectDuplicatedPendingTask(napi_env env, napi_value error);

    /**
     * Called when agent extension sends data.
     * Schedules async task to call JavaScript onData callback.
     *
     * @param data The string data received from agent extension.
     * @return Returns 0 on success, error code otherwise.
     */
    int32_t OnSendData(const std::string &data);

    /**
     * Handle the data received from agent extension.
     * Called on JS thread to invoke the onData callback.
     *
     * @param data The string data received.
     */
    void HandleOnSendData(const std::string &data);

    /**
     * Called when agent extension sends authorization.
     * Schedules async task to call JavaScript onAuth callback.
     *
     * @param data The authorization string received from agent extension.
     * @return Returns 0 on success, error code otherwise.
     */
    int32_t OnAuthorize(const std::string &data);

    /**
     * Handle the authorization received from agent extension.
     * Called on JS thread to invoke the onAuth callback.
     *
     * @param data The authorization string received.
     */
    void HandleOnAuthorize(const std::string &data);

    /**
     * Call the onDisconnect method on the JavaScript callback object.
     */
    void CallJsOnDisconnect();

    /**
     * Check if two JavaScript callback objects are equal.
     *
     * @param env The N-API environment.
     * @param callback The first callback reference.
     * @param value The second callback N-API value.
     * @return Returns true if they are equal, false otherwise.
     */
    static bool IsJsCallbackObjectEquals(napi_env env,
        std::unique_ptr<NativeReference> &callback, napi_value value);

private:
    /**
     * The service host stub for handling incoming IPC from agent extension.
     */
    sptr<AgentHostProxyStubImpl> serviceHostStub_;

    /**
     * The async task for the connection promise.
     */
    std::shared_ptr<NapiAsyncTask> napiAsyncTask_;

    /**
     * The proxy object returned to JavaScript.
     */
    std::unique_ptr<NativeReference> serviceProxyObject_;

    /**
     * List of pending tasks for duplicate connection requests.
     */
    std::vector<std::unique_ptr<NapiAsyncTask>> duplicatedPendingTaskList_;
};

} // namespace AgentRuntime
} // namespace OHOS

#endif // OHOS_AGENT_RUNTIME_JS_AGENT_EXTENSION_CONNECTION_H
