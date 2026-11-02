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

#include "js_launch_service_manager.h"

#include <atomic>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include "ability_business_error.h"
#include "ability_manager_errors.h"
#include "daemon_service_manager_client.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "js_error_utils.h"
#include "napi_common_util.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_FOUR = 4;
constexpr size_t ARGC_FIVE = 5;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t INDEX_TWO = 2;
constexpr size_t INDEX_THREE = 3;
constexpr size_t INDEX_FOUR = 4;
constexpr int32_t CONNECTION_ERROR = 16000050;
constexpr int32_t ALL_USER_ID = -1;
constexpr const char *REGISTER_DAEMON_PERMISSION =
    "ohos.permission.REGISTER_DAEMON_SERVICE";
constexpr const char *USE_DAEMON_PERMISSION =
    "ohos.permission.USE_DAEMON_SERVICE";
constexpr const char *MANAGE_DAEMON_PERMISSION =
    "ohos.permission.MANAGE_DAEMON_SERVICE";

napi_value CreateDescriptor(napi_env env,
                            const LaunchServiceDescriptor &descriptor);

std::atomic<uint32_t> g_nextObserverId{0};

bool ConvertUint8Array(napi_env env, napi_value value,
                       std::vector<uint8_t> &result) {
  bool isTypedArray = false;
  if (napi_is_typedarray(env, value, &isTypedArray) != napi_ok ||
      !isTypedArray) {
    return false;
  }
  napi_typedarray_type type;
  size_t length = 0;
  void *data = nullptr;
  if (napi_get_typedarray_info(env, value, &type, &length, &data, nullptr,
                               nullptr) != napi_ok ||
      type != napi_uint8_array || (length > 0 && data == nullptr)) {
    return false;
  }
  if (length == 0) {
    result.clear();
    return true;
  }
  auto *bytes = static_cast<uint8_t *>(data);
  result.assign(bytes, bytes + length);
  return true;
}

bool IsUndefined(napi_env env, napi_value value) {
  napi_valuetype type = napi_undefined;
  return value != nullptr && napi_typeof(env, value, &type) == napi_ok &&
         type == napi_undefined;
}

class JsLaunchServiceObserver final : public LaunchServiceObserverStub {
public:
  explicit JsLaunchServiceObserver(napi_env env) : env_(env) {}

  int32_t AddObserver(napi_value observer) {
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
    observers_.emplace(observerId,
                       std::shared_ptr<NativeReference>(
                           reinterpret_cast<NativeReference *>(ref)));
    return observerId;
  }

  bool HasObserver(int32_t observerId) {
    std::lock_guard<std::mutex> lock(observerMutex_);
    return observers_.find(observerId) != observers_.end();
  }

  bool IsLastObserver(int32_t observerId) {
    std::lock_guard<std::mutex> lock(observerMutex_);
    return observers_.size() == 1 &&
           observers_.find(observerId) != observers_.end();
  }

  void RemoveObserver(int32_t observerId) {
    std::lock_guard<std::mutex> lock(observerMutex_);
    observers_.erase(observerId);
  }

  bool IsEmpty() {
    std::lock_guard<std::mutex> lock(observerMutex_);
    return observers_.empty();
  }

  void Disable() {
    active_.store(false);
    std::lock_guard<std::mutex> lock(observerMutex_);
    observers_.clear();
  }

  void OnLaunchServiceStatusChanged(
      const LaunchServiceStateData &stateData) override {
    Schedule("onLaunchServiceRegisterStatusChanged", stateData.descriptor,
             static_cast<int32_t>(stateData.state), 0, false);
  }

  void OnLaunchServiceProcessStateChanged(
      const LaunchServiceProcessStateData &processStateData) override {
    Schedule("onLaunchServiceProcessStateChanged", processStateData.descriptor,
             static_cast<int32_t>(processStateData.state), processStateData.pid,
             true);
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

  void Schedule(const char *method, const LaunchServiceDescriptor &descriptor,
                int32_t state, int32_t pid, bool processData) {
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
          std::vector<std::shared_ptr<NativeReference>> observers;
          {
            std::lock_guard<std::mutex> lock(self->observerMutex_);
            for (const auto &item : self->observers_) {
              observers.emplace_back(item.second);
            }
          }
          napi_value data = nullptr;
          napi_create_object(env, &data);
          napi_set_named_property(env, data, "descriptor",
                                  CreateDescriptor(env, descriptor));
          napi_value argument = data;
          if (processData) {
            napi_set_named_property(env, data, "pid", CreateJsValue(env, pid));
            napi_set_named_property(env, data, "state",
                                    CreateJsValue(env, state));
          } else {
            napi_set_named_property(
                env, data, "registerState",
                CreateJsValue(env, state != static_cast<int32_t>(
                    LaunchServiceState::NOT_REGISTERED)));
            napi_create_array_with_length(env, 1, &argument);
            napi_set_element(env, argument, 0, data);
          }
          for (const auto &observer : observers) {
            napi_value object = observer->GetNapiValue();
            napi_value callback = nullptr;
            if (object == nullptr ||
                napi_get_named_property(env, object, methodName.c_str(),
                                        &callback) != napi_ok) {
              continue;
            }
            napi_value result = nullptr;
            napi_call_function(env, object, callback, 1, &argument, &result);
          }
        });
    napi_ref callback = nullptr;
    NapiAsyncTask::Schedule("JsLaunchServiceObserver::Notify", env_,
                            std::make_unique<NapiAsyncTask>(
                                callback, nullptr, std::move(complete)));
  }

  napi_env env_ = nullptr;
  std::atomic_bool active_ = true;
  std::mutex observerMutex_;
  std::map<int32_t, std::shared_ptr<NativeReference>> observers_;
};

std::mutex g_observerMutex;
std::map<napi_env, sptr<JsLaunchServiceObserver>> g_observers;

void CleanupObservers(void *data) {
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
    DaemonServiceManagerClient::GetInstance().UnregisterObserver(observer);
    observer->Disable();
  }
}

bool HasFunction(napi_env env, napi_value object, const char *name) {
  napi_value method = nullptr;
  napi_valuetype type = napi_undefined;
  return napi_get_named_property(env, object, name, &method) == napi_ok &&
         method != nullptr && napi_typeof(env, method, &type) == napi_ok &&
         type == napi_function;
}

bool GetRequiredString(napi_env env, napi_value object, const char *name,
                       std::string &value) {
  napi_value property = nullptr;
  return napi_get_named_property(env, object, name, &property) == napi_ok &&
         property != nullptr && ConvertFromJsValue(env, property, value) &&
         !value.empty();
}

bool GetOptionalInt(napi_env env, napi_value object, const char *name,
                    bool &hasValue, int32_t &value) {
  bool hasProperty = false;
  if (napi_has_named_property(env, object, name, &hasProperty) != napi_ok) {
    return false;
  }
  hasValue = hasProperty;
  if (!hasProperty) {
    return true;
  }
  napi_value property = nullptr;
  return napi_get_named_property(env, object, name, &property) == napi_ok &&
         property != nullptr && ConvertFromJsValue(env, property, value) &&
         value >= 0;
}

bool UnwrapDescriptor(napi_env env, napi_value value,
                      LaunchServiceDescriptor &descriptor) {
  napi_valuetype type = napi_undefined;
  if (napi_typeof(env, value, &type) != napi_ok || type != napi_object) {
    return false;
  }
  return GetRequiredString(env, value, "bundleName", descriptor.bundleName) &&
         GetRequiredString(env, value, "moduleName", descriptor.moduleName) &&
         GetRequiredString(env, value, "serviceName", descriptor.serviceName) &&
         GetOptionalInt(env, value, "userId", descriptor.hasUserId,
                        descriptor.userId) &&
         GetOptionalInt(env, value, "appIndex", descriptor.hasAppIndex,
                        descriptor.appIndex);
}

napi_value CreateDescriptor(napi_env env,
                            const LaunchServiceDescriptor &descriptor) {
  napi_value object = nullptr;
  napi_create_object(env, &object);
  napi_set_named_property(env, object, "bundleName",
                          CreateJsValue(env, descriptor.bundleName));
  napi_set_named_property(env, object, "moduleName",
                          CreateJsValue(env, descriptor.moduleName));
  napi_set_named_property(env, object, "serviceName",
                          CreateJsValue(env, descriptor.serviceName));
  if (descriptor.hasUserId) {
    napi_set_named_property(env, object, "userId",
                            CreateJsValue(env, descriptor.userId));
  }
  if (descriptor.hasAppIndex) {
    napi_set_named_property(env, object, "appIndex",
                            CreateJsValue(env, descriptor.appIndex));
  }
  return object;
}

napi_value CreateInformation(napi_env env,
                             const LaunchServiceInformation &information) {
  napi_value object = nullptr;
  napi_create_object(env, &object);
  napi_set_named_property(env, object, "descriptor",
                          CreateDescriptor(env, information.descriptor));
  napi_set_named_property(
      env, object, "state",
      CreateJsValue(env, static_cast<int32_t>(information.state)));
  napi_set_named_property(env, object, "pid",
                          CreateJsValue(env, information.pid));
  return object;
}

napi_value CreateInformationArray(
    napi_env env, const std::vector<LaunchServiceInformation> &informations) {
  napi_value array = nullptr;
  napi_create_array_with_length(env, informations.size(), &array);
  for (size_t index = 0; index < informations.size(); ++index) {
    napi_set_element(env, array, index,
                     CreateInformation(env, informations[index]));
  }
  return array;
}

napi_value CreateLaunchServiceError(napi_env env, int32_t errorCode,
                                    const char *permission) {
  if (errorCode == AAFwk::ABILITY_SERVICE_NOT_CONNECTED) {
    return CreateJsError(env, CONNECTION_ERROR,
                         "Failed to connect to the system service.");
  }
  return CreateJsErrorByNativeErr(env, errorCode, permission);
}

using LaunchServiceClientCall = std::function<int32_t(
    DaemonServiceManagerClient &, const LaunchServiceDescriptor &)>;

napi_value InvokeLaunchServiceCall(napi_env env, napi_callback_info info,
                                   const char *taskName, const char *permission,
                                   const LaunchServiceClientCall &call) {
  size_t argc = ARGC_ONE;
  napi_value argv[ARGC_ONE] = {nullptr};
  napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
  if (argc != ARGC_ONE) {
    ThrowInvalidNumParametersError(env);
    return CreateJsUndefined(env);
  }
  auto descriptor = std::make_shared<LaunchServiceDescriptor>();
  if (!UnwrapDescriptor(env, argv[INDEX_ZERO], *descriptor)) {
    ThrowInvalidParamError(env, "Parameter error. The descriptor is invalid.");
    return CreateJsUndefined(env);
  }
  auto errorCode = std::make_shared<int32_t>(ERR_OK);
  NapiAsyncTask::ExecuteCallback execute = [descriptor, errorCode, call]() {
    *errorCode = call(DaemonServiceManagerClient::GetInstance(), *descriptor);
  };
  NapiAsyncTask::CompleteCallback complete =
      [errorCode, permission](napi_env env, NapiAsyncTask &task, int32_t) {
        if (*errorCode != ERR_OK) {
          task.Reject(env, CreateLaunchServiceError(env, *errorCode, permission));
          return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
      };
  napi_value promise = nullptr;
  NapiAsyncTask::Schedule(
      taskName, env, CreateAsyncTaskWithLastParam(
          env, nullptr, std::move(execute), std::move(complete), &promise));
  return promise;
}

napi_value Register(napi_env env, napi_callback_info info) {
  return InvokeLaunchServiceCall(env, info, "JsLaunchServiceManager::Register",
      REGISTER_DAEMON_PERMISSION, [](auto &client, const auto &descriptor) {
        return client.Register(descriptor);
      });
}

napi_value Unregister(napi_env env, napi_callback_info info) {
  return InvokeLaunchServiceCall(env, info, "JsLaunchServiceManager::Unregister",
      REGISTER_DAEMON_PERMISSION, [](auto &client, const auto &descriptor) {
        return client.Unregister(descriptor);
      });
}

napi_value Start(napi_env env, napi_callback_info info) {
  return InvokeLaunchServiceCall(env, info, "JsLaunchServiceManager::Start",
      USE_DAEMON_PERMISSION, [](auto &client, const auto &descriptor) {
        return client.Start(descriptor);
      });
}

napi_value Stop(napi_env env, napi_callback_info info) {
  return InvokeLaunchServiceCall(env, info, "JsLaunchServiceManager::Stop",
      USE_DAEMON_PERMISSION, [](auto &client, const auto &descriptor) {
        return client.Stop(descriptor);
      });
}

napi_value GetInformation(napi_env env, napi_callback_info info) {
  size_t argc = ARGC_ONE;
  napi_value argv[ARGC_ONE] = {nullptr};
  napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
  if (argc != ARGC_ONE) {
    ThrowInvalidNumParametersError(env);
    return CreateJsUndefined(env);
  }
  auto descriptor = std::make_shared<LaunchServiceDescriptor>();
  if (!UnwrapDescriptor(env, argv[INDEX_ZERO], *descriptor)) {
    ThrowInvalidParamError(env, "Parameter error. The descriptor is invalid.");
    return CreateJsUndefined(env);
  }
  auto result = std::make_shared<LaunchServiceInformation>();
  auto errorCode = std::make_shared<int32_t>(ERR_OK);
  NapiAsyncTask::ExecuteCallback execute = [descriptor, result, errorCode]() {
    *errorCode = DaemonServiceManagerClient::GetInstance().GetInformation(
        *descriptor, *result);
  };
  NapiAsyncTask::CompleteCallback complete =
      [result, errorCode](napi_env env, NapiAsyncTask &task, int32_t) {
        if (*errorCode != ERR_OK) {
          task.Reject(env,
                      CreateLaunchServiceError(env, *errorCode, USE_DAEMON_PERMISSION));
          return;
        }
        task.ResolveWithNoError(env, CreateInformation(env, *result));
      };
  napi_value promise = nullptr;
  NapiAsyncTask::Schedule(
      "JsLaunchServiceManager::GetInformation", env,
      CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute),
                                   std::move(complete), &promise));
  return promise;
}

napi_value GetAllRegisteredInformation(napi_env env, napi_callback_info info) {
  size_t argc = ARGC_ONE;
  napi_value argv[ARGC_ONE] = {nullptr};
  napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
  auto userId = std::make_shared<int32_t>(ALL_USER_ID);
  if (argc == ARGC_ONE && !IsUndefined(env, argv[INDEX_ZERO]) &&
      (!ConvertFromJsValue(env, argv[INDEX_ZERO], *userId) || *userId < 0)) {
    ThrowInvalidParamError(env, "Parameter error. The userId is invalid.");
    return CreateJsUndefined(env);
  }
  auto result = std::make_shared<std::vector<LaunchServiceInformation>>();
  auto errorCode = std::make_shared<int32_t>(ERR_OK);
  NapiAsyncTask::ExecuteCallback execute = [userId, result, errorCode]() {
    *errorCode = DaemonServiceManagerClient::GetInstance().GetAllInformations(
        *userId, *result);
  };
  NapiAsyncTask::CompleteCallback complete =
      [result, errorCode](napi_env env, NapiAsyncTask &task, int32_t) {
        if (*errorCode != ERR_OK) {
          task.Reject(
              env, CreateLaunchServiceError(env, *errorCode, MANAGE_DAEMON_PERMISSION));
          return;
        }
        task.ResolveWithNoError(env, CreateInformationArray(env, *result));
      };
  napi_value promise = nullptr;
  NapiAsyncTask::Schedule(
      "JsLaunchServiceManager::GetAllRegisteredInformation", env,
      CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute),
                                   std::move(complete), &promise));
  return promise;
}

napi_value SetDaemonEnable(napi_env env, napi_callback_info info) {
  size_t argc = ARGC_FIVE;
  napi_value argv[ARGC_FIVE] = {nullptr};
  napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
  if (argc != ARGC_FOUR && argc != ARGC_FIVE) {
    ThrowInvalidNumParametersError(env);
    return CreateJsUndefined(env);
  }

  std::string bundleName;
  int32_t userId = 0;
  int32_t appIndex = 0;
  std::vector<uint8_t> token;
  bool enable = false;
  if (!ConvertFromJsValue(env, argv[INDEX_ZERO], bundleName) || bundleName.empty() ||
      !ConvertFromJsValue(env, argv[INDEX_ONE], userId) ||
      userId < 0 ||
      !ConvertFromJsValue(env, argv[INDEX_TWO], appIndex) ||
      appIndex < 0 ||
      !ConvertFromJsValue(env, argv[INDEX_THREE], enable) ||
      (argc == ARGC_FIVE && !IsUndefined(env, argv[INDEX_FOUR]) &&
       !ConvertUint8Array(env, argv[INDEX_FOUR], token))) {
    ThrowInvalidParamError(env, "Parameter error. Invalid daemon identity, token, or enable state.");
    return CreateJsUndefined(env);
  }

  auto errorCode = std::make_shared<int32_t>(ERR_OK);
  NapiAsyncTask::ExecuteCallback execute =
      [bundleName, userId, appIndex, token, enable, errorCode]() {
        *errorCode = DaemonServiceManagerClient::GetInstance()
                         .SetDaemonEnable(bundleName, userId, appIndex, token, enable);
      };
  NapiAsyncTask::CompleteCallback complete =
      [errorCode](napi_env env, NapiAsyncTask &task, int32_t) {
        if (*errorCode != ERR_OK) {
          task.Reject(env, CreateLaunchServiceError(
              env, *errorCode, MANAGE_DAEMON_PERMISSION));
          return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
      };
  napi_value promise = nullptr;
  NapiAsyncTask::ScheduleHighQos(
      "JsLaunchServiceManager::SetDaemonEnable", env,
      CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute),
                                   std::move(complete), &promise));
  return promise;
}

int32_t RemoveLaunchServiceObserver(napi_env env, int32_t observerId,
                                    bool &invalidObserverId) {
  std::lock_guard<std::mutex> lock(g_observerMutex);
  auto it = g_observers.find(env);
  if (it == g_observers.end() || !it->second->HasObserver(observerId)) {
    invalidObserverId = true;
    return ERR_OK;
  }
  int32_t ret = ERR_OK;
  if (it->second->IsLastObserver(observerId)) {
    ret = DaemonServiceManagerClient::GetInstance().UnregisterObserver(it->second);
  }
  if (ret == ERR_OK) {
    it->second->RemoveObserver(observerId);
    if (it->second->IsEmpty()) {
      g_observers.erase(it);
    }
  }
  return ret;
}

napi_value OnLaunchServiceObserver(napi_env env, napi_callback_info info) {
  size_t argc = ARGC_ONE;
  napi_value argv[ARGC_ONE] = {nullptr};
  napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
  napi_valuetype type = napi_undefined;
  if (argc != ARGC_ONE ||
      napi_typeof(env, argv[INDEX_ZERO], &type) != napi_ok ||
      type != napi_object ||
      !HasFunction(env, argv[INDEX_ZERO],
                   "onLaunchServiceRegisterStatusChanged") ||
      !HasFunction(env, argv[INDEX_ZERO],
                   "onLaunchServiceProcessStateChanged")) {
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
      ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
      return CreateJsUndefined(env);
    }
  }
  const int32_t observerId = observer->AddObserver(argv[INDEX_ZERO]);
  if (observerId <= 0) {
    if (needRegister) {
      g_observers.erase(env);
    }
    ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    return CreateJsUndefined(env);
  }
  if (needRegister) {
    int32_t ret =
        DaemonServiceManagerClient::GetInstance().RegisterObserver(observer);
    if (ret != ERR_OK) {
      observer->RemoveObserver(observerId);
      g_observers.erase(env);
      napi_throw(env, CreateLaunchServiceError(env, ret, MANAGE_DAEMON_PERMISSION));
      return CreateJsUndefined(env);
    }
  }
  return CreateJsValue(env, observerId);
}

napi_value OffLaunchServiceObserver(napi_env env, napi_callback_info info) {
  size_t argc = ARGC_ONE;
  napi_value argv[ARGC_ONE] = {nullptr};
  napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
  auto observerId = std::make_shared<int32_t>(0);
  if (argc != ARGC_ONE ||
      !ConvertFromJsValue(env, argv[INDEX_ZERO], *observerId)) {
    ThrowInvalidParamError(env, "Parameter error. The observerId is invalid.");
    return CreateJsUndefined(env);
  }
  NapiAsyncTask::ExecuteCallback execute = []() {};
  NapiAsyncTask::CompleteCallback complete = [observerId](napi_env env,
                                                          NapiAsyncTask &task,
                                                          int32_t) {
    bool invalidObserverId = false;
    int32_t ret = RemoveLaunchServiceObserver(env, *observerId, invalidObserverId);
    if (invalidObserverId) {
      task.Reject(env,
                  CreateJsError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_LAUNCH_SERVICE_INVALID_ID),
                                "The specified observerId is invalid."));
      return;
    }
    if (ret != ERR_OK) {
      task.Reject(env, CreateLaunchServiceError(env, ret, MANAGE_DAEMON_PERMISSION));
      return;
    }
    task.ResolveWithNoError(env, CreateJsUndefined(env));
  };
  napi_value promise = nullptr;
  NapiAsyncTask::Schedule(
      "JsLaunchServiceManager::OffLaunchServiceObserver", env,
      CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute),
                                   std::move(complete), &promise));
  return promise;
}
} // namespace

napi_value JsLaunchServiceManagerInit(napi_env env, napi_value exports) {
  napi_add_env_cleanup_hook(env, CleanupObservers, env);
  BindNativeFunction(env, exports, "register", "JsLaunchServiceManager", Register);
  BindNativeFunction(env, exports, "unregister", "JsLaunchServiceManager", Unregister);
  BindNativeFunction(env, exports, "start", "JsLaunchServiceManager", Start);
  BindNativeFunction(env, exports, "stop", "JsLaunchServiceManager", Stop);
  BindNativeFunction(env, exports, "getInformation", "JsLaunchServiceManager",
                     GetInformation);
  BindNativeFunction(env, exports, "getAllRegisteredInformation",
                     "JsLaunchServiceManager", GetAllRegisteredInformation);
  BindNativeFunction(env, exports, "setDaemonEnable",
                     "JsLaunchServiceManager", SetDaemonEnable);
  BindNativeFunction(env, exports, "onLaunchServiceObserver",
                     "JsLaunchServiceManager", OnLaunchServiceObserver);
  BindNativeFunction(env, exports, "offLaunchServiceObserver",
                     "JsLaunchServiceManager", OffLaunchServiceObserver);

  napi_value state = nullptr;
  napi_create_object(env, &state);
  napi_set_named_property(
      env, state, "NOT_REGISTERED",
      CreateJsValue(env,
                    static_cast<int32_t>(LaunchServiceState::NOT_REGISTERED)));
  napi_set_named_property(
      env, state, "ENABLED",
      CreateJsValue(env, static_cast<int32_t>(LaunchServiceState::ENABLED)));
  napi_set_named_property(
      env, state, "DISABLED",
      CreateJsValue(env, static_cast<int32_t>(LaunchServiceState::DISABLED)));
  napi_set_named_property(env, exports, "LaunchServiceState", state);
  napi_value processState = nullptr;
  napi_create_object(env, &processState);
  napi_set_named_property(
      env, processState, "CREATED",
      CreateJsValue(env,
                    static_cast<int32_t>(LaunchServiceProcessState::CREATED)));
  napi_set_named_property(
      env, processState, "TERMINATED",
      CreateJsValue(
          env, static_cast<int32_t>(LaunchServiceProcessState::TERMINATED)));
  napi_set_named_property(env, exports, "LaunchServiceProcessState",
                          processState);
  return exports;
}
} // namespace AbilityRuntime
} // namespace OHOS
