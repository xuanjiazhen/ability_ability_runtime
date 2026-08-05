import display from "@ohos:display";
import extension from "@ohos:app.ability.ServiceExtensionAbility";
import window from "@ohos:window";
import deviceInfo from "@ohos:deviceInfo";
const TAG = 'JumpInterceptorDialog_Service';
let jumpWinNum = 1;
let win;
export default class JumpInterceptorServiceExtAbility extends extension {
    onCreate(c19) {
        console.debug(TAG, 'onCreate, want: ' + JSON.stringify(c19));
        globalThis.jumpInterceptorExtensionContext = this.context;
    }
    async onRequest(x18, y18) {
        globalThis.abilityWant = x18;
        try {
            globalThis.params = JSON.parse(x18.parameters.params);
            globalThis.position = JSON.parse(x18.parameters.position);
            globalThis.interceptor_callerBundleName = x18.parameters.interceptor_callerBundleName;
            globalThis.interceptor_callerModuleName = x18.parameters.interceptor_callerModuleName;
            globalThis.interceptor_callerLabelId = x18.parameters.interceptor_callerLabelId;
            globalThis.interceptor_targetModuleName = x18.parameters.interceptor_targetModuleName;
            globalThis.interceptor_targetLabelId = x18.parameters.interceptor_targetLabelId;
        }
        catch (b19) {
            console.error(TAG, `getMediaBase64 error:${JSON.stringify(b19)}`);
        }
        await this.getHapResource();
        display.getDefaultDisplay().then(z18 => {
            let a19 = {
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
                this.createWindow('JumpInterceptorDialog ' + y18, window.WindowType.TYPE_SYSTEM_ALERT, a19);
            }
            else {
                this.createWindow('JumpInterceptorDialog ' + y18, window.WindowType.TYPE_FLOAT, a19);
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
    async loadAppName(r18: string, s18: string, t18: number) {
        let u18 = globalThis.jumpInterceptorExtensionContext.createModuleContext(r18, s18);
        let v18: string = '';
        try {
            v18 = await u18.resourceManager.getString(t18);
        }
        catch (w18) {
            console.error(TAG, `getMediaBase64 error:${JSON.stringify(w18)}`);
        }
        return v18;
    }
    onDestroy() {
        console.info(TAG, 'JumpInterceptorServiceExtAbility onDestroy.');
        if (win !== undefined) {
            win.destroy();
        }
    }
    private async createWindow(n18: string, o18: number, p18) {
        let q18 = deviceInfo.deviceType;
        console.info(TAG, 'create window');
        try {
            win = await window.create(globalThis.jumpInterceptorExtensionContext, n18, o18);
            if (q18 !== 'default') {
                await win.hideNonSystemFloatingWindows(true);
            }
            await win.moveTo(p18.left, p18.top);
            await win.resetSize(p18.width, p18.height);
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
