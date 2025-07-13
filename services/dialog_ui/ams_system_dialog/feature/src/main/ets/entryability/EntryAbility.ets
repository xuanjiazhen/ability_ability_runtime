import Want from '@ohos.app.ability.Want';
import AbilityConstant from '@ohos.app.ability.AbilityConstant';
import ConfigurationConstant from '@ohos.app.ability.ConfigurationConstant';
import UIAbility from '@ohos.app.ability.UIAbility';
import hilog from '@ohos.hilog';
import window from '@ohos.window';

const DOMAIN = 0x1323;

export default class EntryAbility extends UIAbility {
  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam): void {
    this.context.getApplicationContext().setColorMode(ConfigurationConstant.ColorMode.COLOR_MODE_NOT_SET);
    AppStorage.setAndLink('context', this.context);
    hilog.info(DOMAIN, 'Intent', '%{public}s', 'Ability onCreate');
  }

  onDestroy(): void {
    hilog.info(DOMAIN, 'Intent', '%{public}s', 'Ability onDestroy');
  }

  onWindowStageCreate(windowStage: window.WindowStage): void {

    let windowClass: window.Window;
    windowStage.getMainWindow((err, data) => {
      windowClass = data;
      windowClass.setWindowLayoutFullScreen(true).then(() => {
      })
      const top = windowClass.getWindowAvoidArea(window.AvoidAreaType.TYPE_SYSTEM).topRect
      let navigationHeight = windowClass.getWindowAvoidArea(window.AvoidAreaType.TYPE_SYSTEM).bottomRect.height
      PersistentStorage.PersistProps([{
        key: "navigationHeight", defaultValue: navigationHeight
      }])
      windowStage.loadContent('pages/Index', (err, data) => {

      });
    });

    hilog.info(DOMAIN, 'Intent', '%{public}s', 'Ability onWindowStageCreate');

    windowStage.loadContent('pages/Index', (err) => {
      if (err.code) {
        hilog.error(DOMAIN, 'Intent', 'Failed to load the content. Cause: %{public}s', JSON.stringify(err));
        return;
      }
      hilog.info(DOMAIN, 'Intent', 'Succeeded in loading the content.');
    });
  }

  onWindowStageDestroy(): void {
    hilog.info(DOMAIN, 'Intent', '%{public}s', 'Ability onWindowStageDestroy');
  }

  onForeground(): void {
    hilog.info(DOMAIN, 'Intent', '%{public}s', 'Ability onForeground');
  }

  onBackground(): void {
    hilog.info(DOMAIN, 'Intent', '%{public}s', 'Ability onBackground');
  }
}