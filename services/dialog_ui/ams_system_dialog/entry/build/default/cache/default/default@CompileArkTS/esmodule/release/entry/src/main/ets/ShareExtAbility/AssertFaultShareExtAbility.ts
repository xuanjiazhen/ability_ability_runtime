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
    onSessionCreate(b23: Want, c23: UIExtensionContentSession): void {
        try {
            this.sessionId = b23.parameters[wantConstant.Params.ASSERT_FAULT_SESSION_ID] as string,
                this.storage = new LocalStorage({
                    'session': c23,
                    'textDetail': b23.parameters[TEXT_DETAIL]
                });
        }
        catch (d23) {
            console.error('Failed to register callback. Code: ' + JSON.stringify(d23));
        }
        c23.loadContent('pages/assertFaultDialog', this.storage);
        c23.setWindowBackgroundColor('#00000000');
    }
    onDestroy(): void {
        console.info(TAG, 'onDestroy');
    }
    onSessionDestroy(x22: UIExtensionContentSession): void {
        console.info(TAG, 'onSessionDestroy');
        let y22 = AppStorage.get<abilityManager.UserStatus>(DEBUG_ASSERT_RESULT);
        if (y22 === undefined) {
            y22 = abilityManager.UserStatus.ASSERT_TERMINATE;
        }
        try {
            abilityManager.notifyDebugAssertResult(this.sessionId, y22).then(() => {
                console.log(TAG, 'notifyDebugAssertResult success.');
            }).catch((a23: BusinessError) => {
                console.error(TAG, `notifyDebugAssertResult failed, error: ${JSON.stringify(a23)}`);
            });
        }
        catch (z22) {
            console.error(TAG, `try notifyDebugAssertResult failed, error: ${JSON.stringify(z22)}`);
        }
    }
}
;
