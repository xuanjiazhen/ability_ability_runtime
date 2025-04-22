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
import { ProcessInformation as _ProcessInformation } from 'application.ProcessInformation';
import { RunningMultiAppInfo as _RunningMultiAppInfo } from 'application.RunningMultiAppInfo';
import _AbilityStateData from 'application.AbilityStateData';
import _AppStateData from 'application.AppStateData';
import hilog from '@ohos.hilog';
import { BusinessError, AsyncCallback } from '@ohos.base';
import _ProcessData from 'application.ProcessData';
import _ApplicationStateObserver from 'application.ApplicationStateObserver';
//import _AppForegroundStateObserver from 'application.AppForegroundStateObserver';
//import _AbilityFirstFrameStateObserver from 'application.AbilityFirstFrameStateObserver';
import { AsyncCallbackCommonWrapper } from './appManagerCommon.ets';

export default namespace appManager {

export type ProcessInformation = _ProcessInformation;
export type AbilityStateData = _AbilityStateData;
export type AppStateData = _AppStateData;
export type RunningMultiAppInfo = _RunningMultiAppInfo;
export type ProcessData = _ProcessData;
export type ApplicationStateObserver = _ApplicationStateObserver;
//export type AppForegroundStateObserver = _AppForegroundStateObserver;
//export type AbilityFirstFrameStateObserver = _AbilityFirstFrameStateObserver;

export enum PreloadMode {
  PRESS_DOWN
}

export enum ApplicationState {
  STATE_CREATE,
  STATE_FOREGROUND,
  STATE_ACTIVE,
  STATE_BACKGROUND,
  STATE_DESTROY
}

export native function nativePreloadApplication(bundleName: string, userId: number, mode: PreloadMode, appIndex?: number): void;
export function preloadApplication(bundleName: string, userId: number, mode: PreloadMode, appIndex?: number):Promise<void>{
    let p = new Promise<void>((resolve: (v: undefined) => void, reject: (error: Object) => void):void => {
      let p1 = taskpool.execute(():void => {appManager.nativePreloadApplication(bundleName, userId, mode, appIndex);});
      p1.then((e :NullishType)=>{
          resolve(undefined);
      }, (err:Object): void => {
      });
    });
    return p;
}

export native function nativeGetRunningProcessInformation(callback:AsyncCallbackCommonWrapper<Array<ProcessInformation>>): void;
export function getRunningProcessInformation(callback: AsyncCallback<Array<ProcessInformation>>): void {
   let myCall = new AsyncCallbackCommonWrapper<Array<ProcessInformation>>(callback);
    taskpool.execute((): void => {
      appManager.nativeGetRunningProcessInformation(myCall);
    });
}

export function getRunningProcessInformation(): Promise<Array<ProcessInformation>> {
    let p:Promise<Array<ProcessInformation>> = new Promise<Array<ProcessInformation>>((resolve:(data:undefined)=>void, reject:(err: BusinessError<void>)=>void):void => {
      let myCall = new AsyncCallbackCommonWrapper<Array<ProcessInformation>>((err: BusinessError<void>)=>{
        if (err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        appManager.nativeGetRunningProcessInformation(myCall);
      });
    });
    return p;
}

export native function nativeGetForegroundApplications(callback:AsyncCallbackCommonWrapper<Array<AppStateData>>): void;
export function getForegroundApplications(callback: AsyncCallback<Array<AppStateData>>): void {
    let myCall = new AsyncCallbackCommonWrapper<Array<AppStateData>>(callback);
    taskpool.execute((): void => {
      appManager.nativeGetForegroundApplications(myCall);
    });
}

export function getForegroundApplications(): Promise<Array<AppStateData>> {
    let p:Promise<Array<AppStateData>> = new Promise<Array<AppStateData>>((resolve:(data:undefined)=>void, reject:(err: BusinessError<void>)=>void):void => {
      let myCall = new AsyncCallbackCommonWrapper<Array<AppStateData>>((err: BusinessError<void>)=>{
        if (err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        appManager.nativeGetForegroundApplications(myCall);
      });
    });
    return p;
}

export native function nativeGetRunningMultiAppInfo(bundleName: string, callback:AsyncCallbackCommonWrapper<RunningMultiAppInfo>): void;
export function getRunningMultiAppInfo(bundleName: string): Promise<RunningMultiAppInfo> {
    let p:Promise<RunningMultiAppInfo> = new Promise<RunningMultiAppInfo>((resolve:(data:undefined)=>void, reject:(err: BusinessError<void>)=>void):void => {
      let myCall = new AsyncCallbackCommonWrapper<RunningMultiAppInfo>((err: BusinessError<void>)=>{
        if (err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        appManager.nativeGetRunningMultiAppInfo(bundleName, myCall);
      });
    });
    return p;
}

export native function nativeGetRunningProcessInfoByBundleName(bundleName: string, callback:AsyncCallbackCommonWrapper< Array<ProcessInformation>>):void;
export function getRunningProcessInfoByBundleName(bundleName: string, callback: AsyncCallback<Array<ProcessInformation>>): void  {
    let myCall = new AsyncCallbackCommonWrapper<Array<ProcessInformation>>(callback);
    taskpool.execute((): void => {
      appManager.nativeGetRunningProcessInfoByBundleName(bundleName, myCall);
    });
}

export function getRunningProcessInfoByBundleName(bundleName: string): Promise<Array<ProcessInformation>> {
    let p:Promise<Array<ProcessInformation>> = new Promise<Array<ProcessInformation>>((resolve:(data:undefined)=>void, reject:(err: BusinessError<void>)=>void):void => {
      let myCall = new AsyncCallbackCommonWrapper<Array<ProcessInformation>>((err: BusinessError<void>)=>{
        if (err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        appManager.nativeGetRunningProcessInfoByBundleName(bundleName, myCall);
      });
    });
    return p;
}

export native function nativeGetRunningProcessInfoByBundleNameAndUserId(bundleName: string,
  userId: int, callback:AsyncCallbackCommonWrapper<Array<ProcessInformation>>):void;
export function getRunningProcessInfoByBundleName(bundleName: string, userId: int, callback: AsyncCallback<Array<ProcessInformation>>): void {
    let myCall = new AsyncCallbackCommonWrapper<Array<ProcessInformation>>(callback);
    taskpool.execute((): void => {
      appManager.nativeGetRunningProcessInfoByBundleNameAndUserId(bundleName, userId, myCall);
    });
}

export function getRunningProcessInfoByBundleName(bundleName: string, userId: int): Promise<Array<ProcessInformation>> {
    let p:Promise<Array<ProcessInformation>> = new Promise<Array<ProcessInformation>>((resolve:(data:undefined)=>void, reject:(err: BusinessError<void>)=>void):void => {
      let myCall = new AsyncCallbackCommonWrapper<Array<ProcessInformation>>((err: BusinessError<void>)=>{
        if (err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        appManager.nativeGetRunningProcessInfoByBundleNameAndUserId(bundleName, userId, myCall);
      });
    });
    return p;
}

export native function nativeOn(type: string, observer: ApplicationStateObserver, bundleNameList?: Array<string>) : number;
export native function nativeOn(type: string, observer: ApplicationStateObserver) : number;
//export native function nativeOn(type: string, observer: AppForegroundStateObserver) : number;
//export native function nativeOn(type: string, observer: AbilityFirstFrameStateObserver, bundleName: string) : number;

export function on(type: 'applicationState', observer: ApplicationStateObserver): number {
  return appManager.nativeOn('applicationState', observer);
}

export function on(type: 'applicationState', observer: ApplicationStateObserver, bundleNameList: Array<string>): number {
  return appManager.nativeOn('applicationState', observer, bundleNameList);
}

// export function on(type: 'appForegroundState', observer: AppForegroundStateObserver): void {
//   appManager.nativeOn('appForegroundState', observer);
// }

// export function on(type: 'abilityFirstFrameState', observer: AbilityFirstFrameStateObserver, bundleName: string): void {
//   appManager.nativeOn('abilityFirstFrameState', observer, bundleName);
// }

export native function nativeOff(type:string, observerId: number) : void;
export function off(type: 'applicationState', observerId: number, callback: AsyncCallback<void>): void {
  let p1 = taskpool.execute(():undefined => {appManager.nativeOff(type, observerId); return undefined;});
      p1.then((e :NullishType)=>{
        let resultCode: Int  = 0;
        let error: BusinessError = { code: resultCode.intValue(), data: undefined };
        callback(error, undefined);
        }, (err: Object) : void => {
      });
}

export function off(type: 'applicationState', observerId: number): Promise<void> {
    let p = new Promise<void>((resolve: (v: PromiseLike<void>) => void, reject: (error: Object) => void) => {
      let p1 = taskpool.execute(():undefined => {appManager.nativeOff(type, observerId);  return undefined});
      p1.then((e :NullishType)=>{
      let PromiseLike = e as PromiseLike<void>;
      resolve(PromiseLike);
      }, (err:Object): void => {
      });
  });
  return p;
}

// export native function nativeOff(type:string, observer?: AppForegroundStateObserver) : void;
// export function off(type: 'appForegroundState', observer?: AppForegroundStateObserver): void {
//   appManager.nativeOff(type, observer);
// }

// export native function nativeOff(type:string, observer?: AbilityFirstFrameStateObserver) : void;
// export function off(type: 'abilityFirstFrameState', observer?: AbilityFirstFrameStateObserver): void {
//   appManager.nativeOff(type, observer);
// }

} // namespace appManager
