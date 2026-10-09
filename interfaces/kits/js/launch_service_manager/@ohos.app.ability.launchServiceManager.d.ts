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
 * @file Launch service management.
 * @kit AbilityKit
 */

/**
 * Provides APIs for querying and observing launch services.
 *
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @stagemodelonly
 * @since 26.2.0 dynamic&static
 */
declare namespace launchServiceManager {
  export interface LaunchServiceDescriptor {
    /** Launch service bundle name. */
    bundleName: string;
    /** Launch service module name. */
    moduleName: string;
    /** Launch service name. */
    serviceName: string;
    /** User ID, when a specific user is selected. */
    userId?: int;
    /** Application clone index, when a clone is selected. */
    appIndex?: int;
  }

  /**
   * Enumerates launch service registration states.
   *
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.2.0 dynamic&static
   */
  export enum LaunchServiceState {
    /**
     * The launch service is not registered.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @stagemodelonly
     * @since 26.2.0 dynamic&static
     */
    NOT_REGISTERED = 0,
    /**
     * The launch service is registered and enabled.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @stagemodelonly
     * @since 26.2.0 dynamic&static
     */
    ENABLED = 1,
    /**
     * The launch service is registered but disabled.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @stagemodelonly
     * @since 26.2.0 dynamic&static
     */
    DISABLED = 2
  }

  /**
   * Enumerates launch service process states.
   *
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.2.0 dynamic&static
   */
  export enum LaunchServiceProcessState {
    /**
     * The launch service process has been created.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @stagemodelonly
     * @since 26.2.0 dynamic&static
     */
    CREATED = 0,
    /**
     * The launch service process has been terminated.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @stagemodelonly
     * @since 26.2.0 dynamic&static
     */
    TERMINATED = 1
  }

  /**
   * Describes a launch service state change.
   *
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.2.0 dynamic&static
   */
  export interface LaunchServiceStateData {
    /**
     * The descriptor of the launch service whose state changed.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.2.0 dynamic&static
     */
    descriptor: LaunchServiceDescriptor;
    /**
     * The launch service state after the change.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.2.0 dynamic&static
     */
    state: LaunchServiceState;
  }

  /**
   * Describes a launch service process state change.
   *
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.2.0 dynamic&static
   */
  export interface LaunchServiceProcessStateData {
    /**
     * The descriptor of the launch service whose process state changed.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.2.0 dynamic&static
     */
    descriptor: LaunchServiceDescriptor;
    /**
     * The process ID. Zero indicates that the process is not running.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.2.0 dynamic&static
     */
    pid: int;
    /**
     * The launch service process state after the change.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.2.0 dynamic&static
     */
    state: LaunchServiceProcessState;
  }

  /**
   * Observes launch service state and process state changes.
   *
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.2.0 dynamic&static
   */
  export interface LaunchServiceObserver {
    /**
     * Called when a launch service is registered, unregistered, enabled, or disabled.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.2.0 dynamic&static
     */
    onLaunchServiceStatusChanged(stateData: LaunchServiceStateData): void;
    /**
     * Called when a launch service process is created or terminated.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.2.0 dynamic&static
     */
    onLaunchServiceProcessStateChanged(processStateData: LaunchServiceProcessStateData): void;
  }

  export interface LaunchServiceInformation {
    descriptor: LaunchServiceDescriptor;
    state: LaunchServiceState;
    pid: int;
  }

  /**
   * Obtains information about a launch service owned by the same developer.
   *
   * @permission ohos.permission.USE_DAEMON_SERVICE
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 16000050 - Failed to connect to the system service.
   * @throws { BusinessError } 16002011 - The specified launch service does not exist.
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @stagemodelonly
   * @since 26.2.0 dynamic&static
   */
  function getInformation(descriptor: LaunchServiceDescriptor): Promise<LaunchServiceInformation>;

  /**
   * Obtains information about all launch services.
   *
   * @permission ohos.permission.MANAGE_DAEMON_SERVICE
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Not a system application.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 16000050 - Failed to connect to the system service.
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.2.0 dynamic&static
   */
  function getAllInformations(): Promise<Array<LaunchServiceInformation>>;

  /**
   * Registers an observer for launch service state and process state changes.
   *
   * @permission ohos.permission.MANAGE_DAEMON_SERVICE
   * @throws { BusinessError } 201 - Permission verification failed.
   * @throws { BusinessError } 202 - A non-system application calls a system API.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 16000050 - System service connection failed.
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.2.0 dynamic&static
   */
  function onLaunchServiceObserver(observer: LaunchServiceObserver): int;

  /**
   * Unregisters a launch service observer by its registration ID.
   *
   * @permission ohos.permission.MANAGE_DAEMON_SERVICE
   * @throws { BusinessError } 201 - Permission verification failed.
   * @throws { BusinessError } 202 - A non-system application calls a system API.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 16000050 - System service connection failed.
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.2.0 dynamic&static
   */
  function offLaunchServiceObserver(observerId: int): Promise<void>;
}

export default launchServiceManager;
