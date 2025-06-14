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

type refType = Object | ESObject;
type RegisterEmitFunction = (event: string, ...args: FixedArray<NullishType>) => void;
 
interface EventHubRegistration {
  registerOn: (event: string, callback: Function) => void;
  registerOff: (event: string, callback?: Function) => void;
  registerEmit: RegisterEmitFunction;
}

type refType = Object | ESObject;
type RegisterEmitFunction = (event: string, ...args: FixedArray<NullishType>) => void;

interface EventHubRegistration {
  registerOn: (event: string, callback: Function) => void;
  registerOff: (event: string, callback?: Function) => void;
  registerEmit: RegisterEmitFunction;
}

export default class EventHub {

  native nativeGetEventHubOnRef(): ESObject;
  native nativeGetEventHubOffRef(): ESObject;
  native nativeGetEventHubEmitRef(): ESObject;

  native saveEventHubOnHybridGref(ref: refType): void;
  native saveEventHubOffHybridGref(ref: refType): void;
  native saveEventHubEmitHybridGref(ref: refType): void;

  private eventMap: Map<string, Function[]> = new Map<string, Function[]>();

  constructor() {
    this.saveEventHubOnHybridGref({
      registerOn: (event: string, callback: Function): void => {
        this.on(event, callback, true);
      }
    } as EventHubRegistration);
    this.saveEventHubOffHybridGref({
      registerOff: (event: string, callback?: Function): void => {
        this.off(event, callback, true);
      }
    } as EventHubRegistration);

    const emitHandler: RegisterEmitFunction = (event: string, ...args: FixedArray<NullishType>): void => {
        this.emit(event, ...args);
    }

    this.saveEventHubEmitHybridGref({ registerEmit: emitHandler } as EventHubRegistration);
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

  on(event: string, callback: Function, fromNative: boolean = false): void {
    this.registerOnCallback(event, callback);

    if (!fromNative) {
      let nativeObj = this.nativeGetEventHubOnRef();
      if (nativeObj) {
        nativeObj.invokeMethod(event, callback);
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

  off(event: string, callback?: Function, fromNative: boolean = false): void {
    this.registerOffCallback(event, callback);

    if (!fromNative) {
      const nativeObj = this.nativeGetEventHubOffRef();
      if (nativeObj) {
        if (callback) {
          nativeObj.invokeMethod(event, callback);
        } else {
          nativeObj.invokeMethod(event);
        }
      }
    }
  }

  private registerEmitCallback(event: string, ...args: FixedArray<NullishType>): void {
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
    this.registerEmitCallback(event, ...args);
    const nativeObj = this.nativeGetEventHubEmitRef();
    if (nativeObj) {
      nativeObj.invokeMethod(event, ...args);
    }
  }
}