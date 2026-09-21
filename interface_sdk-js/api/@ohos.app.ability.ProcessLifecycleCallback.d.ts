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

import type { ProcessLifecycleInfo } from './application/ProcessLifecycleInfo';

/**
 * Provides the callbacks for process lifecycle. The callback is triggered when a process of the current application
 * is created or exits.
 *
 * To listen for process lifecycle events, register this callback through
 * {@link ApplicationContext.onProcessLifecycle | ApplicationContext.onProcessLifecycle()}.
 *
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @stagemodelonly
 * @since 27.0.0 dynamic&static
 */
export default class ProcessLifecycleCallback {
    /**
     * Called when a process of the current application is created.
     *
     * The process types that can trigger this callback include:
     * - Child processes started by {@link childProcessManager.startChildProcess} (APP_SPAWN_FORK mode),
     *   {@link childProcessManager.startArkChildProcess}, or {@link childProcessManager.startNativeChildProcess}.
     * - Native child processes started by {@link OH_Ability_CreateNativeChildProcess} or
     *   {@link OH_Ability_StartNativeChildProcess}.
     * - Render processes (a subtype of child processes).
     * - Preloaded template processes.
     * - Worker processes derived from preloaded templates.
     * - Independent processes (multi-process HAP modules).
     *
     * SELF_FORK processes and mirror processes are not included.
     *
     * @param { ProcessLifecycleInfo } info - Information about the created process.
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @stagemodelonly
     * @since 27.0.0 dynamic&static
     */
    onProcessCreated(info: ProcessLifecycleInfo): void;

    /**
     * Called when a process of the current application exits, including abnormal exits such as crash or kill -9.
     *
     * The process types that can trigger this callback are the same as those described in
     * {@link onProcessCreated}. SELF_FORK processes and mirror processes are not included.
     *
     * The exiting process itself does not receive this callback. Only other surviving processes
     * of the same application that have registered this callback will be notified.
     *
     * @param { ProcessLifecycleInfo } info - Information about the exited process.
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @stagemodelonly
     * @since 27.0.0 dynamic&static
     */
    onProcessExited(info: ProcessLifecycleInfo): void;
}
