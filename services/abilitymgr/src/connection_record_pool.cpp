/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "connection_record_pool.h"
#include "ability_connect_manager.h"
#include "hilog/log.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ABILITY, "ConnectionRecordPool" };
constexpr size_t DEFAULT_INITIAL_POOL_SIZE = 16;
constexpr size_t DEFAULT_MAX_POOL_SIZE = 128;
} // namespace

ConnectionRecordPool& ConnectionRecordPool::GetInstance()
{
    static ConnectionRecordPool instance;
    return instance;
}

ConnectionRecordPool::ConnectionRecordPool()
    : maxPoolSize_(DEFAULT_MAX_POOL_SIZE),
      initialPoolSize_(DEFAULT_INITIAL_POOL_SIZE)
{
    TAG_LOGI(LABEL, "ConnectionRecordPool created with initialSize=%{public}zu, maxSize=%{public}zu",
        initialPoolSize_, maxPoolSize_);
}

ConnectionRecordPool::~ConnectionRecordPool()
{
    TAG_LOGI(LABEL, "ConnectionRecordPool destroyed. Total allocated: %{public}zu, Total reused: %{public}zu",
        totalAllocated_.load(), totalReused_.load());
}

std::shared_ptr<ConnectionRecord> ConnectionRecordPool::Acquire(
    const sptr<IRemoteObject>& callerToken,
    const std::shared_ptr<BaseExtensionRecord>& targetService,
    const sptr<IAbilityConnection>& connCallback,
    std::shared_ptr<AbilityConnectManager> abilityConnectManager)
{
    // Pre-allocate pool on first access
    if (!preAllocated_.exchange(true)) {
        PreAllocate(initialPoolSize_);
    }

    std::shared_ptr<ConnectionRecord> record;

    {
        std::lock_guard<ffrt::mutex> lock(poolMutex_);
        if (!freeList_.empty()) {
            record = freeList_.front();
            freeList_.pop();
            currentPoolSize_--;
            totalReused_++;

            TAG_LOGD(LABEL, "Pool HIT - reused record, poolSize=%{public}zu", currentPoolSize_.load());
        }
    }

    if (!record) {
        // Pool exhausted, create new object
        record = std::make_shared<ConnectionRecord>(
            callerToken, targetService, connCallback, abilityConnectManager);
        totalAllocated_++;

        TAG_LOGD(LABEL, "Pool MISS - allocated new record, totalAllocated=%{public}zu",
            totalAllocated_.load());

        if (record == nullptr) {
            TAG_LOGE(LABEL, "Failed to allocate ConnectionRecord");
            return nullptr;
        }
    } else {
        // Reuse - reinitialize with new parameters
        ResetConnectionRecord(record.get());
        record->Reinitialize(callerToken, targetService, connCallback, abilityConnectManager);
        TAG_LOGD(LABEL, "Record reinitialized successfully");
    }

    return record;
}

void ConnectionRecordPool::Release(const std::shared_ptr<ConnectionRecord>& record)
{
    if (!record) {
        TAG_LOGW(LABEL, "Attempted to release null record");
        return;
    }

    // Only return DISCONNECTED records to pool
    if (record->GetConnectState() != ConnectionState::DISCONNECTED) {
        TAG_LOGW(LABEL, "Cannot return non-DISCONNECTED record to pool, state=%{public}d",
            static_cast<int>(record->GetConnectState()));
        return;
    }

    std::lock_guard<ffrt::mutex> lock(poolMutex_);

    if (currentPoolSize_ >= maxPoolSize_) {
        // Pool full, let object be destroyed
        TAG_LOGD(LABEL, "Pool full (%{public}zu >= %{public}zu), discarding record",
            currentPoolSize_.load(), maxPoolSize_);
        return;
    }

    freeList_.push(record);
    currentPoolSize_++;

    TAG_LOGD(LABEL, "Record returned to pool, poolSize=%{public}zu", currentPoolSize_.load());
}

ConnectionRecordPool::PoolStats ConnectionRecordPool::GetStats() const
{
    PoolStats stats;
    stats.poolSize = currentPoolSize_.load();
    stats.totalAllocated = totalAllocated_.load();
    stats.totalReused = totalReused_.load();
    stats.currentCapacity = maxPoolSize_;
    return stats;
}

void ConnectionRecordPool::SetMaxPoolSize(size_t maxSize)
{
    if (maxSize == 0) {
        TAG_LOGW(LABEL, "Invalid max pool size: 0, using default");
        return;
    }

    std::lock_guard<ffrt::mutex> lock(poolMutex_);
    maxPoolSize_ = maxSize;
    TAG_LOGI(LABEL, "Max pool size set to %{public}zu", maxSize);

    // Trim pool if current size exceeds new max
    while (currentPoolSize_ > maxPoolSize_ && !freeList_.empty()) {
        freeList_.pop();
        currentPoolSize_--;
    }
}

void ConnectionRecordPool::SetInitialPoolSize(size_t initialSize)
{
    if (initialSize == 0) {
        TAG_LOGW(LABEL, "Invalid initial pool size: 0");
        return;
    }

    initialPoolSize_ = initialSize;
    TAG_LOGI(LABEL, "Initial pool size set to %{public}zu", initialSize);
}

void ConnectionRecordPool::ResetConnectionRecord(ConnectionRecord* record)
{
    if (!record) {
        return;
    }

    // Clear the record for reuse
    record->ClearForReuse();
}

void ConnectionRecordPool::PreAllocate(size_t count)
{
    if (count == 0) {
        return;
    }

    TAG_LOGI(LABEL, "Pre-allocating %{public}zu ConnectionRecord objects", count);

    std::lock_guard<ffrt::mutex> lock(poolMutex_);

    // Only pre-allocate if pool is empty
    if (!freeList_.empty()) {
        TAG_LOGD(LABEL, "Pool not empty, skipping pre-allocation");
        return;
    }

    for (size_t i = 0; i < count && i < maxPoolSize_; ++i) {
        auto record = std::make_shared<ConnectionRecord>(
            nullptr, nullptr, nullptr, nullptr);
        if (record) {
            record->ClearForReuse();
            freeList_.push(record);
            currentPoolSize_++;
            totalAllocated_++;
        }
    }

    TAG_LOGI(LABEL, "Pre-allocated %{public}zu objects", currentPoolSize_.load());
}

} // namespace AAFwk
} // namespace OHOS
