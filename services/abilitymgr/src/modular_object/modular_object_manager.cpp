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

#include "modular_object_manager.h"
#include "ability_manager_errors.h"
#include "os_account_manager_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
ModularObjectManager::ModularObjectManager() = default;
ModularObjectManager::~ModularObjectManager() = default;

int32_t ModularObjectManager::QueryAllModularObjectExtensionInfo(std::vector<AAFwk::ModularObjectExtensionInfo> &infos)
{
    return DelayedSingleton<ModularObjectExtensionRdbStorageMgr>::GetInstance()->QueryAllData(infos);
}

int32_t ModularObjectManager::SetModularObjectDisabledState(const std::string &bundleName,
    const std::string &moduleName, const std::string &abilityName, int appIndex, bool isDisable)
{
    std::vector<AAFwk::ModularObjectExtensionInfo> infos;
    int32_t userId = OHOS::AppExecFwk::OsAccountManagerWrapper::GetCurrentActiveAccountId();
    std::string key = std::to_string(userId) + bundleName + std::to_string(appIndex); 
    if (DelayedSingleton<ModularObjectExtensionRdbStorageMgr>::GetInstance()->QueryData(key, infos) !=
        NativeRdb::E_OK) {
        return ERR_INVALID_VALUE;
    }
    for (auto &info : infos) {
        if (info.moduleName == moduleName && info.abilityName == abilityName) {
            info.isSystemDisabled = isDisable;
            return DelayedSingleton<ModularObjectExtensionRdbStorageMgr>::GetInstance()->UpdateData(key, infos);
        }
    }
    return ERR_INVALID_VALUE;
}

int32_t ModularObjectManager::SetModularObjectGlobalDisabledState(bool isDisabled)
{
    return DelayedSingleton<ModularObjectExtensionRdbStorageMgr>::GetInstance()->SetGlobalDisabledState(isDisabled);
}

int32_t ModularObjectManager::GetModularObjectGlobalDisabledState(bool &isDisabled)
{
    return DelayedSingleton<ModularObjectExtensionRdbStorageMgr>::GetInstance()->GetGlobalDisabledState(isDisabled);
}
}
}