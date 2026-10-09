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

export default namespace daemonServiceManager {
  loadLibrary('daemon_service_manager_ani_kit.z')

  export native function nativeRegisterDaemon(bundleName: string, daemonName: string,
    callback: AsyncCallbackWrapper<void>): void;

  export native function nativeUnregisterDaemon(bundleName: string, daemonName: string,
    callback: AsyncCallbackWrapper<void>): void;

  export native function nativeStartDaemon(bundleName: string, daemonName: string,
    callback: AsyncCallbackWrapper<void>): void;

  export native function nativeStopDaemon(bundleName: string, daemonName: string,
    callback: AsyncCallbackWrapper<void>): void;

  /**
   * Registers a daemon declared in module.json5 daemonServices.
   *
   * @permission ohos.permission.MANAGE_DAEMON_SERVICE
   */
  export function register(bundleName: string, daemonName: string): Promise<void> {
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
        daemonServiceManager.nativeRegisterDaemon(bundleName, daemonName, callback);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  /**
   * Unregisters a previously registered daemon.
   *
   * @permission ohos.permission.MANAGE_DAEMON_SERVICE
   */
  export function unregister(bundleName: string, daemonName: string): Promise<void> {
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
        daemonServiceManager.nativeUnregisterDaemon(bundleName, daemonName, callback);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  /**
   * Starts a registered and enabled daemon process.
   *
   * @permission ohos.permission.MANAGE_DAEMON_SERVICE
   */
  export function start(bundleName: string, daemonName: string): Promise<void> {
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
        daemonServiceManager.nativeStartDaemon(bundleName, daemonName, callback);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  /**
   * Stops a running daemon process.
   *
   * @permission ohos.permission.MANAGE_DAEMON_SERVICE
   */
  export function stop(bundleName: string, daemonName: string): Promise<void> {
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
        daemonServiceManager.nativeStopDaemon(bundleName, daemonName, callback);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }
}
