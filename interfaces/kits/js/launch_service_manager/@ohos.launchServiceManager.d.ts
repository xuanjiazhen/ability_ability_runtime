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
 * Provides launch service management APIs.
 *
 * @namespace launchServiceManager
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 26.2.0
 * @stagemodelonly
 */
declare namespace launchServiceManager {
  /**
   * Describes a launch service declared in module.json5 daemonServices.
   *
   * @since 26.2.0
   */
  export interface LaunchServiceDescriptor {
    /**
     * Bundle that owns the launch service.
     *
     * @since 26.2.0
     */
    bundleName: string;
    /**
     * Launch service name declared in module.json5 daemonServices.
     *
     * @since 26.2.0
     */
    daemonName: string;
  }

  /**
   * Registration state of a launch service.
   *
   * @since 26.2.0
   */
  export enum LaunchServiceState {
    /**
     * The launch service is not registered.
     *
     * @since 26.2.0
     */
    NOT_REGISTERED = 0,
    /**
     * The launch service is registered and enabled.
     *
     * @since 26.2.0
     */
    ENABLED = 1,
    /**
     * The launch service is registered but disabled.
     *
     * @since 26.2.0
     */
    DISABLED = 2
  }

  /**
   * Query result of a launch service.
   *
   * @since 26.2.0
   */
  export interface LaunchServiceInformation {
    /**
     * Bundle that owns the launch service.
     *
     * @since 26.2.0
     */
    bundleName: string;
    /**
     * Launch service name.
     *
     * @since 26.2.0
     */
    daemonName: string;
    /**
     * Registration state of the launch service.
     *
     * @since 26.2.0
     */
    state: LaunchServiceState;
    /**
     * Process id of the running launch service, 0 when not running.
     *
     * @since 26.2.0
     */
    pid: number;
  }

  /**
   * Registers a launch service declared in module.json5 daemonServices. The registration is
   * persisted after IAM administrator approval.
   *
   * @param { LaunchServiceDescriptor } descriptor - Launch service descriptor.
   * @returns { Promise<void> } Promise resolved after the launch service is registered.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 16000050 - Failed to connect to the system service.
   * @throws { BusinessError } 16000211 - Launch service not found.
   * @throws { BusinessError } 16000212 - Launch service is already registered.
   * @throws { BusinessError } 16000215 - Registration was rejected by the administrator.
   * @permission ohos.permission.RIGISTER_DAEMON_SERVICE
   * @since 26.2.0
   */
  function register(descriptor: LaunchServiceDescriptor): Promise<void>;

  /**
   * Unregisters a launch service. A running launch service process is stopped.
   *
   * @param { LaunchServiceDescriptor } descriptor - Launch service descriptor.
   * @returns { Promise<void> } Promise resolved after the launch service is unregistered.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 16000050 - Failed to connect to the system service.
   * @throws { BusinessError } 16000211 - Launch service not found.
   * @throws { BusinessError } 16000215 - Operation was rejected by the administrator.
   * @permission ohos.permission.RIGISTER_DAEMON_SERVICE
   * @since 26.2.0
   */
  function unregister(descriptor: LaunchServiceDescriptor): Promise<void>;

  /**
   * Starts a registered and enabled launch service process.
   *
   * @param { LaunchServiceDescriptor } descriptor - Launch service descriptor.
   * @returns { Promise<void> } Promise resolved after the launch service process is started.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 16000050 - Failed to connect to the system service.
   * @throws { BusinessError } 16000211 - Launch service not found.
   * @throws { BusinessError } 16000213 - Launch service is disabled.
   * @permission ohos.permission.USE_DAEMON_SERVICE
   * @since 26.2.0
   */
  function start(descriptor: LaunchServiceDescriptor): Promise<void>;

  /**
   * Stops a running launch service process.
   *
   * @param { LaunchServiceDescriptor } descriptor - Launch service descriptor.
   * @returns { Promise<void> } Promise resolved after the launch service process is stopped.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 16000050 - Failed to connect to the system service.
   * @throws { BusinessError } 16000211 - Launch service not found.
   * @throws { BusinessError } 16000214 - Launch service is not running.
   * @permission ohos.permission.USE_DAEMON_SERVICE
   * @since 26.2.0
   */
  function stop(descriptor: LaunchServiceDescriptor): Promise<void>;

  /**
   * Queries the registration state and running pid of a launch service.
   *
   * @param { LaunchServiceDescriptor } descriptor - Launch service descriptor.
   * @returns { Promise<LaunchServiceInformation> } Promise resolved with the launch service information.
   * @throws { BusinessError } 16000050 - Failed to connect to the system service.
   * @since 26.2.0
   */
  function getInformation(descriptor: LaunchServiceDescriptor): Promise<LaunchServiceInformation>;
}

export default launchServiceManager;
