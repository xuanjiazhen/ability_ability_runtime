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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_PERMISSION_CHECKER_H
#define OHOS_ABILITY_RUNTIME_ABILITY_PERMISSION_CHECKER_H

#include <cstdint>
#include <memory>

#include "bundle_constants.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
// Permission checks against target-declared restrictions; caller-side validation is never affected.
class AbilityPermissionChecker {
public:
    AbilityPermissionChecker() = default;
    virtual ~AbilityPermissionChecker() = default;

    virtual const char *GetSceneName() const
    {
        return "default";
    }

    // Evaluate the permission the target declared for ordinary starts; PERMISSION_GRANTED when waived.
    template <typename RealCheck>
    int32_t CheckTargetStaticPermission(const RealCheck &realCheck) const
    {
        if (NeedCheckTargetStaticPermission()) {
            return realCheck();
        }
        TAG_LOGD(AAFwkTag::ABILITYMGR, "target static permission waived, scene:%{public}s", GetSceneName());
        return AppExecFwk::Constants::PERMISSION_GRANTED;
    }

    // Fold declaredVisible into the value PermissionVerification should use; true when waived.
    bool ResolveTargetVisible(bool declaredVisible) const
    {
        if (NeedCheckTargetVisible()) {
            return declaredVisible;
        }
        TAG_LOGD(AAFwkTag::ABILITYMGR, "target visible waived, scene:%{public}s", GetSceneName());
        return true;
    }

    // Whether the target-declared permission must be evaluated.
    virtual bool NeedCheckTargetStaticPermission() const
    {
        return true;
    }

    // Whether abilityInfo.visible must be evaluated.
    virtual bool NeedCheckTargetVisible() const
    {
        return true;
    }
};

// Insight intent channel: both target-declared checks are waived, caller-side checks still run.
class InsightIntentPermissionChecker final : public AbilityPermissionChecker {
public:
    InsightIntentPermissionChecker() = default;
    ~InsightIntentPermissionChecker() override = default;

    const char *GetSceneName() const override
    {
        return "insight_intent";
    }

    bool NeedCheckTargetStaticPermission() const override
    {
        return false;
    }

    bool NeedCheckTargetVisible() const override
    {
        return false;
    }
};

// The all-checks-enabled checker used by channels that carry no policy.
inline const std::shared_ptr<const AbilityPermissionChecker> &GetDefaultPermissionChecker()
{
    static const std::shared_ptr<const AbilityPermissionChecker> defaultChecker =
        std::make_shared<const AbilityPermissionChecker>();
    return defaultChecker;
}

// nullptr falls back to the default checker, so call sites never branch on null.
inline const AbilityPermissionChecker &GetPermissionChecker(
    const std::shared_ptr<const AbilityPermissionChecker> &permissionChecker)
{
    return permissionChecker == nullptr ? *GetDefaultPermissionChecker() : *permissionChecker;
}
} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_ABILITY_PERMISSION_CHECKER_H
