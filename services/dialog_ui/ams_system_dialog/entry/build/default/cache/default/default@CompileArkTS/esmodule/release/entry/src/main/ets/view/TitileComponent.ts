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
    constructor(n32, o32, p32, q32 = -1, r32 = undefined, s32) {
        super(n32, p32, q32, s32);
        if (typeof r32 === "function") {
            this.paramsGenerator_ = r32;
        }
        this.__level2Page = this.initializeConsume("level2Page", "level2Page");
        this.__isChange = this.initializeConsume("isChange", "isChange");
        this.__level10pacity = new ObservedPropertySimplePU(0, this, "level10pacity");
        this.__titleHeight = this.initializeConsume("titleHeight", "titleHeight");
        this.__notDisplayLevel1 = new ObservedPropertySimplePU(false, this, "notDisplayLevel1");
        this.storage = null;
        this.__currentColorMode = this.createStorageProp('currentColorMode', ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT, "currentColorMode");
        this.setInitiallyProvidedValue(o32);
        this.declareWatch("level2Page", this.onLevel2PageChanged);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(m32: TitleComponent_Params) {
        if (m32.level10pacity !== undefined) {
            this.level10pacity = m32.level10pacity;
        }
        if (m32.notDisplayLevel1 !== undefined) {
            this.notDisplayLevel1 = m32.notDisplayLevel1;
        }
        if (m32.storage !== undefined) {
            this.storage = m32.storage;
        }
    }
    updateStateVars(l32: TitleComponent_Params) {
    }
    purgeVariableDependenciesOnElmtId(k32) {
        this.__level2Page.purgeDependencyOnElmtId(k32);
        this.__isChange.purgeDependencyOnElmtId(k32);
        this.__level10pacity.purgeDependencyOnElmtId(k32);
        this.__titleHeight.purgeDependencyOnElmtId(k32);
        this.__notDisplayLevel1.purgeDependencyOnElmtId(k32);
        this.__currentColorMode.purgeDependencyOnElmtId(k32);
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
    set level2Page(j32: boolean) {
        this.__level2Page.set(j32);
    }
    private __isChange: ObservedPropertyAbstractPU<boolean>;
    get isChange() {
        return this.__isChange.get();
    }
    set isChange(i32: boolean) {
        this.__isChange.set(i32);
    }
    private __level10pacity: ObservedPropertySimplePU<number>;
    get level10pacity() {
        return this.__level10pacity.get();
    }
    set level10pacity(h32: number) {
        this.__level10pacity.set(h32);
    }
    private __titleHeight: ObservedPropertyAbstractPU<number>;
    get titleHeight() {
        return this.__titleHeight.get();
    }
    set titleHeight(g32: number) {
        this.__titleHeight.set(g32);
    }
    private __notDisplayLevel1: ObservedPropertySimplePU<boolean>;
    get notDisplayLevel1() {
        return this.__notDisplayLevel1.get();
    }
    set notDisplayLevel1(f32: boolean) {
        this.__notDisplayLevel1.set(f32);
    }
    private storage: LocalStorage | null;
    private __currentColorMode: ObservedPropertyAbstractPU<number>;
    get currentColorMode() {
        return this.__currentColorMode.get();
    }
    set currentColorMode(e32: number) {
        this.__currentColorMode.set(e32);
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
        this.observeComponentCreation((a32, b32) => {
            ViewStackProcessor.StartGetAccessRecordingFor(a32);
            Row.create();
            Row.height(56);
            Row.constraintSize({ minHeight: 32 });
            Row.justifyContent(FlexAlign.SpaceBetween);
            Row.alignItems(VerticalAlign.Center);
            Row.width('100%');
            Row.onAreaChange((c32: Area, d32: Area) => {
                if (c32.height !== d32.height) {
                    this.titleHeight = Number(d32.height);
                }
            });
            if (!b32) {
                Row.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((y31, z31) => {
            ViewStackProcessor.StartGetAccessRecordingFor(y31);
            Stack.create();
            Stack.alignContent(Alignment.Start);
            if (!z31) {
                Stack.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((w31, x31) => {
            ViewStackProcessor.StartGetAccessRecordingFor(w31);
            Row.create();
            if (!x31) {
                Row.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((t31, u31) => {
            ViewStackProcessor.StartGetAccessRecordingFor(t31);
            Button.createWithChild({ type: ButtonType.Normal });
            Button.opacity(this.level10pacity);
            Button.width(24);
            Button.height(24);
            Button.backgroundColor({ "id": 125829552, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Button.onClick((v31) => {
                this.level2Page = false;
            });
            if (!u31) {
                Button.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((r31, s31) => {
            ViewStackProcessor.StartGetAccessRecordingFor(r31);
            Image.create(this.currentColorMode === ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT ? { "id": 33554477, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" } : { "id": 33554478, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Image.width(24);
            Image.height(24);
            Image.draggable(false);
            if (!s31) {
                Image.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Button.pop();
        this.observeComponentCreation((p31, q31) => {
            ViewStackProcessor.StartGetAccessRecordingFor(p31);
            Text.create('All open methods');
            Text.fontColor({ "id": 125829210, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontSize({ "id": 125829676, "type": 10002, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontWeight(FontWeight.Medium);
            Text.maxLines(1);
            Text.textOverflow({ overflow: TextOverflow.Ellipsis });
            Text.textAlign(TextAlign.Start);
            Text.margin({ left: 16 });
            Text.opacity(this.level10pacity);
            if (!q31) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Row.pop();
        this.observeComponentCreation((l31, m31) => {
            ViewStackProcessor.StartGetAccessRecordingFor(l31);
            If.create();
            if (!this.notDisplayLevel1) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation((n31, o31) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(n31);
                        Text.create('Select open mode');
                        Text.fontColor({ "id": 125829210, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
                        Text.fontSize({ "id": 125829676, "type": 10002, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
                        Text.fontWeight(FontWeight.Medium);
                        Text.maxLines(1);
                        Text.textOverflow({ overflow: TextOverflow.Ellipsis });
                        Text.textAlign(TextAlign.Start);
                        Text.opacity(1 - this.level10pacity);
                        if (!o31) {
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
            if (!m31) {
                If.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        If.pop();
        Stack.pop();
        this.observeComponentCreation((h31, i31) => {
            ViewStackProcessor.StartGetAccessRecordingFor(h31);
            Button.createWithChild({ type: ButtonType.Circle });
            ViewStackProcessor.visualState("pressed");
            Button.backgroundColor({ "id": 125829165, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            ViewStackProcessor.visualState("normal");
            Button.backgroundColor({ "id": 125829552, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            ViewStackProcessor.visualState();
            Button.width(24);
            Button.height(24);
            Button.onClick((j31 => {
                const k31: UIExtensionContentSession = this.storage?.get('session') as UIExtensionContentSession;
                k31?.terminateSelf();
                this.storage?.clear();
            }));
            if (!i31) {
                Button.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((f31, g31) => {
            ViewStackProcessor.StartGetAccessRecordingFor(f31);
            Image.create(this.currentColorMode === ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT ? { "id": 33554479, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" } : { "id": 33554480, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Image.width(24);
            Image.height(24);
            Image.objectFit(ImageFit.Fill);
            Image.draggable(false);
            if (!g31) {
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
    public __resetStateVarsOnReuse__Internal(e31: Object): void {
        this.__reInitializeConsume__Internal("level2Page", "level2Page");
        this.__reInitializeConsume__Internal("isChange", "isChange");
        this.__reInitializeConsume__Internal("titleHeight", "titleHeight");
        this.__currentColorMode.resetSource();
    }
}
