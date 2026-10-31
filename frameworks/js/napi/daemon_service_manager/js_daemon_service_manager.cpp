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

#include "js_daemon_service_manager.h"

#include <atomic>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include "app_mgr_client.h"
#include "daemon_service_info.h"
#include "daemon_service_observer.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr int32_t EXTERNAL_INVALID_PARAMETER = 401;
constexpr int32_t EXTERNAL_INNER_ERROR = 16000050;
constexpr const char *RIGISTER_DAEMON_PERMISSION = "ohos.permission.RIGISTER_DAEMON_SERVICE";
constexpr const char *USE_DAEMON_PERMISSION = "ohos.permission.USE_DAEMON_SERVICE";
constexpr const char *MANAGE_DAEMON_PERMISSION = "ohos.permission.MANAGE_DAEMON_SERVICE";

napi_value CreateDaemonError(napi_env env, int32_t result, const char *permission)
{
    if (result == AppExecFwk::ERROR_SERVICE_NOT_CONNECTED) {
        return CreateJsError(env, EXTERNAL_INNER_ERROR, "Failed to connect to the system service.");
    }
    return CreateJsErrorByNativeErr(env, result, permission);
}

bool GetOptionalInt(napi_env env, napi_value value, const char *name, int32_t &result,
    bool &isPresent)
{
    isPresent = false;
    napi_value property = nullptr;
    if (napi_get_named_property(env, value, name, &property) != napi_ok) {
        return false;
    }
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, property, &type) != napi_ok) {
        return false;
    }
    if (type == napi_undefined) {
        return true;
    }
    if (type != napi_number || napi_get_value_int32(env, property, &result) != napi_ok) {
        return false;
    }
    isPresent = true;
    return true;
}

bool GetDescriptor(napi_env env, napi_value value, AAFwk::LaunchServiceDescriptor &descriptor)
{
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, value, &type) != napi_ok || type != napi_object) {
        return false;
    }
    napi_value bundleValue = nullptr;
    napi_value moduleValue = nullptr;
    napi_value serviceValue = nullptr;
    if (napi_get_named_property(env, value, "bundleName", &bundleValue) != napi_ok ||
        napi_get_named_property(env, value, "moduleName", &moduleValue) != napi_ok ||
        napi_get_named_property(env, value, "serviceName", &serviceValue) != napi_ok) {
        return false;
    }
    if (!ConvertFromJsValue(env, bundleValue, descriptor.bundleName) || descriptor.bundleName.empty() ||
        !ConvertFromJsValue(env, moduleValue, descriptor.moduleName) || descriptor.moduleName.empty() ||
        !ConvertFromJsValue(env, serviceValue, descriptor.serviceName) || descriptor.serviceName.empty()) {
        return false;
    }
    bool isUserIdPresent = false;
    bool isAppIndexPresent = false;
    if (!GetOptionalInt(env, value, "userId", descriptor.userId, isUserIdPresent) ||
        !GetOptionalInt(env, value, "appIndex", descriptor.appIndex, isAppIndexPresent)) {
        return false;
    }
    if ((isUserIdPresent && descriptor.userId < 0) || (isAppIndexPresent && descriptor.appIndex < 0)) {
        return false;
    }
    return descriptor.userId >= -1 && descriptor.appIndex >= 0;
}

using DaemonClientCall = std::function<int32_t(const std::shared_ptr<AppExecFwk::AppMgrClient> &,
    const AAFwk::LaunchServiceDescriptor &)>;

napi_value InvokeDaemonCall(napi_env env, napi_callback_info info, const std::string &name,
    const char *permission, const DaemonClientCall &call)
{
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = { nullptr };
    if (napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr) != napi_ok || argc != ARGC_ONE) {
        ThrowInvalidNumParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::LaunchServiceDescriptor descriptor;
    if (!GetDescriptor(env, argv[0], descriptor)) {
        ThrowInvalidParamError(env, "Invalid launch service descriptor.");
        return CreateJsUndefined(env);
    }

    auto resultCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [descriptor, call, resultCode]() {
        auto client = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
        if (client == nullptr) {
            *resultCode = AppExecFwk::ERROR_SERVICE_NOT_CONNECTED;
            return;
        }
        *resultCode = call(client, descriptor);
    };
    NapiAsyncTask::CompleteCallback complete = [resultCode, permission](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*resultCode != ERR_OK) {
            task.Reject(env, CreateDaemonError(env, *resultCode, permission));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos(name, env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value RegisterDaemon(napi_env env, napi_callback_info info)
{
    return InvokeDaemonCall(env, info, "JsLaunchServiceManager::Register", RIGISTER_DAEMON_PERMISSION,
        [](const std::shared_ptr<AppExecFwk::AppMgrClient> &client,
            const AAFwk::LaunchServiceDescriptor &descriptor) { return client->RegisterDaemon(descriptor); });
}

napi_value UnregisterDaemon(napi_env env, napi_callback_info info)
{
    return InvokeDaemonCall(env, info, "JsLaunchServiceManager::Unregister", RIGISTER_DAEMON_PERMISSION,
        [](const std::shared_ptr<AppExecFwk::AppMgrClient> &client,
            const AAFwk::LaunchServiceDescriptor &descriptor) { return client->UnregisterDaemon(descriptor); });
}

napi_value StartDaemon(napi_env env, napi_callback_info info)
{
    return InvokeDaemonCall(env, info, "JsLaunchServiceManager::Start", USE_DAEMON_PERMISSION,
        [](const std::shared_ptr<AppExecFwk::AppMgrClient> &client,
            const AAFwk::LaunchServiceDescriptor &descriptor) { return client->StartDaemon(descriptor); });
}

napi_value StopDaemon(napi_env env, napi_callback_info info)
{
    return InvokeDaemonCall(env, info, "JsLaunchServiceManager::Stop", USE_DAEMON_PERMISSION,
        [](const std::shared_ptr<AppExecFwk::AppMgrClient> &client,
            const AAFwk::LaunchServiceDescriptor &descriptor) { return client->StopDaemon(descriptor); });
}

bool HasFunction(napi_env env, napi_value object, const char *name)
{
    napi_value method = nullptr;
    napi_valuetype type = napi_undefined;
    return napi_get_named_property(env, object, name, &method) == napi_ok &&
        method != nullptr && napi_typeof(env, method, &type) == napi_ok && type == napi_function;
}

napi_value CreateDescriptor(napi_env env, const AAFwk::LaunchServiceDescriptor &descriptor)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    napi_set_named_property(env, object, "bundleName", CreateJsValue(env, descriptor.bundleName));
    napi_set_named_property(env, object, "moduleName", CreateJsValue(env, descriptor.moduleName));
    napi_set_named_property(env, object, "serviceName", CreateJsValue(env, descriptor.serviceName));
    if (descriptor.userId >= 0) {
        napi_set_named_property(env, object, "userId", CreateJsValue(env, descriptor.userId));
    }
    if (descriptor.appIndex >= 0) {
        napi_set_named_property(env, object, "appIndex", CreateJsValue(env, descriptor.appIndex));
    }
    return object;
}

std::atomic<uint32_t> g_nextObserverId {0};

class JsLaunchServiceObserver final : public AAFwk::LaunchServiceObserverStub {
public:
    explicit JsLaunchServiceObserver(napi_env env) : env_(env) {}

    int32_t AddObserver(napi_value observer)
    {
        napi_ref ref = nullptr;
        if (napi_create_reference(env_, observer, 1, &ref) != napi_ok) {
            return 0;
        }
        std::lock_guard<std::mutex> lock(observerMutex_);
        const int32_t observerId = AllocateObserverIdLocked();
        if (observerId <= 0) {
            napi_delete_reference(env_, ref);
            return 0;
        }
        observers_.emplace(observerId, ref);
        return observerId;
    }

    bool HasObserver(int32_t observerId)
    {
        std::lock_guard<std::mutex> lock(observerMutex_);
        return observers_.find(observerId) != observers_.end();
    }

    bool IsLastObserver(int32_t observerId)
    {
        std::lock_guard<std::mutex> lock(observerMutex_);
        return observers_.size() == 1 && observers_.find(observerId) != observers_.end();
    }

    void RemoveObserver(int32_t observerId)
    {
        std::lock_guard<std::mutex> lock(observerMutex_);
        auto it = observers_.find(observerId);
        if (it != observers_.end()) {
            napi_delete_reference(env_, it->second);
            observers_.erase(it);
        }
    }

    bool IsEmpty()
    {
        std::lock_guard<std::mutex> lock(observerMutex_);
        return observers_.empty();
    }

    void Disable()
    {
        active_.store(false);
        std::lock_guard<std::mutex> lock(observerMutex_);
        for (const auto &item : observers_) {
            napi_delete_reference(env_, item.second);
        }
        observers_.clear();
    }

    void OnLaunchServiceStatusChanged(const AAFwk::LaunchServiceStateData &stateData) override
    {
        Schedule("onLaunchServiceStatusChanged", stateData.descriptor,
            static_cast<int32_t>(stateData.state), 0, false);
    }

    void OnLaunchServiceProcessStateChanged(const AAFwk::LaunchServiceProcessStateData &processStateData) override
    {
        Schedule("onLaunchServiceProcessStateChanged", processStateData.descriptor,
            static_cast<int32_t>(processStateData.state), processStateData.pid, true);
    }

private:
    int32_t AllocateObserverIdLocked()
    {
        for (size_t count = 0; count <= observers_.size(); ++count) {
            const uint32_t serial = g_nextObserverId.fetch_add(1, std::memory_order_relaxed);
            const int32_t candidate = static_cast<int32_t>(
                serial % static_cast<uint32_t>(std::numeric_limits<int32_t>::max())) + 1;
            if (observers_.find(candidate) == observers_.end()) {
                return candidate;
            }
        }
        return 0;
    }

    void Schedule(const char *method, const AAFwk::LaunchServiceDescriptor &descriptor,
        int32_t state, int32_t pid, bool processData)
    {
        if (!active_.load()) {
            return;
        }
        wptr<JsLaunchServiceObserver> weak = this;
        auto complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
            [weak, methodName = std::string(method), descriptor, state, pid,
                processData](napi_env env, NapiAsyncTask &, int32_t) {
                auto self = weak.promote();
                if (self == nullptr || !self->active_.load()) {
                    return;
                }
                std::vector<napi_ref> observers;
                {
                    std::lock_guard<std::mutex> lock(self->observerMutex_);
                    for (const auto &item : self->observers_) {
                        observers.emplace_back(item.second);
                    }
                }
                napi_value data = nullptr;
                napi_create_object(env, &data);
                napi_set_named_property(env, data, "descriptor", CreateDescriptor(env, descriptor));
                if (processData) {
                    napi_set_named_property(env, data, "pid", CreateJsValue(env, pid));
                }
                napi_set_named_property(env, data, "state", CreateJsValue(env, state));
                for (const auto &ref : observers) {
                    napi_value object = nullptr;
                    if (napi_get_reference_value(env, ref, &object) != napi_ok || object == nullptr) {
                        continue;
                    }
                    napi_value callback = nullptr;
                    if (napi_get_named_property(env, object, methodName.c_str(), &callback) != napi_ok ||
                        callback == nullptr) {
                        continue;
                    }
                    napi_value result = nullptr;
                    napi_call_function(env, object, callback, 1, &data, &result);
                }
            });
        napi_ref callback = nullptr;
        NapiAsyncTask::Schedule("JsLaunchServiceObserver::Notify", env_,
            std::make_unique<NapiAsyncTask>(callback, nullptr, std::move(complete)));
    }

    napi_env env_ = nullptr;
    std::atomic_bool active_ = true;
    std::mutex observerMutex_;
    std::map<int32_t, napi_ref> observers_;
};

std::mutex g_observerMutex;
std::map<napi_env, sptr<JsLaunchServiceObserver>> g_observers;

void CleanupObservers(void *data)
{
    napi_env env = static_cast<napi_env>(data);
    sptr<JsLaunchServiceObserver> observer;
    {
        std::lock_guard<std::mutex> lock(g_observerMutex);
        auto it = g_observers.find(env);
        if (it != g_observers.end()) {
            observer = it->second;
            g_observers.erase(it);
        }
    }
    if (observer != nullptr) {
        auto client = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
        if (client != nullptr) {
            client->UnregisterLaunchServiceObserver(observer);
        }
        observer->Disable();
    }
}

napi_value OnLaunchServiceObserver(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_valuetype type = napi_undefined;
    if (argc != ARGC_ONE || napi_typeof(env, argv[0], &type) != napi_ok || type != napi_object ||
        !HasFunction(env, argv[0], "onLaunchServiceStatusChanged") ||
        !HasFunction(env, argv[0], "onLaunchServiceProcessStateChanged")) {
        ThrowInvalidParamError(env, "Parameter error. The observer is invalid.");
        return CreateJsUndefined(env);
    }
    std::lock_guard<std::mutex> lock(g_observerMutex);
    auto &observer = g_observers[env];
    const bool needRegister = observer == nullptr;
    if (needRegister) {
        observer = new (std::nothrow) JsLaunchServiceObserver(env);
        if (observer == nullptr) {
            g_observers.erase(env);
            ThrowError(env, EXTERNAL_INNER_ERROR, "Failed to create launch service observer.");
            return CreateJsUndefined(env);
        }
    }
    const int32_t observerId = observer->AddObserver(argv[0]);
    if (observerId <= 0) {
        if (needRegister) {
            g_observers.erase(env);
        }
        ThrowError(env, EXTERNAL_INNER_ERROR, "Failed to add launch service observer.");
        return CreateJsUndefined(env);
    }
    if (needRegister) {
        auto client = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
        int32_t ret = client == nullptr ? AppExecFwk::ERROR_SERVICE_NOT_CONNECTED :
            client->RegisterLaunchServiceObserver(observer);
        if (ret != ERR_OK) {
            observer->RemoveObserver(observerId);
            g_observers.erase(env);
            napi_throw(env, CreateDaemonError(env, ret, MANAGE_DAEMON_PERMISSION));
            return CreateJsUndefined(env);
        }
    }
    return CreateJsValue(env, observerId);
}

napi_value OffLaunchServiceObserver(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    auto observerId = std::make_shared<int32_t>(0);
    if (argc != ARGC_ONE || !ConvertFromJsValue(env, argv[0], *observerId) || *observerId <= 0) {
        ThrowInvalidParamError(env, "Parameter error. The observerId is invalid.");
        return CreateJsUndefined(env);
    }
    NapiAsyncTask::ExecuteCallback execute = []() {};
    NapiAsyncTask::CompleteCallback complete = [observerId](napi_env env, NapiAsyncTask &task, int32_t) {
        bool invalidObserverId = false;
        int32_t ret = ERR_OK;
        {
            std::lock_guard<std::mutex> lock(g_observerMutex);
            auto it = g_observers.find(env);
            if (it == g_observers.end() || !it->second->HasObserver(*observerId)) {
                invalidObserverId = true;
            } else {
                if (it->second->IsLastObserver(*observerId)) {
                    auto client = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
                    ret = client == nullptr ? AppExecFwk::ERROR_SERVICE_NOT_CONNECTED :
                        client->UnregisterLaunchServiceObserver(it->second);
                }
                if (ret == ERR_OK) {
                    it->second->RemoveObserver(*observerId);
                    if (it->second->IsEmpty()) {
                        g_observers.erase(it);
                    }
                }
            }
        }
        if (invalidObserverId) {
            task.Reject(env, CreateJsError(env, EXTERNAL_INVALID_PARAMETER,
                "Parameter error. The observerId is invalid."));
            return;
        }
        if (ret != ERR_OK) {
            task.Reject(env, CreateDaemonError(env, ret, MANAGE_DAEMON_PERMISSION));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };
    napi_value promise = nullptr;
    NapiAsyncTask::Schedule("JsLaunchServiceManager::OffLaunchServiceObserver", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &promise));
    return promise;
}

} // namespace

napi_value JsLaunchServiceManagerInit(napi_env env, napi_value exports)
{
    napi_add_env_cleanup_hook(env, CleanupObservers, env);
    BindNativeFunction(env, exports, "register", "JsLaunchServiceManager", RegisterDaemon);
    BindNativeFunction(env, exports, "unregister", "JsLaunchServiceManager", UnregisterDaemon);
    BindNativeFunction(env, exports, "start", "JsLaunchServiceManager", StartDaemon);
    BindNativeFunction(env, exports, "stop", "JsLaunchServiceManager", StopDaemon);
    BindNativeFunction(env, exports, "onLaunchServiceObserver", "JsLaunchServiceManager", OnLaunchServiceObserver);
    BindNativeFunction(env, exports, "offLaunchServiceObserver", "JsLaunchServiceManager", OffLaunchServiceObserver);
    return exports;
}
} // namespace AbilityRuntime
} // namespace OHOS
