import UIExtensionAbility from "@ohos:app.ability.UIExtensionAbility";
import type UIExtensionContentSession from "@ohos:app.ability.UIExtensionContentSession";
import type Want from "@ohos:app.ability.Want";
import ServiceExtImpl from "@normalized:N&&&entry/src/main/ets/IdlServiceExt/idl_service_ext_impl&";
const TAG: string = '[testTag] UIExtAbility';
export default class UIExtAbility extends UIExtensionAbility {
    serviceExtImpl = new ServiceExtImpl("ExtImpl");
    onCreate() {
        console.log(TAG, `onCreate`);
    }
    onForeground() {
        console.log(TAG, `onForeground`);
    }
    onBackground() {
        console.log(TAG, `onBackground`);
    }
    onDestroy() {
        console.log(TAG, `onDestroy`);
    }
    onSessionCreate(want: Want, session: UIExtensionContentSession) {
        console.log(TAG, `onSessionCreate, want: ${JSON.stringify(want)}`);
        let storage: LocalStorage = new LocalStorage();
        storage.setOrCreate('session', session);
        session.loadContent('pages/Extension', storage);
    }
    onSessionDestroy(session: UIExtensionContentSession) {
        console.log(TAG, `onSessionDestroy`);
    }
    // just for test connect ui extension ability
    onConnect(want: Want) {
        console.log(TAG, `onConnect, want: ${JSON.stringify(want)}`);
        return this.serviceExtImpl;
    }
    onDisConnect(want: Want) {
        console.log(TAG, `onDisConnect, want: ${JSON.stringify(want)}`);
    }
}
;
