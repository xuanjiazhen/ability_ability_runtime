/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

import StartupConfig from './@ohos.app.appstartup.StartupConfig';
import AsyncCallbackWrapper from './utils/AbilityUtils';

declare namespace startupManager {

  loadLibrary("startupmanager_ani_kit.z")

  native function nativeRunSync(startupTasks: Array<string>, config: StartupConfig,
    callback: AsyncCallbackWrapper<void>): void;
  native function nativeRemoveAllStartupTaskResults(): void;
  native function nativeGetStartupTaskResult(startupTask: string): Object;
  native function nativeIsStartupTaskInitialized(startupTask: string): boolean;
  native function nativeRemoveStartupTaskResult(startupTask): void;

  export function run(startupTasks: Array<string>, config?: StartupConfig): Promise<void> {
     return new Promise<void>((resolve: (data:undefined)=>void, reject:(err: BusinessError<void>)=>void):void => {
      let asyncCall = new AsyncCallbackWrapper<void>((err: BusinessError<void>)=>{
        if (err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void =>{
        nativeRunSync(startupTasks, config, asyncCall);
      });
    });
  }

  export function removeAllStartupTaskResults(): void {
    nativeRemoveAllStartupTaskResults();
  }

  export function getStartupTaskResult(startupTask: string): Object {
    return nativeGetStartupTaskResult(startupTask);
  }

  export function isStartupTaskInitialized(startupTask: string): boolean {
    return nativeIsStartupTaskInitialized(startupTask);
  }

  export function removeStartupTaskResult(startupTask: string): void {
    nativeRemoveStartupTaskResult(startupTask);
  }
}

export default startupManager;