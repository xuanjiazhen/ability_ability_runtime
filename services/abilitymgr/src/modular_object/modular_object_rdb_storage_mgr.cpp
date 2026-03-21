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
namespace {
constexpr const char* MODULAR_OBJECT_EXTENSION_INFO = "ModularObjectExtensionInfo";
const std::string IS_SYSTEM_DISABLED = "isSystemDisabled";
const std::string BOOL_TRUE = "true";
const std::string BOOL_FALSE = "false";
}
ModularObjectExtensionRdbStorageMgr::ModularObjectExtensionRdbStorageMgr()
{
}

ModularObjectExtensionRdbStorageMgr::~ModularObjectExtensionRdbStorageMgr()
{
    TAG_LOGD(AAFwkTag::INTENT, "ModularObjectExtensionRdbStorageMgr is deleted");
}

int32_t ModularObjectExtensionRdbStorageMgr::InsertData(
    const std::string& key, const std::vector<AAFwk::ModularObjectExtensionInfo> &info)
{
    std::string jsonString = ToJsonString(info);
    return DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance()->InsertData(key, jsonString);
}
int32_t ModularObjectExtensionRdbStorageMgr::UpdateData(
    const std::string& key, const std::vector<AAFwk::ModularObjectExtensionInfo> &info)
{
    std::string jsonString = ToJsonString(info);
    return DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance()->UpdateData(key, jsonString);
}

int32_t ModularObjectExtensionRdbStorageMgr::DeleteData(const std::string& key)
{
    return DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance()->DeleteData(key);
}

int32_t ModularObjectExtensionRdbStorageMgr::QueryData(const std::string& key,
    std::vector<AAFwk::ModularObjectExtensionInfo> &infos)
{
    std::string jsonString;
    auto ret = DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance()->QueryData(key, jsonString);
    if (ret != NativeRdb::E_OK) {
        return ret;
    }
    FromJsonString(jsonString, infos);
    return NativeRdb::E_OK;
}

int32_t ModularObjectExtensionRdbStorageMgr::QueryAllData(std::vector<AAFwk::ModularObjectExtensionInfo> &datas)
{
    std::map<std::string, std::string> jsonDatas;
    auto ret = DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance()->QueryAllData(jsonDatas);
    if (ret != NativeRdb::E_OK) {
        return ret;
    }
    for (const auto& [key, jsonString] : jsonDatas) {
        FromJsonString(jsonString, datas);
    }
    return NativeRdb::E_OK;
}

std::string ModularObjectExtensionRdbStorageMgr::ToJsonString(
    const std::vector<AAFwk::ModularObjectExtensionInfo> &infos)
{
    nlohmann::json jsonNodes = nlohmann::json::array();
    auto size = infos.size();
    for (size_t i = 0; i < size; i++) {
        jsonNodes.emplace_back(infos[i].ToJsonString());
    }
    nlohmann::json jsonObject {
        {MODULAR_OBJECT_EXTENSION_INFO, jsonNodes},
    };
    return jsonObject.dump();
}

void ModularObjectExtensionRdbStorageMgr::FromJsonString(const std::string& jsonStr,
    std::vector<AAFwk::ModularObjectExtensionInfo> &infos)
{
    nlohmann::json jsonObject = nlohmann::json::parse(jsonStr, nullptr, false);
    if (jsonObject.is_discarded()) {
        return;
    }
    if (jsonObject.contains(MODULAR_OBJECT_EXTENSION_INFO) && jsonObject[MODULAR_OBJECT_EXTENSION_INFO].is_array()) {
        auto size = jsonObject[MODULAR_OBJECT_EXTENSION_INFO].size();
        for (size_t i = 0; i < size; i++) {
            if (jsonObject[MODULAR_OBJECT_EXTENSION_INFO][i].is_string()) {
                AAFwk::ModularObjectExtensionInfo info;
                info.FromJsonString(jsonObject[MODULAR_OBJECT_EXTENSION_INFO][i]);
                infos.emplace_back(info);
            }
        }
    }
}

int32_t ModularObjectExtensionRdbStorageMgr::SetGlobalDisabledState(bool isDisabled)
{
    std::string value = isDisabled ? BOOL_TRUE : BOOL_FALSE;
    return DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance()->InsertData(IS_SYSTEM_DISABLED, value);
}

int32_t ModularObjectExtensionRdbStorageMgr::GetGlobalDisabledState(bool &isDisabled)
{
    std::string value;
    int32_t ret = DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance()->QueryData(
        IS_SYSTEM_DISABLED, value);
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::EXT, "Query global disabled state failed, ret:%{public}d", ret);
        return ret;
    }

    if (value == BOOL_TRUE) {
        isDisabled = true;
    } else {
        isDisabled = false;
    }

    return NativeRdb::E_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS
