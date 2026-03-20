/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_MODULAR_OBJECT_MANAGER_H
#define OHOS_MODULAR_OBJECT_MANAGER_H

#include "modular_object_rdb_storage_mgr.h"
#include "modular_object_extension_info.h"

namespace OHOS {
namespace AbilityRuntime {
class ModularObjectManager : public std::enable_shared_from_this<ModularObjectManager> {
    DECLARE_DELAYED_SINGLETON(ModularObjectManager)
public:
    int32_t SetModularObjectDisableState(const std::string &bundleName, const std::string &moduleName,
        const std::string &abilityName, int appIndex, bool isDisable);
    int32_t QueryAllModularObjectExtensionInfo(std::vector<AAFwk::ModularObjectExtensionInfo> &infos);
    int32_t SetModularObjectGlobalDisabledState(bool isDisabled);
    int32_t GetModularObjectGlobalDisabledState(bool &isDisabled);
};
}
}

#endif // OHOS_MODULAR_OBJECT_MANAGER_H