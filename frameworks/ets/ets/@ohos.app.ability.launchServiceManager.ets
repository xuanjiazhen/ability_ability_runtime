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

import { BusinessError } from '@ohos.base';
import AsyncCallbackWrapper from './utils/AbilityUtils';
import { LaunchServiceDescriptor as _LaunchServiceDescriptor } from './application/LaunchServiceDescriptor';
import { LaunchServiceObserver as _LaunchServiceObserver,
  LaunchServiceStateData as _LaunchServiceStateData,
  LaunchServiceProcessStateData as _LaunchServiceProcessStateData } from './application/LaunchServiceObserver';

export default namespace launchServiceManager {
  loadLibrary('daemon_service_manager_ani_kit.z')

  /**
   * Describes a launch service declared in module.json5.
   */
  export type LaunchServiceDescriptor = _LaunchServiceDescriptor;

  export enum LaunchServiceState {
    NOT_REGISTERED = 0,
    ENABLED = 1,
    DISABLED = 2
  }

  export enum LaunchServiceProcessState {
    CREATED = 0,
    TERMINATED = 1
  }

  export type LaunchServiceObserver = _LaunchServiceObserver;
  export type LaunchServiceStateData = _LaunchServiceStateData;
  export type LaunchServiceProcessStateData = _LaunchServiceProcessStateData;

  export native function nativeRegisterDaemon(bundleName: string, moduleName: string, serviceName: string,
    userId: int, appIndex: int, callback: AsyncCallbackWrapper<void>): void;

  export native function nativeUnregisterDaemon(bundleName: string, moduleName: string, serviceName: string,
    userId: int, appIndex: int, callback: AsyncCallbackWrapper<void>): void;

  export native function nativeStartDaemon(bundleName: string, moduleName: string, serviceName: string,
    userId: int, appIndex: int, callback: AsyncCallbackWrapper<void>): void;

  export native function nativeStopDaemon(bundleName: string, moduleName: string, serviceName: string,
    userId: int, appIndex: int, callback: AsyncCallbackWrapper<void>): void;

  export native function nativeOnLaunchServiceObserver(observer: LaunchServiceObserver): int;

  export native function nativeOffLaunchServiceObserver(observerId: int,
    callback: AsyncCallbackWrapper<void>): void;

  function getDescriptorUserId(descriptor: LaunchServiceDescriptor): int {
    return descriptor.userId ?? -1;
  }

  function getDescriptorAppIndex(descriptor: LaunchServiceDescriptor): int {
    return descriptor.appIndex ?? 0;
  }

  function isValidDescriptor(descriptor: LaunchServiceDescriptor): boolean {
    const userId = descriptor.userId;
    if (userId === undefined) {
      return true;
    }
    return userId >= 0;
  }

  function createInvalidParameterError(): BusinessError {
    let error: BusinessError = {
      code: 401,
      name: 'BusinessError',
      message: 'Invalid launch service descriptor.'
    };
    return error;
  }

  /**
   * Registers a launch service declared in module.json5 daemonServices. The registration
   * is persisted after IAM administrator approval.
   *
   * @param descriptor Launch service descriptor.
   * @permission ohos.permission.RIGISTER_DAEMON_SERVICE
   */
  export function register(descriptor: LaunchServiceDescriptor): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void,
      reject: (err: BusinessError) => void): void => {
      if (!isValidDescriptor(descriptor)) {
        reject(createInvalidParameterError());
        return;
      }
      let callback = new AsyncCallbackWrapper<void>((err: BusinessError | null): void => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        launchServiceManager.nativeRegisterDaemon(descriptor.bundleName, descriptor.moduleName, descriptor.serviceName,
          getDescriptorUserId(descriptor), getDescriptorAppIndex(descriptor), callback);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  /**
   * Unregisters a launch service. A running launch service process is stopped.
   *
   * @param descriptor Launch service descriptor.
   * @permission ohos.permission.RIGISTER_DAEMON_SERVICE
   */
  export function unregister(descriptor: LaunchServiceDescriptor): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void,
      reject: (err: BusinessError) => void): void => {
      if (!isValidDescriptor(descriptor)) {
        reject(createInvalidParameterError());
        return;
      }
      let callback = new AsyncCallbackWrapper<void>((err: BusinessError | null): void => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        launchServiceManager.nativeUnregisterDaemon(descriptor.bundleName, descriptor.moduleName, descriptor.serviceName,
          getDescriptorUserId(descriptor), getDescriptorAppIndex(descriptor), callback);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  /**
   * Starts a registered and enabled launch service process.
   *
   * @param descriptor Launch service descriptor.
   * @permission ohos.permission.USE_DAEMON_SERVICE
   */
  export function start(descriptor: LaunchServiceDescriptor): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void,
      reject: (err: BusinessError) => void): void => {
      if (!isValidDescriptor(descriptor)) {
        reject(createInvalidParameterError());
        return;
      }
      let callback = new AsyncCallbackWrapper<void>((err: BusinessError | null): void => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        launchServiceManager.nativeStartDaemon(descriptor.bundleName, descriptor.moduleName, descriptor.serviceName,
          getDescriptorUserId(descriptor), getDescriptorAppIndex(descriptor), callback);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  /**
   * Stops a running launch service process.
   *
   * @param descriptor Launch service descriptor.
   * @permission ohos.permission.USE_DAEMON_SERVICE
   */
  export function stop(descriptor: LaunchServiceDescriptor): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void,
      reject: (err: BusinessError) => void): void => {
      if (!isValidDescriptor(descriptor)) {
        reject(createInvalidParameterError());
        return;
      }
      let callback = new AsyncCallbackWrapper<void>((err: BusinessError | null): void => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        launchServiceManager.nativeStopDaemon(descriptor.bundleName, descriptor.moduleName, descriptor.serviceName,
          getDescriptorUserId(descriptor), getDescriptorAppIndex(descriptor), callback);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  /**
   * Registers an observer for launch service state and process state changes.
   *
   * @param observer Observer used to listen for launch service state and process state changes.
   * @returns ID of the observer registered.
   * @permission ohos.permission.MANAGE_DAEMON_SERVICE
   */
  export function onLaunchServiceObserver(observer: LaunchServiceObserver): int {
    return launchServiceManager.nativeOnLaunchServiceObserver(observer);
  }

  /**
   * Unregisters the observer used to listen for launch service state and process state changes.
   *
   * @param observerId ID of the observer registered.
   * @permission ohos.permission.MANAGE_DAEMON_SERVICE
   */
  export function offLaunchServiceObserver(observerId: int): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void,
      reject: (err: BusinessError) => void): void => {
      let callback = new AsyncCallbackWrapper<void>((err: BusinessError | null): void => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        launchServiceManager.nativeOffLaunchServiceObserver(observerId, callback);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }
}
