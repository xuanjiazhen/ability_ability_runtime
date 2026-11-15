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

import window from '@ohos.window';
import insightIntent from '@ohos.app.ability.insightIntent';
import InsightIntentContext from '@ohos.app.ability.InsightIntentContext';
import type UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import { RecordData } from '@ohos.base';

export default class InsightIntentExecutor {
  context: InsightIntentContext = {};
  public nativeExecutor: long = 0;
  public isOnExecuteInUIAbilityForegroundModeAsync: boolean = true;
  public isOnExecuteInUIAbilityBackgroundModeAsync: boolean = true;
  public isOnExecuteInUIExtensionAbilityAsync: boolean = true;
  public isOnExecuteInServiceExtensionAbilityAsync: boolean = true;

  public native nativeOnExecuteResult(data: insightIntent.ExecuteResult): void;

  public callOnExecuteInUIAbilityForegroundMode(name: string, param: Record<string, RecordData>,
    pageLoader: window.WindowStage, executionContext?: InsightIntentExecutionContext): insightIntent.ExecuteResult {
    let p: insightIntent.ExecuteResult | Promise<insightIntent.ExecuteResult>;
    if (executionContext === undefined) {
      p = this.onExecuteInUIAbilityForegroundMode(name, param, pageLoader);
    } else {
      p = this.onExecuteInUIAbilityForegroundMode(name, param, pageLoader, executionContext);
    }
    if (p instanceof insightIntent.ExecuteResult) {
      this.isOnExecuteInUIAbilityForegroundModeAsync = false;
      return p;
    } else {
      this.isOnExecuteInUIAbilityForegroundModeAsync = true;
      p.then((result: insightIntent.ExecuteResult): void => {
        this.nativeOnExecuteResult(result);
      });
      let ret: insightIntent.ExecuteResult = {code: 0};
      return ret;
    }
  }

  public callOnExecuteInUIAbilityBackgroundMode(name: string, param: Record<string, RecordData>,
    executionContext?: InsightIntentExecutionContext):
    insightIntent.ExecuteResult {
    let p: insightIntent.ExecuteResult | Promise<insightIntent.ExecuteResult>;
    if (executionContext === undefined) {
      p = this.onExecuteInUIAbilityBackgroundMode(name, param);
    } else {
      p = this.onExecuteInUIAbilityBackgroundMode(name, param, executionContext);
    }
    if (p instanceof insightIntent.ExecuteResult) {
      this.isOnExecuteInUIAbilityBackgroundModeAsync = false;
      return p;
    } else {
      this.isOnExecuteInUIAbilityBackgroundModeAsync = true;
      p.then<void>((result: insightIntent.ExecuteResult): void => {
        this.nativeOnExecuteResult(result);
      });
      let ret: insightIntent.ExecuteResult = {code: 0};
      return ret;
    }
  }

  public callOnExecuteInUIExtensionAbility(name: string, param: Record<string, RecordData>,
    pageLoader: UIExtensionContentSession, executionContext?: InsightIntentExecutionContext):
    insightIntent.ExecuteResult {
    let p: insightIntent.ExecuteResult | Promise<insightIntent.ExecuteResult>;
    if (executionContext === undefined) {
      p = this.onExecuteInUIExtensionAbility(name, param, pageLoader);
    } else {
      p = this.onExecuteInUIExtensionAbility(name, param, pageLoader, executionContext);
    }
    if (p instanceof insightIntent.ExecuteResult) {
      this.isOnExecuteInUIExtensionAbilityAsync = false;
      return p;
    } else {
      this.isOnExecuteInUIExtensionAbilityAsync = true;
      p.then<void>((result: insightIntent.ExecuteResult): void => {
        this.nativeOnExecuteResult(result);
      });
      let ret: insightIntent.ExecuteResult = {code: 0};
      return ret;
    }
  }

  public callOnExecuteInServiceExtensionAbility(name: string, param: Record<string, RecordData>,
    executionContext?: InsightIntentExecutionContext):
    insightIntent.ExecuteResult {
    let p: insightIntent.ExecuteResult | Promise<insightIntent.ExecuteResult>;
    if (executionContext === undefined) {
      p = this.onExecuteInServiceExtensionAbility(name, param);
    } else {
      p = this.onExecuteInServiceExtensionAbility(name, param, executionContext);
    }
    if (p instanceof insightIntent.ExecuteResult) {
      this.isOnExecuteInServiceExtensionAbilityAsync = false;
      return p;
    } else {
      this.isOnExecuteInServiceExtensionAbilityAsync = true;
      p.then<void>((result: insightIntent.ExecuteResult): void => {
        this.nativeOnExecuteResult(result);
      });
      let ret: insightIntent.ExecuteResult = {code: 0};
      return ret;
    }
  }

  onExecuteInUIAbilityForegroundMode(name: string, param: Record<string, RecordData>, pageLoader: window.WindowStage):
    insightIntent.ExecuteResult | Promise<insightIntent.ExecuteResult> {
    let ret: insightIntent.ExecuteResult = {code: 0};
    return ret;
  }

  onExecuteInUIAbilityForegroundMode(name: string, param: Record<string, RecordData>, pageLoader: window.WindowStage,
    executionContext: InsightIntentExecutionContext):
    insightIntent.ExecuteResult | Promise<insightIntent.ExecuteResult> {
    let ret: insightIntent.ExecuteResult = {code: 0};
    return ret;
  }

  onExecuteInUIAbilityBackgroundMode(name: string, param: Record<string, RecordData>):
    insightIntent.ExecuteResult | Promise<insightIntent.ExecuteResult> {
    let ret: insightIntent.ExecuteResult = {code: 0};
    return ret;
  }

  onExecuteInUIAbilityBackgroundMode(name: string, param: Record<string, RecordData>,
    executionContext: InsightIntentExecutionContext):
    insightIntent.ExecuteResult | Promise<insightIntent.ExecuteResult> {
    let ret: insightIntent.ExecuteResult = {code: 0};
    return ret;
  }

  onExecuteInUIExtensionAbility(name: string, param: Record<string, RecordData>, pageLoader: UIExtensionContentSession):
    insightIntent.ExecuteResult | Promise<insightIntent.ExecuteResult> {
    let ret: insightIntent.ExecuteResult = {code: 0};
    return ret;
  }

  onExecuteInUIExtensionAbility(name: string, param: Record<string, RecordData>, pageLoader: UIExtensionContentSession,
    executionContext: InsightIntentExecutionContext):
    insightIntent.ExecuteResult | Promise<insightIntent.ExecuteResult> {
    let ret: insightIntent.ExecuteResult = {code: 0};
    return ret;
  }

  onExecuteInServiceExtensionAbility(name: string, param: Record<string, RecordData>):
    insightIntent.ExecuteResult | Promise<insightIntent.ExecuteResult> {
    let ret: insightIntent.ExecuteResult = {code: 0};
    return ret;
  }

  onExecuteInServiceExtensionAbility(name: string, param: Record<string, RecordData>,
    executionContext: InsightIntentExecutionContext):
    insightIntent.ExecuteResult | Promise<insightIntent.ExecuteResult> {
    let ret: insightIntent.ExecuteResult = {code: 0};
    return ret;
  }
}

export class InsightIntentCancellationSignal {
  nativeSignal: long = 0;
  public native isCancelled(): boolean;
  public native setOnCancelListener(listener: insightIntent.CancelListenerCallback): void;
}

export class InsightIntentExecutionContext {
  readonly cancellationSignal: InsightIntentCancellationSignal = new InsightIntentCancellationSignal();
}