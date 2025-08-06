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

  finishTest(msg: string, code: long, callback: AsyncCallback<void>): void;

  finishTest(msg: string, code: long): Promise<void>;

  executeShellCommand(cmd: String, callback: AsyncCallback<ShellCmdResult>): void;

  executeShellCommand(cmd: String, timeoutSecs: long, callback: AsyncCallback<ShellCmdResult>): void;

  executeShellCommand(cmd: string, timeoutSecs?: long): Promise<ShellCmdResult>;

  addAbilityMonitor(monitor: AbilityMonitor, callback: AsyncCallback<void>): void;

  addAbilityMonitor(monitor: AbilityMonitor): Promise<void>;

  addAbilityMonitorSync(monitor: AbilityMonitor): void;

  removeAbilityMonitor(monitor: AbilityMonitor, callback: AsyncCallback<void>): void

  removeAbilityMonitor(monitor: AbilityMonitor): Promise<void>;

  removeAbilityMonitorSync(monitor: AbilityMonitor): void;

  waitAbilityMonitor(monitor: AbilityMonitor, callback: AsyncCallback<UIAbility>): void;

  waitAbilityMonitor(monitor: AbilityMonitor, timeout: long, callback: AsyncCallback<UIAbility>): void;

  waitAbilityMonitor(monitor: AbilityMonitor, timeout?: long): Promise<UIAbility>;

  addAbilityStageMonitor(monitor: AbilityStageMonitor, callback: AsyncCallback<void>): void;

  addAbilityStageMonitor(monitor: AbilityStageMonitor): Promise<void>;

  addAbilityStageMonitorSync(monitor: AbilityStageMonitor): void;

  removeAbilityStageMonitor(monitor: AbilityStageMonitor, callback: AsyncCallback<void>): void;

  removeAbilityStageMonitor(monitor: AbilityStageMonitor): Promise<void>;

  removeAbilityStageMonitorSync(monitor: AbilityStageMonitor): void;

  waitAbilityStageMonitor(monitor: AbilityStageMonitor, callback: AsyncCallback<AbilityStage>): void;

  waitAbilityStageMonitor(monitor: AbilityStageMonitor, timeout: long, callback: AsyncCallback<AbilityStage>): void;

  waitAbilityStageMonitor(monitor: AbilityStageMonitor, timeout?: long): Promise<AbilityStage>;

  doAbilityForeground(ability: UIAbility, callback: AsyncCallback<void>): void;

  doAbilityForeground(ability: UIAbility): Promise<void>;

  doAbilityBackground(ability: UIAbility, callback: AsyncCallback<void>): void;

  doAbilityBackground(ability: UIAbility): Promise<void>;

  print(msg: string, callback: AsyncCallback<void>): void;

  print(msg: string): Promise<void>;

  getAbilityState(ability: UIAbility): int;

  setMockList(mockList: Record<string, string>): void;

  startAbility(want: Want, callback: AsyncCallback<void>): void;

  startAbility(want: Want): Promise<void>;

  getCurrentTopAbility(callback: AsyncCallback<UIAbility>);

  getCurrentTopAbility(): Promise<UIAbility>;
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

  finishTest(msg: string, code: long, callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeFinishTest(msg, code, asyncCall);
    }).catch((err: BusinessError): void => {
      callback(err, undefined);
    });
  }
  finishTest(msg: string, code: long): Promise<void> {
    let p: Promise<void> =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let asyncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeFinishTest(msg, code, asyncCall);
        }).catch((err: Error): void => {
          reject(err as BusinessError);
        });
      });
    return p;
  }

  executeShellCommand(cmd: string, callback: AsyncCallback<ShellCmdResult, void>): void {
    let timeoutSecs = 0;
    let asyncCall = new AsyncCallbackWrapper<ShellCmdResult>(callback);
    taskpool.execute((): void => {
      this.nativeExecuteShellCommand(cmd, timeoutSecs, asyncCall);
    }).catch((err: BusinessError): void => {
      callback(err, undefined);
    });
  }
  executeShellCommand(cmd: string, timeoutSecs: long, callback: AsyncCallback<ShellCmdResult, void>): void {
    let asyncCall = new AsyncCallbackWrapper<ShellCmdResult>(callback);
    taskpool.execute((): void => {
      this.nativeExecuteShellCommand(cmd, timeoutSecs, asyncCall);
    }).catch((err: BusinessError): void => {
      callback(err, undefined);
    });
  }
  executeShellCommand(cmd: string, timeoutSecs?: long): Promise<ShellCmdResult> {
    let p = new Promise<ShellCmdResult>((resolve: (data: ShellCmdResult) => void,
      reject: (err: BusinessError) => void): void => {
      let asyncCall =
        new AsyncCallbackWrapper<ShellCmdResult>((err: BusinessError | null, data: ShellCmdResult | undefined) => {
          if (err == null || err.code == 0) {
            resolve(data as ShellCmdResult);
          } else {
            reject(err);
          }
        });
      taskpool.execute((): void => {
        if (timeoutSecs == null) {
          this.nativeExecuteShellCommand(cmd, 0, asyncCall);
        } else {
          this.nativeExecuteShellCommand(cmd, timeoutSecs as long, asyncCall);
        }
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
    return p;
  }

  addAbilityMonitor(monitor: AbilityMonitor, callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeAddAbilityMonitor(monitor, asyncCall);
    }).catch((err: BusinessError): void => {
      callback(err, undefined);
    });
  }
  addAbilityMonitor(monitor: AbilityMonitor): Promise<void> {
    let p =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let asyncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeAddAbilityMonitor(monitor, asyncCall);
        }).catch((err: Error): void => {
          reject(err as BusinessError);
        });
      });
    return p;
  }

  removeAbilityMonitor(monitor: AbilityMonitor, callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.removeAbilityMonitorAsync(monitor, asyncCall);
    }).catch((err: BusinessError): void => {
      callback(err, undefined);
    });
  }
  removeAbilityMonitor(monitor: AbilityMonitor): Promise<void> {
    let p =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let asyncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.removeAbilityMonitorAsync(monitor, asyncCall);
        }).catch((err: Error): void => {
          reject(err as BusinessError);
        });
      });
    return p;
  }

  waitAbilityMonitor(monitor: AbilityMonitor, callback: AsyncCallback<UIAbility, void>): void {
    let timeout = -1;
    let asyncCall = new AsyncCallbackWrapper<UIAbility>(callback);
    taskpool.execute((): void => {
      this.waitAbilityMonitorAsync(monitor, timeout, asyncCall);
    }).catch((err: BusinessError): void => {
      callback(err, undefined);
    });
  }
  waitAbilityMonitor(monitor: AbilityMonitor, timeout: long, callback: AsyncCallback<UIAbility, void>): void {
    let asyncCall = new AsyncCallbackWrapper<UIAbility>(callback);
    taskpool.execute((): void => {
      this.waitAbilityMonitorAsync(monitor, timeout, asyncCall);
    }).catch((err: BusinessError): void => {
      callback(err, undefined);
    });
  }
  waitAbilityMonitor(monitor: AbilityMonitor, timeout?: long): Promise<UIAbility> {
    let p = new Promise<UIAbility>((resolve: (data: UIAbility) => void,
      reject: (err: BusinessError) => void): void => {
      let asyncCall = new AsyncCallbackWrapper<UIAbility>((err: BusinessError | null, data: UIAbility | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as UIAbility);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        if (timeout == null) {
          this.waitAbilityMonitorAsync(monitor, -1, asyncCall);
        } else {
          this.waitAbilityMonitorAsync(monitor, timeout as long, asyncCall);
        }
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
    return p;
  }

  addAbilityStageMonitor(monitor: AbilityStageMonitor, callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.addAbilityStageMonitorAsync(monitor, asyncCall);
    }).catch((err: BusinessError): void => {
      callback(err, undefined);
    });
  }
  addAbilityStageMonitor(monitor: AbilityStageMonitor): Promise<void> {
    let p =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let asyncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.addAbilityStageMonitorAsync(monitor, asyncCall);
        }).catch((err: Error): void => {
          reject(err as BusinessError);
        });
      });
    return p;
  }

  removeAbilityStageMonitor(monitor: AbilityStageMonitor, callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.removeAbilityStageMonitorAsync(monitor, asyncCall);
    }).catch((err: BusinessError): void => {
      callback(err, undefined);
    });
  }
  removeAbilityStageMonitor(monitor: AbilityStageMonitor): Promise<void> {
    let p =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let asyncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.removeAbilityStageMonitorAsync(monitor, asyncCall);
        }).catch((err: Error): void => {
          reject(err as BusinessError);
        });
      });
    return p;
  }

  waitAbilityStageMonitor(monitor: AbilityStageMonitor, callback: AsyncCallback<AbilityStage, void>): void {
    let timeout = -1;
    let asyncCall = new AsyncCallbackWrapper<AbilityStage>(callback);
    taskpool.execute((): void => {
      this.waitAbilityStageMonitorAsync(monitor, timeout, asyncCall);
    }).catch((err: BusinessError): void => {
      callback(err, undefined);
    });
  }
  waitAbilityStageMonitor(monitor: AbilityStageMonitor, timeout: long,
    callback: AsyncCallback<AbilityStage, void>): void {
    let asyncCall = new AsyncCallbackWrapper<AbilityStage>(callback);
    taskpool.execute((): void => {
      this.waitAbilityStageMonitorAsync(monitor, timeout, asyncCall);
    }).catch((err: BusinessError): void => {
      callback(err, undefined);
    });
  }
  waitAbilityStageMonitor(monitor: AbilityStageMonitor, timeout?: long): Promise<AbilityStage> {
    let p = new Promise<AbilityStage>((resolve: (data: AbilityStage) => void,
      reject: (err: BusinessError) => void): void => {
      let asyncCall =
        new AsyncCallbackWrapper<AbilityStage>((err: BusinessError | null, data: AbilityStage | undefined) => {
          if (err == null || err.code == 0) {
            resolve(data as AbilityStage);
          } else {
            reject(err);
          }
        });
      taskpool.execute((): void => {
        if (timeout == null) {
          this.waitAbilityStageMonitorAsync(monitor, -1, asyncCall);
        } else {
          this.waitAbilityStageMonitorAsync(monitor, timeout as long, asyncCall);
        }
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
    return p;
  }

  doAbilityForeground(ability: UIAbility, callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.doAbilityForegroundAsync(ability, asyncCall);
    }).catch((err: BusinessError): void => {
      callback(err, undefined);
    });
  }
  doAbilityForeground(ability: UIAbility): Promise<void> {
    let p: Promise<void> =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let asyncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.doAbilityForegroundAsync(ability, asyncCall);
        }).catch((err: Error): void => {
          reject(err as BusinessError);
        });
      });
    return p;
  }

  doAbilityBackground(ability: UIAbility, callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.doAbilityBackgroundAsync(ability, asyncCall);
    }).catch((err: BusinessError): void => {
      callback(err, undefined);
    });
  }
  doAbilityBackground(ability: UIAbility): Promise<void> {
    let p: Promise<void> =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let asyncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.doAbilityBackgroundAsync(ability, asyncCall);
        }).catch((err: Error): void => {
          reject(err as BusinessError);
        });
      });
    return p;
  }

  print(msg: string, callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.printAsync(msg, asyncCall);
    }).catch((err: BusinessError): void => {
      callback(err, undefined);
    });
  }
  print(msg: string): Promise<void> {
    let p: Promise<void> =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let asyncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.printAsync(msg, asyncCall);
        }).catch((err: Error): void => {
          reject(err as BusinessError);
        });
      });
    return p;
  }

  startAbility(want: Want, callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbility(want, asyncCall);
    }).catch((err: BusinessError): void => {
      callback(err, undefined);
    });
  }
  startAbility(want: Want): Promise<void> {
    let p: Promise<void> =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let asyncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeStartAbility(want, asyncCall);
        }).catch((err: Error): void => {
          reject(err as BusinessError);
        });
      });
    return p;
  }

  getCurrentTopAbility(callback: AsyncCallback<UIAbility, void>): void {
    let getCurrentTopAbilityCall = new AsyncCallbackWrapper<UIAbility>(callback);
    let p = taskpool.execute((): UIAbility => {
      this.nativeGetCurrentTopAbility(getCurrentTopAbilityCall)
    }).catch((err: BusinessError): void => {
      callback(err, undefined);
    });
    console.log("getCurrentTopAbility callback2_04");
  }
  getCurrentTopAbility(): Promise<UIAbility> {
    let p: Promise<UIAbility> =
      new Promise<UIAbility>((resolve: (v: UIAbility) => void, reject: (error: Error) => void) => {
        console.log("getCurrentTopAbility promise1_01");
        let getCurrentTopAbilityCall =
          new AsyncCallbackWrapper<UIAbility>((err: BusinessError | null, data: UIAbility | undefined) => {
            if (err == null || err.code == 0) {
              resolve(data as UIAbility);
            } else {
              reject(err);
            }
          });
        taskpool.execute((): UIAbility => {
          this.nativeGetCurrentTopAbility(getCurrentTopAbilityCall);
        }).catch((err: Error): void => {
          reject(err as BusinessError);
        });
      });
    return p;
  }
}
