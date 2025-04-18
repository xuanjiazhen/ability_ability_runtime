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

import AbilityStateData from 'application.AbilityStateData'
import ProcessData from 'application.ProcessData'
import AppStateData from 'application.AppStateData'
import bundleManager from '@ohos.bundle.bundleManager';
import ApplicationStateObserver from 'application.ApplicationStateObserver'
import AppForegroundStateObserver from 'application.AppForegroundStateObserver';
import {AbilityFirstFrameStateData} from 'application.AbilityFirstFrameStateData';
import {AbilityFirstFrameStateObserver} from 'application.AbilityFirstFrameStateObserver';
import { BusinessError, AsyncCallback } from '@ohos.base';
import {ProcessInformation} from 'application.ProcessInformation';

type ResolveCallback<T> = (data: T) => void;
type RejectCallback = (err: Object) => void;

export interface RunningAppClone {
    appCloneIndex: number;
    uid: number;
    pids: number[];
}

export interface RunningMultiInstanceInfo {
    instanceKey: string;
    uid: number;
    pids: number[];
}

export interface RunningMultiAppInfo {
    bundleName: string;
    mode: number;
    runningAppClones: RunningAppClone[];
    runningMultiIntanceInfos: RunningMultiInstanceInfo[];

}

class RunningAppCloneInner implements RunningAppClone {
    appCloneIndex: number = 0;
    uid: number = 0;
    pids: number[] = [];
}

class RunningMultiInstanceInfoInner implements RunningMultiInstanceInfo {
    instanceKey: string = "";
    uid: number = 0;
    pids: number[] = [];
}

class RunningMultiAppInfoInner implements RunningMultiAppInfo {
    bundleName: string = "";
    mode: number = 0;
    runningAppClones: RunningAppClone[] = [];
    runningMultiIntanceInfos: RunningMultiInstanceInfo[] = [];
}

export default namespace appManager {
    export enum ApplicationState {
        STATE_CREATE = 0,
        STATE_FOREGROUND = 1,
        STATE_ACTIVE = 2,
        STATE_BACKGROUND = 3,
        STATE_DESTROY = 4
    }

    export enum ProcessState {
        STATE_CREATE = 0,
        STATE_FOREGROUND = 1,
        STATE_ACTIVE = 2,
        STATE_BACKGROUND = 3,
        STATE_DESTROY = 4
    }
  
    export enum PreloadMode {
        PRESS_DOWN
    }
  
    export enum KeepAliveAppType {
        ALL = 0,
        THIRD_PARTY = 1,
        SYSTEM = 2
    }

    export enum KeepAliveSetter {
        SYSTEM = 0,
        USER = 1
    }

    export native function nativePreloadApplication(bundleName: string, userId: number, mode: PreloadMode, appIndex?: number): void;
    export native function nativeIsRunningInStabilityTest(): boolean;
    export native function nativeKillProcessWithAccount(bundleName: string, accountId: number): int;
    export native function nativeKillProcessWithAccountEx(bundleName: string, accountId: number,
            clearPageStack: boolean, appIndex?: number): int;
    export native function nativeKillProcessesByBundleName(bundleName: string): int;
    export native function nativeKillProcessesByBundleNameEx(bundleName: string, clearPageStack: boolean, appIndex?: number): int;
    export native function nativeClearUpApplicationData(bundleName: string): int;
    export native function nativeIsRamConstrainedDevice(): boolean;
    export native function nativeGetForegroundApplications(): Array<AppStateData>;
    export native function nativeOn(observer: ApplicationStateObserver): number;
    export native function nativeOn(observer: ApplicationStateObserver, bundleNameList: Array<string>): number;
    export native function nativeOn(observer: AppForegroundStateObserver): void;
    export native function nativeOn(observer: AbilityFirstFrameStateObserver, bundleName: string): void;
    export native function nativeOff(observerId: number): void;
    export native function nativeOff(type: string): void;
    export native function nativeOff(observer: AppForegroundStateObserver): void;
    export native function nativeOff(observer: AbilityFirstFrameStateObserver): void;
    export native function nativeGetAppMemorySize(): number;
    export native function nativeIsSharedBundleRunning(bundleName: string, versionCode: number): boolean;
    export native function nativeGetRunningProcessInformation(): Array<ProcessInformation>;
    export native function nativeGetRunningProcessInformationByBundleType(
            bundleType: bundleManager.BundleType): Array<ProcessInformation>;
    export native function nativeIsAppRunning(bundleName: string, appInde?: int): boolean;
    export native function nativeGetProcessMemoryByPid(pid: int): int;
    export native function nativeClearUpAppData(bundleName: string, appIndex?: int): int;
    export native function nativeTerminateMission(missionId: int): int;
    export native function nativeIsApplicationRunning(bundleName: string): boolean;
    export native function nativeGetSupportedProcessCachePids(bundleName: string):Array<number>;
    export native function nativeGetRunningMultiAppInfo(bundleName: string): RunningMultiAppInfo;
    export native function nativeGetRunningProcessInfoByBundleName(bundleName: string, userId?: int): ProcessInformation;

    export function preloadApplication(bundleName: string, userId: number, mode: PreloadMode, appIndex?: number): Promise<void> {
        let p = new Promise<void>((resolve: (v: undefined) => void, reject: (error: Object) => void):void => {
            let p1 = taskpool.execute(():void => {appManager.nativePreloadApplication(bundleName, userId, mode, appIndex);});
            p1.then((e :NullishType)=> {
                resolve(undefined);
            }, (err:Object): void => {
            });
        });
        return p;
    }

    export function isRunningInStabilityTest(callback: AsyncCallback<boolean>): void {
        let p1 = taskpool.execute(():boolean => { appManager.nativeIsRunningInStabilityTest(); });
        p1.then((e : NullishType)=> {
            let resultCode: boolean  = e as boolean;
                let r: BusinessError = { code: 1, data: undefined };
                callback(r, resultCode);
                console.log("isRunningInStabilityTest callback success: ", resultCode)
            }, (err:Object): void => {
                console.log("isRunningInStabilityTest callback fail", err)
            });
        console.log("isRunningInStabilityTest callback end")
    }

    export function isRunningInStabilityTest():Promise<boolean> {
        let p = new Promise<boolean>((resolve: ResolveCallback<boolean>, reject: RejectCallback) => {
            let p1 = taskpool.execute(():boolean => { appManager.nativeIsRunningInStabilityTest(); });
            p1.then((e :NullishType)=> {
            let ret : boolean = e as boolean;
            resolve(ret);
            console.log("isRunningInStabilityTest Promise success: ", ret)
            }, (err:Object): void => {
              reject(err)
              console.log("isRunningInStabilityTest Promise fail", err)
            });
        });
        console.log("isRunningInStabilityTest Promise end")
        return p;
    }

    export function killProcessWithAccount(bundleName: string, accountId: number):Promise<void> {
        let p = new Promise<void>((resolve: ResolveCallback<void>, reject: RejectCallback) => {
            let p1 = taskpool.execute(():int => { appManager.nativeKillProcessWithAccount(bundleName, accountId); });
            p1.then((e :NullishType)=> {
            resolve(undefined);
            console.log("killProcessWithAccount Promise success")
            }, (err:Object): void => {
              reject(err)
              console.log("killProcessWithAccount Promise fail")
            });
        });
        console.log("killProcessWithAccount Promise end")
        return p;
    }

    export function killProcessWithAccount(bundleName: string, accountId: number, callback: AsyncCallback<void>):void {
        let p1 = taskpool.execute(():int => { appManager.nativeKillProcessWithAccount(bundleName, accountId); });
            p1.then((e :NullishType)=> {
                let returnCode : Int = e as Int
                let r: BusinessError = { code: returnCode.intValue(), data: undefined };
                callback(r, undefined);
                console.log("killProcessWithAccount callback success: ")
            }, (err:Object): void => {
            });
        console.log("killProcessWithAccount callback end")
    }

    export function killProcessWithAccount(bundleName: string, accountId: number,
            clearPageStack: boolean, appIndex?: number):Promise<void> {
        let p = new Promise<void>((resolve: ResolveCallback<void>, reject: (error: Object) => void) => {
            let p1 = taskpool.execute(():int => { appManager.nativeKillProcessWithAccountEx(bundleName, accountId, clearPageStack, appIndex); });
            p1.then((e :NullishType)=> {
                resolve(undefined);
                console.log("killProcessWithAccount Promise success")
            }, (err:Object): void => {
                reject(err)
                console.log("killProcessWithAccount Promise fail")
            });
        });
        console.log("killProcessWithAccount Promise end")
        return p;
    }

    export function killProcessesByBundleName(bundleName: string):Promise<void> {
        let p = new Promise<void>((resolve: ResolveCallback<void>, reject: RejectCallback) => {
          let p1 = taskpool.execute(():int => { appManager.nativeKillProcessesByBundleName(bundleName); });
          p1.then((e :NullishType)=> {
          resolve(undefined);
          console.log("killProcessesByBundleName Promise success")
          }, (err:Object): void => {
              reject(err)
              console.log("killProcessesByBundleName Promise fail")
          });
      });
      console.log("killProcessWithAccount Promise end")
      return p;
    }

    export function killProcessesByBundleName(bundleName: string, clearPageStack: boolean,
            appIndex?: number): Promise<void> {
        let p = new Promise<void>((resolve: ResolveCallback<void>, reject: RejectCallback) => {
            let p1 = taskpool.execute(():int => { appManager.nativeKillProcessesByBundleNameEx(bundleName, clearPageStack, appIndex); });
            p1.then((e :NullishType)=> {
                resolve(undefined);
                console.log("killProcessesByBundleName Promise success: ")
            }, (err:Object): void => {
                reject(err)
                console.log("killProcessesByBundleName Promise fail")
            });
        });
        console.log("killProcessWithAccount Promise end")
        return p;
    }

    export function killProcessesByBundleName(bundleName: string, callback: AsyncCallback<void>): void {
        let p1 = taskpool.execute(():int => { appManager.nativeKillProcessesByBundleName(bundleName); });
        p1.then((e :NullishType)=>{
            let returnCode : Int = e as Int
            let r: BusinessError = { code: returnCode.intValue(), data: undefined };
            callback(r, undefined);
            console.log("killProcessesByBundleName callback success: ", returnCode)
        }, (err:Object): void => {
            console.log("killProcessesByBundleName callback fail", err)
        });
        console.log("killProcessesByBundleName callback end")
    }

    export function clearUpApplicationData(bundleName: string): Promise<void> {
        let p = new Promise<void>((resolve: ResolveCallback<void>, reject: RejectCallback) => {
            let p1 = taskpool.execute(():int => { appManager.nativeClearUpApplicationData(bundleName); });
            p1.then((e :NullishType) => {
                let ret : int = e as int;
                resolve(undefined);
                console.log("clearUpApplicationData Promise success: ", ret)
            }, (err:Object): void => {
                reject(err)
                console.log("clearUpApplicationData Promise success: ")
            });
        });
        console.log("clearUpApplicationData Promise end")
        return p;
    }

    export function clearUpApplicationData(bundleName: string, callback: AsyncCallback<void>): void {
        let p1 = taskpool.execute(():int => { appManager.nativeClearUpApplicationData(bundleName); });
            p1.then((e :NullishType) => {
                let returnCode : Int = e as Int
                let r: BusinessError = { code: returnCode.intValue(), data: undefined };
                callback(r, undefined);
                console.log("clearUpApplicationData callback success")
            }, (err:Object): void => {
                console.log("clearUpApplicationData callback fail")
            });
        console.log("clearUpApplicationData callback end")
    }

    export function isRamConstrainedDevice(callback: AsyncCallback<boolean>): void {
        let p1 = taskpool.execute(():int => { appManager.nativeIsRamConstrainedDevice(); });
            p1.then((e :NullishType) => {
                let resultCode: boolean  = e as boolean;
                let r: BusinessError = { code: 1, data: undefined };
                callback(r, resultCode);
                console.log("isRamConstrainedDevice callback success: ", resultCode)
            }, (err:Object): void => {
                console.log("isRamConstrainedDevice callback fail")
            });
        console.log("isRamConstrainedDevice callback end")
    }

    export function isRamConstrainedDevice(): Promise<boolean> {
        let p = new Promise<boolean>((resolve: ResolveCallback<boolean>, reject: RejectCallback) => {
            let p1 = taskpool.execute(():int => { appManager.nativeIsRamConstrainedDevice(); });
            p1.then((e :NullishType)=>{
            let ret : boolean = e as boolean;
            resolve(ret);
            console.log("isRamConstrainedDevice Promise success: ", ret)
            }, (err:Object): void => {
              reject(err)
              console.log("isRamConstrainedDevice Promise fail")
            });
        });
        console.log("isRamConstrainedDevice Promise end")
        return p;
    }

    export function getForegroundApplications(callback: AsyncCallback<Array<AppStateData>>):void{
        let p1 = taskpool.execute(():Object => { appManager.nativeGetForegroundApplications(); });
        p1.then((e :NullishType) => {
            let data : Array<AppStateData> = e as Array<AppStateData>
            let err : BusinessError = {code: 1, data: undefined}
            callback(err, data)
            console.log("getForegroundApplications callback success")
        }, (err:Object): void => {
            console.log("getForegroundApplications callback fail: ")
        });
        console.log("getForegroundApplications callback end")
    }

    export function getForegroundApplications(): Promise<Array<AppStateData>> {
        let p = new Promise<Array<AppStateData>>((resolve: ResolveCallback<Array<AppStateData>>,
                reject: RejectCallback) => {
            let p1 = taskpool.execute(():Object => { appManager.nativeGetForegroundApplications(); });
            p1.then((e : NullishType) => {
                let data : Array<AppStateData> = e as Array<AppStateData>;
                if(data.length === 0) {
                    console.log("getForegroundApplications Promise data length is null")
                }
                resolve(data);
                console.log("getForegroundApplications Promise success")
            }, (err:Object): void => {
                reject(err)
                console.log("getForegroundApplications Promise fail")
            });
        });
        console.log("getForegroundApplications Promise end")
        return p;
    }

    export function on(type: 'applicationState', observer: ApplicationStateObserver): number {
        return nativeOn(observer);
    }

    export function on(type: 'applicationState', observer: ApplicationStateObserver,
            bundleNameList: Array<string>): number {
        return nativeOn(observer, bundleNameList);
    }

    export function on(type: 'appForegroundState',
            observer: AppForegroundStateObserver): void {
        return nativeOn(observer);
    }

    export function on(type: 'abilityFirstFrameState', observer: AbilityFirstFrameStateObserver,
            bundleName: string): void {
        return nativeOn(observer, bundleName);
    }

    export function off(type: 'applicationState', observerId: number, callback: AsyncCallback<void>): void {
        let p1 = launch nativeOff(observerId);
        p1.then((e: NullishType): void => {
            let err: BusinessError = { code: 0, data: undefined };
            callback(err, undefined);
        }, (error: Object): void => {
        })
    }

    export function off(type: 'applicationState', observerId: number): Promise<void> {
        let pPromise = new Promise<void> ((resolve: ResolveCallback<void>, reject: RejectCallback): void => {
            let p1 = launch nativeOff(observerId);
            p1.then((e: NullishType): void => {
                resolve(undefined);
            }, (error: Object): void => {
                reject(error);
            });
        });
        return pPromise;
    }

    export function off(type: 'appForegroundState', observer?: AppForegroundStateObserver): void {
        if (observer === undefined) {
            nativeOff(type);
        } else {
            nativeOff(observer);
        }
    }

    export function off(type: 'abilityFirstFrameState', observer?: AbilityFirstFrameStateObserver): void {
        if (observer === undefined) {
            nativeOff(type);
        } else {
            nativeOff(observer);
        }
    }

    export function getRunningProcessInformation(): Promise<Array<ProcessInformation>> {
        let pPromise = new Promise<Array<ProcessInformation>>((resolve: ResolveCallback<Array<ProcessInformation>>,
                reject: RejectCallback): void => {
            let p = taskpool.execute((): Array<ProcessInformation> => { return appManager.nativeGetRunningProcessInformation(); });
            p.then((e: NullishType): void => {
              let info: Array<ProcessInformation> = e as Array<ProcessInformation>;
              resolve(info);
            }, (error: Object): void => {
              reject(error);
            });
        });
        return pPromise;
    }

    export function getRunningProcessInformationByBundleType(
            bundleType: bundleManager.BundleType): Promise<Array<ProcessInformation>> {
        let pPromise = new Promise<Array<ProcessInformation>>(
                (resolve: ResolveCallback<Array<ProcessInformation>>, reject: RejectCallback): void => {
            let p = taskpool.execute((): Array<ProcessInformation> => {
                return appManager.nativeGetRunningProcessInformationByBundleType(bundleType);
            });
            p.then((e: NullishType): void => {
                let data: Array<ProcessInformation> = e as Array<ProcessInformation>;
                resolve(data);
            }, (error: Object): void => {
                reject(error);
            });
        })
        return pPromise;
    }

    export function getRunningProcessInformation(callback: AsyncCallback<Array<ProcessInformation>>): void {
        let p = taskpool.execute((): Array<ProcessInformation> => {
            return appManager.nativeGetRunningProcessInformation();
        });
        p.then((e: NullishType): void => {
            let data: Array<ProcessInformation> = e as Array<ProcessInformation>;
            let err: BusinessError = {code: 0, data: undefined};
            callback(err, data);
        }, (error: Object): void => {
        });
    }

    export function getAppMemorySize(): Promise<number> {
        let pPromise = new Promise<number>(
                (resolve: ResolveCallback<number>, reject: RejectCallback): void => {
            let p = taskpool.execute((): number => { return appManager.nativeGetAppMemorySize(); });
            p.then((data: number): void => {
                resolve(data);
            }, (error: Object): void => {
                reject(error);
            });
        });
        return pPromise;
    }

    export function getAppMemorySize(callback: AsyncCallback<number>): void {
        let p = taskpool.execute((): number => { return appManager.nativeGetAppMemorySize(); });
        p.then((data: number): void => {
            let err: BusinessError = {code: 0, data: undefined};
            callback(err, data);
        }, (error: Object): void => {
        });
    }

    export function isSharedBundleRunning(bundleName: string, versionCode: number): Promise<boolean> {
        let pPromise = new Promise<boolean>(
                (resolve: ResolveCallback<boolean>, reject: RejectCallback): void => {
            let p = taskpool.execute((): boolean => { return appManager.nativeIsSharedBundleRunning(bundleName, versionCode); });
            p.then((data: boolean): void => {
                resolve(data);
            }, (error: Object): void => {
                reject(error);
            });
        });
        return pPromise;
    }

    export function isSharedBundleRunning(
            bundleName: string, versionCode: number, callback: AsyncCallback<boolean>): void {
        let p = taskpool.execute((): boolean => { return appManager.nativeIsSharedBundleRunning(bundleName, versionCode); });
        p.then((data: boolean): void => {
            let err: BusinessError = {code: 0, data: undefined};
            callback(err, data);
        }, (error: Object): void => {
        });
    }

    export function isAppRunning(bundleName: string): Promise<boolean>
    {
        console.log("isAppRunning Promise callback enter");
        let p  = new Promise<boolean>((resolve: (v: boolean) => void, reject: (error: Object) => void) => {
            console.log("isAppRunning Promise launch nativeIsAppRunning");
            let p1 = launch nativeIsAppRunning(bundleName);
            p1.then((e :NullishType)=>{
                let r : boolean =  e as boolean;
                console.log("isAppRunning Promise Promise success r ", r);
            }, ( error: Object) : void => {
                console.log("isAppRunning Promise Promise fail. ");
            });
        });
        console.log("etsIsAppRunning Promise end");
        return p;
    }

    export function isAppRunning(bundleName: string, appIndex: int): Promise<boolean>
    {
        console.log("isAppRunning appIndex Promise enter");
        let p  = new Promise<boolean>((resolve: (v: boolean) => void, reject: (error: Object) => void) => {
            console.log("isAppRunning appIndex Promise launch nativeIsAppRunning");
            let p1 = launch nativeIsAppRunning(bundleName, appIndex);
            p1.then((e :NullishType)=>{
                let r : boolean = e as boolean;
                console.log("isAppRunning Promise appIndex promise success. r ", r);
            }, ( error: Object) : void => {
                console.log("isAppRunning Promise appIndex promise fail. end");
            });
        });
        console.log("etsIsAppRunning Promise appIndex end");
        return p;
    }

    export function getProcessMemoryByPid(pid: int, callback: AsyncCallback<int>): void
    {
        console.log("[callback] getProcessMemoryByPid enter");
        let p1 = launch nativeGetProcessMemoryByPid(pid);
        p1.then((e: int): void => {
            let data : int = e as int;
            let err: BusinessError<int> = { code: 0, data: data };
            //[callback] getProcessMemoryByPid success. data is  0
            //[callback] getProcessMemoryByPid success. err is  @ohos.app.ability.appManager.BusinessError {code: 0, data: 0}
            console.log("[callback] getProcessMemoryByPid success. data is ", data);
            console.log("[callback] getProcessMemoryByPid success. err is ", err);
            callback(err, data);
        }, (error: Object): void => {
            console.log("[callback] getProcessMemoryByPid fail.");
        });
    }

    export function getProcessMemoryByPid(pid: int): Promise<int>
    {
        console.log("[Promise] getProcessMemoryByPid enter");
        let p = new Promise<int>((resolve: (v: int) => void, reject: (error: Object) => void) => {
            console.log("[Promise] getProcessMemoryByPid launch nativeGetProcessMemoryByPid");
            let p1 = launch nativeGetProcessMemoryByPid(pid);
            p1.then((e: int) => {
                let ret : int = e as int
                //[Promise] getProcessMemoryByPid success. ret  0
                console.log("[Promise] getProcessMemoryByPid success. ret ", ret);
                resolve(ret);
            }, (error: Object): void => {
                console.log("[Promise] getProcessMemoryByPid fail.");
                reject(error);
            });
        });
        console.log("getProcessMemoryByPid Promise end");
        return p;
    }

    export function clearUpAppData(bundleName: string, appIndex?: int): Promise<void>
    {
        console.log("clearUpAppData Promise enter");
        let p = new Promise<void>((resolve: ResolveCallback<void>, reject: RejectCallback) => {
            console.log("clearUpAppData Promise launch nativeClearUpAppData");
            const p = launch nativeClearUpAppData(bundleName, appIndex);
            console.log("clearUpAppData Promise launch nativeClearUpAppData end");
            p.then((e :NullishType)=>{
                let ret : int = e as int
                console.log("clearUpAppData Promise success: ret ", ret)
                resolve(undefined);
            }, (err:Object): void => {
                console.log("clearUpAppData Promise fail: err ", err)
                reject(err)
            });
        });
        console.log("clearUpAppData Promise end")
        return p;
    }

    export function terminateMission(missionId: int): Promise<void>
    {
        console.log("terminateMission Promise enter");
        let p = new Promise<void>((resolve: ResolveCallback<void>, reject: RejectCallback) => {
            console.log("terminateMission Promise launch nativeTerminateMission");
            let p2 = launch nativeTerminateMission(missionId);
            p2.then((e :NullishType) => {
                let ret : int = e as int  
                resolve(undefined);
                console.log("terminateMission Promise success: ", ret)
            }, (err:Object): void => {
                   console.log("terminateMission Promise fail: ", err)
                   reject(err)
            });
        });
        console.log("terminateMission Promis end");
        return p;
    }

    export function isApplicationRunning(bundleName: string, callback: AsyncCallback<boolean>): void
    {
        console.log("[callback] isApplicationRunning enter");
        let p = launch nativeIsApplicationRunning(bundleName);
        p.then((e: NullishType) :void =>{
            let resultCode: boolean  = e as boolean;
            let r: BusinessError<boolean> = { code: 1, data: resultCode };
            callback(r, resultCode);
            console.log("[callback] isApplicationRunning success. resultCode  is ", resultCode);
        }, (error: Object) :void => {
            console.log("[callback] isApplicationRunning fail. error ", error);
        })
        console.log("[callback] isApplicationRunning end");
    }

    export function isApplicationRunning(bundleName: string): Promise<boolean>
    {
        console.log("[Promise] isApplicationRunning enter");
        let pPromise = new Promise<boolean>((resolve: (v: boolean) => void, reject: (error: Object) => void) => {
            console.log("[Promise] isApplicationRunning call nativeIsApplicationRunning");
            let p = launch nativeIsApplicationRunning(bundleName);
            p.then((e :NullishType)=>{
                let ret : boolean = e as boolean;
                console.log("[Promise] isApplicationRunning success, ret ", ret);
                resolve(ret);
            }, (error: Object) : void => {
                console.log("[Promise] isApplicationRunning fail. error", error);
                reject(error)
            });
        });
        console.log("[Promise] isApplicationRunning end");
        return pPromise;
    }

    export function getRunningMultiAppInfo(bundleName: string): Promise<RunningMultiAppInfo>
    {
        console.log("[Promise] getRunningMultiAppInfo enter");
        let pPromise = new Promise<RunningMultiAppInfo>((resolve: ResolveCallback<RunningMultiAppInfo>,
                reject: RejectCallback): void => {
            let p = launch nativeGetRunningMultiAppInfo(bundleName);
            p.then((e: NullishType): void => {
              console.log("[Promise] launch getRunningMultiAppInfo then. enter");
              let info: RunningMultiAppInfo = e as RunningMultiAppInfo;
              resolve(info);
            }, (error: Object): void => {
              console.log("[Promise] launch getRunningMultiAppInfo error", error);
              reject(error);
            });
        });
        return pPromise;
    }

    export function getSupportedProcessCachePids(bundleName: string): Promise<Array<number>>
    {
        console.log("[Promise] getSupportedProcessCachePids enter");
        let pPromise = new Promise<Array<number>>((resolve: ResolveCallback<Array<number>>,
                reject: RejectCallback): void => {
            let p = launch nativeGetSupportedProcessCachePids(bundleName);
            p.then((e: NullishType): void => {
              console.log("[Promise] launch getSupportedProcessCachePids then. enter");
              let info: Array<number> = e as Array<number>;
              resolve(info);
            }, (error: Object): void => {
              console.log("[Promise] launch getSupportedProcessCachePids error", error);
              reject(error);
            });
        });
        return pPromise;
    }

    export function getRunningProcessInfoByBundleName(bundleName: string, userId?: int, callback: AsyncCallback<ProcessInformation>): void
    {
        console.log("[callback] getRunningProcessInfoByBundleName userId enter");
        let p = launch nativeGetRunningProcessInfoByBundleName(bundleName, userId);
        p.then((e: NullishType): void => {
            console.log("[callback] launch getRunningProcessInfoByBundleName userId then. enter");
            let data: ProcessInformation = e as ProcessInformation;
            let err: BusinessError<ProcessInformation> = {code: 0, data: data};
            callback(err, data);
        }, (error: Object): void => {
            console.log("[callback] launch getRunningProcessInfoByBundleName userId error");
        })
    }

    export function getRunningProcessInfoByBundleName(bundleName: string, userId: int): Promise<ProcessInformation>
    {
        console.log("[Promise] getRunningProcessInfoByBundleName userId enter");
        let pPromise = new Promise<ProcessInformation>((resolve: ResolveCallback<ProcessInformation>,
                reject: RejectCallback): void => {
            let p = launch nativeGetRunningProcessInfoByBundleName(bundleName, userId);
            p.then((e: NullishType): void => {
              console.log("[Promise] launch getRunningProcessInfoByBundleName userId then. enter");
              let info: ProcessInformation = e as ProcessInformation;
              resolve(info);
            }, (error: Object): void => {
              console.log("[Promise] launch getRunningProcessInfoByBundleName userId error", error);
              reject(error);
            });
        });
        return pPromise;
    }

    export function getRunningProcessInfoByBundleName(bundleName: string, callback: AsyncCallback<ProcessInformation>): void
    {
        console.log("[callback] getRunningProcessInfoByBundleName enter");
        let p = launch nativeGetRunningProcessInfoByBundleName(bundleName);
        p.then((e: NullishType): void => {
            console.log("[callback] launch getRunningProcessInfoByBundleName then. enter");
            let data: ProcessInformation = e as ProcessInformation;
            let err: BusinessError<ProcessInformation> = {code: 0, data: data};
            callback(err, data);
        }, (error: Object): void => {
            console.log("[callback] launch getRunningProcessInfoByBundleName error");
        })
    }

    export function getRunningProcessInfoByBundleName(bundleName: string): Promise<ProcessInformation>
    {
        console.log("[Promise] getRunningProcessInfoByBundleName enter");
        let pPromise = new Promise<ProcessInformation>((resolve: ResolveCallback<ProcessInformation>,
                reject: RejectCallback): void => {
            let p = launch nativeGetRunningProcessInfoByBundleName(bundleName);
            p.then((e: NullishType): void => {
              console.log("[Promise] launch getRunningProcessInfoByBundleName then. enter");
              let info: ProcessInformation = e as ProcessInformation;
              resolve(info);
            }, (error: Object): void => {
              console.log("[Promise] launch getRunningProcessInfoByBundleName error", error);
              reject(error);
            });
        });
        return pPromise;
    }
}