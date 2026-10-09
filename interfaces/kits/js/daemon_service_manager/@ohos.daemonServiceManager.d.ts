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
 * Provides daemon service management APIs.
 *
 * @namespace daemonServiceManager
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @systemapi
 */
declare namespace daemonServiceManager {
  /**
   * Sets whether a registered daemon can be started.
   *
   * @param bundleName Bundle that owns the daemon.
   * @param daemonName Name used when the daemon was registered.
   * @param enabled Whether subsequent start requests are allowed.
   * @returns Promise resolved after the state has been updated.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 2930003 - Daemon is not registered.
   * @permission ohos.permission.MANAGE_DAEMON_SERVICE
   * @systemapi
   */
  function setDaemonEnabled(bundleName: string, daemonName: string, enabled: boolean): Promise<void>;
}

export default daemonServiceManager;
