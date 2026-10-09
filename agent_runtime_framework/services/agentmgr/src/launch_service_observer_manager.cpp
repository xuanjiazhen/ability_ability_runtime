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

#include "launch_service_observer_manager.h"

#include <limits>

#include "ipc_types.h"

namespace OHOS {
namespace AgentRuntime {
LaunchServiceObserverManager &LaunchServiceObserverManager::GetInstance() {
  static LaunchServiceObserverManager instance;
  return instance;
}

int32_t LaunchServiceObserverManager::AllocateObserverIdLocked() {
  for (size_t count = 0; count <= observers_.size(); ++count) {
    if (nextObserverId_ <= 0) {
      nextObserverId_ = 1;
    }
    const int32_t candidate = nextObserverId_;
    nextObserverId_ = nextObserverId_ == std::numeric_limits<int32_t>::max()
                          ? 1
                          : nextObserverId_ + 1;
    if (observers_.find(candidate) == observers_.end()) {
      return candidate;
    }
  }
  return -1;
}

int32_t LaunchServiceObserverManager::RegisterObserver(
    const sptr<IRemoteObject> &remote, uint64_t ownerTokenId,
    int32_t &observerId) {
  if (remote == nullptr) {
    return ERR_INVALID_VALUE;
  }
  auto observer = iface_cast<AbilityRuntime::ILaunchServiceObserver>(remote);
  if (observer == nullptr) {
    return ERR_INVALID_VALUE;
  }

  std::lock_guard<std::mutex> lock(mutex_);
  const int32_t id = AllocateObserverIdLocked();
  if (id <= 0) {
    return ERR_NO_MEMORY;
  }
  sptr<IRemoteObject::DeathRecipient> recipient =
      new (std::nothrow) ObserverDeathRecipient(id);
  if (recipient == nullptr) {
    return ERR_NO_MEMORY;
  }
  auto record = std::make_shared<ObserverRecord>();
  record->observer = observer;
  record->deathRecipient = recipient;
  record->ownerTokenId = ownerTokenId;
  observers_.emplace(id, record);
  if (remote->IsProxyObject() && !remote->AddDeathRecipient(recipient)) {
    observers_.erase(id);
    return ERR_INVALID_STATE;
  }
  observerId = id;
  return ERR_OK;
}

int32_t
LaunchServiceObserverManager::UnregisterObserver(int32_t observerId,
                                                 uint64_t ownerTokenId) {
  std::shared_ptr<ObserverRecord> record;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = observers_.find(observerId);
    if (it == observers_.end() || it->second->ownerTokenId != ownerTokenId) {
      return ERR_OK;
    }
    record = it->second;
    observers_.erase(it);
  }
  {
    std::lock_guard<std::mutex> lock(record->mutex);
    record->active = false;
  }
  auto remote = record->observer->AsObject();
  if (remote != nullptr && remote->IsProxyObject()) {
    remote->RemoveDeathRecipient(record->deathRecipient);
  }
  return ERR_OK;
}

int32_t LaunchServiceObserverManager::RemoveObserver(int32_t observerId) {
  std::shared_ptr<ObserverRecord> record;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = observers_.find(observerId);
    if (it == observers_.end()) {
      return ERR_OK;
    }
    record = it->second;
    observers_.erase(it);
  }
  {
    std::lock_guard<std::mutex> lock(record->mutex);
    record->active = false;
  }
  auto remote = record->observer->AsObject();
  if (remote != nullptr && remote->IsProxyObject()) {
    remote->RemoveDeathRecipient(record->deathRecipient);
  }
  return ERR_OK;
}

void LaunchServiceObserverManager::NotifyStatusChanged(
    const AbilityRuntime::LaunchServiceStateData &stateData) {
  std::vector<std::shared_ptr<ObserverRecord>> observers;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto &item : observers_) {
      observers.emplace_back(item.second);
    }
  }
  for (const auto &record : observers) {
    std::lock_guard<std::mutex> lock(record->mutex);
    if (record->active) {
      record->observer->OnLaunchServiceStatusChanged(stateData);
    }
  }
}

void LaunchServiceObserverManager::NotifyProcessStateChanged(
    const AbilityRuntime::LaunchServiceProcessStateData &processStateData) {
  std::vector<std::shared_ptr<ObserverRecord>> observers;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto &item : observers_) {
      observers.emplace_back(item.second);
    }
  }
  for (const auto &record : observers) {
    std::lock_guard<std::mutex> lock(record->mutex);
    if (record->active) {
      record->observer->OnLaunchServiceProcessStateChanged(processStateData);
    }
  }
}

void LaunchServiceObserverManager::ObserverDeathRecipient::OnRemoteDied(
    [[maybe_unused]] const wptr<IRemoteObject> &remote) {
  LaunchServiceObserverManager::GetInstance().RemoveObserver(observerId_);
}
} // namespace AgentRuntime
} // namespace OHOS
