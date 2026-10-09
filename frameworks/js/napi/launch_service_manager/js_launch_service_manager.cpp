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
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include "ability_business_error.h"
#include "ability_manager_errors.h"
#include "daemon_service_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "js_error_utils.h"
#include "napi_common_util.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t INDEX_ZERO = 0;
constexpr int32_t CONNECTION_ERROR = 16000050;
constexpr const char *USE_DAEMON_PERMISSION =
    "ohos.permission.USE_DAEMON_SERVICE";
constexpr const char *MANAGE_DAEMON_PERMISSION =
    "ohos.permission.MANAGE_DAEMON_SERVICE";

napi_value CreateDescriptor(napi_env env,
                            const LaunchServiceDescriptor &descriptor);

class JsLaunchServiceObserver final : public LaunchServiceObserverStub {
public:
  JsLaunchServiceObserver(napi_env env, napi_value observer) : env_(env) {
    napi_ref ref = nullptr;
    if (napi_create_reference(env, observer, 1, &ref) == napi_ok) {
      observer_ = std::shared_ptr<NativeReference>(
          reinterpret_cast<NativeReference *>(ref));
    }
  }

  bool IsValid() const { return observer_ != nullptr; }
  napi_env GetEnv() const { return env_; }
  void Disable() { active_.store(false); }

  void OnLaunchServiceStatusChanged(
      const LaunchServiceStateData &stateData) override {
    Schedule("onLaunchServiceStatusChanged", stateData.descriptor,
             static_cast<int32_t>(stateData.state), 0, false);
  }

  void OnLaunchServiceProcessStateChanged(
      const LaunchServiceProcessStateData &processStateData) override {
    Schedule("onLaunchServiceProcessStateChanged", processStateData.descriptor,
             static_cast<int32_t>(processStateData.state), processStateData.pid,
             true);
  }

private:
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
          if (self == nullptr || !self->active_.load() ||
              self->observer_ == nullptr) {
            return;
          }
          napi_value object = self->observer_->GetNapiValue();
          napi_value callback = nullptr;
          if (object == nullptr ||
              napi_get_named_property(env, object, methodName.c_str(),
                                      &callback) != napi_ok) {
            return;
          }
          napi_value data = nullptr;
          napi_create_object(env, &data);
          napi_set_named_property(env, data, "descriptor",
                                  CreateDescriptor(env, descriptor));
          if (processData) {
            napi_set_named_property(env, data, "pid", CreateJsValue(env, pid));
          }
          napi_set_named_property(env, data, "state",
                                  CreateJsValue(env, state));
          napi_value result = nullptr;
          napi_call_function(env, object, callback, 1, &data, &result);
        });
    napi_ref callback = nullptr;
    NapiAsyncTask::Schedule(
        "JsLaunchServiceObserver::Notify", env_,
        std::make_unique<NapiAsyncTask>(callback, nullptr, std::move(complete)));
  }

  napi_env env_ = nullptr;
  std::atomic_bool active_ = true;
  std::shared_ptr<NativeReference> observer_;
};

std::mutex g_observerMutex;
std::map<int32_t, sptr<JsLaunchServiceObserver>> g_observers;

void CleanupObservers(void *data) {
  napi_env env = static_cast<napi_env>(data);
  std::vector<int32_t> observerIds;
  {
    std::lock_guard<std::mutex> lock(g_observerMutex);
    for (auto it = g_observers.begin(); it != g_observers.end();) {
      if (it->second != nullptr && it->second->GetEnv() == env) {
        it->second->Disable();
        observerIds.emplace_back(it->first);
        it = g_observers.erase(it);
      } else {
        ++it;
      }
    }
  }
  for (int32_t observerId : observerIds) {
    DaemonServiceManagerClient::GetInstance().UnregisterObserver(observerId);
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

napi_value CreateQueryError(napi_env env, int32_t errorCode,
                            const char *permission) {
  if (errorCode == AAFwk::ERR_NULL_AGENT_MGR_PROXY) {
    return CreateJsError(env, CONNECTION_ERROR,
                         "Failed to connect to the system service.");
  }
  return CreateJsErrorByNativeErr(env, errorCode, permission);
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
                      CreateQueryError(env, *errorCode, USE_DAEMON_PERMISSION));
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

napi_value GetAllInformations(napi_env env, napi_callback_info info) {
  auto result = std::make_shared<std::vector<LaunchServiceInformation>>();
  auto errorCode = std::make_shared<int32_t>(ERR_OK);
  NapiAsyncTask::ExecuteCallback execute = [result, errorCode]() {
    *errorCode =
        DaemonServiceManagerClient::GetInstance().GetAllInformations(*result);
  };
  NapiAsyncTask::CompleteCallback complete =
      [result, errorCode](napi_env env, NapiAsyncTask &task, int32_t) {
        if (*errorCode != ERR_OK) {
          task.Reject(
              env, CreateQueryError(env, *errorCode, MANAGE_DAEMON_PERMISSION));
          return;
        }
        task.ResolveWithNoError(env, CreateInformationArray(env, *result));
      };
  napi_value promise = nullptr;
  NapiAsyncTask::Schedule(
      "JsLaunchServiceManager::GetAllInformations", env,
      CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute),
                                   std::move(complete), &promise));
  return promise;
}

napi_value OnLaunchServiceObserver(napi_env env, napi_callback_info info) {
  size_t argc = ARGC_ONE;
  napi_value argv[ARGC_ONE] = {nullptr};
  napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
  napi_valuetype type = napi_undefined;
  if (argc != ARGC_ONE ||
      napi_typeof(env, argv[INDEX_ZERO], &type) != napi_ok ||
      type != napi_object ||
      !HasFunction(env, argv[INDEX_ZERO], "onLaunchServiceStatusChanged") ||
      !HasFunction(env, argv[INDEX_ZERO],
                   "onLaunchServiceProcessStateChanged")) {
    ThrowInvalidParamError(env, "Parameter error. The observer is invalid.");
    return CreateJsUndefined(env);
  }
  sptr<JsLaunchServiceObserver> observer =
      new (std::nothrow) JsLaunchServiceObserver(env, argv[INDEX_ZERO]);
  if (observer == nullptr || !observer->IsValid()) {
    ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    return CreateJsUndefined(env);
  }
  int32_t observerId = 0;
  int32_t ret = DaemonServiceManagerClient::GetInstance().RegisterObserver(
      observer, observerId);
  if (ret != ERR_OK) {
    napi_throw(env, CreateQueryError(env, ret, MANAGE_DAEMON_PERMISSION));
    return CreateJsUndefined(env);
  }
  {
    std::lock_guard<std::mutex> lock(g_observerMutex);
    g_observers[observerId] = observer;
  }
  return CreateJsValue(env, observerId);
}

napi_value OffLaunchServiceObserver(napi_env env, napi_callback_info info) {
  size_t argc = ARGC_ONE;
  napi_value argv[ARGC_ONE] = {nullptr};
  napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
  auto observerId = std::make_shared<int32_t>(0);
  if (argc != ARGC_ONE ||
      !ConvertFromJsValue(env, argv[INDEX_ZERO], *observerId) ||
      *observerId <= 0) {
    ThrowInvalidParamError(env, "Parameter error. The observerId is invalid.");
    return CreateJsUndefined(env);
  }
  auto errorCode = std::make_shared<int32_t>(ERR_OK);
  NapiAsyncTask::ExecuteCallback execute = [observerId, errorCode]() {
    *errorCode = DaemonServiceManagerClient::GetInstance().UnregisterObserver(
        *observerId);
  };
  NapiAsyncTask::CompleteCallback complete =
      [observerId, errorCode](napi_env env, NapiAsyncTask &task, int32_t) {
        if (*errorCode != ERR_OK) {
          task.Reject(
              env, CreateQueryError(env, *errorCode, MANAGE_DAEMON_PERMISSION));
          return;
        }
        {
          std::lock_guard<std::mutex> lock(g_observerMutex);
          auto it = g_observers.find(*observerId);
          if (it != g_observers.end()) {
            it->second->Disable();
          }
          g_observers.erase(*observerId);
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
  BindNativeFunction(env, exports, "getInformation", "JsLaunchServiceManager",
                     GetInformation);
  BindNativeFunction(env, exports, "getAllInformations",
                     "JsLaunchServiceManager", GetAllInformations);
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
