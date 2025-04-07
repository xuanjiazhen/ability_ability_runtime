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
import AbilityFirstFrameStateObserver from 'application.AbilityFirstFrameStateObserver';

type AsyncCallback<T> = (err: BusinessError<T>, data: T)=> void;
export class BusinessError<T = void> {
    code: number;
    data: T;
  }

type ResolveCallback<T> = (data: T) => void;
type RejectCallback = (err: Object) => void;

export interface ProcessInformation {
    pid: number;
    uid: number;
    processName: string;
    bundleNames: Array<string>;
    state: appManager.ProcessState;
    // bundleType: bundleManager.BundleType;
    appCloneIndex?: number;
}

class ProcessInformationInner implements ProcessInformation {
    pid: number = -1;
    uid: number = -1;
    processName: string = "";
    bundleNames: Array<string> = new Array<string>();
    state: appManager.ProcessState = appManager.ProcessState.STATE_DESTROY;
    // bundleType: bundleManager.BundleType = bundleManager.BundleType.APP;
    appCloneIndex?: number = -1;
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
    export native function nativeKillProcessWithAccount(bundleName: string, accountId: int): int;
    export native function nativeKillProcessWithAccountEx(bundleName: string, accountId: int,
            clearPageStack: boolean, appIndex?: int): int;
    export native function nativeKillProcessesByBundleName(bundleName: string): int;
    export native function nativeKillProcessesByBundleNameEx(bundleName: string, clearPageStack: boolean, appIndex?: int): int;
    export native function nativeClearUpApplicationData(bundleName: string): int;
    export native function nativeIsRamConstrainedDevice(): boolean;
    export native function nativeGetForegroundApplications(): Array<AppStateData>;
    export native function nativeOn(observer: ApplicationStateObserver): int;
    export native function nativeOn(observer: ApplicationStateObserver, bundleNameList: Array<string>): int;
    export native function nativeOn(observer: AppForegroundStateObserver): void;
    export native function nativeOn(observer: AbilityFirstFrameStateObserver, bundleName: string): void;
    export native function nativeOff(observerId: int): void;
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
    export native function nativeClearUpAppData(bundleName: string, appIndex?: int): Promise<int>;
    export native function nativeTerminateMission(missionId: int): Promise<int>;
    export native function nativeIsApplicationRunning(bundleName: string): boolean;
    export native function nativeGetSupportedProcessCachePids(bundleName: string): Promise<Array<number>>;
    export native function nativeGetRunningMultiAppInfo(bundleName: string): ProcessInformation;
    export native function nativeGetRunningProcessInfoByBundleName(bundleName: string, userId?: int): ProcessInformation;
    export native function nativePreloadApplication(bundleName: string, userId: int, mode: PreloadMode, appInde?: int): Promise<int>;


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
        let p1 = launch nativeIsRunningInStabilityTest();
        p1.then((e :NullishType)=> {
            let resultCode: boolean  = e as boolean;
                let r: BusinessError<boolean> = { code: 1, data: resultCode };
                callback(r, resultCode);
                console.log("isRunningInStabilityTest callback success: ", resultCode)
            }, (err:Object): void => {
                console.log("isRunningInStabilityTest callback fail", err)
            });
        console.log("isRunningInStabilityTest callback end")
    }

    export function isRunningInStabilityTest():Promise<boolean> {
        let p = new Promise<boolean>((resolve: ResolveCallback<boolean>, reject: RejectCallback) => {
            let p1 = launch nativeIsRunningInStabilityTest();
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

    export function killProcessWithAccount(bundleName: string, accountId: int):Promise<void> {
        let p = new Promise<void>((resolve: ResolveCallback<void>, reject: RejectCallback) => {
            let p1 = launch nativeKillProcessWithAccount(bundleName, accountId);
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

    export function killProcessWithAccount(bundleName: string, accountId: int, callback: AsyncCallback<void>):void {
        let p1 = launch nativeKillProcessWithAccount(bundleName, accountId);
            p1.then((e :NullishType)=> {
                let returnCode : Int = e as Int
                let r: BusinessError = { code: returnCode.intValue(), data: undefined };
                callback(r, undefined);
                console.log("killProcessWithAccount callback success: ")
            }, (err:Object): void => {
            });
        console.log("killProcessWithAccount callback end")
    }

    export function killProcessWithAccount(bundleName: string, accountId: int,
            clearPageStack: boolean, appIndex?: int):Promise<void> {
        let p = new Promise<void>((resolve: ResolveCallback<void>, reject: (error: Object) => void) => {
            let p1 = launch nativeKillProcessWithAccountEx(bundleName, accountId, clearPageStack, appIndex);
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
            let p1 = launch nativeKillProcessesByBundleName(bundleName);
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
            appIndex?: int): Promise<void> {
        let p = new Promise<void>((resolve: ResolveCallback<void>, reject: RejectCallback) => {
            let p1 = launch nativeKillProcessesByBundleNameEx(bundleName, clearPageStack, appIndex);
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
        let p1 = launch nativeKillProcessesByBundleName(bundleName);
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
            let p1 = launch nativeClearUpApplicationData(bundleName);
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
        let p1 = launch nativeClearUpApplicationData(bundleName);
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
        let p1 = launch nativeIsRamConstrainedDevice();
            p1.then((e :NullishType) => {
                let resultCode: boolean  = e as boolean;
                let r: BusinessError<boolean> = { code: 1, data: resultCode };
                callback(r, resultCode);
                console.log("isRamConstrainedDevice callback success: ", resultCode)
            }, (err:Object): void => {
                console.log("isRamConstrainedDevice callback fail")
            });
        console.log("isRamConstrainedDevice callback end")
    }

    export function isRamConstrainedDevice(): Promise<boolean> {
        let p = new Promise<boolean>((resolve: ResolveCallback<boolean>, reject: RejectCallback) => {
            let p1 = launch nativeIsRamConstrainedDevice();
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
        let p1 = launch nativeGetForegroundApplications();
        p1.then((e :NullishType) => {
            try {
                let data : Array<AppStateData> = e as Array<AppStateData>
                let err : BusinessError<Array<AppStateData>> = {code: 0, data: data}
                callback(err, data)
                console.log("getForegroundApplications callback success")
            } catch (error) {
                console.log("getForegroundApplications callback fail: ", error)
            }

        }, (err:Object): void => {
            console.log("getForegroundApplications callback fail: ")
        });
        console.log("getForegroundApplications callback end")
    }

    export function getForegroundApplications(): Promise<Array<AppStateData>> {
        let p = new Promise<Array<AppStateData>>((resolve: ResolveCallback<Array<AppStateData>>,
                reject: RejectCallback) => {
            let p1 = launch nativeGetForegroundApplications();
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

    export function on(type: 'applicationState', observer: ApplicationStateObserver): int {
        return nativeOn(observer);
    }

    export function on(type: 'applicationState', observer: ApplicationStateObserver,
            bundleNameList: Array<string>): int {
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
        let obsId = observerId as int;
        let p1 = launch nativeOff(obsId);
        p1.then((e: NullishType): void => {
            let err: BusinessError<void> = { code: 0, data: undefined };
            callback(err, undefined);
            console.log("[callback] executeOff suc end");
        }, (error: Object): void => {
            console.log("[callback] executeOff err end");
        })
    }

    export function off(type: 'applicationState', observerId: number): Promise<void> {
        let obsId = observerId as int;
        let pPromise = new Promise<void> ((resolve: ResolveCallback<void>, reject: RejectCallback): void => {
            let p1 = launch nativeOff(obsId);
            p1.then((e: NullishType): void => {
                console.log("[promise] executeOff resolve. Before");
                resolve(undefined);
            }, (error: Object): void => {
                console.log("[promise] executeOff reject Before");
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
            let p = launch nativeGetRunningProcessInformation();
            p.then((e: NullishType): void => {
              console.log("[Promise] launch nativeGetRunningProcessInformation then. enter");
              let info: Array<ProcessInformation> = e as Array<ProcessInformation>;
              resolve(info);
            }, (error: Object): void => {
              console.log("[Promise] launch nativeGetRunningProcessInformation error", error);
              reject(error);
            });
        });
        return pPromise;
    }    

    export function getRunningProcessInformationByBundleType(
            bundleType: bundleManager.BundleType): Promise<Array<ProcessInformation>> {
        let pPromise = new Promise<Array<ProcessInformation>>(
                (resolve: ResolveCallback<Array<ProcessInformation>>, reject: RejectCallback): void => {
            let p = launch nativeGetRunningProcessInformationByBundleType(bundleType);
            p.then((e: NullishType): void => {
                console.log("[Promise] launch nativeGetRunningProcessInformationByBundleType then. enter");
                let data: Array<ProcessInformation> = e as Array<ProcessInformation>;
                resolve(data);
            }, (error: Object): void => {
                console.log("[Promise] launch nativeGetRunningProcessInformationByBundleType error");
                reject(error);
            });
        })
        return pPromise;
    }

    export function getRunningProcessInformation(callback: AsyncCallback<Array<ProcessInformation>>): void {
        let p = launch nativeGetRunningProcessInformation();
        p.then((e: NullishType): void => {
            console.log("[callback] launch nativeGetRunningProcessInformation then. enter");
            let data: Array<ProcessInformation> = e as Array<ProcessInformation>;
            let err: BusinessError<Array<ProcessInformation>> = {code: 0, data: data};
            callback(err, data);
        }, (error: Object): void => {
            console.log("[callback] launch nativeGetRunningProcessInformation error");
        })
    }

    export function getAppMemorySize(): Promise<number> {
        let pPromise = new Promise<number>(
                (resolve: ResolveCallback<number>, reject: RejectCallback): void => {
            let p1 = launch nativeGetAppMemorySize();
            p1.then((data: number): void => {
                console.log("[promise] nativeGetAppMemorySize then. data", data);
                resolve(data);
            }, (error: Object): void => {
                console.log("[promise] nativeGetAppMemorySize reject Before");
                reject(error);
            });
        });
        return pPromise;
    }

    export function getAppMemorySize(callback: AsyncCallback<number>): void {
        let p1 = launch nativeGetAppMemorySize();
        p1.then((e: number): void => {
            let data: number = e as number;
            let err: BusinessError<number> = { code: 0, data: data };
            callback(err, data);
            console.log("[callback] nativeGetAppMemorySize suc end");
        }, (error: Object): void => {
            console.log("[callback] nativeGetAppMemorySize err end");
        })
    }

    export function isSharedBundleRunning(bundleName: string, versionCode: number): Promise<boolean> {
        let pPromise = new Promise<boolean>(
                (resolve: ResolveCallback<boolean>, reject: RejectCallback): void => {
            let p1 = launch nativeIsSharedBundleRunning(bundleName, versionCode);
            p1.then((data: boolean): void => {
                console.log("[promise] nativeIsSharedBundleRunning then. data", data);
                resolve(data);
            }, (error: Object): void => {
                console.log("[promise] nativeIsSharedBundleRunning reject Before");
                reject(error);
            });
        });
        return pPromise;
    }

    export function isSharedBundleRunning(
            bundleName: string, versionCode: number, callback: AsyncCallback<boolean>): void {
        let p1 = launch nativeIsSharedBundleRunning(bundleName, versionCode);
        p1.then((e: NullishType): void => {
            try {
                let data : boolean = e as boolean;
                let err: BusinessError<boolean> = { code: 0, data: data };
                callback(err, data);
                console.log("[callback] nativeIsSharedBundleRunning suc end");
            } catch(error) {
                console.log('[callback] nativeIsSharedBundleRunning faild.', error);
            }
        }, (error: Object): void => {
            console.log("[callback] nativeIsSharedBundleRunning err end");
        })
    }
	export function isAppRunning(bundleName: string): Promise<boolean>
	{
		console.log(" ##### etsIsAppRunning Promise callback_01");
		let p  = new Promise<boolean>((resolve: (v: boolean) => void, reject: (error: Object) => void) => {
		console.log(" ##### etsIsAppRunning Promise  begin");
			let p1 = launch nativeIsAppRunning(bundleName);
			
			console.log(" ##### etsIsAppRunning Promise  end");
			p1.then((e :NullishType)=>{
				let r : boolean =  e as boolean;
				console.log("etsIsAppRunning Promise  promise1 r ", r);
				resolve(r);
			}, ( error: Object) : void => {
				console.log("etsIsAppRunning Promise  promise1 end");
			});
		});
		console.log("etsIsAppRunning Promise return");
		return p;
	}

	export function isAppRunning(bundleName: string, appIndex: int): Promise<boolean>
	{
		console.log(" ##### etsIsAppRunning appIndex Promise callback_01");
		let p  = new Promise<boolean>((resolve: (v: boolean) => void, reject: (error: Object) => void) => {
		console.log(" ##### etsIsAppRunning appIndex Promise  begin");
			let p1 = launch nativeIsAppRunning(bundleName, appIndex);
			
			console.log(" ##### etsIsAppRunning appIndex Promise  end");
			p1.then((e :NullishType)=>{
				let r : boolean =  e as boolean;
				console.log("etsIsAppRunning Promise  appIndex promise1_04  r ", r);
				resolve(r);
			}, ( error: Object) : void => {
				console.log("etsIsAppRunning Promise  appIndex promise1 end");
			});
		});
		console.log("etsIsAppRunning Promise appIndex return");
		return p;
	}

	export function getProcessMemoryByPid(pid: int, callback: AsyncCallback<int>): void
	{
		console.log(" ***** etsGetProcessMemoryByPid callback_01");
		let p1 = launch nativeGetProcessMemoryByPid(pid);
		console.log(" ***** etsGetProcessMemoryByPid callback_02");
		p1.then((e: int): void => {
			console.log(" ***** etsGetProcessMemoryByPid callback_03");
			let err = new BusinessError<int>();
			err.code = 0;
			let data: int = e;

			callback(err, data);
			console.log(" ***** callback etsGetProcessMemoryByPid suc end  data  is ", data);
		}, (error: Object): void => {
			console.log(" ***** callback etsGetProcessMemoryByPid err end");
		});
	}

	export function getProcessMemoryByPid(pid: int): Promise<int>
	{
		console.log(" ##### etsGetProcessMemoryByPid Promise ");
		let p = new Promise<int>((resolve: (v: int) => void, reject: (error: Object) => void) => {
			console.log(" ##### etsGetProcessMemoryByPid Promise  begin");
			let p1 = launch nativeGetProcessMemoryByPid(pid);
			console.log(" ##### etsGetProcessMemoryByPid Promise  end");
			p1.then((e: int) => {
				let r: int = e;
				console.log(" ##### etsGetProcessMemoryByPid Promise  success r ", r);
				resolve(r);
			}, (error: Object): void => {
				console.log(" ##### etsGetProcessMemoryByPid Promise  fail");
				reject(error);
			});
		});
	    console.log(" ##### etsGetProcessMemoryByPid Promise  promise1 return p is ",  p);
	    return p;
	}

    export function clearUpAppData(bundleName: string, appIndex?: int): Promise<void>
    {
        let p = new Promise<void>((resolve: ResolveCallback<void>, reject: RejectCallback) => {
		   const p1 = launch nativeClearUpAppData(bundleName, appIndex);
		   p1.then((e :NullishType)=>{
		       let ret : int = e as int  
		       resolve(undefined);
		       console.log("etsClearUpAppData Promise success: ", ret)
		   }, (err:Object): void => {
		       reject(err)
		       console.log("etsClearUpAppData Promise fail: ")
		   });
        });
        console.log("etsClearUpAppData Promise end")
        return p;
    }

	export function terminateMission(missionId: int): Promise<void>
	{
		console.log("etsTerminateMission Promise_01");
	    let p = new Promise<void>((resolve: ResolveCallback<void>, reject: RejectCallback) => {
			console.log("etsTerminateMission Promise_02");
			let p2 = launch nativeTerminateMission(missionId);
			console.log("etsTerminateMission Promise_03");
			p2.then((e :NullishType) => {
	            let ret : int = e as int  
	            resolve(undefined);
	            console.log("etsTerminateMission Promise succ: ", ret)
	        }, (err:Object): void => {
	            reject(err)
	            console.log("etsTerminateMission Promise err: ")
	        });
		});
	    console.log("etsTerminateMission Promis return");
	    return p;
	}

	export function isApplicationRunning(bundleName: string, callback: AsyncCallback<boolean>): void
	{
		console.log("etsIsApplicationRunning callback_01");
		let p1 = launch nativeIsApplicationRunning(bundleName);
		console.log("etsIsApplicationRunning callback_02");
		p1.then((e: NullishType) :void =>{
			console.log("etsIsApplicationRunning callback_03");
			let err = new BusinessError<boolean>;
			err.code = 0;
			let data : boolean = e as boolean;

			callback(err, data);
			console.log("callback etsIsApplicationRunning suc end	data  is ", data);
		}, (error: Object) :void => {
			console.log("callback etsIsApplicationRunning err end");
		})
	}

	export function isApplicationRunning(bundleName: string): Promise<boolean>
	{
		console.log("etsIsApplicationRunning Promise_01");
		let pPromise = new Promise<boolean>((resolve: (v: boolean) => void, reject: (error: Object) => void) => {
			console.log("etsIsApplicationRunning Promise_01");
			let p1 = launch nativeIsApplicationRunning(bundleName);
			p1.then((e :NullishType)=>{
				let r : boolean =  e as boolean;
				console.log("etsIsApplicationRunning Promise04	r ", r);
				resolve(r);
			}, ( error: Object) : void => {
				console.log("etsIsApplicationRunning Promise end");
			});
		});
		return pPromise;
	}

	export function getRunningMultiAppInfo(bundleName: string): Promise<ProcessInformation>
	{
		console.log(" ##### etsGetRunningMultiAppInfo Promise_01");
		let p = new Promise<ProcessInformation>((resolve: (v: ProcessInformation) => void, reject: (error: Object) => void) => {
		    console.log(" ##### etsGetRunningMultiAppInfo Promise_02");
			let p1 = launch nativeGetRunningMultiAppInfo(bundleName);
			console.log(" ##### etsGetRunningMultiAppInfo Promise_03");
			p1.then((e: ProcessInformation) => {
				let r: ProcessInformation = e;
				console.log(" ##### etsGetRunningMultiAppInfo Promise_04 r ", r);
				resolve(r);
			}, (error: Object): void => {
				console.log(" ##### etsGetRunningMultiAppInfo Promise_05");
				reject(error);
			});
		});
		console.log("##### etsGetRunningMultiAppInfo Promise  return");
		return p;
	}

	export function getSupportedProcessCachePids(bundleName: string): Promise<Array<number>>
	{
		console.log("##### etsGetSupportedProcessCachePids Promise begin");
		let pPromise = new Promise<Array<number>>((resolve: (v: Array<number>) => void, reject: (error: Object) => void) => {
			console.log("##### etsGetSupportedProcessCachePids Promise call executeGetSupportedProcessCachePids");
			let p1 = launch nativeGetSupportedProcessCachePids(bundleName);
			p1.then((e :NullishType)=>{
				let r : Array<number> =  e as Array<number>;
				console.log("##### etsGetSupportedProcessCachePids Promise  r ", r);
				resolve(r);
			}, ( error: Object) : void => {
				console.log("##### etsGetSupportedProcessCachePids Promise error");
			});
		});
		console.log("##### etsGetSupportedProcessCachePids Promise end");
		return pPromise;
	}

	export function getRunningProcessInfoByBundleName(bundleName: string, userId?: int, callback: AsyncCallback<ProcessInformation>): void
	{
		console.log(" ##### etsGetRunningProcessInfoByBundleName callback_01 userId enter");
		let p1 = launch nativeGetRunningProcessInfoByBundleName(bundleName, userId);
		p1.then((e: ProcessInformation): void => {
			let err = new BusinessError<ProcessInformation>();
			err.code = 0;
			let data: ProcessInformation = e;

			callback(err, data);
			console.log(" ##### callback etsGetRunningProcessInfoByBundleName userId suc data s ", data);
		}, (error: Object): void => {
			console.log(" ##### callback etsGetRunningProcessInfoByBundleName userId err");
		});
	}

	export function getRunningProcessInfoByBundleName(bundleName: string, userId: int): Promise<ProcessInformation>
	{
		console.log(" ##### etsGetRunningProcessInfoByBundleName Promise enter");
		let pPromise =  new Promise<ProcessInformation>((resolve: (v: ProcessInformation) => void, reject: (error: Object) => void) => {
			let p1 = launch nativeGetRunningProcessInfoByBundleName(bundleName, userId);
			console.log(" ##### etsGetRunningProcessInfoByBundleName userId Promise	end");
			p1.then((e: ProcessInformation) => {
				let r: ProcessInformation = e as ProcessInformation;
				console.log(" ##### etsGetRunningProcessInfoByBundleName userId Promise success  r ", r);
				resolve(r);
			}, (error: Object): void => {
				console.log(" ##### etsGetRunningProcessInfoByBundleName userId Promise fail");
				console.error("jrl test  Native error: ==== ", error);
				let err = new BusinessError<ProcessInformation>();
				console.error("jrl test  Native error: ==== ", err);
			 	reject(error);
			});
		});
		return pPromise;
	}

	export function getRunningProcessInfoByBundleName(bundleName: string, callback: AsyncCallback<ProcessInformation>): void
	{
		console.log(" ##### etsGetRunningProcessInfoByBundleName enter");
		let p1 = launch nativeGetRunningProcessInfoByBundleName(bundleName);
		p1.then((e: ProcessInformation): void => {
			let err = new BusinessError<ProcessInformation>();
			err.code = 0;
			let data: ProcessInformation = e;
			console.log(" ##### callback etsGetRunningProcessInfoByBundleNameUserId success data ", data);

			callback(err, data);
		}, (error: Object): void => {
			console.log(" ##### callback etsGetRunningProcessInfoByBundleNameUserId error fail");
		});
	}

	export function getRunningProcessInfoByBundleName(bundleName: string): Promise<ProcessInformation>
	{
		console.log(" ##### etsGetRunningProcessInfoByBundleName Promise_01");
		let pPromise =	new Promise<ProcessInformation>((resolve: (v: ProcessInformation) => void, reject: (error: Object) => void) => {
			console.log(" ##### etsGetRunningProcessInfoByBundleName Promise begin");
			console.log(" #####   bundleName = ", bundleName);
			let p1 = launch nativeGetRunningProcessInfoByBundleName(bundleName);
			console.log(" ##### etsGetRunningProcessInfoByBundleName Promise end");
			p1.then((e: ProcessInformation) => { // 明确类型为 ProcessInformation
				let r: ProcessInformation = e as ProcessInformation;
				console.log(" ##### etsGetRunningProcessInfoByBundleName Promise success r ", r);
				resolve(r);
			}, (error: Object): void => {
				console.log(" ##### etsGetRunningProcessInfoByBundleName Promise fail");
				reject(error);
			});
		});
		return pPromise;
	}

	export function preloadApplication(bundleName: string, userId: int, mode: PreloadMode, appIndex?: int): Promise<void>
	{
		console.log(" ##### etsPreloadApplication Promise_01");
	    let p = new Promise<void>((resolve: ResolveCallback<void>, reject: RejectCallback) => {
			console.log(" ##### etsPreloadApplication Promise  begin");
			let p1 = launch nativePreloadApplication(bundleName, userId, mode, appIndex);  
			p1.then((e :NullishType) => {
				console.log(" ##### etsPreloadApplication Promise  end");
				let ret : int = e as int  
				resolve(undefined);
				console.log("etsPreloadApplication Promise success: ", ret)
	        }, (err:Object): void => {
				reject(err)
				console.log("etsPreloadApplication Promise fail: ")
	        });
		});
		return p;
	}
}


