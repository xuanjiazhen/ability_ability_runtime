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

#ifndef OHOS_APP_EXECFWK_TASK_MGR_CRYPTO_HELPER_H
#define OHOS_APP_EXECFWK_TASK_MGR_CRYPTO_HELPER_H

#include <cstdint>
#include <functional>
#include <mutex>
#include <vector>

#include "refbase.h"

namespace OHOS {
namespace StorageManager {
class IStorageManager;
}

namespace AppExecFwk {
/**
 * @brief Bridges the task lifecycle to the EL5 user-key crypto interfaces of the
 *        storage manager: StartTask unlocks (decrypts) the caller user's EL5 key with
 *        the SAF agent fence token (the secret stays empty by design), and the task
 *        finish paths lock (encrypt) it back.
 * @note Thread-safe singleton. The real path performs synchronous cross-SA IPC, so
 *       callers must not hold locks across the calls (OH-7 lock rule).
 */
class CryptoHelper {
public:
    using DecryptFunc = std::function<int32_t(uint32_t, const std::vector<uint8_t> &)>;
    using EncryptFunc = std::function<int32_t(uint32_t)>;

    static CryptoHelper &GetInstance();

    /**
     * @brief Unlocks (decrypts) the user's EL5 user key through the storage manager.
     * @param userId Indicates the OS account id of the task creator.
     * @param token Indicates the auth token bytes retrieved by the SAF agent fence;
     *               never logged and wiped by the caller after use.
     * @return Returns ERR_OK on success, or the storage manager error code on
     *         failure (the caller maps it into its own error space).
     */
    [[nodiscard]] int32_t DecryptUserKey(uint32_t userId, const std::vector<uint8_t> &token);

    /**
     * @brief Locks (encrypts) the user's EL5 user key back through the storage manager.
     * @param userId Indicates the OS account id of the task creator.
     * @return Returns ERR_OK on success, or the storage manager error code on failure.
     */
    [[nodiscard]] int32_t EncryptUserKey(uint32_t userId);

    /**
     * @brief Replaces both crypto calls for unit tests; nullptr restores the real path.
     */
    void SetCryptoFuncsForTest(DecryptFunc decrypt, EncryptFunc encrypt);

private:
    CryptoHelper() = default;
    ~CryptoHelper() = default;

    sptr<StorageManager::IStorageManager> GetStorageManager();

    std::mutex mutex_;
    DecryptFunc decrypt_;
    EncryptFunc encrypt_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_APP_EXECFWK_TASK_MGR_CRYPTO_HELPER_H
