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

import { AbilityConstant, ConfigurationConstant, errorManager, UIAbility, Want } from '@kit.AbilityKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import { window } from '@kit.ArkUI';
import { BusinessError } from '@kit.BasicServicesKit';

const DOMAIN = 0x1323;

export default class EntryAbility extends UIAbility {
  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam): void {
    this.context.getApplicationContext().setColorMode(ConfigurationConstant.ColorMode.COLOR_MODE_NOT_SET);
    hilog.info(DOMAIN, 'InsightIntents', '%{public}s', 'Ability onCreate');

    let observer1: errorManager.ErrorObserver = {
      onUnhandledException(errorMsg) {
        console.info('onUnhandledException, errorMsg: ', errorMsg);
      },
      onException(errorObj) {
        console.info('onException, name: ', errorObj.name);
        console.info('onException, message: ', errorObj.message);
        if (typeof (errorObj.stack) === 'string') {
          console.info('onException, stack: ', errorObj.stack);
        }
      }
    };
    let observerId = -1;
    try {
      observerId = errorManager.on('error', observer1);
    } catch (paramError) {
      let code = (paramError as BusinessError).code;
      let message = (paramError as BusinessError).message;
      console.error(`error: ${code}, ${message}`);
    }

    let observer2: errorManager.UnhandledRejectionObserver = (reason: Error, promise: Promise<void>) => {
      console.info('reason.name: ', reason.name);
      console.info('reason.message: ', reason.message);
      if (reason.stack) {
        console.info('reason.stack: ', reason.stack);
      }
    };
    errorManager.on('unhandledRejection', observer2);
  }

  onDestroy(): void {
    hilog.info(DOMAIN, 'InsightIntents', '%{public}s', 'Ability onDestroy');
  }

  onWindowStageCreate(windowStage: window.WindowStage): void {

    let windowClass: window.Window;
    windowStage.getMainWindow((err, data) => {
      windowClass = data;
      windowClass.setWindowLayoutFullScreen(true).then(() => {
        hilog.info(DOMAIN, 'InsightIntents', 'Set Window Layout Full Screen.');
      })

      windowStage.loadContent('pages/Index', (err) => {
        if (err.code) {
          hilog.error(DOMAIN, 'InsightIntents', 'Failed to load the content. Cause: %{public}s', JSON.stringify(err));
          return;
        }
        hilog.info(DOMAIN, 'InsightIntents', 'Succeeded in loading the content.');
      });
    });
  }

  onWindowStageDestroy(): void {
    hilog.info(DOMAIN, 'InsightIntents', '%{public}s', 'Ability onWindowStageDestroy');
  }

  onForeground(): void {
    hilog.info(DOMAIN, 'InsightIntents', '%{public}s', 'Ability onForeground');
  }

  onBackground(): void {
    hilog.info(DOMAIN, 'InsightIntents', '%{public}s', 'Ability onBackground');
  }
}