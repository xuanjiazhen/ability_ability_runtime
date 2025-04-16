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

namespace OHOS {
namespace AAFwk {
void InsightIntentDbCache::InitInsightIntentCache()
{
    std::vector<ExtraInsightIntentInfo> totalInfos;
    totalInfos.clear();
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadInsightIntentInfos(totalInfos) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "LoadIntentData failed");
        return;
    }
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    for (unsigned int i = 0; i < totalInfos.size(); i++) {
        ExtraInsightIntentInfo info = totalInfos.at(i);
        std::string bundleName = totalInfos.at(i).genericInfo.bundleName;
        intentGenericInfos_[bundleName].push_back(info.genericInfo);
    }
}

int32_t InsightIntentDbCache::SaveInsightIntentTotalInfo(const std::string bundleName, const std::string moduleName,
    ExtraInsightIntentProfileInfoVec profileInfos)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    std::vector<ExtraInsightIntentGenericInfo> genericInfos;
    for (auto profileInfo : profileInfos.insightIntents) {
        ExtraInsightIntentInfo info;
        ExtraInsightIntentProfile::ProfileInfoFormat(profileInfo, info);
        genericInfos.emplace_back(info.genericInfo);
    }
    auto it = intentGenericInfos_.find(bundleName);
    if (it != intentGenericInfos_.end()) {
        TAG_LOGW(AAFwkTag::INTENT, "need update, bundleName %{public}s", bundleName.c_str());
        for (auto iter = intentGenericInfos_[bundleName].begin(); iter != intentGenericInfos_[bundleName].end();) {
            if (strcmp(iter->moduleName, moduleName.c_str()) == 0) {
                iter = intentGenericInfos_[bundleName].erase(iter);
            } else {
                iter++;
            }
        }
        it->second.insert(it->second.end(), genericInfos.begin(), genericInfos.end());
    } else {
        intentGenericInfos_[bundleName] = genericInfos;
    }
    return DelayedSingleton<InsightRdbStorageMgr>::GetInstance()
            ->SaveStorageInsightIntentData(bundleName, moduleName, profileInfos);
}

int32_t InsightIntentDbCache::DeleteInsightIntentTotalInfo(const std::string bundleName)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    intentGenericInfos_.erase(bundleName);
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentData(bundleName) == ERR_OK) {
        return ERR_OK;
    } else {
        return ERR_INVALID_VALUE;
    }
}

void InsightIntentDbCache::GetAllInsightIntentGenericInfo(std::vector<ExtraInsightIntentGenericInfo> &genericInfos)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    for (auto iter = intentGenericInfos_.begin(); iter != intentGenericInfos_.end(); ++iter) {
        genericInfos.insert(genericInfos.end(), iter->second.begin(), iter->second.end());
    }
}

void InsightIntentDbCache::GetInsightIntentGenericInfoByName(const std::string bundleName,
    std::vector<ExtraInsightIntentGenericInfo> &genericInfos)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    genericInfos = intentGenericInfos_[bundleName];
}

void InsightIntentDbCache::GetInsightIntentGenericInfo(const std::string bundleName,
    const std::string intentName, std::vector<ExtraInsightIntentGenericInfo> &genericInfos)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    for (auto info : intentGenericInfos_[bundleName]) {
        if (strcmp(info.intentName.c_str(), intentName.c_str()) == 0) {
            genericInfos.push_back(info);
        }
    }
}

void InsightIntentDbCache::GetAllInsightIntentInfo(std::vector<ExtraInsightIntentInfo> &infos)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadInsightIntentInfos(infos) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "LoadIntentData failed");
        return;
    }
}

void InsightIntentDbCache::GetInsightIntentInfoByName(const std::string bundleName,
    std::vector<ExtraInsightIntentInfo> &infos)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadInsightIntentInfoByName(bundleName, infos) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "LoadInsightIntentInfoByName failed");
        return;
    }
}

void InsightIntentDbCache::GetInsightIntentInfo(const std::string bundleName, const std::string intentName,
        std::vector<ExtraInsightIntentInfo> &infos)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadInsightIntentInfo(bundleName, intentName, infos) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "LoadInsightIntentInfoByName failed");
        return;
    }
}
} // namespace AAFwk
} // namespace OHOS
