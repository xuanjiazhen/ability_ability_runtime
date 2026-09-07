if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
interface IntentContent_Params {
    articlePathStack?: NavPathStack;
    scroller?: Scroller;
    listMaterial?: IntentInfo[] | null;
    intermediateGroupMaterial?: Record<string, IntentInfo[]>;
    groupMaterial?: Record<string, IntentInfo[]>;
    context?;
    pixelMap?: image.PixelMap | ResourceStr | DrawableDescriptor | undefined;
    isCompActive?: boolean;
    createdPixelMaps?: image.PixelMap[];
}
import application from "@ohos:app.ability.application";
import insightIntentDriver from "@ohos:app.ability.insightIntentDriver";
import type common from "@ohos:app.ability.common";
import type { BusinessError } from "@ohos:base";
import hilog from "@ohos:hilog";
import image from "@ohos:multimedia.image";
import { jsClone } from "@bundle:com.ohos.amsdialog/default/ets/view/format";
import { TAG, DOMAIN } from "@bundle:com.ohos.amsdialog/default/ets/pages/Index";
import bundle from "@ohos:bundle.bundleManager";
export interface ItemInfo {
    title: string;
    brief: string;
}
export interface IntentInfo extends insightIntentDriver.InsightIntentInfo {
    bundleNameCN: string;
    iconImg: image.PixelMap | ResourceStr | DrawableDescriptor | undefined;
}
export interface IntentGrout {
    string: IntentInfo[];
}
interface MaterialResult {
    bundleName: string;
    materialItems: IntentInfo[];
}
export class IntentContent extends ViewPU {
    constructor(r9, s9, t9, u9 = -1, v9 = undefined, w9) {
        super(r9, t9, u9, w9);
        if (typeof v9 === "function") {
            this.paramsGenerator_ = v9;
        }
        this.__articlePathStack = this.initializeConsume('articlePathStack', "articlePathStack");
        this.scroller = new Scroller();
        this.__listMaterial = new ObservedPropertyObjectPU(null, this, "listMaterial");
        this.__intermediateGroupMaterial = new ObservedPropertyObjectPU({}, this, "intermediateGroupMaterial");
        this.__groupMaterial = new ObservedPropertyObjectPU({}, this, "groupMaterial");
        this.context = this.getUIContext().getHostContext();
        this.__pixelMap = new ObservedPropertyObjectPU(undefined, this, "pixelMap");
        this.isCompActive = true;
        this.createdPixelMaps = [];
        this.setInitiallyProvidedValue(s9);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(q9: IntentContent_Params) {
        if (q9.scroller !== undefined) {
            this.scroller = q9.scroller;
        }
        if (q9.listMaterial !== undefined) {
            this.listMaterial = q9.listMaterial;
        }
        if (q9.intermediateGroupMaterial !== undefined) {
            this.intermediateGroupMaterial = q9.intermediateGroupMaterial;
        }
        if (q9.groupMaterial !== undefined) {
            this.groupMaterial = q9.groupMaterial;
        }
        if (q9.context !== undefined) {
            this.context = q9.context;
        }
        if (q9.pixelMap !== undefined) {
            this.pixelMap = q9.pixelMap;
        }
        if (q9.isCompActive !== undefined) {
            this.isCompActive = q9.isCompActive;
        }
        if (q9.createdPixelMaps !== undefined) {
            this.createdPixelMaps = q9.createdPixelMaps;
        }
    }
    updateStateVars(p9: IntentContent_Params) {
    }
    purgeVariableDependenciesOnElmtId(o9) {
        this.__articlePathStack.purgeDependencyOnElmtId(o9);
        this.__listMaterial.purgeDependencyOnElmtId(o9);
        this.__intermediateGroupMaterial.purgeDependencyOnElmtId(o9);
        this.__groupMaterial.purgeDependencyOnElmtId(o9);
        this.__pixelMap.purgeDependencyOnElmtId(o9);
    }
    aboutToBeDeleted() {
        this.__articlePathStack.aboutToBeDeleted();
        this.__listMaterial.aboutToBeDeleted();
        this.__intermediateGroupMaterial.aboutToBeDeleted();
        this.__groupMaterial.aboutToBeDeleted();
        this.__pixelMap.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal();
    }
    private __articlePathStack: ObservedPropertyAbstractPU<NavPathStack>;
    get articlePathStack() {
        return this.__articlePathStack.get();
    }
    set articlePathStack(n9: NavPathStack) {
        this.__articlePathStack.set(n9);
    }
    private scroller: Scroller;
    private __listMaterial: ObservedPropertyObjectPU<IntentInfo[] | null>;
    get listMaterial() {
        return this.__listMaterial.get();
    }
    set listMaterial(m9: IntentInfo[] | null) {
        this.__listMaterial.set(m9);
    }
    private __intermediateGroupMaterial: ObservedPropertyObjectPU<Record<string, IntentInfo[]>>;
    get intermediateGroupMaterial() {
        return this.__intermediateGroupMaterial.get();
    }
    set intermediateGroupMaterial(l9: Record<string, IntentInfo[]>) {
        this.__intermediateGroupMaterial.set(l9);
    }
    private __groupMaterial: ObservedPropertyObjectPU<Record<string, IntentInfo[]>>;
    get groupMaterial() {
        return this.__groupMaterial.get();
    }
    set groupMaterial(k9: Record<string, IntentInfo[]>) {
        this.__groupMaterial.set(k9);
    }
    private context;
    private __pixelMap: ObservedPropertyObjectPU<image.PixelMap | ResourceStr | DrawableDescriptor | undefined>;
    get pixelMap() {
        return this.__pixelMap.get();
    }
    set pixelMap(j9: image.PixelMap | ResourceStr | DrawableDescriptor | undefined) {
        this.__pixelMap.set(j9);
    }
    private isCompActive: boolean;
    private createdPixelMaps: image.PixelMap[];
    aboutToAppear() {
        this.getInfos();
    }
    aboutToDisappear() {
        this.isCompActive = false;
        this.releasePixelMaps();
    }
    private async releasePixelMaps(): Promise<void> {
        for (const h9 of this.createdPixelMaps) {
            try {
                await h9.release();
            }
            catch (i9) {
                hilog.error(DOMAIN, TAG, 'release pixelMap failed: %{public}s', JSON.stringify(i9));
            }
        }
        this.createdPixelMaps = [];
    }
    private async trackPixelMap(f9: image.PixelMap | null | undefined): Promise<image.PixelMap | undefined> {
        if (!f9) {
            return undefined;
        }
        if (!this.isCompActive) {
            try {
                await f9.release();
            }
            catch (g9) {
                hilog.error(DOMAIN, TAG, 'release pixelMap failed: %{public}s', JSON.stringify(g9));
            }
            return undefined;
        }
        this.createdPixelMaps.push(f9);
        return f9;
    }
    onBackPress() {
        this.terminateSelf();
    }
    terminateSelf() {
        try {
            let e9 = this.getUIContext().getHostContext() as common.UIAbilityContext;
            if (e9) {
                e9.terminateSelf();
            }
        }
        catch (d9) {
            hilog.error(DOMAIN, TAG, 'Failed to context.terminateSelf. Cause: %{public}s', JSON.stringify(d9));
        }
    }
    async getInfos() {
        try {
            insightIntentDriver.getAllInsightIntentInfo(insightIntentDriver.GetInsightIntentFlag.GET_FULL_INSIGHT_INTENT)
                .then((i8) => {
                if (!this.isCompActive) {
                    hilog.info(DOMAIN, TAG, 'component destroyed, skip getInfos result');
                    return;
                }
                hilog.info(DOMAIN, TAG, `getAllInsightIntentInfo success: found ${i8.length} total intents`);
                this.listMaterial = i8 as IntentInfo[];
                this.intermediateGroupMaterial = this.listMaterial
                    .filter(c9 => c9.developType != insightIntentDriver.DevelopType.CONFIGURATION)
                    .reduce((z8, a9) => {
                    const b9: string = a9.bundleName;
                    if (!z8[b9]) {
                        z8[b9] = [];
                    }
                    z8[b9].push(a9);
                    return z8;
                }, {} as Record<string, IntentInfo[]>);
                const j8 = Object.keys(this.intermediateGroupMaterial);
                const k8 = j8.map(async (r8) => {
                    try {
                        const t8 = await bundle.getApplicationInfo(r8, bundle.ApplicationFlag.GET_APPLICATION_INFO_WITH_PERMISSION);
                        if (t8.debug) {
                            const u8 = this.intermediateGroupMaterial[r8];
                            for (const w8 of Object.keys(u8)) {
                                const x8 = await this.getAllIntentIcon(u8[w8].icon, r8);
                                if (!this.isCompActive) {
                                    return null;
                                }
                                u8[w8].iconImg = x8;
                                const y8 = await this.setAppName(u8[0]);
                                u8[w8].bundleNameCN = y8;
                            }
                            const v8: MaterialResult = { bundleName: r8, materialItems: u8 };
                            return v8;
                        }
                    }
                    catch (s8) {
                        hilog.error(DOMAIN, TAG, 'getApplicationInfo failed. Cause: %{public}s', JSON.stringify(s8));
                    }
                    return null;
                });
                Promise.all(k8).then((l8: Array<MaterialResult | null>) => {
                    if (!this.isCompActive) {
                        return;
                    }
                    l8.forEach((q8: MaterialResult | null) => {
                        if (q8 && q8.bundleName && q8.materialItems) {
                            this.groupMaterial[q8.bundleName] = q8.materialItems;
                        }
                    });
                    this.groupMaterial = jsClone(this.groupMaterial);
                    Object.keys(this.groupMaterial).forEach((m8: string) => {
                        const n8 = this.groupMaterial[m8];
                        hilog.info(DOMAIN, TAG, `Bundle: ${m8}, AppName: ${n8[0].bundleNameCN}, Intents: ${n8.length}`);
                        n8.forEach((o8: IntentInfo, p8: number) => {
                            hilog.info(DOMAIN, TAG, `  [${p8 + 1}] IntentName: ${o8.intentName}, Type: ${o8.intentType}, DisplayName: ${o8.displayName}, Module: ${o8.moduleName}`);
                        });
                    });
                });
            }).catch((h8: BusinessError) => {
                hilog.error(DOMAIN, TAG, 'getAllInsightIntentInfo errCode: %{public}d', h8.code);
                hilog.error(DOMAIN, TAG, 'getAllInsightIntentInfo errMessage: %{public}s', h8.message);
            });
        }
        catch (g8) {
            hilog.error(DOMAIN, TAG, 'getAllInsightIntentInfo error caught %{public}s', JSON.stringify(g8));
        }
    }
    async setAppName(z7: IntentInfo): Promise<string> {
        let a8: string = z7.bundleName;
        let b8: string = z7.moduleName;
        let c8 = this.getUIContext().getHostContext();
        let d8: string = '';
        try {
            const f8: Context = await application.createModuleContext(c8, a8, b8);
            d8 = f8.resourceManager.getStringSync(f8.applicationInfo.labelId);
        }
        catch (e8) {
            hilog.error(DOMAIN, TAG, 'createBundleContext failed. Cause: %{public}s', JSON.stringify(e8));
        }
        return d8;
    }
    getDisplayName(w7: string): string {
        const x7 = this.groupMaterial[w7];
        if (!x7 || x7.length === 0) {
            return w7;
        }
        const y7 = x7[0].bundleNameCN;
        return y7;
    }
    IntentBlock(w6: string, x6: IntentInfo[], y6 = null) {
        this.observeComponentCreation((u7, v7) => {
            ViewStackProcessor.StartGetAccessRecordingFor(u7);
            Column.create();
            Column.width('100%');
            Column.margin({ top: 10, bottom: 18 });
            Column.alignItems(HorizontalAlign.Center);
            if (!v7) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((s7, t7) => {
            ViewStackProcessor.StartGetAccessRecordingFor(s7);
            Text.create(w6);
            Text.fontFamily('HarmonyHeiTi-Medium');
            Text.fontSize(16);
            Text.fontWeight(500);
            Text.fontColor('#67686A');
            Text.margin({ left: 10, bottom: 8 });
            Text.textAlign(TextAlign.Start);
            Text.width('100%');
            if (!t7) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((q7, r7) => {
            ViewStackProcessor.StartGetAccessRecordingFor(q7);
            List.create({ space: 12 });
            List.backgroundColor({ "id": 16777240, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            List.borderRadius(16);
            List.padding({
                top: 4,
                left: 12,
                right: 12,
                bottom: 4
            });
            List.divider({
                strokeWidth: 0.5,
                startMargin: 38,
                endMargin: 0,
                color: { "id": 16777243, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" }
            });
            if (!r7) {
                List.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((z6, a7) => {
            ViewStackProcessor.StartGetAccessRecordingFor(z6);
            ForEach.create();
            const b7 = (e7, f7: number) => {
                const g7 = e7;
                {
                    const h7 = (o7, p7) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(o7);
                        i7(o7, p7);
                        if (!p7) {
                            ListItem.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    };
                    const i7 = (m7, n7) => {
                        ListItem.create(j7, true);
                        ListItem.onClick(() => {
                            this.articlePathStack.replacePath({ name: 'intentDetail', param: g7 });
                        });
                    };
                    const j7 = (k7, l7) => {
                        h7(k7, l7);
                        this.updateFuncByElmtId.set(k7, h7);
                        this.IntentBlockLine.bind(this)(g7);
                        ListItem.pop();
                    };
                    this.observeComponentCreation(h7);
                    ListItem.pop();
                }
            };
            this.forEachUpdateFunction(z6, x6, b7, (c7: IntentInfo, d7: number) => c7.intentName, true, true);
            if (!a7) {
                ForEach.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        ForEach.pop();
        List.pop();
        Column.pop();
    }
    async getAllIntentIcon(j6: string, k6: string): Promise<PixelMap | ResourceStr | DrawableDescriptor | undefined> {
        let l6: image.PixelMap | ResourceStr | DrawableDescriptor | undefined = undefined;
        if (j6?.startsWith('$r')) {
            try {
                let n6: string | undefined = j6.match(/['"](.+)['"]/)?.[1];
                if (!n6) {
                    return j6;
                }
                let o6: string | undefined = n6.split('.').pop();
                if (!o6) {
                    return j6;
                }
                let p6 = this.getUIContext().getHostContext();
                const q6: Promise<Context> = application.createBundleContext(p6, k6);
                q6.catch((v6: BusinessError) => {
                    hilog.error(DOMAIN, TAG, 'createBundleContext failed. Cause: %{public}s', JSON.stringify(v6));
                });
                const r6 = await q6;
                const s6: Promise<Uint8Array> = r6.resourceManager.getMediaByName(o6);
                s6.catch((u6: BusinessError) => {
                    hilog.error(DOMAIN, TAG, 'callback getMediaByName failed. Cause: %{public}s', JSON.stringify(u6));
                });
                const t6: Uint8Array = await s6;
                l6 = await this.trackPixelMap(await this.uint8ArrayToImage(t6));
            }
            catch (m6) {
                hilog.error(DOMAIN, TAG, 'Failed to getAllIntentIcon', JSON.stringify(m6));
                return l6;
            }
        }
        else {
            return '';
        }
        return l6;
    }
    async uint8ArrayToImage(f6: Uint8Array): Promise<PixelMap | null> {
        let g6: image.PixelMap | null = null;
        let h6: image.ImageSource | null = null;
        try {
            h6 = image.createImageSource(f6.buffer);
            if (!h6) {
                throw new Error('Failed to create ImageSource from buffer.');
            }
            g6 = await this.getUint8Array(h6);
            return g6;
        }
        catch (i6) {
            hilog.error(DOMAIN, TAG, 'Error in uint8ArrayToImage', JSON.stringify(i6));
        }
        finally {
            if (h6) {
                h6.release();
            }
        }
        return g6;
    }
    async getUint8Array(c6: image.ImageSource): Promise<PixelMap | null> {
        let d6: image.PixelMap | null = null;
        try {
            d6 = await c6.createPixelMap({
                desiredSize: { width: 24, height: 24 }
            });
            return d6;
        }
        catch (e6) {
            hilog.error(DOMAIN, TAG, 'Error in getUint8Array', JSON.stringify(e6));
        }
        return d6;
    }
    IntentBlockLine(m5: IntentInfo, n5 = null) {
        this.observeComponentCreation((a6, b6) => {
            ViewStackProcessor.StartGetAccessRecordingFor(a6);
            Row.create();
            Row.width('100%');
            Row.height(64);
            Row.alignItems(VerticalAlign.Center);
            if (!b6) {
                Row.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((y5, z5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(y5);
            Image.create(m5.iconImg ?? { "id": 16777253, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Image.width(24);
            Image.height(24);
            Image.borderRadius(6);
            Image.clip(true);
            Image.objectFit(ImageFit.Auto);
            Image.alt({ "id": 16777253, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            if (!z5) {
                Image.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((w5, x5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(w5);
            Column.create();
            Column.alignItems(HorizontalAlign.Start);
            Column.margin({ left: 16 });
            if (!x5) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((u5, v5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(u5);
            Text.create(m5.displayName);
            Text.fontFamily('HarmonyHeiTi-Medium');
            Text.fontSize(16);
            Text.fontColor('#494947');
            Text.fontWeight(500);
            Text.margin({ bottom: 6 });
            if (!v5) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((s5, t5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(s5);
            Text.create(m5.displayDescription);
            Text.fontFamily('HarmonyHeiTi');
            Text.fontSize(14);
            Text.fontWeight(400);
            Text.fontColor('#777775');
            if (!t5) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Column.pop();
        this.observeComponentCreation((q5, r5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(q5);
            Blank.create();
            if (!r5) {
                Blank.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Blank.pop();
        this.observeComponentCreation((o5, p5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(o5);
            Image.create({ "id": 16777254, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Image.width(12);
            Image.height(24);
            if (!p5) {
                Image.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Row.pop();
    }
    initialRender() {
        this.observeComponentCreation((k5, l5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(k5);
            Column.create();
            Column.width('100%');
            Column.height('100%');
            Column.expandSafeArea([SafeAreaType.SYSTEM], [SafeAreaEdge.BOTTOM]);
            Column.backgroundColor({ "id": 16777238, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            if (!l5) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((i5, j5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(i5);
            Row.create();
            Row.alignItems(VerticalAlign.Center);
            Row.width('100%');
            Row.height(56);
            Row.padding({
                top: 14.5,
                right: 16,
                left: 16,
                bottom: 14.5
            });
            Row.position({ top: 0, left: 0 });
            Row.zIndex(100);
            Row.backgroundColor({ "id": 16777238, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            if (!j5) {
                Row.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((g5, h5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(g5);
            Column.create();
            Column.width(40);
            Column.height(40);
            Column.borderRadius(20);
            Column.backgroundColor({ "id": 16777245, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Column.justifyContent(FlexAlign.Center);
            Column.onClick(() => {
                this.terminateSelf();
            });
            if (!h5) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((e5, f5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(e5);
            Image.create({ "id": 16777255, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Image.width(8.68);
            Image.height(16.48);
            Image.opacity(0.6);
            if (!f5) {
                Image.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Column.pop();
        this.observeComponentCreation((c5, d5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(c5);
            Text.create({ "id": 16777223, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.margin({ left: 8 });
            Text.fontSize(20);
            Text.fontWeight(700);
            Text.fontColor({ "id": 16777249, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            if (!d5) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Row.pop();
        this.observeComponentCreation((a5, b5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(a5);
            Scroll.create(this.scroller);
            Scroll.align(Alignment.TopStart);
            Scroll.padding({
                top: 76,
                right: 20,
                left: 20,
                bottom: 20
            });
            Scroll.constraintSize({ minHeight: '100%' });
            Scroll.align(Alignment.TopStart);
            Scroll.edgeEffect(EdgeEffect.Spring);
            Scroll.scrollable(ScrollDirection.Vertical);
            Scroll.scrollBar(BarState.Off);
            Scroll.width('100%');
            Scroll.height('100%');
            Scroll.backgroundColor({ "id": 16777238, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Scroll.clip(true);
            if (!b5) {
                Scroll.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((y4, z4) => {
            ViewStackProcessor.StartGetAccessRecordingFor(y4);
            Column.create();
            if (!z4) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((s4, t4) => {
            ViewStackProcessor.StartGetAccessRecordingFor(s4);
            ForEach.create();
            const u4 = w4 => {
                const x4 = w4;
                this.IntentBlock.bind(this)(this.getDisplayName(x4), this.groupMaterial[x4]);
            };
            this.forEachUpdateFunction(s4, Object.keys(ObservedObject.GetRawObject(this.groupMaterial)), u4, (v4: string) => v4, false, false);
            if (!t4) {
                ForEach.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        ForEach.pop();
        Column.pop();
        Scroll.pop();
        Column.pop();
    }
    rerender() {
        this.updateDirtyElements();
    }
}
