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
 *
 * @note This file carries the incremental additions for the process lifecycle callback
 * feature (REQ-PROCESS-LIFECYCLE-001). It should be merged into the existing
 * ApplicationContext.d.ts of the interface_sdk-js repository.
 */

import type { ProcessLifecycleCallback } from '../@ohos.app.ability.ProcessLifecycleCallback';

/**
 * Provides the context environment of the application.
 *
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @stagemodelonly
 * @since 9
 */
export default class ApplicationContext {
    /**
     * Registers a callback to listen for process lifecycle events of the current application.
     *
     * When registered, the callback will be invoked when any process of the current application is created or exits,
     * including child processes (APP_SPAWN_FORK mode), native child processes, render processes (a subtype of child
     * processes), preloaded template processes, worker processes derived from templates, and independent processes
     * (multi-process HAP modules). SELF_FORK processes and mirror processes are not included.
     *
     * The callback is invoked on all surviving processes of the same application that have registered this callback.
     * The exiting process itself does not receive the callback.
     *
     * No permission is required.
     *
     * @param { ProcessLifecycleCallback } callback - Callback to register.
     * @returns { int } ID of the observer registered. You can pass this ID to
     *     [offProcessLifecycle]{@link ApplicationContext.offProcessLifecycle} to unregister the callback.
     * @throws { BusinessError } 401 - Parameter error. Possible causes:
     *     1. Mandatory parameters are left unspecified; 2. Incorrect parameter types.
     * @throws { BusinessError } 16000050 - Internal error.
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @stagemodelonly
     * @since 27.0.0 dynamic&static
     */
    onProcessLifecycle(callback: ProcessLifecycleCallback): int;

    /**
     * Unregisters a callback for process lifecycle events by observerId.
     *
     * @param { int } observerId - ID of the observer registered, which is the ID returned by
     *     [onProcessLifecycle]{@link ApplicationContext.onProcessLifecycle}.
     * @throws { BusinessError } 401 - Parameter error. Possible causes:
     *     1. Mandatory parameters are left unspecified; 2. Incorrect parameter types.
     * @throws { BusinessError } 16000050 - Internal error.
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @stagemodelonly
     * @since 27.0.0 dynamic&static
     */
    offProcessLifecycle(observerId: int): void;
}
