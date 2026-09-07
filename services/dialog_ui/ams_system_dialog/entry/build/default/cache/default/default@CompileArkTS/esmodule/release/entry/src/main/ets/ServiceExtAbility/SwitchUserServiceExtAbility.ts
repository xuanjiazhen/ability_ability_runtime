import extension from "@ohos:app.ability.ServiceExtensionAbility";
import window from "@ohos:window";
import display from "@ohos:display";
import deviceInfo from "@ohos:deviceInfo";
const TAG = 'SwitchUserDialog_Service';
const LEFT_COEFFICIENT = 0.1;
const HELF = 2;
const WIDTH_COEFFICIENT = 0.9;
const HEIGHT = 64;
const marginBotton = 20;
let winNum = 1;
let win;
export default class SwitchUserServiceExtensionAbility extends extension {
    onCreate(j22) {
        console.info(TAG, 'onCreate, want: ' + JSON.stringify(j22));
    }
    onRequest(f22, g22) {
        console.info(TAG, 'onRequest, want: ' + JSON.stringify(f22));
        globalThis.abilityWant = f22;
        display.getDefaultDisplay().then(h22 => {
            let i22 = this.getDialogSize(h22);
            if (winNum > 1) {
                win.destroy();
                winNum--;
            }
            this.createWindow('SwitchUserDialog' + g22, window.WindowType.TYPE_SCREENSHOT, i22);
            winNum++;
        });
    }
    onDestroy() {
        console.info(TAG, 'onDestroy.');
    }
    private getDialogSize(b22: display.Display): Object {
        let c22 = b22.height;
        let d22 = b22.width;
        let e22 = b22.densityPixels;
        return {
            left: d22 * LEFT_COEFFICIENT / HELF,
            top: c22 - HEIGHT * e22 - marginBotton,
            width: d22 * WIDTH_COEFFICIENT,
            height: HEIGHT * e22
        };
    }
    private async createWindow(w21: string, x21: number, y21): Promise<void> {
        let z21 = deviceInfo.deviceType;
        console.info(TAG, 'create window rect is ' + JSON.stringify(y21));
        try {
            win = await window.create(this.context, w21, x21);
            if (z21 !== 'default') {
                await win.hideNonSystemFloatingWindows(true);
            }
            await win.moveWindowTo(y21.left, y21.top);
            await win.resize(y21.width, y21.height);
            await win.loadContent('pages/switchUserDialog');
            await win.setWindowBackgroundColor('#00000000');
            await win.showWindow();
        }
        catch (a22) {
            console.error(TAG, `window create failed! ${a22}`);
        }
    }
}
;
