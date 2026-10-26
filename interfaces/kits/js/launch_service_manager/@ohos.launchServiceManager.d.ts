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
     * Bundle name of the application that owns the launch service.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @stagemodelonly
     * @since 26.2.0 dynamic&static
     */
    bundleName: string;
    /**
     * Module name where the launch service is declared.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @stagemodelonly
     * @since 26.2.0 dynamic&static
     */
    moduleName: string;
    /**
     * Name of the launch service.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @stagemodelonly
     * @since 26.2.0 dynamic&static
     */
    serviceName: string;
    /**
     * User ID. If this parameter is not specified, the current user is used by default.
     * The value should be an integer.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @stagemodelonly
     * @since 26.2.0 dynamic&static
     */
    userId?: number;
    /**
     * Index of an application clone. The default value **0** indicates the main application.
     * The value should be an integer.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @stagemodelonly
     * @since 26.2.0 dynamic&static
     */
    appIndex?: number;
  }

  /**
   * Enumerates launch service states.
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
   * Enumerates launch service process states.
   *
   * @since 26.2.0
   */
  export enum LaunchServiceProcessState {
    /**
     * The launch service process has been created.
     *
     * @since 26.2.0
     */
    CREATED = 0,
    /**
     * The launch service process has been terminated.
     *
     * @since 26.2.0
     */
    TERMINATED = 1
  }

  /**
   * Describes the launch service state change data.
   *
   * @since 26.2.0
   */
  export interface LaunchServiceStateData {
    /**
     * Descriptor of the launch service.
     *
     * @since 26.2.0
     */
    descriptor: LaunchServiceDescriptor;
    /**
     * Current launch service state.
     *
     * @since 26.2.0
     */
    state: LaunchServiceState;
  }

  /**
   * Describes the launch service process state change data.
   *
   * @since 26.2.0
   */
  export interface LaunchServiceProcessStateData {
    /**
     * Descriptor of the launch service.
     *
     * @since 26.2.0
     */
    descriptor: LaunchServiceDescriptor;
    /**
     * Process ID of the launch service.
     *
     * @since 26.2.0
     */
    pid: number;
    /**
     * Current launch service process state.
     *
     * @since 26.2.0
     */
    state: LaunchServiceProcessState;
  }

  /**
   * Observer for launch service state and process state changes.
   *
   * @since 26.2.0
   */
  export interface LaunchServiceObserver {
    /**
     * Called when the launch service state changes.
     *
     * @param { LaunchServiceStateData } stateData - State change data.
     * @since 26.2.0
     */
    onLaunchServiceStatusChanged(stateData: LaunchServiceStateData): void;
    /**
     * Called when the launch service process state changes.
     *
     * @param { LaunchServiceProcessStateData } processStateData - Process state change data.
     * @since 26.2.0
     */
    onLaunchServiceProcessStateChanged(processStateData: LaunchServiceProcessStateData): void;
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
   * Registers an observer for launch service state and process state changes.
   *
   * @param { LaunchServiceObserver } observer - Observer used to listen for state changes.
   * @returns { number } ID of the observer registered.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Permission denied. A non-system application calls a system API.
   * @throws { BusinessError } 16000050 - Failed to connect to the system service.
   * @permission ohos.permission.MANAGE_DAEMON_SERVICE
   * @since 26.2.0
   */
  function onLaunchServiceObserver(observer: LaunchServiceObserver): number;

  /**
   * Unregisters the observer used to listen for launch service state and process state changes.
   *
   * @param { number } observerId - ID of the observer registered.
   * @returns { Promise<void> } Promise resolved after the observer is unregistered.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Permission denied. A non-system application calls a system API.
   * @throws { BusinessError } 16000050 - Failed to connect to the system service.
   * @permission ohos.permission.MANAGE_DAEMON_SERVICE
   * @since 26.2.0
   */
  function offLaunchServiceObserver(observerId: number): Promise<void>;
}

export default launchServiceManager;
