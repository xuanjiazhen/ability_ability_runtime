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

#include "page_data_manager.h"

#include <charconv>

#include "ability_context.h"
#include "match_tag_wrapper.h"
#include "ipc_skeleton_utils.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr const char* SETTING_DATA_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
constexpr const char* SETTING_DATA_KEY_URI = "&key=";
constexpr const char* SETTING_DATA_COLUMN_KEYWORD = "KEYWORD";
constexpr const char* SETTING_DATA_COLUMN_VALUE = "VALUE";
constexpr int32_t INDEX0 = 0;
}

PageDataManager &PageDataManager::GetInstance()
{
    static PageDataManager instance;
    return instance;
}

ErrCode PageDataManager::Initialize(std::weak_ptr<OHOS::AbilityRuntime::AbilityContext> &context)
{
    std::lock_guard guard(initializeMutex_);
    if (isInitialized_) {
        TAG_LOGD(AAFwkTag::ABILITY, "already initialized");
        return ERR_OK;
    }
    auto sharedContext = context.lock();
    if (sharedContext) {
        remoteObject_ = sharedContext->GetToken();
    } else {
        remoteObject_ = nullptr;
        TAG_LOGE(AAFwkTag::ABILITY, "remoteObj is null");
        return ERR_NO_INIT;
    }
    isInitialized_ = true;
    return ERR_OK;
}

bool PageDataManager::IsInitialized() const
{
    return isInitialized_;
}

void PageDataManager::CreateDataShareHelperAndUri(const std::string& key,
    std::string& uri, std::shared_ptr<DataShare::DataShareHelper>& helper) const
{
    uri = AssembleUri(key);
    helper = CreateDataShareHelper();
}

std::shared_ptr<DataShare::DataShareHelper> PageDataManager::CreateDataShareHelper() const
{
    if (remoteObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "remoteObject_ is null");
        return nullptr;
    }
    const auto [errCode, helper] =
        DataShare::DataShareHelper::Create(remoteObject_, SETTING_DATA_URI,"");
    if (errCode != DataShare::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "create data share helper failed: %{public}d", errCode);
        return nullptr;
    }

    return helper;
}

ErrCode PageDataManager::GetStringValue(const std::string &key, std::string &value) {
    std::string uriString;
    ResetCallingIdentityScope scope;
    std::shared_ptr<DataShare::DataShareHelper> helper;
    CreateDataShareHelperAndUri(key, uriString, helper);
    if (helper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "helper is null");
        return ERR_NO_INIT;
    }

    std::vector<std::string> columns = { SETTING_DATA_COLUMN_VALUE };
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_DATA_COLUMN_KEYWORD, key);
    Uri uri(uriString);
    auto result = helper->Query(uri, predicates, columns);
    ReleaseDataShareHelper(helper);
    if (result == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "query return null, key: %{public}s", key.c_str());
        return ERR_INVALID_OPERATION;
    }
    int32_t count = 0;
    result->GetRowCount(count);
    if (count == 0) {
        TAG_LOGI(AAFwkTag::ABILITY, "not found, key: %{public}s", key.c_str());
        result->Close();
        return ERR_NAME_NOT_FOUND;
    }

    result->GoToRow(INDEX0);
    int32_t code = result->GetString(INDEX0, value);
    if (code != 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "get string failed, key: %{public}s, ret: %{public}d", key.c_str(), code);
        result->Close();
        return ERR_INVALID_VALUE;
    }
    result->Close();
    TAG_LOGD(AAFwkTag::ABILITY, "Get key: %{public}s, value: %{public}s", key.c_str(), value.c_str());
    return ERR_OK;
}

std::string PageDataManager::AssembleUri(const std::string& key) const {
    std::string uriString = SETTING_DATA_URI;
    uriString.append(SETTING_DATA_KEY_URI + key);
    return uriString;
}

bool PageDataManager::ReleaseDataShareHelper(const std::shared_ptr<DataShare::DataShareHelper>& helper)
{
    if (helper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "helper is null");
        return false;
    }
    if (!helper->Release()) {
        TAG_LOGE(AAFwkTag::ABILITY, "release helper failed");
        return false;
    }
    return true;
}
} // namespace AppExecFwk
} // namespace OHOS