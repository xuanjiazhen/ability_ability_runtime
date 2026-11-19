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
    constructor(d6, e6, f6, g6 = -1, h6 = undefined, i6) {
        super(d6, f6, g6, i6);
        if (typeof h6 === "function") {
            this.paramsGenerator_ = h6;
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
        this.setInitiallyProvidedValue(e6);
        this.declareWatch("level2Page", this.onLevel2PageChanged);
        this.declareWatch("currentLang", this.refreshTarget);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(c6: PhonePage_Params) {
        if (c6.openUIExtension !== undefined) {
            this.openUIExtension = c6.openUIExtension;
        }
        if (c6.uiExtensionInfo !== undefined) {
            this.uiExtensionInfo = c6.uiExtensionInfo;
        }
        if (c6.level2Page !== undefined) {
            this.level2Page = c6.level2Page;
        }
        if (c6.realHeight !== undefined) {
            this.realHeight = c6.realHeight;
        }
        if (c6.displayException !== undefined) {
            this.displayException = c6.displayException;
        }
        if (c6.targetInfoArray !== undefined) {
            this.targetInfoArray = c6.targetInfoArray;
        }
        if (c6.targetInfoArraylevel2 !== undefined) {
            this.targetInfoArraylevel2 = c6.targetInfoArraylevel2;
        }
        if (c6.maxLengthOneLine !== undefined) {
            this.maxLengthOneLine = c6.maxLengthOneLine;
        }
        if (c6.session !== undefined) {
            this.session = c6.session;
        }
        if (c6.defaultStatusBarHeight !== undefined) {
            this.defaultStatusBarHeight = c6.defaultStatusBarHeight;
        }
        if (c6.statusBarHeight !== undefined) {
            this.statusBarHeight = c6.statusBarHeight;
        }
        if (c6.panelTopPadding !== undefined) {
            this.panelTopPadding = c6.panelTopPadding;
        }
        if (c6.panelHeight !== undefined) {
            this.panelHeight = c6.panelHeight;
        }
        if (c6.titleHeight !== undefined) {
            this.titleHeight = c6.titleHeight;
        }
        if (c6.rowHeight !== undefined) {
            this.rowHeight = c6.rowHeight;
        }
        if (c6.rowTopPadding !== undefined) {
            this.rowTopPadding = c6.rowTopPadding;
        }
        if (c6.isLandscape !== undefined) {
            this.isLandscape = c6.isLandscape;
        }
        if (c6.dialogController !== undefined) {
            this.dialogController = c6.dialogController;
        }
        if (c6.tipsDialogBottomOffset !== undefined) {
            this.tipsDialogBottomOffset = c6.tipsDialogBottomOffset;
        }
        if (c6.listener !== undefined) {
            this.listener = c6.listener;
        }
        if (c6.dialogAlign !== undefined) {
            this.dialogAlign = c6.dialogAlign;
        }
    }
    updateStateVars(b6: PhonePage_Params) {
    }
    purgeVariableDependenciesOnElmtId(a6) {
        this.__openUIExtension.purgeDependencyOnElmtId(a6);
        this.__uiExtensionInfo.purgeDependencyOnElmtId(a6);
        this.__level2Page.purgeDependencyOnElmtId(a6);
        this.__realHeight.purgeDependencyOnElmtId(a6);
        this.__displayException.purgeDependencyOnElmtId(a6);
        this.__targetInfoArray.purgeDependencyOnElmtId(a6);
        this.__targetInfoArraylevel2.purgeDependencyOnElmtId(a6);
        this.__maxLengthOneLine.purgeDependencyOnElmtId(a6);
        this.__currentColorMode.purgeDependencyOnElmtId(a6);
        this.__currentLang.purgeDependencyOnElmtId(a6);
        this.__panelHeight.purgeDependencyOnElmtId(a6);
        this.__dialogAlign.purgeDependencyOnElmtId(a6);
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
    set openUIExtension(z5: boolean) {
        this.__openUIExtension.set(z5);
    }
    private __uiExtensionInfo: ObservedPropertyObjectPU<UIExtensionInfo | null>;
    get uiExtensionInfo() {
        return this.__uiExtensionInfo.get();
    }
    set uiExtensionInfo(y5: UIExtensionInfo | null) {
        this.__uiExtensionInfo.set(y5);
    }
    private __level2Page: ObservedPropertySimplePU<boolean>;
    get level2Page() {
        return this.__level2Page.get();
    }
    set level2Page(x5: boolean) {
        this.__level2Page.set(x5);
    }
    private __realHeight: ObservedPropertySimplePU<number | PanelHeight>;
    get realHeight() {
        return this.__realHeight.get();
    }
    set realHeight(w5: number | PanelHeight) {
        this.__realHeight.set(w5);
    }
    private __displayException: ObservedPropertySimplePU<boolean>;
    get displayException() {
        return this.__displayException.get();
    }
    set displayException(v5: boolean) {
        this.__displayException.set(v5);
    }
    private __targetInfoArray: ObservedPropertyObjectPU<Array<TargetInfo>>;
    get targetInfoArray() {
        return this.__targetInfoArray.get();
    }
    set targetInfoArray(u5: Array<TargetInfo>) {
        this.__targetInfoArray.set(u5);
    }
    private __targetInfoArraylevel2: ObservedPropertyObjectPU<Array<TargetInfo>>;
    get targetInfoArraylevel2() {
        return this.__targetInfoArraylevel2.get();
    }
    set targetInfoArraylevel2(t5: Array<TargetInfo>) {
        this.__targetInfoArraylevel2.set(t5);
    }
    private __maxLengthOneLine: ObservedPropertySimplePU<number>;
    get maxLengthOneLine() {
        return this.__maxLengthOneLine.get();
    }
    set maxLengthOneLine(s5: number) {
        this.__maxLengthOneLine.set(s5);
    }
    private session: UIExtensionContentSession;
    private __currentColorMode: ObservedPropertyAbstractPU<number>;
    get currentColorMode() {
        return this.__currentColorMode.get();
    }
    set currentColorMode(r5: number) {
        this.__currentColorMode.set(r5);
    }
    private __currentLang: ObservedPropertyAbstractPU<string>;
    get currentLang() {
        return this.__currentLang.get();
    }
    set currentLang(q5: string) {
        this.__currentLang.set(q5);
    }
    private defaultStatusBarHeight: number;
    private statusBarHeight: number;
    private panelTopPadding: number;
    private __panelHeight: ObservedPropertySimplePU<number>;
    get panelHeight() {
        return this.__panelHeight.get();
    }
    set panelHeight(p5: number) {
        this.__panelHeight.set(p5);
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
    set dialogAlign(o5: DialogAlignment) {
        this.__dialogAlign.set(o5);
    }
    aboutToAppear() {
        console.info(TAG, `aboutToAppear`);
        if (!storage || !storage.get(localStorageKeyConstants.CALLER_WANT)) {
            console.error(TAG, `storage or callerWant is null`);
            return;
        }
        this.refreshTarget();
        const i5: Context = getContext(this);
        this.defaultStatusBarHeight = 137;
        this.rowTopPadding = 16;
        let j5: display.Display;
        try {
            j5 = display.getDefaultDisplaySync();
            this.dialogAlign = DeviceInfoUtil.isTable() || (display.isFoldable() &&
                display.getFoldDisplayMode() === display.FoldDisplayMode.FOLD_DISPLAY_MODE_FULL) ?
                DialogAlignment.Center : DialogAlignment.Bottom;
        }
        catch (n5) {
            console.error(TAG, `Invalid display or screen`);
            return;
        }
        const k5 = j5.width / j5.densityPixels;
        if (k5 < FOUR_COLUMN_WIDTH) {
            this.maxLengthOneLine = TargetsUpperLimitOneLine.BREAKPOINT_4_COLUMN;
        }
        this.registerBreakpointSystem();
        this.tipsDialogBottomOffset = -16;
        this.statusBarHeight = this.defaultStatusBarHeight / j5.densityPixels;
        this.panelTopPadding = 8;
        this.panelHeight = j5.height / j5.densityPixels - this.statusBarHeight - this.panelTopPadding;
        const l5 = (m5: mediaquery.MediaQueryResult): void => this.onPortrait(m5);
        this.listener.on('change', l5);
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
        const e5: Context = getContext(this);
        (mediaSyncFacade['BREAK_POINT_SYSTEM'] as BreakpointSystem).register((f5: ColumnType) => {
            let g5: display.Display;
            try {
                g5 = display.getDefaultDisplaySync();
                this.dialogAlign = DeviceInfoUtil.isTable() || (display.isFoldable() &&
                    display.getFoldDisplayMode() === display.FoldDisplayMode.FOLD_DISPLAY_MODE_FULL) ?
                    DialogAlignment.Center : DialogAlignment.Bottom;
            }
            catch (h5) {
                console.error(TAG, `Invalid display or screen`);
                return;
            }
            if (f5 === ColumnType.BREAKPOINT_8_COLUMN || f5 === ColumnType.BREAKPOINT_12_COLUMN) {
                this.maxLengthOneLine = TargetsUpperLimitOneLine.BREAKPOINT_8_12_COLUMN;
                if (DeviceInfoUtil.isTable()) {
                    this.panelTopPadding = (g5.height / g5.densityPixels - this.statusBarHeight) *
                        (this.isLandscape ? 0.1 : 0.2) + this.statusBarHeight;
                }
                this.panelHeight =
                    g5.height / g5.densityPixels - this.statusBarHeight - this.panelTopPadding;
            }
            else {
                this.maxLengthOneLine = TargetsUpperLimitOneLine.BREAKPOINT_4_COLUMN;
                this.panelTopPadding = 8;
                this.panelHeight =
                    g5.height / g5.densityPixels - this.statusBarHeight - this.panelTopPadding;
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
    onPortrait(d5: mediaquery.MediaQueryResult) {
        if (DeviceInfoUtil.isTable() && d5.matches) {
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
                let b5 = new TipsDialogComponent(this, {
                    storage: storage,
                }, undefined, -1, () => { }, { page: "entry/src/main/ets/pages/PhonePage.ets", line: 183, col: 22 });
                b5.setController(this.dialogController);
                ViewPU.create(b5);
                let c5 = () => {
                    return {
                        storage: storage
                    };
                };
                b5.paramsGenerator_ = c5;
            },
            autoCancel: false,
            backgroundColor: { "id": 125829507, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" },
            maskColor: { "id": 125829357, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" },
            alignment: this.dialogAlign,
            offset: { dx: 0, dy: DeviceInfoUtil.isTable() ? 0 : this.tipsDialogBottomOffset },
            cancel: () => {
                const a5: UIExtensionContentSession = storage.get(localStorageKeyConstants.SESSION) as UIExtensionContentSession;
                a5.terminateSelf();
                storage.clear();
            }
        }, this);
    }
    displayPanel(r4 = null) {
        this.observeComponentCreation((y4, z4) => {
            ViewStackProcessor.StartGetAccessRecordingFor(y4);
            Column.create();
            Column.width('100%');
            Column.backgroundColor({ "id": 125829357, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            if (!z4) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((w4, x4) => {
            ViewStackProcessor.StartGetAccessRecordingFor(w4);
            Panel.create(true);
            Panel.type(PanelType.CUSTOM);
            Panel.customHeight(this.level2Page ? this.realHeight > this.panelHeight ? this.panelHeight : this.realHeight :
                this.titleHeight +
                    this.rowHeight * (this.targetInfoArray.length > this.maxLengthOneLine ? 2 : 1) + this.rowTopPadding);
            Panel.backgroundColor({ "id": 125829552, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Panel.dragBar(false);
            Panel.width(this.maxLengthOneLine === TargetsUpperLimitOneLine.BREAKPOINT_4_COLUMN ? '100%' : 480);
            if (!x4) {
                Panel.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        {
            this.observeComponentCreation((s4, t4) => {
                ViewStackProcessor.StartGetAccessRecordingFor(s4);
                if (t4) {
                    let u4 = new ContentDisplayComponent(this, {
                        storage: storage,
                        maxLengthOneLine: this.maxLengthOneLine,
                        targetInfoArray: this.__targetInfoArray,
                        panelHeight: this.panelHeight
                    }, undefined, s4, () => { }, { page: "entry/src/main/ets/pages/PhonePage.ets", line: 204, col: 17 });
                    ViewPU.create(u4);
                    let v4 = () => {
                        return {
                            storage: storage,
                            maxLengthOneLine: this.maxLengthOneLine,
                            __targetInfoArray__Link__Internal: this.__targetInfoArray,
                            targetInfoArray: this.targetInfoArray,
                            panelHeight: this.panelHeight
                        };
                    };
                    u4.paramsGenerator_ = v4;
                }
                else {
                    this.updateStateVarsOfChildByElmtId(s4, {
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
    displayUIExtension(k4: UIExtensionInfo | null, l4 = null) {
        this.observeComponentCreation((m4, n4) => {
            ViewStackProcessor.StartGetAccessRecordingFor(m4);
            UIExtensionComponent.create({
                bundleName: k4?.bundleName,
                abilityName: k4?.abilityName,
                flags: wantConstant.Flags.FLAG_AUTH_READ_URI_PERMISSION,
                parameters: k4?.parameters
            });
            UIExtensionComponent.width('100%');
            UIExtensionComponent.height('100%');
            UIExtensionComponent.onRelease((q4) => {
                this.session.terminateSelf();
                storage.clear();
            });
            UIExtensionComponent.onError((p4) => {
                console.error(TAG, `Start ui extension with err: ${p4}`);
                this.session.terminateSelf();
                storage.clear();
            });
            UIExtensionComponent.onResult((o4) => {
                this.session.terminateSelfWithResult({ resultCode: o4.code, want: o4.want });
                storage.clear();
            });
            if (!n4) {
                UIExtensionComponent.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
    }
    initialRender() {
        this.observeComponentCreation((i4, j4) => {
            ViewStackProcessor.StartGetAccessRecordingFor(i4);
            Row.create();
            Row.height('100%');
            Row.width('100%');
            Row.backgroundColor({ "id": 125829357, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            if (!j4) {
                Row.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((e4, f4) => {
            ViewStackProcessor.StartGetAccessRecordingFor(e4);
            If.create();
            if (!this.displayException) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation((g4, h4) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(g4);
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
                        if (!h4) {
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
            if (!f4) {
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
    public __resetStateVarsOnReuse__Internal(d4: Object): void {
        this.__currentColorMode.resetSource();
        this.__currentLang.resetSource();
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
