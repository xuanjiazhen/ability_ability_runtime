if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
interface ContentDisplayComponent_Params {
    currentMode?: number;
    targetInfoArray?: Array<TargetInfo>;
    currentLang?: string;
    targetInfoArraySplit?: Array<Array<TargetInfo>>;
    panelHeight?: number;
    realHeight?: Dimension | PanelHeight;
    isOverHeight?: boolean;
    scrollerHeight?: number;
    storage?: LocalStorage;
    scroller?: Scroller;
    titleHeight?: number;
    singleRowHeight?: number;
    maxLengthOneLine?: number;
    isChange?: boolean;
}
import ConfigurationConstant from "@ohos:app.ability.ConfigurationConstant";
import { TargetInfo } from "@bundle:com.ohos.amsdialog/entry/ets/data/Model";
import { SingleRowComponent } from "@bundle:com.ohos.amsdialog/entry/ets/view/SingleRowComponent";
import { TitleComponent } from "@bundle:com.ohos.amsdialog/entry/ets/view/TitileComponent";
const TAG = 'SelectorExtensionAbility.ContentDisplayComponent';
export class ContentDisplayComponent extends ViewPU {
    constructor(n27, o27, p27, q27 = -1, r27 = undefined, s27) {
        super(n27, p27, q27, s27);
        if (typeof r27 === "function") {
            this.paramsGenerator_ = r27;
        }
        this.__currentMode = this.createStorageProp('currentColorMode', ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT, "currentMode");
        this.__targetInfoArray = new SynchedPropertyObjectTwoWayPU(o27.targetInfoArray, this, "targetInfoArray");
        this.__currentLang = this.createStorageProp('currentLanguage', '', "currentLang");
        this.__targetInfoArraySplit = new ObservedPropertyObjectPU([], this, "targetInfoArraySplit");
        this.__panelHeight = new SynchedPropertySimpleOneWayPU(o27.panelHeight, this, "panelHeight");
        this.__realHeight = this.initializeConsume("realHeight", "realHeight");
        this.__isOverHeight = new ObservedPropertySimplePU(false, this, "isOverHeight");
        this.__scrollerHeight = new ObservedPropertySimplePU(0, this, "scrollerHeight");
        this.storage = undefined;
        this.scroller = new Scroller();
        this.__titleHeight = new ObservedPropertySimplePU(0, this, "titleHeight");
        this.addProvidedVar("titleHeight", this.__titleHeight, false);
        this.__singleRowHeight = new ObservedPropertySimplePU(0, this, "singleRowHeight");
        this.addProvidedVar("singleRowHeight", this.__singleRowHeight, false);
        this.__maxLengthOneLine = new SynchedPropertySimpleOneWayPU(o27.maxLengthOneLine, this, "maxLengthOneLine");
        this.__isChange = new ObservedPropertySimplePU(false, this, "isChange");
        this.addProvidedVar("isChange", this.__isChange, false);
        this.setInitiallyProvidedValue(o27);
        this.declareWatch("currentMode", this.onColorModeChange);
        this.declareWatch("targetInfoArray", this.onTargetInfoChanged);
        this.declareWatch("panelHeight", this.onTargetInfoChanged);
        this.declareWatch("maxLengthOneLine", this.onTargetInfoChanged);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(m27: ContentDisplayComponent_Params) {
        if (m27.targetInfoArraySplit !== undefined) {
            this.targetInfoArraySplit = m27.targetInfoArraySplit;
        }
        if (m27.isOverHeight !== undefined) {
            this.isOverHeight = m27.isOverHeight;
        }
        if (m27.scrollerHeight !== undefined) {
            this.scrollerHeight = m27.scrollerHeight;
        }
        if (m27.storage !== undefined) {
            this.storage = m27.storage;
        }
        if (m27.scroller !== undefined) {
            this.scroller = m27.scroller;
        }
        if (m27.titleHeight !== undefined) {
            this.titleHeight = m27.titleHeight;
        }
        if (m27.singleRowHeight !== undefined) {
            this.singleRowHeight = m27.singleRowHeight;
        }
        if (m27.isChange !== undefined) {
            this.isChange = m27.isChange;
        }
    }
    updateStateVars(l27: ContentDisplayComponent_Params) {
        this.__panelHeight.reset(l27.panelHeight);
        this.__maxLengthOneLine.reset(l27.maxLengthOneLine);
    }
    purgeVariableDependenciesOnElmtId(k27) {
        this.__currentMode.purgeDependencyOnElmtId(k27);
        this.__targetInfoArray.purgeDependencyOnElmtId(k27);
        this.__currentLang.purgeDependencyOnElmtId(k27);
        this.__targetInfoArraySplit.purgeDependencyOnElmtId(k27);
        this.__panelHeight.purgeDependencyOnElmtId(k27);
        this.__realHeight.purgeDependencyOnElmtId(k27);
        this.__isOverHeight.purgeDependencyOnElmtId(k27);
        this.__scrollerHeight.purgeDependencyOnElmtId(k27);
        this.__titleHeight.purgeDependencyOnElmtId(k27);
        this.__singleRowHeight.purgeDependencyOnElmtId(k27);
        this.__maxLengthOneLine.purgeDependencyOnElmtId(k27);
        this.__isChange.purgeDependencyOnElmtId(k27);
    }
    aboutToBeDeleted() {
        this.__currentMode.aboutToBeDeleted();
        this.__targetInfoArray.aboutToBeDeleted();
        this.__currentLang.aboutToBeDeleted();
        this.__targetInfoArraySplit.aboutToBeDeleted();
        this.__panelHeight.aboutToBeDeleted();
        this.__realHeight.aboutToBeDeleted();
        this.__isOverHeight.aboutToBeDeleted();
        this.__scrollerHeight.aboutToBeDeleted();
        this.__titleHeight.aboutToBeDeleted();
        this.__singleRowHeight.aboutToBeDeleted();
        this.__maxLengthOneLine.aboutToBeDeleted();
        this.__isChange.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal();
    }
    private __currentMode: ObservedPropertyAbstractPU<number>;
    get currentMode() {
        return this.__currentMode.get();
    }
    set currentMode(j27: number) {
        this.__currentMode.set(j27);
    }
    private __targetInfoArray: SynchedPropertySimpleOneWayPU<Array<TargetInfo>>;
    get targetInfoArray() {
        return this.__targetInfoArray.get();
    }
    set targetInfoArray(i27: Array<TargetInfo>) {
        this.__targetInfoArray.set(i27);
    }
    private __currentLang: ObservedPropertyAbstractPU<string>;
    get currentLang() {
        return this.__currentLang.get();
    }
    set currentLang(h27: string) {
        this.__currentLang.set(h27);
    }
    private __targetInfoArraySplit: ObservedPropertyObjectPU<Array<Array<TargetInfo>>>;
    get targetInfoArraySplit() {
        return this.__targetInfoArraySplit.get();
    }
    set targetInfoArraySplit(g27: Array<Array<TargetInfo>>) {
        this.__targetInfoArraySplit.set(g27);
    }
    private __panelHeight: SynchedPropertySimpleOneWayPU<number>;
    get panelHeight() {
        return this.__panelHeight.get();
    }
    set panelHeight(f27: number) {
        this.__panelHeight.set(f27);
    }
    private __realHeight: ObservedPropertyAbstractPU<Dimension | PanelHeight>;
    get realHeight() {
        return this.__realHeight.get();
    }
    set realHeight(e27: Dimension | PanelHeight) {
        this.__realHeight.set(e27);
    }
    private __isOverHeight: ObservedPropertySimplePU<boolean>;
    get isOverHeight() {
        return this.__isOverHeight.get();
    }
    set isOverHeight(d27: boolean) {
        this.__isOverHeight.set(d27);
    }
    private __scrollerHeight: ObservedPropertySimplePU<number>;
    get scrollerHeight() {
        return this.__scrollerHeight.get();
    }
    set scrollerHeight(c27: number) {
        this.__scrollerHeight.set(c27);
    }
    private storage?: LocalStorage;
    private scroller: Scroller;
    private __titleHeight: ObservedPropertySimplePU<number>;
    get titleHeight() {
        return this.__titleHeight.get();
    }
    set titleHeight(b27: number) {
        this.__titleHeight.set(b27);
    }
    private __singleRowHeight: ObservedPropertySimplePU<number>;
    get singleRowHeight() {
        return this.__singleRowHeight.get();
    }
    set singleRowHeight(a27: number) {
        this.__singleRowHeight.set(a27);
    }
    private __maxLengthOneLine: SynchedPropertySimpleOneWayPU<number>;
    get maxLengthOneLine() {
        return this.__maxLengthOneLine.get();
    }
    set maxLengthOneLine(z26: number) {
        this.__maxLengthOneLine.set(z26);
    }
    private __isChange: ObservedPropertySimplePU<boolean>;
    get isChange() {
        return this.__isChange.get();
    }
    set isChange(y26: boolean) {
        this.__isChange.set(y26);
    }
    onColorModeChange() {
        let x26 = this.currentMode === ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT ? false : true;
    }
    aboutToAppear() {
        const w26: Context = getContext(this);
        this.titleHeight = 56;
        this.getTargetsToDisplay();
    }
    onTargetInfoChanged() {
        this.isChange = true;
        this.getTargetsToDisplay();
    }
    getTargetsToDisplay() {
        const v26: TargetInfo[][] = this.splitArray(this.targetInfoArray);
        this.targetInfoArraySplit = this.fillUpTheGapsForOneLine(v26);
        if (this.isChange) {
            this.scrollerHeight = (this.panelHeight - this.titleHeight < this
                .singleRowHeight * this.targetInfoArraySplit.length ? this.panelHeight - this.titleHeight
                : this.singleRowHeight * this.targetInfoArraySplit.length) + 16;
            this.realHeight = this.titleHeight + this.scrollerHeight;
        }
    }
    splitArray(q26: Array<TargetInfo>): Array<Array<TargetInfo>> {
        let r26: TargetInfo[][] = [];
        if (this.maxLengthOneLine === 0) {
            r26.push(q26);
            return r26;
        }
        let s26 = Math.floor(q26.length / this.maxLengthOneLine);
        if (s26 === 0) {
            r26.push(q26);
            return r26;
        }
        let t26 = 0;
        for (t26 = 0; t26 < s26; t26++) {
            r26.push(q26.slice(t26 * this.maxLengthOneLine, (t26 + 1) * this.maxLengthOneLine));
        }
        let u26 = q26.length % this.maxLengthOneLine;
        if (u26 !== 0) {
            r26.push(q26.slice(t26 * this.maxLengthOneLine, t26 * this.maxLengthOneLine + u26));
        }
        return r26;
    }
    fillUpTheGapsForOneLine(m26: Array<Array<TargetInfo>>): Array<Array<TargetInfo>> {
        const n26: Array<TargetInfo> | undefined = m26.pop();
        if (n26 === undefined) {
            return m26;
        }
        const o26 = n26.length;
        if (o26 === this.maxLengthOneLine) {
            m26.push(n26);
            return m26;
        }
        for (let p26 = 0; p26 < this.maxLengthOneLine - o26; p26++) {
            n26.push(new TargetInfo(null, null, false));
        }
        m26.push(n26);
        return m26;
    }
    initialRender() {
        this.observeComponentCreation((i26, j26) => {
            ViewStackProcessor.StartGetAccessRecordingFor(i26);
            Column.create();
            Column.alignItems(HorizontalAlign.Start);
            Column.onAreaChange((k26: Area, l26: Area) => {
                if (k26.height !== l26.height) {
                    this.scrollerHeight =
                        (this.panelHeight - this.titleHeight <
                            this.singleRowHeight * this.targetInfoArraySplit.length ? this.panelHeight -
                            this.titleHeight : this.singleRowHeight * this.targetInfoArraySplit.length) + 16;
                    this.realHeight = this.titleHeight + this.scrollerHeight;
                }
            });
            if (!j26) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((g26, h26) => {
            ViewStackProcessor.StartGetAccessRecordingFor(g26);
            Row.create();
            Row.justifyContent(FlexAlign.Start);
            Row.width('100%');
            Row.padding({
                left: 16,
                right: 16
            });
            if (!h26) {
                Row.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        {
            this.observeComponentCreation((c26, d26) => {
                ViewStackProcessor.StartGetAccessRecordingFor(c26);
                if (d26) {
                    let e26 = new TitleComponent(this, { storage: this.storage }, undefined, c26, () => { }, { page: "entry/src/main/ets/view/ContentDisplayComponent.ets", line: 110, col: 17 });
                    ViewPU.create(e26);
                    let f26 = () => {
                        return {
                            storage: this.storage
                        };
                    };
                    e26.paramsGenerator_ = f26;
                }
                else {
                    this.updateStateVarsOfChildByElmtId(c26, {});
                }
                ViewStackProcessor.StopGetAccessRecording();
            });
        }
        Row.pop();
        this.observeComponentCreation((a26, b26) => {
            ViewStackProcessor.StartGetAccessRecordingFor(a26);
            Scroll.create(this.scroller);
            Scroll.constraintSize({ maxHeight: this.panelHeight - this.titleHeight });
            Scroll.width('100%');
            Scroll.scrollable(ScrollDirection.Vertical);
            Scroll.scrollBar(BarState.On);
            Scroll.edgeEffect(EdgeEffect.None);
            if (!b26) {
                Scroll.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((y25, z25) => {
            ViewStackProcessor.StartGetAccessRecordingFor(y25);
            Column.create();
            Column.padding({
                left: 8,
                right: 8,
                top: 16,
                bottom: 8
            });
            if (!z25) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((o25, p25) => {
            ViewStackProcessor.StartGetAccessRecordingFor(o25);
            ForEach.create();
            const q25 = (r25, s25?: number) => {
                const t25 = r25;
                {
                    this.observeComponentCreation((u25, v25) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(u25);
                        if (v25) {
                            let w25 = new SingleRowComponent(this, { targetInfoArray: t25, storage: this.storage }, undefined, u25, () => { }, { page: "entry/src/main/ets/view/ContentDisplayComponent.ets", line: 122, col: 25 });
                            ViewPU.create(w25);
                            let x25 = () => {
                                return {
                                    targetInfoArray: t25,
                                    storage: this.storage
                                };
                            };
                            w25.paramsGenerator_ = x25;
                        }
                        else {
                            this.updateStateVarsOfChildByElmtId(u25, {});
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                }
            };
            this.forEachUpdateFunction(o25, this.targetInfoArraySplit, q25, undefined, true, false);
            if (!p25) {
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
    public __resetStateVarsOnReuse__Internal(n25: Object): void {
        this.__currentMode.resetSource();
        this.__currentLang.resetSource();
        this.__reInitializeConsume__Internal("realHeight", "realHeight");
        this.__targetInfoArray.resetSource(n25.__targetInfoArray__Link__Internal);
    }
}
