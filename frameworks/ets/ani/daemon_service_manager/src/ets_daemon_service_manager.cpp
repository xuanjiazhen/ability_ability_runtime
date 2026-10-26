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

#include "ets_daemon_service_manager.h"

#include <array>
#include <atomic>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include "ani_common_util.h"
#include "app_mgr_client.h"
#include "daemon_service_info.h"
#include "daemon_service_observer.h"
#include "errors.h"
#include "ets_error_utils.h"
#include "event_handler.h"
#include "hilog_tag_wrapper.h"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *LAUNCH_SERVICE_MANAGER_NAMESPACE = "@ohos.launchServiceManager.launchServiceManager";
constexpr const char *DAEMON_CALL_SIGNATURE =
    "C{std.core.String}C{std.core.String}C{std.core.String}iiC{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *DESCRIPTOR_CLASS =
    "application.LaunchServiceDescriptor.LaunchServiceDescriptorInner";
constexpr const char *STATE_DATA_CLASS =
    "application.LaunchServiceObserver.LaunchServiceStateDataInner";
constexpr const char *PROCESS_STATE_DATA_CLASS =
    "application.LaunchServiceObserver.LaunchServiceProcessStateDataInner";
constexpr int32_t EXTERNAL_INVALID_PARAMETER = 401;
constexpr int32_t EXTERNAL_INNER_ERROR = 16000050;

ani_object CreateDaemonError(ani_env *env, int32_t result)
{
    if (result == AppExecFwk::ERROR_SERVICE_NOT_CONNECTED) {
        return EtsErrorUtil::CreateError(
            env, EXTERNAL_INNER_ERROR, "Failed to connect to the system service.");
    }
    return EtsErrorUtil::CreateErrorByNativeErr(env, result);
}

using DaemonClientCall = std::function<int32_t(const std::shared_ptr<AppExecFwk::AppMgrClient> &,
    const AAFwk::LaunchServiceDescriptor &)>;

bool GetDescriptor(ani_env *env, ani_string aniBundleName, ani_string aniModuleName, ani_string aniServiceName,
    ani_int userId, ani_int appIndex, AAFwk::LaunchServiceDescriptor &descriptor)
{
    if (!AppExecFwk::GetStdString(env, aniBundleName, descriptor.bundleName) || descriptor.bundleName.empty() ||
        !AppExecFwk::GetStdString(env, aniModuleName, descriptor.moduleName) || descriptor.moduleName.empty() ||
        !AppExecFwk::GetStdString(env, aniServiceName, descriptor.serviceName) || descriptor.serviceName.empty() ||
        userId < -1 || appIndex < 0) {
        return false;
    }
    descriptor.userId = userId;
    descriptor.appIndex = appIndex;
    return true;
}

void InvokeDaemonCall(ani_env *env, ani_string aniBundleName, ani_string aniModuleName, ani_string aniServiceName,
    ani_int userId, ani_int appIndex, ani_object callback, const DaemonClientCall &call)
{
    if (env == nullptr) {
        return;
    }
    AAFwk::LaunchServiceDescriptor descriptor;
    if (!GetDescriptor(env, aniBundleName, aniModuleName, aniServiceName, userId, appIndex, descriptor)) {
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, EXTERNAL_INVALID_PARAMETER, "Invalid launch service descriptor."), nullptr);
        return;
    }

    int32_t result = AppExecFwk::ERROR_SERVICE_NOT_CONNECTED;
    auto client = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    if (client != nullptr) {
        result = call(client, descriptor);
    }
    if (result != ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback, CreateDaemonError(env, result), nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback, nullptr, nullptr);
}

void RegisterDaemon(ani_env *env, ani_string aniBundleName, ani_string aniModuleName, ani_string aniServiceName,
    ani_int userId, ani_int appIndex, ani_object callback)
{
    InvokeDaemonCall(env, aniBundleName, aniModuleName, aniServiceName, userId, appIndex, callback,
        [](const std::shared_ptr<AppExecFwk::AppMgrClient> &client,
            const AAFwk::LaunchServiceDescriptor &descriptor) { return client->RegisterDaemon(descriptor); });
}

void UnregisterDaemon(ani_env *env, ani_string aniBundleName, ani_string aniModuleName, ani_string aniServiceName,
    ani_int userId, ani_int appIndex, ani_object callback)
{
    InvokeDaemonCall(env, aniBundleName, aniModuleName, aniServiceName, userId, appIndex, callback,
        [](const std::shared_ptr<AppExecFwk::AppMgrClient> &client,
            const AAFwk::LaunchServiceDescriptor &descriptor) { return client->UnregisterDaemon(descriptor); });
}

void StartDaemon(ani_env *env, ani_string aniBundleName, ani_string aniModuleName, ani_string aniServiceName,
    ani_int userId, ani_int appIndex, ani_object callback)
{
    InvokeDaemonCall(env, aniBundleName, aniModuleName, aniServiceName, userId, appIndex, callback,
        [](const std::shared_ptr<AppExecFwk::AppMgrClient> &client,
            const AAFwk::LaunchServiceDescriptor &descriptor) { return client->StartDaemon(descriptor); });
}

void StopDaemon(ani_env *env, ani_string aniBundleName, ani_string aniModuleName, ani_string aniServiceName,
    ani_int userId, ani_int appIndex, ani_object callback)
{
    InvokeDaemonCall(env, aniBundleName, aniModuleName, aniServiceName, userId, appIndex, callback,
        [](const std::shared_ptr<AppExecFwk::AppMgrClient> &client,
            const AAFwk::LaunchServiceDescriptor &descriptor) { return client->StopDaemon(descriptor); });
}

ani_object NewObject(ani_env *env, const char *className)
{
    ani_class cls = nullptr;
    ani_method ctor = nullptr;
    ani_object object = nullptr;
    if (env->FindClass(className, &cls) != ANI_OK || cls == nullptr ||
        env->Class_FindMethod(cls, "<ctor>", ":", &ctor) != ANI_OK || ctor == nullptr ||
        env->Object_New(cls, ctor, &object) != ANI_OK) {
        return nullptr;
    }
    return object;
}

ani_object CreateDescriptor(ani_env *env, const AAFwk::LaunchServiceDescriptor &descriptor)
{
    ani_object object = NewObject(env, DESCRIPTOR_CLASS);
    if (object == nullptr ||
        env->Object_SetPropertyByName_Ref(object, "bundleName",
            AppExecFwk::GetAniString(env, descriptor.bundleName)) != ANI_OK ||
        env->Object_SetPropertyByName_Ref(object, "moduleName",
            AppExecFwk::GetAniString(env, descriptor.moduleName)) != ANI_OK ||
        env->Object_SetPropertyByName_Ref(object, "serviceName",
            AppExecFwk::GetAniString(env, descriptor.serviceName)) != ANI_OK) {
        return nullptr;
    }
    if (descriptor.userId >= 0 &&
        env->Object_SetPropertyByName_Int(object, "userId", descriptor.userId) != ANI_OK) {
        return nullptr;
    }
    if (descriptor.appIndex >= 0 &&
        env->Object_SetPropertyByName_Int(object, "appIndex", descriptor.appIndex) != ANI_OK) {
        return nullptr;
    }
    return object;
}

std::atomic<uint32_t> g_nextObserverId {0};

class EtsLaunchServiceObserver final : public AAFwk::LaunchServiceObserverStub {
public:
    explicit EtsLaunchServiceObserver(ani_vm *vm) : vm_(vm) {}

    int32_t AddObserver(ani_env *env, ani_object observer)
    {
        ani_ref observerRef = nullptr;
        if (env->GlobalReference_Create(observer, &observerRef) != ANI_OK || observerRef == nullptr) {
            return 0;
        }
        std::lock_guard<std::mutex> lock(observerMutex_);
        const int32_t observerId = AllocateObserverIdLocked();
        if (observerId <= 0) {
            env->GlobalReference_Delete(observerRef);
            return 0;
        }
        observers_.emplace(observerId, observerRef);
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

    void RemoveObserver(ani_env *env, int32_t observerId)
    {
        std::lock_guard<std::mutex> lock(observerMutex_);
        auto it = observers_.find(observerId);
        if (it != observers_.end()) {
            env->GlobalReference_Delete(it->second);
            observers_.erase(it);
        }
    }

    bool IsEmpty()
    {
        std::lock_guard<std::mutex> lock(observerMutex_);
        return observers_.empty();
    }

    void OnLaunchServiceStatusChanged(const AAFwk::LaunchServiceStateData &stateData) override
    {
        PostTask([weak = wptr<EtsLaunchServiceObserver>(this), stateData]() {
            auto self = weak.promote();
            if (self != nullptr) {
                self->HandleStatusChanged(stateData);
            }
        });
    }

    void OnLaunchServiceProcessStateChanged(
        const AAFwk::LaunchServiceProcessStateData &processStateData) override
    {
        PostTask([weak = wptr<EtsLaunchServiceObserver>(this), processStateData]() {
            auto self = weak.promote();
            if (self != nullptr) {
                self->HandleProcessStateChanged(processStateData);
            }
        });
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

    void PostTask(std::function<void()> task)
    {
        std::lock_guard<std::mutex> lock(handlerMutex_);
        if (handler_ == nullptr) {
            auto runner = AppExecFwk::EventRunner::GetMainEventRunner();
            if (runner == nullptr) {
                return;
            }
            handler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
        }
        handler_->PostTask(std::move(task));
    }

    ani_env *GetEnv(bool &attached)
    {
        return AppExecFwk::AttachAniEnv(vm_, attached);
    }

    void HandleStatusChanged(const AAFwk::LaunchServiceStateData &stateData)
    {
        bool attached = false;
        ani_env *env = GetEnv(attached);
        if (env == nullptr) {
            return;
        }
        std::lock_guard<std::mutex> lock(observerMutex_);
        if (observers_.empty()) {
            AppExecFwk::DetachAniEnv(vm_, attached);
            return;
        }
        ani_object data = NewObject(env, STATE_DATA_CLASS);
        ani_object descriptor = CreateDescriptor(env, stateData.descriptor);
        if (data != nullptr && descriptor != nullptr &&
            env->Object_SetPropertyByName_Ref(data, "descriptor", descriptor) == ANI_OK &&
            env->Object_SetPropertyByName_Int(
                data, "state", static_cast<int32_t>(stateData.state)) == ANI_OK) {
            for (const auto &item : observers_) {
                env->Object_CallMethodByName_Void(reinterpret_cast<ani_object>(item.second),
                    "onLaunchServiceStatusChanged",
                    "C{application.LaunchServiceObserver.LaunchServiceStateData}:", data);
            }
        }
        AppExecFwk::DetachAniEnv(vm_, attached);
    }

    void HandleProcessStateChanged(const AAFwk::LaunchServiceProcessStateData &processStateData)
    {
        bool attached = false;
        ani_env *env = GetEnv(attached);
        if (env == nullptr) {
            return;
        }
        std::lock_guard<std::mutex> lock(observerMutex_);
        if (observers_.empty()) {
            AppExecFwk::DetachAniEnv(vm_, attached);
            return;
        }
        ani_object data = NewObject(env, PROCESS_STATE_DATA_CLASS);
        ani_object descriptor = CreateDescriptor(env, processStateData.descriptor);
        if (data != nullptr && descriptor != nullptr &&
            env->Object_SetPropertyByName_Ref(data, "descriptor", descriptor) == ANI_OK &&
            env->Object_SetPropertyByName_Int(data, "pid", processStateData.pid) == ANI_OK &&
            env->Object_SetPropertyByName_Int(
                data, "state", static_cast<int32_t>(processStateData.state)) == ANI_OK) {
            for (const auto &item : observers_) {
                env->Object_CallMethodByName_Void(reinterpret_cast<ani_object>(item.second),
                    "onLaunchServiceProcessStateChanged",
                    "C{application.LaunchServiceObserver.LaunchServiceProcessStateData}:", data);
            }
        }
        AppExecFwk::DetachAniEnv(vm_, attached);
    }

    ani_vm *vm_ = nullptr;
    std::mutex observerMutex_;
    std::map<int32_t, ani_ref> observers_;
    std::mutex handlerMutex_;
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
};

std::mutex g_observerMutex;
std::map<ani_vm *, sptr<EtsLaunchServiceObserver>> g_observers;

ani_int OnLaunchServiceObserver(ani_env *env, ani_object observerObject)
{
    ani_vm *vm = nullptr;
    if (env == nullptr || observerObject == nullptr || env->GetVM(&vm) != ANI_OK || vm == nullptr) {
        EtsErrorUtil::ThrowError(env, EXTERNAL_INVALID_PARAMETER, "Parameter error. The observer is invalid.");
        return 0;
    }
    std::lock_guard<std::mutex> lock(g_observerMutex);
    auto &observer = g_observers[vm];
    const bool needRegister = observer == nullptr;
    if (needRegister) {
        observer = new (std::nothrow) EtsLaunchServiceObserver(vm);
        if (observer == nullptr) {
            g_observers.erase(vm);
            EtsErrorUtil::ThrowError(env, EXTERNAL_INNER_ERROR, "Failed to create launch service observer.");
            return 0;
        }
    }
    const int32_t observerId = observer->AddObserver(env, observerObject);
    if (observerId <= 0) {
        if (needRegister) {
            g_observers.erase(vm);
        }
        EtsErrorUtil::ThrowError(env, EXTERNAL_INNER_ERROR, "Failed to add launch service observer.");
        return 0;
    }
    if (needRegister) {
        auto client = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
        int32_t ret = client == nullptr ? AppExecFwk::ERROR_SERVICE_NOT_CONNECTED :
            client->RegisterLaunchServiceObserver(observer);
        if (ret != ERR_OK) {
            observer->RemoveObserver(env, observerId);
            g_observers.erase(vm);
            EtsErrorUtil::ThrowError(env, EXTERNAL_INNER_ERROR, "Failed to register launch service observer.");
            return 0;
        }
    }
    return observerId;
}

void OffLaunchServiceObserver(ani_env *env, ani_int observerId, ani_object callback)
{
    if (observerId <= 0) {
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, EXTERNAL_INVALID_PARAMETER,
                "Parameter error. The observerId is invalid."), nullptr);
        return;
    }
    ani_vm *vm = nullptr;
    if (env == nullptr || env->GetVM(&vm) != ANI_OK || vm == nullptr) {
        return;
    }
    bool invalidObserverId = false;
    int32_t ret = ERR_OK;
    {
        std::lock_guard<std::mutex> lock(g_observerMutex);
        auto it = g_observers.find(vm);
        if (it == g_observers.end() || !it->second->HasObserver(observerId)) {
            invalidObserverId = true;
        } else {
            if (it->second->IsLastObserver(observerId)) {
                auto client = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
                ret = client == nullptr ? AppExecFwk::ERROR_SERVICE_NOT_CONNECTED :
                    client->UnregisterLaunchServiceObserver(it->second);
            }
            if (ret == ERR_OK) {
                it->second->RemoveObserver(env, observerId);
                if (it->second->IsEmpty()) {
                    g_observers.erase(it);
                }
            }
        }
    }
    if (invalidObserverId) {
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, EXTERNAL_INVALID_PARAMETER,
                "Parameter error. The observerId is invalid."), nullptr);
        return;
    }
    if (ret != ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback, CreateDaemonError(env, ret), nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback, nullptr, nullptr);
}

} // namespace

void EtsLaunchServiceManagerInit(ani_env *env)
{
    ani_namespace ns = nullptr;
    if (env == nullptr || env->FindNamespace(LAUNCH_SERVICE_MANAGER_NAMESPACE, &ns) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "find launch service manager namespace failed");
        return;
    }
    std::array methods = {
        ani_native_function { "nativeRegisterDaemon", DAEMON_CALL_SIGNATURE,
            reinterpret_cast<void *>(RegisterDaemon) },
        ani_native_function { "nativeUnregisterDaemon", DAEMON_CALL_SIGNATURE,
            reinterpret_cast<void *>(UnregisterDaemon) },
        ani_native_function { "nativeStartDaemon", DAEMON_CALL_SIGNATURE,
            reinterpret_cast<void *>(StartDaemon) },
        ani_native_function { "nativeStopDaemon", DAEMON_CALL_SIGNATURE,
            reinterpret_cast<void *>(StopDaemon) },
        ani_native_function { "nativeOnLaunchServiceObserver",
            "C{application.LaunchServiceObserver.LaunchServiceObserver}:i",
            reinterpret_cast<void *>(OnLaunchServiceObserver) },
        ani_native_function { "nativeOffLaunchServiceObserver",
            "iC{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(OffLaunchServiceObserver) },
    };
    if (env->Namespace_BindNativeFunctions(ns, methods.data(), methods.size()) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bind launch service manager methods failed");
    }
}

extern "C" ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    if (vm == nullptr || result == nullptr) {
        return ANI_INVALID_ARGS;
    }
    ani_env *env = nullptr;
    if (vm->GetEnv(ANI_VERSION_1, &env) != ANI_OK || env == nullptr) {
        return ANI_NOT_FOUND;
    }
    EtsLaunchServiceManagerInit(env);
    *result = ANI_VERSION_1;
    return ANI_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS
