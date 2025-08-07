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

import { AbilityMonitor } from 'application.AbilityMonitor';
import Want from '@ohos.app.ability.Want';
import Context from 'application.Context';
import { BusinessError, AsyncCallback } from '@ohos.base';
import { ShellCmdResult } from 'application.shellCmdResult';
import UIAbility from '@ohos.app.ability.UIAbility';
import AsyncCallbackWrapper from '../utils/AbilityUtils';
import { AbilityStageMonitor } from 'application.AbilityStageMonitor';
import AbilityStage from '@ohos.app.ability.AbilityStage';

export interface AbilityDelegator {
  printSync(msg: string): void;

  getAppContext(): Context;

  finishTestWithMsgCodeCallback(msg: string, code: long, callback: AsyncCallback<void>): void;

  finishTestWithMsgCodeReturnsPromise(msg: string, code: long): Promise<void>;

  overload finishTest {
    finishTestWithMsgCodeCallback,
    finishTestWithMsgCodeReturnsPromise
  }

  executeShellCommandWithCmdCallback(cmd: String, callback: AsyncCallback<ShellCmdResult>): void;

  executeShellCommandWithCmdTimeoutSecsCallback(cmd: String, timeoutSecs: long,
    callback: AsyncCallback<ShellCmdResult>): void;

  executeShellCommandWithCmdTimeoutSecsReturnsPromise(cmd: string, timeoutSecs?: long): Promise<ShellCmdResult>;

  overload executeShellCommand {
    executeShellCommandWithCmdCallback,
    executeShellCommandWithCmdTimeoutSecsCallback,
    executeShellCommandWithCmdTimeoutSecsReturnsPromise
  }

  addAbilityMonitorWithMonitorCallback(monitor: AbilityMonitor, callback: AsyncCallback<void>): void;

  addAbilityMonitorWithMonitorReturnsPromise(monitor: AbilityMonitor): Promise<void>;

  overload addAbilityMonitor {
    addAbilityMonitorWithMonitorCallback,
    addAbilityMonitorWithMonitorReturnsPromise
  }

  addAbilityMonitorSync(monitor: AbilityMonitor): void;

  removeAbilityMonitorWithMonitorCallback(monitor: AbilityMonitor, callback: AsyncCallback<void>): void

  removeAbilityMonitorWithMonitorReturnsPromise(monitor: AbilityMonitor): Promise<void>;

  overload removeAbilityMonitor {
    removeAbilityMonitorWithMonitorCallback,
    removeAbilityMonitorWithMonitorReturnsPromise
  }

  removeAbilityMonitorSync(monitor: AbilityMonitor): void;

  waitAbilityMonitorWithMonitorCallback(monitor: AbilityMonitor, callback: AsyncCallback<UIAbility>): void;

  waitAbilityMonitorWithMonitorTimeoutCallback(monitor: AbilityMonitor, timeout: long,
    callback: AsyncCallback<UIAbility>): void;

  waitAbilityMonitorWithMonitorTimeoutReturnsPromise(monitor: AbilityMonitor, timeout?: long): Promise<UIAbility>;

  overload waitAbilityMonitor {
    waitAbilityMonitorWithMonitorCallback,
    waitAbilityMonitorWithMonitorTimeoutCallback,
    waitAbilityMonitorWithMonitorTimeoutReturnsPromise
  }

  addAbilityStageMonitorWithMonitorCallback(monitor: AbilityStageMonitor, callback: AsyncCallback<void>): void;

  addAbilityStageMonitorWithMonitorReturnsPromise(monitor: AbilityStageMonitor): Promise<void>;

  overload addAbilityStageMonitor {
    addAbilityStageMonitorWithMonitorCallback,
    addAbilityStageMonitorWithMonitorReturnsPromise
  }

  addAbilityStageMonitorSync(monitor: AbilityStageMonitor): void;

  removeAbilityStageMonitorWithMonitorCallback(monitor: AbilityStageMonitor, callback: AsyncCallback<void>): void;

  removeAbilityStageMonitorWithMonitorReturnsPromise(monitor: AbilityStageMonitor): Promise<void>;

  overload removeAbilityStageMonitor {
    removeAbilityStageMonitorWithMonitorCallback,
    removeAbilityStageMonitorWithMonitorReturnsPromise
  }

  removeAbilityStageMonitorSync(monitor: AbilityStageMonitor): void;

  waitAbilityStageMonitorWithMonitorCallback(monitor: AbilityStageMonitor,
    callback: AsyncCallback<AbilityStage>): void;

  waitAbilityStageMonitorWithMonitorTimeoutCallback(monitor: AbilityStageMonitor,
    timeout: long, callback: AsyncCallback<AbilityStage>): void;

  waitAbilityStageMonitorWithMonitorTimeoutReturnsPromise(monitor: AbilityStageMonitor,
    timeout?: long): Promise<AbilityStage>;

  overload waitAbilityStageMonitor {
    waitAbilityStageMonitorWithMonitorCallback,
    waitAbilityStageMonitorWithMonitorTimeoutCallback,
    waitAbilityStageMonitorWithMonitorTimeoutReturnsPromise
  }

  doAbilityForegroundWithAbilityCallback(ability: UIAbility, callback: AsyncCallback<void>): void;

  doAbilityForegroundWithAbilityReturnsPromise(ability: UIAbility): Promise<void>;

  overload doAbilityForeground {
    doAbilityForegroundWithAbilityCallback,
    doAbilityForegroundWithAbilityReturnsPromise
  }

  doAbilityBackgroundWithAbilityCallback(ability: UIAbility, callback: AsyncCallback<void>): void;

  doAbilityBackgroundWithAbilityReturnsPromise(ability: UIAbility): Promise<void>;

  overload doAbilityBackground {
    doAbilityBackgroundWithAbilityCallback,
    doAbilityBackgroundWithAbilityReturnsPromise
  }

  printWithMsgCallback(msg: string, callback: AsyncCallback<void>): void;

  printWithMsgReturnsPromise(msg: string): Promise<void>;

  overload print {
    printWithMsgCallback,
    printWithMsgReturnsPromise
  }

  getAbilityState(ability: UIAbility): int;

  setMockList(mockList: Record<string, string>): void;

  startAbilityWithWantCallback(want: Want, callback: AsyncCallback<void>): void;

  startAbilityWithWantReturnsPromise(want: Want): Promise<void>;

  overload startAbility {
    startAbilityWithWantCallback,
    startAbilityWithWantReturnsPromise
  }

  getCurrentTopAbilityWithCallback(callback: AsyncCallback<UIAbility>);

  getCurrentTopAbilityReturnsPromise(): Promise<UIAbility>;

  overload getCurrentTopAbility {
    getCurrentTopAbilityWithCallback,
    getCurrentTopAbilityReturnsPromise
  }
}

class AbilityDelegatorInner implements AbilityDelegator {
  public native nativeGetCurrentTopAbility(callback: AsyncCallbackWrapper<UIAbility>): UIAbility;

  public native printSync(msg: string): void;

  public native getAppContext(): Context;

  public native nativeFinishTest(msg: String, code: long, callback: AsyncCallbackWrapper<void>): void;

  public native nativeExecuteShellCommand(cmd: string, timeoutSecs: long,
    callback: AsyncCallbackWrapper<ShellCmdResult>): void;

  public native nativeAddAbilityMonitor(monitor: AbilityMonitor, callback: AsyncCallbackWrapper<void>): void;

  public native addAbilityMonitorSync(monitor: AbilityMonitor): void;

  public native removeAbilityMonitorAsync(monitor: AbilityMonitor, callback: AsyncCallbackWrapper<void>): void;

  public native removeAbilityMonitorSync(monitor: AbilityMonitor): void;

  public native waitAbilityMonitorAsync(monitor: AbilityMonitor, timeout: long,
    callback: AsyncCallbackWrapper<UIAbility>): void;

  public native addAbilityStageMonitorAsync(monitor: AbilityStageMonitor, callback: AsyncCallbackWrapper<void>): void;

  public native addAbilityStageMonitorSync(monitor: AbilityStageMonitor): void;

  public native removeAbilityStageMonitorAsync(monitor: AbilityStageMonitor,
    callback: AsyncCallbackWrapper<void>): void;

  public native removeAbilityStageMonitorSync(monitor: AbilityStageMonitor): void;

  public native waitAbilityStageMonitorAsync(monitor: AbilityStageMonitor, timeout: long,
    callback: AsyncCallbackWrapper<AbilityStage>): void;

  public native doAbilityForegroundAsync(ability: UIAbility, callback: AsyncCallbackWrapper<void>): void;

  public native doAbilityBackgroundAsync(ability: UIAbility, callback: AsyncCallbackWrapper<void>): void;

  public native printAsync(msg: string, callback: AsyncCallbackWrapper<void>): void;

  public native setMockList(mockList: Record<string, string>): void;

  public native getAbilityState(ability: UIAbility): int;

  public native nativeStartAbility(want: Want, callback: AsyncCallbackWrapper<void>): void;

  finishTestWithMsgCodeCallback(msg: string, code: long, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeFinishTest(msg, code, myCall);
    });
  }
  finishTestWithMsgCodeReturnsPromise(msg: string, code: long): Promise<void> {
    let p: Promise<void> =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeFinishTest(msg, code, myCall);
        });
      });
    return p;
  }

  overload finishTest {
    finishTestWithMsgCodeCallback,
    finishTestWithMsgCodeReturnsPromise
  }

  executeShellCommandWithCmdCallback(cmd: string, callback: AsyncCallback<ShellCmdResult, void>): void {
    let timeoutSecs = 0;
    let myCall = new AsyncCallbackWrapper<ShellCmdResult>(callback);
    taskpool.execute((): void => {
      this.nativeExecuteShellCommand(cmd, timeoutSecs, myCall);
    });
  }
  executeShellCommandWithCmdTimeoutSecsCallback(cmd: string, timeoutSecs: long,
    callback: AsyncCallback<ShellCmdResult, void>): void {
    let myCall = new AsyncCallbackWrapper<ShellCmdResult>(callback);
    taskpool.execute((): void => {
      this.nativeExecuteShellCommand(cmd, timeoutSecs, myCall);
    });
  }
  executeShellCommandWithCmdTimeoutSecsReturnsPromise(cmd: string, timeoutSecs?: long): Promise<ShellCmdResult> {
    let p = new Promise<ShellCmdResult>((resolve: (data: ShellCmdResult) => void,
      reject: (err: BusinessError) => void): void => {
      let myCall =
        new AsyncCallbackWrapper<ShellCmdResult>((err: BusinessError | null, data: ShellCmdResult | undefined) => {
          if (err == null || err.code == 0) {
            resolve(data as ShellCmdResult);
          } else {
            reject(err);
          }
        });
      taskpool.execute((): void => {
        if (timeoutSecs == null) {
          this.nativeExecuteShellCommand(cmd, 0, myCall);
        } else {
          this.nativeExecuteShellCommand(cmd, timeoutSecs as long, myCall);
        }
      });
    });
    return p;
  }

  overload executeShellCommand {
    executeShellCommandWithCmdCallback,
    executeShellCommandWithCmdTimeoutSecsCallback,
    executeShellCommandWithCmdTimeoutSecsReturnsPromise
  }

  addAbilityMonitorWithMonitorCallback(monitor: AbilityMonitor, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeAddAbilityMonitor(monitor, myCall);
    });
  }
  addAbilityMonitorWithMonitorReturnsPromise(monitor: AbilityMonitor): Promise<void> {
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
          this.nativeAddAbilityMonitor(monitor, myCall);
        });
      });
    return p;
  }

  overload addAbilityMonitor {
    addAbilityMonitorWithMonitorCallback,
    addAbilityMonitorWithMonitorReturnsPromise
  }

  removeAbilityMonitorWithMonitorCallback(monitor: AbilityMonitor, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.removeAbilityMonitorAsync(monitor, myCall);
    });
  }
  removeAbilityMonitorWithMonitorReturnsPromise(monitor: AbilityMonitor): Promise<void> {
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
          this.removeAbilityMonitorAsync(monitor, myCall);
        });
      });
    return p;
  }

  overload removeAbilityMonitor {
    removeAbilityMonitorWithMonitorCallback,
    removeAbilityMonitorWithMonitorReturnsPromise
  }

  waitAbilityMonitorWithMonitorCallback(monitor: AbilityMonitor, callback: AsyncCallback<UIAbility, void>): void {
    let timeout = -1;
    let myCall = new AsyncCallbackWrapper<UIAbility>(callback);
    taskpool.execute((): void => {
      this.waitAbilityMonitorAsync(monitor, timeout, myCall);
    });
  }
  waitAbilityMonitorWithMonitorTimeoutCallback(monitor: AbilityMonitor, timeout: long,
    callback: AsyncCallback<UIAbility, void>): void {
    let myCall = new AsyncCallbackWrapper<UIAbility>(callback);
    taskpool.execute((): void => {
      this.waitAbilityMonitorAsync(monitor, timeout, myCall);
    });
  }
  waitAbilityMonitorWithMonitorTimeoutReturnsPromise(monitor: AbilityMonitor, timeout?: long): Promise<UIAbility> {
    let p = new Promise<UIAbility>((resolve: (data: UIAbility) => void,
      reject: (err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<UIAbility>((err: BusinessError | null, data: UIAbility | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as UIAbility);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        if (timeout == null) {
          this.waitAbilityMonitorAsync(monitor, -1, myCall);
        } else {
          this.waitAbilityMonitorAsync(monitor, timeout as long, myCall);
        }
      });
    });
    return p;
  }

  overload waitAbilityMonitor {
    waitAbilityMonitorWithMonitorCallback,
    waitAbilityMonitorWithMonitorTimeoutCallback,
    waitAbilityMonitorWithMonitorTimeoutReturnsPromise
  }

  addAbilityStageMonitorWithMonitorCallback(monitor: AbilityStageMonitor, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.addAbilityStageMonitorAsync(monitor, myCall);
    });
  }
  addAbilityStageMonitorWithMonitorReturnsPromise(monitor: AbilityStageMonitor): Promise<void> {
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
          this.addAbilityStageMonitorAsync(monitor, myCall);
        });
      });
    return p;
  }

  overload addAbilityStageMonitor {
    addAbilityStageMonitorWithMonitorCallback,
    addAbilityStageMonitorWithMonitorReturnsPromise
  }

  removeAbilityStageMonitorWithMonitorCallback(monitor: AbilityStageMonitor, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.removeAbilityStageMonitorAsync(monitor, myCall);
    });
  }
  removeAbilityStageMonitorWithMonitorReturnsPromise(monitor: AbilityStageMonitor): Promise<void> {
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
          this.removeAbilityStageMonitorAsync(monitor, myCall);
        });
      });
    return p;
  }

  overload removeAbilityStageMonitor {
    removeAbilityStageMonitorWithMonitorCallback,
    removeAbilityStageMonitorWithMonitorReturnsPromise
  }

  waitAbilityStageMonitorWithMonitorCallback(monitor: AbilityStageMonitor,
    callback: AsyncCallback<AbilityStage, void>): void {
    let timeout = -1;
    let myCall = new AsyncCallbackWrapper<AbilityStage>(callback);
    taskpool.execute((): void => {
      this.waitAbilityStageMonitorAsync(monitor, timeout, myCall);
    });
  }
  waitAbilityStageMonitorWithMonitorTimeoutCallback(monitor: AbilityStageMonitor, timeout: long,
    callback: AsyncCallback<AbilityStage, void>): void {
    let myCall = new AsyncCallbackWrapper<AbilityStage>(callback);
    taskpool.execute((): void => {
      this.waitAbilityStageMonitorAsync(monitor, timeout, myCall);
    });
  }
  waitAbilityStageMonitorWithMonitorTimeoutReturnsPromise(monitor: AbilityStageMonitor,
    timeout?: long): Promise<AbilityStage> {
    let p = new Promise<AbilityStage>((resolve: (data: AbilityStage) => void,
      reject: (err: BusinessError) => void): void => {
      let myCall =
        new AsyncCallbackWrapper<AbilityStage>((err: BusinessError | null, data: AbilityStage | undefined) => {
          if (err == null || err.code == 0) {
            resolve(data as AbilityStage);
          } else {
            reject(err);
          }
        });
      taskpool.execute((): void => {
        if (timeout == null) {
          this.waitAbilityStageMonitorAsync(monitor, -1, myCall);
        } else {
          this.waitAbilityStageMonitorAsync(monitor, timeout as long, myCall);
        }
      });
    });
    return p;
  }

  overload waitAbilityStageMonitor {
    waitAbilityStageMonitorWithMonitorCallback,
    waitAbilityStageMonitorWithMonitorTimeoutCallback,
    waitAbilityStageMonitorWithMonitorTimeoutReturnsPromise
  }

  doAbilityForegroundWithAbilityCallback(ability: UIAbility, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.doAbilityForegroundAsync(ability, myCall);
    });
  }
  doAbilityForegroundWithAbilityReturnsPromise(ability: UIAbility): Promise<void> {
    let p: Promise<void> =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.doAbilityForegroundAsync(ability, myCall);
        });
      });
    return p;
  }

  overload doAbilityForeground {
    doAbilityForegroundWithAbilityCallback,
    doAbilityForegroundWithAbilityReturnsPromise
  }

  doAbilityBackgroundWithAbilityCallback(ability: UIAbility, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.doAbilityBackgroundAsync(ability, myCall);
    });
  }
  doAbilityBackgroundWithAbilityReturnsPromise(ability: UIAbility): Promise<void> {
    let p: Promise<void> =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.doAbilityBackgroundAsync(ability, myCall);
        });
      });
    return p;
  }

  overload doAbilityBackground {
    doAbilityBackgroundWithAbilityCallback,
    doAbilityBackgroundWithAbilityReturnsPromise
  }

  printWithMsgCallback(msg: string, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.printAsync(msg, myCall);
    });
  }
  printWithMsgReturnsPromise(msg: string): Promise<void> {
    let p: Promise<void> =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.printAsync(msg, myCall);
        });
      });
    return p;
  }

  overload print {
    printWithMsgCallback,
    printWithMsgReturnsPromise
  }

  startAbilityWithWantCallback(want: Want, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbility(want, myCall);
    });
  }
  startAbilityWithWantReturnsPromise(want: Want): Promise<void> {
    let p: Promise<void> =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeStartAbility(want, myCall);
        });
      });
    return p;
  }

  overload startAbility {
    startAbilityWithWantCallback,
    startAbilityWithWantReturnsPromise
  }

  getCurrentTopAbilityWithCallback(callback: AsyncCallback<UIAbility, void>): void {
    let getCurrentTopAbilityCall = new AsyncCallbackWrapper<UIAbility>(callback);
    let p = taskpool.execute((): UIAbility => {
      this.nativeGetCurrentTopAbility(getCurrentTopAbilityCall)
    });
    console.log("getCurrentTopAbility callback2_04");
  }
  getCurrentTopAbilityReturnsPromise(): Promise<UIAbility> {
    let p: Promise<UIAbility>  = new Promise<UIAbility>((resolve: (v: UIAbility) => void, reject: (error: Error) => void) => {
      console.log("getCurrentTopAbility promise1_01");
      let getCurrentTopAbilityCall = new AsyncCallbackWrapper<UIAbility>((err: BusinessError | null, data: UIAbility | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as UIAbility);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): UIAbility => {
        this.nativeGetCurrentTopAbility(getCurrentTopAbilityCall);
      });
    });
    return p;
  }

  overload getCurrentTopAbility {
    getCurrentTopAbilityWithCallback,
    getCurrentTopAbilityReturnsPromise
  }
}
