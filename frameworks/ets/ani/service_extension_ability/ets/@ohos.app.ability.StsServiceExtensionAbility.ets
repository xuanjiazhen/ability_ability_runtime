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

import Want from '@ohos.app.ability.Want';
import ServiceExtensionContext from './ServiceExtensionContext';

export default class ServiceExtensionAbility {
  private isOnDestroyAsync: boolean = true;
  private destroyCallbackPoint: long;
  private native nativeOnDestroyCallback(): void;
  private callOnDestroy(): boolean {
    let p = this.onDestroyAsync();
    if (this.isOnDestroyAsync) {
      p.then<void>((a:undefined): void => {
        this.nativeOnDestroyCallback();
      });
    } else {
      this.onDestroy();
    }
    return this.isOnDestroyAsync;
  }

  private isOnDisconnectAsync: boolean = true;
  private disconnectCallbackPoint: long;
  private native nativeOnDisconnectCallback(): void;
  private callOnDisconnect(want: Want): boolean {
    let p = this.onDisconnectSync(want);
    if (this.isOnDisconnectAsync) {
        p.then<void>((a:undefined): void => {
        this.nativeOnDisconnectCallback();
      });
    } else {
      this.onDisconnect(want);
    }
    return this.isOnDisconnectAsync;
  }
  launchWant: Want = new Want();
  lastRequestWant: Want = new Want();
  context: ServiceExtensionContext = {};
  onCreate(want: Want): void{
    console.log("onCreate");
  }
  onDestroy(): void {
    console.log("ServiceExtensionAbility onRequest");
  }
  onDestroyAsync(): Promise<void> {
    this.isOnDestroyAsync = false;
    return new Promise<void>((resolve: (a: undefined)=>void, reject: (err: Error)=>void): void => {});
  }
  onRequest(want: Want, startld: int): void {
    console.log("onRequest");
  }
  onConnect(want: Want): void {
      console.log("onConnect");
  }
  onDisconnect(want: Want): void {
      console.log("onDisconnect");
  }
  onDisconnectSync(want: Want): Promise<void> {
      this.isOnDisconnectAsync = false;
      return new Promise<void>((resolve: (a: undefined)=>void, reject: (err: Error)=>void): void => {});
  }
}