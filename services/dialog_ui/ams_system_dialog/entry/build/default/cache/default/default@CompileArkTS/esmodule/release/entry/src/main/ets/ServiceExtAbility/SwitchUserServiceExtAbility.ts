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
    onCreate(r22) {
        console.info(TAG, 'onCreate, want: ' + JSON.stringify(r22));
    }
    onRequest(n22, o22) {
        console.info(TAG, 'onRequest, want: ' + JSON.stringify(n22));
        globalThis.abilityWant = n22;
        display.getDefaultDisplay().then(p22 => {
            let q22 = this.getDialogSize(p22);
            if (winNum > 1) {
                win.destroy();
                winNum--;
            }
            this.createWindow('SwitchUserDialog' + o22, window.WindowType.TYPE_SCREENSHOT, q22);
            winNum++;
        });
    }
    onDestroy() {
        console.info(TAG, 'onDestroy.');
    }
    private getDialogSize(j22: display.Display): Object {
        let k22 = j22.height;
        let l22 = j22.width;
        let m22 = j22.densityPixels;
        return {
            left: l22 * LEFT_COEFFICIENT / HELF,
            top: k22 - HEIGHT * m22 - marginBotton,
            width: l22 * WIDTH_COEFFICIENT,
            height: HEIGHT * m22
        };
    }
    private async createWindow(e22: string, f22: number, g22): Promise<void> {
        let h22 = deviceInfo.deviceType;
        console.info(TAG, 'create window rect is ' + JSON.stringify(g22));
        try {
            win = await window.create(this.context, e22, f22);
            if (h22 !== 'default') {
                await win.hideNonSystemFloatingWindows(true);
            }
            await win.moveWindowTo(g22.left, g22.top);
            await win.resize(g22.width, g22.height);
            await win.loadContent('pages/switchUserDialog');
            await win.setWindowBackgroundColor('#00000000');
            await win.showWindow();
        }
        catch (i22) {
            console.error(TAG, `window create failed! ${i22}`);
        }
    }
}
;
