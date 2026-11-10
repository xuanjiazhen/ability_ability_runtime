/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

import type UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import Want from '@ohos.app.ability.Want';
import type { BusinessError } from '@ohos.base';

let storage = LocalStorage.GetShared();
const TAG: string = 'InsightIntentConsentDialog_Page';
const CONSENT_RESULT: string = 'ohos.insightIntent.consent.result';

@Entry(storage)
@Component
struct InsightIntentConsentDialog {
  @State private callerBundle: string = '';
  @State private targetBundle: string = '';
  @State private targetIntent: string = '';

  aboutToAppear() {
    console.info(TAG, 'dialog page appears');
    this.callerBundle = storage.get<string>('callerBundle') ?? '';
    this.targetBundle = storage.get<string>('targetBundle') ?? '';
    this.targetIntent = storage.get<string>('targetIntent') ?? '';
  }

  settle(granted: boolean) {
    console.info(TAG, `settle called, granted: ${granted}`);
    try {
      const session: UIExtensionContentSession = storage.get<UIExtensionContentSession>('session');
      const resultWant: Want = new Want();
      resultWant.parameters = {};
      resultWant.parameters[CONSENT_RESULT] = granted;
      session.terminateSelfWithResult({ resultCode: 0, want: resultWant }).then(() => {
        console.info(TAG, 'terminateSelfWithResult success');
      }).catch((err: BusinessError) => {
        console.error(TAG, `terminateSelfWithResult failed, error: ${JSON.stringify(err)}`);
      });
    } catch (error) {
      console.error(TAG, `try terminateSelfWithResult failed, error: ${JSON.stringify(error)}`);
    }
  }

  onAllow() {
    console.info(TAG, 'user allowed');
    this.settle(true);
  }

  onDeny() {
    console.info(TAG, 'user denied');
    this.settle(false);
  }

  build() {
    Column(
      AlertDialog.show(
        {
          title: '授权确认',
          message: `应用 ${this.callerBundle} 请求调用能力 ${this.targetIntent}`,
          autoCancel: false,
          alignment: DialogAlignment.Center,
          gridCount: 4,
          buttonDirection: DialogButtonDirection.HORIZONTAL,
          buttons: [
            {
              enabled: true,
              defaultFocus: false,
              style: DialogButtonStyle.DEFAULT,
              value: '拒绝',
              action: this.onDeny
            },
            {
              enabled: true,
              defaultFocus: true,
              style: DialogButtonStyle.HIGHLIGHT,
              value: '允许',
              fontColor: $r('sys.color.ohos_id_color_foreground_contrary'),
              backgroundColor: $r('app.color.button_color'),
              action: this.onAllow
            }
          ]
        }
      )
    ) {}
  }
}