if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
interface JumpInterceptorDialog_Params {
    isDefaultSelector?: boolean;
    btn?: TipBtn;
    TAG?;
}
interface TipBtn {
    color: string;
}
class JumpInterceptorDialog extends ViewPU {
    constructor(r2, s2, t2, u2 = -1, v2 = undefined, w2) {
        super(r2, t2, u2, w2);
        if (typeof v2 === "function") {
            this.paramsGenerator_ = v2;
        }
        this.__isDefaultSelector = new ObservedPropertySimplePU(true, this, "isDefaultSelector");
        this.__btn = new ObservedPropertyObjectPU({ color: '#FFFFFF' }, this, "btn");
        this.TAG = 'JumpInterceptorDialog_Page';
        this.setInitiallyProvidedValue(s2);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(q2: JumpInterceptorDialog_Params) {
        if (q2.isDefaultSelector !== undefined) {
            this.isDefaultSelector = q2.isDefaultSelector;
        }
        if (q2.btn !== undefined) {
            this.btn = q2.btn;
        }
        if (q2.TAG !== undefined) {
            this.TAG = q2.TAG;
        }
    }
    updateStateVars(p2: JumpInterceptorDialog_Params) {
    }
    purgeVariableDependenciesOnElmtId(o2) {
        this.__isDefaultSelector.purgeDependencyOnElmtId(o2);
        this.__btn.purgeDependencyOnElmtId(o2);
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
    set isDefaultSelector(n2: boolean) {
        this.__isDefaultSelector.set(n2);
    }
    private __btn: ObservedPropertyObjectPU<TipBtn>;
    get btn() {
        return this.__btn.get();
    }
    set btn(m2: TipBtn) {
        this.__btn.set(m2);
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
        globalThis.jumpInterceptorExtensionContext.terminateSelf();
    }
    onJumpApp() {
        globalThis.abilityWant.bundleName = globalThis.params.bundleName;
        globalThis.abilityWant.abilityName = globalThis.params.abilityName;
        globalThis.abilityWant.moduleName = globalThis.params.moduleName;
        globalThis.jumpInterceptorExtensionContext.startAbilityAsCaller(globalThis.abilityWant, (k2, l2) => {
            if (l2) {
                console.error(this.TAG + 'startAbility finish, error: ' + JSON.stringify(l2));
                return;
            }
            console.log(this.TAG + 'startAbility finish, data: ' + JSON.stringify(k2));
            globalThis.jumpInterceptorExtensionContext.terminateSelf();
        });
    }
    initialRender() {
        this.observeComponentCreation((i2, j2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(i2);
            Flex.create({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            Flex.borderRadius(20);
            Flex.borderWidth(1);
            Flex.borderColor('#e9e9e9');
            Flex.backgroundColor('#FFFFFF');
            if (!j2) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((g2, h2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(g2);
            Flex.create({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            if (!h2) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((e2, f2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(e2);
            Text.create({ "id": 33554461, "type": 10003, params: [globalThis.callerAppName, globalThis.targetAppName], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontSize(22);
            Text.fontWeight(FontWeight.Medium);
            Text.height('29%');
            Text.textOverflow({ overflow: TextOverflow.Ellipsis });
            Text.maxLines(2);
            Text.textAlign(TextAlign.Center);
            if (!f2) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Flex.pop();
        this.observeComponentCreation((c2, d2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(c2);
            Flex.create({ justifyContent: FlexAlign.Center });
            Flex.margin({ top: 10 });
            if (!d2) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((a2, b2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(a2);
            Flex.create({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            Flex.width(175);
            Flex.height(50);
            Flex.borderRadius(28);
            Flex.backgroundColor(this.btn.color);
            Flex.onClick(() => {
                this.onCloseApp();
            });
            if (!b2) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((y1, z1) => {
            ViewStackProcessor.StartGetAccessRecordingFor(y1);
            Text.create({ "id": 33554450, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontSize('21fp');
            Text.fontColor('#0A59F7');
            Text.fontWeight(FontWeight.Regular);
            Text.textAlign(TextAlign.Center);
            if (!z1) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Flex.pop();
        this.observeComponentCreation((w1, x1) => {
            ViewStackProcessor.StartGetAccessRecordingFor(w1);
            Flex.create({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            Flex.width(175);
            Flex.height(50);
            Flex.borderRadius(28);
            Flex.backgroundColor(this.btn.color);
            Flex.onClick(() => {
                this.onJumpApp();
            });
            if (!x1) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((u1, v1) => {
            ViewStackProcessor.StartGetAccessRecordingFor(u1);
            Text.create({ "id": 33554454, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontSize('21fp');
            Text.fontColor('#0A59F7');
            Text.fontWeight(FontWeight.Regular);
            Text.textAlign(TextAlign.Center);
            if (!v1) {
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
loadDocument(new JumpInterceptorDialog(undefined, {}));
ViewStackProcessor.StopGetAccessRecording();
