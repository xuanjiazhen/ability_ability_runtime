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

#ifndef OHOS_MODULAR_OBJECT_RDB_STORAGE_MGR_H
#define OHOS_MODULAR_OBJECT_RDB_STORAGE_MGR_H

#include <singleton.h>
#include <string>

#include "modular_object_extension_info.h"
#include "modular_object_rdb_data_mgr.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
class ModularObjectExtensionRdbStorageMgr : public std::enable_shared_from_this<ModularObjectExtensionRdbStorageMgr> {
    DECLARE_DELAYED_SINGLETON(ModularObjectExtensionRdbStorageMgr)
public:
    bool InsertData(const std::string &key, const AAFwk::ModularObjectExtensionInfo &info);
    bool UpdateData(const std::string &key, const AAFwk::ModularObjectExtensionInfo &info);
    bool DeleteData(const std::string &key);
    bool QueryData(const std::string &key, AAFwk::ModularObjectExtensionInfo &info);
    bool QueryAllData(std::map<std::string, AAFwk::ModularObjectExtensionInfo> &datas);
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // OHOS_MODULAR_OBJECT_RDB_STORAGE_MGR_H