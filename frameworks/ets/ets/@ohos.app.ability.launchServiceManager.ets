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
import { LaunchServiceObserver as _LaunchServiceObserver,
  LaunchServiceStateData as _LaunchServiceStateData,
  LaunchServiceProcessStateData as _LaunchServiceProcessStateData } from './application/LaunchServiceObserver';
import AsyncCallbackWrapper from './utils/AbilityUtils';

export default namespace launchServiceManager {
  loadLibrary('launch_service_manager_ani.z')
  export native function nativeGetInformation(descriptor: LaunchServiceDescriptor,
    callback: AsyncCallbackWrapper<LaunchServiceInformation>): void;
  export native function nativeGetAllInformations(
    callback: AsyncCallbackWrapper<Array<LaunchServiceInformation>>): void;
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
  export type LaunchServiceObserver = _LaunchServiceObserver;
  export type LaunchServiceStateData = _LaunchServiceStateData;
  export type LaunchServiceProcessStateData = _LaunchServiceProcessStateData;

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

  export function getInformation(descriptor: LaunchServiceDescriptor): Promise<LaunchServiceInformation> {
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

  export function getAllInformations(): Promise<Array<LaunchServiceInformation>> {
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
      taskpool.execute((): void => launchServiceManager.nativeGetAllInformations(callback));
    });
  }
}
