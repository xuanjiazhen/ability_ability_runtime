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
    constructor(j9, k9, l9, m9 = -1, n9 = undefined, o9) {
        super(j9, l9, m9, o9);
        if (typeof n9 === "function") {
            this.paramsGenerator_ = n9;
        }
        this.__articlePathStack = this.initializeConsume('articlePathStack', "articlePathStack");
        this.scroller = new Scroller();
        this.__listMaterial = new ObservedPropertyObjectPU(null, this, "listMaterial");
        this.__intermediateGroupMaterial = new ObservedPropertyObjectPU({}, this, "intermediateGroupMaterial");
        this.__groupMaterial = new ObservedPropertyObjectPU({}, this, "groupMaterial");
        this.context = this.getUIContext().getHostContext();
        this.__pixelMap = new ObservedPropertyObjectPU(undefined, this, "pixelMap");
        this.setInitiallyProvidedValue(k9);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(i9: IntentContent_Params) {
        if (i9.scroller !== undefined) {
            this.scroller = i9.scroller;
        }
        if (i9.listMaterial !== undefined) {
            this.listMaterial = i9.listMaterial;
        }
        if (i9.intermediateGroupMaterial !== undefined) {
            this.intermediateGroupMaterial = i9.intermediateGroupMaterial;
        }
        if (i9.groupMaterial !== undefined) {
            this.groupMaterial = i9.groupMaterial;
        }
        if (i9.context !== undefined) {
            this.context = i9.context;
        }
        if (i9.pixelMap !== undefined) {
            this.pixelMap = i9.pixelMap;
        }
    }
    updateStateVars(h9: IntentContent_Params) {
    }
    purgeVariableDependenciesOnElmtId(g9) {
        this.__articlePathStack.purgeDependencyOnElmtId(g9);
        this.__listMaterial.purgeDependencyOnElmtId(g9);
        this.__intermediateGroupMaterial.purgeDependencyOnElmtId(g9);
        this.__groupMaterial.purgeDependencyOnElmtId(g9);
        this.__pixelMap.purgeDependencyOnElmtId(g9);
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
    set articlePathStack(f9: NavPathStack) {
        this.__articlePathStack.set(f9);
    }
    private scroller: Scroller;
    private __listMaterial: ObservedPropertyObjectPU<IntentInfo[] | null>;
    get listMaterial() {
        return this.__listMaterial.get();
    }
    set listMaterial(e9: IntentInfo[] | null) {
        this.__listMaterial.set(e9);
    }
    private __intermediateGroupMaterial: ObservedPropertyObjectPU<Record<string, IntentInfo[]>>;
    get intermediateGroupMaterial() {
        return this.__intermediateGroupMaterial.get();
    }
    set intermediateGroupMaterial(d9: Record<string, IntentInfo[]>) {
        this.__intermediateGroupMaterial.set(d9);
    }
    private __groupMaterial: ObservedPropertyObjectPU<Record<string, IntentInfo[]>>;
    get groupMaterial() {
        return this.__groupMaterial.get();
    }
    set groupMaterial(c9: Record<string, IntentInfo[]>) {
        this.__groupMaterial.set(c9);
    }
    private context;
    private __pixelMap: ObservedPropertyObjectPU<image.PixelMap | ResourceStr | DrawableDescriptor | undefined>;
    get pixelMap() {
        return this.__pixelMap.get();
    }
    set pixelMap(b9: image.PixelMap | ResourceStr | DrawableDescriptor | undefined) {
        this.__pixelMap.set(b9);
    }
    aboutToAppear() {
        this.getInfos();
    }
    onBackPress() {
        this.terminateSelf();
    }
    terminateSelf() {
        try {
            let a9 = this.getUIContext().getHostContext() as common.UIAbilityContext;
            a9.terminateSelf();
        }
        catch (z8) {
            hilog.error(DOMAIN, TAG, 'Failed to context.terminateSelf. Cause: %{public}s', JSON.stringify(z8));
        }
    }
    async getInfos() {
        try {
            insightIntentDriver.getAllInsightIntentInfo(insightIntentDriver.GetInsightIntentFlag.GET_FULL_INSIGHT_INTENT)
                .then((e8) => {
                hilog.info(DOMAIN, TAG, `getAllInsightIntentInfo success: found ${e8.length} total intents`);
                this.listMaterial = e8 as IntentInfo[];
                this.intermediateGroupMaterial = this.listMaterial
                    .filter(y8 => y8.developType != insightIntentDriver.DevelopType.CONFIGURATION)
                    .reduce((v8, w8) => {
                    const x8: string = w8.bundleName;
                    if (!v8[x8]) {
                        v8[x8] = [];
                    }
                    v8[x8].push(w8);
                    return v8;
                }, {} as Record<string, IntentInfo[]>);
                const f8 = Object.keys(this.intermediateGroupMaterial);
                const g8 = f8.map(async (n8) => {
                    try {
                        const p8 = await bundle.getApplicationInfo(n8, bundle.ApplicationFlag.GET_APPLICATION_INFO_WITH_PERMISSION);
                        if (p8.debug) {
                            const q8 = this.intermediateGroupMaterial[n8];
                            for (const s8 of Object.keys(q8)) {
                                const t8 = await this.getAllIntentIcon(q8[s8].icon, n8);
                                q8[s8].iconImg = t8;
                                const u8 = await this.setAppName(q8[0]);
                                q8[s8].bundleNameCN = u8;
                            }
                            const r8: MaterialResult = { bundleName: n8, materialItems: q8 };
                            return r8;
                        }
                    }
                    catch (o8) {
                        hilog.error(DOMAIN, TAG, 'getApplicationInfo failed. Cause: %{public}s', JSON.stringify(o8));
                    }
                    return null;
                });
                Promise.all(g8).then((h8: Array<MaterialResult | null>) => {
                    h8.forEach((m8: MaterialResult | null) => {
                        if (m8 && m8.bundleName && m8.materialItems) {
                            this.groupMaterial[m8.bundleName] = m8.materialItems;
                        }
                    });
                    this.groupMaterial = jsClone(this.groupMaterial);
                    Object.keys(this.groupMaterial).forEach((i8: string) => {
                        const j8 = this.groupMaterial[i8];
                        hilog.info(DOMAIN, TAG, `Bundle: ${i8}, AppName: ${j8[0].bundleNameCN}, Intents: ${j8.length}`);
                        j8.forEach((k8: IntentInfo, l8: number) => {
                            hilog.info(DOMAIN, TAG, `  [${l8 + 1}] IntentName: ${k8.intentName}, Type: ${k8.intentType}, DisplayName: ${k8.displayName}, Module: ${k8.moduleName}`);
                        });
                    });
                });
            }).catch((d8: BusinessError) => {
                hilog.error(DOMAIN, TAG, 'getAllInsightIntentInfo errCode: %{public}d', d8.code);
                hilog.error(DOMAIN, TAG, 'getAllInsightIntentInfo errMessage: %{public}s', d8.message);
            });
        }
        catch (c8) {
            hilog.error(DOMAIN, TAG, 'getAllInsightIntentInfo error caught %{public}s', JSON.stringify(c8));
        }
    }
    async setAppName(v7: IntentInfo): Promise<string> {
        let w7: string = v7.bundleName;
        let x7: string = v7.moduleName;
        let y7 = this.getUIContext().getHostContext();
        let z7: string = '';
        try {
            const b8: Context = await application.createModuleContext(y7, w7, x7);
            z7 = b8.resourceManager.getStringSync(b8.applicationInfo.labelId);
        }
        catch (a8) {
            hilog.error(DOMAIN, TAG, 'createBundleContext failed. Cause: %{public}s', JSON.stringify(a8));
        }
        return z7;
    }
    getDisplayName(s7: string): string {
        const t7 = this.groupMaterial[s7];
        if (!t7 || t7.length === 0) {
            return s7;
        }
        const u7 = t7[0].bundleNameCN;
        return u7;
    }
    IntentBlock(s6: string, t6: IntentInfo[], u6 = null) {
        this.observeComponentCreation((q7, r7) => {
            ViewStackProcessor.StartGetAccessRecordingFor(q7);
            Column.create();
            Column.width('100%');
            Column.margin({ top: 10, bottom: 18 });
            Column.alignItems(HorizontalAlign.Center);
            if (!r7) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((o7, p7) => {
            ViewStackProcessor.StartGetAccessRecordingFor(o7);
            Text.create(s6);
            Text.fontFamily('HarmonyHeiTi-Medium');
            Text.fontSize(16);
            Text.fontWeight(500);
            Text.fontColor('#67686A');
            Text.margin({ left: 10, bottom: 8 });
            Text.textAlign(TextAlign.Start);
            Text.width('100%');
            if (!p7) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((m7, n7) => {
            ViewStackProcessor.StartGetAccessRecordingFor(m7);
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
            if (!n7) {
                List.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((v6, w6) => {
            ViewStackProcessor.StartGetAccessRecordingFor(v6);
            ForEach.create();
            const x6 = (a7, b7: number) => {
                const c7 = a7;
                {
                    const d7 = (k7, l7) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(k7);
                        e7(k7, l7);
                        if (!l7) {
                            ListItem.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    };
                    const e7 = (i7, j7) => {
                        ListItem.create(f7, true);
                        ListItem.onClick(() => {
                            this.articlePathStack.replacePath({ name: 'intentDetail', param: c7 });
                        });
                    };
                    const f7 = (g7, h7) => {
                        d7(g7, h7);
                        this.updateFuncByElmtId.set(g7, d7);
                        this.IntentBlockLine.bind(this)(c7);
                        ListItem.pop();
                    };
                    this.observeComponentCreation(d7);
                    ListItem.pop();
                }
            };
            this.forEachUpdateFunction(v6, t6, x6, (y6: IntentInfo, z6: number) => y6.intentName, true, true);
            if (!w6) {
                ForEach.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        ForEach.pop();
        List.pop();
        Column.pop();
    }
    async getAllIntentIcon(f6: string, g6: string): Promise<PixelMap | ResourceStr | DrawableDescriptor | undefined> {
        let h6: image.PixelMap | ResourceStr | DrawableDescriptor | undefined = undefined;
        if (f6?.startsWith('$r')) {
            try {
                let j6: string | undefined = f6.match(/['"](.+)['"]/)?.[1];
                if (!j6) {
                    return f6;
                }
                let k6: string | undefined = j6.split('.').pop();
                if (!k6) {
                    return f6;
                }
                let l6 = this.getUIContext().getHostContext();
                const m6: Promise<Context> = application.createBundleContext(l6, g6);
                m6.catch((r6: BusinessError) => {
                    hilog.error(DOMAIN, TAG, 'createBundleContext failed. Cause: %{public}s', JSON.stringify(r6));
                });
                const n6 = await m6;
                const o6: Promise<Uint8Array> = n6.resourceManager.getMediaByName(k6);
                o6.catch((q6: BusinessError) => {
                    hilog.error(DOMAIN, TAG, 'callback getMediaByName failed. Cause: %{public}s', JSON.stringify(q6));
                });
                const p6: Uint8Array = await o6;
                h6 = await this.uint8ArrayToImage(p6) ?? undefined;
            }
            catch (i6) {
                hilog.error(DOMAIN, TAG, 'Failed to getAllIntentIcon', JSON.stringify(i6));
                return h6;
            }
        }
        else {
            return '';
        }
        return h6;
    }
    async uint8ArrayToImage(b6: Uint8Array): Promise<PixelMap | null> {
        let c6: image.PixelMap | null = null;
        let d6: image.ImageSource | null = null;
        try {
            d6 = image.createImageSource(b6.buffer);
            if (!d6) {
                throw new Error('Failed to create ImageSource from buffer.');
            }
            c6 = await this.getUint8Array(d6);
            return c6;
        }
        catch (e6) {
            hilog.error(DOMAIN, TAG, 'Error in uint8ArrayToImage', JSON.stringify(e6));
        }
        finally {
            if (d6) {
                d6.release();
            }
        }
        return c6;
    }
    async getUint8Array(y5: image.ImageSource): Promise<PixelMap | null> {
        let z5: image.PixelMap | null = null;
        try {
            z5 = await y5.createPixelMap({
                desiredSize: { width: 24, height: 24 }
            });
            return z5;
        }
        catch (a6) {
            hilog.error(DOMAIN, TAG, 'Error in getUint8Array', JSON.stringify(a6));
        }
        return z5;
    }
    IntentBlockLine(i5: IntentInfo, j5 = null) {
        this.observeComponentCreation((w5, x5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(w5);
            Row.create();
            Row.width('100%');
            Row.height(64);
            Row.alignItems(VerticalAlign.Center);
            if (!x5) {
                Row.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((u5, v5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(u5);
            Image.create(i5.iconImg ?? { "id": 16777253, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Image.width(24);
            Image.height(24);
            Image.borderRadius(6);
            Image.clip(true);
            Image.objectFit(ImageFit.Auto);
            Image.alt({ "id": 16777253, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            if (!v5) {
                Image.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((s5, t5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(s5);
            Column.create();
            Column.alignItems(HorizontalAlign.Start);
            Column.margin({ left: 16 });
            if (!t5) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((q5, r5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(q5);
            Text.create(i5.displayName);
            Text.fontFamily('HarmonyHeiTi-Medium');
            Text.fontSize(16);
            Text.fontColor('#494947');
            Text.fontWeight(500);
            Text.margin({ bottom: 6 });
            if (!r5) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((o5, p5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(o5);
            Text.create(i5.displayDescription);
            Text.fontFamily('HarmonyHeiTi');
            Text.fontSize(14);
            Text.fontWeight(400);
            Text.fontColor('#777775');
            if (!p5) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Column.pop();
        this.observeComponentCreation((m5, n5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(m5);
            Blank.create();
            if (!n5) {
                Blank.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Blank.pop();
        this.observeComponentCreation((k5, l5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(k5);
            Image.create({ "id": 16777254, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Image.width(12);
            Image.height(24);
            if (!l5) {
                Image.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Row.pop();
    }
    initialRender() {
        this.observeComponentCreation((g5, h5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(g5);
            Column.create();
            Column.width('100%');
            Column.height('100%');
            Column.expandSafeArea([SafeAreaType.SYSTEM], [SafeAreaEdge.BOTTOM]);
            Column.backgroundColor({ "id": 16777238, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            if (!h5) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((e5, f5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(e5);
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
            if (!f5) {
                Row.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((c5, d5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(c5);
            Column.create();
            Column.width(40);
            Column.height(40);
            Column.borderRadius(20);
            Column.backgroundColor({ "id": 16777245, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Column.justifyContent(FlexAlign.Center);
            Column.onClick(() => {
                this.terminateSelf();
            });
            if (!d5) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((a5, b5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(a5);
            Image.create({ "id": 16777255, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Image.width(8.68);
            Image.height(16.48);
            Image.opacity(0.6);
            if (!b5) {
                Image.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Column.pop();
        this.observeComponentCreation((y4, z4) => {
            ViewStackProcessor.StartGetAccessRecordingFor(y4);
            Text.create({ "id": 16777223, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.margin({ left: 8 });
            Text.fontSize(20);
            Text.fontWeight(700);
            Text.fontColor({ "id": 16777249, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            if (!z4) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Row.pop();
        this.observeComponentCreation((w4, x4) => {
            ViewStackProcessor.StartGetAccessRecordingFor(w4);
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
            if (!x4) {
                Scroll.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((u4, v4) => {
            ViewStackProcessor.StartGetAccessRecordingFor(u4);
            Column.create();
            if (!v4) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((o4, p4) => {
            ViewStackProcessor.StartGetAccessRecordingFor(o4);
            ForEach.create();
            const q4 = s4 => {
                const t4 = s4;
                this.IntentBlock.bind(this)(this.getDisplayName(t4), this.groupMaterial[t4]);
            };
            this.forEachUpdateFunction(o4, Object.keys(ObservedObject.GetRawObject(this.groupMaterial)), q4, (r4: string) => r4, false, false);
            if (!p4) {
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
