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

export default namespace appManager {
  static { loadLibrary("ability_app_manager_ani_kit.z"); }
  export enum PreloadMode {
    PRESS_DOWN
  }

  export native function nativePreloadApplication(bundleName: string, userId: number, mode: PreloadMode, appIndex?: number): Promise<void>;
  export function preloadApplication(bundleName: string, userId: number, mode: PreloadMode, appIndex?: number):Promise<void>{
    let p = new Promise<void>((resolve: (v: PromiseLike<void>) => void, reject: (error: Object) => void) => {
        let p1 = launch nativePreloadApplication(msg, code);
        p1.then((e :NullishType)=>{
        let PromiseLike = e as PromiseLike<void>;
        resolve(PromiseLike);
        }, (err:Object): void => {
        });
    });
    return p;
  }
}


