if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
interface TitleComponent_Params {
    level2Page?: boolean;
    isChange?: boolean;
    level10pacity?: number;
    titleHeight?: number;
    notDisplayLevel1?: boolean;
    storage?: LocalStorage | null;
    currentColorMode?: number;
}
import ConfigurationConstant from "@ohos:app.ability.ConfigurationConstant";
import type UIExtensionContentSession from "@ohos:app.ability.UIExtensionContentSession";
export class TitleComponent extends ViewPU {
    constructor(b32, c32, d32, e32 = -1, f32 = undefined, g32) {
        super(b32, d32, e32, g32);
        if (typeof f32 === "function") {
            this.paramsGenerator_ = f32;
        }
        this.__level2Page = this.initializeConsume("level2Page", "level2Page");
        this.__isChange = this.initializeConsume("isChange", "isChange");
        this.__level10pacity = new ObservedPropertySimplePU(0, this, "level10pacity");
        this.__titleHeight = this.initializeConsume("titleHeight", "titleHeight");
        this.__notDisplayLevel1 = new ObservedPropertySimplePU(false, this, "notDisplayLevel1");
        this.storage = null;
        this.__currentColorMode = this.createStorageProp('currentColorMode', ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT, "currentColorMode");
        this.setInitiallyProvidedValue(c32);
        this.declareWatch("level2Page", this.onLevel2PageChanged);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(a32: TitleComponent_Params) {
        if (a32.level10pacity !== undefined) {
            this.level10pacity = a32.level10pacity;
        }
        if (a32.notDisplayLevel1 !== undefined) {
            this.notDisplayLevel1 = a32.notDisplayLevel1;
        }
        if (a32.storage !== undefined) {
            this.storage = a32.storage;
        }
    }
    updateStateVars(z31: TitleComponent_Params) {
    }
    purgeVariableDependenciesOnElmtId(y31) {
        this.__level2Page.purgeDependencyOnElmtId(y31);
        this.__isChange.purgeDependencyOnElmtId(y31);
        this.__level10pacity.purgeDependencyOnElmtId(y31);
        this.__titleHeight.purgeDependencyOnElmtId(y31);
        this.__notDisplayLevel1.purgeDependencyOnElmtId(y31);
        this.__currentColorMode.purgeDependencyOnElmtId(y31);
    }
    aboutToBeDeleted() {
        this.__level2Page.aboutToBeDeleted();
        this.__isChange.aboutToBeDeleted();
        this.__level10pacity.aboutToBeDeleted();
        this.__titleHeight.aboutToBeDeleted();
        this.__notDisplayLevel1.aboutToBeDeleted();
        this.__currentColorMode.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal();
    }
    private __level2Page: ObservedPropertyAbstractPU<boolean>;
    get level2Page() {
        return this.__level2Page.get();
    }
    set level2Page(x31: boolean) {
        this.__level2Page.set(x31);
    }
    private __isChange: ObservedPropertyAbstractPU<boolean>;
    get isChange() {
        return this.__isChange.get();
    }
    set isChange(w31: boolean) {
        this.__isChange.set(w31);
    }
    private __level10pacity: ObservedPropertySimplePU<number>;
    get level10pacity() {
        return this.__level10pacity.get();
    }
    set level10pacity(v31: number) {
        this.__level10pacity.set(v31);
    }
    private __titleHeight: ObservedPropertyAbstractPU<number>;
    get titleHeight() {
        return this.__titleHeight.get();
    }
    set titleHeight(u31: number) {
        this.__titleHeight.set(u31);
    }
    private __notDisplayLevel1: ObservedPropertySimplePU<boolean>;
    get notDisplayLevel1() {
        return this.__notDisplayLevel1.get();
    }
    set notDisplayLevel1(t31: boolean) {
        this.__notDisplayLevel1.set(t31);
    }
    private storage: LocalStorage | null;
    private __currentColorMode: ObservedPropertyAbstractPU<number>;
    get currentColorMode() {
        return this.__currentColorMode.get();
    }
    set currentColorMode(s31: number) {
        this.__currentColorMode.set(s31);
    }
    onLevel2PageChanged() {
        this.isChange = true;
        Context.animateTo({ duration: 300 }, () => {
            this.notDisplayLevel1 = this.level2Page;
            this.level10pacity = this.level2Page ? 1 : 0;
        });
    }
    onBackPress() {
        if (this.level2Page) {
            this.level2Page = false;
        }
    }
    initialRender() {
        this.observeComponentCreation((o31, p31) => {
            ViewStackProcessor.StartGetAccessRecordingFor(o31);
            Row.create();
            Row.height(56);
            Row.constraintSize({ minHeight: 32 });
            Row.justifyContent(FlexAlign.SpaceBetween);
            Row.alignItems(VerticalAlign.Center);
            Row.width('100%');
            Row.onAreaChange((q31: Area, r31: Area) => {
                if (q31.height !== r31.height) {
                    this.titleHeight = Number(r31.height);
                }
            });
            if (!p31) {
                Row.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((m31, n31) => {
            ViewStackProcessor.StartGetAccessRecordingFor(m31);
            Stack.create();
            Stack.alignContent(Alignment.Start);
            if (!n31) {
                Stack.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((k31, l31) => {
            ViewStackProcessor.StartGetAccessRecordingFor(k31);
            Row.create();
            if (!l31) {
                Row.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((h31, i31) => {
            ViewStackProcessor.StartGetAccessRecordingFor(h31);
            Button.createWithChild({ type: ButtonType.Normal });
            Button.opacity(this.level10pacity);
            Button.width(24);
            Button.height(24);
            Button.backgroundColor({ "id": 125829552, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Button.onClick((j31) => {
                this.level2Page = false;
            });
            if (!i31) {
                Button.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((f31, g31) => {
            ViewStackProcessor.StartGetAccessRecordingFor(f31);
            Image.create(this.currentColorMode === ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT ? { "id": 33554477, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" } : { "id": 33554478, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Image.width(24);
            Image.height(24);
            Image.draggable(false);
            if (!g31) {
                Image.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Button.pop();
        this.observeComponentCreation((d31, e31) => {
            ViewStackProcessor.StartGetAccessRecordingFor(d31);
            Text.create('All open methods');
            Text.fontColor({ "id": 125829210, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontSize({ "id": 125829676, "type": 10002, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontWeight(FontWeight.Medium);
            Text.maxLines(1);
            Text.textOverflow({ overflow: TextOverflow.Ellipsis });
            Text.textAlign(TextAlign.Start);
            Text.margin({ left: 16 });
            Text.opacity(this.level10pacity);
            if (!e31) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Row.pop();
        this.observeComponentCreation((z30, a31) => {
            ViewStackProcessor.StartGetAccessRecordingFor(z30);
            If.create();
            if (!this.notDisplayLevel1) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation((b31, c31) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(b31);
                        Text.create('Select open mode');
                        Text.fontColor({ "id": 125829210, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
                        Text.fontSize({ "id": 125829676, "type": 10002, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
                        Text.fontWeight(FontWeight.Medium);
                        Text.maxLines(1);
                        Text.textOverflow({ overflow: TextOverflow.Ellipsis });
                        Text.textAlign(TextAlign.Start);
                        Text.opacity(1 - this.level10pacity);
                        if (!c31) {
                            Text.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    Text.pop();
                });
            }
            else {
                this.ifElseBranchUpdateFunction(1, () => {
                });
            }
            if (!a31) {
                If.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        If.pop();
        Stack.pop();
        this.observeComponentCreation((v30, w30) => {
            ViewStackProcessor.StartGetAccessRecordingFor(v30);
            Button.createWithChild({ type: ButtonType.Circle });
            ViewStackProcessor.visualState("pressed");
            Button.backgroundColor({ "id": 125829165, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            ViewStackProcessor.visualState("normal");
            Button.backgroundColor({ "id": 125829552, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            ViewStackProcessor.visualState();
            Button.width(24);
            Button.height(24);
            Button.onClick((x30 => {
                const y30: UIExtensionContentSession = this.storage?.get('session') as UIExtensionContentSession;
                y30?.terminateSelf();
                this.storage?.clear();
            }));
            if (!w30) {
                Button.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((t30, u30) => {
            ViewStackProcessor.StartGetAccessRecordingFor(t30);
            Image.create(this.currentColorMode === ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT ? { "id": 33554479, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" } : { "id": 33554480, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Image.width(24);
            Image.height(24);
            Image.objectFit(ImageFit.Fill);
            Image.draggable(false);
            if (!u30) {
                Image.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Button.pop();
        Row.pop();
    }
    rerender() {
        this.updateDirtyElements();
    }
}
