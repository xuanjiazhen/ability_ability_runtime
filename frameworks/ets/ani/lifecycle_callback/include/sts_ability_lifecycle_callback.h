/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_STS_ABILITY_LIFECYCLE_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_STS_ABILITY_LIFECYCLE_CALLBACK_H

#include <optional>

#include "sts_runtime.h"

namespace OHOS {
namespace AbilityRuntime {
class STSAbilityLifecycleCallback : public std::enable_shared_from_this<STSAbilityLifecycleCallback> {
public:
    explicit STSAbilityLifecycleCallback(ani_env* env);
    
    /**
     * Called back when the ability is started for initialization.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void OnAbilityCreate(const std::shared_ptr<AbilityRuntime::STSNativeReference> &ability);

    /**
     * Called back when the window stage is created.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @param windowStage: Indicates the window stage to create.
     * @StageModelOnly
     */
    void OnWindowStageCreate(const std::shared_ptr<AbilityRuntime::STSNativeReference> &ability,
        const std::shared_ptr<AbilityRuntime::STSNativeReference> &windowStage);

    /**
     * Called back when the window stage is destroy.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @param windowStage: Indicates the window stage to destroy.
     * @StageModelOnly
     */
    void OnWindowStageDestroy(const std::shared_ptr<AbilityRuntime::STSNativeReference> &ability,
        const std::shared_ptr<AbilityRuntime::STSNativeReference> &windowStage);

    /**
     * Called back when the window stage is active.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @param windowStage: Indicates the window stage to active.
     * @StageModelOnly
     */
    void OnWindowStageActive(const std::shared_ptr<AbilityRuntime::STSNativeReference> &ability,
        const std::shared_ptr<AbilityRuntime::STSNativeReference> &windowStage);

    /**
     * Called back when the window stage is inactive.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @param windowStage: Indicates the window stage to inactive.
     * @StageModelOnly
     */
    void OnWindowStageInactive(const std::shared_ptr<AbilityRuntime::STSNativeReference> &ability,
        const std::shared_ptr<AbilityRuntime::STSNativeReference> &windowStage);

    /**
     * Called back when the ability is destroy.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void OnAbilityDestroy(const std::shared_ptr<AbilityRuntime::STSNativeReference> &ability);

    /**
     * Called back when the ability is foreground.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void OnAbilityForeground(const std::shared_ptr<AbilityRuntime::STSNativeReference> &ability);

    /**
     * Called back when the ability is background.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void OnAbilityBackground(const std::shared_ptr<AbilityRuntime::STSNativeReference> &ability);

    /**
     * Called back when the ability is continue.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void OnAbilityContinue(const std::shared_ptr<AbilityRuntime::STSNativeReference> &ability);

    /**
     * Called back before an ability is started for initialization.
     *
     * @since 12
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void OnAbilityWillCreate(const std::shared_ptr<AbilityRuntime::STSNativeReference> &ability);

    /**
     * Called back before a window stage is created.
     *
     * @since 12
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @param windowStage: Indicates the window stage to active.
     * @StageModelOnly
     */
    void OnWindowStageWillCreate(const std::shared_ptr<AbilityRuntime::STSNativeReference> &ability,
        const std::shared_ptr<AbilityRuntime::STSNativeReference> &windowStage);

    /**
     * Called back before the UIAbility will called onNewWant.
     *
     * @since 12
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void OnWillNewWant(const std::shared_ptr<AbilityRuntime::STSNativeReference> &ability);

    /**
     * Called back after the UIAbility called onNewWant.
     *
     * @since 12
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void OnNewWant(const std::shared_ptr<AbilityRuntime::STSNativeReference> &ability);

    /**
     * Called back before a window stage is destroyed.
     *
     * @since 12
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @param windowStage: Indicates the window stage to active.
     * @StageModelOnly
     */
    void OnWindowStageWillDestroy(const std::shared_ptr<AbilityRuntime::STSNativeReference> &ability,
        const std::shared_ptr<AbilityRuntime::STSNativeReference> &windowStage);

    /**
     * Called back before an ability is destroyed.
     *
     * @since 12
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void OnAbilityWillDestroy(const std::shared_ptr<AbilityRuntime::STSNativeReference> &ability);

    /**
     * Called back before the state of an ability changes to foreground.
     *
     * @since 12
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void OnAbilityWillForeground(const std::shared_ptr<AbilityRuntime::STSNativeReference> &ability);

    /**
     * Called back before the state of an ability changes to background.
     *
     * @since 12
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void OnAbilityWillBackground(const std::shared_ptr<AbilityRuntime::STSNativeReference> &ability);

    /**
     * Callback before the onContinue trigger of UIAbility is invoked
     * 
     * @since 12
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void OnAbilityWillContinue(const std::shared_ptr<AbilityRuntime::STSNativeReference> &ability);
    
    /**
     * Callback before the onWindowStageRestore event of UIAbility is triggered
     *
     * @since 12
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability Indicates the ability to register for listening.
     * @param windowStage The window stage to be restored.
     * @StageModelOnly
     */
    void OnWindowStageWillRestore(const std::shared_ptr<AbilityRuntime::STSNativeReference> &ability,
        const std::shared_ptr<AbilityRuntime::STSNativeReference> &windowStage);

    /**
     * After the onWindowStageRestore event of UIAbility is triggered, it will call back.
     *
     * @since 12
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability Indicates the ability to register for listening.
     * @param windowStage The restored window stage instance.
     * @StageModelOnly
     */
    void OnWindowStageRestore(const std::shared_ptr<AbilityRuntime::STSNativeReference> &ability,
        const std::shared_ptr<AbilityRuntime::STSNativeReference> &windowStage);

    /**
     * Callback before the onSaveState trigger of UIAbility is invoked
     *
     * @since 12
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void OnAbilityWillSaveState(const std::shared_ptr<AbilityRuntime::STSNativeReference> &ability);

    /**
     * After the onSaveState trigger of UIAbility is invoked, the callback is triggered.
     *
     * @since 12
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void OnAbilitySaveState(const std::shared_ptr<AbilityRuntime::STSNativeReference> &ability);

private:
    void CallLifecycleCallbackFunction(const std::string &functionName,
        const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj,
        const std::shared_ptr<AbilityRuntime::STSNativeReference> &windowStageObj = nullptr);
    void CallLifecycleCallbackOptionalFunction(const std::string &functionName,
        const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj,
        const std::shared_ptr<AbilityRuntime::STSNativeReference> &windowStageObj = nullptr);
    std::optional<std::pair<ani_class, ani_object>> InitializeCallbackObject(const std::string &functionName);

private:
    ani_env* env_ = nullptr;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_STS_ABILITY_LIFECYCLE_CALLBACK_H