import abilityManager from "@ohos:app.ability.abilityManager";
import type { BusinessError } from "@ohos:base";
import UIExtensionAbility from "@ohos:app.ability.UIExtensionAbility";
import wantConstant from "@ohos:app.ability.wantConstant";
import type Want from "@ohos:app.ability.Want";
import type UIExtensionContentSession from "@ohos:app.ability.UIExtensionContentSession";
const TAG = 'AssertFaultDialog_UIExtension';
const TEXT_DETAIL = 'assertFaultDialogDetail';
const DEBUG_ASSERT_RESULT = 'assertResult';
export default class UiExtAbility extends UIExtensionAbility {
    storage: LocalStorage;
    sessionId: string;
    onCreate(): void {
        console.info(TAG, 'onCreate');
    }
    onForeground(): void {
        console.info(TAG, 'onForeground');
    }
    onBackground(): void {
        console.info(TAG, 'onBackground');
    }
    onSessionCreate(j23: Want, k23: UIExtensionContentSession): void {
        try {
            this.sessionId = j23.parameters[wantConstant.Params.ASSERT_FAULT_SESSION_ID] as string,
                this.storage = new LocalStorage({
                    'session': k23,
                    'textDetail': j23.parameters[TEXT_DETAIL]
                });
        }
        catch (l23) {
            console.error('Failed to register callback. Code: ' + JSON.stringify(l23));
        }
        k23.loadContent('pages/assertFaultDialog', this.storage);
        k23.setWindowBackgroundColor('#00000000');
    }
    onDestroy(): void {
        console.info(TAG, 'onDestroy');
    }
    onSessionDestroy(f23: UIExtensionContentSession): void {
        console.info(TAG, 'onSessionDestroy');
        let g23 = AppStorage.get<abilityManager.UserStatus>(DEBUG_ASSERT_RESULT);
        if (g23 === undefined) {
            g23 = abilityManager.UserStatus.ASSERT_TERMINATE;
        }
        try {
            abilityManager.notifyDebugAssertResult(this.sessionId, g23).then(() => {
                console.log(TAG, 'notifyDebugAssertResult success.');
            }).catch((i23: BusinessError) => {
                console.error(TAG, `notifyDebugAssertResult failed, error: ${JSON.stringify(i23)}`);
            });
        }
        catch (h23) {
            console.error(TAG, `try notifyDebugAssertResult failed, error: ${JSON.stringify(h23)}`);
        }
    }
}
;
