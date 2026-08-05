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
    constructor(n30, o30, p30, q30 = -1, r30 = undefined, s30) {
        super(n30, p30, q30, s30);
        if (typeof r30 === "function") {
            this.paramsGenerator_ = r30;
        }
        this.storage = undefined;
        this.controller = undefined;
        this.setInitiallyProvidedValue(o30);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(m30: TipsDialogComponent_Params) {
        if (m30.storage !== undefined) {
            this.storage = m30.storage;
        }
        if (m30.controller !== undefined) {
            this.controller = m30.controller;
        }
    }
    updateStateVars(l30: TipsDialogComponent_Params) {
    }
    purgeVariableDependenciesOnElmtId(k30) {
    }
    aboutToBeDeleted() {
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal();
    }
    private storage?: LocalStorage;
    private controller?: CustomDialogController;
    setController(j30: CustomDialogController) {
        this.controller = j30;
    }
    initialRender() {
        this.observeComponentCreation((h30, i30) => {
            ViewStackProcessor.StartGetAccessRecordingFor(h30);
            Column.create();
            if (!i30) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((f30, g30) => {
            ViewStackProcessor.StartGetAccessRecordingFor(f30);
            Row.create();
            Row.padding({
                left: 24,
                right: 24
            });
            Row.margin({
                top: 24
            });
            if (!g30) {
                Row.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((d30, e30) => {
            ViewStackProcessor.StartGetAccessRecordingFor(d30);
            Text.create('No available applications targets');
            Text.fontSize({ "id": 125829684, "type": 10002, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontWeight(FontWeight.Regular);
            Text.fontColor({ "id": 125829210, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            if (!e30) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Row.pop();
        this.observeComponentCreation((b30, c30) => {
            ViewStackProcessor.StartGetAccessRecordingFor(b30);
            Row.create();
            Row.padding({
                left: 16,
                right: 16
            });
            Row.margin({
                top: 8,
                bottom: 16
            });
            if (!c30) {
                Row.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((y29, z29) => {
            ViewStackProcessor.StartGetAccessRecordingFor(y29);
            Button.createWithChild({ type: ButtonType.Normal, stateEffect: false });
            Button.height(40);
            Button.constraintSize({ minHeight: 40 });
            Button.backgroundColor(Color.Transparent);
            Button.onClick(() => {
                this.controller?.close();
                const a30: UIExtensionContentSession = this.storage?.get(localStorageKeyConstants.SESSION) as UIExtensionContentSession;
                a30?.terminateSelf();
                this.storage?.clear();
            });
            if (!z29) {
                Button.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((w29, x29) => {
            ViewStackProcessor.StartGetAccessRecordingFor(w29);
            Text.create('Got it');
            Text.fontSize({ "id": 125829681, "type": 10002, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontWeight(FontWeight.Medium);
            Text.fontColor({ "id": 125829231, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.maxLines(1);
            if (!x29) {
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
