if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
interface DetailItem_Params {
    intentDetail?: IntentInfo | null;
}
import type { IntentInfo } from './IntentContent';
export class DetailItem extends ViewPU {
    constructor(a3, b3, c3, d3 = -1, e3 = undefined, f3) {
        super(a3, c3, d3, f3);
        if (typeof e3 === "function") {
            this.paramsGenerator_ = e3;
        }
        this.__intentDetail = new SynchedPropertyObjectOneWayPU(b3.intentDetail, this, "intentDetail");
        this.setInitiallyProvidedValue(b3);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(z2: DetailItem_Params) {
        if (z2.intentDetail === undefined) {
            this.__intentDetail.set(null);
        }
    }
    updateStateVars(y2: DetailItem_Params) {
        this.__intentDetail.reset(y2.intentDetail);
    }
    purgeVariableDependenciesOnElmtId(x2) {
        this.__intentDetail.purgeDependencyOnElmtId(x2);
    }
    aboutToBeDeleted() {
        this.__intentDetail.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal();
    }
    private __intentDetail: SynchedPropertySimpleOneWayPU<IntentInfo | null>;
    get intentDetail() {
        return this.__intentDetail.get();
    }
    set intentDetail(w2: IntentInfo | null) {
        this.__intentDetail.set(w2);
    }
    initialRender() {
        this.observeComponentCreation((u2, v2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(u2);
            Column.create();
            if (!v2) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((s2, t2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(s2);
            Text.create({ "id": 16777227, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.fontFamily('HarmonyHeiTi-Medium');
            Text.fontSize(14);
            Text.fontWeight(500);
            Text.fontColor({ "id": 16777248, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.textAlign(TextAlign.Start);
            Text.lineHeight(20);
            Text.width('100%');
            Text.padding({
                top: 28,
                bottom: 8,
                left: 30,
                right: 30
            });
            if (!t2) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((q2, r2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(q2);
            Column.create();
            Column.padding({
                top: 3,
                right: 12,
                bottom: 12,
                left: 12
            });
            Column.margin({ left: 18, right: 18 });
            Column.alignItems(HorizontalAlign.Start);
            Column.backgroundColor({ "id": 16777241, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Column.borderRadius(20);
            if (!r2) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((o2, p2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(o2);
            Text.create({ "id": 16777229, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.fontSize(16);
            Text.fontWeight(500);
            Text.fontColor({ "id": 16777249, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.fontFamily('HarmonyHeiTi-Medium');
            Text.lineHeight(22);
            Text.width('100%');
            Text.margin({ top: 10 });
            if (!p2) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((m2, n2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(m2);
            Text.create(this.intentDetail?.bundleName);
            Text.fontSize(14);
            Text.fontWeight(400);
            Text.fontColor({ "id": 16777248, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.fontFamily('HarmonyHeiTi');
            Text.lineHeight(20);
            Text.maxLines(1);
            Text.constraintSize({ maxWidth: '100%' });
            Text.textOverflow({ overflow: TextOverflow.Ellipsis });
            Text.width('100%');
            Text.margin({ bottom: 10 });
            if (!n2) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((k2, l2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(k2);
            Text.create({ "id": 16777230, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.fontSize(16);
            Text.fontWeight(500);
            Text.fontColor({ "id": 16777249, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.fontFamily('HarmonyHeiTi-Medium');
            Text.lineHeight(22);
            Text.width('100%');
            Text.margin({ top: 10 });
            if (!l2) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((i2, j2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(i2);
            Text.create(this.intentDetail?.intentName);
            Text.fontSize(14);
            Text.fontWeight(400);
            Text.fontColor({ "id": 16777248, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.fontFamily('HarmonyHeiTi');
            Text.lineHeight(20);
            Text.maxLines(1);
            Text.constraintSize({ maxWidth: '100%' });
            Text.textOverflow({ overflow: TextOverflow.Ellipsis });
            Text.width('100%');
            Text.margin({ bottom: 10 });
            if (!j2) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((g2, h2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(g2);
            Text.create({ "id": 16777231, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.fontSize(16);
            Text.fontWeight(500);
            Text.fontColor({ "id": 16777249, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.fontFamily('HarmonyHeiTi-Medium');
            Text.lineHeight(22);
            Text.width('100%');
            Text.margin({ top: 10 });
            if (!h2) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((e2, f2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(e2);
            Text.create(this.intentDetail?.intentType);
            Text.fontSize(14);
            Text.fontWeight(400);
            Text.fontColor({ "id": 16777248, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.fontFamily('HarmonyHeiTi');
            Text.lineHeight(20);
            Text.maxLines(1);
            Text.constraintSize({ maxWidth: '100%' });
            Text.textOverflow({ overflow: TextOverflow.Ellipsis });
            Text.width('100%');
            Text.margin({ bottom: 10 });
            if (!f2) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Column.pop();
        Column.pop();
    }
    rerender() {
        this.updateDirtyElements();
    }
    public __resetStateVarsOnReuse__Internal(d2: Object): void {
    }
}
