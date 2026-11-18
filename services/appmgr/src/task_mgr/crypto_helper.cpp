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

#include "task_mgr/crypto_helper.h"

#include <cstdint>
#include <utility>
#include <vector>

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "istorage_manager.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {

CryptoHelper &CryptoHelper::GetInstance()
{
    static CryptoHelper instance;
    return instance;
}

sptr<StorageManager::IStorageManager> CryptoHelper::GetStorageManager()
{
    // shortcut: no cached proxy / death recipient — task start and finish are rare,
    // so the per-call lookup beats maintaining a stale-proxy lifecycle.
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null systemAbilityMgr");
        return nullptr;
    }
    return iface_cast<StorageManager::IStorageManager>(
        systemAbilityMgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID));
}

int32_t CryptoHelper::DecryptUserKey(uint32_t userId, const std::vector<uint8_t> &token)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (decrypt_) {
            return decrypt_(userId, token);
        }
    }
    auto mgr = GetStorageManager();
    if (mgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null storageManager, userId: %{public}d", static_cast<int32_t>(userId));
        return AAFwk::INNER_ERR;
    }
    // token-only unlock: the secret stays empty by storage-side contract
    return mgr->DecryptEl5UserKey(userId, token, std::vector<uint8_t>());
}

int32_t CryptoHelper::EncryptUserKey(uint32_t userId)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (encrypt_) {
            return encrypt_(userId);
        }
    }
    auto mgr = GetStorageManager();
    if (mgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null storageManager, userId: %{public}d", static_cast<int32_t>(userId));
        return AAFwk::INNER_ERR;
    }
    return mgr->EncryptEl5UserKey(userId);
}

void CryptoHelper::SetCryptoFuncsForTest(DecryptFunc decrypt, EncryptFunc encrypt)
{
    std::lock_guard<std::mutex> lock(mutex_);
    decrypt_ = std::move(decrypt);
    encrypt_ = std::move(encrypt);
}
}  // namespace AppExecFwk
}  // namespace OHOS
