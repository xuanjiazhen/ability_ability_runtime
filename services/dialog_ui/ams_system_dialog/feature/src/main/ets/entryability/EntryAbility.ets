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

import { AbilityConstant, application, Configuration, ConfigurationConstant, errorManager, UIAbility, Want } from '@kit.AbilityKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import { window } from '@kit.ArkUI';
import { BusinessError } from '@kit.BasicServicesKit';
import { TAG, DOMAIN} from '../pages/Index'
import { settings } from '@kit.BasicServicesKit';

function errorHandlerFunc(observer: errorManager.GlobalError) {
  hilog.error(DOMAIN, TAG, 'result name: %{public}s', observer.name);
  hilog.error(DOMAIN, TAG, 'result message: %{public}s', observer.message);
  if (typeof (observer.stack) === 'string') {
      hilog.error(DOMAIN, TAG, 'result stack: %{public}s', observer.stack);
  }
  hilog.error(DOMAIN, TAG, 'result instanceName: %{public}s', observer.instanceName);
  hilog.error(DOMAIN, TAG, 'result instanceType: %{public}s', observer.instanceType);
  restartApp()
}

let errorObserver: errorManager.ErrorObserver = {
  onUnhandledException(errorMsg) {
    hilog.error(DOMAIN, TAG, 'onUnhandledException, errorMsg: %{public}s', errorMsg);
    restartApp()
  },
  onException(errorObj) {
    hilog.error(DOMAIN, TAG, 'result name: %{public}s', errorObj.name);
    hilog.error(DOMAIN, TAG, 'result message: %{public}s', errorObj.message);
    if (typeof (errorObj.stack) === 'string') {
      hilog.error(DOMAIN, TAG, 'result stack: %{public}s', errorObj.stack);
    }
    restartApp()
  }
};

const DEFAULT_OBSERVER_ID = -1;
let observerId = DEFAULT_OBSERVER_ID;
let restartWant: Want | undefined = undefined;
function restartApp() {
  try {
    application.getApplicationContext().restartApp(restartWant);
    hilog.info(DOMAIN, TAG, 'restartApp success');
  } catch (err) {
    hilog.error(DOMAIN, TAG, 'Failed to restartApp. Cause: %{public}s', JSON.stringify(err));
  }
}

export default class EntryAbility extends UIAbility {
  private colorMode: ConfigurationConstant.ColorMode = ConfigurationConstant.ColorMode.COLOR_MODE_NOT_SET;
  private mainWindow: window.Window | null = null;

  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam): void {
    this.context.getApplicationContext().setColorMode(ConfigurationConstant.ColorMode.COLOR_MODE_NOT_SET);
    hilog.info(DOMAIN, TAG, '%{public}s', 'Ability onCreate');
    restartWant = want;
    const INSIGHT_INTENT_DEBUG = 'ai_insightintent_debug'
    let settingStatus = 'default';
    try {
      settingStatus = settings.getValueSync(this.context, INSIGHT_INTENT_DEBUG, 'default', settings.domainName.USER_SECURITY);
    } catch (err) {
      hilog.error(DOMAIN, TAG, 'Failed to getValueSync. Cause: %{public}s', JSON.stringify(err));
    }
    if (settingStatus !== 'true') {
      try {
        this.context.terminateSelf();
      } catch (err) {
        hilog.error(DOMAIN, TAG, 'Failed to context.terminateSelf. Cause: %{public}s', JSON.stringify(err));
      }
    }
    try {
      observerId = errorManager.on('error', errorObserver);
      errorManager.on('globalErrorOccurred', errorHandlerFunc);
    } catch (error) {
      hilog.error(DOMAIN, TAG, 'registerErrorManager failed, Cause: %{public}s', JSON.stringify(error));
    }
  }

  onDestroy(): void {
    hilog.info(DOMAIN, TAG, '%{public}s', 'Ability onDestroy');
    try {
      if (observerId != DEFAULT_OBSERVER_ID) {
        errorManager.off('error', observerId);
      }
      errorManager.off('globalErrorOccurred', errorHandlerFunc);
    } catch (error) {
      hilog.error(DOMAIN, TAG, 'cancelErrorManager failed, Cause: %{public}s', JSON.stringify(error));
    }
  }

  onWindowStageCreate(windowStage: window.WindowStage): void {
    this.colorMode = this.context.config.colorMode;
    hilog.info(DOMAIN, TAG, 'onWindowStageCreate: colorMode=%{public}d', this.colorMode);
    try {
      windowStage.getMainWindow((err, data) => {
        if (err && err.code !== 0) {
          hilog.error(DOMAIN, TAG, 'Failed to get main window. code=%{public}d, Cause: %{public}s',
            err.code, JSON.stringify(err.message ?? err));
          return;
        }
        this.mainWindow = data;
        const avoidArea = data.getWindowAvoidArea(window.AvoidAreaType.TYPE_SYSTEM);
        AppStorage.setOrCreate('statusBarHeight', avoidArea.topRect.height);
        AppStorage.setOrCreate('navBarHeight', avoidArea.bottomRect.height);
        hilog.info(DOMAIN, TAG, 'avoid area: statusBarHeight=%{public}d, navBarHeight=%{public}d',
          avoidArea.topRect.height, avoidArea.bottomRect.height);
        this.logWindowState(data, 'after_getMainWindow');
        hilog.info(DOMAIN, TAG, 'getMainWindow success, set full-screen before loadContent.');
        data.setWindowLayoutFullScreen(true).then(() => {
          hilog.info(DOMAIN, TAG, 'setWindowLayoutFullScreen success.');
          this.logWindowState(data, 'after_setFullScreen');
        }).catch((err: BusinessError) => {
          hilog.error(DOMAIN, TAG, 'Failed to set the window layout to full-screen mode. Cause: %{public}s', JSON.stringify(err));
        });
        windowStage.loadContent('pages/Index', (err) => {
          if (err.code) {
            hilog.error(DOMAIN, TAG, 'Failed to load the content. Cause: %{public}s', JSON.stringify(err));
            return;
          }
          hilog.info(DOMAIN, TAG, 'Succeeded in loading the content.');
          this.logWindowState(data, 'after_loadContent');
          this.updateAppearance(data);
          data.setWindowSystemBarProperties({
            navigationBarColor: '#00000000',
            statusBarColor: '#00000000'
          }).then(() => {
            hilog.info(DOMAIN, TAG, 'Succeeded in setting the system bar properties.');
          }).catch((err: BusinessError) => {
            hilog.error(DOMAIN, TAG, 'Failed to set the system bar properties. Cause: %{public}s', JSON.stringify(err));
          });
        });
      });
    } catch (err) {
      hilog.error(DOMAIN, TAG, 'Failed to getMainWindow. Cause: %{public}s', JSON.stringify(err));
    }
  }

  getMainPageColor(): string {
    return this.colorMode === ConfigurationConstant.ColorMode.COLOR_MODE_DARK ? '#FF151515' : '#F1F3F5';
  }

  logWindowState(data: window.Window, scene: string): void {
    try {
      const props = data.getWindowProperties();
      const avoidArea = data.getWindowAvoidArea(window.AvoidAreaType.TYPE_SYSTEM);
      hilog.info(DOMAIN, TAG,
        '%{public}s: winRect=[%{public}d,%{public}d,%{public}d,%{public}d] ' +
        'avoidTop=[%{public}d,%{public}d,%{public}d,%{public}d] ' +
        'avoidBottom=[%{public}d,%{public}d,%{public}d,%{public}d]',
        scene,
        props.windowRect.left, props.windowRect.top, props.windowRect.width, props.windowRect.height,
        avoidArea.topRect.left, avoidArea.topRect.top, avoidArea.topRect.width, avoidArea.topRect.height,
        avoidArea.bottomRect.left, avoidArea.bottomRect.top, avoidArea.bottomRect.width, avoidArea.bottomRect.height);
    } catch (err) {
      hilog.error(DOMAIN, TAG, 'logWindowState failed at %{public}s. Cause: %{public}s', scene, JSON.stringify(err));
    }
  }

  updateAppearance(data: window.Window): void {
    const pageColor = this.getMainPageColor();
    hilog.info(DOMAIN, TAG, 'updateAppearance: colorMode=%{public}d, pageColor=%{public}s', this.colorMode, pageColor);
    try {
      data.setWindowBackgroundColor(pageColor);
    } catch (err) {
      hilog.error(DOMAIN, TAG, 'setWindowBackgroundColor failed. Cause: %{public}s', JSON.stringify(err));
    }
  }

  onConfigurationUpdate(newConfig: Configuration): void {
    super.onConfigurationUpdate(newConfig);
    this.colorMode = newConfig.colorMode;
    hilog.info(DOMAIN, TAG, 'onConfigurationUpdate: colorMode=%{public}d, mainWindow=%{public}s', this.colorMode,
      this.mainWindow ? 'not null' : 'null');
    if (this.mainWindow) {
      this.logWindowState(this.mainWindow, 'onConfigurationUpdate');
      this.updateAppearance(this.mainWindow);
    }
  }

  onWindowStageDestroy(): void {
    hilog.info(DOMAIN, TAG, '%{public}s', 'Ability onWindowStageDestroy');
  }

  onForeground(): void {
    hilog.info(DOMAIN, TAG, '%{public}s', 'Ability onForeground');
    if (this.mainWindow) {
      this.logWindowState(this.mainWindow, 'onForeground');
    }
  }

  onBackground(): void {
    hilog.info(DOMAIN, TAG, '%{public}s', 'Ability onBackground');
  }
}