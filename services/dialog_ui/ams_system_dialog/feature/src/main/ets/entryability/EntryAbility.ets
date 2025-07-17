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

function errorFunc(observer: errorManager.GlobalError) {
  hilog.error(DOMAIN, 'InsightIntents', 'ERROR FUNCTION, result name :', observer.name);
  hilog.error(DOMAIN, 'InsightIntents', 'ERROR FUNCTION, result message :', observer.message);
  hilog.error(DOMAIN, 'InsightIntents', 'ERROR FUNCTION, result stack :', observer.stack);
  hilog.error(DOMAIN, 'InsightIntents', 'ERROR FUNCTION, result instanceName :', observer.instanceName);
  hilog.error(DOMAIN, 'InsightIntents', 'ERROR FUNCTION, result instanceType :', observer.instanceType);
}

function promiseFunc(observer: errorManager.GlobalError) {
  hilog.error(DOMAIN, 'InsightIntents', 'PROMISE FUNCTION, result name :', observer.name);
  hilog.error(DOMAIN, 'InsightIntents', 'PROMISE FUNCTION, result message :', observer.message);
  hilog.error(DOMAIN, 'InsightIntents', 'PROMISE FUNCTION, result stack :', observer.stack);
  hilog.error(DOMAIN, 'InsightIntents', 'PROMISE FUNCTION, result instanceName :', observer.instanceName);
  hilog.error(DOMAIN, 'InsightIntents', 'PROMISE FUNCTION, result instanceType :', observer.instanceType);
}

export default class EntryAbility extends UIAbility {
  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam): void {
    this.context.getApplicationContext().setColorMode(ConfigurationConstant.ColorMode.COLOR_MODE_NOT_SET);
    hilog.info(DOMAIN, 'InsightIntents', '%{public}s', 'Ability onCreate');

    errorManager.on('globalErrorOccurred', errorFunc);
    errorManager.on('globalUnhandledRejectionDetected', promiseFunc);
  }

  onDestroy(): void {
    hilog.info(DOMAIN, 'InsightIntents', '%{public}s', 'Ability onDestroy');

    errorManager.off('globalErrorOccurred', errorFunc);
    errorManager.off('globalUnhandledRejectionDetected', promiseFunc);
  }

  onWindowStageCreate(windowStage: window.WindowStage): void {
    try {
      let windowClass: window.Window;
      windowStage.getMainWindow((err, data) => {
        windowClass = data;
        windowClass.setWindowLayoutFullScreen(true).then(() => {
          hilog.info(DOMAIN, 'InsightIntents', 'Set Window Layout Full Screen.');

          try {
            let navigationHeight = windowClass.getWindowAvoidArea(window.AvoidAreaType.TYPE_SYSTEM).bottomRect.height;
            PersistentStorage.PersistProps([{
              key: 'navigationHeight', defaultValue: navigationHeight
            }]);
          } catch (err) {
            hilog.error(DOMAIN, 'InsightIntents', 'Failed to getWindowAvoidArea.', JSON.stringify(err));
          }
        })

        windowStage.loadContent('pages/Index', (err) => {
          if (err.code) {
            hilog.error(DOMAIN, 'InsightIntents', 'Failed to load the content. Cause: %{public}s', JSON.stringify(err));
            return;
          }
          hilog.info(DOMAIN, 'InsightIntents', 'Succeeded in loading the content.');
        });
      });
    } catch (err) {
      hilog.error(DOMAIN, 'InsightIntents', 'Failed to getMainWindow.');
    }
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