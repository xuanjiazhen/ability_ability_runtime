/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")_;
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

#include "insight_intent_rdb_storage_mgr.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AAFwk {
InsightRdbStorageMgr::InsightRdbStorageMgr()
{
    TAG_LOGD(AAFwkTag::INTENT, "InsightRdbStorageMgr is created");
    IntentRdbConfig intentRdbConfig;
    if (!DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->InitIntentTable(intentRdbConfig)) {
        TAG_LOGE(AAFwkTag::INTENT, "insight intent info rdb storage mgr init rdb table fail");
    }
}

InsightRdbStorageMgr::~InsightRdbStorageMgr()
{
    TAG_LOGD(AAFwkTag::INTENT, "InsightRdbStorageMgr is deleted");
}

int32_t InsightRdbStorageMgr::LoadInsightIntentInfos(std::vector<ExtraInsightIntentInfo> &totalInfos)
{
    TAG_LOGD(AAFwkTag::INTENT, "InsightRdbStorageMgr load all intent total infos");
    std::unordered_map<std::string, std::string> value;
    int32_t result = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->QueryAllData(value);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "get entries error");
        return ERR_INVALID_VALUE;
    }
    Transform(value, totalInfos);
    return ERR_OK;
}

int32_t InsightRdbStorageMgr::LoadInsightIntentInfoByName(const std::string bundleName,
    std::vector<ExtraInsightIntentInfo> &totalInfos)
{
    TAG_LOGD(AAFwkTag::INTENT, "InsightRdbStorageMgr load intent total infos by bundleName");
    std::unordered_map<std::string, std::string> value;
    int32_t result = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->QueryData(bundleName, value);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "get entries error");
        return ERR_INVALID_VALUE;
    }
    Transform(value, totalInfos);
    return ERR_OK;
}

int32_t InsightRdbStorageMgr::LoadInsightIntentInfo(const std::string bundleName, const std::string intentName,
    std::vector<ExtraInsightIntentInfo> &totalInfos)
{
    TAG_LOGD(AAFwkTag::INTENT, "InsightRdbStorageMgr load intent total info");
    std::unordered_map<std::string, std::string> value;
    std::string key = std::string().append(bundleName).append("/").append(intentName);
    int32_t result = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->QueryData(key, value);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "get entries error");
        return ERR_INVALID_VALUE;
    }
    Transform(value, totalInfos);
    return ERR_OK;
}

void InsightRdbStorageMgr::Transform(std::unordered_map<std::string, std::string> value,
    std::vector<ExtraInsightIntentInfo> &totalInfos)
{
    for (const auto &item : value) {
        ExtraInsightIntentProfileInfoVec profileInfos;
        if (jsonObject.is_discarded() || !ExtraInsightIntentProfile::TransformTo(item.second, profileInfos)) {
            TAG_LOGE(AAFwkTag::INTENT, "error key: %{private}s", item.first.c_str());
            DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->DeleteData(item.first);
        }
        for (const auto &profileInfo : profileInfos.insightIntents) {
            ExtraInsightIntentInfo totalInfo;
            if (!ExtraInsightIntentProfile::ProfileInfoFormat(profileInfo, totalInfo)) {
                TAG_LOGE(AAFwkTag::INTENT, "ProfileInfoFormat error, key: %{private}s", item.first.c_str());
            }
            totalInfos.emplace_back(totalInfo);
        }
    }
}

int32_t InsightRdbStorageMgr::SaveStorageInsightIntentData(const std::string bundleName, const std::string moduleName,
    ExtraInsightIntentProfileInfoVec &profileInfos)
{
    int32_t result;
    {
        std::lock_guard<std::mutex> lock(rdbStorePtrMutex_);
        for (auto profileInfo : profileInfos.insightIntents) {
            std::string key = std::string().append(bundleName).append("/").append(profileInfo.intentName).append("/")
                .append("/").append(moduleName);
            nlohmann::json jsonObject;
            if (ExtraInsightIntentProfile::ToJson(profileInfo, jsonObject)) {
                TAG_LOGE(AAFwkTag::INTENT, "Transform error, key: %{private}s", key.c_str());
                return ERR_INVALID_VALUE;
            }
            result = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->InsertData(key, jsonObject.dump());
            if (result != ERR_OK) {
                TAG_LOGE(AAFwkTag::INTENT, "InsertData error, key: %{private}s", key.c_str());
            }
        }
    }
    return ERR_OK;
}

int32_t InsightRdbStorageMgr::DeleteStorageInsightIntentData(const std::string bundleName)
{
    bool result;
    {
        std::lock_guard<std::mutex> lock(rdbStorePtrMutex_);
        result = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->DeleteDataBeginWithKey(bundleName);
    }
    if (!result) {
        TAG_LOGE(AAFwkTag::INTENT, "delete key error");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}
} // namespace AAFwk
} // namespace OHOS
