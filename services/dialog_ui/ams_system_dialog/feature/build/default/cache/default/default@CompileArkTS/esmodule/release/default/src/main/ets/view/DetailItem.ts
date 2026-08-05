if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
interface DetailItem_Params {
    intentDetail?: IntentInfo | null;
}
import type { IntentInfo } from './IntentContent';
export class DetailItem extends ViewPU {
    constructor(u2, v2, w2, x2 = -1, y2 = undefined, z2) {
        super(u2, w2, x2, z2);
        if (typeof y2 === "function") {
            this.paramsGenerator_ = y2;
        }
        this.__intentDetail = new SynchedPropertyObjectOneWayPU(v2.intentDetail, this, "intentDetail");
        this.setInitiallyProvidedValue(v2);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(t2: DetailItem_Params) {
        if (t2.intentDetail === undefined) {
            this.__intentDetail.set(null);
        }
    }
    updateStateVars(s2: DetailItem_Params) {
        this.__intentDetail.reset(s2.intentDetail);
    }
    purgeVariableDependenciesOnElmtId(r2) {
        this.__intentDetail.purgeDependencyOnElmtId(r2);
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
    set intentDetail(q2: IntentInfo | null) {
        this.__intentDetail.set(q2);
    }
    initialRender() {
        this.observeComponentCreation((o2, p2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(o2);
            Column.create();
            if (!p2) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((m2, n2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(m2);
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
            if (!n2) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((k2, l2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(k2);
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
            if (!l2) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((i2, j2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(i2);
            Text.create({ "id": 16777229, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
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
            if (!h2) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((e2, f2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(e2);
            Text.create({ "id": 16777230, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
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
            if (!d2) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((a2, b2) => {
            ViewStackProcessor.StartGetAccessRecordingFor(a2);
            Text.create({ "id": 16777231, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.fontSize(16);
            Text.fontWeight(500);
            Text.fontColor({ "id": 16777249, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.fontFamily('HarmonyHeiTi-Medium');
            Text.lineHeight(22);
            Text.width('100%');
            Text.margin({ top: 10 });
            if (!b2) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((y1, z1) => {
            ViewStackProcessor.StartGetAccessRecordingFor(y1);
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
            if (!z1) {
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
