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
import ExtensionContext from 'application.ExtensionContext';
import StartOptions from '@ohos.app.ability.StartOptions';
import Want from '@ohos.app.ability.Want';

class Cleaner {
  static callback(cleaner: Cleaner): void {
    console.println("enter Cleaner.callback");
    cleaner.clean()
  }

  constructor(targetPtr: long) {
    this.targetPtr = targetPtr
  }

  native clean(): void

  private targetPtr: long = 0
}

class FinalizationAgent<T extends Object> {
  constructor(obj: T, ptr: long) {
    this.register(obj, ptr);
  }

  register(obj: T, ptr: long): void {
    this.unregisterToken = {};
    this.cleaner = new Cleaner(ptr);
    finalizer.register(obj, this.cleaner!, this.unregisterToken);
  }

  unregister(): void {
    finalizer.unregister(this.unregisterToken);
  }

  private cleaner: Cleaner | null = null;
  private unregisterToken: object;
}

let finalizer = new FinalizationRegistry<Cleaner>(Cleaner.callback)

export default class ServiceExtensionContext extends ExtensionContext {
  constructor(context:long) {
    if(this.nativeEtsContext == 0){
      this.nativeEtsContext = context;
    }
    this.clearAgent = new FinalizationAgent<ServiceExtensionContext>(this, this.nativeEtsContext);
  }
  registerCleaner(ptr: long): void {
    this.cleaner = new Cleaner(ptr)
    destroyRegister.register(this, this.cleaner!, unregisterToken);
  }

  unregisterCleaner(): void {
    this.clearAgent.unregister();
  }
  nativeEtsContext:long = 0;
  clearAgent: FinalizationAgent<ServiceExtensionContext>;

  native nativeTerminateSelf(callback:AsyncCallbackWrapper<void>): void;
  native nativeStartAbility(want: Want, callback: AsyncCallbackWrapper<void>): void;
  native nativeStartAbility(want: Want, options: StartOptions, callback:AsyncCallbackWrapper<void>): void;
  native nativeStartServiceExtensionAbility(want: Want, callback: AsyncCallbackWrapper<void>): void;

  terminateSelf(callback:AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeTerminateSelf(myCall);
    });
  }

  terminateSelf(): Promise<void> {
    let p = new Promise<void>((resolve: (data: undefined) => void, reject:(err: BusinessError<void>) => void): void => {
      let myCall = new AsyncCallbackWrapper<void>((err: BusinessError<void>) => {
        if (err.code == 0) {
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

  startAbility(want: Want, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbility(want, myCall);
    });
  }

  startAbility(want: Want, options: StartOptions, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbility(want, options, myCall);
    });
  }

  startAbility(want: Want, options: StartOptions): Promise<void> {
    let p = new Promise<void>((resolve: (data:undefined)=>void, reject:(err: BusinessError<void>)=>void):void => {
      let myCall = new AsyncCallbackWrapper<void>((err: BusinessError<void>)=>{
        if (err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeStartAbility(want, options, myCall);
      });
    });
    return p;
  }

  startServiceExtensionAbility(want: Want, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartServiceExtensionAbility(want, myCall);
    });
  }

  startServiceExtensionAbility(want: Want): Promise<void> {
    let p = new Promise<void>((resolve: (data: undefined)=>void, reject:(err: BusinessError<void>) => void): void => {
      let myCall = new AsyncCallbackWrapper<void>((err: BusinessError<void>) => {
        if (err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeStartServiceExtensionAbility(want, myCall);
      });
    });
    return p;
  }
}