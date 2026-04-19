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

#include "modular_object_connection_handler_impl.h"
#include "ability_manager_service.h"
#include "hilog_tag_wrapper.h"
#include "scene_board_judgement.h"

namespace OHOS {
namespace AbilityRuntime {

ModularObjectConnectionHandlerImpl::ModularObjectConnectionHandlerImpl(
    std::weak_ptr<AAFwk::AbilityManagerService> service)
    : serviceWeak_(service)
{}

int32_t ModularObjectConnectionHandlerImpl::ConnectAbilityCommon(const AAFwk::Want &want,
    sptr<AAFwk::IAbilityConnection> connect, sptr<IRemoteObject> callerToken,
    AppExecFwk::ExtensionAbilityType extensionType, int32_t userId)
{
    auto service = serviceWeak_.lock();
    if (!service) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Service is null");
        return ERR_NO_INIT;
    }
    return service->ConnectAbilityCommon(want, connect, callerToken, extensionType, userId);
}

int32_t ModularObjectConnectionHandlerImpl::DisconnectAbility(sptr<AAFwk::IAbilityConnection> connect)
{
    auto service = serviceWeak_.lock();
    if (!service) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Service is null");
        return ERR_NO_INIT;
    }
    return service->DisconnectAbility(connect);
}

bool ModularObjectConnectionHandlerImpl::HasRunningUIAbilityOrExtension(int32_t targetUid, int32_t userId)
{
    auto service = serviceWeak_.lock();
    if (!service) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Service is null");
        return false;
    }

    // Check UIAbility
    std::vector<std::string> abilityList;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = service->GetUIAbilityManagerByUserId(userId);
        if (uiAbilityManager) {
            uiAbilityManager->GetActiveAbilityList(targetUid, abilityList);
        }
    } else {
        auto missionListManager = service->GetMissionListManagerByUserId(userId);
        if (missionListManager) {
            missionListManager->GetActiveAbilityList(targetUid, abilityList);
        }
    }
    if (!abilityList.empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Found running UIAbility for uid: %{public}d", targetUid);
        return true;
    }

    // Check UIExtension
    auto uiExtManager = service->GetUIExtensionAbilityManagerByUserId(userId);
    if (uiExtManager) {
        std::vector<std::string> extensionList;
        uiExtManager->GetActiveUIExtensionListByUid(targetUid, extensionList);
        if (!extensionList.empty()) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "Found running UIExtension for uid: %{public}d", targetUid);
            return true;
        }
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "No UIAbility or UIExt for uid: %{public}d", targetUid);
    return false;
}

} // namespace AbilityRuntime
} // namespace OHOS
