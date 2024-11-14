/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "disposed_observer/disposed_first_frame_observer.h"

#include "ability_manager_service.h"
#include "ability_record.h"
#include "ability_util.h"
#include "disposed_observer/disposed_observer_manager.h"
#include "hilog_tag_wrapper.h"
#include "modal_system_ui_extension.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* UIEXTENSION_MODAL_TYPE = "ability.want.params.modalType";
constexpr const char* INTERCEPT_MISSION_ID = "intercept_missionId";
}

void DisposedFirstFrameObserver::OnAbilityFirstFrameState(
    const AppExecFwk::AbilityFirstFrameStateData &abilityFirstFrameStateData)
{
    if (disposedRule_.componentType == AppExecFwk::ComponentType::UI_ABILITY) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "start ui ability");
        int ret = IN_PROCESS_CALL(AbilityManagerClient::GetInstance()->StartAbility(*disposedRule_.want));
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "call failed");
            DelayedSingleton<DisposedObserverManager>::GetInstance()->
                UnregisterObserver(abilityFirstFrameStateData.bundleName);
            return;
        }
    }

    if (type_ == DisposedModalType::MODAL_SYSTEM) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "modal system");
        auto systemUIExtension = std::make_shared<OHOS::Rosen::ModalSystemUiExtension>();
        Want want = *disposedRule_.want;
        want.SetParam(UIEXTENSION_MODAL_TYPE, 1);
        bool ret = IN_PROCESS_CALL(systemUIExtension->CreateModalUIExtension(want));
        if (!ret) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "call failed");
        }
        DelayedSingleton<DisposedObserverManager>::GetInstance()->
            UnregisterObserver(abilityFirstFrameStateData.bundleName);
        return;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "modal app");
    auto abilityRecord = DelayedSingleton<AbilityManagerService>::GetInstance()->GetAbilityRecordById(
        abilityFirstFrameStateData.sessionId);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord nullptr");
        DelayedSingleton<DisposedObserverManager>::GetInstance()->
            UnregisterObserver(abilityFirstFrameStateData.bundleName);
        return;
    }
    abilityRecord->SetDisposedBundleName(disposedRule_.want->GetElement().GetBundleName());
    Want want = *disposedRule_.want;
    auto sessionInfo = abilityRecord->GetSessionInfo();
    if (sessionInfo != nullptr) {
        want.SetParam(INTERCEPT_MISSION_ID, sessionInfo->persistentId);
    } else {
        want.SetParam(INTERCEPT_MISSION_ID, abilityRecord->GetMissionId());
    }
    int ret = abilityRecord->CreateModalUIExtension(want);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "call failed");
    }
    DelayedSingleton<DisposedObserverManager>::GetInstance()->
        UnregisterObserver(abilityFirstFrameStateData.bundleName);
}

void DisposedFirstFrameObserver::SetModalType(DisposedModalType type)
{
    type_ = type;
}
} // namespace AAFwk
} // namespace OHOS
