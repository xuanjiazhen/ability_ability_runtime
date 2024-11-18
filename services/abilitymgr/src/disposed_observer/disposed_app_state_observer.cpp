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

#include "disposed_observer/disposed_app_state_observer.h"

#include "ability_record.h"
#include "ability_util.h"
#include "disposed_observer/disposed_observer_manager.h"
#include "hilog_wrapper.h"
#include "modal_system_ui_extension.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* UIEXTENSION_MODAL_TYPE = "ability.want.params.modalType";
constexpr const char* INTERCEPT_MISSION_ID = "intercept_missionId";
}

void DisposedAppStateObserver::OnPageShow(const AppExecFwk::PageStateData &pageStateData)
{
    if (disposedRule_.componentType == AppExecFwk::ComponentType::UI_ABILITY) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Call");
        int ret = IN_PROCESS_CALL(AbilityManagerClient::GetInstance()->StartAbility(*disposedRule_.want));
        if (ret != ERR_OK) {
            DelayedSingleton<DisposedObserverManager>::GetInstance()->UnregisterObserver(pageStateData.bundleName);
            TAG_LOGE(AAFwkTag::ABILITYMGR, "call failed");
            return;
        }
    }
    if (disposedRule_.componentType == AppExecFwk::ComponentType::UI_EXTENSION) {
        auto systemUIExtension = std::make_shared<OHOS::Rosen::ModalSystemUiExtension>();
        Want want = *disposedRule_.want;
        want.SetParam(UIEXTENSION_MODAL_TYPE, 1);
        TAG_LOGD(AAFwkTag::ABILITYMGR, "modal system");
        bool ret = IN_PROCESS_CALL(systemUIExtension->CreateModalUIExtension(want));
        if (!ret) {
            DelayedSingleton<DisposedObserverManager>::GetInstance()->UnregisterObserver(pageStateData.bundleName);
            TAG_LOGE(AAFwkTag::ABILITYMGR, "call failed");
            return;
        }
    }
    DelayedSingleton<DisposedObserverManager>::GetInstance()->UnregisterObserver(pageStateData.bundleName);
}

void DisposedAppStateObserver::OnProcessDied(const AppExecFwk::ProcessData &processData)
{
    DelayedSingleton<DisposedObserverManager>::GetInstance()->UnregisterObserver(processData.bundleName);
}
} // namespace AAFwk
} // namespace OHOS