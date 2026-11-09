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

namespace OHOS {
namespace AAFwk {
/**
 * @brief Which channel issues the start request, and therefore which target-declared restrictions
 * may be waived. The scene is decided by AMS itself and is never carried in a Want or any
 * parcelable, so an external caller cannot upgrade its own scene.
 */
enum class PermissionCheckScene : int32_t {
    DEFAULT = 0,
    INSIGHT_INTENT = 1,
};

/**
 * @brief Policy object consulted by AMS before evaluating the restrictions declared by the TARGET
 * component (module.json5 side). It never touches caller-side validation.
 *
 * Only two switches exist, both about what the target application declared:
 *   - NeedCheckAbilityStaticPermission(): abilityInfo.permissions / skill.permissions, i.e. the
 *     static config permission evaluated by CheckStaticCfgPermission.
 *   - NeedCheckAbilityVisible(): abilityInfo.visible (exported=false), evaluated by
 *     JudgeAbilityVisibleControl and by the invisible check inside JudgeInvisibleAndBackground.
 *
 * Everything decided by the CALLER stays as is for every scene: background start permission,
 * ability background communication permission, continuous task validation, UI_SERVICE background
 * validation, associated wake up, and the identity carried with the request.
 *
 * No checker at all (nullptr) means "keep every existing check", which is what all current start
 * paths use.
 */
class AbilityPermissionChecker {
public:
    AbilityPermissionChecker() = default;
    virtual ~AbilityPermissionChecker() = default;

    virtual PermissionCheckScene GetScene() const
    {
        return PermissionCheckScene::DEFAULT;
    }

    /** @return false to treat the target as if it declared no permission (CheckStaticCfgPermission). */
    virtual bool NeedCheckAbilityStaticPermission() const
    {
        return true;
    }

    /** @return false to treat the target as visible (exported=true) for visibility judging. */
    virtual bool NeedCheckAbilityVisible() const
    {
        return true;
    }

    virtual const char *GetSceneName() const
    {
        return "default";
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

    bool NeedCheckAbilityStaticPermission() const override
    {
        return false;
    }

    bool NeedCheckAbilityVisible() const override
    {
        return false;
    }

    const char *GetSceneName() const override
    {
        return "insight_intent";
    }
};

/**
 * @brief Build the checker for a scene. nullptr means "no override": every existing check stays.
 */
inline std::shared_ptr<const AbilityPermissionChecker> CreatePermissionChecker(PermissionCheckScene scene)
{
    if (scene == PermissionCheckScene::INSIGHT_INTENT) {
        return std::make_shared<const InsightIntentPermissionChecker>();
    }
    return nullptr;
}
} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_ABILITY_PERMISSION_CHECKER_H
