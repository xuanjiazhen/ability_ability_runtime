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
    constructor(e27, f27, g27, h27 = -1, i27 = undefined, j27) {
        super(e27, g27, h27, j27);
        if (typeof i27 === "function") {
            this.paramsGenerator_ = i27;
        }
        this.__currentMode = this.createStorageProp('currentColorMode', ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT, "currentMode");
        this.__targetInfoArray = new SynchedPropertyObjectTwoWayPU(f27.targetInfoArray, this, "targetInfoArray");
        this.__currentLang = this.createStorageProp('currentLanguage', '', "currentLang");
        this.__targetInfoArraySplit = new ObservedPropertyObjectPU([], this, "targetInfoArraySplit");
        this.__panelHeight = new SynchedPropertySimpleOneWayPU(f27.panelHeight, this, "panelHeight");
        this.__realHeight = this.initializeConsume("realHeight", "realHeight");
        this.__isOverHeight = new ObservedPropertySimplePU(false, this, "isOverHeight");
        this.__scrollerHeight = new ObservedPropertySimplePU(0, this, "scrollerHeight");
        this.storage = undefined;
        this.scroller = new Scroller();
        this.__titleHeight = new ObservedPropertySimplePU(0, this, "titleHeight");
        this.addProvidedVar("titleHeight", this.__titleHeight, false);
        this.__singleRowHeight = new ObservedPropertySimplePU(0, this, "singleRowHeight");
        this.addProvidedVar("singleRowHeight", this.__singleRowHeight, false);
        this.__maxLengthOneLine = new SynchedPropertySimpleOneWayPU(f27.maxLengthOneLine, this, "maxLengthOneLine");
        this.__isChange = new ObservedPropertySimplePU(false, this, "isChange");
        this.addProvidedVar("isChange", this.__isChange, false);
        this.setInitiallyProvidedValue(f27);
        this.declareWatch("currentMode", this.onColorModeChange);
        this.declareWatch("targetInfoArray", this.onTargetInfoChanged);
        this.declareWatch("panelHeight", this.onTargetInfoChanged);
        this.declareWatch("maxLengthOneLine", this.onTargetInfoChanged);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(d27: ContentDisplayComponent_Params) {
        if (d27.targetInfoArraySplit !== undefined) {
            this.targetInfoArraySplit = d27.targetInfoArraySplit;
        }
        if (d27.isOverHeight !== undefined) {
            this.isOverHeight = d27.isOverHeight;
        }
        if (d27.scrollerHeight !== undefined) {
            this.scrollerHeight = d27.scrollerHeight;
        }
        if (d27.storage !== undefined) {
            this.storage = d27.storage;
        }
        if (d27.scroller !== undefined) {
            this.scroller = d27.scroller;
        }
        if (d27.titleHeight !== undefined) {
            this.titleHeight = d27.titleHeight;
        }
        if (d27.singleRowHeight !== undefined) {
            this.singleRowHeight = d27.singleRowHeight;
        }
        if (d27.isChange !== undefined) {
            this.isChange = d27.isChange;
        }
    }
    updateStateVars(c27: ContentDisplayComponent_Params) {
        this.__panelHeight.reset(c27.panelHeight);
        this.__maxLengthOneLine.reset(c27.maxLengthOneLine);
    }
    purgeVariableDependenciesOnElmtId(b27) {
        this.__currentMode.purgeDependencyOnElmtId(b27);
        this.__targetInfoArray.purgeDependencyOnElmtId(b27);
        this.__currentLang.purgeDependencyOnElmtId(b27);
        this.__targetInfoArraySplit.purgeDependencyOnElmtId(b27);
        this.__panelHeight.purgeDependencyOnElmtId(b27);
        this.__realHeight.purgeDependencyOnElmtId(b27);
        this.__isOverHeight.purgeDependencyOnElmtId(b27);
        this.__scrollerHeight.purgeDependencyOnElmtId(b27);
        this.__titleHeight.purgeDependencyOnElmtId(b27);
        this.__singleRowHeight.purgeDependencyOnElmtId(b27);
        this.__maxLengthOneLine.purgeDependencyOnElmtId(b27);
        this.__isChange.purgeDependencyOnElmtId(b27);
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
    set currentMode(a27: number) {
        this.__currentMode.set(a27);
    }
    private __targetInfoArray: SynchedPropertySimpleOneWayPU<Array<TargetInfo>>;
    get targetInfoArray() {
        return this.__targetInfoArray.get();
    }
    set targetInfoArray(z26: Array<TargetInfo>) {
        this.__targetInfoArray.set(z26);
    }
    private __currentLang: ObservedPropertyAbstractPU<string>;
    get currentLang() {
        return this.__currentLang.get();
    }
    set currentLang(y26: string) {
        this.__currentLang.set(y26);
    }
    private __targetInfoArraySplit: ObservedPropertyObjectPU<Array<Array<TargetInfo>>>;
    get targetInfoArraySplit() {
        return this.__targetInfoArraySplit.get();
    }
    set targetInfoArraySplit(x26: Array<Array<TargetInfo>>) {
        this.__targetInfoArraySplit.set(x26);
    }
    private __panelHeight: SynchedPropertySimpleOneWayPU<number>;
    get panelHeight() {
        return this.__panelHeight.get();
    }
    set panelHeight(w26: number) {
        this.__panelHeight.set(w26);
    }
    private __realHeight: ObservedPropertyAbstractPU<Dimension | PanelHeight>;
    get realHeight() {
        return this.__realHeight.get();
    }
    set realHeight(v26: Dimension | PanelHeight) {
        this.__realHeight.set(v26);
    }
    private __isOverHeight: ObservedPropertySimplePU<boolean>;
    get isOverHeight() {
        return this.__isOverHeight.get();
    }
    set isOverHeight(u26: boolean) {
        this.__isOverHeight.set(u26);
    }
    private __scrollerHeight: ObservedPropertySimplePU<number>;
    get scrollerHeight() {
        return this.__scrollerHeight.get();
    }
    set scrollerHeight(t26: number) {
        this.__scrollerHeight.set(t26);
    }
    private storage?: LocalStorage;
    private scroller: Scroller;
    private __titleHeight: ObservedPropertySimplePU<number>;
    get titleHeight() {
        return this.__titleHeight.get();
    }
    set titleHeight(s26: number) {
        this.__titleHeight.set(s26);
    }
    private __singleRowHeight: ObservedPropertySimplePU<number>;
    get singleRowHeight() {
        return this.__singleRowHeight.get();
    }
    set singleRowHeight(r26: number) {
        this.__singleRowHeight.set(r26);
    }
    private __maxLengthOneLine: SynchedPropertySimpleOneWayPU<number>;
    get maxLengthOneLine() {
        return this.__maxLengthOneLine.get();
    }
    set maxLengthOneLine(q26: number) {
        this.__maxLengthOneLine.set(q26);
    }
    private __isChange: ObservedPropertySimplePU<boolean>;
    get isChange() {
        return this.__isChange.get();
    }
    set isChange(p26: boolean) {
        this.__isChange.set(p26);
    }
    onColorModeChange() {
        let o26 = this.currentMode === ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT ? false : true;
    }
    aboutToAppear() {
        const n26: Context = getContext(this);
        this.titleHeight = 56;
        this.getTargetsToDisplay();
    }
    onTargetInfoChanged() {
        this.isChange = true;
        this.getTargetsToDisplay();
    }
    getTargetsToDisplay() {
        const m26: TargetInfo[][] = this.splitArray(this.targetInfoArray);
        this.targetInfoArraySplit = this.fillUpTheGapsForOneLine(m26);
        if (this.isChange) {
            this.scrollerHeight = (this.panelHeight - this.titleHeight < this
                .singleRowHeight * this.targetInfoArraySplit.length ? this.panelHeight - this.titleHeight
                : this.singleRowHeight * this.targetInfoArraySplit.length) + 16;
            this.realHeight = this.titleHeight + this.scrollerHeight;
        }
    }
    splitArray(h26: Array<TargetInfo>): Array<Array<TargetInfo>> {
        let i26: TargetInfo[][] = [];
        if (this.maxLengthOneLine === 0) {
            i26.push(h26);
            return i26;
        }
        let j26 = Math.floor(h26.length / this.maxLengthOneLine);
        if (j26 === 0) {
            i26.push(h26);
            return i26;
        }
        let k26 = 0;
        for (k26 = 0; k26 < j26; k26++) {
            i26.push(h26.slice(k26 * this.maxLengthOneLine, (k26 + 1) * this.maxLengthOneLine));
        }
        let l26 = h26.length % this.maxLengthOneLine;
        if (l26 !== 0) {
            i26.push(h26.slice(k26 * this.maxLengthOneLine, k26 * this.maxLengthOneLine + l26));
        }
        return i26;
    }
    fillUpTheGapsForOneLine(d26: Array<Array<TargetInfo>>): Array<Array<TargetInfo>> {
        const e26: Array<TargetInfo> | undefined = d26.pop();
        if (e26 === undefined) {
            return d26;
        }
        const f26 = e26.length;
        if (f26 === this.maxLengthOneLine) {
            d26.push(e26);
            return d26;
        }
        for (let g26 = 0; g26 < this.maxLengthOneLine - f26; g26++) {
            e26.push(new TargetInfo(null, null, false));
        }
        d26.push(e26);
        return d26;
    }
    initialRender() {
        this.observeComponentCreation((z25, a26) => {
            ViewStackProcessor.StartGetAccessRecordingFor(z25);
            Column.create();
            Column.alignItems(HorizontalAlign.Start);
            Column.onAreaChange((b26: Area, c26: Area) => {
                if (b26.height !== c26.height) {
                    this.scrollerHeight =
                        (this.panelHeight - this.titleHeight <
                            this.singleRowHeight * this.targetInfoArraySplit.length ? this.panelHeight -
                            this.titleHeight : this.singleRowHeight * this.targetInfoArraySplit.length) + 16;
                    this.realHeight = this.titleHeight + this.scrollerHeight;
                }
            });
            if (!a26) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((x25, y25) => {
            ViewStackProcessor.StartGetAccessRecordingFor(x25);
            Row.create();
            Row.justifyContent(FlexAlign.Start);
            Row.width('100%');
            Row.padding({
                left: 16,
                right: 16
            });
            if (!y25) {
                Row.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        {
            this.observeComponentCreation((t25, u25) => {
                ViewStackProcessor.StartGetAccessRecordingFor(t25);
                if (u25) {
                    let v25 = new TitleComponent(this, { storage: this.storage }, undefined, t25, () => { }, { page: "entry/src/main/ets/view/ContentDisplayComponent.ets", line: 110, col: 17 });
                    ViewPU.create(v25);
                    let w25 = () => {
                        return {
                            storage: this.storage
                        };
                    };
                    v25.paramsGenerator_ = w25;
                }
                else {
                    this.updateStateVarsOfChildByElmtId(t25, {});
                }
                ViewStackProcessor.StopGetAccessRecording();
            });
        }
        Row.pop();
        this.observeComponentCreation((r25, s25) => {
            ViewStackProcessor.StartGetAccessRecordingFor(r25);
            Scroll.create(this.scroller);
            Scroll.constraintSize({ maxHeight: this.panelHeight - this.titleHeight });
            Scroll.width('100%');
            Scroll.scrollable(ScrollDirection.Vertical);
            Scroll.scrollBar(BarState.On);
            Scroll.edgeEffect(EdgeEffect.None);
            if (!s25) {
                Scroll.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((p25, q25) => {
            ViewStackProcessor.StartGetAccessRecordingFor(p25);
            Column.create();
            Column.padding({
                left: 8,
                right: 8,
                top: 16,
                bottom: 8
            });
            if (!q25) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((f25, g25) => {
            ViewStackProcessor.StartGetAccessRecordingFor(f25);
            ForEach.create();
            const h25 = (i25, j25?: number) => {
                const k25 = i25;
                {
                    this.observeComponentCreation((l25, m25) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(l25);
                        if (m25) {
                            let n25 = new SingleRowComponent(this, { targetInfoArray: k25, storage: this.storage }, undefined, l25, () => { }, { page: "entry/src/main/ets/view/ContentDisplayComponent.ets", line: 122, col: 25 });
                            ViewPU.create(n25);
                            let o25 = () => {
                                return {
                                    targetInfoArray: k25,
                                    storage: this.storage
                                };
                            };
                            n25.paramsGenerator_ = o25;
                        }
                        else {
                            this.updateStateVarsOfChildByElmtId(l25, {});
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                }
            };
            this.forEachUpdateFunction(f25, this.targetInfoArraySplit, h25, undefined, true, false);
            if (!g25) {
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
