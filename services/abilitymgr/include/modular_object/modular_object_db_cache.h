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

#ifndef OHOS_MODULAR_OBJECT_DB_CACHE_H
#define OHOS_MODULAR_OBJECT_DB_CACHE_H

#include <mutex>
#include <set>
#include <singleton.h>
#include <vector>
#include "modular_object_rdb_storage_mgr.h"
#include "modular_object_extension_info.h"

namespace OHOS {
namespace AbilityRuntime {
class ModularObjectExtensionDbCache : public std::enable_shared_from_this<ModularObjectExtensionDbCache> {
    DECLARE_DELAYED_SINGLETON(ModularObjectExtensionDbCache)
public:
    void InsertData(const std::string &key, const AAFwk::ModularObjectExtensionInfo &info);
    void UpdateData(const std::string &key, const AAFwk::ModularObjectExtensionInfo &info);
    void DeleteData(const std::string &key);
    bool QueryData(const std::string &key, AAFwk::ModularObjectExtensionInfo &info);
    bool QueryAllData(std::map<std::string, AAFwk::ModularObjectExtensionInfo> &datas);
private:
    mutable std::mutex modularObjectExtensionInfoMutex_;
    std::map<std::string, AAFwk::ModularObjectExtensionInfo> modularObjectExtensionInfo_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_MODULAR_OBJECT_DB_CACHE_H
