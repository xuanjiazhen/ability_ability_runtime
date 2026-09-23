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

#ifndef MOCK_FOCUS_STATE_OBSERVER_STUB_H
#define MOCK_FOCUS_STATE_OBSERVER_STUB_H

#include <gmock/gmock.h>
#include "focus_state_observer_stub.h"

namespace OHOS {
namespace AppExecFwk {
class MockFocusStateObserverStub : public FocusStateObserverStub {
public:
    MockFocusStateObserverStub() = default;
    ~MockFocusStateObserverStub() override = default;
    MOCK_METHOD1(OnFocusStateChanged, void(const FocusStateData &focusStateData));
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MOCK_FOCUS_STATE_OBSERVER_STUB_H

一、/interface/sdk-js/api/application/FocusedApplicationObserver.d.ts 文档：
/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

/**
 * @file
 * @kit AbilityKit
 */

import FocusedApplicationInfo from './FocusedApplicationInfo';

/**
 * Defines the observer used to listen for focused application changes. It is used as an input parameter of
 * [appManager.onFocusedApplicationChanged]{@link @ohos.app.ability.appManager:appManager.onFocusedApplicationChanged}
 * to listen for focused application changes.
 *
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @stagemodelonly
 * @since 26.2.0 dynamic&static
 */
declare interface FocusedApplicationObserver {
  /**
   * Called when the focused application changes.
   *
   * @param { FocusedApplicationInfo } info - Focused application information.
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @stagemodelonly
   * @since 26.2.0 dynamic&static
   */
  onFocusedApplicationChanged(info: FocusedApplicationInfo): void;
}

export default FocusedApplicationObserver;

二、/interface/sdk-js/api/application/FocusedApplicationInfo.d.ts 文档：
/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

/**
 * @file
 * @kit AbilityKit
 */

/**
 * Defines the focused application information reported when the focused application changes.
 *
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @stagemodelonly
 * @since 26.2.0 dynamic&static
 */
declare interface FocusedApplicationInfo {
  /**
   * Bundle name of the focused application.
   *
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @stagemodelonly
   * @since 26.2.0 dynamic&static
   */
  focusedBundleName: string;

  /**
   * UID of the focused application.
   * The value should be an integer.
   *
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @stagemodelonly
   * @since 26.2.0 dynamic&static
   */
  focusedUid: int;
}

export default FocusedApplicationInfo;


三、/interface/sdk-js/api/@ohos.app.ability.appManager.d.ts 文档修改：
1、
import _FocusedApplicationObserver from './application/FocusedApplicationObserver';
import _FocusedApplicationInfo from './application/FocusedApplicationInfo';
2、
  /**
   * Registers an observer to listen for focused application changes.
   *
   * @permission ohos.permission.RUNNING_STATE_OBSERVER
   * @param { FocusedApplicationObserver } observer - Observer used to listen for focused application changes.
   * @returns { int } ID of the observer registered. You can pass this ID to
   *     [offFocusedApplicationChanged]{@link appManager.offFocusedApplicationChanged} to unregister the observer.
   * @throws { BusinessError } 201 - Permission verification failed. The application does not have the permission
   *     required to call the API.
   * @throws { BusinessError } 16000050 - Connect to system server failed.
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @stagemodelonly
   * @since 26.2.0 dynamic&static
   */
  function onFocusedApplicationChanged(observer: FocusedApplicationObserver): int;

  /**
   * Unregisters the observer used to listen for focused application changes.
   *
   * @permission ohos.permission.RUNNING_STATE_OBSERVER
   * @param { int } observerId - ID of the observer registered, which is the listener ID returned by
   *     [onFocusedApplicationChanged]{@link appManager.onFocusedApplicationChanged}.
   * @throws { BusinessError } 201 - Permission verification failed. The application does not have the permission
   *     required to call the API.
   * @throws { BusinessError } 16000050 - Connect to system server failed.
   * @throws { BusinessError } 16300004 - The observer does not exist.
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @stagemodelonly
   * @since 26.2.0 dynamic&static
   */
  function offFocusedApplicationChanged(observerId: int): void;
  3、
    /**
   * Defines the observer used to listen for focused application changes.
   *
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @stagemodelonly
   * @since 26.2.0 dynamic&static
   */
  export type FocusedApplicationObserver = _FocusedApplicationObserver;

  /**
   * Defines the focused application information reported when the focused application changes.
   *
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @stagemodelonly
   * @since 26.2.0 dynamic&static
   */
  export type FocusedApplicationInfo = _FocusedApplicationInfo;
