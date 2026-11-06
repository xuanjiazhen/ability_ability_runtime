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
import { LaunchServiceDescriptor as _LaunchServiceDescriptor } from './application/LaunchServiceDescriptor';
import { LaunchServiceInformation as _LaunchServiceInformation } from './application/LaunchServiceInformation';
import { LaunchServiceConnectCallback as _LaunchServiceConnectCallback } from './application/LaunchServiceConnectCallback';
import { LaunchServiceObserver as _LaunchServiceObserver } from './application/LaunchServiceObserver';
import { LaunchServiceProcessStateData as _LaunchServiceProcessStateData }
  from './application/LaunchServiceProcessStateData';
import { LaunchServiceStateData as _LaunchServiceStateData } from './application/LaunchServiceStateData';
import AsyncCallbackWrapper from './utils/AbilityUtils';
import { AbilityUtils } from './utils/AbilityUtils';

const ERROR_CODE_INVALID_PARAM: int = 401;
const MAX_APP_INDEX: int = 5;

function checkIdentity(bundleName: string, userId?: int, appIndex?: int): void {
  const checkedUserId: int = userId ?? 0;
  const checkedAppIndex: int = appIndex ?? 0;
  if (bundleName.length == 0) {
    throw AbilityUtils.createBusinessError(ERROR_CODE_INVALID_PARAM, 'Parameter error. bundleName must not be empty.');
  }
  if (checkedUserId < 0) {
    throw AbilityUtils.createBusinessError(ERROR_CODE_INVALID_PARAM, 'Parameter error. userId must not be negative.');
  }
  if (checkedAppIndex < 0) {
    throw AbilityUtils.createBusinessError(ERROR_CODE_INVALID_PARAM, 'Parameter error. appIndex must not be negative.');
  }
  if (checkedAppIndex > MAX_APP_INDEX) {
    throw AbilityUtils.createBusinessError(ERROR_CODE_INVALID_PARAM, 'Parameter error. appIndex must not exceed 5.');
  }
}

function invokeLaunchServiceOperation(descriptor: _LaunchServiceDescriptor,
  operation: (descriptor: _LaunchServiceDescriptor, callback: AsyncCallbackWrapper<void>) => void): Promise<void> {
  checkIdentity(descriptor.bundleName, descriptor.userId, descriptor.appIndex);
  return new Promise<void>((resolve: (data: undefined) => void,
    reject: (reason: BusinessError) => void): void => {
    const callback = new AsyncCallbackWrapper<void>((error: BusinessError | null): void => {
      if (error == null || error.code == 0) {
        resolve(undefined);
      } else {
        reject(error);
      }
    });
    taskpool.execute((): void => operation(descriptor, callback));
  });
}

export default namespace launchServiceManager {
  loadLibrary('launch_service_manager_ani.z')
  export native function nativeRegister(descriptor: LaunchServiceDescriptor,
    callback: AsyncCallbackWrapper<void>): void;
  export native function nativeUnregister(descriptor: LaunchServiceDescriptor,
    callback: AsyncCallbackWrapper<void>): void;
  export native function nativeStart(descriptor: LaunchServiceDescriptor,
    callback: AsyncCallbackWrapper<void>): void;
  export native function nativeStop(descriptor: LaunchServiceDescriptor,
    callback: AsyncCallbackWrapper<void>): void;
  export native function nativeGetInformation(descriptor: LaunchServiceDescriptor,
    callback: AsyncCallbackWrapper<LaunchServiceInformation>): void;
  export native function nativeGetAllRegisteredInformation(userId: int,
    callback: AsyncCallbackWrapper<Array<LaunchServiceInformation>>): void;
  export native function nativeSetDaemonEnable(bundleName: string, userId: int, appIndex: int,
    enable: boolean, token: Uint8Array, callback: AsyncCallbackWrapper<void>): void;
  export native function nativeOnLaunchServiceObserver(observer: LaunchServiceObserver): int;
  export native function nativeOffLaunchServiceObserver(observerId: int,
    callback: AsyncCallbackWrapper<void>): void;

  export enum LaunchServiceState {
    NOT_REGISTERED = 0,
    ENABLED = 1,
    DISABLED = 2
  }

  export enum LaunchServiceProcessState {
    CREATED = 0,
    TERMINATED = 1
  }

  export type LaunchServiceDescriptor = _LaunchServiceDescriptor;
  export type LaunchServiceInformation = _LaunchServiceInformation;
  export type LaunchServiceConnectCallback = _LaunchServiceConnectCallback;
  export type LaunchServiceObserver = _LaunchServiceObserver;
  export type LaunchServiceStateData = _LaunchServiceStateData;
  export type LaunchServiceProcessStateData = _LaunchServiceProcessStateData;

  export function register(descriptor: LaunchServiceDescriptor): Promise<void> {
    return invokeLaunchServiceOperation(descriptor, launchServiceManager.nativeRegister);
  }

  export function unregister(descriptor: LaunchServiceDescriptor): Promise<void> {
    return invokeLaunchServiceOperation(descriptor, launchServiceManager.nativeUnregister);
  }

  export function start(descriptor: LaunchServiceDescriptor): Promise<void> {
    return invokeLaunchServiceOperation(descriptor, launchServiceManager.nativeStart);
  }

  export function stop(descriptor: LaunchServiceDescriptor): Promise<void> {
    return invokeLaunchServiceOperation(descriptor, launchServiceManager.nativeStop);
  }

  export function getInformation(descriptor: LaunchServiceDescriptor): Promise<LaunchServiceInformation> {
    checkIdentity(descriptor.bundleName, descriptor.userId, descriptor.appIndex);
    return new Promise<LaunchServiceInformation>((resolve: (value: LaunchServiceInformation) => void,
      reject: (reason: BusinessError) => void): void => {
      const callback = new AsyncCallbackWrapper<LaunchServiceInformation>(
        (error: BusinessError | null, value: LaunchServiceInformation | undefined): void => {
          if (error == null || error.code == 0) {
            resolve(value as LaunchServiceInformation);
          } else {
            reject(error);
          }
        });
      taskpool.execute((): void => launchServiceManager.nativeGetInformation(descriptor, callback));
    });
  }

  export function getAllRegisteredInformation(userId?: int): Promise<Array<LaunchServiceInformation>> {
    const resolvedUserId: int = userId ?? -3;
    if (userId !== undefined && resolvedUserId < 0) {
      throw AbilityUtils.createBusinessError(ERROR_CODE_INVALID_PARAM, 'Parameter error. userId must not be negative.');
    }
    return new Promise<Array<LaunchServiceInformation>>((resolve: (value: Array<LaunchServiceInformation>) => void,
      reject: (reason: BusinessError) => void): void => {
      const callback = new AsyncCallbackWrapper<Array<LaunchServiceInformation>>(
        (error: BusinessError | null, value: Array<LaunchServiceInformation> | undefined): void => {
          if (error == null || error.code == 0) {
            resolve(value as Array<LaunchServiceInformation>);
          } else {
            reject(error);
          }
        });
      taskpool.execute((): void => launchServiceManager.nativeGetAllRegisteredInformation(resolvedUserId, callback));
    });
  }

  export function setDaemonEnable(bundleName: string, userId: int, appIndex: int,
    enable: boolean, token?: Uint8Array): Promise<void> {
    checkIdentity(bundleName, userId, appIndex);
    return new Promise<void>((resolve: (data: undefined) => void,
      reject: (reason: BusinessError) => void): void => {
      const callback = new AsyncCallbackWrapper<void>((error: BusinessError | null): void => {
        if (error == null || error.code == 0) {
          resolve(undefined);
        } else {
          reject(error);
        }
      });
      taskpool.execute((): void =>
        launchServiceManager.nativeSetDaemonEnable(
          bundleName, userId, appIndex, enable, token ?? new Uint8Array(0), callback));
    });
  }

  export function onLaunchServiceObserver(observer: LaunchServiceObserver): int {
    return launchServiceManager.nativeOnLaunchServiceObserver(observer);
  }

  export function offLaunchServiceObserver(observerId: int): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void,
      reject: (reason: BusinessError) => void): void => {
      const callback = new AsyncCallbackWrapper<void>((error: BusinessError | null): void => {
        if (error == null || error.code == 0) {
          resolve(undefined);
        } else {
          reject(error);
        }
      });
      taskpool.execute((): void => launchServiceManager.nativeOffLaunchServiceObserver(observerId, callback));
    });
  }
}
