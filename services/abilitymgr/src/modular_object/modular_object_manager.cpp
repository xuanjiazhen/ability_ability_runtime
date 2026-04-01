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
#include "ability_manager_service.h"
#include "app_utils.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ipc_skeleton.h"
#include "modular_object_rdb_storage_mgr.h"

namespace OHOS {
namespace AbilityRuntime {
ModularObjectManager::ModularObjectManager() = default;
ModularObjectManager::~ModularObjectManager() = default;

int32_t ModularObjectManager::QuerySelfModularObjectExtensionInfos(int32_t userId, const std::string &bundleName,
    int32_t appIndex, std::vector<AAFwk::ModularObjectExtensionInfo> &infos)
{
    std::string key = std::to_string(userId) + "_" + bundleName + "_" + std::to_string(appIndex);
    return DelayedSingleton<ModularObjectExtensionRdbStorageMgr>::GetInstance()->QueryData(key, infos);
}

int32_t ModularObjectManager::ConnectModularObjectExtension(const AAFwk::Want &want,
    sptr<AAFwk::IAbilityConnection> connect, sptr<IRemoteObject> callerToken, int32_t userId)
{
    if (!AppUtils::GetInstance().IsSupportModularObjectExtension()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "device not supported");
        return ERR_CAPABILITY_NOT_SUPPORT;
    }
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "ConnectModularObjectExtension bundlename: %{public}s, ability: %{public}s, userId: %{private}d",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), userId);
    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);

    auto element = want.GetElement();
    std::string bundleName = element.GetBundleName();
    std::string abilityName = element.GetAbilityName();
    int32_t appIndex = want.GetIntParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, 0);

    std::string key = std::to_string(userId) + "_" + bundleName + "_" + std::to_string(appIndex);
    std::vector<AAFwk::ModularObjectExtensionInfo> infos;
    auto ret = DelayedSingleton<ModularObjectExtensionRdbStorageMgr>::GetInstance()->QueryData(key, infos);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "QueryData failed, ret: %{public}d", ret);
        return ret;
    }

    bool found = false;
    for (const auto &info : infos) {
        if (info.bundleName == bundleName && info.abilityName == abilityName) {
            found = true;
            break;
        }
    }
    if (!found) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Extension not found: %{public}s/%{public}s",
            bundleName.c_str(), abilityName.c_str());
        return INNER_ERR;
    }

    auto service = DelayedSingleton<AAFwk::AbilityManagerService>::GetInstance();
    CHECK_POINTER_AND_RETURN(service, ERR_INVALID_VALUE);
    return service->ConnectAbilityCommon(want, connect, callerToken,
        AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT, userId);
}

int32_t ModularObjectManager::DisconnectModularObjectExtension(sptr<AAFwk::IAbilityConnection> connect)
{
    if (!AppUtils::GetInstance().IsSupportModularObjectExtension()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "device not supported");
        return ERR_CAPABILITY_NOT_SUPPORT;
    }
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisconnectModularObjectExtension");
    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    auto service = DelayedSingleton<AAFwk::AbilityManagerService>::GetInstance();
    CHECK_POINTER_AND_RETURN(service, ERR_INVALID_VALUE);
    return service->DisconnectAbility(connect);
}
}
}
