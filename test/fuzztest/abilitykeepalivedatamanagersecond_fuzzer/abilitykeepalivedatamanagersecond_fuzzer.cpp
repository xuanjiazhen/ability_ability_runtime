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

#include "abilitykeepalivedatamanagersecond_fuzzer.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#include "ability_keep_alive_data_manager.h"
#undef private

#include "distributed_kv_data_manager.h"
#include "keep_alive_info.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    (void)fdp.ConsumeBool();

    auto &mgr = AbilityKeepAliveDataManager::GetInstance();

    // IsRecoverableStatus: covered for both recoverable and unrecoverable statuses.
    (void)mgr.IsRecoverableStatus(DistributedKv::Status::DATA_CORRUPTED);
    (void)mgr.IsRecoverableStatus(DistributedKv::Status::DB_ERROR);
    (void)mgr.IsRecoverableStatus(DistributedKv::Status::SUCCESS);

    // BackupKvStore is null-safe (checks CheckKvStore internally).
    mgr.BackupKvStore();

    // ScheduleBackupFlush is safe regardless of kv store state.
    mgr.ScheduleBackupFlush(std::chrono::steady_clock::now());

    // RetryBackup / DetectAndHealCorruptedStore / RestoreFromBackupWithRetry /
    // RestoreIfStoreEmpty dereference kvStorePtr_ directly, so only invoke them
    // once a store instance is actually available.
    (void)mgr.GetKvStore();
    if (mgr.kvStorePtr_ == nullptr) {
        return false;
    }

    (void)mgr.RetryBackup();
    mgr.DetectAndHealCorruptedStore(DistributedKv::Status::DATA_CORRUPTED);
    mgr.DetectAndHealCorruptedStore(DistributedKv::Status::DB_ERROR);
    (void)mgr.RestoreFromBackupWithRetry();
    mgr.RestoreIfStoreEmpty();
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}