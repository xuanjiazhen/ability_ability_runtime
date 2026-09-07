import extension from "@ohos:app.ability.ServiceExtensionAbility";
import window from "@ohos:window";
import display from "@ohos:display";
import PositionUtils from "@bundle:com.ohos.amsdialog/entry/ets/utils/PositionUtils";
import deviceInfo from "@ohos:deviceInfo";
const TAG = 'TipsDialog_Service';
let tipsWinNum = 1;
let win;
export default class TipsServiceExtensionAbility extends extension {
    onCreate(w22) {
        console.debug(TAG, 'onCreate, want: ' + JSON.stringify(w22));
        globalThis.tipsExtensionContext = this.context;
    }
    onRequest(o22, p22) {
        console.debug(TAG, 'onRequest, want: ' + JSON.stringify(o22));
        globalThis.abilityWant = o22;
        try {
            globalThis.params = JSON.parse(o22.parameters.params);
            globalThis.position = PositionUtils.getTipsDialogPosition();
            globalThis.callerToken = o22.parameters.callerToken;
            display.on('change', (u22: number) => {
                let v22 = PositionUtils.getTipsDialogPosition();
                if (v22.offsetX !== globalThis.position.offsetX || v22.offsetY !== globalThis.position.offsetY) {
                    win.moveTo(v22.offsetX, v22.offsetY);
                }
                if (v22.width !== globalThis.position.width || v22.height !== globalThis.position.height) {
                    win.resetSize(v22.width, v22.height);
                }
                globalThis.position = v22;
            });
        }
        catch (t22) {
            console.error('Failed to register callback. Code: ' + JSON.stringify(t22));
        }
        display.getDefaultDisplay().then(q22 => {
            let r22 = {
                left: globalThis.position.offsetX,
                top: globalThis.position.offsetY,
                width: globalThis.position.width,
                height: globalThis.position.height
            };
            if (tipsWinNum > 1) {
                win.destroy();
                tipsWinNum--;
            }
            let s22 = (typeof (globalThis.callerToken) === 'object' && globalThis.callerToken !== null) ?
                window.WindowType.TYPE_DIALOG : window.WindowType.TYPE_SYSTEM_ALERT;
            this.createWindow('TipsDialog' + p22, s22, r22);
            tipsWinNum++;
        });
    }
    onDestroy() {
        console.info(TAG, 'TipsServiceExtensionAbility onDestroy.');
        if (win !== undefined) {
            win.destroy();
        }
    }
    private async createWindow(k22: string, l22: number, m22) {
        let n22 = deviceInfo.deviceType;
        console.info(TAG, 'create window');
        try {
            win = await window.create(globalThis.tipsExtensionContext, k22, l22);
            if (l22 === window.WindowType.TYPE_DIALOG) {
                await win.bindDialogTarget(globalThis.callerToken.value, () => {
                    win.destroyWindow();
                    tipsWinNum--;
                    if (tipsWinNum === 0) {
                        globalThis.tipsExtensionContext.terminateSelf();
                    }
                });
            }
            if (n22 !== 'default') {
                await win.hideNonSystemFloatingWindows(true);
            }
            await win.moveTo(m22.left, m22.top);
            await win.resetSize(m22.width, m22.height);
            await win.loadContent('pages/tipsDialog');
            await win.setBackgroundColor('#00000000');
            await win.show();
        }
        catch {
            console.error(TAG, 'window create failed!');
        }
    }
}
;
