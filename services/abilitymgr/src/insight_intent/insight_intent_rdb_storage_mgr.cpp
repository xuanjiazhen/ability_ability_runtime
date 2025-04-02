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

namespace OHOS {
namespace AAFwk {
InsightRdbStorageMgr::InsightRdbStorageMgr()
{
    TAG_LOGD(AAFwkTag::INTENT, "InsightRdbStorageMgr is created");
    IntentRdbConfig intentRdbConfig;
    if (InsightIntentDbCache::GetInstance().InitIntentTable(intentRdbConfig) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "insight intent info rdb storage mgr init form rdb table fail");
    }
}

InsightRdbStorageMgr::~InsightRdbStorageMgr()
{
    TAG_LOGD(AAFwkTag::INTENT, "InsightRdbStorageMgr is deleted");
}

ErrCode InsightRdbStorageMgr::LoadInsightIntentInfos(std::vector<InstghtIntentGenericInfoVec> &genericInfos)
{
    TAG_LOGD(AAFwkTag::INTENT, "FormInfoAllRdbStorageMgr load all form infos");
    std::unordered_map<std::string, std::string> value;
    ErrCode result = InsightIntentRdbDataMgr::GetInstance().QueryAllData(value);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "get entries error");
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }
    SaveEntries(value, genericInfos);
    return ERR_OK;
}

void InsightRdbStorageMgr::SaveEntries(
    const std::unordered_map<std::string, std::string> &value, std::vector<InstghtIntentGenericInfoVec> &genericInfos)
{
    for (const auto &item : value) {
        InstghtIntentGenericInfoVec genericInfo;
        nlohmann::json jsonObject = nlohmann::json::parse(item.second, nullptr, false);
        if (jsonObject.is_discarded() || genericInfo.FromJson(jsonObject) != true) {
            TAG_LOGE(AAFwkTag::INTENT, "error key: %{private}s", item.first.c_str());
            InsightIntentRdbDataMgr::GetInstance().DeleteData(item.first);
            continue;
        }

        if (std::find(genericInfos.begin(), genericInfos.end(), innerFormInfo) == genericInfos.end()) {
            genericInfos.emplace_back(genericInfo);
        }
    }
}

ErrCode InsightRdbStorageMgr::SaveStorageInsightIntentData(const InstghtIntentGenericInfoVec &genericInfo)
{
    std::string key = genericInfo.getBundleName();
    std::string value = genericInfo.ToString();
    ErrCode result;
    {
        std::lock_guard<std::mutex> lock(rdbStorePtrMutex_);
        result = InsightIntentRdbDataMgr::GetInstance().InsertData(key, value);
    }
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "put innerFormInfo to RdbStore error");
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }
    return ERR_OK;
}

ErrCode InsightRdbStorageMgr::ModifyStorageInsightIntentData(const InstghtIntentGenericInfoVec &genericInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "formId[%{public}" PRId64 "]", innerFormInfo.GetFormId());
    std::string bundleName = genericInfo.getBundleName();
    ErrCode ret = DeleteStorageInsightIntentData(bundleName);
    if (ret == ERR_OK) {
        SaveStorageInsightIntentData(genericInfo);
    }
    return ret;
}

ErrCode InsightRdbStorageMgr::DeleteStorageInsightIntentData(const std::string bundleName)
{
    std::string key = std::string().append(bundleName);
    bool result;
    {
        std::lock_guard<std::mutex> lock(rdbStorePtrMutex_);
        result = InsightIntentRdbDataMgr::GetInstance().DeleteData(key);
    }
    if (!result) {
        TAG_LOGE(AAFwkTag::INTENT, "delete key error");
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }
    return ERR_OK;
}
} // namespace AAFwk
} // namespace OHOS
