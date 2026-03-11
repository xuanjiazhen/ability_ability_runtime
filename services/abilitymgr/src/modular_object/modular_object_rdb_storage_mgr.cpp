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

#include <sstream>

#include "modular_object_rdb_data_mgr.h"
#include "modular_object_rdb_storage_mgr.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AbilityRuntime {
const int32_t BUNDLE_NAME = 1;
const int32_t VERSION = 4;
const int32_t INTENT_KEY_LENGTH = 5;
ModularObjectExtensionRdbStorageMgr::ModularObjectExtensionRdbStorageMgr()
{
}

ModularObjectExtensionRdbStorageMgr::~ModularObjectExtensionRdbStorageMgr()
{
    TAG_LOGD(AAFwkTag::INTENT, "ModularObjectExtensionRdbStorageMgr is deleted");
}

bool ModularObjectExtensionRdbStorageMgr::InsertData(
    const std::string& key, const AAFwk::ModularObjectExtensionInfo &info)
{
    std::string jsonString = info.ToJsonString();
    return  DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance()->InsertData(key, jsonString);
}
bool ModularObjectExtensionRdbStorageMgr::UpdateData(
    const std::string& key, const AAFwk::ModularObjectExtensionInfo &info)
{
    std::string jsonString = info.ToJsonString();
    return DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance()->UpdateData(key, jsonString);
}

bool ModularObjectExtensionRdbStorageMgr::DeleteData(const std::string& key)
{
    return DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance()->DeleteData(key);
}

bool ModularObjectExtensionRdbStorageMgr::QueryData(const std::string& key, AAFwk::ModularObjectExtensionInfo &info)
{
    std::string jsonString;
    if (!DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance()->QueryData(key, jsonString)) {
        return false;
    }
    return info.FromJsonString(jsonString);
}

bool ModularObjectExtensionRdbStorageMgr::QueryAllData(std::vector<AAFwk::ModularObjectExtensionInfo> &datas)
{
    std::map<std::string, std::string> jsonDatas;
    if (!DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance()->QueryAllData(jsonDatas)) {
        return false;
    }
    for (const auto& [key, jsonString] : jsonDatas) {
        AAFwk::ModularObjectExtensionInfo info;
        if (info.FromJsonString(jsonString)) {
            datas.push_back(info);
        }
    }
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
