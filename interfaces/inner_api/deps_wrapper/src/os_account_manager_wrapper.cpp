/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "os_account_manager_wrapper.h"

#include "hilog_tag_wrapper.h"
#ifdef OS_ACCOUNT_PART_ENABLED
#include "os_account_manager.h"
#endif // OS_ACCOUNT_PART_ENABLED

namespace OHOS {
namespace AppExecFwk {
namespace {
#ifndef OS_ACCOUNT_PART_ENABLED
const int32_t DEFAULT_OS_ACCOUNT_ID = 0; // default id when there is no os_account part
const int32_t USER_ID_U100 = 100;
const int32_t UID_TRANSFORM_DIVISOR = 200000;
#endif // OS_ACCOUNT_PART_ENABLED
constexpr int32_t ACCOUNT_MGR_SERVICE_UID = 3058;
}

ErrCode OsAccountManagerWrapper::QueryActiveOsAccountIds(std::vector<int32_t>& ids)
{
#ifndef OS_ACCOUNT_PART_ENABLED
    TAG_LOGD(AAFwkTag::DEFAULT, "Without os account subsystem");
    ids.emplace_back(DEFAULT_OS_ACCOUNT_ID);
    return ERR_OK;
#else
    TAG_LOGD(AAFwkTag::DEFAULT, "os account subsystem");
    return AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
#endif // OS_ACCOUNT_PART_ENABLED
}

ErrCode OsAccountManagerWrapper::GetOsAccountLocalIdFromUid(const int32_t uid, int32_t &id)
{
#ifndef OS_ACCOUNT_PART_ENABLED
    TAG_LOGD(AAFwkTag::DEFAULT, "Without os account subsystem");
    id = uid / UID_TRANSFORM_DIVISOR;
    return ERR_OK;
#else
    TAG_LOGD(AAFwkTag::DEFAULT, "os account subsystem");
    return AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(uid, id);
#endif // OS_ACCOUNT_PART_ENABLED
}

ErrCode OsAccountManagerWrapper::GetOsAccountLocalIdFromProcess(int &id)
{
#ifndef OS_ACCOUNT_PART_ENABLED
    TAG_LOGD(AAFwkTag::DEFAULT, "Without os account subsystem");
    id = DEFAULT_OS_ACCOUNT_ID;
    return ERR_OK;
#else
    TAG_LOGD(AAFwkTag::DEFAULT, "os account subsystem");
    return AccountSA::OsAccountManager::GetOsAccountLocalIdFromProcess(id);
#endif // OS_ACCOUNT_PART_ENABLED
}

ErrCode OsAccountManagerWrapper::IsOsAccountExists(const int id, bool &isOsAccountExists)
{
#ifndef OS_ACCOUNT_PART_ENABLED
    TAG_LOGD(AAFwkTag::DEFAULT, "Without os account subsystem");
    isOsAccountExists = (id == DEFAULT_OS_ACCOUNT_ID);
    return ERR_OK;
#else // OS_ACCOUNT_PART_ENABLED
    TAG_LOGD(AAFwkTag::DEFAULT, "os account subsystem");
    return AccountSA::OsAccountManager::IsOsAccountExists(id, isOsAccountExists);
#endif // OS_ACCOUNT_PART_ENABLED
}

ErrCode OsAccountManagerWrapper::CreateOsAccount(const std::string &name, int32_t &osAccountUserId)
{
#ifndef OS_ACCOUNT_PART_ENABLED
    TAG_LOGD(AAFwkTag::DEFAULT, "Without os account subsystem");
    osAccountUserId = USER_ID_U100;
    return ERR_OK;
#else // OS_ACCOUNT_PART_ENABLED
    TAG_LOGD(AAFwkTag::DEFAULT, "os account subsystem");
    AccountSA::OsAccountInfo osAccountInfo;
    ErrCode errCode = AccountSA::OsAccountManager::CreateOsAccount(name,
        AccountSA::OsAccountType::NORMAL, osAccountInfo);
    osAccountUserId = osAccountInfo.GetLocalId();
    return errCode;
#endif // OS_ACCOUNT_PART_ENABLED
}

ErrCode OsAccountManagerWrapper::RemoveOsAccount(const int id)
{
#ifndef OS_ACCOUNT_PART_ENABLED
    TAG_LOGD(AAFwkTag::DEFAULT, "Without os account subsystem");
    return ERR_OK;
#else // OS_ACCOUNT_PART_ENABLED
    TAG_LOGD(AAFwkTag::DEFAULT, "os account subsystem");
    return AccountSA::OsAccountManager::RemoveOsAccount(id);
#endif // OS_ACCOUNT_PART_ENABLED
}

int32_t OsAccountManagerWrapper::GetCurrentActiveAccountId()
{
    std::vector<int32_t> accountIds;
    auto instance = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance();
    if (instance == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "Get OsAccountManager Failed");
        return 0;
    }

    ErrCode ret = instance->QueryActiveOsAccountIds(accountIds);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::DEFAULT, "Query active id failed. ret: %{public}d", ret);
        return 0;
    }

    if (accountIds.empty()) {
        TAG_LOGE(AAFwkTag::DEFAULT, "account empty");
        return 0;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "accountId: %{public}d", accountIds[0]);
    return accountIds[0];
}

bool OsAccountManagerWrapper::IsOsAccountForeground(const int32_t userId, bool isSwitchToUser, int32_t callerUid)
{
#ifndef OS_ACCOUNT_PART_ENABLED
    TAG_LOGD(AAFwkTag::DEFAULT, "userId: %{public}d is not in foreground accounts.", userId);
    return USER_ID_U100 == userId;
#else // OS_ACCOUNT_PART_ENABLED
    if (isSwitchToUser && callerUid == ACCOUNT_MGR_SERVICE_UID) {
        TAG_LOGD(AAFwkTag::DEFAULT,
            "userId: %{public}d is not in foreground accounts, but it is a account mgr service uid.", userId);
        return true; // Account Manager Service UID is always considered as foreground
    }

    std::vector<AccountSA::ForegroundOsAccount> accounts;
    ErrCode errCode = AccountSA::OsAccountManager::GetForegroundOsAccounts(accounts);
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::DEFAULT, "GetForegroundOsAccounts failed, errCode: %{public}d, userId: %{public}d",
            errCode, userId);
        return false;
    }

    for (const auto& account : accounts) {
        if (account.localId == userId) {
            TAG_LOGD(AAFwkTag::DEFAULT, "userId: %{public}d is in foreground accounts.", userId);
            return true;
        }
    }

    TAG_LOGW(AAFwkTag::DEFAULT,
        "userId: %{public}d is not in foreground accounts. size: %{public}zu", userId, accounts.size());
    return false;
#endif // OS_ACCOUNT_PART_ENABLED
}

ErrCode OsAccountManagerWrapper::GetForegroundOsAccountLocalId(int32_t &userId)
{
#ifndef OS_ACCOUNT_PART_ENABLED
    userId = USER_ID_U100;
    TAG_LOGD(AAFwkTag::DEFAULT, "Without os account subsystem, set userId: %{public}d", userId);
    return ERR_OK; // No foreground account in this case
#else // OS_ACCOUNT_PART_ENABLED
    auto errCode = AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(userId);
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::DEFAULT,
            "GetForegroundOsAccountLocalId failed, errCode: %{public}d, userId: %{public}d", errCode, userId);
        return ERR_OK;
    }
    return errCode;
#endif // OS_ACCOUNT_PART_ENABLED
}

}  // namespace AppExecFwk
}  // namespace OHOS