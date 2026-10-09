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
   * Registers a daemon declared in module.json5 daemonServices.
   *
   * @param bundleName Bundle that owns the daemon. Must match the caller's bundle name.
   * @param daemonName Name declared in module.json5 daemonServices.
   * @returns Promise resolved after the daemon is registered.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 2930001 - Daemon is already registered.
   * @throws { BusinessError } 2930002 - Registration was rejected by the administrator.
   * @throws { BusinessError } 2930006 - Device does not support daemon service.
   * @permission ohos.permission.MANAGE_DAEMON_SERVICE
   */
  function register(bundleName: string, daemonName: string): Promise<void>;

  /**
   * Unregisters a previously registered daemon. Stops a running daemon process.
   *
   * @param bundleName Bundle that owns the daemon.
   * @param daemonName Name used when the daemon was registered.
   * @returns Promise resolved after the daemon is unregistered.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 2930003 - Daemon is not registered.
   * @permission ohos.permission.MANAGE_DAEMON_SERVICE
   */
  function unregister(bundleName: string, daemonName: string): Promise<void>;
