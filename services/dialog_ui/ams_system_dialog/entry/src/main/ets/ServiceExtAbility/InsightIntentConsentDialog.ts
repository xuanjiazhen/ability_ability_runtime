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

import UIExtensionAbility from '@ohos.app.ability.UIExtensionAbility';
import type UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import type Want from '@ohos.app.ability.Want';

const TAG = 'InsightIntentConsentDialog';

const PARAM_CALLER_BUNDLE = 'ohos.insightIntent.consent.callerBundle';
const PARAM_TARGET_BUNDLE = 'ohos.insightIntent.consent.targetBundle';
const PARAM_TARGET_INTENT = 'ohos.insightIntent.consent.targetIntent';

/**
 * System-fixed consent dialog for insight intent invocation. The host (invokeFunction)
 * mounts this sys/commonUI modal UIExtension on its own window to confirm, with the
 * user, that the caller may invoke the target capability. The decision is written
 * back through terminateSelfWithResult under "ohos.insightIntent.consent.result".
 */
export default class InsightIntentConsentDialogAbility extends UIExtensionAbility {
  onCreate(): void {
    console.info(TAG, 'onCreate');
  }

  onSessionCreate(want: Want, session: UIExtensionContentSession): void {
    console.info(TAG, 'onSessionCreate');
    const storage = new LocalStorage({
      'session': session,
      'callerBundle': (want.parameters?.[PARAM_CALLER_BUNDLE] as string) ?? '',
      'targetBundle': (want.parameters?.[PARAM_TARGET_BUNDLE] as string) ?? '',
      'targetIntent': (want.parameters?.[PARAM_TARGET_INTENT] as string) ?? ''
    });
    session.loadContent('pages/insightIntentConsentDialog', storage);
    session.setWindowBackgroundColor('#00000000');
  }

  onSessionDestroy(session: UIExtensionContentSession): void {
    console.info(TAG, 'onSessionDestroy');
  }

  onForeground(): void {
    console.info(TAG, 'onForeground');
  }

  onBackground(): void {
    console.info(TAG, 'onBackground');
  }

  onDestroy(): void {
    console.info(TAG, 'onDestroy');
  }
}
