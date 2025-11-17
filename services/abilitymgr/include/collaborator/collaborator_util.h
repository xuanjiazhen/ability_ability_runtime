/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_COLLABORATOR_UTIL_H
#define OHOS_ABILITY_RUNTIME_COLLABORATOR_UTIL_H

#include <string>
#include <unordered_set>

#include "ability_config.h"

namespace OHOS {
namespace AAFwk {
namespace CollaboratorUtil {
constexpr const char* SYSTEM_BASIC = "system_basic";
static void UpdateCallerIfNeed(const sptr<IRemoteObject> &callerToken,
    const sptr<IAbilityManagerCollaborator> &collaborator, const Want &want)
{
    if (!StartAbilityUtils::IsCallFromAncoShellOrBroker(callerToken)) {
        return;
    }
    auto collaborator = GetCollaborator(CollaboratorType::RESERVE_TYPE);
    if (collaborator == nullptr) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "collaborator is nullptr");
        return;
    }
    int32_t ret = collaborator->UpdateCallerIfNeed(abilityRequest.want);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "UpdateCallerIfNeed end,ret:%{public}d", ret);
}

static void RemoveCallerIfNeed(const sptr<IRemoteObject> &callerToken, const std::string &callerBundleName,
    const sptr<IAbilityManagerCollaborator> &collaborator, const Want &want)
{
    if (AppUtils.InAppTransferList(callerBundleName)) {
        auto collaborator = GetCollaborator(CollaboratorType::RESERVE_TYPE);
        if (collaborator == nullptr) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "collaborator is nullptr");
            return;
        }
        int32_t ret = collaborator->RemoveCallerIfNeed(abilityRequest.want);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "RemoveCallerIfNeed end,ret:%{public}d", ret);
    }
}
}  // namespace AbilityUtil
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_COLLABORATOR_UTIL_H