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
    std::vector<ExtraInsightIntentTotalInfo> totalInfos;
    totalInfos.clear();
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadInsightIntentInfos(totalInfos) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "LoadIntentData failed");
        return;
    }
    std::lock_guard<std::mutex> lock(totalInfosMutex_);
    for (unsigned int i = 0; i < totalInfos.size(); i++) {
        ExtraInsightIntentTotalInfo info = totalInfos.at(i);
        std::string bundleName = totalInfos.at(i).bundleName;
        std::string moduleName = totalInfos.at(i).moduleName;
        std::string key = std::string().append(bundleName).append("/").append(moduleName);
        insightIntentTotalInfos_[key].push_back(info);
    }
}

int32_t InsightIntentDbCache::SaveInsightIntentTotalInfo(const std::string bundleName, const std::string moduleName,
    std::vector<ExtraInsightIntentTotalInfo> totalInfos)
{
    std::lock_guard<std::mutex> lock(totalInfosMutex_);
    std::string key = std::string().append(bundleName).append("/").append(moduleName);
    auto it = insightIntentTotalInfos_.find(key);
    if (it != insightIntentTotalInfos_.end()) {
        TAG_LOGW(AAFwkTag::INTENT, "need update, bundleName %{public}s", bundleName.c_str());
        it->second = totalInfos;
        return DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
            ModifyStorageInsightIntentData(bundleName, moduleName, totalInfos);
    } else {
        insightIntentTotalInfos_[key] = totalInfos;
        return DelayedSingleton<InsightRdbStorageMgr>::GetInstance()
            ->SaveStorageInsightIntentData(bundleName, moduleName, totalInfos);
    }
}

int32_t InsightIntentDbCache::DeleteInsightIntentTotalInfo(const std::string bundleName, const std::string moduleName)
{
    std::lock_guard<std::mutex> lock(totalInfosMutex_);
    std::string key = std::string().append(bundleName).append("/").append(moduleName);
    insightIntentTotalInfos_.erase(key);
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        DeleteStorageInsightIntentData(bundleName, moduleName) == ERR_OK) {
        return ERR_OK;
    } else {
        return ERR_INVALID_VALUE;
    }
}

void InsightIntentDbCache::GetAllInsightIntentTotalInfo(std::vector<ExtraInsightIntentTotalInfo> &totalInfos)
{
    std::lock_guard<std::mutex> lock(totalInfosMutex_);
    for (auto iter = insightIntentTotalInfos_.begin(); iter != insightIntentTotalInfos_.end(); ++iter) {
        totalInfos.insert(totalInfos.end(), iter->second.begin(), iter->second.end());
    }
}

void InsightIntentDbCache::GetInsightIntentTotalInfoByName(const std::string bundleName,
    const std::string moduleName, std::vector<ExtraInsightIntentTotalInfo> &totalInfos)
{
    std::lock_guard<std::mutex> lock(totalInfosMutex_);
    std::string key = std::string().append(bundleName).append("/").append(moduleName);
    totalInfos = insightIntentTotalInfos_[key];
}

void InsightIntentDbCache::GetInsightIntentTotalInfo(const std::string bundleName, const std::string moduleName,
    const std::string intentName, ExtraInsightIntentTotalInfo &totalInfo)
{
    std::lock_guard<std::mutex> lock(totalInfosMutex_);
    std::string key = std::string().append(bundleName).append("/").append(moduleName);
    for (auto info : insightIntentTotalInfos_[key]) {
        if (strcmp(info.intentName.c_str(), intentName.c_str()) == 0) {
            totalInfo = info;
            return;
        }
    }
}
} // namespace AAFwk
} // namespace OHOS
