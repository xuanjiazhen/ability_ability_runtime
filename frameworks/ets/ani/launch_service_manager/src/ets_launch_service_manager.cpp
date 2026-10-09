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

#include <array>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include "ability_business_error.h"
#include "ability_manager_errors.h"
#include "ani_common_util.h"
#include "daemon_service_manager_client.h"
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
    "application.LaunchServiceObserver.LaunchServiceStateDataInner";
constexpr const char *PROCESS_STATE_DATA_CLASS =
    "application.LaunchServiceObserver.LaunchServiceProcessStateDataInner";
constexpr int32_t CONNECTION_ERROR = 16000050;
constexpr const char *USE_DAEMON_PERMISSION =
    "ohos.permission.USE_DAEMON_SERVICE";
constexpr const char *MANAGE_DAEMON_PERMISSION =
    "ohos.permission.MANAGE_DAEMON_SERVICE";

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
  EtsLaunchServiceObserver(ani_vm *vm, ani_ref observer)
      : vm_(vm), observer_(observer) {}

  void OnLaunchServiceStatusChanged(
      const LaunchServiceStateData &stateData) override {
    PostTask([weak = wptr<EtsLaunchServiceObserver>(this), stateData]() {
      auto self = weak.promote();
      if (self != nullptr) {
        self->HandleStatusChanged(stateData);
      }
    });
  }

  void OnLaunchServiceProcessStateChanged(
      const LaunchServiceProcessStateData &processStateData) override {
    PostTask([weak = wptr<EtsLaunchServiceObserver>(this), processStateData]() {
      auto self = weak.promote();
      if (self != nullptr) {
        self->HandleProcessStateChanged(processStateData);
      }
    });
  }

  void Release(ani_env *env) {
    std::lock_guard<std::mutex> lock(observerMutex_);
    if (observer_ != nullptr) {
      env->GlobalReference_Delete(observer_);
      observer_ = nullptr;
    }
  }

private:
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
    if (observer_ == nullptr) {
      AppExecFwk::DetachAniEnv(vm_, attached);
      return;
    }
    ani_object data = NewObject(env, STATE_DATA_CLASS);
    ani_object descriptor = CreateDescriptor(env, stateData.descriptor);
    if (data != nullptr && descriptor != nullptr &&
        env->Object_SetPropertyByName_Ref(data, "descriptor", descriptor) ==
            ANI_OK &&
        env->Object_SetPropertyByName_Int(
            data, "state", static_cast<int32_t>(stateData.state)) == ANI_OK) {
      env->Object_CallMethodByName_Void(
          reinterpret_cast<ani_object>(observer_),
          "onLaunchServiceStatusChanged",
          "C{application.LaunchServiceObserver.LaunchServiceStateData}:", data);
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
    if (observer_ == nullptr) {
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
      env->Object_CallMethodByName_Void(
          reinterpret_cast<ani_object>(observer_),
          "onLaunchServiceProcessStateChanged",
          "C{application.LaunchServiceObserver.LaunchServiceProcessStateData}:",
          data);
    }
    AppExecFwk::DetachAniEnv(vm_, attached);
  }

  ani_vm *vm_ = nullptr;
  ani_ref observer_ = nullptr;
  std::mutex observerMutex_;
  std::mutex handlerMutex_;
  std::shared_ptr<AppExecFwk::EventHandler> handler_;
};

std::mutex g_observerMutex;
std::map<int32_t, sptr<EtsLaunchServiceObserver>> g_observers;

ani_object CreateQueryError(ani_env *env, int32_t errorCode,
                            const char *permission) {
  if (errorCode == AAFwk::ERR_NULL_AGENT_MGR_PROXY) {
    return EtsErrorUtil::CreateError(
        env, CONNECTION_ERROR, "Failed to connect to the system service.");
  }
  if (errorCode == ERR_LAUNCH_SERVICE_NOT_FOUND) {
    return EtsErrorUtil::CreateError(
        env, errorCode, "The specified launch service does not exist.");
  }
  return EtsErrorUtil::CreateErrorByNativeErr(env, errorCode, permission);
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
                              CreateQueryError(env, ret, USE_DAEMON_PERMISSION),
                              nullptr);
    return;
  }
  AppExecFwk::AsyncCallback(
      env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK),
      CreateInformation(env, information));
}

void GetAllInformations(ani_env *env, ani_object callback) {
  std::vector<LaunchServiceInformation> informations;
  int32_t ret = DaemonServiceManagerClient::GetInstance().GetAllInformations(
      informations);
  if (ret != ERR_OK) {
    AppExecFwk::AsyncCallback(
        env, callback, CreateQueryError(env, ret, MANAGE_DAEMON_PERMISSION),
        nullptr);
    return;
  }
  AppExecFwk::AsyncCallback(
      env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK),
      CreateInformationArray(env, informations));
}

ani_int OnLaunchServiceObserver(ani_env *env, ani_object observerObject) {
  ani_vm *vm = nullptr;
  ani_ref observerRef = nullptr;
  if (env == nullptr || observerObject == nullptr ||
      env->GetVM(&vm) != ANI_OK || vm == nullptr ||
      env->GlobalReference_Create(observerObject, &observerRef) != ANI_OK ||
      observerRef == nullptr) {
    EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
    return 0;
  }
  sptr<EtsLaunchServiceObserver> observer =
      new (std::nothrow) EtsLaunchServiceObserver(vm, observerRef);
  if (observer == nullptr) {
    env->GlobalReference_Delete(observerRef);
    EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    return 0;
  }
  int32_t observerId = 0;
  int32_t ret = DaemonServiceManagerClient::GetInstance().RegisterObserver(
      observer, observerId);
  if (ret != ERR_OK) {
    observer->Release(env);
    if (ret == AAFwk::ERR_NULL_AGENT_MGR_PROXY) {
      EtsErrorUtil::ThrowError(env, CONNECTION_ERROR,
                               "Failed to connect to the system service.");
    } else {
      EtsErrorUtil::ThrowErrorByNativeErr(env, ret);
    }
    return 0;
  }
  std::lock_guard<std::mutex> lock(g_observerMutex);
  g_observers[observerId] = observer;
  return observerId;
}

void OffLaunchServiceObserver(ani_env *env, ani_int observerId,
                              ani_object callback) {
  if (observerId <= 0) {
    AppExecFwk::AsyncCallback(
        env, callback,
        EtsErrorUtil::CreateInvalidParamError(
            env, "Parameter error. The observerId is invalid."),
        nullptr);
    return;
  }
  int32_t ret =
      DaemonServiceManagerClient::GetInstance().UnregisterObserver(observerId);
  if (ret != ERR_OK) {
    AppExecFwk::AsyncCallback(
        env, callback, CreateQueryError(env, ret, MANAGE_DAEMON_PERMISSION),
        nullptr);
    return;
  }
  {
    std::lock_guard<std::mutex> lock(g_observerMutex);
    auto it = g_observers.find(observerId);
    if (it != g_observers.end()) {
      it->second->Release(env);
      g_observers.erase(it);
    }
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
      ani_native_function{
          "nativeGetInformation",
          "C{application.LaunchServiceDescriptor.LaunchServiceDescriptor}C{"
          "utils.AbilityUtils.AsyncCallbackWrapper}:",
          reinterpret_cast<void *>(GetInformation)},
      ani_native_function{"nativeGetAllInformations",
                          "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                          reinterpret_cast<void *>(GetAllInformations)},
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
} // namespace AbilityRuntime
} // namespace OHOS
