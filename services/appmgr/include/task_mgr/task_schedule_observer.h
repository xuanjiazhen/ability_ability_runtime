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

#ifndef OHOS_APP_EXECFWK_TASK_MGR_TASK_SCHEDULE_OBSERVER_H
#define OHOS_APP_EXECFWK_TASK_MGR_TASK_SCHEDULE_OBSERVER_H

#include "ability_state_data.h"
#include "application_state_observer_stub.h"
#include "app_mgr_constants.h"
#include "hilog_tag_wrapper.h"
#include "task_mgr/task_manager_service.h"

namespace OHOS {
namespace AppExecFwk {

/**
 * @brief Internal in-process observer that funnels ability foreground/background transitions
 *        into the task priority judge. Registered with AppStateObserverManager when the task
 *        manager assembles; the callbacks arrive on the observer handler thread, so the
 *        business lock is taken inside TaskManagerService, not here (design ADR-6).
 */
class TaskScheduleAbilityObserver : public ApplicationStateObserverStub {
public:
    TaskScheduleAbilityObserver() = default;
    ~TaskScheduleAbilityObserver() override = default;

    void OnAbilityStateChanged(const AbilityStateData &abilityStateData) override
    {
        bool foreground =
            (abilityStateData.abilityState == static_cast<int32_t>(AbilityState::ABILITY_STATE_FOREGROUND));
        bool background =
            (abilityStateData.abilityState == static_cast<int32_t>(AbilityState::ABILITY_STATE_BACKGROUND));
        if (!foreground && !background) {
            return;
        }
        TAG_LOGD(AAFwkTag::APPMGR, "ability state changed, uid: %{public}d fg: %{public}d",
            abilityStateData.uid, foreground);
        TaskManagerService::GetInstance().OnAbilityForegroundChanged(abilityStateData.uid, foreground);
    }
};

}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_APP_EXECFWK_TASK_MGR_TASK_SCHEDULE_OBSERVER_H
