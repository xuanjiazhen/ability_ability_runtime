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
        let r19 = { name: 'dialogStore' };
        globalThis.preferences = dataPreferences.getPreferencesSync(this.context, r19);
    }
    onSessionCreate(m19: Want, n19: UIExtensionContentSession) {
        const o19: LocalStorage = new LocalStorage({
            'session': n19,
            'extensionAbility': this,
            'callerWant': m19
        } as Record<string, object>);
        globalThis.ExtensionType = 'UIExtension';
        if (systemparameter.getSync('persist.sys.abilityms.isdialogconfirmpermission', 'false') === 'false' &&
            globalThis.preferences.getSync('isdialogconfirmpermission', 'false') === 'false') {
            globalThis.currentURL = 'pages/PhonePage';
            n19.loadContent('pages/permissionConfirmDialog', o19);
        }
        else {
            n19.loadContent('pages/PhonePage', o19);
        }
        try {
            const q19: string = '#40FFFFFF';
        }
        catch (p19) {
            console.info(TAG, `Set window background color fail, ${JSON.stringify(p19)}`);
        }
    }
    onSessionDestroy(l19: UIExtensionContentSession) {
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
