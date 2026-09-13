'use static'
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

import { FocusStateData } from './FocusStateData';

/**
 * The module defines the focus state data reported by the callback when the application focus changes. After
 * registering the application focus state change listener by using
 * [onAppFocusStateChange]{@link ./../@ohos.app.ability.appManager:appManager.onAppFocusStateChange}
 * , you can obtain the reported struct through the
 * [onFocusStateChanged]{@link AppFocusStateObserver:AppFocusStateObserver.onFocusStateChanged}
 * callback of [AppFocusStateObserver]{@link AppFocusStateObserver:AppFocusStateObserver}.
 *
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 27 dynamic&static
 */
declare interface FocusStateData {
  /**
   * The UID of the application that gains focus.
   *
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @since 27 dynamic&static
   */
  focusedUid: int;

  /**
   * The UID of the application that loses focus. The value **-1** indicates that there is no previously focused
   * application (for example, the first application to gain focus after the device is started).
   *
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @since 27 dynamic&static
   */
  unfocusedUid: int;

  /**
   * The bundle name of the application that gains focus.
   *
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @since 27 dynamic&static
   */
  focusedBundleName: string;

  /**
   * The bundle name of the application that loses focus. The value is an empty string if there is no previously
   * focused application (for example, the first application to gain focus after the device is started).
   *
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @since 27 dynamic&static
   */
  unfocusedBundleName: string;
}

export default FocusStateData;
