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

import AbilityMonitor from './AbilityMonitor';
import Want from '@ohos.app.ability.Want';
import Context from 'application.Context';
import { BusinessError, AsyncCallback } from '@ohos.base';
import ShellCmdResult  from './shellCmdResult';
import UIAbility from '@ohos.app.ability.UIAbility';

export interface AbilityDelegator {
    printSync(msg: string): void;
    getAppContext(): Context;
    finishTest(msg: string, code: number, callback: AsyncCallback<void>):void;
    finishTest(msg: string, code: number):Promise<void>;
    executeShellCommand (cmd: String, callback: AsyncCallback<ShellCmdResult>) : void;
    executeShellCommand (cmd: String, timeoutSecs: number, callback: AsyncCallback<ShellCmdResult>) : void;
    executeShellCommand (cmd: string, timeoutSecs?: number) : Promise<ShellCmdResult>;
    addAbilityMonitor(monitor: AbilityMonitor, callback: AsyncCallback<void>): void
    addAbilityMonitor(monitor: AbilityMonitor): Promise<void>;
    addAbilityMonitorAsync(monitor: AbilityMonitor): void;
    startAbility(want: Want, callback: AsyncCallback<void>): void;
    startAbility(want: Want): Promise<void>;
    getCurrentTopAbility(callback: AsyncCallback<UIAbility>);
    getCurrentTopAbility(): Promise<UIAbility>;
    // other api
}

class AbilityDelegatorInner implements AbilityDelegator {
    public native printSync(msg: string): void;
    public native finishTestSync(msg: String, code: number): void;
    public native executeShellCommandsync(cmd: string, timeoutSecs: number): ShellCmdResult;
    public native getAppContext(): Context;
    public native addAbilityMonitorAsync(monitor: AbilityMonitor): void;
    public native startAbilityAsync(want: Want) : void;
    public native GetCurrentTopAbilitySync() : UIAbility;
    // other api

    finishTest(msg: string, code: number, callback: AsyncCallback<void>):void{
        let p1 = taskpool.execute(():undefined => {return this.finishTestSync(msg, code)});
            p1.then((e :NullishType)=>{
                let resultCode: Int  = 0;
                let error: BusinessError = { code: resultCode.intValue(), data: undefined };
                callback(error, undefined);
            }).catch((err: BusinessError): void => {
                callback(err, undefined);
            });
    }
    finishTest(msg: string, code: number):Promise<void>{
        let p = new Promise<void>((resolve, reject) => {
            let p1 = taskpool.execute(():undefined => {return this.finishTestSync(msg, code)});
            p1.then((e :NullishType)=>{
                let r:  undefined;
                resolve(r);
            }).catch((err: BusinessError): void => {
                reject(err);
          });
        });
        return p;
    }

    executeShellCommand (cmd: String, callback: AsyncCallback<ShellCmdResult>) : void {
        let timeoutSecs =  0;
        let p = taskpool.execute(():ShellCmdResult => {return this.executeShellCommandsync(cmd, timeoutSecs)});
        p.then((e :NullishType)=>{
            let cmdresult = e as ShellCmdResult;
            let resultCode: Int  = 0;
            let error: BusinessError = { code: resultCode.intValue(), data: undefined };
            callback(error, cmdresult);
        }).catch((err: BusinessError): void => {
            callback(err, {});
        });
    }
    executeShellCommand (cmd: String, timeoutSecs: number, callback: AsyncCallback<ShellCmdResult>) : void {
        let p = taskpool.execute(():ShellCmdResult => {return this.executeShellCommandsync(cmd, timeoutSecs)});
        p.then((e :NullishType)=>{
            let cmdresult = e as ShellCmdResult;
            let resultCode: Int  = 0;
            let error: BusinessError = { code: resultCode.intValue(), data: undefined };
            callback(error, cmdresult);
        }).catch((err: BusinessError): void => {
            callback(err, {});
        });
    }
    executeShellCommand (cmd: string, timeoutSecs?: number) : Promise<ShellCmdResult>{
        if (timeoutSecs == null) {
        let pPromise2 = new Promise<ShellCmdResult>((resolve: (v: ShellCmdResult) => void, reject: (error: Object) => void) => {
            let pPromise1 = taskpool.execute(():ShellCmdResult => {return this.executeShellCommandsync(cmd, 0);});
            pPromise1.then((e :NullishType)=>{
                let r : ShellCmdResult =  e as ShellCmdResult;
                resolve(r);
            }).catch((err: BusinessError): void => {
                reject(err);
          });
        });
        return pPromise2;
        } else {
        let pPromise = new Promise<ShellCmdResult>((resolve: (v: ShellCmdResult) => void, reject: (error: Object) => void) => {
            let pPromise1 = taskpool.execute(():ShellCmdResult=> {return  this.executeShellCommandsync(cmd, timeoutSecs as Double)});
            pPromise1.then((e :NullishType)=>{
                let r : ShellCmdResult =  e as ShellCmdResult;
                resolve(r);
            }).catch((err: BusinessError): void => {
                reject(err);
          });
        });
        return pPromise;
        }
    }

    addAbilityMonitor(monitor: AbilityMonitor, callback: AsyncCallback<void>): void {
        let p1 = taskpool.execute(():undefined => {return this.addAbilityMonitorAsync(monitor)});
        p1.then((e: NullishType)=>{
                let resultCode: Int  = 0;
                let error: BusinessError = { code: resultCode.intValue(), data: undefined };
                callback(error, undefined);
            }).catch((err: BusinessError): void => {
                callback(err, undefined);
            });
    }
    addAbilityMonitor(monitor: AbilityMonitor): Promise<void> {
        let p = new Promise<void>((resolve, reject) => {
            let p1 = taskpool.execute(():undefined => {return this.addAbilityMonitorAsync(monitor);});
            p1.then((e: NullishType)=>{
                let r : undefined;
                resolve(r);
            }).catch((err: BusinessError): void => {
                reject(err);
          });
        });
        return p;
    }

    startAbility(want: Want, callback: AsyncCallback<void>): void {
        let p1 = taskpool.execute(():undefined => {return this.startAbilityAsync(want)});
            p1.then((e :NullishType)=>{
                let resultCode: Int  = 0;
                let error: BusinessError = { code: resultCode.intValue(), data: undefined };
                callback(error, undefined);
            }).catch((err: BusinessError): void => {
                callback(err, undefined);
            });
    }
    startAbility(want: Want): Promise<void> {
        let p = new Promise<void>((resolve, reject) => {
            let p1 = taskpool.execute(():undefined => {return this.startAbilityAsync(want);});
            p1.then((e :NullishType)=>{
                let r : undefined;
                resolve(r);
            }).catch((err: BusinessError): void => {
                reject(err);
                console.log("getCurrentTopAbility promise1 end");
          });
        });
        return p;
    }
    getCurrentTopAbility (callback: AsyncCallback<UIAbility>) : void {
        let p = taskpool.execute(():UIAbility => {return this.GetCurrentTopAbilitySync()});
        console.log("getCurrentTopAbility callback2_04");
        p.then((e :NullishType)=>{
            let ability = e as UIAbility;
            let resultCode: Int  = 0;
            let error: BusinessError = { code: resultCode.intValue(), data: undefined };
            callback(error, ability);
            console.log("getCurrentTopAbility callback2_06");
        }).catch((err: BusinessError): void => {
            console.log("getCurrentTopAbility callback2_0606 err end");
            callback(err, {});
        });
    }
    getCurrentTopAbility() : Promise<UIAbility>{
        let p = new Promise<UIAbility>((resolve: (v: UIAbility) => void, reject: (error: Object) => void) => {
            console.log("getCurrentTopAbility promise1_01");
            let pPromise1 = taskpool.execute(():UIAbility => {return this.GetCurrentTopAbilitySync();});
            pPromise1.then((e :NullishType)=>{
                console.log("getCurrentTopAbility promise1_04");
                let r : UIAbility = e as UIAbility;
                resolve(r);
            }).catch((err: BusinessError): void => {
                reject(err);
                console.log("getCurrentTopAbility promise1 end");
          });
        });
        return p;
    }
}
