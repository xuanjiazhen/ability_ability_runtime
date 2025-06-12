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

// eventHub.ets
native function nativeGetEventHubOnRef(): ESObject;
native function nativeGetEventHubOffRef(): ESObject;
native function nativeGetEventHubEmitRef(): ESObject;

type refType = Object | ESObject;
native function saveEventHubOnHybridGref(ref: refType): void;
native function saveEventHubOffHybridGref(ref: refType): void;
native function saveEventHubEmitHybridGref(ref: refType): void;

export default class EventHub {
  private eventMap: Map<string, Function[]> = new Map();

  constructor() {
    saveEventHubOnHybridGref({
      registerOn: (event: string, callback: Function) => this.registerOnCallback(event, callback, true)
    });
    saveEventHubOffHybridGref({
      registerOff: (event: string, callback?: Function) => this.registerOffCallback(event, callback, true)
    })
    saveEventHubEmitHybridGref({
      registerEmit: (event: string, ...args: FixedArray<NullishType>) => this.registerEmitCallback(event, ...args)
    })
  }

  private registerOnCallback(event: string, callback: Function): void {
    if (!this.eventMap.has(event)) {
      this.eventMap.set(event, []);
    }
    
    const callbacks = this.eventMap.get(event)!;
    if (!callbacks.includes(callback)) {
      callbacks.push(callback);
    }
  }

  on(event: string, callback: Function, fromNative = false): void {
    this.registerOnCallback(event, callback);

    if (!fromNative) {
      const nativeObj = this.nativeGetEventHubOnRef();
      if (nativeObj) {
        obj.invokeMethod(event, callback);
      }
    }
  }

  private registerOffCallback(event: string, callback?: Function): void {
    if (!this.eventMap.has(event)) {
      return;
    }
    
    if (!callback) {
      this.eventMap.delete(event);
    } else {
      const callbacks = this.eventMap.get(event)!;
      const index = callbacks.indexOf(callback);
      if (index > -1) {
        callbacks.splice(index, 1);
      }
      
      if (callbacks.length === 0) {
        this.eventMap.delete(event);
      }
    }
  }

  off(event: string, callback?: Function, fromNative = false): void {
    this.registerOffCallback(event, callback);

    if (!fromNative) {
      const nativeObj = this.nativeGetEventHubOffRef();
      if (nativeObj) {
        if (callback) {
          nativeObj.callFunction(event, callback);
        } else {
          nativeObj.callFunction(event);
        }
      }
    }
  }

  private registerEmitCallback(event: string, args: FixedArray<NullishType>[]): void {
    if (!this.eventMap.has(event)) {
      return;
    }
    
    const callbacks = [...this.eventMap.get(event)!];
    callbacks.forEach(callback => {
      try {
        callback.unsafeCall(...args);
      } catch (e) {
        console.error(`Error triggering event '${event}':`, e);
      }
    });
  }

  emit(event: string, ...args: FixedArray<NullishType>): void {
    this.registerEmitCallback(event, args);
    if (nativeObj) {
      nativeObj.callFunction(event, ...args);
    }
  }
}