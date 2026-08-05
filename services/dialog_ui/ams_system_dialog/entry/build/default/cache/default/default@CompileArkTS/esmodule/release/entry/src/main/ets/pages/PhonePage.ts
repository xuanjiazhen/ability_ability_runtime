if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
interface PhonePage_Params {
    openUIExtension?: boolean;
    uiExtensionInfo?: UIExtensionInfo | null;
    level2Page?: boolean;
    realHeight?: number | PanelHeight;
    displayException?: boolean;
    targetInfoArray?: Array<TargetInfo>;
    targetInfoArraylevel2?: Array<TargetInfo>;
    maxLengthOneLine?: number;
    session?: UIExtensionContentSession;
    currentColorMode?: number;
    currentLang?: string;
    defaultStatusBarHeight?: number;
    statusBarHeight?: number;
    panelTopPadding?: number;
    panelHeight?: number;
    titleHeight?: number;
    rowHeight?: number;
    rowTopPadding?: number;
    isLandscape?: boolean;
    dialogController?: CustomDialogController;
    tipsDialogBottomOffset?: number;
    listener?;
    dialogAlign?: DialogAlignment;
}
import display from "@ohos:display";
import type { TargetInfo, UIExtensionInfo } from '../data/Model';
import { ContentDisplayComponent } from "@bundle:com.ohos.amsdialog/entry/ets/view/ContentDisplayComponent";
import { TargetManager } from "@bundle:com.ohos.amsdialog/entry/ets/data/TargetManager";
import type UIExtensionContentSession from "@ohos:app.ability.UIExtensionContentSession";
import wantConstant from "@ohos:app.ability.wantConstant";
import { ColumnType } from "@bundle:com.ohos.amsdialog/entry/ets/utils/BreakpointSystem";
import type BreakpointSystem from "@bundle:com.ohos.amsdialog/entry/ets/utils/BreakpointSystem";
import { mediaSyncFacade } from "@bundle:com.ohos.amsdialog/entry/ets/utils/MediaQueryFacade";
import mediaquery from "@ohos:mediaquery";
import { DeviceInfoUtil } from "@bundle:com.ohos.amsdialog/entry/ets/utils/DeviceUtil";
import { TipsDialogComponent } from "@bundle:com.ohos.amsdialog/entry/ets/view/TipsDialogComponent";
import ConfigurationConstant from "@ohos:app.ability.ConfigurationConstant";
import { localStorageKeyConstants, TargetsUpperLimitOneLine } from "@bundle:com.ohos.amsdialog/entry/ets/utils/Constants";
const storage = LocalStorage.GetShared();
const TAG = 'SelectorExtensionAbility.PhonePage';
const FOUR_COLUMN_WIDTH: number = 600;
class PhonePage extends ViewPU {
    constructor(z5, a6, b6, c6 = -1, d6 = undefined, e6) {
        super(z5, b6, c6, e6);
        if (typeof d6 === "function") {
            this.paramsGenerator_ = d6;
        }
        this.__openUIExtension = new ObservedPropertySimplePU(false, this, "openUIExtension");
        this.addProvidedVar("openUIExtension", this.__openUIExtension, false);
        this.__uiExtensionInfo = new ObservedPropertyObjectPU(null, this, "uiExtensionInfo");
        this.addProvidedVar("uiExtensionInfo", this.__uiExtensionInfo, false);
        this.__level2Page = new ObservedPropertySimplePU(false, this, "level2Page");
        this.addProvidedVar("level2Page", this.__level2Page, false);
        this.__realHeight = new ObservedPropertySimplePU(PanelHeight.WRAP_CONTENT, this, "realHeight");
        this.addProvidedVar("realHeight", this.__realHeight, false);
        this.__displayException = new ObservedPropertySimplePU(false, this, "displayException");
        this.__targetInfoArray = new ObservedPropertyObjectPU([], this, "targetInfoArray");
        this.__targetInfoArraylevel2 = new ObservedPropertyObjectPU([], this, "targetInfoArraylevel2");
        this.__maxLengthOneLine = new ObservedPropertySimplePU(TargetsUpperLimitOneLine.BREAKPOINT_8_12_COLUMN, this, "maxLengthOneLine");
        this.session = storage.get('session') as UIExtensionContentSession;
        this.__currentColorMode = this.createStorageProp('currentColorMode', ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT, "currentColorMode");
        this.__currentLang = this.createStorageProp('currentLanguage', '', "currentLang");
        this.defaultStatusBarHeight = 0;
        this.statusBarHeight = 0;
        this.panelTopPadding = 0;
        this.__panelHeight = new ObservedPropertySimplePU(0, this, "panelHeight");
        this.titleHeight = 0;
        this.rowHeight = 0;
        this.rowTopPadding = 0;
        this.isLandscape = false;
        this.dialogController = undefined;
        this.tipsDialogBottomOffset = 0;
        this.listener = mediaquery.matchMediaSync('(orientation: landscape)');
        this.__dialogAlign = new ObservedPropertySimplePU(DialogAlignment.Bottom, this, "dialogAlign");
        this.setInitiallyProvidedValue(a6);
        this.declareWatch("level2Page", this.onLevel2PageChanged);
        this.declareWatch("currentLang", this.refreshTarget);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(y5: PhonePage_Params) {
        if (y5.openUIExtension !== undefined) {
            this.openUIExtension = y5.openUIExtension;
        }
        if (y5.uiExtensionInfo !== undefined) {
            this.uiExtensionInfo = y5.uiExtensionInfo;
        }
        if (y5.level2Page !== undefined) {
            this.level2Page = y5.level2Page;
        }
        if (y5.realHeight !== undefined) {
            this.realHeight = y5.realHeight;
        }
        if (y5.displayException !== undefined) {
            this.displayException = y5.displayException;
        }
        if (y5.targetInfoArray !== undefined) {
            this.targetInfoArray = y5.targetInfoArray;
        }
        if (y5.targetInfoArraylevel2 !== undefined) {
            this.targetInfoArraylevel2 = y5.targetInfoArraylevel2;
        }
        if (y5.maxLengthOneLine !== undefined) {
            this.maxLengthOneLine = y5.maxLengthOneLine;
        }
        if (y5.session !== undefined) {
            this.session = y5.session;
        }
        if (y5.defaultStatusBarHeight !== undefined) {
            this.defaultStatusBarHeight = y5.defaultStatusBarHeight;
        }
        if (y5.statusBarHeight !== undefined) {
            this.statusBarHeight = y5.statusBarHeight;
        }
        if (y5.panelTopPadding !== undefined) {
            this.panelTopPadding = y5.panelTopPadding;
        }
        if (y5.panelHeight !== undefined) {
            this.panelHeight = y5.panelHeight;
        }
        if (y5.titleHeight !== undefined) {
            this.titleHeight = y5.titleHeight;
        }
        if (y5.rowHeight !== undefined) {
            this.rowHeight = y5.rowHeight;
        }
        if (y5.rowTopPadding !== undefined) {
            this.rowTopPadding = y5.rowTopPadding;
        }
        if (y5.isLandscape !== undefined) {
            this.isLandscape = y5.isLandscape;
        }
        if (y5.dialogController !== undefined) {
            this.dialogController = y5.dialogController;
        }
        if (y5.tipsDialogBottomOffset !== undefined) {
            this.tipsDialogBottomOffset = y5.tipsDialogBottomOffset;
        }
        if (y5.listener !== undefined) {
            this.listener = y5.listener;
        }
        if (y5.dialogAlign !== undefined) {
            this.dialogAlign = y5.dialogAlign;
        }
    }
    updateStateVars(x5: PhonePage_Params) {
    }
    purgeVariableDependenciesOnElmtId(w5) {
        this.__openUIExtension.purgeDependencyOnElmtId(w5);
        this.__uiExtensionInfo.purgeDependencyOnElmtId(w5);
        this.__level2Page.purgeDependencyOnElmtId(w5);
        this.__realHeight.purgeDependencyOnElmtId(w5);
        this.__displayException.purgeDependencyOnElmtId(w5);
        this.__targetInfoArray.purgeDependencyOnElmtId(w5);
        this.__targetInfoArraylevel2.purgeDependencyOnElmtId(w5);
        this.__maxLengthOneLine.purgeDependencyOnElmtId(w5);
        this.__currentColorMode.purgeDependencyOnElmtId(w5);
        this.__currentLang.purgeDependencyOnElmtId(w5);
        this.__panelHeight.purgeDependencyOnElmtId(w5);
        this.__dialogAlign.purgeDependencyOnElmtId(w5);
    }
    aboutToBeDeleted() {
        this.__openUIExtension.aboutToBeDeleted();
        this.__uiExtensionInfo.aboutToBeDeleted();
        this.__level2Page.aboutToBeDeleted();
        this.__realHeight.aboutToBeDeleted();
        this.__displayException.aboutToBeDeleted();
        this.__targetInfoArray.aboutToBeDeleted();
        this.__targetInfoArraylevel2.aboutToBeDeleted();
        this.__maxLengthOneLine.aboutToBeDeleted();
        this.__currentColorMode.aboutToBeDeleted();
        this.__currentLang.aboutToBeDeleted();
        this.__panelHeight.aboutToBeDeleted();
        this.__dialogAlign.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal();
    }
    private __openUIExtension: ObservedPropertySimplePU<boolean>;
    get openUIExtension() {
        return this.__openUIExtension.get();
    }
    set openUIExtension(v5: boolean) {
        this.__openUIExtension.set(v5);
    }
    private __uiExtensionInfo: ObservedPropertyObjectPU<UIExtensionInfo | null>;
    get uiExtensionInfo() {
        return this.__uiExtensionInfo.get();
    }
    set uiExtensionInfo(u5: UIExtensionInfo | null) {
        this.__uiExtensionInfo.set(u5);
    }
    private __level2Page: ObservedPropertySimplePU<boolean>;
    get level2Page() {
        return this.__level2Page.get();
    }
    set level2Page(t5: boolean) {
        this.__level2Page.set(t5);
    }
    private __realHeight: ObservedPropertySimplePU<number | PanelHeight>;
    get realHeight() {
        return this.__realHeight.get();
    }
    set realHeight(s5: number | PanelHeight) {
        this.__realHeight.set(s5);
    }
    private __displayException: ObservedPropertySimplePU<boolean>;
    get displayException() {
        return this.__displayException.get();
    }
    set displayException(r5: boolean) {
        this.__displayException.set(r5);
    }
    private __targetInfoArray: ObservedPropertyObjectPU<Array<TargetInfo>>;
    get targetInfoArray() {
        return this.__targetInfoArray.get();
    }
    set targetInfoArray(q5: Array<TargetInfo>) {
        this.__targetInfoArray.set(q5);
    }
    private __targetInfoArraylevel2: ObservedPropertyObjectPU<Array<TargetInfo>>;
    get targetInfoArraylevel2() {
        return this.__targetInfoArraylevel2.get();
    }
    set targetInfoArraylevel2(p5: Array<TargetInfo>) {
        this.__targetInfoArraylevel2.set(p5);
    }
    private __maxLengthOneLine: ObservedPropertySimplePU<number>;
    get maxLengthOneLine() {
        return this.__maxLengthOneLine.get();
    }
    set maxLengthOneLine(o5: number) {
        this.__maxLengthOneLine.set(o5);
    }
    private session: UIExtensionContentSession;
    private __currentColorMode: ObservedPropertyAbstractPU<number>;
    get currentColorMode() {
        return this.__currentColorMode.get();
    }
    set currentColorMode(n5: number) {
        this.__currentColorMode.set(n5);
    }
    private __currentLang: ObservedPropertyAbstractPU<string>;
    get currentLang() {
        return this.__currentLang.get();
    }
    set currentLang(m5: string) {
        this.__currentLang.set(m5);
    }
    private defaultStatusBarHeight: number;
    private statusBarHeight: number;
    private panelTopPadding: number;
    private __panelHeight: ObservedPropertySimplePU<number>;
    get panelHeight() {
        return this.__panelHeight.get();
    }
    set panelHeight(l5: number) {
        this.__panelHeight.set(l5);
    }
    private titleHeight: number;
    private rowHeight: number;
    private rowTopPadding: number;
    private isLandscape: boolean;
    private dialogController?: CustomDialogController;
    private tipsDialogBottomOffset: number;
    private listener;
    private __dialogAlign: ObservedPropertySimplePU<DialogAlignment>;
    get dialogAlign() {
        return this.__dialogAlign.get();
    }
    set dialogAlign(k5: DialogAlignment) {
        this.__dialogAlign.set(k5);
    }
    aboutToAppear() {
        console.info(TAG, `aboutToAppear`);
        if (!storage || !storage.get(localStorageKeyConstants.CALLER_WANT)) {
            console.error(TAG, `storage or callerWant is null`);
            return;
        }
        this.refreshTarget();
        const e5: Context = getContext(this);
        this.defaultStatusBarHeight = 137;
        this.rowTopPadding = 16;
        let f5: display.Display;
        try {
            f5 = display.getDefaultDisplaySync();
            this.dialogAlign = DeviceInfoUtil.isTable() || (display.isFoldable() &&
                display.getFoldDisplayMode() === display.FoldDisplayMode.FOLD_DISPLAY_MODE_FULL) ?
                DialogAlignment.Center : DialogAlignment.Bottom;
        }
        catch (j5) {
            console.error(TAG, `Invalid display or screen`);
            return;
        }
        const g5 = f5.width / f5.densityPixels;
        if (g5 < FOUR_COLUMN_WIDTH) {
            this.maxLengthOneLine = TargetsUpperLimitOneLine.BREAKPOINT_4_COLUMN;
        }
        this.registerBreakpointSystem();
        this.tipsDialogBottomOffset = -16;
        this.statusBarHeight = this.defaultStatusBarHeight / f5.densityPixels;
        this.panelTopPadding = 8;
        this.panelHeight = f5.height / f5.densityPixels - this.statusBarHeight - this.panelTopPadding;
        const h5 = (i5: mediaquery.MediaQueryResult): void => this.onPortrait(i5);
        this.listener.on('change', h5);
        this.titleHeight = 56;
        this.rowHeight = 104;
        this.initTipsDialogController();
        this.getTargetsToDisplay();
    }
    refreshTarget() {
        this.targetInfoArraylevel2 =
            TargetManager.getAvailableTargets(storage.get(localStorageKeyConstants.CALLER_WANT) ?? {});
        this.getTargetsToDisplay();
    }
    registerBreakpointSystem() {
        const a5: Context = getContext(this);
        (mediaSyncFacade['BREAK_POINT_SYSTEM'] as BreakpointSystem).register((b5: ColumnType) => {
            let c5: display.Display;
            try {
                c5 = display.getDefaultDisplaySync();
                this.dialogAlign = DeviceInfoUtil.isTable() || (display.isFoldable() &&
                    display.getFoldDisplayMode() === display.FoldDisplayMode.FOLD_DISPLAY_MODE_FULL) ?
                    DialogAlignment.Center : DialogAlignment.Bottom;
            }
            catch (d5) {
                console.error(TAG, `Invalid display or screen`);
                return;
            }
            if (b5 === ColumnType.BREAKPOINT_8_COLUMN || b5 === ColumnType.BREAKPOINT_12_COLUMN) {
                this.maxLengthOneLine = TargetsUpperLimitOneLine.BREAKPOINT_8_12_COLUMN;
                if (DeviceInfoUtil.isTable()) {
                    this.panelTopPadding = (c5.height / c5.densityPixels - this.statusBarHeight) *
                        (this.isLandscape ? 0.1 : 0.2) + this.statusBarHeight;
                }
                this.panelHeight =
                    c5.height / c5.densityPixels - this.statusBarHeight - this.panelTopPadding;
            }
            else {
                this.maxLengthOneLine = TargetsUpperLimitOneLine.BREAKPOINT_4_COLUMN;
                this.panelTopPadding = 8;
                this.panelHeight =
                    c5.height / c5.densityPixels - this.statusBarHeight - this.panelTopPadding;
            }
            if (!this.level2Page) {
                this.targetInfoArray =
                    TargetManager.removePartialTargetListIfExceedsMax(this.targetInfoArraylevel2, 2 * this.maxLengthOneLine);
            }
        });
    }
    onBackPress() {
        console.info(TAG, `onBackPress.this.level2Page: ${this.level2Page}`);
        if (this.level2Page) {
            this.level2Page = false;
            return;
        }
    }
    onPortrait(z4: mediaquery.MediaQueryResult) {
        if (DeviceInfoUtil.isTable() && z4.matches) {
            this.isLandscape = true;
        }
        else {
            this.isLandscape = false;
        }
    }
    onLevel2PageChanged() {
        this.getTargetsToDisplay();
    }
    getTargetsToDisplay() {
        if (this.targetInfoArraylevel2.length === 0) {
            this.displayException = true;
            this.openTipsDialogAndReport();
            return;
        }
        if (this.level2Page) {
            this.targetInfoArray = this.targetInfoArraylevel2;
            return;
        }
        this.targetInfoArray =
            TargetManager.removePartialTargetListIfExceedsMax(this.targetInfoArraylevel2, this.maxLengthOneLine * 2);
    }
    openTipsDialogAndReport(): void {
        this.dialogController?.open();
    }
    initTipsDialogController() {
        this.dialogController = new CustomDialogController({
            builder: () => {
                let x4 = new TipsDialogComponent(this, {
                    storage: storage,
                }, undefined, -1, () => { }, { page: "entry/src/main/ets/pages/PhonePage.ets", line: 183, col: 22 });
                x4.setController(this.dialogController);
                ViewPU.create(x4);
                let y4 = () => {
                    return {
                        storage: storage
                    };
                };
                x4.paramsGenerator_ = y4;
            },
            autoCancel: false,
            backgroundColor: { "id": 125829507, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" },
            maskColor: { "id": 125829357, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" },
            alignment: this.dialogAlign,
            offset: { dx: 0, dy: DeviceInfoUtil.isTable() ? 0 : this.tipsDialogBottomOffset },
            cancel: () => {
                const w4: UIExtensionContentSession = storage.get(localStorageKeyConstants.SESSION) as UIExtensionContentSession;
                w4.terminateSelf();
                storage.clear();
            }
        }, this);
    }
    displayPanel(n4 = null) {
        this.observeComponentCreation((u4, v4) => {
            ViewStackProcessor.StartGetAccessRecordingFor(u4);
            Column.create();
            Column.width('100%');
            Column.backgroundColor({ "id": 125829357, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            if (!v4) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((s4, t4) => {
            ViewStackProcessor.StartGetAccessRecordingFor(s4);
            Panel.create(true);
            Panel.type(PanelType.CUSTOM);
            Panel.customHeight(this.level2Page ? this.realHeight > this.panelHeight ? this.panelHeight : this.realHeight :
                this.titleHeight +
                    this.rowHeight * (this.targetInfoArray.length > this.maxLengthOneLine ? 2 : 1) + this.rowTopPadding);
            Panel.backgroundColor({ "id": 125829552, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Panel.dragBar(false);
            Panel.width(this.maxLengthOneLine === TargetsUpperLimitOneLine.BREAKPOINT_4_COLUMN ? '100%' : 480);
            if (!t4) {
                Panel.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        {
            this.observeComponentCreation((o4, p4) => {
                ViewStackProcessor.StartGetAccessRecordingFor(o4);
                if (p4) {
                    let q4 = new ContentDisplayComponent(this, {
                        storage: storage,
                        maxLengthOneLine: this.maxLengthOneLine,
                        targetInfoArray: this.__targetInfoArray,
                        panelHeight: this.panelHeight
                    }, undefined, o4, () => { }, { page: "entry/src/main/ets/pages/PhonePage.ets", line: 204, col: 17 });
                    ViewPU.create(q4);
                    let r4 = () => {
                        return {
                            storage: storage,
                            maxLengthOneLine: this.maxLengthOneLine,
                            targetInfoArray: this.targetInfoArray,
                            panelHeight: this.panelHeight
                        };
                    };
                    q4.paramsGenerator_ = r4;
                }
                else {
                    this.updateStateVarsOfChildByElmtId(o4, {
                        maxLengthOneLine: this.maxLengthOneLine,
                        panelHeight: this.panelHeight
                    });
                }
                ViewStackProcessor.StopGetAccessRecording();
            });
        }
        Panel.pop();
        Column.pop();
    }
    displayUIExtension(g4: UIExtensionInfo | null, h4 = null) {
        this.observeComponentCreation((i4, j4) => {
            ViewStackProcessor.StartGetAccessRecordingFor(i4);
            UIExtensionComponent.create({
                bundleName: g4?.bundleName,
                abilityName: g4?.abilityName,
                flags: wantConstant.Flags.FLAG_AUTH_READ_URI_PERMISSION,
                parameters: g4?.parameters
            });
            UIExtensionComponent.width('100%');
            UIExtensionComponent.height('100%');
            UIExtensionComponent.onRelease((m4) => {
                this.session.terminateSelf();
                storage.clear();
            });
            UIExtensionComponent.onError((l4) => {
                console.error(TAG, `Start ui extension with err: ${l4}`);
                this.session.terminateSelf();
                storage.clear();
            });
            UIExtensionComponent.onResult((k4) => {
                this.session.terminateSelfWithResult({ resultCode: k4.code, want: k4.want });
                storage.clear();
            });
            if (!j4) {
                UIExtensionComponent.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
    }
    initialRender() {
        this.observeComponentCreation((e4, f4) => {
            ViewStackProcessor.StartGetAccessRecordingFor(e4);
            Row.create();
            Row.height('100%');
            Row.width('100%');
            Row.backgroundColor({ "id": 125829357, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            if (!f4) {
                Row.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((a4, b4) => {
            ViewStackProcessor.StartGetAccessRecordingFor(a4);
            If.create();
            if (!this.displayException) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation((c4, d4) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(c4);
                        If.create();
                        if (this.openUIExtension) {
                            this.ifElseBranchUpdateFunction(0, () => {
                                this.displayUIExtension.bind(this)(ObservedObject.GetRawObject(this.uiExtensionInfo));
                            });
                        }
                        else {
                            this.ifElseBranchUpdateFunction(1, () => {
                                this.displayPanel.bind(this)();
                            });
                        }
                        if (!d4) {
                            If.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    If.pop();
                });
            }
            else {
                this.ifElseBranchUpdateFunction(1, () => {
                });
            }
            if (!b4) {
                If.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        If.pop();
        Row.pop();
    }
    rerender() {
        this.updateDirtyElements();
    }
}
if (storage && storage.routeName != undefined && storage.storage != undefined) {
    registerNamedRoute(() => new PhonePage(undefined, {}, storage.useSharedStorage ? LocalStorage.getShared() : storage.storage), storage.routeName, { bundleName: "com.ohos.amsdialog", moduleName: "entry", pagePath: "pages/PhonePage", pageFullPath: "entry/src/main/ets/pages/PhonePage", integratedHsp: "false", moduleType: "followWithHap" });
}
else if (storage && storage.routeName != undefined && storage.storage == undefined) {
    registerNamedRoute(() => new PhonePage(undefined, {}, storage.useSharedStorage ? LocalStorage.getShared() : storage.storage), storage.routeName, { bundleName: "com.ohos.amsdialog", moduleName: "entry", pagePath: "pages/PhonePage", pageFullPath: "entry/src/main/ets/pages/PhonePage", integratedHsp: "false", moduleType: "followWithHap" });
}
else if (storage && storage.routeName == undefined && storage.storage != undefined) {
    ViewStackProcessor.StartGetAccessRecordingFor(ViewStackProcessor.AllocateNewElmetIdForNextComponent());
    loadDocument(new PhonePage(undefined, {}, storage.useSharedStorage ? LocalStorage.getShared() : storage.storage));
    ViewStackProcessor.StopGetAccessRecording();
}
else if (storage && storage.useSharedStorage != undefined) {
    ViewStackProcessor.StartGetAccessRecordingFor(ViewStackProcessor.AllocateNewElmetIdForNextComponent());
    loadDocument(new PhonePage(undefined, {}, storage.useSharedStorage ? LocalStorage.getShared() : undefined));
    ViewStackProcessor.StopGetAccessRecording();
}
else {
    ViewStackProcessor.StartGetAccessRecordingFor(ViewStackProcessor.AllocateNewElmetIdForNextComponent());
    loadDocument(new PhonePage(undefined, {}, storage));
    ViewStackProcessor.StopGetAccessRecording();
}
