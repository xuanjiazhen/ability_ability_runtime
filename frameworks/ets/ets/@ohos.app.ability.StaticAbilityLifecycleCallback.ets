/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

import UIAbility from '@ohos.app.ability.UIAbility';
import window from '@ohos.window';

export default class StaticAbilityLifecycleCallback {
  onAbilityCreate(ability: UIAbility): void {};

  onAbilityWillCreate?: (ability: UIAbility) => void;

  onWindowStageCreate(ability: UIAbility, windowStage: window.WindowStage): void {};

  onWindowStageWillCreate?: (ability: UIAbility, windowStage: window.WindowStage) => void;

  onWillNewWant?: (ability: UIAbility) => void;

  onNewWant?: (ability: UIAbility) => void;

  onWindowStageActive(ability: UIAbility, windowStage: window.WindowStage): void {};

  onWindowStageInactive(ability: UIAbility, windowStage: window.WindowStage): void {};

  onWindowStageDestroy(ability: UIAbility, windowStage: window.WindowStage): void {};

  onWindowStageWillDestroy?: (ability: UIAbility, windowStage: window.WindowStage) => void;

  onAbilityDestroy(ability: UIAbility): void {};

  onAbilityWillDestroy?: (ability: UIAbility) => void;

  onAbilityForeground(ability: UIAbility): void {};

  onAbilityWillForeground?: (ability: UIAbility) => void;

  onAbilityBackground(ability: UIAbility): void {};

  onAbilityWillBackground?: (ability: UIAbility) => void;

  onAbilityContinue(ability: UIAbility): void {};

  onAbilityWillContinue?: (ability: UIAbility) => void;

  onWindowStageRestore?: (ability: UIAbility, windowStage: window.WindowStage) => void;

  onWindowStageWillRestore?: (ability: UIAbility, windowStage: window.WindowStage) => void;

  onAbilitySaveState?: (ability: UIAbility) => void;

  onAbilityWillSaveState?: (ability: UIAbility) => void;
}
