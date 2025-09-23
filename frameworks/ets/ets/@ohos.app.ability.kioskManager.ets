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

/**
 * @file
 * @kit AbilityKit
 */

import UIAbilityContext from './application/UIAbilityContext';
import { KioskStatus as _KioskStatus } from './application/KioskStatus';

export default namespace kioskManager {
    export native function nativeEnterKioskMode(UIAbilityContext context ,callback: AsyncCallbackWrapper<void>): void;
    export native function nativeExitKioskMode(UIAbilityContext context ,callback: AsyncCallbackWrapper<void>): void;
    export native function nativeGetKioskStatus(): KioskStatus;
    function enterKioskMode(context: UIAbilityContext): Promise<void>{
      let p: Promise<void> = 
        new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError | null) => void): void => {
            let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null)=> {
              if (err == null || err.code == 0) {
                resolve(undefined);
              } else {
                reject(err);
              }
            });
            taskpool.execute((): void => {
                kioskManager.nativeEnterKioskMode(context, myCall);
            });
        });
      return p;
    }
    function exitKioskMode(context: UIAbilityContext): Promise<void>{
      let p: Promise<void> = 
        new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError | null) => void): void => {
            let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null)=> {
              if (err == null || err.code == 0) {
                resolve(undefined);
              } else {
                reject(err);
              }
            });
            taskpool.execute((): void => {
                kioskManager.nativeExitKioskMode(context, myCall);
            });
        });
      return p;
    }
    function getKioskStatus(): Promise<KioskStatus>{
        let p: Promise<void> = 
        new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError | null) => void): void => {
            let myCall = new AsyncCallbackWrapper<KioskStatus>((err: BusinessError | , data: KioskStatus | undefined)=> {
              if (err == null || err.code == 0) {
                resolve(data as KioskStatus);
              } else {
                reject(err);
              }
            });
            taskpool.execute((): void => {
                kioskManager.nativeGetKioskStatus(myCall);
            });
        });
      return p;
    }
    export type KioskStatus = _KioskStatus;
}