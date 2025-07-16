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

import window from '@ohos.window';

export default class InteropAbilityLifecycleCallback {
  onAbilityCreate(ability: Object): void {};

  onAbilityWillCreate?: (ability: Object) => void;

  onWindowStageCreate(ability: Object, windowStage: window.WindowStage): void {};

  onWindowStageWillCreate?: (ability: Object, windowStage: window.WindowStage) => void;

  onWillNewWant?: (ability: Object) => void;

  onNewWant?: (ability: Object) => void;

  onWindowStageActive(ability: Object, windowStage: window.WindowStage): void {};

  onWindowStageInactive(ability: Object, windowStage: window.WindowStage): void {};

  onWindowStageDestroy(ability: Object, windowStage: window.WindowStage): void {};

  onWindowStageWillDestroy?: (ability: Object, windowStage: window.WindowStage) => void;

  onAbilityDestroy(ability: Object): void {};

  onAbilityWillDestroy?: (ability: Object) => void;

  onAbilityForeground(ability: Object): void {};

  onAbilityWillForeground?: (ability: Object) => void;

  onAbilityBackground(ability: Object): void {};

  onAbilityWillBackground?: (ability: Object) => void;

  onAbilityContinue(ability: Object): void {};

  onAbilityWillContinue?: (ability: Object) => void;

  onWindowStageRestore?: (ability: Object, windowStage: window.WindowStage) => void;

  onWindowStageWillRestore?: (ability: Object, windowStage: window.WindowStage) => void;

  onAbilitySaveState?: (ability: Object) => void;

  onAbilityWillSaveState?: (ability: Object) => void;
}
