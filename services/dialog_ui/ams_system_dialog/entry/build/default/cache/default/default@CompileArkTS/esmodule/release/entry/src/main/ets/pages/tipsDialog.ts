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
    constructor(p18, q18, r18, s18 = -1, t18 = undefined, u18) {
        super(p18, r18, s18, u18);
        if (typeof t18 === "function") {
            this.paramsGenerator_ = t18;
        }
        this.__isDefaultSelector = new ObservedPropertySimplePU(true, this, "isDefaultSelector");
        this.__btn = new ObservedPropertyObjectPU({ color: '#FFFFFF' }, this, "btn");
        this.TAG = 'TipsDialog_Page';
        this.setInitiallyProvidedValue(q18);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(o18: TipsDialog_Params) {
        if (o18.isDefaultSelector !== undefined) {
            this.isDefaultSelector = o18.isDefaultSelector;
        }
        if (o18.btn !== undefined) {
            this.btn = o18.btn;
        }
        if (o18.TAG !== undefined) {
            this.TAG = o18.TAG;
        }
    }
    updateStateVars(n18: TipsDialog_Params) {
    }
    purgeVariableDependenciesOnElmtId(m18) {
        this.__isDefaultSelector.purgeDependencyOnElmtId(m18);
        this.__btn.purgeDependencyOnElmtId(m18);
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
    set isDefaultSelector(l18: boolean) {
        this.__isDefaultSelector.set(l18);
    }
    private __btn: ObservedPropertyObjectPU<TipBtn>;
    get btn() {
        return this.__btn.get();
    }
    set btn(k18: TipBtn) {
        this.__btn.set(k18);
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
        this.observeComponentCreation((i18, j18) => {
            ViewStackProcessor.StartGetAccessRecordingFor(i18);
            Flex.create({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            Flex.borderRadius(20);
            Flex.borderWidth(1);
            Flex.borderColor('#e9e9e9');
            Flex.backgroundColor('#FFFFFF');
            if (!j18) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((g18, h18) => {
            ViewStackProcessor.StartGetAccessRecordingFor(g18);
            Flex.create({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            if (!h18) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((e18, f18) => {
            ViewStackProcessor.StartGetAccessRecordingFor(e18);
            Text.create({ "id": 33554466, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontSize(22);
            Text.fontWeight(FontWeight.Medium);
            Text.height('29%');
            Text.textOverflow({ overflow: TextOverflow.Ellipsis });
            Text.textAlign(TextAlign.Center);
            if (!f18) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Flex.pop();
        this.observeComponentCreation((c18, d18) => {
            ViewStackProcessor.StartGetAccessRecordingFor(c18);
            Flex.create({ justifyContent: FlexAlign.Center });
            Flex.margin({ top: 10 });
            if (!d18) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((a18, b18) => {
            ViewStackProcessor.StartGetAccessRecordingFor(a18);
            Flex.create({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            Flex.width(175);
            Flex.height(50);
            Flex.borderRadius(28);
            Flex.backgroundColor(this.btn.color);
            Flex.onClick(() => {
                this.onCloseApp();
            });
            if (!b18) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((y17, z17) => {
            ViewStackProcessor.StartGetAccessRecordingFor(y17);
            Text.create({ "id": 33554453, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontSize('21fp');
            Text.fontColor('#0A59F7');
            Text.fontWeight(FontWeight.Regular);
            Text.textAlign(TextAlign.Center);
            if (!z17) {
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
    public __resetStateVarsOnReuse__Internal(x17: Object): void {
    }
}
ViewStackProcessor.StartGetAccessRecordingFor(ViewStackProcessor.AllocateNewElmetIdForNextComponent());
loadDocument(new TipsDialog(undefined, {}));
ViewStackProcessor.StopGetAccessRecording();
