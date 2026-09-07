if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
interface DetailItem_Params {
    intentDetail?: IntentInfo | null;
}
import type { IntentInfo } from './IntentContent';
export class DetailItem extends ViewPU {
    constructor(y2, z2, a3, b3 = -1, c3 = undefined, d3) {
        super(y2, a3, b3, d3);
        if (typeof c3 === "function") {
            this.paramsGenerator_ = c3;
        }
        this.__intentDetail = new SynchedPropertyObjectOneWayPU(z2.intentDetail, this, "intentDetail");
        this.setInitiallyProvidedValue(z2);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(x2: DetailItem_Params) {
        if (x2.intentDetail === undefined) {
            this.__intentDetail.set(null);
        }
    }
    updateStateVars(w2: DetailItem_Params) {
        this.__intentDetail.reset(w2.intentDetail);
    }
    purgeVariableDependenciesOnElmtId(v2) {
        this.__intentDetail.purgeDependencyOnElmtId(v2);
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
    set intentDetail(u2: IntentInfo | null) {
        this.__intentDetail.set(u2);
    }
    initialRender() {
        this.observeComponentCreation((s2, t2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(s2);
            Column.create();
            if (!t2) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((q2, r2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(q2);
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
            if (!r2) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((o2, p2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(o2);
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
            if (!p2) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((m2, n2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(m2);
            Text.create({ "id": 16777229, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.fontSize(16);
            Text.fontWeight(500);
            Text.fontColor({ "id": 16777249, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.fontFamily('HarmonyHeiTi-Medium');
            Text.lineHeight(22);
            Text.width('100%');
            Text.margin({ top: 10 });
            if (!n2) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((k2, l2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(k2);
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
            if (!l2) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((i2, j2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(i2);
            Text.create({ "id": 16777230, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.fontSize(16);
            Text.fontWeight(500);
            Text.fontColor({ "id": 16777249, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.fontFamily('HarmonyHeiTi-Medium');
            Text.lineHeight(22);
            Text.width('100%');
            Text.margin({ top: 10 });
            if (!j2) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((g2, h2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(g2);
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
            if (!h2) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((e2, f2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(e2);
            Text.create({ "id": 16777231, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.fontSize(16);
            Text.fontWeight(500);
            Text.fontColor({ "id": 16777249, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.fontFamily('HarmonyHeiTi-Medium');
            Text.lineHeight(22);
            Text.width('100%');
            Text.margin({ top: 10 });
            if (!f2) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((c2, d2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(c2);
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
            if (!d2) {
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
}
