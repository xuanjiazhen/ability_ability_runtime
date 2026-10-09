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
#include "agent_manager_client.h"
#include "parameters.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char DEVICE_TYPE_PARAMETER[] = "const.product.devicetype";
constexpr char PC_MODE_PARAMETER[] = "persist.sceneboard.ispcmode";

bool IsLaunchServiceSupported() {
  const std::string deviceType =
      system::GetParameter(DEVICE_TYPE_PARAMETER, "");
  return deviceType == "2in1" ||
         (deviceType == "tablet" &&
          system::GetBoolParameter(PC_MODE_PARAMETER, false));
}
} // namespace

DaemonServiceManagerClient &DaemonServiceManagerClient::GetInstance() {
  static DaemonServiceManagerClient instance;
  return instance;
}

int32_t DaemonServiceManagerClient::GetInformation(
    const LaunchServiceDescriptor &descriptor,
    LaunchServiceInformation &information) {
  if (!IsLaunchServiceSupported()) {
    return AAFwk::ERR_CAPABILITY_NOT_SUPPORT;
  }
  return AgentRuntime::AgentManagerClient::GetInstance()
      .GetLaunchServiceInformation(descriptor, information);
}

int32_t DaemonServiceManagerClient::GetAllInformations(
    std::vector<LaunchServiceInformation> &informations) {
  if (!IsLaunchServiceSupported()) {
    return AAFwk::ERR_CAPABILITY_NOT_SUPPORT;
  }
  return AgentRuntime::AgentManagerClient::GetInstance()
      .GetAllLaunchServiceInformations(informations);
}

int32_t DaemonServiceManagerClient::RegisterObserver(
    const sptr<ILaunchServiceObserver> &observer, int32_t &observerId) {
  if (!IsLaunchServiceSupported()) {
    return AAFwk::ERR_CAPABILITY_NOT_SUPPORT;
  }
  return AgentRuntime::AgentManagerClient::GetInstance()
      .RegisterLaunchServiceObserver(observer, observerId);
}

int32_t DaemonServiceManagerClient::UnregisterObserver(int32_t observerId) {
  if (!IsLaunchServiceSupported()) {
    return AAFwk::ERR_CAPABILITY_NOT_SUPPORT;
  }
  return AgentRuntime::AgentManagerClient::GetInstance()
      .UnregisterLaunchServiceObserver(observerId);
}
} // namespace AbilityRuntime
} // namespace OHOS
