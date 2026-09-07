if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
interface TipsDialog_Params {
    isDefaultSelector?: boolean;
    btn?: TipBtn;
    TAG?;
}
interface TipBtn {
    color: string;
}
class TipsDialog extends ViewPU {
    constructor(h18, i18, j18, k18 = -1, l18 = undefined, m18) {
        super(h18, j18, k18, m18);
        if (typeof l18 === "function") {
            this.paramsGenerator_ = l18;
        }
        this.__isDefaultSelector = new ObservedPropertySimplePU(true, this, "isDefaultSelector");
        this.__btn = new ObservedPropertyObjectPU({ color: '#FFFFFF' }, this, "btn");
        this.TAG = 'TipsDialog_Page';
        this.setInitiallyProvidedValue(i18);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(g18: TipsDialog_Params) {
        if (g18.isDefaultSelector !== undefined) {
            this.isDefaultSelector = g18.isDefaultSelector;
        }
        if (g18.btn !== undefined) {
            this.btn = g18.btn;
        }
        if (g18.TAG !== undefined) {
            this.TAG = g18.TAG;
        }
    }
    updateStateVars(f18: TipsDialog_Params) {
    }
    purgeVariableDependenciesOnElmtId(e18) {
        this.__isDefaultSelector.purgeDependencyOnElmtId(e18);
        this.__btn.purgeDependencyOnElmtId(e18);
    }
    aboutToBeDeleted() {
        this.__isDefaultSelector.aboutToBeDeleted();
        this.__btn.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal();
    }
    private __isDefaultSelector: ObservedPropertySimplePU<boolean>;
    get isDefaultSelector() {
        return this.__isDefaultSelector.get();
    }
    set isDefaultSelector(d18: boolean) {
        this.__isDefaultSelector.set(d18);
    }
    private __btn: ObservedPropertyObjectPU<TipBtn>;
    get btn() {
        return this.__btn.get();
    }
    set btn(c18: TipBtn) {
        this.__btn.set(c18);
    }
    private TAG;
    aboutToAppear() {
        console.log(this.TAG, 'dialog page appears');
        this.isDefaultSelector = globalThis.params.isDefaultSelector;
        if (this.isDefaultSelector == false) {
            this.btn.color = '#F2F2F2';
        }
    }
    onCloseApp() {
        console.info(this.TAG, 'click close app');
        globalThis.tipsExtensionContext.terminateSelf();
    }
    initialRender() {
        this.observeComponentCreation((a18, b18) => {
            ViewStackProcessor.StartGetAccessRecordingFor(a18);
            Flex.create({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            Flex.borderRadius(20);
            Flex.borderWidth(1);
            Flex.borderColor('#e9e9e9');
            Flex.backgroundColor('#FFFFFF');
            if (!b18) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((y17, z17) => {
            ViewStackProcessor.StartGetAccessRecordingFor(y17);
            Flex.create({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            if (!z17) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((w17, x17) => {
            ViewStackProcessor.StartGetAccessRecordingFor(w17);
            Text.create({ "id": 33554466, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontSize(22);
            Text.fontWeight(FontWeight.Medium);
            Text.height('29%');
            Text.textOverflow({ overflow: TextOverflow.Ellipsis });
            Text.textAlign(TextAlign.Center);
            if (!x17) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Flex.pop();
        this.observeComponentCreation((u17, v17) => {
            ViewStackProcessor.StartGetAccessRecordingFor(u17);
            Flex.create({ justifyContent: FlexAlign.Center });
            Flex.margin({ top: 10 });
            if (!v17) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((s17, t17) => {
            ViewStackProcessor.StartGetAccessRecordingFor(s17);
            Flex.create({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            Flex.width(175);
            Flex.height(50);
            Flex.borderRadius(28);
            Flex.backgroundColor(this.btn.color);
            Flex.onClick(() => {
                this.onCloseApp();
            });
            if (!t17) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((q17, r17) => {
            ViewStackProcessor.StartGetAccessRecordingFor(q17);
            Text.create({ "id": 33554453, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontSize('21fp');
            Text.fontColor('#0A59F7');
            Text.fontWeight(FontWeight.Regular);
            Text.textAlign(TextAlign.Center);
            if (!r17) {
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
ViewStackProcessor.StartGetAccessRecordingFor(ViewStackProcessor.AllocateNewElmetIdForNextComponent());
loadDocument(new TipsDialog(undefined, {}));
ViewStackProcessor.StopGetAccessRecording();
