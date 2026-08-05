import UIExtensionAbility from "@ohos:app.ability.UIExtensionAbility";
import type UIExtensionContentSession from "@ohos:app.ability.UIExtensionContentSession";
import type Want from "@ohos:app.ability.Want";
import systemparameter from "@ohos:systemParameterEnhance";
import dataPreferences from "@ohos:data.preferences";
const TAG = 'SelectorExtensionAbility';
export default class SelectorExtensionAbility extends UIExtensionAbility {
    onCreate() {
        console.info(TAG, 'SelectorExtensionAbility onCreate');
        globalThis.currentExtensionContext = this.context;
        let j19 = { name: 'dialogStore' };
        globalThis.preferences = dataPreferences.getPreferencesSync(this.context, j19);
    }
    onSessionCreate(e19: Want, f19: UIExtensionContentSession) {
        const g19: LocalStorage = new LocalStorage({
            'session': f19,
            'extensionAbility': this,
            'callerWant': e19
        } as Record<string, object>);
        globalThis.ExtensionType = 'UIExtension';
        if (systemparameter.getSync('persist.sys.abilityms.isdialogconfirmpermission', 'false') === 'false' &&
            globalThis.preferences.getSync('isdialogconfirmpermission', 'false') === 'false') {
            globalThis.currentURL = 'pages/PhonePage';
            f19.loadContent('pages/permissionConfirmDialog', g19);
        }
        else {
            f19.loadContent('pages/PhonePage', g19);
        }
        try {
            const i19: string = '#40FFFFFF';
        }
        catch (h19) {
            console.info(TAG, `Set window background color fail, ${JSON.stringify(h19)}`);
        }
    }
    onSessionDestroy(d19: UIExtensionContentSession) {
        console.info(TAG, 'SelectorExtensionAbility onSessionDestroy');
    }
    onForeground() {
        console.info(TAG, 'SelectorExtensionAbility onForeground');
    }
    onBackground() {
        console.info(TAG, 'SelectorExtensionAbility onBackground');
    }
    onDestroy() {
        console.info(TAG, 'SelectorExtensionAbility onDestroy');
    }
}
