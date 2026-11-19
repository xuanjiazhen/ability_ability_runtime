if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
interface SwitchUserDialog_Params {
    TAG?;
}
class SwitchUserDialog extends ViewPU {
    constructor(r17, s17, t17, u17 = -1, v17 = undefined, w17) {
        super(r17, t17, u17, w17);
        if (typeof v17 === "function") {
            this.paramsGenerator_ = v17;
        }
        this.TAG = 'SwitchUserDialog_Page';
        this.setInitiallyProvidedValue(s17);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(q17: SwitchUserDialog_Params) {
        if (q17.TAG !== undefined) {
            this.TAG = q17.TAG;
        }
    }
    updateStateVars(p17: SwitchUserDialog_Params) {
    }
    purgeVariableDependenciesOnElmtId(o17) {
    }
    aboutToBeDeleted() {
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal();
    }
    private TAG;
    aboutToAppear() {
        console.info(this.TAG, 'dialog page appears');
    }
    initialRender() {
        this.observeComponentCreation((m17, n17) => {
            ViewStackProcessor.StartGetAccessRecordingFor(m17);
            Flex.create({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            Flex.borderRadius(16);
            Flex.borderWidth(1);
            Flex.borderColor('#e9e9e9');
            Flex.backgroundColor('#FFFFFF');
            if (!n17) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((k17, l17) => {
            ViewStackProcessor.StartGetAccessRecordingFor(k17);
            Flex.create({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            Flex.height(64);
            if (!l17) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((i17, j17) => {
            ViewStackProcessor.StartGetAccessRecordingFor(i17);
            Text.create({ "id": 33554465, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontSize(16);
            Text.fontColor('#000000');
            Text.fontFamily('HarmonyHeiTi');
            Text.fontWeight(FontWeight.Medium);
            Text.textOverflow({ overflow: TextOverflow.Ellipsis });
            Text.textAlign(TextAlign.Center);
            if (!j17) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Flex.pop();
        Flex.pop();
    }
    rerender() {
        this.updateDirtyElements();
    }
    public __resetStateVarsOnReuse__Internal(h17: Object): void {
    }
}
ViewStackProcessor.StartGetAccessRecordingFor(ViewStackProcessor.AllocateNewElmetIdForNextComponent());
loadDocument(new SwitchUserDialog(undefined, {}));
ViewStackProcessor.StopGetAccessRecording();
