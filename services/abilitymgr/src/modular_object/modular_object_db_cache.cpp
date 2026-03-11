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

#include "modular_object_db_cache.h"

namespace OHOS {
namespace AbilityRuntime {
ModularObjectExtensionDbCache::ModularObjectExtensionDbCache()
{}
ModularObjectExtensionDbCache::~ModularObjectExtensionDbCache()
{}

void ModularObjectExtensionDbCache::InsertData(const std::string &key, const AAFwk::ModularObjectExtensionInfo &info)
{
    std::lock_guard<std::mutex> lock(modularObjectExtensionInfoMutex_);
    modularObjectExtensionInfo_[key] = info;
    DelayedSingleton<ModularObjectExtensionRdbStorageMgr>::GetInstance()->InsertData(key,info);
}

void ModularObjectExtensionDbCache::UpdateData(const std::string &key, const AAFwk::ModularObjectExtensionInfo &info)
{
    std::lock_guard<std::mutex> lock(modularObjectExtensionInfoMutex_);
    modularObjectExtensionInfo_[key] = info;
    DelayedSingleton<ModularObjectExtensionRdbStorageMgr>::GetInstance()->UpdateData(key,info);
}

void ModularObjectExtensionDbCache::DeleteData(const std::string &key)
{
    std::lock_guard<std::mutex> lock(modularObjectExtensionInfoMutex_);
    modularObjectExtensionInfo_.erase(key);
    DelayedSingleton<ModularObjectExtensionRdbStorageMgr>::GetInstance()->DeleteData(key);
}

bool ModularObjectExtensionDbCache::QueryData(const std::string &key, AAFwk::ModularObjectExtensionInfo &info)
{
    std::lock_guard<std::mutex> lock(modularObjectExtensionInfoMutex_);
    auto iter = modularObjectExtensionInfo_.find(key);
    if (iter != modularObjectExtensionInfo_.end()) {
        info = iter->second;
        return true;
    }
    if (!DelayedSingleton<ModularObjectExtensionRdbStorageMgr>::GetInstance()->QueryData(key, info)) {
        return false;
    }
    return true;
}

bool ModularObjectExtensionDbCache::QueryAllData(std::map<std::string, AAFwk::ModularObjectExtensionInfo> &datas)
{
    std::lock_guard<std::mutex> lock(modularObjectExtensionInfoMutex_);
    for (const auto &item : modularObjectExtensionInfo_) {
        datas.emplace(item.first, item.second);
    }
    if (datas.empty()) {
        if (!DelayedSingleton<ModularObjectExtensionRdbStorageMgr>::GetInstance()->QueryAllData(datas)) {
            return false;
        }
    }
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
