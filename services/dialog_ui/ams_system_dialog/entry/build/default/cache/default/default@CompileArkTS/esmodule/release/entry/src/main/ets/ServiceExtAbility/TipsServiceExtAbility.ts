import extension from "@ohos:app.ability.ServiceExtensionAbility";
import window from "@ohos:window";
import display from "@ohos:display";
import PositionUtils from "@bundle:com.ohos.amsdialog/entry/ets/utils/PositionUtils";
import deviceInfo from "@ohos:deviceInfo";
const TAG = 'TipsDialog_Service';
let tipsWinNum = 1;
let win;
export default class TipsServiceExtensionAbility extends extension {
    onCreate(e23) {
        console.debug(TAG, 'onCreate, want: ' + JSON.stringify(e23));
        globalThis.tipsExtensionContext = this.context;
    }
    onRequest(w22, x22) {
        console.debug(TAG, 'onRequest, want: ' + JSON.stringify(w22));
        globalThis.abilityWant = w22;
        try {
            globalThis.params = JSON.parse(w22.parameters.params);
            globalThis.position = PositionUtils.getTipsDialogPosition();
            globalThis.callerToken = w22.parameters.callerToken;
            display.on('change', (c23: number) => {
                let d23 = PositionUtils.getTipsDialogPosition();
                if (d23.offsetX !== globalThis.position.offsetX || d23.offsetY !== globalThis.position.offsetY) {
                    win.moveTo(d23.offsetX, d23.offsetY);
                }
                if (d23.width !== globalThis.position.width || d23.height !== globalThis.position.height) {
                    win.resetSize(d23.width, d23.height);
                }
                globalThis.position = d23;
            });
        }
        catch (b23) {
            console.error('Failed to register callback. Code: ' + JSON.stringify(b23));
        }
        display.getDefaultDisplay().then(y22 => {
            let z22 = {
                left: globalThis.position.offsetX,
                top: globalThis.position.offsetY,
                width: globalThis.position.width,
                height: globalThis.position.height
            };
            if (tipsWinNum > 1) {
                win.destroy();
                tipsWinNum--;
            }
            let a23 = (typeof (globalThis.callerToken) === 'object' && globalThis.callerToken !== null) ?
                window.WindowType.TYPE_DIALOG : window.WindowType.TYPE_SYSTEM_ALERT;
            this.createWindow('TipsDialog' + x22, a23, z22);
            tipsWinNum++;
        });
    }
    onDestroy() {
        console.info(TAG, 'TipsServiceExtensionAbility onDestroy.');
        if (win !== undefined) {
            win.destroy();
        }
    }
    private async createWindow(s22: string, t22: number, u22) {
        let v22 = deviceInfo.deviceType;
        console.info(TAG, 'create window');
        try {
            win = await window.create(globalThis.tipsExtensionContext, s22, t22);
            if (t22 === window.WindowType.TYPE_DIALOG) {
                await win.bindDialogTarget(globalThis.callerToken.value, () => {
                    win.destroyWindow();
                    tipsWinNum--;
                    if (tipsWinNum === 0) {
                        globalThis.tipsExtensionContext.terminateSelf();
                    }
                });
            }
            if (v22 !== 'default') {
                await win.hideNonSystemFloatingWindows(true);
            }
            await win.moveTo(u22.left, u22.top);
            await win.resetSize(u22.width, u22.height);
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
