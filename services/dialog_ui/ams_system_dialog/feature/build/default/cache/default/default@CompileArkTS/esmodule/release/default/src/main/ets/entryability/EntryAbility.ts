import application from "@ohos:app.ability.application";
import ConfigurationConstant from "@ohos:app.ability.ConfigurationConstant";
import errorManager from "@ohos:app.ability.errorManager";
import UIAbility from "@ohos:app.ability.UIAbility";
import type AbilityConstant from "@ohos:app.ability.AbilityConstant";
import type { Configuration } from "@ohos:app.ability.Configuration";
import type Want from "@ohos:app.ability.Want";
import hilog from "@ohos:hilog";
import type window from "@ohos:window";
import type { BusinessError } from "@ohos:base";
import { TAG, DOMAIN } from "@bundle:com.ohos.amsdialog/default/ets/pages/Index";
import settings from "@ohos:settings";
function errorHandlerFunc(w: errorManager.GlobalError) {
    hilog.error(DOMAIN, TAG, 'result name: %{public}s', w.name);
    hilog.error(DOMAIN, TAG, 'result message: %{public}s', w.message);
    if (typeof (w.stack) === 'string') {
        hilog.error(DOMAIN, TAG, 'result stack: %{public}s', w.stack);
    }
    hilog.error(DOMAIN, TAG, 'result instanceName: %{public}s', w.instanceName);
    hilog.error(DOMAIN, TAG, 'result instanceType: %{public}s', w.instanceType);
    restartApp();
}
let errorObserver: errorManager.ErrorObserver = {
    onUnhandledException(v) {
        hilog.error(DOMAIN, TAG, 'onUnhandledException, errorMsg: %{public}s', v);
        restartApp();
    },
    onException(u) {
        hilog.error(DOMAIN, TAG, 'result name: %{public}s', u.name);
        hilog.error(DOMAIN, TAG, 'result message: %{public}s', u.message);
        if (typeof (u.stack) === 'string') {
            hilog.error(DOMAIN, TAG, 'result stack: %{public}s', u.stack);
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
    catch (t) {
        hilog.error(DOMAIN, TAG, 'Failed to restartApp. Cause: %{public}s', JSON.stringify(t));
    }
}
export default class EntryAbility extends UIAbility {
    private colorMode: ConfigurationConstant.ColorMode = ConfigurationConstant.ColorMode.COLOR_MODE_NOT_SET;
    private mainWindow: window.Window | null = null;
    onCreate(m: Want, n: AbilityConstant.LaunchParam): void {
        this.context.getApplicationContext().setColorMode(ConfigurationConstant.ColorMode.COLOR_MODE_NOT_SET);
        hilog.info(DOMAIN, TAG, '%{public}s', 'Ability onCreate');
        restartWant = m;
        const o = 'ai_insightintent_debug';
        let p = 'default';
        try {
            p = settings.getValueSync(this.context, o, 'default', settings.domainName.USER_SECURITY);
        }
        catch (s) {
            hilog.error(DOMAIN, TAG, 'Failed to getValueSync. Cause: %{public}s', JSON.stringify(s));
        }
        if (p !== 'true') {
            try {
                this.context.terminateSelf();
            }
            catch (r) {
                hilog.error(DOMAIN, TAG, 'Failed to context.terminateSelf. Cause: %{public}s', JSON.stringify(r));
            }
        }
        try {
            observerId = errorManager.on('error', errorObserver);
            errorManager.on('globalErrorOccurred', errorHandlerFunc);
        }
        catch (q) {
            hilog.error(DOMAIN, TAG, 'registerErrorManager failed, Cause: %{public}s', JSON.stringify(q));
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
        catch (l) {
            hilog.error(DOMAIN, TAG, 'cancelErrorManager failed, Cause: %{public}s', JSON.stringify(l));
        }
    }
    onWindowStageCreate(g: window.WindowStage): void {
        this.colorMode = this.context.config.colorMode;
        g.loadContent('pages/Index', (h) => {
            if (h.code) {
                hilog.error(DOMAIN, TAG, 'Failed to load the content. Cause: %{public}s', JSON.stringify(h));
                return;
            }
            hilog.info(DOMAIN, TAG, 'Succeeded in loading the content.');
            try {
                g.getMainWindow((j, k) => {
                    if (j && j.code !== 0) {
                        hilog.error(DOMAIN, TAG, 'Failed to get main window. code=%{public}d, Cause: %{public}s', j.code, JSON.stringify(j.message ?? j));
                        return;
                    }
                    this.mainWindow = k;
                    this.applyAppearance(k);
                });
            }
            catch (i) {
                hilog.error(DOMAIN, TAG, 'Failed to getMainWindow. Cause: %{public}s', JSON.stringify(i));
            }
        });
    }
    getMainPageColor(): string {
        return this.colorMode === ConfigurationConstant.ColorMode.COLOR_MODE_DARK ? '#FF151515' : '#F1F3F5';
    }
    applyAppearance(b: window.Window): void {
        b.setWindowLayoutFullScreen(true).then(() => {
            hilog.info(DOMAIN, TAG, 'Succeeded in setting the window layout to full-screen mode.');
        }).catch((f: BusinessError) => {
            hilog.error(DOMAIN, TAG, 'Failed to set the window layout to full-screen mode. Cause: %{public}s', JSON.stringify(f));
        });
        const c = this.getMainPageColor();
        try {
            b.setWindowBackgroundColor(c);
        }
        catch (e) {
            hilog.error(DOMAIN, TAG, 'setWindowBackgroundColor failed. Cause: %{public}s', JSON.stringify(e));
        }
        b.setWindowSystemBarProperties({
            navigationBarColor: '#00000000',
            statusBarColor: '#00000000'
        }).then(() => {
            hilog.info(DOMAIN, TAG, 'Succeeded in setting the system bar properties.');
        }).catch((d: BusinessError) => {
            hilog.error(DOMAIN, TAG, 'Failed to set the system bar properties. Cause: %{public}s', JSON.stringify(d));
        });
    }
    onConfigurationUpdate(a: Configuration): void {
        super.onConfigurationUpdate(a);
        this.colorMode = a.colorMode;
        if (this.mainWindow) {
            this.applyAppearance(this.mainWindow);
        }
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
