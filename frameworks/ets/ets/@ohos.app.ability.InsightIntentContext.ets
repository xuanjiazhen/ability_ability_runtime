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

import { BusinessError, AsyncCallback } from '@ohos.base';
import AsyncCallbackWrapper from './utils/AbilityUtils';
import insightIntent from './@ohos.app.ability.insightIntent';
import Want from '@ohos.app.ability.Want';


import InsightIntentExecutor from '@ohos.app.ability.insightIntentDriver.ets'


export default class InsightIntentContext {
  static {
    loadLibrary("insight_intent_context_ani_kit.z");
  }
  nativeContext: long;

  private static native nativeTransferStatic(input: ESValue, type: string): Object;

  private static native nativeTransferDynamic(input: Object): ESValue;

  private static contextType: string = 'InsightIntentContext';

  static transferStatic(input: Any): Object {
    let type: string = '';
    try {
      type = ESValue.wrap(input).getProperty('__context_impl__').getProperty('contextType').toString();
    } catch (err) {
      console.log('transferStatic err: ' + err.toString());
    }
    let contextImpl : ESValue = ESValue.wrap(input).getProperty('__context_impl__');
    console.log('transferStatic type: ' + type);
    return InsightIntentContext.nativeTransferStatic(contextImpl, type);
  }

  static transferDynamic(input: Object): Any {
    return InsightIntentContext.nativeTransferDynamic(input).unwrap();
  }

  public native nativeStartAbilitySync(want: Want): BusinessError;
  public native nativeDelayInsightIntentResult(): int;
  public native nativeSendInsightIntentResult(insightIntentId: int): void;
  public native nativeBackToCallerExecuteResult(insightIntentId: int, result:insightIntent.ExecuteResult): Promise<void>;

  startAbility(want: Want, callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): BusinessError => {
      return this.nativeStartAbilitySync(want);
    });
    p1.then((e: NullishType) => {
      let retError = e as BusinessError;
      callback(retError, undefined);
    }, (err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  startAbility(want: Want): Promise<void> {
    let p = new Promise<void>((resolve: (v: undefined) => void, reject: (error: BusinessError) => void): void => {
      let p1 = taskpool.execute((): BusinessError => {
        return this.nativeStartAbilitySync(want);
      });
      p1.then((e: NullishType) => {
        let retError = e as BusinessError;
        if (retError.code === 0) {
          resolve(undefined);
        } else {
          reject(retError);
        }
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
    return p;
  }

  delayInsightIntentResult() : int {
    return this.nativeDelayInsightIntentResult();
  }

  sendInsightIntentResult(insightIntentId: int) : void {
    return this.nativeSendInsightIntentResult(insightIntentId);
  }

  backToCallerExecuteResult(insightIntentId: int, result:insightIntent.ExecuteResult) {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeBackToCallerExecuteResult(insightIntentId, result);
      });
    });
  }
}