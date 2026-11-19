if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
interface PermissionConfirmDialog_Params {
    btn?: TipBtn;
    TAG?;
}
import router from "@ohos:router";
import type UIExtensionContentSession from "@ohos:app.ability.UIExtensionContentSession";
interface TipBtn {
    color: string;
}
let storage = LocalStorage.GetShared();
class PermissionConfirmDialog extends ViewPU {
    constructor(x3, y3, z3, a4 = -1, b4 = undefined, c4) {
        super(x3, z3, a4, c4);
        if (typeof b4 === "function") {
            this.paramsGenerator_ = b4;
        }
        this.__btn = new ObservedPropertyObjectPU({ color: '#FFFFFF' }, this, "btn");
        this.TAG = 'permissionconfirm_Page';
        this.setInitiallyProvidedValue(y3);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(w3: PermissionConfirmDialog_Params) {
        if (w3.btn !== undefined) {
            this.btn = w3.btn;
        }
        if (w3.TAG !== undefined) {
            this.TAG = w3.TAG;
        }
    }
    updateStateVars(v3: PermissionConfirmDialog_Params) {
    }
    purgeVariableDependenciesOnElmtId(u3) {
        this.__btn.purgeDependencyOnElmtId(u3);
    }
    aboutToBeDeleted() {
        this.__btn.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal();
    }
    private __btn: ObservedPropertyObjectPU<TipBtn>;
    get btn() {
        return this.__btn.get();
    }
    set btn(t3: TipBtn) {
        this.__btn.set(t3);
    }
    private TAG;
    onCloseApp() {
        console.info(this.TAG, 'click close app');
        if (globalThis.ExtensionType === 'ServiceExtension') {
            globalThis.currentExtensionContext.terminateSelf();
        }
        else {
            storage.get<UIExtensionContentSession>('session').terminateSelf();
        }
    }
    onConfirmApp() {
        console.info(this.TAG, 'click confirm app');
        globalThis.preferences.putSync('isdialogconfirmpermission', 'true');
        globalThis.preferences.flush();
        try {
            router.replaceUrl({
                url: globalThis.currentURL,
            });
        }
        catch (s3) {
            console.error(this.TAG, 'Failed to replaceUrl. Cause: ' + JSON.stringify(s3));
        }
    }
    initialRender() {
        this.observeComponentCreation((q3, r3) => {
            ViewStackProcessor.StartGetAccessRecordingFor(q3);
            Flex.create({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            Flex.borderRadius(20);
            Flex.borderWidth(1);
            Flex.borderColor('#e9e9e9');
            Flex.backgroundColor('#FFFFFF');
            if (!r3) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((o3, p3) => {
            ViewStackProcessor.StartGetAccessRecordingFor(o3);
            Flex.create({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            if (!p3) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((m3, n3) => {
            ViewStackProcessor.StartGetAccessRecordingFor(m3);
            Text.create({ "id": 33554463, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontSize(16);
            Text.fontWeight(FontWeight.Medium);
            Text.height('29%');
            Text.textOverflow({ overflow: TextOverflow.Ellipsis });
            Text.textAlign(TextAlign.Center);
            if (!n3) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Flex.pop();
        this.observeComponentCreation((k3, l3) => {
            ViewStackProcessor.StartGetAccessRecordingFor(k3);
            Flex.create({ justifyContent: FlexAlign.Center });
            Flex.margin({ top: 10 });
            if (!l3) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((i3, j3) => {
            ViewStackProcessor.StartGetAccessRecordingFor(i3);
            Flex.create({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            Flex.width(175);
            Flex.height(50);
            Flex.borderRadius(28);
            Flex.backgroundColor(this.btn.color);
            Flex.onClick(() => {
                this.onConfirmApp();
            });
            if (!j3) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((g3, h3) => {
            ViewStackProcessor.StartGetAccessRecordingFor(g3);
            Text.create({ "id": 33554455, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontSize('21fp');
            Text.fontColor('#0A59F7');
            Text.fontWeight(FontWeight.Regular);
            Text.textAlign(TextAlign.Center);
            if (!h3) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Flex.pop();
        Flex.pop();
        this.observeComponentCreation((e3, f3) => {
            ViewStackProcessor.StartGetAccessRecordingFor(e3);
            Flex.create({ justifyContent: FlexAlign.Center });
            Flex.margin({ top: 10 });
            if (!f3) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((c3, d3) => {
            ViewStackProcessor.StartGetAccessRecordingFor(c3);
            Flex.create({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            Flex.width(175);
            Flex.height(50);
            Flex.borderRadius(28);
            Flex.backgroundColor(this.btn.color);
            Flex.onClick(() => {
                this.onCloseApp();
            });
            if (!d3) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((a3, b3) => {
            ViewStackProcessor.StartGetAccessRecordingFor(a3);
            Text.create({ "id": 33554451, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontSize('21fp');
            Text.fontColor('#0A59F7');
            Text.fontWeight(FontWeight.Regular);
            Text.textAlign(TextAlign.Center);
            if (!b3) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Flex.pop();
        Flex.pop();
        Flex.pop();
    }
    rerender() {
        this.updateDirtyElements();
    }
    public __resetStateVarsOnReuse__Internal(z2: Object): void {
    }
}
if (storage && storage.routeName != undefined && storage.storage != undefined) {
    registerNamedRoute(() => new PermissionConfirmDialog(undefined, {}, storage.useSharedStorage ? LocalStorage.getShared() : storage.storage), storage.routeName, { bundleName: "com.ohos.amsdialog", moduleName: "entry", pagePath: "pages/permissionConfirmDialog", pageFullPath: "entry/src/main/ets/pages/permissionConfirmDialog", integratedHsp: "false", moduleType: "followWithHap" });
}
else if (storage && storage.routeName != undefined && storage.storage == undefined) {
    registerNamedRoute(() => new PermissionConfirmDialog(undefined, {}, storage.useSharedStorage ? LocalStorage.getShared() : storage.storage), storage.routeName, { bundleName: "com.ohos.amsdialog", moduleName: "entry", pagePath: "pages/permissionConfirmDialog", pageFullPath: "entry/src/main/ets/pages/permissionConfirmDialog", integratedHsp: "false", moduleType: "followWithHap" });
}
else if (storage && storage.routeName == undefined && storage.storage != undefined) {
    ViewStackProcessor.StartGetAccessRecordingFor(ViewStackProcessor.AllocateNewElmetIdForNextComponent());
    loadDocument(new PermissionConfirmDialog(undefined, {}, storage.useSharedStorage ? LocalStorage.getShared() : storage.storage));
    ViewStackProcessor.StopGetAccessRecording();
}
else if (storage && storage.useSharedStorage != undefined) {
    ViewStackProcessor.StartGetAccessRecordingFor(ViewStackProcessor.AllocateNewElmetIdForNextComponent());
    loadDocument(new PermissionConfirmDialog(undefined, {}, storage.useSharedStorage ? LocalStorage.getShared() : undefined));
    ViewStackProcessor.StopGetAccessRecording();
}
else {
    ViewStackProcessor.StartGetAccessRecordingFor(ViewStackProcessor.AllocateNewElmetIdForNextComponent());
    loadDocument(new PermissionConfirmDialog(undefined, {}, storage));
    ViewStackProcessor.StopGetAccessRecording();
}
