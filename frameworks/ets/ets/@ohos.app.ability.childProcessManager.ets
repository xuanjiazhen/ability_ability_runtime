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

namespace childProcessManager {
  loadLibrary("ability_child_process_manager_ani_kit.z")

  export const enum StartMode {
    SELF_FORK = 0,
    APP_SPAWN_FORK = 1,
  }

  export native function nativeStartChildProcess(srcEntry: string, startMode: StartMode, callback: AsyncCallbackWrapper<number>): void;
  export function startChildProcess(srcEntry: string, startMode: StartMode): Promise<number> {
    let p = new Promise<number>((resolve:(data: number)=>void, reject:(err: BusinessError<void>)=>void):void => {
      let myCall = new AsyncCallbackWrapper<number>((err: BusinessError<void>, data: number)=>{
        if (err.code == 0) {
          resolve(data);
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
}

export default childProcessManager;