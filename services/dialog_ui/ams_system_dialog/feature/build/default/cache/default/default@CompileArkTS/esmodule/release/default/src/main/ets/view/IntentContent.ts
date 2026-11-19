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
    constructor(u9, v9, w9, x9 = -1, y9 = undefined, z9) {
        super(u9, w9, x9, z9);
        if (typeof y9 === "function") {
            this.paramsGenerator_ = y9;
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
        this.setInitiallyProvidedValue(v9);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(t9: IntentContent_Params) {
        if (t9.scroller !== undefined) {
            this.scroller = t9.scroller;
        }
        if (t9.listMaterial !== undefined) {
            this.listMaterial = t9.listMaterial;
        }
        if (t9.intermediateGroupMaterial !== undefined) {
            this.intermediateGroupMaterial = t9.intermediateGroupMaterial;
        }
        if (t9.groupMaterial !== undefined) {
            this.groupMaterial = t9.groupMaterial;
        }
        if (t9.context !== undefined) {
            this.context = t9.context;
        }
        if (t9.pixelMap !== undefined) {
            this.pixelMap = t9.pixelMap;
        }
        if (t9.isCompActive !== undefined) {
            this.isCompActive = t9.isCompActive;
        }
        if (t9.createdPixelMaps !== undefined) {
            this.createdPixelMaps = t9.createdPixelMaps;
        }
    }
    updateStateVars(s9: IntentContent_Params) {
    }
    purgeVariableDependenciesOnElmtId(r9) {
        this.__articlePathStack.purgeDependencyOnElmtId(r9);
        this.__listMaterial.purgeDependencyOnElmtId(r9);
        this.__intermediateGroupMaterial.purgeDependencyOnElmtId(r9);
        this.__groupMaterial.purgeDependencyOnElmtId(r9);
        this.__pixelMap.purgeDependencyOnElmtId(r9);
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
    set articlePathStack(q9: NavPathStack) {
        this.__articlePathStack.set(q9);
    }
    private scroller: Scroller;
    private __listMaterial: ObservedPropertyObjectPU<IntentInfo[] | null>;
    get listMaterial() {
        return this.__listMaterial.get();
    }
    set listMaterial(p9: IntentInfo[] | null) {
        this.__listMaterial.set(p9);
    }
    private __intermediateGroupMaterial: ObservedPropertyObjectPU<Record<string, IntentInfo[]>>;
    get intermediateGroupMaterial() {
        return this.__intermediateGroupMaterial.get();
    }
    set intermediateGroupMaterial(o9: Record<string, IntentInfo[]>) {
        this.__intermediateGroupMaterial.set(o9);
    }
    private __groupMaterial: ObservedPropertyObjectPU<Record<string, IntentInfo[]>>;
    get groupMaterial() {
        return this.__groupMaterial.get();
    }
    set groupMaterial(n9: Record<string, IntentInfo[]>) {
        this.__groupMaterial.set(n9);
    }
    private context;
    private __pixelMap: ObservedPropertyObjectPU<image.PixelMap | ResourceStr | DrawableDescriptor | undefined>;
    get pixelMap() {
        return this.__pixelMap.get();
    }
    set pixelMap(m9: image.PixelMap | ResourceStr | DrawableDescriptor | undefined) {
        this.__pixelMap.set(m9);
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
        for (const k9 of this.createdPixelMaps) {
            try {
                await k9.release();
            }
            catch (l9) {
                hilog.error(DOMAIN, TAG, 'release pixelMap failed: %{public}s', JSON.stringify(l9));
            }
        }
        this.createdPixelMaps = [];
    }
    private async trackPixelMap(i9: image.PixelMap | null | undefined): Promise<image.PixelMap | undefined> {
        if (!i9) {
            return undefined;
        }
        if (!this.isCompActive) {
            try {
                await i9.release();
            }
            catch (j9) {
                hilog.error(DOMAIN, TAG, 'release pixelMap failed: %{public}s', JSON.stringify(j9));
            }
            return undefined;
        }
        this.createdPixelMaps.push(i9);
        return i9;
    }
    onBackPress() {
        this.terminateSelf();
    }
    terminateSelf() {
        try {
            let h9 = this.getUIContext().getHostContext() as common.UIAbilityContext;
            if (h9) {
                h9.terminateSelf();
            }
        }
        catch (g9) {
            hilog.error(DOMAIN, TAG, 'Failed to context.terminateSelf. Cause: %{public}s', JSON.stringify(g9));
        }
    }
    async getInfos() {
        try {
            insightIntentDriver.getAllInsightIntentInfo(insightIntentDriver.GetInsightIntentFlag.GET_FULL_INSIGHT_INTENT)
                .then((l8) => {
                if (!this.isCompActive) {
                    hilog.info(DOMAIN, TAG, 'component destroyed, skip getInfos result');
                    return;
                }
                hilog.info(DOMAIN, TAG, `getAllInsightIntentInfo success: found ${l8.length} total intents`);
                this.listMaterial = l8 as IntentInfo[];
                this.intermediateGroupMaterial = this.listMaterial
                    .filter(f9 => f9.developType != insightIntentDriver.DevelopType.CONFIGURATION)
                    .reduce((c9, d9) => {
                    const e9: string = d9.bundleName;
                    if (!c9[e9]) {
                        c9[e9] = [];
                    }
                    c9[e9].push(d9);
                    return c9;
                }, {} as Record<string, IntentInfo[]>);
                const m8 = Object.keys(this.intermediateGroupMaterial);
                const n8 = m8.map(async (u8) => {
                    try {
                        const w8 = await bundle.getApplicationInfo(u8, bundle.ApplicationFlag.GET_APPLICATION_INFO_WITH_PERMISSION);
                        if (w8.debug) {
                            const x8 = this.intermediateGroupMaterial[u8];
                            for (const z8 of Object.keys(x8)) {
                                const a9 = await this.getAllIntentIcon(x8[z8].icon, u8);
                                if (!this.isCompActive) {
                                    return null;
                                }
                                x8[z8].iconImg = a9;
                                const b9 = await this.setAppName(x8[0]);
                                x8[z8].bundleNameCN = b9;
                            }
                            const y8: MaterialResult = { bundleName: u8, materialItems: x8 };
                            return y8;
                        }
                    }
                    catch (v8) {
                        hilog.error(DOMAIN, TAG, 'getApplicationInfo failed. Cause: %{public}s', JSON.stringify(v8));
                    }
                    return null;
                });
                Promise.all(n8).then((o8: Array<MaterialResult | null>) => {
                    if (!this.isCompActive) {
                        return;
                    }
                    o8.forEach((t8: MaterialResult | null) => {
                        if (t8 && t8.bundleName && t8.materialItems) {
                            this.groupMaterial[t8.bundleName] = t8.materialItems;
                        }
                    });
                    this.groupMaterial = jsClone(this.groupMaterial);
                    Object.keys(this.groupMaterial).forEach((p8: string) => {
                        const q8 = this.groupMaterial[p8];
                        hilog.info(DOMAIN, TAG, `Bundle: ${p8}, AppName: ${q8[0].bundleNameCN}, Intents: ${q8.length}`);
                        q8.forEach((r8: IntentInfo, s8: number) => {
                            hilog.info(DOMAIN, TAG, `  [${s8 + 1}] IntentName: ${r8.intentName}, Type: ${r8.intentType}, DisplayName: ${r8.displayName}, Module: ${r8.moduleName}`);
                        });
                    });
                });
            }).catch((k8: BusinessError) => {
                hilog.error(DOMAIN, TAG, 'getAllInsightIntentInfo errCode: %{public}d', k8.code);
                hilog.error(DOMAIN, TAG, 'getAllInsightIntentInfo errMessage: %{public}s', k8.message);
            });
        }
        catch (j8) {
            hilog.error(DOMAIN, TAG, 'getAllInsightIntentInfo error caught %{public}s', JSON.stringify(j8));
        }
    }
    async setAppName(c8: IntentInfo): Promise<string> {
        let d8: string = c8.bundleName;
        let e8: string = c8.moduleName;
        let f8 = this.getUIContext().getHostContext();
        let g8: string = '';
        try {
            const i8: Context = await application.createModuleContext(f8, d8, e8);
            g8 = i8.resourceManager.getStringSync(i8.applicationInfo.labelId);
        }
        catch (h8) {
            hilog.error(DOMAIN, TAG, 'createBundleContext failed. Cause: %{public}s', JSON.stringify(h8));
        }
        return g8;
    }
    getDisplayName(z7: string): string {
        const a8 = this.groupMaterial[z7];
        if (!a8 || a8.length === 0) {
            return z7;
        }
        const b8 = a8[0].bundleNameCN;
        return b8;
    }
    IntentBlock(z6: string, a7: IntentInfo[], b7 = null) {
        this.observeComponentCreation((x7, y7) => {
            ViewStackProcessor.StartGetAccessRecordingFor(x7);
            Column.create();
            Column.width('100%');
            Column.margin({ top: 10, bottom: 18 });
            Column.alignItems(HorizontalAlign.Center);
            if (!y7) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((v7, w7) => {
            ViewStackProcessor.StartGetAccessRecordingFor(v7);
            Text.create(z6);
            Text.fontFamily('HarmonyHeiTi-Medium');
            Text.fontSize(16);
            Text.fontWeight(500);
            Text.fontColor('#67686A');
            Text.margin({ left: 10, bottom: 8 });
            Text.textAlign(TextAlign.Start);
            Text.width('100%');
            if (!w7) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((t7, u7) => {
            ViewStackProcessor.StartGetAccessRecordingFor(t7);
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
            if (!u7) {
                List.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((c7, d7) => {
            ViewStackProcessor.StartGetAccessRecordingFor(c7);
            ForEach.create();
            const e7 = (h7, i7: number) => {
                const j7 = h7;
                {
                    const k7 = (r7, s7) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(r7);
                        l7(r7, s7);
                        if (!s7) {
                            ListItem.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    };
                    const l7 = (p7, q7) => {
                        ListItem.create(m7, true);
                        ListItem.onClick(() => {
                            this.articlePathStack.replacePath({ name: 'intentDetail', param: j7 });
                        });
                    };
                    const m7 = (n7, o7) => {
                        k7(n7, o7);
                        this.updateFuncByElmtId.set(n7, k7);
                        this.IntentBlockLine.bind(this)(j7);
                        ListItem.pop();
                    };
                    this.observeComponentCreation(k7);
                    ListItem.pop();
                }
            };
            this.forEachUpdateFunction(c7, a7, e7, (f7: IntentInfo, g7: number) => f7.intentName, true, true);
            if (!d7) {
                ForEach.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        ForEach.pop();
        List.pop();
        Column.pop();
    }
    async getAllIntentIcon(m6: string, n6: string): Promise<PixelMap | ResourceStr | DrawableDescriptor | undefined> {
        let o6: image.PixelMap | ResourceStr | DrawableDescriptor | undefined = undefined;
        if (m6?.startsWith('$r')) {
            try {
                let q6: string | undefined = m6.match(/['"](.+)['"]/)?.[1];
                if (!q6) {
                    return m6;
                }
                let r6: string | undefined = q6.split('.').pop();
                if (!r6) {
                    return m6;
                }
                let s6 = this.getUIContext().getHostContext();
                const t6: Promise<Context> = application.createBundleContext(s6, n6);
                t6.catch((y6: BusinessError) => {
                    hilog.error(DOMAIN, TAG, 'createBundleContext failed. Cause: %{public}s', JSON.stringify(y6));
                });
                const u6 = await t6;
                const v6: Promise<Uint8Array> = u6.resourceManager.getMediaByName(r6);
                v6.catch((x6: BusinessError) => {
                    hilog.error(DOMAIN, TAG, 'callback getMediaByName failed. Cause: %{public}s', JSON.stringify(x6));
                });
                const w6: Uint8Array = await v6;
                o6 = await this.trackPixelMap(await this.uint8ArrayToImage(w6));
            }
            catch (p6) {
                hilog.error(DOMAIN, TAG, 'Failed to getAllIntentIcon', JSON.stringify(p6));
                return o6;
            }
        }
        else {
            return '';
        }
        return o6;
    }
    async uint8ArrayToImage(i6: Uint8Array): Promise<PixelMap | null> {
        let j6: image.PixelMap | null = null;
        let k6: image.ImageSource | null = null;
        try {
            k6 = image.createImageSource(i6.buffer);
            if (!k6) {
                throw new Error('Failed to create ImageSource from buffer.');
            }
            j6 = await this.getUint8Array(k6);
            return j6;
        }
        catch (l6) {
            hilog.error(DOMAIN, TAG, 'Error in uint8ArrayToImage', JSON.stringify(l6));
        }
        finally {
            if (k6) {
                k6.release();
            }
        }
        return j6;
    }
    async getUint8Array(f6: image.ImageSource): Promise<PixelMap | null> {
        let g6: image.PixelMap | null = null;
        try {
            g6 = await f6.createPixelMap({
                desiredSize: { width: 24, height: 24 }
            });
            return g6;
        }
        catch (h6) {
            hilog.error(DOMAIN, TAG, 'Error in getUint8Array', JSON.stringify(h6));
        }
        return g6;
    }
    IntentBlockLine(p5: IntentInfo, q5 = null) {
        this.observeComponentCreation((d6, e6) => {
            ViewStackProcessor.StartGetAccessRecordingFor(d6);
            Row.create();
            Row.width('100%');
            Row.height(64);
            Row.alignItems(VerticalAlign.Center);
            if (!e6) {
                Row.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((b6, c6) => {
            ViewStackProcessor.StartGetAccessRecordingFor(b6);
            Image.create(p5.iconImg ?? { "id": 16777253, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Image.width(24);
            Image.height(24);
            Image.borderRadius(6);
            Image.clip(true);
            Image.objectFit(ImageFit.Auto);
            Image.alt({ "id": 16777253, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            if (!c6) {
                Image.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((z5, a6) => {
            ViewStackProcessor.StartGetAccessRecordingFor(z5);
            Column.create();
            Column.alignItems(HorizontalAlign.Start);
            Column.margin({ left: 16 });
            if (!a6) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((x5, y5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(x5);
            Text.create(p5.displayName);
            Text.fontFamily('HarmonyHeiTi-Medium');
            Text.fontSize(16);
            Text.fontColor('#494947');
            Text.fontWeight(500);
            Text.margin({ bottom: 6 });
            if (!y5) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((v5, w5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(v5);
            Text.create(p5.displayDescription);
            Text.fontFamily('HarmonyHeiTi');
            Text.fontSize(14);
            Text.fontWeight(400);
            Text.fontColor('#777775');
            if (!w5) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Column.pop();
        this.observeComponentCreation((t5, u5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(t5);
            Blank.create();
            if (!u5) {
                Blank.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Blank.pop();
        this.observeComponentCreation((r5, s5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(r5);
            Image.create({ "id": 16777254, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Image.width(12);
            Image.height(24);
            if (!s5) {
                Image.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Row.pop();
    }
    initialRender() {
        this.observeComponentCreation((n5, o5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(n5);
            Column.create();
            Column.width('100%');
            Column.height('100%');
            Column.expandSafeArea([SafeAreaType.SYSTEM], [SafeAreaEdge.BOTTOM]);
            Column.backgroundColor({ "id": 16777238, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            if (!o5) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((l5, m5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(l5);
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
            if (!m5) {
                Row.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((j5, k5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(j5);
            Column.create();
            Column.width(40);
            Column.height(40);
            Column.borderRadius(20);
            Column.backgroundColor({ "id": 16777245, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Column.justifyContent(FlexAlign.Center);
            Column.onClick(() => {
                this.terminateSelf();
            });
            if (!k5) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((h5, i5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(h5);
            Image.create({ "id": 16777255, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Image.width(8.68);
            Image.height(16.48);
            Image.opacity(0.6);
            if (!i5) {
                Image.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Column.pop();
        this.observeComponentCreation((f5, g5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(f5);
            Text.create({ "id": 16777223, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.margin({ left: 8 });
            Text.fontSize(20);
            Text.fontWeight(700);
            Text.fontColor({ "id": 16777249, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            if (!g5) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Row.pop();
        this.observeComponentCreation((d5, e5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(d5);
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
            if (!e5) {
                Scroll.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((b5, c5) => {
            ViewStackProcessor.StartGetAccessRecordingFor(b5);
            Column.create();
            if (!c5) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((v4, w4) => {
            ViewStackProcessor.StartGetAccessRecordingFor(v4);
            ForEach.create();
            const x4 = z4 => {
                const a5 = z4;
                this.IntentBlock.bind(this)(this.getDisplayName(a5), this.groupMaterial[a5]);
            };
            this.forEachUpdateFunction(v4, Object.keys(ObservedObject.GetRawObject(this.groupMaterial)), x4, (y4: string) => y4, false, false);
            if (!w4) {
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
    public __resetStateVarsOnReuse__Internal(u4: Object): void {
        this.__reInitializeConsume__Internal('articlePathStack', "articlePathStack");
    }
}
