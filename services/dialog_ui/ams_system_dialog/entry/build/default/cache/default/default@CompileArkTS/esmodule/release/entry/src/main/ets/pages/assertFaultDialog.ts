if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
interface AssertFaultDialog_Params {
    textDetail?: string;
}
import abilityManager from "@ohos:app.ability.abilityManager";
import type { BusinessError } from "@ohos:base";
import type UIExtensionContentSession from "@ohos:app.ability.UIExtensionContentSession";
let storage = LocalStorage.GetShared();
const TAG: string = 'AssertFaultDialog_Page';
const DEBUG_ASSERT_RESULT: string = 'assertResult';
class AssertFaultDialog extends ViewPU {
    constructor(o1, p1, q1, r1 = -1, s1 = undefined, t1) {
        super(o1, q1, r1, t1);
        if (typeof s1 === "function") {
            this.paramsGenerator_ = s1;
        }
        this.__textDetail = new ObservedPropertySimplePU('', this, "textDetail");
        this.setInitiallyProvidedValue(p1);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(n1: AssertFaultDialog_Params) {
        if (n1.textDetail !== undefined) {
            this.textDetail = n1.textDetail;
        }
    }
    updateStateVars(m1: AssertFaultDialog_Params) {
    }
    purgeVariableDependenciesOnElmtId(l1) {
        this.__textDetail.purgeDependencyOnElmtId(l1);
    }
    aboutToBeDeleted() {
        this.__textDetail.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal();
    }
    private __textDetail: ObservedPropertySimplePU<string>;
    get textDetail() {
        return this.__textDetail.get();
    }
    set textDetail(k1: string) {
        this.__textDetail.set(k1);
    }
    aboutToAppear() {
        console.info(TAG, 'dialog page appears');
        this.textDetail = storage.get<string>('textDetail');
    }
    existApp() {
        console.info(TAG, 'Exist app called');
        try {
            AppStorage.setOrCreate(DEBUG_ASSERT_RESULT, abilityManager.UserStatus.ASSERT_TERMINATE);
            storage.get<UIExtensionContentSession>('session').terminateSelf().then(() => {
                console.log(TAG, 'terminateSelf success.');
            }).catch((j1: BusinessError) => {
                console.error(TAG, `terminateSelf failed, error: ${JSON.stringify(j1)}`);
            });
        }
        catch (i1) {
            console.error(TAG, `try notifyDebugAssertResult failed, error: ${JSON.stringify(i1)}`);
        }
    }
    onContinueCall() {
        console.info(TAG, 'On continue called');
        try {
            AppStorage.setOrCreate(DEBUG_ASSERT_RESULT, abilityManager.UserStatus.ASSERT_CONTINUE);
            storage.get<UIExtensionContentSession>('session').terminateSelf().then(() => {
                console.log(TAG, 'terminateSelf success.');
            }).catch((h1: BusinessError) => {
                console.error(TAG, `terminateSelf failed, error: ${JSON.stringify(h1)}`);
            });
        }
        catch (g1) {
            console.error(TAG, `try notifyDebugAssertResult failed, error: ${JSON.stringify(g1)}`);
        }
    }
    onRetryCall() {
        console.info(TAG, 'On retry called');
        try {
            AppStorage.setOrCreate(DEBUG_ASSERT_RESULT, abilityManager.UserStatus.ASSERT_RETRY);
            storage.get<UIExtensionContentSession>('session').terminateSelf().then(() => {
                console.log(TAG, 'terminateSelf success.');
            }).catch((f1: BusinessError) => {
                console.error(TAG, `terminateSelf failed, error: ${JSON.stringify(f1)}`);
            });
        }
        catch (e1) {
            console.error(TAG, `try notifyDebugAssertResult failed, error: ${JSON.stringify(e1)}`);
        }
    }
    onTerminationCall() {
        console.info(TAG, 'On termination called');
        try {
            AppStorage.setOrCreate(DEBUG_ASSERT_RESULT, abilityManager.UserStatus.ASSERT_TERMINATE);
            storage.get<UIExtensionContentSession>('session').terminateSelf().then(() => {
                console.log(TAG, 'terminateSelf success.');
            }).catch((d1: BusinessError) => {
                console.error(TAG, `terminateSelf failed, error: ${JSON.stringify(d1)}`);
            });
        }
        catch (c1) {
            console.error(TAG, `try notifyDebugAssertResult failed, error: ${JSON.stringify(c1)}`);
        }
    }
    initialRender() {
        this.observeComponentCreation((a1, b1) => {
            ViewStackProcessor.StartGetAccessRecordingFor(a1);
            Column.create(AlertDialog.show({
                title: 'Assert Fault Dialog',
                message: this.textDetail,
                autoCancel: false,
                alignment: DialogAlignment.Center,
                gridCount: 4,
                buttonDirection: DialogButtonDirection.HORIZONTAL,
                cancel: this.existApp,
                buttons: [
                    {
                        enabled: true,
                        defaultFocus: false,
                        style: DialogButtonStyle.HIGHLIGHT,
                        value: { "id": 33554446, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" },
                        fontColor: { "id": 125829123, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" },
                        backgroundColor: { "id": 33554468, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" },
                        action: this.onContinueCall
                    },
                    {
                        enabled: true,
                        defaultFocus: false,
                        style: DialogButtonStyle.HIGHLIGHT,
                        value: { "id": 33554447, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" },
                        fontColor: { "id": 125829123, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" },
                        backgroundColor: { "id": 33554468, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" },
                        action: this.onRetryCall
                    },
                    {
                        enabled: true,
                        defaultFocus: true,
                        style: DialogButtonStyle.HIGHLIGHT,
                        value: { "id": 33554448, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" },
                        fontColor: { "id": 125829123, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" },
                        backgroundColor: { "id": 33554468, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" },
                        action: this.onTerminationCall
                    }
                ]
            }));
            if (!b1) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Column.pop();
    }
    rerender() {
        this.updateDirtyElements();
    }
}
if (storage && storage.routeName != undefined && storage.storage != undefined) {
    registerNamedRoute(() => new AssertFaultDialog(undefined, {}, storage.useSharedStorage ? LocalStorage.getShared() : storage.storage), storage.routeName, { bundleName: "com.ohos.amsdialog", moduleName: "entry", pagePath: "pages/assertFaultDialog", pageFullPath: "entry/src/main/ets/pages/assertFaultDialog", integratedHsp: "false", moduleType: "followWithHap" });
}
else if (storage && storage.routeName != undefined && storage.storage == undefined) {
    registerNamedRoute(() => new AssertFaultDialog(undefined, {}, storage.useSharedStorage ? LocalStorage.getShared() : storage.storage), storage.routeName, { bundleName: "com.ohos.amsdialog", moduleName: "entry", pagePath: "pages/assertFaultDialog", pageFullPath: "entry/src/main/ets/pages/assertFaultDialog", integratedHsp: "false", moduleType: "followWithHap" });
}
else if (storage && storage.routeName == undefined && storage.storage != undefined) {
    ViewStackProcessor.StartGetAccessRecordingFor(ViewStackProcessor.AllocateNewElmetIdForNextComponent());
    loadDocument(new AssertFaultDialog(undefined, {}, storage.useSharedStorage ? LocalStorage.getShared() : storage.storage));
    ViewStackProcessor.StopGetAccessRecording();
}
else if (storage && storage.useSharedStorage != undefined) {
    ViewStackProcessor.StartGetAccessRecordingFor(ViewStackProcessor.AllocateNewElmetIdForNextComponent());
    loadDocument(new AssertFaultDialog(undefined, {}, storage.useSharedStorage ? LocalStorage.getShared() : undefined));
    ViewStackProcessor.StopGetAccessRecording();
}
else {
    ViewStackProcessor.StartGetAccessRecordingFor(ViewStackProcessor.AllocateNewElmetIdForNextComponent());
    loadDocument(new AssertFaultDialog(undefined, {}, storage));
    ViewStackProcessor.StopGetAccessRecording();
}
