if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
interface SelectorPcDialog_Params {
    pcSelectorlist?: SelectorPc;
    hapList?: HapInfo[];
    pcShowMoreHapList?: HapInfo[];
    TAG?;
    moreOpenModeVisiable?: number;
    moreOpenModeAreaHigh?: number;
    moreAppsVisiable?: number;
    moreAppsAreaHigh?: number;
    recommendAppsVisiable?: number;
    checkboxVisiable?: number;
    moreHapListVisibility?: number;
    totalHigh?: number;
    isCheckSelected?: boolean;
    action?: string;
    hapListBackGround?: number[];
    hapListPngVisibility?: number[];
    moreHapListBackGround?: number[];
    moreHapListPngVisibility?: number[];
    moreHapListType?: string;
    isCheck?: boolean;
    selectItem?;
}
import drawableDescriptor from "@ohos:arkui.drawableDescriptor";
interface SelectorPc {
    width: string;
    height: string;
}
interface HapInfo {
    label: string;
    icon: boolean;
    bundle: string;
    ability: string;
    module: string;
}
class SelectorPcDialog extends ViewPU {
    constructor(b14, c14, d14, e14 = -1, f14 = undefined, g14) {
        super(b14, d14, e14, g14);
        if (typeof f14 === "function") {
            this.paramsGenerator_ = f14;
        }
        this.__pcSelectorlist = new ObservedPropertyObjectPU({ width: '100%', height: '240vp' }, this, "pcSelectorlist");
        this.__hapList = new ObservedPropertyObjectPU([], this, "hapList");
        this.__pcShowMoreHapList = new ObservedPropertyObjectPU([], this, "pcShowMoreHapList");
        this.TAG = 'SelectorDialog_Pc_Page';
        this.__moreOpenModeVisiable = new ObservedPropertySimplePU(0, this, "moreOpenModeVisiable");
        this.__moreOpenModeAreaHigh = new ObservedPropertySimplePU(54, this, "moreOpenModeAreaHigh");
        this.__moreAppsVisiable = new ObservedPropertySimplePU(2, this, "moreAppsVisiable");
        this.__moreAppsAreaHigh = new ObservedPropertySimplePU(0, this, "moreAppsAreaHigh");
        this.__recommendAppsVisiable = new ObservedPropertySimplePU(0, this, "recommendAppsVisiable");
        this.__checkboxVisiable = new ObservedPropertySimplePU(0, this, "checkboxVisiable");
        this.__moreHapListVisibility = new ObservedPropertySimplePU(2, this, "moreHapListVisibility");
        this.__totalHigh = new ObservedPropertySimplePU(0, this, "totalHigh");
        this.__isCheckSelected = new ObservedPropertySimplePU(false, this, "isCheckSelected");
        this.__action = new ObservedPropertySimplePU('', this, "action");
        this.__hapListBackGround = new ObservedPropertyObjectPU([0xffffff], this, "hapListBackGround");
        this.__hapListPngVisibility = new ObservedPropertyObjectPU([], this, "hapListPngVisibility");
        this.__moreHapListBackGround = new ObservedPropertyObjectPU([0xffffff], this, "moreHapListBackGround");
        this.__moreHapListPngVisibility = new ObservedPropertyObjectPU([], this, "moreHapListPngVisibility");
        this.moreHapListType = 'reserved/wildcard';
        this.__isCheck = new ObservedPropertySimplePU(false, this, "isCheck");
        this.selectItem = undefined;
        this.setInitiallyProvidedValue(c14);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(a14: SelectorPcDialog_Params) {
        if (a14.pcSelectorlist !== undefined) {
            this.pcSelectorlist = a14.pcSelectorlist;
        }
        if (a14.hapList !== undefined) {
            this.hapList = a14.hapList;
        }
        if (a14.pcShowMoreHapList !== undefined) {
            this.pcShowMoreHapList = a14.pcShowMoreHapList;
        }
        if (a14.TAG !== undefined) {
            this.TAG = a14.TAG;
        }
        if (a14.moreOpenModeVisiable !== undefined) {
            this.moreOpenModeVisiable = a14.moreOpenModeVisiable;
        }
        if (a14.moreOpenModeAreaHigh !== undefined) {
            this.moreOpenModeAreaHigh = a14.moreOpenModeAreaHigh;
        }
        if (a14.moreAppsVisiable !== undefined) {
            this.moreAppsVisiable = a14.moreAppsVisiable;
        }
        if (a14.moreAppsAreaHigh !== undefined) {
            this.moreAppsAreaHigh = a14.moreAppsAreaHigh;
        }
        if (a14.recommendAppsVisiable !== undefined) {
            this.recommendAppsVisiable = a14.recommendAppsVisiable;
        }
        if (a14.checkboxVisiable !== undefined) {
            this.checkboxVisiable = a14.checkboxVisiable;
        }
        if (a14.moreHapListVisibility !== undefined) {
            this.moreHapListVisibility = a14.moreHapListVisibility;
        }
        if (a14.totalHigh !== undefined) {
            this.totalHigh = a14.totalHigh;
        }
        if (a14.isCheckSelected !== undefined) {
            this.isCheckSelected = a14.isCheckSelected;
        }
        if (a14.action !== undefined) {
            this.action = a14.action;
        }
        if (a14.hapListBackGround !== undefined) {
            this.hapListBackGround = a14.hapListBackGround;
        }
        if (a14.hapListPngVisibility !== undefined) {
            this.hapListPngVisibility = a14.hapListPngVisibility;
        }
        if (a14.moreHapListBackGround !== undefined) {
            this.moreHapListBackGround = a14.moreHapListBackGround;
        }
        if (a14.moreHapListPngVisibility !== undefined) {
            this.moreHapListPngVisibility = a14.moreHapListPngVisibility;
        }
        if (a14.moreHapListType !== undefined) {
            this.moreHapListType = a14.moreHapListType;
        }
        if (a14.isCheck !== undefined) {
            this.isCheck = a14.isCheck;
        }
        if (a14.selectItem !== undefined) {
            this.selectItem = a14.selectItem;
        }
    }
    updateStateVars(z13: SelectorPcDialog_Params) {
    }
    purgeVariableDependenciesOnElmtId(y13) {
        this.__pcSelectorlist.purgeDependencyOnElmtId(y13);
        this.__hapList.purgeDependencyOnElmtId(y13);
        this.__pcShowMoreHapList.purgeDependencyOnElmtId(y13);
        this.__moreOpenModeVisiable.purgeDependencyOnElmtId(y13);
        this.__moreOpenModeAreaHigh.purgeDependencyOnElmtId(y13);
        this.__moreAppsVisiable.purgeDependencyOnElmtId(y13);
        this.__moreAppsAreaHigh.purgeDependencyOnElmtId(y13);
        this.__recommendAppsVisiable.purgeDependencyOnElmtId(y13);
        this.__checkboxVisiable.purgeDependencyOnElmtId(y13);
        this.__moreHapListVisibility.purgeDependencyOnElmtId(y13);
        this.__totalHigh.purgeDependencyOnElmtId(y13);
        this.__isCheckSelected.purgeDependencyOnElmtId(y13);
        this.__action.purgeDependencyOnElmtId(y13);
        this.__hapListBackGround.purgeDependencyOnElmtId(y13);
        this.__hapListPngVisibility.purgeDependencyOnElmtId(y13);
        this.__moreHapListBackGround.purgeDependencyOnElmtId(y13);
        this.__moreHapListPngVisibility.purgeDependencyOnElmtId(y13);
        this.__isCheck.purgeDependencyOnElmtId(y13);
    }
    aboutToBeDeleted() {
        this.__pcSelectorlist.aboutToBeDeleted();
        this.__hapList.aboutToBeDeleted();
        this.__pcShowMoreHapList.aboutToBeDeleted();
        this.__moreOpenModeVisiable.aboutToBeDeleted();
        this.__moreOpenModeAreaHigh.aboutToBeDeleted();
        this.__moreAppsVisiable.aboutToBeDeleted();
        this.__moreAppsAreaHigh.aboutToBeDeleted();
        this.__recommendAppsVisiable.aboutToBeDeleted();
        this.__checkboxVisiable.aboutToBeDeleted();
        this.__moreHapListVisibility.aboutToBeDeleted();
        this.__totalHigh.aboutToBeDeleted();
        this.__isCheckSelected.aboutToBeDeleted();
        this.__action.aboutToBeDeleted();
        this.__hapListBackGround.aboutToBeDeleted();
        this.__hapListPngVisibility.aboutToBeDeleted();
        this.__moreHapListBackGround.aboutToBeDeleted();
        this.__moreHapListPngVisibility.aboutToBeDeleted();
        this.__isCheck.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal();
    }
    private __pcSelectorlist: ObservedPropertyObjectPU<SelectorPc>;
    get pcSelectorlist() {
        return this.__pcSelectorlist.get();
    }
    set pcSelectorlist(x13: SelectorPc) {
        this.__pcSelectorlist.set(x13);
    }
    private __hapList: ObservedPropertyObjectPU<HapInfo[]>;
    get hapList() {
        return this.__hapList.get();
    }
    set hapList(w13: HapInfo[]) {
        this.__hapList.set(w13);
    }
    private __pcShowMoreHapList: ObservedPropertyObjectPU<HapInfo[]>;
    get pcShowMoreHapList() {
        return this.__pcShowMoreHapList.get();
    }
    set pcShowMoreHapList(v13: HapInfo[]) {
        this.__pcShowMoreHapList.set(v13);
    }
    private TAG;
    private __moreOpenModeVisiable: ObservedPropertySimplePU<number>;
    get moreOpenModeVisiable() {
        return this.__moreOpenModeVisiable.get();
    }
    set moreOpenModeVisiable(u13: number) {
        this.__moreOpenModeVisiable.set(u13);
    }
    private __moreOpenModeAreaHigh: ObservedPropertySimplePU<number>;
    get moreOpenModeAreaHigh() {
        return this.__moreOpenModeAreaHigh.get();
    }
    set moreOpenModeAreaHigh(t13: number) {
        this.__moreOpenModeAreaHigh.set(t13);
    }
    private __moreAppsVisiable: ObservedPropertySimplePU<number>;
    get moreAppsVisiable() {
        return this.__moreAppsVisiable.get();
    }
    set moreAppsVisiable(s13: number) {
        this.__moreAppsVisiable.set(s13);
    }
    private __moreAppsAreaHigh: ObservedPropertySimplePU<number>;
    get moreAppsAreaHigh() {
        return this.__moreAppsAreaHigh.get();
    }
    set moreAppsAreaHigh(r13: number) {
        this.__moreAppsAreaHigh.set(r13);
    }
    private __recommendAppsVisiable: ObservedPropertySimplePU<number>;
    get recommendAppsVisiable() {
        return this.__recommendAppsVisiable.get();
    }
    set recommendAppsVisiable(q13: number) {
        this.__recommendAppsVisiable.set(q13);
    }
    private __checkboxVisiable: ObservedPropertySimplePU<number>;
    get checkboxVisiable() {
        return this.__checkboxVisiable.get();
    }
    set checkboxVisiable(p13: number) {
        this.__checkboxVisiable.set(p13);
    }
    private __moreHapListVisibility: ObservedPropertySimplePU<number>;
    get moreHapListVisibility() {
        return this.__moreHapListVisibility.get();
    }
    set moreHapListVisibility(o13: number) {
        this.__moreHapListVisibility.set(o13);
    }
    private __totalHigh: ObservedPropertySimplePU<number>;
    get totalHigh() {
        return this.__totalHigh.get();
    }
    set totalHigh(n13: number) {
        this.__totalHigh.set(n13);
    }
    private __isCheckSelected: ObservedPropertySimplePU<boolean>;
    get isCheckSelected() {
        return this.__isCheckSelected.get();
    }
    set isCheckSelected(m13: boolean) {
        this.__isCheckSelected.set(m13);
    }
    private __action: ObservedPropertySimplePU<string>;
    get action() {
        return this.__action.get();
    }
    set action(l13: string) {
        this.__action.set(l13);
    }
    public __hapListBackGround: ObservedPropertyObjectPU<number[]>;
    get hapListBackGround() {
        return this.__hapListBackGround.get();
    }
    set hapListBackGround(k13: number[]) {
        this.__hapListBackGround.set(k13);
    }
    public __hapListPngVisibility: ObservedPropertyObjectPU<number[]>;
    get hapListPngVisibility() {
        return this.__hapListPngVisibility.get();
    }
    set hapListPngVisibility(j13: number[]) {
        this.__hapListPngVisibility.set(j13);
    }
    public __moreHapListBackGround: ObservedPropertyObjectPU<number[]>;
    get moreHapListBackGround() {
        return this.__moreHapListBackGround.get();
    }
    set moreHapListBackGround(i13: number[]) {
        this.__moreHapListBackGround.set(i13);
    }
    public __moreHapListPngVisibility: ObservedPropertyObjectPU<number[]>;
    get moreHapListPngVisibility() {
        return this.__moreHapListPngVisibility.get();
    }
    set moreHapListPngVisibility(h13: number[]) {
        this.__moreHapListPngVisibility.set(h13);
    }
    public moreHapListType: string;
    public __isCheck: ObservedPropertySimplePU<boolean>;
    get isCheck() {
        return this.__isCheck.get();
    }
    set isCheck(g13: boolean) {
        this.__isCheck.set(g13);
    }
    private selectItem;
    aboutToAppear(): void {
        console.log(this.TAG, 'dialog page appears');
        this.hapList = globalThis.params.hapList;
        this.action = globalThis.params.action;
        this.getHapListStyle();
        this.cleanFirstListColorAndVisibility();
    }
    cleanFirstListColorAndVisibility() {
        for (let f13 = 0; f13 <= globalThis.pcShowHapList.length; f13++) {
            this.hapListBackGround[f13] = 0xffffff;
            this.hapListPngVisibility[f13] = 1;
        }
    }
    cleanSecondListColorAndVisibility() {
        for (let e13 = 0; e13 <= this.pcShowMoreHapList.length; e13++) {
            this.moreHapListBackGround[e13] = 0xffffff;
            this.moreHapListPngVisibility[e13] = 1;
        }
    }
    onSelectApp(y12) {
        globalThis.abilityWant.bundleName = y12.split('#')[0];
        globalThis.abilityWant.abilityName = y12.split('#')[1];
        globalThis.abilityWant.moduleName = y12.split('#')[4];
        globalThis.abilityWant.parameters['ohos.extra.param.key.appCloneIndex'] = Number(y12.split('#')[7]);
        globalThis.selectExtensionContext.startAbilityAsCaller(globalThis.abilityWant, (c13, d13) => {
            if (d13) {
                console.error(this.TAG + ' startAbility finish, error: ' + JSON.stringify(d13));
                return;
            }
            console.log(this.TAG + ' startAbility finish, data: ' + JSON.stringify(c13));
        });
        let z12 = y12.split('#')[6];
        let a13 = {
            bundleName: globalThis.abilityWant.bundleName,
            moduleName: globalThis.abilityWant.moduleName,
            abilityName: globalThis.abilityWant.abilityName,
        };
        if (this.isCheckSelected) {
            let b13 = y12.split('#')[5];
            console.log(this.TAG + 'setDefaultApplication typeName:' + JSON.stringify(b13));
            globalThis.defaultAppManager.setDefaultApplication(b13, a13);
        }
        globalThis.selectExtensionContext.terminateSelf();
    }
    onClickMoreApp() {
        try {
            globalThis.bundleManager.queryAbilityInfo({
                action: this.action,
                type: this.moreHapListType,
            }, 0, (v12, w12) => {
                if (w12 != undefined) {
                    for (let x12 = 0; x12 < w12.length; x12++) {
                        this.getHapResource(w12[x12], this.pcShowMoreHapList, x12);
                    }
                    this.getMoreHapListStyle();
                    console.debug('pcShowMoreHapList: ' + JSON.stringify(this.pcShowMoreHapList));
                }
            });
        }
        catch {
            u12 => {
                console.error('queryAbilityInfo error:' + JSON.stringify(u12));
            };
        }
    }
    getHapResource(x11, y11, z11) {
        this.moreHapListBackGround[z11] = 0xffffff;
        this.moreHapListPngVisibility[z11] = 1;
        let a12 = x11.bundleName;
        let b12 = x11.moduleName;
        let c12 = x11.name;
        let d12 = '';
        let e12 = '';
        let f12 = Number(x11.labelId);
        let g12 = x11.uri;
        let h12 = Number(x11.userId);
        let i12 = globalThis.selectExtensionContext.createModuleContext(a12, b12);
        i12.resourceManager.getString(f12).then(k12 => {
            d12 = k12;
            let l12 = Number(x11.iconId);
            i12.resourceManager.getMediaBase64(l12).then(n12 => {
                e12 = n12;
                if (e12.indexOf('image/json') > -1) {
                    try {
                        const r12 = i12.resourceManager.getDrawableDescriptor(l12);
                        if (r12 !== null && r12 !== undefined &&
                            r12 instanceof drawableDescriptor.LayeredDrawableDescriptor) {
                            let s12: drawableDescriptor.LayeredDrawableDescriptor = <drawableDescriptor.LayeredDrawableDescriptor>r12;
                            let t12: drawableDescriptor.DrawableDescriptor = s12.getForeground();
                            if (t12 !== null && t12 !== undefined) {
                                globalThis.jsonIconMap.set(a12 + ':' + b12 + ':' + c12 + ':' + x11.appIndex, t12.getPixelMap());
                            }
                            else {
                                console.error(this.TAG, 'get foregroundDescriptor is null');
                            }
                        }
                    }
                    catch (q12) {
                        console.error(this.TAG, 'get drawableDescriptor error:' + JSON.stringify(q12));
                    }
                }
                let o12 = '';
                o12 = a12 + c12 + b12;
                if (globalThis.params.hapList.find(p12 => String(p12.bundleName + p12.ability + p12.moduleName) ==
                    String(o12)) == undefined) {
                    y11.push(a12 + '#' + c12 + '#' + d12 +
                        '#' + e12 + '#' + b12 + '#' + g12 + '#' + h12);
                    this.getMoreHapListStyle();
                }
            }).catch(m12 => {
                console.error('getMediaBase64 error:' + JSON.stringify(m12));
            });
        }).catch(j12 => {
            console.error('getString error:' + JSON.stringify(j12));
        });
    }
    getMoreHapListStyle() {
        let s11 = 200;
        let t11 = 64;
        let u11 = 48;
        let v11 = 54;
        let w11 = 0;
        w11++;
        if (this.hapList.length == 2) {
            if (w11 == 1) {
                this.totalHigh = 447;
                s11 = 262;
            }
            else if (w11 >= 2) {
                this.totalHigh = 511;
                s11 = 326;
            }
            else {
                ;
            }
        }
        else if (this.hapList.length == 3) {
            this.totalHigh = 479;
            if (w11 >= 1) {
                this.totalHigh = 511;
                s11 = 326;
            }
        }
        else if (this.hapList.length == 4) {
            this.totalHigh = 543;
            s11 = this.hapList.length * t11 + u11 + v11;
            if (w11 != 0) {
            }
            else {
            }
        }
        else if (this.hapList.length > 4) {
            this.totalHigh = 543;
            s11 = 4 * t11 + u11 + v11 - 32;
        }
        this.pcSelectorlist = {
            width: '100%',
            height: s11 + 'vp',
        };
    }
    getHapListStyle() {
        let m11 = 0;
        let n11 = 64;
        let o11 = 48;
        let p11 = 54;
        let q11 = 56;
        let r11 = 81;
        if (globalThis.modelFlag == true) {
            this.recommendAppsVisiable = 2;
            this.moreOpenModeVisiable = 2;
            this.checkboxVisiable = 2;
            m11 = this.hapList.length * n11;
            this.totalHigh = this.hapList.length * n11 + q11 + r11;
            if (this.hapList.length > 6) {
                m11 = 6 * n11;
                this.totalHigh = 6 * n11 + q11 + r11;
            }
        }
        if (globalThis.modelFlag == false) {
            m11 = this.hapList.length * n11 + o11 + p11;
            if (this.hapList.length == 1 || this.hapList.length == 2) {
                this.totalHigh = 415;
            }
            else if (this.hapList.length == 3) {
                this.totalHigh = 479;
            }
            else if (this.hapList.length == 4) {
                this.totalHigh = 543;
            }
            else if (this.hapList.length > 4) {
                this.totalHigh = 543;
                m11 = 4 * n11 + o11 + p11 - 32;
            }
        }
        this.pcSelectorlist = {
            width: '100%',
            height: m11 + 'vp',
        };
    }
    initialRender() {
        this.observeComponentCreation((k11, l11) => {
            ViewStackProcessor.StartGetAccessRecordingFor(k11);
            Flex.create({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            Flex.width('100%');
            Flex.height('100%');
            Flex.backgroundColor('#00ffffff');
            if (!l11) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((i11, j11) => {
            ViewStackProcessor.StartGetAccessRecordingFor(i11);
            Column.create();
            Column.height(this.totalHigh);
            Column.align(Alignment.Center);
            Column.borderRadius(24);
            Column.borderWidth(1);
            Column.borderColor('#e9e9e9');
            Column.width(394);
            Column.backgroundColor('#ffffffff');
            if (!j11) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((g11, h11) => {
            ViewStackProcessor.StartGetAccessRecordingFor(g11);
            Text.create({ "id": 33554462, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontSize(22);
            Text.fontWeight(FontWeight.Medium);
            Text.textAlign(TextAlign.Start);
            Text.margin({ left: 48 });
            Text.width(394);
            Text.height(56);
            if (!h11) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((e11, f11) => {
            ViewStackProcessor.StartGetAccessRecordingFor(e11);
            Scroll.create();
            Scroll.edgeEffect(EdgeEffect.None);
            Scroll.height(this.pcSelectorlist.height);
            Scroll.backgroundColor('#ffffff');
            Scroll.scrollBar(2);
            Scroll.width(346);
            if (!f11) {
                Scroll.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((c11, d11) => {
            ViewStackProcessor.StartGetAccessRecordingFor(c11);
            List.create();
            if (!d11) {
                List.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        {
            const r10 = (a11, b11) => {
                ViewStackProcessor.StartGetAccessRecordingFor(a11);
                s10(a11, b11);
                if (!b11) {
                    ListItem.pop();
                }
                ViewStackProcessor.StopGetAccessRecording();
            };
            const s10 = (y10, z10) => {
                ListItem.create(t10, true);
            };
            const t10 = (u10, v10) => {
                r10(u10, v10);
                this.updateFuncByElmtId.set(u10, r10);
                this.observeComponentCreation((w10, x10) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(w10);
                    Text.create({ "id": 33554459, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
                    Text.fontSize(14);
                    Text.fontWeight(FontWeight.Medium);
                    Text.width(394);
                    Text.height(48);
                    Text.visibility(this.recommendAppsVisiable);
                    if (!x10) {
                        Text.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                Text.pop();
                ListItem.pop();
            };
            this.observeComponentCreation(r10);
            ListItem.pop();
        }
        this.observeComponentCreation((h9, i9) => {
            ViewStackProcessor.StartGetAccessRecordingFor(h9);
            ForEach.create();
            const j9 = (l9, m9) => {
                const n9 = l9;
                {
                    const o9 = (p10, q10) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(p10);
                        p9(p10, q10);
                        if (!q10) {
                            ListItem.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    };
                    const p9 = (n10, o10) => {
                        ListItem.create(q9, true);
                        ListItem.backgroundColor(this.hapListBackGround[m9]);
                        ListItem.borderRadius(12);
                        ListItem.height(64);
                        ListItem.width(328);
                    };
                    const q9 = (r9, s9) => {
                        o9(r9, s9);
                        this.updateFuncByElmtId.set(r9, o9);
                        this.observeComponentCreation((l10, m10) => {
                            ViewStackProcessor.StartGetAccessRecordingFor(l10);
                            Flex.create({ direction: FlexDirection.Row });
                            Flex.onClick(() => {
                                this.cleanFirstListColorAndVisibility();
                                this.cleanSecondListColorAndVisibility();
                                this.selectItem = n9;
                                this.hapListBackGround[m9] = 0x1A0A59F7;
                                this.hapListPngVisibility[m9] = 0;
                                this.isCheck = true;
                            });
                            if (!m10) {
                                Flex.pop();
                            }
                            ViewStackProcessor.StopGetAccessRecording();
                        });
                        this.observeComponentCreation((b10, c10) => {
                            ViewStackProcessor.StartGetAccessRecordingFor(b10);
                            If.create();
                            if (n9.split('#')[3] != '') {
                                this.ifElseBranchUpdateFunction(0, () => {
                                    this.observeComponentCreation((f10, g10) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(f10);
                                        If.create();
                                        if (n9.split('#')[3].indexOf('image/json') > -1) {
                                            this.ifElseBranchUpdateFunction(0, () => {
                                                this.observeComponentCreation((j10, k10) => {
                                                    ViewStackProcessor.StartGetAccessRecordingFor(j10);
                                                    Image.create(globalThis.jsonIconMap.get(n9.split('#')[0] + ':' + n9.split('#')[4] +
                                                        ':' + n9.split('#')[1] + ':' + n9.split('#')[7]));
                                                    Image.height(48);
                                                    Image.width(48);
                                                    Image.alignSelf(ItemAlign.Center);
                                                    Image.margin({ left: 16 });
                                                    Image.borderRadius(12);
                                                    if (!k10) {
                                                        Image.pop();
                                                    }
                                                    ViewStackProcessor.StopGetAccessRecording();
                                                });
                                            });
                                        }
                                        else {
                                            this.ifElseBranchUpdateFunction(1, () => {
                                                this.observeComponentCreation((h10, i10) => {
                                                    ViewStackProcessor.StartGetAccessRecordingFor(h10);
                                                    Image.create(n9.split('#')[3]);
                                                    Image.height(48);
                                                    Image.width(48);
                                                    Image.alignSelf(ItemAlign.Center);
                                                    Image.margin({ left: 16 });
                                                    Image.borderRadius(12);
                                                    if (!i10) {
                                                        Image.pop();
                                                    }
                                                    ViewStackProcessor.StopGetAccessRecording();
                                                });
                                            });
                                        }
                                        if (!g10) {
                                            If.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    If.pop();
                                });
                            }
                            else {
                                this.ifElseBranchUpdateFunction(1, () => {
                                    this.observeComponentCreation((d10, e10) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(d10);
                                        Image.create({ "id": 33554433, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
                                        Image.height(48);
                                        Image.width(48);
                                        Image.alignSelf(ItemAlign.Center);
                                        Image.margin({ left: 16 });
                                        Image.borderRadius(12);
                                        if (!e10) {
                                            Image.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                });
                            }
                            if (!c10) {
                                If.pop();
                            }
                            ViewStackProcessor.StopGetAccessRecording();
                        });
                        If.pop();
                        this.observeComponentCreation((v9, w9) => {
                            ViewStackProcessor.StartGetAccessRecordingFor(v9);
                            If.create();
                            if (n9.split('#')[2] != '') {
                                this.ifElseBranchUpdateFunction(0, () => {
                                    this.observeComponentCreation((z9, a10) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(z9);
                                        Text.create(n9.split('#')[2]);
                                        Text.fontSize(16);
                                        Text.width('60%');
                                        Text.height(64);
                                        Text.alignSelf(ItemAlign.Center);
                                        Text.textOverflow({ overflow: TextOverflow.Ellipsis });
                                        Text.margin({ left: 16 });
                                        if (!a10) {
                                            Text.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    Text.pop();
                                });
                            }
                            else {
                                this.ifElseBranchUpdateFunction(1, () => {
                                    this.observeComponentCreation((x9, y9) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(x9);
                                        Text.create(n9.split('#')[0]);
                                        Text.fontSize(16);
                                        Text.width('60%');
                                        Text.height(64);
                                        Text.alignSelf(ItemAlign.Center);
                                        Text.textOverflow({ overflow: TextOverflow.Ellipsis });
                                        Text.margin({ left: 16 });
                                        if (!y9) {
                                            Text.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    Text.pop();
                                });
                            }
                            if (!w9) {
                                If.pop();
                            }
                            ViewStackProcessor.StopGetAccessRecording();
                        });
                        If.pop();
                        this.observeComponentCreation((t9, u9) => {
                            ViewStackProcessor.StartGetAccessRecordingFor(t9);
                            Image.create({ "id": 33554435, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
                            Image.height(24);
                            Image.width(24);
                            Image.alignSelf(ItemAlign.Center);
                            Image.margin({ right: 16 });
                            Image.visibility(this.hapListPngVisibility[m9]);
                            if (!u9) {
                                Image.pop();
                            }
                            ViewStackProcessor.StopGetAccessRecording();
                        });
                        Flex.pop();
                        ListItem.pop();
                    };
                    this.observeComponentCreation(o9);
                    ListItem.pop();
                }
            };
            this.forEachUpdateFunction(h9, globalThis.pcShowHapList, j9, k9 => k9, true, false);
            if (!i9) {
                ForEach.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        ForEach.pop();
        {
            const w8 = (f9, g9) => {
                ViewStackProcessor.StartGetAccessRecordingFor(f9);
                x8(f9, g9);
                if (!g9) {
                    ListItem.pop();
                }
                ViewStackProcessor.StopGetAccessRecording();
            };
            const x8 = (d9, e9) => {
                ListItem.create(y8, true);
                ListItem.visibility(this.moreOpenModeVisiable);
                ListItem.width(346);
                ListItem.height(this.moreOpenModeAreaHigh);
            };
            const y8 = (z8, a9) => {
                w8(z8, a9);
                this.updateFuncByElmtId.set(z8, w8);
                this.observeComponentCreation((b9, c9) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(b9);
                    Button.createWithLabel({ "id": 33554457, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" }, { type: ButtonType.Capsule });
                    Button.fontSize(16);
                    Button.backgroundColor('#ffffffff');
                    Button.fontWeight(FontWeight.Medium);
                    Button.width(128);
                    Button.height(40);
                    Button.fontColor('#0A59F7');
                    Button.margin({ right: 218 });
                    Button.onClick(() => {
                        this.onClickMoreApp();
                        this.moreOpenModeVisiable = 2;
                        this.moreOpenModeAreaHigh = 0;
                        this.moreHapListVisibility = 0;
                        this.moreAppsVisiable = 0;
                        this.moreAppsAreaHigh = 48;
                    });
                    if (!c9) {
                        Button.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                Button.pop();
                ListItem.pop();
            };
            this.observeComponentCreation(w8);
            ListItem.pop();
        }
        {
            const l8 = (u8, v8) => {
                ViewStackProcessor.StartGetAccessRecordingFor(u8);
                m8(u8, v8);
                if (!v8) {
                    ListItem.pop();
                }
                ViewStackProcessor.StopGetAccessRecording();
            };
            const m8 = (s8, t8) => {
                ListItem.create(n8, true);
                ListItem.width(346);
                ListItem.height(this.moreAppsAreaHigh);
            };
            const n8 = (o8, p8) => {
                l8(o8, p8);
                this.updateFuncByElmtId.set(o8, l8);
                this.observeComponentCreation((q8, r8) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(q8);
                    Text.create({ "id": 33554456, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
                    Text.fontSize(14);
                    Text.fontWeight(FontWeight.Medium);
                    Text.visibility(this.moreAppsVisiable);
                    Text.margin({ right: 234 });
                    if (!r8) {
                        Text.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                Text.pop();
                ListItem.pop();
            };
            this.observeComponentCreation(l8);
            ListItem.pop();
        }
        this.observeComponentCreation((b7, c7) => {
            ViewStackProcessor.StartGetAccessRecordingFor(b7);
            ForEach.create();
            const d7 = (f7, g7) => {
                const h7 = f7;
                {
                    const i7 = (j8, k8) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(j8);
                        j7(j8, k8);
                        if (!k8) {
                            ListItem.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    };
                    const j7 = (h8, i8) => {
                        ListItem.create(k7, true);
                        ListItem.backgroundColor(this.moreHapListBackGround[g7]);
                        ListItem.borderRadius(12);
                        ListItem.height(64);
                        ListItem.width(328);
                        ListItem.visibility(this.moreHapListVisibility);
                    };
                    const k7 = (l7, m7) => {
                        i7(l7, m7);
                        this.updateFuncByElmtId.set(l7, i7);
                        this.observeComponentCreation((f8, g8) => {
                            ViewStackProcessor.StartGetAccessRecordingFor(f8);
                            Flex.create({ direction: FlexDirection.Row });
                            Flex.onClick(() => {
                                this.cleanFirstListColorAndVisibility();
                                this.cleanSecondListColorAndVisibility();
                                this.selectItem = h7;
                                this.moreHapListBackGround[g7] = 0x1A0A59F7;
                                this.moreHapListPngVisibility[g7] = 0;
                                this.isCheck = true;
                            });
                            if (!g8) {
                                Flex.pop();
                            }
                            ViewStackProcessor.StopGetAccessRecording();
                        });
                        this.observeComponentCreation((v7, w7) => {
                            ViewStackProcessor.StartGetAccessRecordingFor(v7);
                            If.create();
                            if (h7.split('#')[3] != '') {
                                this.ifElseBranchUpdateFunction(0, () => {
                                    this.observeComponentCreation((z7, a8) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(z7);
                                        If.create();
                                        if (h7.split('#')[3].indexOf('image/json') > -1) {
                                            this.ifElseBranchUpdateFunction(0, () => {
                                                this.observeComponentCreation((d8, e8) => {
                                                    ViewStackProcessor.StartGetAccessRecordingFor(d8);
                                                    Image.create(globalThis.jsonIconMap.get(h7.split('#')[0] + ':' + h7.split('#')[4] +
                                                        ':' + h7.split('#')[1] + ':' + h7.split('#')[7]));
                                                    Image.height(48);
                                                    Image.width(48);
                                                    Image.alignSelf(ItemAlign.Center);
                                                    Image.margin({ left: 16 });
                                                    Image.borderRadius(12);
                                                    if (!e8) {
                                                        Image.pop();
                                                    }
                                                    ViewStackProcessor.StopGetAccessRecording();
                                                });
                                            });
                                        }
                                        else {
                                            this.ifElseBranchUpdateFunction(1, () => {
                                                this.observeComponentCreation((b8, c8) => {
                                                    ViewStackProcessor.StartGetAccessRecordingFor(b8);
                                                    Image.create(h7.split('#')[3]);
                                                    Image.height(48);
                                                    Image.width(48);
                                                    Image.alignSelf(ItemAlign.Center);
                                                    Image.margin({ left: 16 });
                                                    Image.borderRadius(12);
                                                    if (!c8) {
                                                        Image.pop();
                                                    }
                                                    ViewStackProcessor.StopGetAccessRecording();
                                                });
                                            });
                                        }
                                        if (!a8) {
                                            If.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    If.pop();
                                });
                            }
                            else {
                                this.ifElseBranchUpdateFunction(1, () => {
                                    this.observeComponentCreation((x7, y7) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(x7);
                                        Image.create({ "id": 33554433, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
                                        Image.height(48);
                                        Image.width(48);
                                        Image.alignSelf(ItemAlign.Center);
                                        Image.margin({ left: 16 });
                                        Image.borderRadius(12);
                                        if (!y7) {
                                            Image.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                });
                            }
                            if (!w7) {
                                If.pop();
                            }
                            ViewStackProcessor.StopGetAccessRecording();
                        });
                        If.pop();
                        this.observeComponentCreation((p7, q7) => {
                            ViewStackProcessor.StartGetAccessRecordingFor(p7);
                            If.create();
                            if (h7.split('#')[2] != '') {
                                this.ifElseBranchUpdateFunction(0, () => {
                                    this.observeComponentCreation((t7, u7) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(t7);
                                        Text.create(h7.split('#')[2]);
                                        Text.fontSize(16);
                                        Text.width('60%');
                                        Text.height(64);
                                        Text.alignSelf(ItemAlign.Center);
                                        Text.textOverflow({ overflow: TextOverflow.Ellipsis });
                                        Text.margin({ left: 16 });
                                        if (!u7) {
                                            Text.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    Text.pop();
                                });
                            }
                            else {
                                this.ifElseBranchUpdateFunction(1, () => {
                                    this.observeComponentCreation((r7, s7) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(r7);
                                        Text.create(h7.split('#')[0]);
                                        Text.fontSize(16);
                                        Text.width('60%');
                                        Text.height(64);
                                        Text.alignSelf(ItemAlign.Center);
                                        Text.height(56);
                                        Text.width(394);
                                        if (!s7) {
                                            Text.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    Text.pop();
                                });
                            }
                            if (!q7) {
                                If.pop();
                            }
                            ViewStackProcessor.StopGetAccessRecording();
                        });
                        If.pop();
                        this.observeComponentCreation((n7, o7) => {
                            ViewStackProcessor.StartGetAccessRecordingFor(n7);
                            Image.create({ "id": 33554435, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
                            Image.height(24);
                            Image.width(24);
                            Image.alignSelf(ItemAlign.Center);
                            Image.margin({ right: 16 });
                            Image.visibility(this.moreHapListPngVisibility[g7]);
                            if (!o7) {
                                Image.pop();
                            }
                            ViewStackProcessor.StopGetAccessRecording();
                        });
                        Flex.pop();
                        ListItem.pop();
                    };
                    this.observeComponentCreation(i7);
                    ListItem.pop();
                }
            };
            this.forEachUpdateFunction(b7, this.pcShowMoreHapList, d7, e7 => e7, true, false);
            if (!c7) {
                ForEach.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        ForEach.pop();
        List.pop();
        Scroll.pop();
        this.observeComponentCreation((z6, a7) => {
            ViewStackProcessor.StartGetAccessRecordingFor(z6);
            Flex.create({ direction: FlexDirection.Row });
            Flex.visibility(this.checkboxVisiable);
            Flex.height(48);
            Flex.width(394);
            if (!a7) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((w6, x6) => {
            ViewStackProcessor.StartGetAccessRecordingFor(w6);
            Checkbox.create({ name: 'checkbox' });
            Checkbox.select(this.isCheckSelected);
            Checkbox.selectedColor(0x39a2db);
            Checkbox.margin({ left: 8 });
            Checkbox.onChange((y6: boolean) => {
                this.isCheckSelected = y6;
            });
            if (!x6) {
                Checkbox.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Checkbox.pop();
        this.observeComponentCreation((u6, v6) => {
            ViewStackProcessor.StartGetAccessRecordingFor(u6);
            Text.create({ "id": 33554458, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontSize(14);
            Text.fontColor('#000000');
            Text.fontWeight(FontWeight.Regular);
            if (!v6) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Flex.pop();
        this.observeComponentCreation((s6, t6) => {
            ViewStackProcessor.StartGetAccessRecordingFor(s6);
            Flex.create({ direction: FlexDirection.Row });
            if (!t6) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((q6, r6) => {
            ViewStackProcessor.StartGetAccessRecordingFor(q6);
            Button.createWithLabel({ "id": 33554452, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" }, { type: ButtonType.Capsule });
            Button.backgroundColor('#0d000000');
            Button.fontColor('#0A59F7');
            Button.fontWeight(FontWeight.Medium);
            Button.width(157);
            Button.height(40);
            Button.fontSize(16);
            Button.borderRadius(20);
            Button.margin({ top: 16, left: 16 });
            Button.onClick(() => { globalThis.selectExtensionContext.terminateSelf(); });
            if (!r6) {
                Button.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Button.pop();
        this.observeComponentCreation((k6, l6) => {
            ViewStackProcessor.StartGetAccessRecordingFor(k6);
            If.create();
            if (this.isCheck) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation((o6, p6) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(o6);
                        Button.createWithLabel({ "id": 33554460, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" }, { type: ButtonType.Capsule, stateEffect: this.isCheck });
                        Button.backgroundColor('#EAEAEA');
                        Button.fontColor('#0A59F7');
                        Button.fontWeight(FontWeight.Medium);
                        Button.width(157);
                        Button.height(40);
                        Button.fontSize(16);
                        Button.borderRadius(20);
                        Button.margin({ top: 16, left: 48 });
                        Button.onClick(() => { this.onSelectApp(this.selectItem); });
                        if (!p6) {
                            Button.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    Button.pop();
                });
            }
            else {
                this.ifElseBranchUpdateFunction(1, () => {
                    this.observeComponentCreation((m6, n6) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(m6);
                        Button.createWithLabel({ "id": 33554460, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" }, { type: ButtonType.Capsule, stateEffect: this.isCheck });
                        Button.backgroundColor('#EAEAEA');
                        Button.fontColor('#999999');
                        Button.fontWeight(FontWeight.Medium);
                        Button.width(157);
                        Button.height(40);
                        Button.fontSize(16);
                        Button.borderRadius(20);
                        Button.margin({ top: 16, left: 48 });
                        if (!n6) {
                            Button.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    Button.pop();
                });
            }
            if (!l6) {
                If.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        If.pop();
        Flex.pop();
        Column.pop();
        Flex.pop();
    }
    rerender() {
        this.updateDirtyElements();
    }
    public __resetStateVarsOnReuse__Internal(j6: Object): void {
    }
}
ViewStackProcessor.StartGetAccessRecordingFor(ViewStackProcessor.AllocateNewElmetIdForNextComponent());
loadDocument(new SelectorPcDialog(undefined, {}));
ViewStackProcessor.StopGetAccessRecording();
