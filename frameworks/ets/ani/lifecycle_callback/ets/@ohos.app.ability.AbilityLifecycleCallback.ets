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
import dataAbility from '@ohos.data.dataAbility';
import window from '@ohos.window';

loadLibrary("ability_lifecycle_callback_ani_kit.z");

export default interface AbilityLifecycleCallback {
  onAbilityCreate(ability: UIAbility): void;

  onWindowStageCreate(ability: UIAbility, windowStage: window.WindowStage): void;

  onWindowStageActive(ability: UIAbility, windowStage: window.WindowStage): void;

  onWindowStageInactive(ability: UIAbility, windowStage: window.WindowStage): void;

  onWindowStageDestroy(ability: UIAbility, windowStage: window.WindowStage): void;

  onAbilityDestroy(ability: UIAbility): void;

  onAbilityForeground(ability: UIAbility): void;

  onAbilityBackground(ability: UIAbility): void;

  onAbilityContinue(ability: UIAbility): void;
  
  // 可选方法
  onAbilityWillCreate?: (ability: UIAbility) => void;

  onWindowStageWillCreate?: (ability: UIAbility, windowStage: window.WindowStage) => void;

  onWillNewWant?: (ability: UIAbility) => void;

  onNewWant?: (ability: UIAbility) => void;

  onWindowStageWillDestroy?: (ability: UIAbility, windowStage: window.WindowStage) => void;

  onAbilityWillDestroy?: (ability: UIAbility) => void;

  onAbilityWillForeground?: (ability: UIAbility) => void;

  onAbilityWillBackground?: (ability: UIAbility) => void;

  onAbilityWillContinue?: (ability: UIAbility) => void;

  onWindowStageWillRestore?: (ability: UIAbility, windowStage: window.WindowStage) => void;

  onWindowStageRestore?: (ability: UIAbility, windowStage: window.WindowStage) => void;

  onAbilityWillSaveState?: (ability: UIAbility) => void;

  onAbilitySaveState?: (ability: UIAbility) => void;
}

class AbilityLifecycleCallbackInner implements AbilityLifecycleCallback {
  onAbilityCreate(ability: UIAbility): void {
    console.log("onAbilityCreate is called in ets");
  }

  onWindowStageCreate(ability: UIAbility, windowStage: window.WindowStage): void {
    console.log("onWindowStageCreate is called in ets");
  }

  onWindowStageActive(ability: UIAbility, windowStage: window.WindowStage): void {
    console.log("onWindowStageActive is called in ets");
  }

  onWindowStageInactive(ability: UIAbility, windowStage: window.WindowStage): void {
    console.log("onWindowStageInactive is called in ets");
  }

  onWindowStageDestroy(ability: UIAbility, windowStage: window.WindowStage): void {
    console.log("onWindowStageDestroy is called in ets");
  }

  onAbilityDestroy(ability: UIAbility): void {
    console.log("onAbilityDestroy is called in ets");
  }

  onAbilityForeground(ability: UIAbility): void {
    console.log("onAbilityForeground is called in ets");
  }

  onAbilityBackground(ability: UIAbility): void {
    console.log("onAbilityBackground is called in ets");
  }

  onAbilityContinue(ability: UIAbility): void {
    console.log("onAbilityContinue is called in ets");
  }

  // 可选方法的实现
  onAbilityWillCreate?: (ability: UIAbility) => void = (ability: UIAbility): void => {
    console.log("onAbilityWillCreate is called in ets");
  }

  onWindowStageWillCreate?: (ability: UIAbility, windowStage: window.WindowStage) => void = (ability: UIAbility, windowStage: window.WindowStage): void => {
    console.log("onWindowStageWillCreate is called in ets");
  }

  onWillNewWant?: (ability: UIAbility) => void = (ability: UIAbility): void => {
    console.log("onWillNewWant is called in ets");
  }

  onNewWant?: (ability: UIAbility) => void = (ability: UIAbility): void => {
    console.log("onNewWant is called in ets");
  }

  onWindowStageWillDestroy?: (ability: UIAbility, windowStage: window.WindowStage) => void = (ability: UIAbility, windowStage: window.WindowStage): void => {
    console.log("onWindowStageWillDestroy is called in ets");
  }

  onAbilityWillDestroy?: (ability: UIAbility) => void = (ability: UIAbility): void => {
    console.log("onAbilityWillDestroy is called in ets");
  }

  onAbilityWillForeground?: (ability: UIAbility) => void = (ability: UIAbility): void => {
    console.log("onAbilityWillForeground is called in ets");
  }

  onAbilityWillBackground?: (ability: UIAbility) => void = (ability: UIAbility): void => {
    console.log("onAbilityWillBackground is called in ets");
  }

  onAbilityWillContinue?: (ability: UIAbility) => void = (ability: UIAbility): void => {
    console.log("onAbilityWillContinue is called in ets");
  }

  onWindowStageWillRestore?: (ability: UIAbility, windowStage: window.WindowStage) => void = (ability: UIAbility, windowStage: window.WindowStage): void => {
    console.log("onWindowStageWillRestore is called in ets");
  }

  onWindowStageRestore?: (ability: UIAbility, windowStage: window.WindowStage) => void = (ability: UIAbility, windowStage: window.WindowStage): void => {
    console.log("onWindowStageRestore is called in ets");
  }

  onAbilityWillSaveState?: (ability: UIAbility) => void = (ability: UIAbility): void => {
    console.log("onAbilityWillSaveState is called in ets");
  }

  onAbilitySaveState?: (ability: UIAbility) => void = (ability: UIAbility): void => {
    console.log("onAbilitySaveState is called in ets");
  }
}