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

import AbilityStateData from 'application.AbilityStateData'
import ProcessData from 'application.ProcessData'
import AppStateData from 'application.AppStateData'

export default class ApplicationStateObserver {
  onForegroundApplicationChanged(appStateData: AppStateData): void {
    console.info("========================================================== onForegroundApplicationChanged");
  }
  onAbilityStateChanged(abilityStateData: AbilityStateData): void {
    console.info("========================================================== onAbilityStateChanged");
  }

  onProcessCreated(processData: ProcessData): void {
    console.info("========================================================== onProcessCreated");
  }

  onProcessDied(processData: ProcessData): void {
    console.info("========================================================== onProcessDied");
  }

  onProcessStateChanged(processData: ProcessData): void {
    console.info("========================================================== onProcessStateChanged");
  }

  onAppStarted(appStateData: AppStateData): void {
    console.info("========================================================== onAppStarted");
  }

  onAppStopped(appStateData: AppStateData): void {
    console.info("========================================================== onAppStopped");
  }
}