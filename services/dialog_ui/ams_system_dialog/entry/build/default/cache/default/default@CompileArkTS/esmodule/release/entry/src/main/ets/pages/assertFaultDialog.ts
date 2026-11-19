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
    constructor(p1, q1, r1, s1 = -1, t1 = undefined, u1) {
        super(p1, r1, s1, u1);
        if (typeof t1 === "function") {
            this.paramsGenerator_ = t1;
        }
        this.__textDetail = new ObservedPropertySimplePU('', this, "textDetail");
        this.setInitiallyProvidedValue(q1);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(o1: AssertFaultDialog_Params) {
        if (o1.textDetail !== undefined) {
            this.textDetail = o1.textDetail;
        }
    }
    updateStateVars(n1: AssertFaultDialog_Params) {
    }
    purgeVariableDependenciesOnElmtId(m1) {
        this.__textDetail.purgeDependencyOnElmtId(m1);
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
    set textDetail(l1: string) {
        this.__textDetail.set(l1);
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
            }).catch((k1: BusinessError) => {
                console.error(TAG, `terminateSelf failed, error: ${JSON.stringify(k1)}`);
            });
        }
        catch (j1) {
            console.error(TAG, `try notifyDebugAssertResult failed, error: ${JSON.stringify(j1)}`);
        }
    }
    onContinueCall() {
        console.info(TAG, 'On continue called');
        try {
            AppStorage.setOrCreate(DEBUG_ASSERT_RESULT, abilityManager.UserStatus.ASSERT_CONTINUE);
            storage.get<UIExtensionContentSession>('session').terminateSelf().then(() => {
                console.log(TAG, 'terminateSelf success.');
            }).catch((i1: BusinessError) => {
                console.error(TAG, `terminateSelf failed, error: ${JSON.stringify(i1)}`);
            });
        }
        catch (h1) {
            console.error(TAG, `try notifyDebugAssertResult failed, error: ${JSON.stringify(h1)}`);
        }
    }
    onRetryCall() {
        console.info(TAG, 'On retry called');
        try {
            AppStorage.setOrCreate(DEBUG_ASSERT_RESULT, abilityManager.UserStatus.ASSERT_RETRY);
            storage.get<UIExtensionContentSession>('session').terminateSelf().then(() => {
                console.log(TAG, 'terminateSelf success.');
            }).catch((g1: BusinessError) => {
                console.error(TAG, `terminateSelf failed, error: ${JSON.stringify(g1)}`);
            });
        }
        catch (f1) {
            console.error(TAG, `try notifyDebugAssertResult failed, error: ${JSON.stringify(f1)}`);
        }
    }
    onTerminationCall() {
        console.info(TAG, 'On termination called');
        try {
            AppStorage.setOrCreate(DEBUG_ASSERT_RESULT, abilityManager.UserStatus.ASSERT_TERMINATE);
            storage.get<UIExtensionContentSession>('session').terminateSelf().then(() => {
                console.log(TAG, 'terminateSelf success.');
            }).catch((e1: BusinessError) => {
                console.error(TAG, `terminateSelf failed, error: ${JSON.stringify(e1)}`);
            });
        }
        catch (d1) {
            console.error(TAG, `try notifyDebugAssertResult failed, error: ${JSON.stringify(d1)}`);
        }
    }
    initialRender() {
        this.observeComponentCreation((b1, c1) => {
            ViewStackProcessor.StartGetAccessRecordingFor(b1);
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
            if (!c1) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Column.pop();
    }
    rerender() {
        this.updateDirtyElements();
    }
    public __resetStateVarsOnReuse__Internal(a1: Object): void {
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
