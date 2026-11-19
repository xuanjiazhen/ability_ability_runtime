import display from "@ohos:display";
import extension from "@ohos:app.ability.ServiceExtensionAbility";
import window from "@ohos:window";
import deviceInfo from "@ohos:deviceInfo";
const TAG = 'JumpInterceptorDialog_Service';
let jumpWinNum = 1;
let win;
export default class JumpInterceptorServiceExtAbility extends extension {
    onCreate(k19) {
        console.debug(TAG, 'onCreate, want: ' + JSON.stringify(k19));
        globalThis.jumpInterceptorExtensionContext = this.context;
    }
    async onRequest(f19, g19) {
        globalThis.abilityWant = f19;
        try {
            globalThis.params = JSON.parse(f19.parameters.params);
            globalThis.position = JSON.parse(f19.parameters.position);
            globalThis.interceptor_callerBundleName = f19.parameters.interceptor_callerBundleName;
            globalThis.interceptor_callerModuleName = f19.parameters.interceptor_callerModuleName;
            globalThis.interceptor_callerLabelId = f19.parameters.interceptor_callerLabelId;
            globalThis.interceptor_targetModuleName = f19.parameters.interceptor_targetModuleName;
            globalThis.interceptor_targetLabelId = f19.parameters.interceptor_targetLabelId;
        }
        catch (j19) {
            console.error(TAG, `getMediaBase64 error:${JSON.stringify(j19)}`);
        }
        await this.getHapResource();
        display.getDefaultDisplay().then(h19 => {
            let i19 = {
                left: globalThis.position.offsetX,
                top: globalThis.position.offsetY,
                width: globalThis.position.width,
                height: globalThis.position.height
            };
            if (jumpWinNum > 1) {
                win.destroy();
                jumpWinNum--;
            }
            if (globalThis.params.isDefaultSelector) {
                this.createWindow('JumpInterceptorDialog ' + g19, window.WindowType.TYPE_SYSTEM_ALERT, i19);
            }
            else {
                this.createWindow('JumpInterceptorDialog ' + g19, window.WindowType.TYPE_FLOAT, i19);
            }
            jumpWinNum++;
        });
    }
    async getHapResource() {
        console.debug(TAG, 'start getHapResource');
        globalThis.callerAppName = await this.loadAppName(globalThis.interceptor_callerBundleName, globalThis.interceptor_callerModuleName, globalThis.interceptor_callerLabelId);
        globalThis.targetAppName = await this.loadAppName(globalThis.params.bundleName, globalThis.interceptor_targetModuleName, globalThis.interceptor_targetLabelId);
        console.debug(TAG, 'getHapResource load finished');
    }
    async loadAppName(z18: string, a19: string, b19: number) {
        let c19 = globalThis.jumpInterceptorExtensionContext.createModuleContext(z18, a19);
        let d19: string = '';
        try {
            d19 = await c19.resourceManager.getString(b19);
        }
        catch (e19) {
            console.error(TAG, `getMediaBase64 error:${JSON.stringify(e19)}`);
        }
        return d19;
    }
    onDestroy() {
        console.info(TAG, 'JumpInterceptorServiceExtAbility onDestroy.');
        if (win !== undefined) {
            win.destroy();
        }
    }
    private async createWindow(v18: string, w18: number, x18) {
        let y18 = deviceInfo.deviceType;
        console.info(TAG, 'create window');
        try {
            win = await window.create(globalThis.jumpInterceptorExtensionContext, v18, w18);
            if (y18 !== 'default') {
                await win.hideNonSystemFloatingWindows(true);
            }
            await win.moveTo(x18.left, x18.top);
            await win.resetSize(x18.width, x18.height);
            await win.loadContent('pages/jumpInterceptorDialog');
            await win.setBackgroundColor('#00000000');
            await win.show();
        }
        catch {
            console.error(TAG, 'window create failed!');
        }
    }
}
;
