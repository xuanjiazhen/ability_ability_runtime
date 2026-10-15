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

/**
 * Runtime implementation of LaunchServiceInformation, instantiated by the native layer.
 */
class LaunchServiceInformationImpl {
  bundleName: string = '';
  daemonName: string = '';
  state: int = 0;
  pid: int = 0;
}

export default namespace launchServiceManager {
  loadLibrary('daemon_service_manager_ani_kit.z')

  /**
   * Describes a launch service declared in module.json5 daemonServices.
   */
  export interface LaunchServiceDescriptor {
    /**
     * Bundle that owns the launch service.
     */
    bundleName: string;
    /**
     * Launch service name declared in module.json5 daemonServices.
     */
    daemonName: string;
  }

  /**
   * Registration state of a launch service.
   */
  export enum LaunchServiceState {
    NOT_REGISTERED = 0,
    ENABLED = 1,
    DISABLED = 2
  }

  /**
   * Query result of a launch service.
   */
  export interface LaunchServiceInformation {
    bundleName: string;
    daemonName: string;
    state: LaunchServiceState;
    pid: number;
  }

  export native function nativeRegisterDaemon(bundleName: string, daemonName: string,
    callback: AsyncCallbackWrapper<void>): void;

  export native function nativeUnregisterDaemon(bundleName: string, daemonName: string,
    callback: AsyncCallbackWrapper<void>): void;

  export native function nativeStartDaemon(bundleName: string, daemonName: string,
    callback: AsyncCallbackWrapper<void>): void;

  export native function nativeStopDaemon(bundleName: string, daemonName: string,
    callback: AsyncCallbackWrapper<void>): void;

  export native function nativeGetInformation(bundleName: string, daemonName: string,
    callback: AsyncCallbackWrapper<LaunchServiceInformation>): void;

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
      let callback = new AsyncCallbackWrapper<void>((err: BusinessError | null): void => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        launchServiceManager.nativeRegisterDaemon(descriptor.bundleName, descriptor.daemonName, callback);
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
      let callback = new AsyncCallbackWrapper<void>((err: BusinessError | null): void => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        launchServiceManager.nativeUnregisterDaemon(descriptor.bundleName, descriptor.daemonName, callback);
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
      let callback = new AsyncCallbackWrapper<void>((err: BusinessError | null): void => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        launchServiceManager.nativeStartDaemon(descriptor.bundleName, descriptor.daemonName, callback);
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
      let callback = new AsyncCallbackWrapper<void>((err: BusinessError | null): void => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        launchServiceManager.nativeStopDaemon(descriptor.bundleName, descriptor.daemonName, callback);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  /**
   * Queries the registration state and running pid of a launch service.
   *
   * @param descriptor Launch service descriptor.
   */
  export function getInformation(descriptor: LaunchServiceDescriptor): Promise<LaunchServiceInformation> {
    return new Promise<LaunchServiceInformation>((resolve: (data: LaunchServiceInformation) => void,
      reject: (err: BusinessError) => void): void => {
      let callback = new AsyncCallbackWrapper<LaunchServiceInformation>(
        (err: BusinessError | null, data: LaunchServiceInformation | undefined): void => {
          if (err == null || err.code == 0) {
            resolve(data as LaunchServiceInformation);
          } else {
            reject(err);
          }
        });
      taskpool.execute((): void => {
        launchServiceManager.nativeGetInformation(descriptor.bundleName, descriptor.daemonName, callback);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }
}
