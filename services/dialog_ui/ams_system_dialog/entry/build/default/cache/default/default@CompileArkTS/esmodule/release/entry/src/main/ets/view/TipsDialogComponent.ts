if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
interface TipsDialogComponent_Params {
    storage?: LocalStorage;
    controller?: CustomDialogController;
}
import type UIExtensionContentSession from "@ohos:app.ability.UIExtensionContentSession";
import { localStorageKeyConstants } from "@bundle:com.ohos.amsdialog/entry/ets/utils/Constants";
export class TipsDialogComponent extends ViewPU {
    constructor(y30, z30, a31, b31 = -1, c31 = undefined, d31) {
        super(y30, a31, b31, d31);
        if (typeof c31 === "function") {
            this.paramsGenerator_ = c31;
        }
        this.storage = undefined;
        this.controller = undefined;
        this.setInitiallyProvidedValue(z30);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(x30: TipsDialogComponent_Params) {
        if (x30.storage !== undefined) {
            this.storage = x30.storage;
        }
        if (x30.controller !== undefined) {
            this.controller = x30.controller;
        }
    }
    updateStateVars(w30: TipsDialogComponent_Params) {
    }
    purgeVariableDependenciesOnElmtId(v30) {
    }
    aboutToBeDeleted() {
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal();
    }
    private storage?: LocalStorage;
    private controller?: CustomDialogController;
    setController(u30: CustomDialogController) {
        this.controller = u30;
    }
    initialRender() {
        this.observeComponentCreation((s30, t30) => {
            ViewStackProcessor.StartGetAccessRecordingFor(s30);
            Column.create();
            if (!t30) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((q30, r30) => {
            ViewStackProcessor.StartGetAccessRecordingFor(q30);
            Row.create();
            Row.padding({
                left: 24,
                right: 24
            });
            Row.margin({
                top: 24
            });
            if (!r30) {
                Row.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((o30, p30) => {
            ViewStackProcessor.StartGetAccessRecordingFor(o30);
            Text.create('No available applications targets');
            Text.fontSize({ "id": 125829684, "type": 10002, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontWeight(FontWeight.Regular);
            Text.fontColor({ "id": 125829210, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            if (!p30) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Row.pop();
        this.observeComponentCreation((m30, n30) => {
            ViewStackProcessor.StartGetAccessRecordingFor(m30);
            Row.create();
            Row.padding({
                left: 16,
                right: 16
            });
            Row.margin({
                top: 8,
                bottom: 16
            });
            if (!n30) {
                Row.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((j30, k30) => {
            ViewStackProcessor.StartGetAccessRecordingFor(j30);
            Button.createWithChild({ type: ButtonType.Normal, stateEffect: false });
            Button.height(40);
            Button.constraintSize({ minHeight: 40 });
            Button.backgroundColor(Color.Transparent);
            Button.onClick(() => {
                this.controller?.close();
                const l30: UIExtensionContentSession = this.storage?.get(localStorageKeyConstants.SESSION) as UIExtensionContentSession;
                l30?.terminateSelf();
                this.storage?.clear();
            });
            if (!k30) {
                Button.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((h30, i30) => {
            ViewStackProcessor.StartGetAccessRecordingFor(h30);
            Text.create('Got it');
            Text.fontSize({ "id": 125829681, "type": 10002, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontWeight(FontWeight.Medium);
            Text.fontColor({ "id": 125829231, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.maxLines(1);
            if (!i30) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Button.pop();
        Row.pop();
        Column.pop();
    }
    rerender() {
        this.updateDirtyElements();
    }
}
