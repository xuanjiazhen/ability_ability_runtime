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
    constructor(u3, v3, w3, x3 = -1, y3 = undefined, z3) {
        super(u3, w3, x3, z3);
        if (typeof y3 === "function") {
            this.paramsGenerator_ = y3;
        }
        this.__btn = new ObservedPropertyObjectPU({ color: '#FFFFFF' }, this, "btn");
        this.TAG = 'permissionconfirm_Page';
        this.setInitiallyProvidedValue(v3);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(t3: PermissionConfirmDialog_Params) {
        if (t3.btn !== undefined) {
            this.btn = t3.btn;
        }
        if (t3.TAG !== undefined) {
            this.TAG = t3.TAG;
        }
    }
    updateStateVars(s3: PermissionConfirmDialog_Params) {
    }
    purgeVariableDependenciesOnElmtId(r3) {
        this.__btn.purgeDependencyOnElmtId(r3);
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
    set btn(q3: TipBtn) {
        this.__btn.set(q3);
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
        catch (p3) {
            console.error(this.TAG, 'Failed to replaceUrl. Cause: ' + JSON.stringify(p3));
        }
    }
    initialRender() {
        this.observeComponentCreation((n3, o3) => {
            ViewStackProcessor.StartGetAccessRecordingFor(n3);
            Flex.create({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            Flex.borderRadius(20);
            Flex.borderWidth(1);
            Flex.borderColor('#e9e9e9');
            Flex.backgroundColor('#FFFFFF');
            if (!o3) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((l3, m3) => {
            ViewStackProcessor.StartGetAccessRecordingFor(l3);
            Flex.create({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            if (!m3) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((j3, k3) => {
            ViewStackProcessor.StartGetAccessRecordingFor(j3);
            Text.create({ "id": 33554463, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontSize(16);
            Text.fontWeight(FontWeight.Medium);
            Text.height('29%');
            Text.textOverflow({ overflow: TextOverflow.Ellipsis });
            Text.textAlign(TextAlign.Center);
            if (!k3) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Flex.pop();
        this.observeComponentCreation((h3, i3) => {
            ViewStackProcessor.StartGetAccessRecordingFor(h3);
            Flex.create({ justifyContent: FlexAlign.Center });
            Flex.margin({ top: 10 });
            if (!i3) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((f3, g3) => {
            ViewStackProcessor.StartGetAccessRecordingFor(f3);
            Flex.create({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            Flex.width(175);
            Flex.height(50);
            Flex.borderRadius(28);
            Flex.backgroundColor(this.btn.color);
            Flex.onClick(() => {
                this.onConfirmApp();
            });
            if (!g3) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((d3, e3) => {
            ViewStackProcessor.StartGetAccessRecordingFor(d3);
            Text.create({ "id": 33554455, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontSize('21fp');
            Text.fontColor('#0A59F7');
            Text.fontWeight(FontWeight.Regular);
            Text.textAlign(TextAlign.Center);
            if (!e3) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Flex.pop();
        Flex.pop();
        this.observeComponentCreation((b3, c3) => {
            ViewStackProcessor.StartGetAccessRecordingFor(b3);
            Flex.create({ justifyContent: FlexAlign.Center });
            Flex.margin({ top: 10 });
            if (!c3) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((z2, a3) => {
            ViewStackProcessor.StartGetAccessRecordingFor(z2);
            Flex.create({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            Flex.width(175);
            Flex.height(50);
            Flex.borderRadius(28);
            Flex.backgroundColor(this.btn.color);
            Flex.onClick(() => {
                this.onCloseApp();
            });
            if (!a3) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((x2, y2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(x2);
            Text.create({ "id": 33554451, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontSize('21fp');
            Text.fontColor('#0A59F7');
            Text.fontWeight(FontWeight.Regular);
            Text.textAlign(TextAlign.Center);
            if (!y2) {
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
