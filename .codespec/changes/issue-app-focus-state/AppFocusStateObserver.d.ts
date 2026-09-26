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

/**
 * @file
 * @kit AbilityKit
 */

/*** if arkts dynamic */
import type FocusStateData from './FocusStateData';
/*** endif */
/*** if arkts static */
import { FocusStateData } from './FocusStateData';
/*** endif */

/**
 * The module defines the observer used to listen for application focus state changes. It is used as an input parameter
 * of
 * [onAppFocusStateChange]{@link ./../@ohos.app.ability.appManager:appManager.onAppFocusStateChange}
 * to listen for the focus state changes of all applications.
 *
 * This observer is notified only when the focus changes between different applications (different UIDs).
 * Focus changes within the same application (same UID, such as window switching) do not trigger the callback.
 *
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 27 dynamic&static
 */
export interface AppFocusStateObserver {

  /**
   * Called when the application focus state changes. This callback is triggered only when the focus switches between
   * different applications (different UIDs). Focus changes within the same application do not trigger this callback.
   *
   * @param { FocusStateData } focusStateData - Focus state data, including the UID of the application that gains focus,
   *     the UID of the application that loses focus, and the bundle name of the application that gains focus.
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @since 27 dynamic&static
   */
  onFocusStateChanged(focusStateData: FocusStateData): void;
}
