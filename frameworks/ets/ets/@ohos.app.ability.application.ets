/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

import Context from 'application.Context';
import { BusinessError, AsyncCallback } from '@ohos.base';
import AsyncCallbackWrapper from './utils/AbilityUtils';
import ApplicationContext from 'application.ApplicationContext';

export default namespace application {
  loadLibraryWithPermissionCheck("application_ani.z", "@ohos.app.ability.application")

  export enum AppPreloadType {
    UNSPECIFIED = 0,
    TYPE_CREATE_PROCESS = 1,
    TYPE_CREATE_ABILITY_STAGE = 2,
    TYPE_CREATE_WINDOW_STAGE = 3,
  };

  export native function nativeCreateModuleContext(context: Context, bundleName: string, moduleName: string,
    callback: AsyncCallbackWrapper<Context>): void;

  export native function nativeCreateBundleContext(context: Context, bundleName: string,
    callback: AsyncCallbackWrapper<Context>): void;

  export native function nativeGetApplicationContext(): ApplicationContext;

  export native function nativeDemoteCurrentFromCandidateMasterProcess(callback: AsyncCallbackWrapper<void>): void;

  export native function nativePromoteCurrentToCandidateMasterProcess(isInsertToHead: boolean,
    callback:AsyncCallbackWrapper<void>): void;

  export native function nativeGetAppPreloadType(): AppPreloadType;

  export function createModuleContext(context: Context, moduleName: string): Promise<Context> {
    let p =
      new Promise<Context>((resolve: (data: Context) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<Context>((err: BusinessError | null, data: Context | undefined) => {
          if (err == null || err.code == 0) {
            resolve(data as Context);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          application.nativeCreateModuleContext(context, "", moduleName, myCall);
        });
      });
    return p;
  }

  export function createModuleContext(context: Context, bundleName: string, moduleName: string): Promise<Context> {
    let p =
      new Promise<Context>((resolve: (data: Context) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<Context>((err: BusinessError | null, data: Context | undefined) => {
          if (err == null || err.code == 0) {
            resolve(data as Context);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          application.nativeCreateModuleContext(context, bundleName, moduleName, myCall);
        });
      });
    return p;
  }

  export function createBundleContext(context: Context, bundleName: string): Promise<Context> {
    let p =
      new Promise<Context>((resolve: (data: Context) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<Context>((err: BusinessError | null, data: Context | undefined) => {
          if (err == null || err.code == 0) {
            resolve(data as Context);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          application.nativeCreateBundleContext(context, bundleName, myCall);
        });
      });
    return p;
  }

  export function demoteCurrentFromCandidateMasterProcess(callback: AsyncCallback<void>): void {
    console.log("testTag DemoteCurrentFromCandidateMasterProcess")
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      application.nativeDemoteCurrentFromCandidateMasterProcess(myCall);
    });
  }

  export function demoteCurrentFromCandidateMasterProcess(): Promise<void> {
    console.log("testTag DemoteCurrentFromCandidateMasterProcess")
    let p: Promise<void> =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError | null) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          application.nativeDemoteCurrentFromCandidateMasterProcess(myCall);
        });
      });
    return p;
  }

  export function promoteCurrentToCandidateMasterProcess(isInsertToHead: boolean, callback: AsyncCallback<void>): void {
    console.log("testTag PromoteCurrentToCandidateMasterProcess")
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      application.nativePromoteCurrentToCandidateMasterProcess(isInsertToHead, asyncCall);
    });
  }
  export function promoteCurrentToCandidateMasterProcess(isInsertToHead: boolean): Promise<void> {
    console.log("testTag PromoteCurrentToCandidateMasterProcess")
    return new Promise<void>((resolve: (data: undefined) => void, reject:(err: BusinessError) => void): void => {
      let asyncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        application.nativePromoteCurrentToCandidateMasterProcess(isInsertToHead, asyncCall);
      });
    });
  }

  export function getApplicationContext(): ApplicationContext {
    return nativeGetApplicationContext();
  }

  export function getAppPreloadType(): AppPreloadType {
    return nativeGetAppPreloadType();
  }
}