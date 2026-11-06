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

#include "daemon_service_manager_client.h"

#include "ability_manager_errors.h"
#include "app_mgr_client.h"
#include "parameters.h"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char DEVICE_TYPE_PARAMETER[] = "const.product.devicetype";
constexpr char PC_MODE_PARAMETER[] = "persist.sceneboard.ispcmode";

bool IsLaunchServiceSupported() {
  const std::string deviceType =
      system::GetParameter(DEVICE_TYPE_PARAMETER, "");
  return (deviceType == "2in1" || deviceType == "tablet") &&
         system::GetBoolParameter(PC_MODE_PARAMETER, false);
}

int32_t NormalizeAppMgrResult(int32_t result) {
  return result == AppExecFwk::ERROR_SERVICE_NOT_CONNECTED
             ? AAFwk::ABILITY_SERVICE_NOT_CONNECTED
             : result;
}

template<typename Callable>
int32_t InvokeAppMgr(Callable &&call) {
  if (!IsLaunchServiceSupported()) {
    return AAFwk::ERR_CAPABILITY_NOT_SUPPORT;
  }
  auto client = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
  if (client == nullptr) {
    return AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
  }
  return NormalizeAppMgrResult(call(client));
}

AAFwk::LaunchServiceDescriptor ConvertDescriptor(
    const LaunchServiceDescriptor &descriptor) {
  AAFwk::LaunchServiceDescriptor result;
  result.bundleName = descriptor.bundleName;
  result.moduleName = descriptor.moduleName;
  result.serviceName = descriptor.serviceName;
  result.userId = descriptor.GetUserIdOrDefault();
  result.appIndex = descriptor.GetAppIndexOrDefault();
  return result;
}
} // namespace

DaemonServiceManagerClient &DaemonServiceManagerClient::GetInstance() {
  static DaemonServiceManagerClient instance;
  return instance;
}

int32_t DaemonServiceManagerClient::Register(
    const LaunchServiceDescriptor &descriptor) {
  const auto appMgrDescriptor = ConvertDescriptor(descriptor);
  return InvokeAppMgr([&appMgrDescriptor](const auto &client) {
    return client->RegisterDaemon(appMgrDescriptor);
  });
}

int32_t DaemonServiceManagerClient::Unregister(
    const LaunchServiceDescriptor &descriptor) {
  const auto appMgrDescriptor = ConvertDescriptor(descriptor);
  return InvokeAppMgr([&appMgrDescriptor](const auto &client) {
    return client->UnregisterDaemon(appMgrDescriptor);
  });
}

int32_t DaemonServiceManagerClient::Start(
    const LaunchServiceDescriptor &descriptor) {
  const auto appMgrDescriptor = ConvertDescriptor(descriptor);
  return InvokeAppMgr([&appMgrDescriptor](const auto &client) {
    return client->StartDaemon(appMgrDescriptor);
  });
}

int32_t DaemonServiceManagerClient::Stop(
    const LaunchServiceDescriptor &descriptor) {
  const auto appMgrDescriptor = ConvertDescriptor(descriptor);
  return InvokeAppMgr([&appMgrDescriptor](const auto &client) {
    return client->StopDaemon(appMgrDescriptor);
  });
}

int32_t DaemonServiceManagerClient::GetInformation(
    const LaunchServiceDescriptor &descriptor,
    LaunchServiceInformation &information) {
  return InvokeAppMgr([&descriptor, &information](const auto &client) {
    return client->GetLaunchServiceInformation(descriptor, information);
  });
}

int32_t DaemonServiceManagerClient::GetAllInformations(
    int32_t userId, std::vector<LaunchServiceInformation> &informations) {
  return InvokeAppMgr([userId, &informations](const auto &client) {
    return client->GetAllLaunchServiceInformations(userId, informations);
  });
}

void DaemonServiceManagerClient::SetDaemonEnable(
    const std::string &bundleName, int32_t userId, int32_t appIndex,
    const std::vector<uint8_t> &token, bool enable, std::function<void(int32_t)> completion) {
  if (!completion) {
    return;
  }
  if (!IsLaunchServiceSupported()) {
    completion(AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    return;
  }
  auto client = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
  if (client == nullptr) {
    completion(AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
    return;
  }
  client->SetDaemonEnable(bundleName, userId, appIndex, token, enable,
      [completion = std::move(completion)](int32_t result) {
        completion(NormalizeAppMgrResult(result));
      });
}

int32_t DaemonServiceManagerClient::RegisterObserver(
    const sptr<ILaunchServiceObserver> &observer) {
  return InvokeAppMgr([&observer](const auto &client) {
    return client->RegisterLaunchServiceObserver(observer);
  });
}

int32_t DaemonServiceManagerClient::UnregisterObserver(
    const sptr<ILaunchServiceObserver> &observer) {
  return InvokeAppMgr([&observer](const auto &client) {
    return client->UnregisterLaunchServiceObserver(observer);
  });
}
} // namespace AbilityRuntime
} // namespace OHOS
