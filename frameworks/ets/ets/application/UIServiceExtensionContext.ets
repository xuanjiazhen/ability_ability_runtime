/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

import ExtensionContext from  'application.ExtensionContext';
import type Want from '@ohos.app.ability.Want';
import type StartOptions from '@ohos.app.ability.StartOptions';
import type AbilityStartCallback from 'application.AbilityStartCallback';
import { ConnectOptions } from 'ability.connectOptions';
import AsyncCallbackWrapper from '../utils/AbilityUtils';
import { AsyncCallback } from '@ohos.base';
import { BusinessError } from '@ohos.base';

export default class UIServiceExtensionContext extends ExtensionContext
{
  native nativeStartAbility(callback: AsyncCallback<void>, want: Want, options?: StartOptions): void;
  startAbility(want: Want, options?: StartOptions): Promise<void> {
    let p =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeStartAbility(myCall, want, options);
        });
      });
    return p;
  }

  native nativeTerminateSelf(callback: AsyncCallback<void>): void;
  terminateSelf(): Promise<void> {
    let p = new Promise<void>((resolve: (data: undefined) => void, reject:(err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeTerminateSelf(myCall);
      });
    });
    return p;
  }

  native startAbilityByType(type: string, wantParam: Record<string, Object>, abilityStartCallback: AbilityStartCallback, callback: AsyncCallback<void>): void;
  startAbilityByType(type: string, wantParam: Record<string, Object>, abilityStartCallback: AbilityStartCallback): Promise<void>
  {
    let p =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.startAbilityByType(type, wantParam, abilityStartCallback, myCall);
        });
      });
    return p;
  }

  native nativeConnectServiceExtensionAbility(want: Want, options: ConnectOptions): number;
  connectServiceExtensionAbility(want: Want, options: ConnectOptions): number {
    return this.nativeConnectServiceExtensionAbility(want, options);
  }

  native nativeDisconnectServiceExtensionAbility(connectionId: number,  callback: AsyncCallback<void>): void;
  disconnectServiceExtensionAbility(connectionId: number): Promise<void> {
    let p =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeDisconnectServiceExtensionAbility(connectionId, myCall);
        });
      });
    return p;
  }
}