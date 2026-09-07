if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
interface SwitchUserDialog_Params {
    TAG?;
}
class SwitchUserDialog extends ViewPU {
    constructor(k17, l17, m17, n17 = -1, o17 = undefined, p17) {
        super(k17, m17, n17, p17);
        if (typeof o17 === "function") {
            this.paramsGenerator_ = o17;
        }
        this.TAG = 'SwitchUserDialog_Page';
        this.setInitiallyProvidedValue(l17);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(j17: SwitchUserDialog_Params) {
        if (j17.TAG !== undefined) {
            this.TAG = j17.TAG;
        }
    }
    updateStateVars(i17: SwitchUserDialog_Params) {
    }
    purgeVariableDependenciesOnElmtId(h17) {
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
        this.observeComponentCreation((f17, g17) => {
            ViewStackProcessor.StartGetAccessRecordingFor(f17);
            Flex.create({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            Flex.borderRadius(16);
            Flex.borderWidth(1);
            Flex.borderColor('#e9e9e9');
            Flex.backgroundColor('#FFFFFF');
            if (!g17) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((d17, e17) => {
            ViewStackProcessor.StartGetAccessRecordingFor(d17);
            Flex.create({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            Flex.height(64);
            if (!e17) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((b17, c17) => {
            ViewStackProcessor.StartGetAccessRecordingFor(b17);
            Text.create({ "id": 33554465, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontSize(16);
            Text.fontColor('#000000');
            Text.fontFamily('HarmonyHeiTi');
            Text.fontWeight(FontWeight.Medium);
            Text.textOverflow({ overflow: TextOverflow.Ellipsis });
            Text.textAlign(TextAlign.Center);
            if (!c17) {
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
}
ViewStackProcessor.StartGetAccessRecordingFor(ViewStackProcessor.AllocateNewElmetIdForNextComponent());
loadDocument(new SwitchUserDialog(undefined, {}));
ViewStackProcessor.StopGetAccessRecording();
