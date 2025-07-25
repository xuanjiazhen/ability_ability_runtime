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

import { BusinessError, AsyncCallback } from '@ohos.base';
import AsyncCallbackWrapper from './utils/AbilityUtils';
import { ChildProcessArgs } from '@ohos.app.ability.ChildProcessArgs';
import { ChildProcessOptions } from '@ohos.app.ability.ChildProcessOptions';

namespace childProcessManager {
  loadLibrary("ability_child_process_manager_ani_kit.z")

  export const enum StartMode {
    SELF_FORK = 0,
    APP_SPAWN_FORK = 1,
  }

  export native function nativeStartChildProcess(srcEntry: string, startMode: StartMode,
    callback: AsyncCallbackWrapper<number>): void;
  export native function nativeStartArkChildProcess(srcEntry: string, args: ChildProcessArgs,
    options: ChildProcessOptions, callback: AsyncCallbackWrapper<number>): void;
  export native function nativeStartNativeChildProcess(entryPoint: string, args: ChildProcessArgs,
    options: ChildProcessOptions, callback: AsyncCallbackWrapper<number>): void;

  export function startChildProcess(srcEntry: string, startMode: StartMode): Promise<number> {
    let p = new Promise<number>((resolve:(data: number)=>void, reject:(err: BusinessError)=>void):void => {
      let myCall = new AsyncCallbackWrapper<number>((err: BusinessError | null, data: number | undefined)=>{
        if (err == null || err.code == 0) {
          resolve(data as number);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        childProcessManager.nativeStartChildProcess(srcEntry, startMode, myCall);
      });
    });
    return p;
  }
  export function startChildProcess(srcEntry: string, startMode: StartMode, callback: AsyncCallback<number>): void {
    let myCall = new AsyncCallbackWrapper<number>(callback);
    taskpool.execute((): void => {
      childProcessManager.nativeStartChildProcess(srcEntry, startMode, myCall);
    });
  }
  export function startArkChildProcess(srcEntry: string, args: ChildProcessArgs,
    options: ChildProcessOptions): Promise<number> {
    let p = new Promise<number>((resolve:(data: number)=>void, reject:(err: BusinessError)=>void):void => {
      let myCall = new AsyncCallbackWrapper<number>((err: BusinessError | null, data: number | undefined)=>{
        if (err == null || err.code == 0) {
          resolve(data as number);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        childProcessManager.nativeStartArkChildProcess(srcEntry, args, options, myCall);
      });
    });
    return p;
  }
  export function startNativeChildProcess(entryPoint: string, args: ChildProcessArgs,
    options: ChildProcessOptions): Promise<number> {
    let p = new Promise<number>((resolve:(data: number)=>void, reject:(err: BusinessError)=>void):void => {
      let myCall = new AsyncCallbackWrapper<number>((err: BusinessError | null, data: number | undefined)=>{
        if (err == null || err.code == 0) {
          resolve(data as number);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        childProcessManager.nativeStartNativeChildProcess(entryPoint, args, options, myCall);
      });
    });
    return p;
  }
}
export default childProcessManager;