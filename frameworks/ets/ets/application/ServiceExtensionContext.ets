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

import { AsyncCallback } from '@ohos.base';
import AsyncCallbackWrapper from '../utils/AbilityUtils';
import { BusinessError } from '@ohos.base';
import { ConnectOptions } from 'ability.connectOptions';
import ExtensionContext from 'application.ExtensionContext';
import StartOptions from '@ohos.app.ability.StartOptions';
import Want from '@ohos.app.ability.Want';

class Cleaner {
  public ptr: long = 0;

  constructor(ptr: long) {
    this.ptr = ptr;
  }

  native clean(): void;
}

export function callback(cleaner: Cleaner): void {
  cleaner.clean();
}

let destroyRegister = new FinalizationRegistry<Cleaner>(callback);
let unregisterToken = new object();

export default class ServiceExtensionContext extends ExtensionContext {
  constructor(context: long) {
    if (this.nativeEtsContext == 0) {
      this.nativeEtsContext = context;
    }
    this.registerCleaner(this.nativeEtsContext);
  }

  registerCleaner(ptr: long): void {
    this.cleaner = new Cleaner(ptr);
    destroyRegister.register(this, this.cleaner!, unregisterToken);
  }

  unregisterCleaner(): void {
    destroyRegister.unregister(unregisterToken);
  }

  nativeEtsContext: long = 0;
  private cleaner: Cleaner | null = null;

  native nativeTerminateSelf(callback: AsyncCallbackWrapper<void>): void;

  native nativeStartAbility(want: Want, callback: AsyncCallbackWrapper<void>): void;

  native nativeStartAbility(want: Want, options: StartOptions, callback: AsyncCallbackWrapper<void>): void;

  native nativeStartServiceExtensionAbility(want: Want, callback: AsyncCallbackWrapper<void>): void;

  native nativeStopServiceExtensionAbility(want: Want, callback: AsyncCallbackWrapper<void>): void;

  native nativeConnectServiceExtensionAbility(want: Want, options: ConnectOptions): long;

  native nativeDisconnectServiceExtensionAbility(connection: long, callback: AsyncCallbackWrapper<void>): void;

  terminateSelfWithCallback(callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeTerminateSelf(myCall);
    });
  }

  terminateSelfReturnsPromise(): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
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
  }

  overload terminateSelf {
    terminateSelfWithCallback,
    terminateSelfReturnsPromise
  }

  startAbilityWithWantCallback(want: Want, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbility(want, myCall);
    });
  }

  startAbilityWithWantOptionsCallback(want: Want, options: StartOptions, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbility(want, options, myCall);
    });
  }

  startAbilityWithWantOptionsReturnsPromise(want: Want, options: StartOptions): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeStartAbility(want, options, myCall);
        });
      });
  }

  overload startAbility {
    startAbilityWithWantCallback,
    startAbilityWithWantOptionsCallback,
    startAbilityWithWantOptionsReturnsPromise
  }

  startServiceExtensionAbilityWithWantCallback(want: Want, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartServiceExtensionAbility(want, myCall);
    });
  }

  startServiceExtensionAbilityWithWantReturnsPromise(want: Want): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeStartServiceExtensionAbility(want, myCall);
        });
      });
  }

  overload startServiceExtensionAbility {
    startServiceExtensionAbilityWithWantCallback,
    startServiceExtensionAbilityWithWantReturnsPromise
  }

  stopServiceExtensionAbilityWithWantCallback(want: Want, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStopServiceExtensionAbility(want, myCall);
    });
  }

  stopServiceExtensionAbilityWithWantReturnsPromise(want: Want): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeStopServiceExtensionAbility(want, myCall);
      });
    });
  }

  overload stopServiceExtensionAbility {
    stopServiceExtensionAbilityWithWantCallback,
    stopServiceExtensionAbilityWithWantReturnsPromise
  }

  connectServiceExtensionAbility(want: Want, options: ConnectOptions): long {
    return this.nativeConnectServiceExtensionAbility(want, options);
  }

  disconnectServiceExtensionAbilityWithConnectionCallback(connection: long, callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeDisconnectServiceExtensionAbility(connection, asyncCall);
    });
  }

  disconnectServiceExtensionAbilityWithConnectionReturnsPromise(connection: long): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let asyncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeDisconnectServiceExtensionAbility(connection, asyncCall);
      })
    });
  }

  overload disconnectServiceExtensionAbility {
    disconnectServiceExtensionAbilityWithConnectionCallback,
    disconnectServiceExtensionAbilityWithConnectionReturnsPromise
  }
}
