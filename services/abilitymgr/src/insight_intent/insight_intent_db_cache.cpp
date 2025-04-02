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

#include "insight_intent_db_cache.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
void InsightIntentDbCache::Start()
{
    std::vector<InstghtIntentGenericInfoVec> genericInfos;
    genericInfos.clear();
    if (InsightRdbStorageMgr::GetInstance().LoadInsightIntentInfos(genericInfos) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "LoadIntentData failed");
        return;
    }
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    for (unsigned int i = 0; i < genericInfos.size(); i++) {
        InstghtIntentGenericInfoVec info = genericInfos.at(i).GetGenericInfo();
        std::string bundleName = genericInfos.at(i).GetBundleName();
        genericInfos_[bundleName] = info;
    }
}

ErrCode InsightIntentDbCache::SaveInsightIntentGenericInfo(const std::string bundleName,
    InstghtIntentGenericInfoVec genericInfo)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    auto it = genericInfos_.find(bundleName);
    if (it != genericInfos_.end()) {
        if (it->second->Compare(genericInfo) == false) {
            TAG_LOGW(AAFwkTag::INTENT, ("need update, bundleName %{public}s", bundleName);
            it->second = genericInfo;
            return InsightRdbStorageMgr::GetInstance().ModifyStorageInsightIntentData(genericInfo);
        } else {
            TAG_LOGW(AAFwkTag::INTENT, ("already exist, bundleName %{public}s", bundleName);
            return ERR_OK;
        }
    } else {
        genericInfos_[bundleName] = genericInfo;
        return InsightRdbStorageMgr::GetInstance().SaveStorageInsightIntentData(genericInfo);
    }
}

ErrCode InsightIntentDbCache::DeleteInsightIntentGenericInfo(const std::string bundleName)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    genericInfos_.erase(bundleName);
    if (InsightRdbStorageMgr::GetInstance().DeleteStorageInsightIntentData(bundleName) == ERR_OK) {
        return ERR_OK;
    } else {
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }
}

void InsightIntentDbCache::GetAllInsightIntentGenericInfo(std::vector<InstghtIntentGenericInfoVec> &genericInfos)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    for (iter = genericInfos_.begin(); iter != genericInfos_.end(); ++iter) {
        genericInfos.push_back(iter->second);
    }
}

void InsightIntentDbCache::GetInsightIntentGenericInfo(const std::string bundleName,
    InstghtIntentGenericInfoVec &genericInfo)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    genericInfo = genericInfos_[bundleName];
}

void InsightIntentDbCache::GetInsightIntentDetailInfo(const std::string bundleName, const std::string intentName,
    InstghtIntentInfo &detailInfo)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    InstghtIntentGenericInfoVec info = genericInfos_[bundleName];
    detailInfo = info.getDetailInfo(intentName);
}
} // namespace AppExecFwk
} // namespace OHOS
