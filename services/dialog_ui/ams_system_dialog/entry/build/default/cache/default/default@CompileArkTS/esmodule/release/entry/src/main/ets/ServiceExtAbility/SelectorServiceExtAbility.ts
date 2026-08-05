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
    onCreate(t21) {
        console.debug(TAG, 'onCreate, want: ' + JSON.stringify(t21));
        globalThis.selectExtensionContext = this.context;
        globalThis.currentExtensionContext = this.context;
        globalThis.ExtensionType = 'ServiceExtension';
        globalThis.defaultAppManager = defaultAppManager;
        globalThis.bundleManager = bundleManager;
        let u21 = { name: 'dialogStore' };
        try {
            globalThis.preferences = dataPreferences.getPreferencesSync(this.context, u21);
        }
        catch (v21) {
            console.error(TAG, 'Failed to getPreferencesSync callback. Code: ' + JSON.stringify(v21));
        }
    }
    async getPhoneShowHapList() {
        const k21 = 8;
        let l21 = [];
        let m21 = [];
        let n21: Map<string, image.PixelMap> = new Map();
        for (let s21 = 1; s21 <= globalThis.params.hapList.length; s21++) {
            console.info(TAG, 'hapList[' + (s21 - 1).toString() + ']: ' + JSON.stringify(globalThis.params.hapList[s21 - 1]));
            await this.getHapResource(globalThis.params.hapList[s21 - 1], l21, n21);
            if (s21 % k21 === 0) {
                m21.push(l21);
                l21 = [];
            }
            if (s21 >= globalThis.params.hapList.length && l21.length > 0) {
                m21.push(l21);
            }
        }
        globalThis.phoneShowHapList = m21;
        console.debug(TAG, 'phoneShowHapList: ' + JSON.stringify(m21));
        const o21 = 4;
        let p21 = [];
        let q21 = [];
        for (let r21 = 1; r21 <= globalThis.params.hapList.length; r21++) {
            console.info(TAG, 'hapList[' + (r21 - 1).toString() + ']: ' + JSON.stringify(globalThis.params.hapList[r21 - 1]));
            await this.getHapResource(globalThis.params.hapList[r21 - 1], p21, n21);
            if (r21 % o21 === 0) {
                q21.push(p21);
                p21 = [];
            }
            if (r21 >= globalThis.params.hapList.length && p21.length > 0) {
                q21.push(p21);
            }
        }
        globalThis.signalRowPhoneShowHapList = q21;
        globalThis.jsonIconMap = n21;
        console.debug(TAG, 'signalRowPhoneShowHapList: ' + JSON.stringify(q21));
    }
    async getPcShowHapList() {
        let h21 = [];
        let i21: Map<string, image.PixelMap> = new Map();
        for (let j21 = 0; j21 < globalThis.params.hapList.length; j21++) {
            await this.getHapResource(globalThis.params.hapList[j21], h21, i21);
        }
        globalThis.pcShowHapList = h21;
        globalThis.jsonIconMap = i21;
        console.debug(TAG, 'pcShowHapList: ' + JSON.stringify(h21));
    }
    async getHapResource(l20, m20, n20) {
        let o20 = l20.bundle;
        let p20 = l20.module;
        let q20 = l20.ability;
        let r20 = '';
        let s20 = '';
        let t20 = '';
        let u20 = Number('0');
        let v20 = Number(l20.appIndex);
        if (!globalThis.params.isDefaultSelector) {
            t20 = l20.type;
            u20 = Number(l20.userId);
        }
        let w20 = Number(l20.label);
        if (w20 === 0) {
            w20 = Number(l20.bundleLabel);
        }
        let x20 = globalThis.selectExtensionContext.createModuleContext(o20, p20);
        await x20.resourceManager.getString(w20).then(g21 => {
            if (v20 === 0) {
                r20 = g21;
            }
            else {
                r20 = g21 + l20.appIndex;
            }
        }).catch(f21 => {
            console.error(TAG, 'getString error:' + JSON.stringify(f21));
        });
        let y20 = Number(l20.icon);
        if (y20 === 0) {
            y20 = Number(l20.bundleIcon);
        }
        await x20.resourceManager.getMediaBase64(y20).then(a21 => {
            s20 = a21;
            if (s20.indexOf('image/json') > -1) {
                try {
                    const c21 = x20.resourceManager.getDrawableDescriptor(y20);
                    if (c21 !== null && c21 !== undefined &&
                        c21 instanceof drawableDescriptor.LayeredDrawableDescriptor) {
                        let d21: drawableDescriptor.LayeredDrawableDescriptor = <drawableDescriptor.LayeredDrawableDescriptor>c21;
                        let e21: drawableDescriptor.DrawableDescriptor = d21.getForeground();
                        if (e21 !== null && e21 !== undefined) {
                            n20.set(o20 + ':' + p20 + ':' + q20 + ':' + l20.appIndex, e21.getPixelMap());
                        }
                        else {
                            console.error(TAG, 'get foregroundDescriptor is null');
                        }
                    }
                }
                catch (b21) {
                    console.error(TAG, 'get drawableDescriptor error:' + JSON.stringify(b21));
                }
            }
        }).catch(z20 => {
            console.error(TAG, 'getMediaBase64 error:' + JSON.stringify(z20));
        });
        m20.push(o20 + '#' + q20 + '#' + r20 +
            '#' + s20 + '#' + p20 + '#' + t20 + '#' + u20 + '#' + v20);
    }
    async onRequest(w19, x19) {
        console.debug(TAG, 'onRequest, want: ' + JSON.stringify(w19));
        globalThis.abilityWant = w19;
        try {
            globalThis.params = JSON.parse(w19.parameters.params);
            globalThis.callerToken = w19.parameters.callerToken;
            let h20 = 0;
            if (globalThis.params && globalThis.params.hapList && globalThis.params.hapList.length) {
                h20 = globalThis.params.hapList.length;
            }
            globalThis.lineNums = h20;
            globalThis.position = PositionUtils.getSelectorDialogPosition(h20);
            display.on('change', (i20: number) => {
                try {
                    if (!globalThis.position) {
                        console.error(TAG, 'globalThis.position is invalid in display change callback, skip');
                        return;
                    }
                    let k20 = PositionUtils.getSelectorDialogPosition(h20);
                    if (!k20) {
                        console.error(TAG, 'position is invalid in display change callback, skip');
                        return;
                    }
                    if (k20.offsetX !== globalThis.position.offsetX || k20.offsetY !== globalThis.position.offsetY) {
                        win.moveTo(k20.offsetX, k20.offsetY);
                    }
                    if (k20.width !== globalThis.position.width || k20.height !== globalThis.position.height) {
                        win.resetSize(k20.width, k20.height);
                    }
                    globalThis.position = k20;
                }
                catch (j20) {
                    console.error(TAG, 'display change callback failed: ' + JSON.stringify(j20));
                }
            });
        }
        catch (g20) {
            console.error(TAG, 'Failed to register display change callback. Code: ' + JSON.stringify(g20));
        }
        try {
            let f20 = display.getDefaultDisplaySync();
            console.debug(TAG, 'onRequest display is' + JSON.stringify(f20));
        }
        catch (e20) {
            console.error(TAG, 'Failed to getDefaultDisplaySync callback. Code: ' + JSON.stringify(e20));
        }
        console.debug(TAG, 'onRequest, want: ' + JSON.stringify(w19));
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
        catch (d20) {
            console.error(TAG, 'Failed to get hap list: ' + JSON.stringify(d20));
        }
        try {
            AppStorage.SetOrCreate('oversizeHeight', globalThis.position.oversizeHeight ? 'true' : 'false');
        }
        catch (c20) {
            console.error(TAG, 'Failed to set oversizeHeight: ' + JSON.stringify(c20));
        }
        display.getDefaultDisplay().then(y19 => {
            try {
                if (!globalThis.position) {
                    console.error(TAG, 'globalThis.position is invalid in getDefaultDisplay callback, skip');
                    return;
                }
                let a20 = {
                    left: globalThis.position.offsetX,
                    top: globalThis.position.offsetY,
                    width: globalThis.position.width,
                    height: globalThis.position.height
                };
                if (winNum > 1 && win) {
                    win.destroy();
                    winNum--;
                }
                let b20 = (typeof (globalThis.callerToken) === 'object' && globalThis.callerToken !== null) ?
                    window.WindowType.TYPE_DIALOG : window.WindowType.TYPE_SYSTEM_ALERT;
                this.createWindow('SelectorDialog' + x19, b20, a20);
                winNum++;
            }
            catch (z19) {
                console.error(TAG, 'display getDefaultDisplay callback failed: ' + JSON.stringify(z19));
            }
        });
    }
    onDestroy() {
        console.info(TAG, 'SelectorServiceExtensionAbility onDestroy.');
        if (win !== undefined) {
            win.destroy();
        }
    }
    private async createWindow(r19: string, s19: number, t19) {
        let u19 = deviceInfo.deviceType;
        console.info(TAG, 'create window');
        try {
            win = await window.create(globalThis.selectExtensionContext, r19, s19);
            if (s19 === window.WindowType.TYPE_DIALOG) {
                await win.bindDialogTarget(globalThis.callerToken.value, () => {
                    win.destroyWindow();
                    winNum--;
                    if (winNum === 0) {
                        globalThis.selectExtensionContext.terminateSelf();
                    }
                });
            }
            if (u19 !== 'default') {
                await win.hideNonSystemFloatingWindows(true);
            }
            await win.moveTo(t19.left, t19.top);
            await win.resetSize(t19.width, t19.height);
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
        catch (v19) {
            console.error(TAG, 'window create failed: ' + JSON.stringify(v19));
        }
    }
    private async moveWindow(o19): Promise<void> {
        try {
            await win.moveTo(o19.left, o19.top);
            await win.resetSize(o19.width, o19.height);
            if (globalThis.params.isDefaultSelector) {
                try {
                    await win.loadContent('pages/selectorPhoneDialog');
                    await win.setBackgroundColor('#00000000');
                }
                catch (q19) {
                    console.error(TAG, 'window loadContent failed: ' + JSON.stringify(q19));
                }
            }
            await win.show();
        }
        catch (p19) {
            console.error(TAG, 'window move failed: ' + JSON.stringify(p19));
        }
    }
    onConfigurationUpdate(k19): void {
        console.debug(TAG, 'configuration is : ' + JSON.stringify(k19));
        if (!globalThis.params || !globalThis.params.isDefaultSelector) {
            console.debug(TAG, 'device is not phone');
            return;
        }
        try {
            let m19 = display.getDefaultDisplaySync();
            console.debug(TAG, 'display is' + JSON.stringify(m19));
            globalThis.position = PositionUtils.getSelectorDialogPosition(globalThis.lineNums);
            if (!globalThis.position) {
                console.error(TAG, 'globalThis.position is invalid, skip moveWindow');
                return;
            }
            let n19 = {
                left: globalThis.position.offsetX,
                top: globalThis.position.offsetY,
                width: globalThis.position.width,
                height: globalThis.position.height
            };
            AppStorage.SetOrCreate('oversizeHeight', globalThis.position.oversizeHeight ? 'true' : 'false');
            console.debug(TAG, 'onConfigurationUpdate navigationBarRect is' + JSON.stringify(n19));
            this.moveWindow(n19);
        }
        catch (l19) {
            console.error(TAG, 'onConfigurationUpdate moveWindow failed: ' + JSON.stringify(l19));
        }
    }
}
;
