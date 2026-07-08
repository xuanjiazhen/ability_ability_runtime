/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "interceptor/block_all_app_start_interceptor.h"

#include "ability_manager_service.h"
#include "hilog_tag_wrapper.h"
#include "ui_ability_lifecycle_manager.h"

namespace OHOS {
namespace AAFwk {
ErrCode BlockAllAppStartInterceptor::DoProcess(AbilityInterceptorParam param)
{
    if (param.shouldBlockAllAppStartFunc_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "shouldBlockAllAppStartFunc_ is nullptr");
        return ERR_INVALID_VALUE;
    }

    if (param.shouldBlockAllAppStartFunc_() && !IsUIAbilityAlreadyRunning(param)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "blocking app start due to low memory");
        return ERR_ALL_APP_START_BLOCKED;
    }
    return ERR_OK;
}

bool BlockAllAppStartInterceptor::IsUIAbilityAlreadyRunning(const AbilityInterceptorParam &param)
{
    if (param.want.GetElement().GetAbilityName().empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Implicit start detected, skip check");
        return false;
    }

    if (param.abilityInfo != nullptr && param.abilityInfo->type != AppExecFwk::AbilityType::PAGE) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Target ability is not PAGE type, skip check");
        return false;
    }

    auto ams = AbilityManagerService::GetPubInstance();
    if (ams == nullptr) {
        return false;
    }

    auto uiAbilityManager = ams->GetUIAbilityManagerByUserId(param.userId);
    if (uiAbilityManager == nullptr) {
        return false;
    }

    auto abilityRecords = uiAbilityManager->GetAbilityRecordsByName(
        param.want.GetElement(), param.appIndex);
    if (!abilityRecords.empty()) {
        auto& element = param.want.GetElement();
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Target ability %{public}s/%{public}s is already running, skip block check",
            element.GetBundleName().c_str(), element.GetAbilityName().c_str());
        return true;
    }

    return false;
}
} // namespace AAFwk
} // namespace OHOS