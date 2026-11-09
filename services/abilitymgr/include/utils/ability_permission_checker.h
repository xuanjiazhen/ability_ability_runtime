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
/**
 * @brief Which channel issues the start request, and therefore which restrictions declared by the
 * TARGET component may be waived. The scene is decided by AMS itself and is never carried in a
 * Want or any parcelable, so an external caller cannot upgrade its own scene.
 */
enum class PermissionCheckScene : int32_t {
    DEFAULT = 0,
    INSIGHT_INTENT = 1,
};

/**
 * @brief Permission checks against the restrictions declared by the TARGET component
 * (module.json5 side). Every instance starts from "check everything"; a scene may only turn
 * individual checks off. Caller-side validation is never affected.
 *
 * The business action lives here rather than at the call sites: AMS hands in the real check as a
 * callable and the checker decides whether to run it or to answer with the waived result. Call
 * sites therefore hold no "if (waived)" branch and no duplicated waiver log.
 *
 * Two target-declared restrictions can be waived:
 *   - the static config permission, i.e. abilityInfo.permissions / skill.permissions;
 *   - the exported visibility, i.e. abilityInfo.visible (exported=false).
 */
class AbilityPermissionChecker {
public:
    AbilityPermissionChecker() = default;
    virtual ~AbilityPermissionChecker() = default;

    virtual PermissionCheckScene GetScene() const
    {
        return PermissionCheckScene::DEFAULT;
    }

    virtual const char *GetSceneName() const
    {
        return "default";
    }

    /**
     * @brief Evaluate the permission the target declared for ordinary starts.
     * @param realCheck the real evaluation. It runs only when the check is required by the scene.
     * @return the grant status, AppExecFwk::Constants::PERMISSION_GRANTED when waived.
     */
    template <typename RealCheck>
    int32_t CheckTargetStaticPermission(const RealCheck &realCheck) const
    {
        if (NeedCheckTargetStaticPermission()) {
            return realCheck();
        }
        TAG_LOGD(AAFwkTag::ABILITYMGR, "target static permission waived, scene:%{public}s", GetSceneName());
        return AppExecFwk::Constants::PERMISSION_GRANTED;
    }

    /**
     * @brief Judge whether the caller may start the exported=false target.
     * @param realJudge the real judgement. It runs only when the check is required by the scene.
     * @return ERR_OK when permitted, otherwise the real error code.
     */
    template <typename RealJudge>
    int32_t JudgeTargetVisible(const RealJudge &realJudge) const
    {
        if (NeedCheckTargetVisible()) {
            return realJudge();
        }
        TAG_LOGD(AAFwkTag::ABILITYMGR, "target visible check waived, scene:%{public}s", GetSceneName());
        return ERR_OK;
    }

    /**
     * @brief Fold the declared visibility into the effective one carried by VerificationInfo, so
     * that PermissionVerification receives the value this scene is entitled to.
     * @param declaredVisible abilityInfo.visible as declared by the target, or a value already
     *        adjusted by an earlier rule (the DMS same-app case).
     * @return the visibility the check should use.
     */
    bool ResolveTargetVisible(bool declaredVisible) const
    {
        if (NeedCheckTargetVisible()) {
            return declaredVisible;
        }
        TAG_LOGD(AAFwkTag::ABILITYMGR, "target visible waived, scene:%{public}s", GetSceneName());
        return true;
    }

    /** @return true when the permission declared by the target has to be evaluated. */
    virtual bool NeedCheckTargetStaticPermission() const
    {
        return true;
    }

    /** @return true when abilityInfo.visible has to be evaluated. */
    virtual bool NeedCheckTargetVisible() const
    {
        return true;
    }
};

/**
 * @brief Insight intent execution: the target ability/extension is started on behalf of a verified
 * intent record, so the restrictions the target declared (its own permission and its exported
 * visibility) do not apply to this channel. The caller still has to hold every caller-side
 * permission it would need for a normal start.
 */
class InsightIntentPermissionChecker final : public AbilityPermissionChecker {
public:
    InsightIntentPermissionChecker() = default;
    ~InsightIntentPermissionChecker() override = default;

    PermissionCheckScene GetScene() const override
    {
        return PermissionCheckScene::INSIGHT_INTENT;
    }

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

/**
 * @brief The checker used by every channel that carries no policy: all checks enabled.
 */
inline const std::shared_ptr<const AbilityPermissionChecker> &GetDefaultPermissionChecker()
{
    static const std::shared_ptr<const AbilityPermissionChecker> defaultChecker =
        std::make_shared<const AbilityPermissionChecker>();
    return defaultChecker;
}

/**
 * @brief Build the checker for a scene. Every scene is initialised to "check everything"; only the
 * insight intent channel turns the two target-declared checks off.
 */
inline std::shared_ptr<const AbilityPermissionChecker> CreatePermissionChecker(PermissionCheckScene scene)
{
    if (scene == PermissionCheckScene::INSIGHT_INTENT) {
        return std::make_shared<const InsightIntentPermissionChecker>();
    }
    return GetDefaultPermissionChecker();
}

/**
 * @brief Resolve the checker to consult. Channels that pass no policy (every start except insight
 * intent) fall back to the default checker, i.e. all checks enabled, so call sites never branch on
 * a null pointer.
 */
inline const AbilityPermissionChecker &GetPermissionChecker(
    const std::shared_ptr<const AbilityPermissionChecker> &permissionChecker)
{
    return permissionChecker == nullptr ? *GetDefaultPermissionChecker() : *permissionChecker;
}
} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_ABILITY_PERMISSION_CHECKER_H
