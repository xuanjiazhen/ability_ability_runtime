import application from "@ohos:app.ability.application";
import ConfigurationConstant from "@ohos:app.ability.ConfigurationConstant";
import errorManager from "@ohos:app.ability.errorManager";
import UIAbility from "@ohos:app.ability.UIAbility";
import type AbilityConstant from "@ohos:app.ability.AbilityConstant";
import type Want from "@ohos:app.ability.Want";
import hilog from "@ohos:hilog";
import type window from "@ohos:window";
import type { BusinessError } from "@ohos:base";
import { TAG, DOMAIN } from "@bundle:com.ohos.amsdialog/default/ets/pages/Index";
import settings from "@ohos:settings";
function errorHandlerFunc(s: errorManager.GlobalError) {
    hilog.error(DOMAIN, TAG, 'result name: %{public}s', s.name);
    hilog.error(DOMAIN, TAG, 'result message: %{public}s', s.message);
    if (typeof (s.stack) === 'string') {
        hilog.error(DOMAIN, TAG, 'result stack: %{public}s', s.stack);
    }
    hilog.error(DOMAIN, TAG, 'result instanceName: %{public}s', s.instanceName);
    hilog.error(DOMAIN, TAG, 'result instanceType: %{public}s', s.instanceType);
    restartApp();
}
let errorObserver: errorManager.ErrorObserver = {
    onUnhandledException(r) {
        hilog.error(DOMAIN, TAG, 'onUnhandledException, errorMsg: %{public}s', r);
        restartApp();
    },
    onException(q) {
        hilog.error(DOMAIN, TAG, 'result name: %{public}s', q.name);
        hilog.error(DOMAIN, TAG, 'result message: %{public}s', q.message);
        if (typeof (q.stack) === 'string') {
            hilog.error(DOMAIN, TAG, 'result stack: %{public}s', q.stack);
        }
        restartApp();
    }
};
const DEFAULT_OBSERVER_ID = -1;
let observerId = DEFAULT_OBSERVER_ID;
let restartWant: Want | undefined = undefined;
function restartApp() {
    try {
        application.getApplicationContext().restartApp(restartWant);
        hilog.info(DOMAIN, TAG, 'restartApp success');
    }
    catch (p) {
        hilog.error(DOMAIN, TAG, 'Failed to restartApp. Cause: %{public}s', JSON.stringify(p));
    }
}
export default class EntryAbility extends UIAbility {
    onCreate(i: Want, j: AbilityConstant.LaunchParam): void {
        this.context.getApplicationContext().setColorMode(ConfigurationConstant.ColorMode.COLOR_MODE_NOT_SET);
        hilog.info(DOMAIN, TAG, '%{public}s', 'Ability onCreate');
        restartWant = i;
        const k = 'ai_insightintent_debug';
        let l = 'default';
        try {
            l = settings.getValueSync(this.context, k, 'default', settings.domainName.USER_SECURITY);
        }
        catch (o) {
            hilog.error(DOMAIN, TAG, 'Failed to getValueSync. Cause: %{public}s', JSON.stringify(o));
        }
        if (l !== 'true') {
            try {
                this.context.terminateSelf();
            }
            catch (n) {
                hilog.error(DOMAIN, TAG, 'Failed to context.terminateSelf. Cause: %{public}s', JSON.stringify(n));
            }
        }
        try {
            observerId = errorManager.on('error', errorObserver);
            errorManager.on('globalErrorOccurred', errorHandlerFunc);
        }
        catch (m) {
            hilog.error(DOMAIN, TAG, 'registerErrorManager failed, Cause: %{public}s', JSON.stringify(m));
        }
    }
    onDestroy(): void {
        hilog.info(DOMAIN, TAG, '%{public}s', 'Ability onDestroy');
        try {
            if (observerId != DEFAULT_OBSERVER_ID) {
                errorManager.off('error', observerId);
            }
            errorManager.off('globalErrorOccurred', errorHandlerFunc);
        }
        catch (h) {
            hilog.error(DOMAIN, TAG, 'cancelErrorManager failed, Cause: %{public}s', JSON.stringify(h));
        }
    }
    onWindowStageCreate(a: window.WindowStage): void {
        a.loadContent('pages/Index', (b) => {
            if (b.code) {
                hilog.error(DOMAIN, TAG, 'Failed to load the content. Cause: %{public}s', JSON.stringify(b));
                return;
            }
            hilog.info(DOMAIN, TAG, 'Succeeded in loading the content.');
            try {
                a.getMainWindow((d, e) => {
                    e.setWindowLayoutFullScreen(true).then(() => {
                        hilog.info(DOMAIN, TAG, 'Succeeded in setting the window layout to full-screen mode.');
                    }).catch((g: BusinessError) => {
                        hilog.error(DOMAIN, TAG, 'Failed to set the window layout to full-screen mode. Cause: %{public}s', JSON.stringify(g));
                    });
                    e.setWindowSystemBarProperties({
                        navigationBarColor: '#F1F3F5',
                        statusBarColor: '#F1F3F5'
                    }).then(() => {
                        hilog.info(DOMAIN, TAG, 'Succeeded in setting the system bar properties.');
                    }).catch((f: BusinessError) => {
                        hilog.error(DOMAIN, TAG, 'Failed to set the system bar properties. Cause: %{public}s', JSON.stringify(f));
                    });
                });
            }
            catch (c) {
                hilog.error(DOMAIN, TAG, 'Failed to getMainWindow. Cause: %{public}s', JSON.stringify(c));
            }
        });
    }
    onWindowStageDestroy(): void {
        hilog.info(DOMAIN, TAG, '%{public}s', 'Ability onWindowStageDestroy');
    }
    onForeground(): void {
        hilog.info(DOMAIN, TAG, '%{public}s', 'Ability onForeground');
    }
    onBackground(): void {
        hilog.info(DOMAIN, TAG, '%{public}s', 'Ability onBackground');
    }
}
