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

#ifndef OHOS_AGENT_RUNTIME_LAUNCH_SERVICE_OBSERVER_MANAGER_H
#define OHOS_AGENT_RUNTIME_LAUNCH_SERVICE_OBSERVER_MANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include "launch_service_observer.h"

namespace OHOS {
namespace AgentRuntime {
class LaunchServiceObserverManager final {
public:
  static LaunchServiceObserverManager &GetInstance();

  int32_t RegisterObserver(const sptr<IRemoteObject> &remote,
                           uint64_t ownerTokenId, int32_t &observerId);
  int32_t UnregisterObserver(int32_t observerId, uint64_t ownerTokenId);
  void
  NotifyStatusChanged(const AbilityRuntime::LaunchServiceStateData &stateData);
  void NotifyProcessStateChanged(
      const AbilityRuntime::LaunchServiceProcessStateData &processStateData);

private:
  class ObserverDeathRecipient final : public IRemoteObject::DeathRecipient {
  public:
    explicit ObserverDeathRecipient(int32_t observerId)
        : observerId_(observerId) {}
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

  private:
    int32_t observerId_;
  };

  struct ObserverRecord {
    sptr<AbilityRuntime::ILaunchServiceObserver> observer;
    sptr<IRemoteObject::DeathRecipient> deathRecipient;
    uint64_t ownerTokenId = 0;
    std::mutex mutex;
    bool active = true;
  };

  LaunchServiceObserverManager() = default;
  int32_t AllocateObserverIdLocked();
  int32_t RemoveObserver(int32_t observerId);

  std::mutex mutex_;
  std::map<int32_t, std::shared_ptr<ObserverRecord>> observers_;
  int32_t nextObserverId_ = 1;
};
} // namespace AgentRuntime
} // namespace OHOS

#endif // OHOS_AGENT_RUNTIME_LAUNCH_SERVICE_OBSERVER_MANAGER_H
