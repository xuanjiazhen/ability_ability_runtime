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

#ifndef OHOS_ABILITY_RUNTIME_CONNECTION_RECORD_POOL_H
#define OHOS_ABILITY_RUNTIME_CONNECTION_RECORD_POOL_H

#include <atomic>
#include <memory>
#include <queue>
#include <vector>
#include "cpp/mutex.h"
#include "connection_record.h"
#include "hilog_tag.h"

namespace OHOS {
namespace AAFwk {

/**
 * @class ConnectionRecordPool
 * ConnectionRecordPool manages a pool of reusable ConnectionRecord objects
 * to reduce memory allocation overhead and fragmentation.
 */
class ConnectionRecordPool {
public:
    /**
     * Get the singleton instance of the pool.
     *
     * @return Reference to the singleton pool instance.
     */
    static ConnectionRecordPool& GetInstance();

    /**
     * Acquire a ConnectionRecord from the pool or create a new one.
     *
     * @param callerToken The token of the caller ability.
     * @param targetService The target service ability.
     * @param connCallback The connection callback.
     * @param abilityConnectManager Pointer to the connect manager.
     * @return Shared pointer to the ConnectionRecord.
     */
    std::shared_ptr<ConnectionRecord> Acquire(
        const sptr<IRemoteObject>& callerToken,
        const std::shared_ptr<BaseExtensionRecord>& targetService,
        const sptr<IAbilityConnection>& connCallback,
        std::shared_ptr<AbilityConnectManager> abilityConnectManager);

    /**
     * Return a ConnectionRecord to the pool for reuse.
     *
     * @param record The ConnectionRecord to return to pool.
     */
    void Release(const std::shared_ptr<ConnectionRecord>& record);

    /**
     * Pool statistics structure.
     */
    struct PoolStats {
        size_t poolSize;        ///< Current number of objects in pool
        size_t totalAllocated;  ///< Total objects allocated since pool creation
        size_t totalReused;     ///< Total objects reused from pool
        size_t currentCapacity; ///< Current pool capacity (max pool size)
    };

    /**
     * Get current pool statistics.
     *
     * @return Current pool statistics.
     */
    PoolStats GetStats() const;

    /**
     * Set the maximum pool size.
     *
     * @param maxSize Maximum number of objects to keep in pool.
     */
    void SetMaxPoolSize(size_t maxSize);

    /**
     * Set the initial pool size (pre-allocated on first access).
     *
     * @param initialSize Number of objects to pre-allocate.
     */
    void SetInitialPoolSize(size_t initialSize);

private:
    ConnectionRecordPool();
    ~ConnectionRecordPool();

    DISALLOW_COPY_AND_MOVE(ConnectionRecordPool);

    /**
     * Reset a ConnectionRecord to its initial state for reuse.
     *
     * @param record The ConnectionRecord to reset.
     */
    void ResetConnectionRecord(ConnectionRecord* record);

    /**
     * Pre-allocate a specified number of ConnectionRecord objects.
     *
     * @param count Number of objects to pre-allocate.
     */
    void PreAllocate(size_t count);

    mutable ffrt::mutex poolMutex_;
    std::queue<std::shared_ptr<ConnectionRecord>> freeList_;

    // Configuration
    size_t maxPoolSize_;
    size_t initialPoolSize_;

    // Statistics
    std::atomic<size_t> totalAllocated_{0};
    std::atomic<size_t> totalReused_{0};
    std::atomic<size_t> currentPoolSize_{0};
    bool preAllocated_{false};
};

} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CONNECTION_RECORD_POOL_H
