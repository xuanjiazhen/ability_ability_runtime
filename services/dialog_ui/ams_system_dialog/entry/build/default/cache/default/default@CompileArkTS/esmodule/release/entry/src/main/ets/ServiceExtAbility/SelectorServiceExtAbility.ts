import bundleManager from "@ohos:bundle.bundleManager";
import defaultAppManager from "@ohos:bundle.defaultAppManager";
import display from "@ohos:display";
import drawableDescriptor from "@ohos:arkui.drawableDescriptor";
import extension from "@ohos:app.ability.ServiceExtensionAbility";
import type image from "@ohos:multimedia.image";
import window from "@ohos:window";
import PositionUtils from "@bundle:com.ohos.amsdialog/entry/ets/utils/PositionUtils";
import deviceInfo from "@ohos:deviceInfo";
import systemparameter from "@ohos:systemParameterEnhance";
import dataPreferences from "@ohos:data.preferences";
const TAG = 'SelectorDialog_Service';
let winNum = 1;
let win;
export default class SelectorServiceExtensionAbility extends extension {
    onCreate(b22) {
        console.debug(TAG, 'onCreate, want: ' + JSON.stringify(b22));
        globalThis.selectExtensionContext = this.context;
        globalThis.currentExtensionContext = this.context;
        globalThis.ExtensionType = 'ServiceExtension';
        globalThis.defaultAppManager = defaultAppManager;
        globalThis.bundleManager = bundleManager;
        let c22 = { name: 'dialogStore' };
        try {
            globalThis.preferences = dataPreferences.getPreferencesSync(this.context, c22);
        }
        catch (d22) {
            console.error(TAG, 'Failed to getPreferencesSync callback. Code: ' + JSON.stringify(d22));
        }
    }
    async getPhoneShowHapList() {
        const s21 = 8;
        let t21 = [];
        let u21 = [];
        let v21: Map<string, image.PixelMap> = new Map();
        for (let a22 = 1; a22 <= globalThis.params.hapList.length; a22++) {
            console.info(TAG, 'hapList[' + (a22 - 1).toString() + ']: ' + JSON.stringify(globalThis.params.hapList[a22 - 1]));
            await this.getHapResource(globalThis.params.hapList[a22 - 1], t21, v21);
            if (a22 % s21 === 0) {
                u21.push(t21);
                t21 = [];
            }
            if (a22 >= globalThis.params.hapList.length && t21.length > 0) {
                u21.push(t21);
            }
        }
        globalThis.phoneShowHapList = u21;
        console.debug(TAG, 'phoneShowHapList: ' + JSON.stringify(u21));
        const w21 = 4;
        let x21 = [];
        let y21 = [];
        for (let z21 = 1; z21 <= globalThis.params.hapList.length; z21++) {
            console.info(TAG, 'hapList[' + (z21 - 1).toString() + ']: ' + JSON.stringify(globalThis.params.hapList[z21 - 1]));
            await this.getHapResource(globalThis.params.hapList[z21 - 1], x21, v21);
            if (z21 % w21 === 0) {
                y21.push(x21);
                x21 = [];
            }
            if (z21 >= globalThis.params.hapList.length && x21.length > 0) {
                y21.push(x21);
            }
        }
        globalThis.signalRowPhoneShowHapList = y21;
        globalThis.jsonIconMap = v21;
        console.debug(TAG, 'signalRowPhoneShowHapList: ' + JSON.stringify(y21));
    }
    async getPcShowHapList() {
        let p21 = [];
        let q21: Map<string, image.PixelMap> = new Map();
        for (let r21 = 0; r21 < globalThis.params.hapList.length; r21++) {
            await this.getHapResource(globalThis.params.hapList[r21], p21, q21);
        }
        globalThis.pcShowHapList = p21;
        globalThis.jsonIconMap = q21;
        console.debug(TAG, 'pcShowHapList: ' + JSON.stringify(p21));
    }
    async getHapResource(t20, u20, v20) {
        let w20 = t20.bundle;
        let x20 = t20.module;
        let y20 = t20.ability;
        let z20 = '';
        let a21 = '';
        let b21 = '';
        let c21 = Number('0');
        let d21 = Number(t20.appIndex);
        if (!globalThis.params.isDefaultSelector) {
            b21 = t20.type;
            c21 = Number(t20.userId);
        }
        let e21 = Number(t20.label);
        if (e21 === 0) {
            e21 = Number(t20.bundleLabel);
        }
        let f21 = globalThis.selectExtensionContext.createModuleContext(w20, x20);
        await f21.resourceManager.getString(e21).then(o21 => {
            if (d21 === 0) {
                z20 = o21;
            }
            else {
                z20 = o21 + t20.appIndex;
            }
        }).catch(n21 => {
            console.error(TAG, 'getString error:' + JSON.stringify(n21));
        });
        let g21 = Number(t20.icon);
        if (g21 === 0) {
            g21 = Number(t20.bundleIcon);
        }
        await f21.resourceManager.getMediaBase64(g21).then(i21 => {
            a21 = i21;
            if (a21.indexOf('image/json') > -1) {
                try {
                    const k21 = f21.resourceManager.getDrawableDescriptor(g21);
                    if (k21 !== null && k21 !== undefined &&
                        k21 instanceof drawableDescriptor.LayeredDrawableDescriptor) {
                        let l21: drawableDescriptor.LayeredDrawableDescriptor = <drawableDescriptor.LayeredDrawableDescriptor>k21;
                        let m21: drawableDescriptor.DrawableDescriptor = l21.getForeground();
                        if (m21 !== null && m21 !== undefined) {
                            v20.set(w20 + ':' + x20 + ':' + y20 + ':' + t20.appIndex, m21.getPixelMap());
                        }
                        else {
                            console.error(TAG, 'get foregroundDescriptor is null');
                        }
                    }
                }
                catch (j21) {
                    console.error(TAG, 'get drawableDescriptor error:' + JSON.stringify(j21));
                }
            }
        }).catch(h21 => {
            console.error(TAG, 'getMediaBase64 error:' + JSON.stringify(h21));
        });
        u20.push(w20 + '#' + y20 + '#' + z20 +
            '#' + a21 + '#' + x20 + '#' + b21 + '#' + c21 + '#' + d21);
    }
    async onRequest(e20, f20) {
        console.debug(TAG, 'onRequest, want: ' + JSON.stringify(e20));
        globalThis.abilityWant = e20;
        try {
            globalThis.params = JSON.parse(e20.parameters.params);
            globalThis.callerToken = e20.parameters.callerToken;
            let p20 = 0;
            if (globalThis.params && globalThis.params.hapList && globalThis.params.hapList.length) {
                p20 = globalThis.params.hapList.length;
            }
            globalThis.lineNums = p20;
            globalThis.position = PositionUtils.getSelectorDialogPosition(p20);
            display.on('change', (q20: number) => {
                try {
                    if (!globalThis.position) {
                        console.error(TAG, 'globalThis.position is invalid in display change callback, skip');
                        return;
                    }
                    let s20 = PositionUtils.getSelectorDialogPosition(p20);
                    if (!s20) {
                        console.error(TAG, 'position is invalid in display change callback, skip');
                        return;
                    }
                    if (s20.offsetX !== globalThis.position.offsetX || s20.offsetY !== globalThis.position.offsetY) {
                        win.moveTo(s20.offsetX, s20.offsetY);
                    }
                    if (s20.width !== globalThis.position.width || s20.height !== globalThis.position.height) {
                        win.resetSize(s20.width, s20.height);
                    }
                    globalThis.position = s20;
                }
                catch (r20) {
                    console.error(TAG, 'display change callback failed: ' + JSON.stringify(r20));
                }
            });
        }
        catch (o20) {
            console.error(TAG, 'Failed to register display change callback. Code: ' + JSON.stringify(o20));
        }
        try {
            let n20 = display.getDefaultDisplaySync();
            console.debug(TAG, 'onRequest display is' + JSON.stringify(n20));
        }
        catch (m20) {
            console.error(TAG, 'Failed to getDefaultDisplaySync callback. Code: ' + JSON.stringify(m20));
        }
        console.debug(TAG, 'onRequest, want: ' + JSON.stringify(e20));
        console.debug(TAG, 'onRequest, params: ' + JSON.stringify(globalThis.params));
        console.debug(TAG, 'onRequest, position: ' + JSON.stringify(globalThis.position));
        if (!globalThis.params.isDefaultSelector) {
            globalThis.modelFlag = Boolean(globalThis.params.modelFlag);
            globalThis.action = Boolean(globalThis.params.action);
        }
        try {
            if (globalThis.params.isDefaultSelector) {
                await this.getPhoneShowHapList();
            }
            else {
                await this.getPcShowHapList();
            }
        }
        catch (l20) {
            console.error(TAG, 'Failed to get hap list: ' + JSON.stringify(l20));
        }
        try {
            AppStorage.SetOrCreate('oversizeHeight', globalThis.position.oversizeHeight ? 'true' : 'false');
        }
        catch (k20) {
            console.error(TAG, 'Failed to set oversizeHeight: ' + JSON.stringify(k20));
        }
        display.getDefaultDisplay().then(g20 => {
            try {
                if (!globalThis.position) {
                    console.error(TAG, 'globalThis.position is invalid in getDefaultDisplay callback, skip');
                    return;
                }
                let i20 = {
                    left: globalThis.position.offsetX,
                    top: globalThis.position.offsetY,
                    width: globalThis.position.width,
                    height: globalThis.position.height
                };
                if (winNum > 1 && win) {
                    win.destroy();
                    winNum--;
                }
                let j20 = (typeof (globalThis.callerToken) === 'object' && globalThis.callerToken !== null) ?
                    window.WindowType.TYPE_DIALOG : window.WindowType.TYPE_SYSTEM_ALERT;
                this.createWindow('SelectorDialog' + f20, j20, i20);
                winNum++;
            }
            catch (h20) {
                console.error(TAG, 'display getDefaultDisplay callback failed: ' + JSON.stringify(h20));
            }
        });
    }
    onDestroy() {
        console.info(TAG, 'SelectorServiceExtensionAbility onDestroy.');
        if (win !== undefined) {
            win.destroy();
        }
    }
    private async createWindow(z19: string, a20: number, b20) {
        let c20 = deviceInfo.deviceType;
        console.info(TAG, 'create window');
        try {
            win = await window.create(globalThis.selectExtensionContext, z19, a20);
            if (a20 === window.WindowType.TYPE_DIALOG) {
                await win.bindDialogTarget(globalThis.callerToken.value, () => {
                    win.destroyWindow();
                    winNum--;
                    if (winNum === 0) {
                        globalThis.selectExtensionContext.terminateSelf();
                    }
                });
            }
            if (c20 !== 'default') {
                await win.hideNonSystemFloatingWindows(true);
            }
            await win.moveTo(b20.left, b20.top);
            await win.resetSize(b20.width, b20.height);
            if (systemparameter.getSync('persist.sys.abilityms.isdialogconfirmpermission', 'false') === 'false' &&
                globalThis.preferences.getSync('isdialogconfirmpermission', 'false') === 'false') {
                if (globalThis.params.isDefaultSelector) {
                    globalThis.currentURL = 'pages/selectorPhoneDialog';
                }
                else {
                    globalThis.currentURL = 'pages/selectorPcDialog';
                }
                await win.loadContent('pages/permissionConfirmDialog');
            }
            else {
                if (globalThis.params.isDefaultSelector) {
                    await win.loadContent('pages/selectorPhoneDialog');
                }
                else {
                    await win.loadContent('pages/selectorPcDialog');
                }
            }
            await win.setBackgroundColor('#00000000');
            await win.show();
        }
        catch (d20) {
            console.error(TAG, 'window create failed: ' + JSON.stringify(d20));
        }
    }
    private async moveWindow(w19): Promise<void> {
        try {
            await win.moveTo(w19.left, w19.top);
            await win.resetSize(w19.width, w19.height);
            if (globalThis.params.isDefaultSelector) {
                try {
                    await win.loadContent('pages/selectorPhoneDialog');
                    await win.setBackgroundColor('#00000000');
                }
                catch (y19) {
                    console.error(TAG, 'window loadContent failed: ' + JSON.stringify(y19));
                }
            }
            await win.show();
        }
        catch (x19) {
            console.error(TAG, 'window move failed: ' + JSON.stringify(x19));
        }
    }
    onConfigurationUpdate(s19): void {
        console.debug(TAG, 'configuration is : ' + JSON.stringify(s19));
        if (!globalThis.params || !globalThis.params.isDefaultSelector) {
            console.debug(TAG, 'device is not phone');
            return;
        }
        try {
            let u19 = display.getDefaultDisplaySync();
            console.debug(TAG, 'display is' + JSON.stringify(u19));
            globalThis.position = PositionUtils.getSelectorDialogPosition(globalThis.lineNums);
            if (!globalThis.position) {
                console.error(TAG, 'globalThis.position is invalid, skip moveWindow');
                return;
            }
            let v19 = {
                left: globalThis.position.offsetX,
                top: globalThis.position.offsetY,
                width: globalThis.position.width,
                height: globalThis.position.height
            };
            AppStorage.SetOrCreate('oversizeHeight', globalThis.position.oversizeHeight ? 'true' : 'false');
            console.debug(TAG, 'onConfigurationUpdate navigationBarRect is' + JSON.stringify(v19));
            this.moveWindow(v19);
        }
        catch (t19) {
            console.error(TAG, 'onConfigurationUpdate moveWindow failed: ' + JSON.stringify(t19));
        }
    }
}
;
