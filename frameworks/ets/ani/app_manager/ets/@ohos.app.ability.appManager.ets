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

import AppStateData from './application/AppStateData'

type AsyncCallback<T> = (err: BusinessError<T>, data: T)=> void;
export class BusinessError<T = void> {
    code: number;
    data: T;
  }

type ResolveCallback<T> = (data: T)=> void;
type RejectCallback = (err: Object)=> void;

export default namespace appManager {
  export enum PreloadMode {
    PRESS_DOWN
  }

  export native function nativePreloadApplication(bundleName: string, userId: number, mode: PreloadMode, appIndex?: number): void;
  export native function nativeIsRunningInStabilityTest(): boolean;
  export native function nativeKillProcessWithAccount(bundleName: string, accountId: int): int;
  export native function nativeKillProcessWithAccountEx(bundleName: string, accountId: int, clearPageStack: boolean, appIndex?: int): int;
  export native function nativeKillProcessesByBundleName(bundleName: string): int;
  export native function nativeKillProcessesByBundleNameEx(bundleName: string, clearPageStack: boolean, appIndex?: int): int;
  export native function nativeClearUpApplicationData(bundleName: string): int;
  export native function nativeIsRamConstrainedDevice(): boolean;
  export native function nativeGetForegroundApplications(): Array<AppStateData>;


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

  export function isRunningInStabilityTest(callback: AsyncCallback<boolean>):void{
        let p1 = launch nativeIsRunningInStabilityTest();
            p1.then((e :NullishType)=>{
                let resultCode: boolean  = e as boolean;
                let r: BusinessError<boolean> = { code: 1, data: resultCode };
                callback(r, resultCode);
                console.log("isRunningInStabilityTest callback success: ", resultCode)
            }, (err:Object): void => {
                console.log("isRunningInStabilityTest callback fail", err)
            });
        console.log("isRunningInStabilityTest callback end")
    }
  export function isRunningInStabilityTest():Promise<boolean>{
        let p = new Promise<boolean>((resolve: ResolveCallback<boolean>, reject: RejectCallback) => {
            let p1 = launch nativeIsRunningInStabilityTest();
            p1.then((e :NullishType)=>{
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

  export function killProcessWithAccount(bundleName: string, accountId: int):Promise<void>{
        let p = new Promise<void>((resolve: ResolveCallback<void>, reject: RejectCallback) => {
            let p1 = launch nativeKillProcessWithAccount(bundleName, accountId);
            p1.then((e :NullishType)=>{
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

  export function killProcessWithAccount(bundleName: string, accountId: int, callback: AsyncCallback<void>):void{
        let p1 = launch nativeKillProcessWithAccount(bundleName, accountId);
            p1.then((e :NullishType)=>{
                let returnCode : Int = e as Int
                let r: BusinessError = { code: returnCode.intValue(), data: undefined };
                callback(r, undefined);
                console.log("killProcessWithAccount callback success: ")
            }, (err:Object): void => {
            });
        console.log("killProcessWithAccount callback end")
    }
  export function killProcessWithAccount(bundleName: string, accountId: int, clearPageStack: boolean, appIndex?: int):Promise<void>{
            let p = new Promise<void>((resolve: ResolveCallback<void>, reject: (error: Object) => void) => {
                let p1 = launch nativeKillProcessWithAccountEx(bundleName, accountId, clearPageStack, appIndex);
                p1.then((e :NullishType)=>{
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

    export function killProcessesByBundleName(bundleName: string):Promise<void>{
        let p = new Promise<void>((resolve: ResolveCallback<void>, reject: RejectCallback) => {
            let p1 = launch nativeKillProcessesByBundleName(bundleName);
            p1.then((e :NullishType)=>{
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

    export function killProcessesByBundleName(bundleName: string, clearPageStack: boolean, appIndex?: int):Promise<void>{
            let p = new Promise<void>((resolve: ResolveCallback<void>, reject: RejectCallback) => {
                let p1 = launch nativeKillProcessesByBundleNameEx(bundleName, clearPageStack, appIndex);
                p1.then((e :NullishType)=>{
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

    export function killProcessesByBundleName(bundleName: string, callback: AsyncCallback<void>):void{
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

    export function clearUpApplicationData(bundleName: string):Promise<void>{
        let p = new Promise<void>((resolve: ResolveCallback<void>, reject: RejectCallback) => {
            let p1 = launch nativeClearUpApplicationData(bundleName);
            p1.then((e :NullishType)=>{
            let ret : int = e as int  
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

  export function clearUpApplicationData(bundleName: string, callback: AsyncCallback<void>):void{
        let p1 = launch nativeClearUpApplicationData(bundleName);
            p1.then((e :NullishType)=>{
                let returnCode : Int = e as Int
                let r: BusinessError = { code: returnCode.intValue(), data: undefined };
                callback(r, undefined);
                console.log("clearUpApplicationData callback success")
            }, (err:Object): void => {
                console.log("clearUpApplicationData callback fail")
            });
        console.log("clearUpApplicationData callback end")
    }

  export function isRamConstrainedDevice(callback: AsyncCallback<boolean>):void{
        let p1 = launch nativeIsRamConstrainedDevice();
            p1.then((e :NullishType)=>{
                let resultCode: boolean  = e as boolean;
                let r: BusinessError<boolean> = { code: 1, data: resultCode };
                callback(r, resultCode);
                console.log("isRamConstrainedDevice callback success: ", resultCode)
            }, (err:Object): void => {
                console.log("isRamConstrainedDevice callback fail")
            });
        console.log("isRamConstrainedDevice callback end")
    }
  export function isRamConstrainedDevice():Promise<boolean>{
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
            p1.then((e :NullishType)=>{
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

  export function getForegroundApplications():Promise<Array<AppStateData>>{
        let p = new Promise<Array<AppStateData>>((resolve: ResolveCallback<Array<AppStateData>>, reject: RejectCallback) => {
                let p1 = launch nativeGetForegroundApplications();
                p1.then((e : NullishType)=>{
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

}


