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

#include "js_agent_extension_connection.h"

#include <mutex>

#include "ability_business_error.h"
#include "hilog_tag_wrapper.h"
#include "js_agent_connector_stub_impl.h"
#include "js_agent_receiver_proxy.h"
#include "js_error_utils.h"

namespace OHOS {
namespace AgentRuntime {

namespace {
constexpr int32_t ARGC_ONE = 1;
// Registry for agent extension connections
static std::map<ConnectionKey, sptr<JSAgentExtensionConnection>, KeyCompare> g_agentExtensionConnects;
static std::recursive_mutex g_agentExtensionConnectsLock_;
static int64_t g_agentExtensionSerialNumber = 0;
} // namespace

namespace AgentExtensionConnection {

void RemoveAgentExtensionConnection(int64_t connectId)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "RemoveAgentExtensionConnection, connectId: %{public}ld", connectId);
    std::lock_guard<std::recursive_mutex> lock(g_agentExtensionConnectsLock_);
    auto item = std::find_if(g_agentExtensionConnects.begin(), g_agentExtensionConnects.end(),
        [&connectId](const auto &obj) {
            return connectId == obj.first.id;
        });
    if (item != g_agentExtensionConnects.end()) {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Found connection to remove");
        if (item->second) {
            item->second->RemoveConnectionObject();
        }
        g_agentExtensionConnects.erase(item);
    } else {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Connection not found");
    }
}

int64_t InsertAgentExtensionConnection(sptr<JSAgentExtensionConnection> connection,
    const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "InsertAgentExtensionConnection");
    std::lock_guard<std::recursive_mutex> lock(g_agentExtensionConnectsLock_);
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null connection");
        return -1;
    }
    int64_t connectId = g_agentExtensionSerialNumber;
    ConnectionKey key;
    key.id = g_agentExtensionSerialNumber;
    key.want = want;
    key.accountId = 0;
    connection->SetConnectionId(key.id);
    g_agentExtensionConnects.emplace(key, connection);
    if (g_agentExtensionSerialNumber < INT64_MAX) {
        g_agentExtensionSerialNumber++;
    } else {
        g_agentExtensionSerialNumber = 0;
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Connection inserted, id: %{public}ld", connectId);
    return connectId;
}

void FindAgentExtensionConnection(const int64_t &connectId, AAFwk::Want &want,
    sptr<JSAgentExtensionConnection> &connection)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "FindAgentExtensionConnection by id: %{public}ld", connectId);
    std::lock_guard<std::recursive_mutex> lock(g_agentExtensionConnectsLock_);
    auto item = std::find_if(g_agentExtensionConnects.begin(), g_agentExtensionConnects.end(),
        [&connectId](const auto &obj) {
            return connectId == obj.first.id;
        });
    if (item != g_agentExtensionConnects.end()) {
        want = item->first.want;
        connection = item->second;
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Found connection");
    } else {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Connection not found");
    }
}

void FindAgentExtensionConnection(napi_env env, AAFwk::Want &want, napi_value callback,
    sptr<JSAgentExtensionConnection> &connection)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "FindAgentExtensionConnection by want+callback");
    std::lock_guard<std::recursive_mutex> lock(g_agentExtensionConnectsLock_);
    auto item = std::find_if(g_agentExtensionConnects.begin(), g_agentExtensionConnects.end(),
        [&want, env, callback](const auto &obj) {
        bool wantEquals = (obj.first.want.GetElement() == want.GetElement());
        std::unique_ptr<NativeReference> &tempCallbackPtr = obj.second->GetJsConnectionObject();
        bool callbackObjectEquals =
            JSAgentExtensionConnection::IsJsCallbackObjectEquals(env, tempCallbackPtr, callback);
        return wantEquals && callbackObjectEquals;
    });
    if (item == g_agentExtensionConnects.end()) {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Connection not found");
        return;
    }
    connection = item->second;
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Found connection");
}

} // namespace AgentExtensionConnection

} // namespace AgentRuntime
} // namespace OHOS

namespace OHOS {
namespace AgentRuntime {

JSAgentExtensionConnection::JSAgentExtensionConnection(napi_env env) : JSAbilityConnection(env)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "JSAgentExtensionConnection constructor");
    wptr<JSAgentExtensionConnection> weakthis = this;
    serviceHostStub_ = sptr<JsAgentConnectorStubImpl>::MakeSptr(weakthis);
}

JSAgentExtensionConnection::~JSAgentExtensionConnection()
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "~JSAgentExtensionConnection destructor");
    serviceHostStub_ = nullptr;
    napiAsyncTask_ = nullptr;
    ReleaseNativeReference(serviceProxyObject_.release());
}

void JSAgentExtensionConnection::HandleOnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "HandleOnAbilityConnectDone, resultCode: %{public}d", resultCode);
    if (napiAsyncTask_ != nullptr) {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Creating JsAgentReceiverProxy");
        sptr<JsAgentConnectorStubImpl> hostStub = GetServiceHostStub();
        sptr<IRemoteObject> hostProxy = nullptr;
        if (hostStub != nullptr) {
            hostProxy = hostStub->AsObject();
        }
        napi_value proxy = AgentRuntime::JsAgentReceiverProxy::CreateJsAgentReceiverProxy(env_, remoteObject,
            connectionId_, hostProxy);
        SetProxyObject(proxy);
        napiAsyncTask_->ResolveWithNoError(env_, proxy);
        ResolveDuplicatedPendingTask(env_, proxy);
    } else {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "napiAsyncTask_ is null");
    }
    napiAsyncTask_ = nullptr;
}

void JSAgentExtensionConnection::HandleOnAbilityDisconnectDone(
    const AppExecFwk::ElementName &element, int resultCode)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "HandleOnAbilityDisconnectDone, resultCode: %{public}d", resultCode);
    if (napiAsyncTask_ != nullptr) {
        napi_value innerError = CreateJsError(env_, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        napiAsyncTask_->Reject(env_, innerError);
        RejectDuplicatedPendingTask(env_, innerError);
        napiAsyncTask_ = nullptr;
    }

    CallJsOnDisconnect();
    AgentExtensionConnection::RemoveAgentExtensionConnection(connectionId_);
}

void JSAgentExtensionConnection::SetProxyObject(napi_value proxy)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SetProxyObject");
    serviceProxyObject_.reset();
    if (proxy != nullptr) {
        napi_ref ref = nullptr;
        napi_create_reference(env_, proxy, 1, &ref);
        serviceProxyObject_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
    }
}

napi_value JSAgentExtensionConnection::GetProxyObject()
{
    if (serviceProxyObject_ == nullptr) {
        return nullptr;
    }
    return serviceProxyObject_->GetNapiValue();
}

void JSAgentExtensionConnection::SetNapiAsyncTask(std::shared_ptr<AbilityRuntime::NapiAsyncTask> &task)
{
    napiAsyncTask_ = task;
}

void JSAgentExtensionConnection::AddDuplicatedPendingTask(std::unique_ptr<AbilityRuntime::NapiAsyncTask> &task)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "AddDuplicatedPendingTask");
    duplicatedPendingTaskList_.push_back(std::move(task));
}

void JSAgentExtensionConnection::ResolveDuplicatedPendingTask(napi_env env, napi_value proxy)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "ResolveDuplicatedPendingTask, count: %{public}zu", duplicatedPendingTaskList_.size());
    for (auto &task : duplicatedPendingTaskList_) {
        if (task != nullptr) {
            task->ResolveWithNoError(env, proxy);
        }
    }
    duplicatedPendingTaskList_.clear();
}

void JSAgentExtensionConnection::RejectDuplicatedPendingTask(napi_env env, napi_value error)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "RejectDuplicatedPendingTask, count: %{public}zu", duplicatedPendingTaskList_.size());
    for (auto &task : duplicatedPendingTaskList_) {
        if (task != nullptr) {
            task->Reject(env, error);
        }
    }
    duplicatedPendingTaskList_.clear();
}

int32_t JSAgentExtensionConnection::OnSendData(const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnSendData called, data length: %{public}zu", data.length());
    wptr<JSAgentExtensionConnection> connection = this;
    std::unique_ptr<AbilityRuntime::NapiAsyncTask::CompleteCallback> complete =
        std::make_unique<AbilityRuntime::NapiAsyncTask::CompleteCallback>(
            [connection, data](napi_env env, AbilityRuntime::NapiAsyncTask &task, int32_t status) {
                sptr<JSAgentExtensionConnection> connectionSptr = connection.promote();
                if (!connectionSptr) {
                    TAG_LOGE(AAFwkTag::SER_ROUTER, "null connectionSptr");
                    return;
                }
                connectionSptr->HandleOnSendData(data);
            });

    napi_ref callback = nullptr;
    std::unique_ptr<AbilityRuntime::NapiAsyncTask::ExecuteCallback> execute = nullptr;
    AbilityRuntime::NapiAsyncTask::Schedule("JSAgentExtensionConnection::SendData",
        env_, std::make_unique<AbilityRuntime::NapiAsyncTask>(callback, std::move(execute), std::move(complete)));

    return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_OK);
}

void JSAgentExtensionConnection::HandleOnSendData(const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "HandleOnSendData called");
    napi_value argv[] = { CreateJsValue(env_, data) };
    CallObjectMethod("onData", argv, ARGC_ONE);
}

int32_t JSAgentExtensionConnection::OnAuthorize(const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnAuthorize called, data length: %{public}zu", data.length());
    wptr<JSAgentExtensionConnection> connection = this;
    std::unique_ptr<AbilityRuntime::NapiAsyncTask::CompleteCallback> complete =
        std::make_unique<AbilityRuntime::NapiAsyncTask::CompleteCallback>(
            [connection, data](napi_env env, AbilityRuntime::NapiAsyncTask &task, int32_t status) {
                sptr<JSAgentExtensionConnection> connectionSptr = connection.promote();
                if (!connectionSptr) {
                    TAG_LOGE(AAFwkTag::SER_ROUTER, "null connectionSptr");
                    return;
                }
                connectionSptr->HandleOnAuthorize(data);
            });

    napi_ref callback = nullptr;
    std::unique_ptr<AbilityRuntime::NapiAsyncTask::ExecuteCallback> execute = nullptr;
    AbilityRuntime::NapiAsyncTask::Schedule("JSAgentExtensionConnection::Authorize",
        env_, std::make_unique<AbilityRuntime::NapiAsyncTask>(callback, std::move(execute), std::move(complete)));

    return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_OK);
}

void JSAgentExtensionConnection::HandleOnAuthorize(const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "HandleOnAuthorize called");
    napi_value argv[] = { CreateJsValue(env_, data) };
    CallObjectMethod("onAuth", argv, ARGC_ONE);
}

void JSAgentExtensionConnection::CallJsOnDisconnect()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "CallJsOnDisconnect");
    CallObjectMethod("onDisconnect", nullptr, 0);
}

bool JSAgentExtensionConnection::IsJsCallbackObjectEquals(napi_env env,
    std::unique_ptr<NativeReference> &callback, napi_value value)
{
    if (value == nullptr || callback == nullptr) {
        return callback.get() == reinterpret_cast<NativeReference*>(value);
    }
    auto object = callback->GetNapiValue();
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null object");
        return false;
    }

    bool isEqual = false;
    if (napi_strict_equals(env, object, value, &isEqual) != napi_ok) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "object does not match value");
        return false;
    }
    return isEqual;
}
} // namespace AgentRuntime
} // namespace OHOS
