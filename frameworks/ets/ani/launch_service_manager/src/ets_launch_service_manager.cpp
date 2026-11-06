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

#include "ets_launch_service_manager.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include "ability_business_error.h"
#include "ability_manager_errors.h"
#include "ani_common_util.h"
#include "daemon_service_manager_client.h"
#include "errors.h"
#include "ets_error_utils.h"
#include "event_handler.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *MANAGER_NAMESPACE =
    "@ohos.app.ability.launchServiceManager.launchServiceManager";
constexpr const char *DESCRIPTOR_CLASS =
    "application.LaunchServiceDescriptor.LaunchServiceDescriptorInner";
constexpr const char *INFORMATION_CLASS =
    "application.LaunchServiceInformation.LaunchServiceInformationInner";
constexpr const char *STATE_DATA_CLASS =
    "application.LaunchServiceStateData.LaunchServiceStateDataInner";
constexpr const char *PROCESS_STATE_DATA_CLASS =
    "application.LaunchServiceProcessStateData.LaunchServiceProcessStateDataInner";
constexpr int32_t CONNECTION_ERROR = 16000050;
constexpr const char *REGISTER_DAEMON_PERMISSION =
    "ohos.permission.REGISTER_DAEMON_SERVICE";
constexpr const char *USE_DAEMON_PERMISSION =
    "ohos.permission.USE_DAEMON_SERVICE";
constexpr const char *MANAGE_DAEMON_PERMISSION =
    "ohos.permission.MANAGE_DAEMON_SERVICE";
constexpr const char *SET_ENABLE_SIGNATURE =
    "C{std.core.String}iizC{escompat.Uint8Array}C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *DESCRIPTOR_CALL_SIGNATURE =
    "C{application.LaunchServiceDescriptor.LaunchServiceDescriptor}"
    "C{utils.AbilityUtils.AsyncCallbackWrapper}:";
std::atomic<uint32_t> g_nextObserverId{0};

bool GetUint8Array(ani_env *env, ani_object array, std::vector<uint8_t> &result) {
  ani_ref buffer = nullptr;
  ani_int byteLength = 0;
  ani_int byteOffset = 0;
  if (env->Object_GetFieldByName_Ref(array, "buffer", &buffer) != ANI_OK ||
      buffer == nullptr ||
      env->Object_GetFieldByName_Int(array, "byteLengthInt", &byteLength) != ANI_OK ||
      env->Object_GetFieldByName_Int(array, "byteOffsetInt", &byteOffset) != ANI_OK ||
      byteLength < 0 || byteOffset < 0) {
    return false;
  }
  void *data = nullptr;
  size_t bufferLength = 0;
  if (env->ArrayBuffer_GetInfo(static_cast<ani_arraybuffer>(buffer), &data,
                               &bufferLength) != ANI_OK) {
    return false;
  }
  const size_t length = static_cast<size_t>(byteLength);
  const size_t offset = static_cast<size_t>(byteOffset);
  if (offset > bufferLength || length > bufferLength - offset ||
      (length > 0 && data == nullptr)) {
    return false;
  }
  if (length == 0) {
    result.clear();
    return true;
  }
  auto *bytes = static_cast<uint8_t *>(data) + offset;
  result.assign(bytes, bytes + length);
  return true;
}

ani_object NewObject(ani_env *env, const char *className) {
  ani_class cls = nullptr;
  ani_method ctor = nullptr;
  ani_object object = nullptr;
  if (env->FindClass(className, &cls) != ANI_OK || cls == nullptr ||
      env->Class_FindMethod(cls, "<ctor>", ":", &ctor) != ANI_OK ||
      ctor == nullptr || env->Object_New(cls, ctor, &object) != ANI_OK) {
    return nullptr;
  }
  return object;
}

bool UnwrapDescriptor(ani_env *env, ani_object object,
                      LaunchServiceDescriptor &descriptor) {
  if (env == nullptr || object == nullptr ||
      !AppExecFwk::GetStringProperty(env, object, "bundleName",
                                     descriptor.bundleName) ||
      !AppExecFwk::GetStringProperty(env, object, "moduleName",
                                     descriptor.moduleName) ||
      !AppExecFwk::GetStringProperty(env, object, "serviceName",
                                     descriptor.serviceName) ||
      descriptor.bundleName.empty() || descriptor.moduleName.empty() ||
      descriptor.serviceName.empty()) {
    return false;
  }
  descriptor.hasUserId =
      AppExecFwk::GetIntPropertyValue(env, object, "userId", descriptor.userId);
  if (!descriptor.hasUserId) {
    env->ResetError();
  }
  descriptor.hasAppIndex = AppExecFwk::GetIntPropertyValue(
      env, object, "appIndex", descriptor.appIndex);
  if (!descriptor.hasAppIndex) {
    env->ResetError();
  }
  return (!descriptor.hasUserId || descriptor.userId >= 0) &&
         (!descriptor.hasAppIndex || descriptor.appIndex >= 0);
}

ani_object CreateDescriptor(ani_env *env,
                            const LaunchServiceDescriptor &descriptor) {
  ani_object object = NewObject(env, DESCRIPTOR_CLASS);
  if (object == nullptr ||
      env->Object_SetPropertyByName_Ref(
          object, "bundleName",
          AppExecFwk::GetAniString(env, descriptor.bundleName)) != ANI_OK ||
      env->Object_SetPropertyByName_Ref(
          object, "moduleName",
          AppExecFwk::GetAniString(env, descriptor.moduleName)) != ANI_OK ||
      env->Object_SetPropertyByName_Ref(
          object, "serviceName",
          AppExecFwk::GetAniString(env, descriptor.serviceName)) != ANI_OK) {
    return nullptr;
  }
  if (descriptor.hasUserId &&
      env->Object_SetPropertyByName_Int(object, "userId", descriptor.userId) !=
          ANI_OK) {
    return nullptr;
  }
  if (descriptor.hasAppIndex &&
      env->Object_SetPropertyByName_Int(object, "appIndex",
                                        descriptor.appIndex) != ANI_OK) {
    return nullptr;
  }
  return object;
}

ani_object CreateInformation(ani_env *env,
                             const LaunchServiceInformation &information) {
  ani_object object = NewObject(env, INFORMATION_CLASS);
  ani_object descriptor = CreateDescriptor(env, information.descriptor);
  if (object == nullptr || descriptor == nullptr ||
      env->Object_SetPropertyByName_Ref(object, "descriptor", descriptor) !=
          ANI_OK ||
      env->Object_SetPropertyByName_Int(
          object, "state", static_cast<int32_t>(information.state)) != ANI_OK ||
      env->Object_SetPropertyByName_Int(object, "pid", information.pid) !=
          ANI_OK) {
    return nullptr;
  }
  return object;
}

ani_object CreateInformationArray(
    ani_env *env, const std::vector<LaunchServiceInformation> &informations) {
  ani_object array = nullptr;
  if (!AppExecFwk::CreateArrayObject(env, array, informations.size())) {
    return nullptr;
  }
  for (size_t index = 0; index < informations.size(); ++index) {
    ani_object information = CreateInformation(env, informations[index]);
    if (information == nullptr ||
        env->Object_CallMethodByName_Void(array, "$_set",
                                          "iY:", static_cast<ani_int>(index),
                                          information) != ANI_OK) {
      return nullptr;
    }
  }
  return array;
}

class EtsLaunchServiceObserver final : public LaunchServiceObserverStub {
public:
  explicit EtsLaunchServiceObserver(ani_vm *vm) : vm_(vm) {}

  int32_t AddObserver(ani_env *env, ani_object observer) {
    ani_ref observerRef = nullptr;
    if (env->GlobalReference_Create(observer, &observerRef) != ANI_OK ||
        observerRef == nullptr) {
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

  bool HasObserver(int32_t observerId) {
    std::lock_guard<std::mutex> lock(observerMutex_);
    return observers_.find(observerId) != observers_.end();
  }

  size_t GetObserverCount() {
    std::lock_guard<std::mutex> lock(observerMutex_);
    return observers_.size();
  }

  void RemoveObserver(ani_env *env, int32_t observerId) {
    std::lock_guard<std::mutex> lock(observerMutex_);
    auto it = observers_.find(observerId);
    if (it != observers_.end()) {
      env->GlobalReference_Delete(it->second);
      observers_.erase(it);
    }
  }

  bool IsEmpty() {
    std::lock_guard<std::mutex> lock(observerMutex_);
    return observers_.empty();
  }

  void Disable(ani_env *env) {
    active_.store(false);
    std::lock_guard<std::mutex> lock(observerMutex_);
    if (env != nullptr) {
      for (const auto &item : observers_) {
        env->GlobalReference_Delete(item.second);
      }
    }
    observers_.clear();
  }

  void OnLaunchServiceStatusChanged(
      const LaunchServiceStateData &stateData) override {
    if (!active_.load()) {
      return;
    }
    PostTask([weak = wptr<EtsLaunchServiceObserver>(this), stateData]() {
      auto self = weak.promote();
      if (self != nullptr) {
        self->HandleStatusChanged(stateData);
      }
    });
  }

  void OnLaunchServiceProcessStateChanged(
      const LaunchServiceProcessStateData &processStateData) override {
    if (!active_.load()) {
      return;
    }
    PostTask([weak = wptr<EtsLaunchServiceObserver>(this), processStateData]() {
      auto self = weak.promote();
      if (self != nullptr) {
        self->HandleProcessStateChanged(processStateData);
      }
    });
  }

private:
  int32_t AllocateObserverIdLocked() {
    for (size_t count = 0; count <= observers_.size(); ++count) {
      const uint32_t serial =
          g_nextObserverId.fetch_add(1, std::memory_order_relaxed);
      const int32_t candidate =
          static_cast<int32_t>(
              serial %
              static_cast<uint32_t>(std::numeric_limits<int32_t>::max())) +
          1;
      if (observers_.find(candidate) == observers_.end()) {
        return candidate;
      }
    }
    return 0;
  }

  void PostTask(std::function<void()> task) {
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

  ani_env *GetEnv(bool &attached) {
    return AppExecFwk::AttachAniEnv(vm_, attached);
  }

  void HandleStatusChanged(const LaunchServiceStateData &stateData) {
    bool attached = false;
    ani_env *env = GetEnv(attached);
    if (env == nullptr) {
      return;
    }
    std::lock_guard<std::mutex> lock(observerMutex_);
    if (!active_.load() || observers_.empty()) {
      AppExecFwk::DetachAniEnv(vm_, attached);
      return;
    }
    ani_object data = NewObject(env, STATE_DATA_CLASS);
    ani_object descriptor = CreateDescriptor(env, stateData.descriptor);
    ani_object dataArray = nullptr;
    if (data != nullptr && descriptor != nullptr &&
        env->Object_SetPropertyByName_Ref(data, "descriptor", descriptor) ==
            ANI_OK &&
        env->Object_SetPropertyByName_Boolean(
            data, "registerState",
            stateData.state != LaunchServiceState::NOT_REGISTERED) == ANI_OK &&
        AppExecFwk::CreateArrayObject(env, dataArray, 1) &&
        env->Object_CallMethodByName_Void(dataArray, "$_set", "iY:", 0,
                                          data) == ANI_OK) {
      for (const auto &item : observers_) {
        env->Object_CallMethodByName_Void(
            reinterpret_cast<ani_object>(item.second),
            "onLaunchServiceRegisterStatusChanged", "C{escompat.Array}:",
            dataArray);
      }
    }
    AppExecFwk::DetachAniEnv(vm_, attached);
  }

  void HandleProcessStateChanged(
      const LaunchServiceProcessStateData &processStateData) {
    bool attached = false;
    ani_env *env = GetEnv(attached);
    if (env == nullptr) {
      return;
    }
    std::lock_guard<std::mutex> lock(observerMutex_);
    if (!active_.load() || observers_.empty()) {
      AppExecFwk::DetachAniEnv(vm_, attached);
      return;
    }
    ani_object data = NewObject(env, PROCESS_STATE_DATA_CLASS);
    ani_object descriptor = CreateDescriptor(env, processStateData.descriptor);
    if (data != nullptr && descriptor != nullptr &&
        env->Object_SetPropertyByName_Ref(data, "descriptor", descriptor) ==
            ANI_OK &&
        env->Object_SetPropertyByName_Int(data, "pid", processStateData.pid) ==
            ANI_OK &&
        env->Object_SetPropertyByName_Int(
            data, "state", static_cast<int32_t>(processStateData.state)) ==
            ANI_OK) {
      for (const auto &item : observers_) {
        env->Object_CallMethodByName_Void(
            reinterpret_cast<ani_object>(item.second),
            "onLaunchServiceProcessStateChanged",
            "C{application.LaunchServiceProcessStateData.LaunchServiceProcessStateData}"
            ":",
            data);
      }
    }
    AppExecFwk::DetachAniEnv(vm_, attached);
  }

  ani_vm *vm_ = nullptr;
  std::atomic_bool active_ = true;
  std::mutex observerMutex_;
  std::map<int32_t, ani_ref> observers_;
  std::mutex handlerMutex_;
  std::shared_ptr<AppExecFwk::EventHandler> handler_;
};

std::mutex g_observerMutex;
std::map<ani_vm *, sptr<EtsLaunchServiceObserver>> g_observers;

void CleanupObservers(ani_vm *vm) {
  sptr<EtsLaunchServiceObserver> observer;
  {
    std::lock_guard<std::mutex> lock(g_observerMutex);
    auto it = g_observers.find(vm);
    if (it == g_observers.end()) {
      return;
    }
    observer = it->second;
    g_observers.erase(it);
  }
  const size_t count = observer->GetObserverCount();
  ani_env *env = nullptr;
  if (vm != nullptr && vm->GetEnv(ANI_VERSION_1, &env) == ANI_OK) {
    observer->Disable(env);
  } else {
    observer->Disable(nullptr);
  }
  for (size_t index = 0; index < count; ++index) {
    DaemonServiceManagerClient::GetInstance().UnregisterObserver(observer);
  }
}

ani_object CreateLaunchServiceError(ani_env *env, int32_t errorCode,
                                    const char *permission) {
  if (errorCode == AAFwk::ABILITY_SERVICE_NOT_CONNECTED) {
    return EtsErrorUtil::CreateError(
        env, CONNECTION_ERROR, "Failed to connect to the system service.");
  }
  return EtsErrorUtil::CreateErrorByNativeErr(env, errorCode, permission);
}

using LaunchServiceClientCall = std::function<int32_t(
    DaemonServiceManagerClient &, const LaunchServiceDescriptor &)>;

void InvokeLaunchServiceCall(ani_env *env, ani_object descriptorObject,
                             ani_object callback, const char *permission,
                             const LaunchServiceClientCall &call) {
  LaunchServiceDescriptor descriptor;
  if (!UnwrapDescriptor(env, descriptorObject, descriptor)) {
    AppExecFwk::AsyncCallback(
        env, callback,
        EtsErrorUtil::CreateInvalidParamError(
            env, "Parameter error. The descriptor is invalid."),
        nullptr);
    return;
  }
  int32_t ret = call(DaemonServiceManagerClient::GetInstance(), descriptor);
  if (ret != ERR_OK) {
    AppExecFwk::AsyncCallback(
        env, callback, CreateLaunchServiceError(env, ret, permission), nullptr);
    return;
  }
  AppExecFwk::AsyncCallback(
      env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK),
      nullptr);
}

void Register(ani_env *env, ani_object descriptorObject, ani_object callback) {
  InvokeLaunchServiceCall(
      env, descriptorObject, callback, REGISTER_DAEMON_PERMISSION,
      [](auto &client, const auto &descriptor) {
        return client.Register(descriptor);
      });
}

void Unregister(ani_env *env, ani_object descriptorObject, ani_object callback) {
  InvokeLaunchServiceCall(
      env, descriptorObject, callback, REGISTER_DAEMON_PERMISSION,
      [](auto &client, const auto &descriptor) {
        return client.Unregister(descriptor);
      });
}

void Start(ani_env *env, ani_object descriptorObject, ani_object callback) {
  InvokeLaunchServiceCall(
      env, descriptorObject, callback, USE_DAEMON_PERMISSION,
      [](auto &client, const auto &descriptor) {
        return client.Start(descriptor);
      });
}

void Stop(ani_env *env, ani_object descriptorObject, ani_object callback) {
  InvokeLaunchServiceCall(
      env, descriptorObject, callback, USE_DAEMON_PERMISSION,
      [](auto &client, const auto &descriptor) {
        return client.Stop(descriptor);
      });
}

void GetInformation(ani_env *env, ani_object descriptorObject,
                    ani_object callback) {
  LaunchServiceDescriptor descriptor;
  if (!UnwrapDescriptor(env, descriptorObject, descriptor)) {
    AppExecFwk::AsyncCallback(
        env, callback,
        EtsErrorUtil::CreateInvalidParamError(
            env, "Parameter error. The descriptor is invalid."),
        nullptr);
    return;
  }
  LaunchServiceInformation information;
  int32_t ret = DaemonServiceManagerClient::GetInstance().GetInformation(
      descriptor, information);
  if (ret != ERR_OK) {
    AppExecFwk::AsyncCallback(env, callback,
                              CreateLaunchServiceError(env, ret, USE_DAEMON_PERMISSION),
                              nullptr);
    return;
  }
  AppExecFwk::AsyncCallback(
      env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK),
      CreateInformation(env, information));
}

void GetAllRegisteredInformation(ani_env *env, ani_int userId,
                                 ani_object callback) {
  if (userId < 0 && userId != -3) {
    AppExecFwk::AsyncCallback(
        env, callback,
        EtsErrorUtil::CreateInvalidParamError(
            env, "Parameter error. The userId is invalid."),
        nullptr);
    return;
  }
  std::vector<LaunchServiceInformation> informations;
  int32_t ret = DaemonServiceManagerClient::GetInstance().GetAllInformations(
      userId, informations);
  if (ret != ERR_OK) {
    AppExecFwk::AsyncCallback(
        env, callback, CreateLaunchServiceError(env, ret, MANAGE_DAEMON_PERMISSION),
        nullptr);
    return;
  }
  AppExecFwk::AsyncCallback(
      env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK),
      CreateInformationArray(env, informations));
}

struct PendingEnableRequest {
  ani_vm *vm = nullptr;
  ani_ref callback = nullptr;
  std::shared_ptr<AppExecFwk::EventHandler> handler;
  bool posted = false;
};

// Completion runs on the main runner; cleanup may run on the VM teardown thread.
// Recursive locking also permits an ETS callback to initiate another request.
std::recursive_mutex g_enableMutex;
std::map<ani_vm *, std::vector<std::shared_ptr<PendingEnableRequest>>> g_enableRequests;

void CleanupEnableRequests(ani_vm *vm) {
  std::lock_guard<std::recursive_mutex> lock(g_enableMutex);
  auto it = g_enableRequests.find(vm);
  if (it == g_enableRequests.end()) {
    return;
  }
  bool attached = false;
  ani_env *env = AppExecFwk::AttachAniEnv(vm, attached);
  for (const auto &request : it->second) {
    if (env != nullptr && request->callback != nullptr) {
      env->GlobalReference_Delete(request->callback);
    }
    request->callback = nullptr;
    request->vm = nullptr;
  }
  g_enableRequests.erase(it);
  AppExecFwk::DetachAniEnv(vm, attached);
}

void HandleEnableResult(const std::shared_ptr<PendingEnableRequest> &request, int32_t result) {
  std::lock_guard<std::recursive_mutex> lock(g_enableMutex);
  auto vm = request->vm;
  if (vm == nullptr || request->callback == nullptr) {
    return;
  }
  bool attached = false;
  ani_env *env = AppExecFwk::AttachAniEnv(vm, attached);
  if (env == nullptr) {
    TAG_LOGE(AAFwkTag::ANI, "attach VM for daemon enable result failed");
    return;
  }
  auto callback = request->callback;
  request->callback = nullptr;
  request->vm = nullptr;
  auto it = g_enableRequests.find(vm);
  if (it != g_enableRequests.end()) {
    auto &requests = it->second;
    requests.erase(std::remove(requests.begin(), requests.end(), request), requests.end());
    if (requests.empty()) {
      g_enableRequests.erase(it);
    }
  }
  ani_object error = result == ERR_OK ?
      EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK) :
      CreateLaunchServiceError(env, result, MANAGE_DAEMON_PERMISSION);
  AppExecFwk::AsyncCallback(env, static_cast<ani_object>(callback), error, nullptr);
  env->GlobalReference_Delete(callback);
  AppExecFwk::DetachAniEnv(vm, attached);
}

void CompleteEnableRequest(const std::shared_ptr<PendingEnableRequest> &request, int32_t result) {
  std::lock_guard<std::recursive_mutex> lock(g_enableMutex);
  if (request->vm == nullptr || request->posted) {
    return;
  }
  request->posted = request->handler->PostTask([request, result]() {
    HandleEnableResult(request, result);
  });
  if (!request->posted) {
    TAG_LOGE(AAFwkTag::ANI, "post daemon enable result failed");
    // No runner is available. Use the common ANI attach helper to finish and
    // release the global reference on this thread instead of leaking the request.
    HandleEnableResult(request, result);
  }
}

void SetDaemonEnable(ani_env *env, ani_string aniBundleName, ani_int userId,
                     ani_int appIndex, ani_boolean enable, ani_object aniToken,
                     ani_object callback) {
  if (env == nullptr) {
    return;
  }
  std::string bundleName;
  if (!AppExecFwk::GetStdString(env, aniBundleName, bundleName) || bundleName.empty() ||
      userId < 0 || appIndex < 0 || appIndex > 5) {
    AppExecFwk::AsyncCallback(
        env, callback,
        EtsErrorUtil::CreateInvalidParamError(
            env, "Parameter error. Invalid daemon identity."),
        nullptr);
    return;
  }
  std::vector<uint8_t> token;
  if (!GetUint8Array(env, aniToken, token)) {
    AppExecFwk::AsyncCallback(
        env, callback,
        EtsErrorUtil::CreateInvalidParamError(
            env, "Parameter error. Invalid authentication token."),
        nullptr);
    return;
  }
  auto request = std::make_shared<PendingEnableRequest>();
  auto runner = AppExecFwk::EventRunner::GetMainEventRunner();
  if (env->GetVM(&request->vm) != ANI_OK || request->vm == nullptr || runner == nullptr ||
      env->GlobalReference_Create(callback, &request->callback) != ANI_OK || request->callback == nullptr) {
    if (request->callback != nullptr) {
      env->GlobalReference_Delete(request->callback);
    }
    AppExecFwk::AsyncCallback(env, callback,
        CreateLaunchServiceError(env, AAFwk::ABILITY_SERVICE_NOT_CONNECTED, MANAGE_DAEMON_PERMISSION), nullptr);
    return;
  }
  request->handler = std::make_shared<AppExecFwk::EventHandler>(runner);
  {
    std::lock_guard<std::recursive_mutex> lock(g_enableMutex);
    g_enableRequests[request->vm].emplace_back(request);
  }
  // Return to taskpool after submission; authentication completes via the callback.
  DaemonServiceManagerClient::GetInstance().SetDaemonEnable(
      bundleName, userId, appIndex, token, enable != ANI_FALSE,
      [request](int32_t result) { CompleteEnableRequest(request, result); });
}

int32_t RemoveLaunchServiceObserver(ani_env *env, ani_vm *vm, int32_t observerId,
                                    bool &invalidObserverId) {
  std::lock_guard<std::mutex> lock(g_observerMutex);
  auto it = g_observers.find(vm);
  if (it == g_observers.end() || !it->second->HasObserver(observerId)) {
    invalidObserverId = true;
    return ERR_OK;
  }
  int32_t ret = DaemonServiceManagerClient::GetInstance().UnregisterObserver(it->second);
  if (ret == ERR_OK) {
    it->second->RemoveObserver(env, observerId);
    if (it->second->IsEmpty()) {
      g_observers.erase(it);
    }
  }
  return ret;
}

ani_int OnLaunchServiceObserver(ani_env *env, ani_object observerObject) {
  ani_vm *vm = nullptr;
  if (env == nullptr || observerObject == nullptr ||
      env->GetVM(&vm) != ANI_OK || vm == nullptr) {
    EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
    return 0;
  }
  std::lock_guard<std::mutex> lock(g_observerMutex);
  auto &observer = g_observers[vm];
  const bool needRegister = observer == nullptr;
  if (needRegister) {
    observer = new (std::nothrow) EtsLaunchServiceObserver(vm);
  }
  if (observer == nullptr) {
    g_observers.erase(vm);
    EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    return 0;
  }
  const int32_t observerId = observer->AddObserver(env, observerObject);
  if (observerId <= 0) {
    if (needRegister) {
      g_observers.erase(vm);
    }
    EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    return 0;
  }
  int32_t ret =
      DaemonServiceManagerClient::GetInstance().RegisterObserver(observer);
  if (ret != ERR_OK) {
    observer->RemoveObserver(env, observerId);
    if (observer->IsEmpty()) {
      g_observers.erase(vm);
    }
    EtsErrorUtil::ThrowError(
        env, CreateLaunchServiceError(env, ret, MANAGE_DAEMON_PERMISSION));
    return 0;
  }
  return observerId;
}

void OffLaunchServiceObserver(ani_env *env, ani_int observerId,
                              ani_object callback) {
  if (observerId <= 0) {
    AppExecFwk::AsyncCallback(
        env, callback, EtsErrorUtil::CreateError(
            env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_LAUNCH_SERVICE_INVALID_ID),
            "The specified observerId is invalid."),
        nullptr);
    return;
  }
  ani_vm *vm = nullptr;
  if (env == nullptr || env->GetVM(&vm) != ANI_OK || vm == nullptr) {
    return;
  }
  bool invalidObserverId = false;
  int32_t ret = RemoveLaunchServiceObserver(env, vm, observerId, invalidObserverId);
  if (invalidObserverId) {
    AppExecFwk::AsyncCallback(
        env, callback, EtsErrorUtil::CreateError(
            env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_LAUNCH_SERVICE_INVALID_ID),
            "The specified observerId is invalid."),
        nullptr);
    return;
  }
  if (ret != ERR_OK) {
    AppExecFwk::AsyncCallback(
        env, callback, CreateLaunchServiceError(env, ret, MANAGE_DAEMON_PERMISSION),
        nullptr);
    return;
  }
  AppExecFwk::AsyncCallback(
      env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK),
      nullptr);
}
} // namespace

void EtsLaunchServiceManagerInit(ani_env *env) {
  ani_namespace ns = nullptr;
  if (env == nullptr || env->FindNamespace(MANAGER_NAMESPACE, &ns) != ANI_OK ||
      ns == nullptr) {
    return;
  }
  std::array methods = {
      ani_native_function{"nativeRegister", DESCRIPTOR_CALL_SIGNATURE,
                          reinterpret_cast<void *>(Register)},
      ani_native_function{"nativeUnregister", DESCRIPTOR_CALL_SIGNATURE,
                          reinterpret_cast<void *>(Unregister)},
      ani_native_function{"nativeStart", DESCRIPTOR_CALL_SIGNATURE,
                          reinterpret_cast<void *>(Start)},
      ani_native_function{"nativeStop", DESCRIPTOR_CALL_SIGNATURE,
                          reinterpret_cast<void *>(Stop)},
      ani_native_function{
          "nativeGetInformation",
          "C{application.LaunchServiceDescriptor.LaunchServiceDescriptor}C{"
          "utils.AbilityUtils.AsyncCallbackWrapper}:",
          reinterpret_cast<void *>(GetInformation)},
      ani_native_function{"nativeGetAllRegisteredInformation",
                          "iC{utils.AbilityUtils.AsyncCallbackWrapper}:",
                          reinterpret_cast<void *>(GetAllRegisteredInformation)},
      ani_native_function{"nativeSetDaemonEnable", SET_ENABLE_SIGNATURE,
                          reinterpret_cast<void *>(SetDaemonEnable)},
      ani_native_function{
          "nativeOnLaunchServiceObserver",
          "C{application.LaunchServiceObserver.LaunchServiceObserver}:i",
          reinterpret_cast<void *>(OnLaunchServiceObserver)},
      ani_native_function{"nativeOffLaunchServiceObserver",
                          "iC{utils.AbilityUtils.AsyncCallbackWrapper}:",
                          reinterpret_cast<void *>(OffLaunchServiceObserver)},
  };
  if (env->Namespace_BindNativeFunctions(ns, methods.data(), methods.size()) !=
      ANI_OK) {
    TAG_LOGE(AAFwkTag::ANI, "bind launch service manager failed");
  }
}

extern "C" ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result) {
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

extern "C" ANI_EXPORT ani_status ANI_Destructor(ani_vm *vm) {
  if (vm == nullptr) {
    return ANI_INVALID_ARGS;
  }
  CleanupEnableRequests(vm);
  CleanupObservers(vm);
  return ANI_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS
