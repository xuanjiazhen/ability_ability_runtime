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
    constructor(w13, x13, y13, z13 = -1, a14 = undefined, b14) {
        super(w13, y13, z13, b14);
        if (typeof a14 === "function") {
            this.paramsGenerator_ = a14;
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
        this.setInitiallyProvidedValue(x13);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(v13: SelectorPcDialog_Params) {
        if (v13.pcSelectorlist !== undefined) {
            this.pcSelectorlist = v13.pcSelectorlist;
        }
        if (v13.hapList !== undefined) {
            this.hapList = v13.hapList;
        }
        if (v13.pcShowMoreHapList !== undefined) {
            this.pcShowMoreHapList = v13.pcShowMoreHapList;
        }
        if (v13.TAG !== undefined) {
            this.TAG = v13.TAG;
        }
        if (v13.moreOpenModeVisiable !== undefined) {
            this.moreOpenModeVisiable = v13.moreOpenModeVisiable;
        }
        if (v13.moreOpenModeAreaHigh !== undefined) {
            this.moreOpenModeAreaHigh = v13.moreOpenModeAreaHigh;
        }
        if (v13.moreAppsVisiable !== undefined) {
            this.moreAppsVisiable = v13.moreAppsVisiable;
        }
        if (v13.moreAppsAreaHigh !== undefined) {
            this.moreAppsAreaHigh = v13.moreAppsAreaHigh;
        }
        if (v13.recommendAppsVisiable !== undefined) {
            this.recommendAppsVisiable = v13.recommendAppsVisiable;
        }
        if (v13.checkboxVisiable !== undefined) {
            this.checkboxVisiable = v13.checkboxVisiable;
        }
        if (v13.moreHapListVisibility !== undefined) {
            this.moreHapListVisibility = v13.moreHapListVisibility;
        }
        if (v13.totalHigh !== undefined) {
            this.totalHigh = v13.totalHigh;
        }
        if (v13.isCheckSelected !== undefined) {
            this.isCheckSelected = v13.isCheckSelected;
        }
        if (v13.action !== undefined) {
            this.action = v13.action;
        }
        if (v13.hapListBackGround !== undefined) {
            this.hapListBackGround = v13.hapListBackGround;
        }
        if (v13.hapListPngVisibility !== undefined) {
            this.hapListPngVisibility = v13.hapListPngVisibility;
        }
        if (v13.moreHapListBackGround !== undefined) {
            this.moreHapListBackGround = v13.moreHapListBackGround;
        }
        if (v13.moreHapListPngVisibility !== undefined) {
            this.moreHapListPngVisibility = v13.moreHapListPngVisibility;
        }
        if (v13.moreHapListType !== undefined) {
            this.moreHapListType = v13.moreHapListType;
        }
        if (v13.isCheck !== undefined) {
            this.isCheck = v13.isCheck;
        }
        if (v13.selectItem !== undefined) {
            this.selectItem = v13.selectItem;
        }
    }
    updateStateVars(u13: SelectorPcDialog_Params) {
    }
    purgeVariableDependenciesOnElmtId(t13) {
        this.__pcSelectorlist.purgeDependencyOnElmtId(t13);
        this.__hapList.purgeDependencyOnElmtId(t13);
        this.__pcShowMoreHapList.purgeDependencyOnElmtId(t13);
        this.__moreOpenModeVisiable.purgeDependencyOnElmtId(t13);
        this.__moreOpenModeAreaHigh.purgeDependencyOnElmtId(t13);
        this.__moreAppsVisiable.purgeDependencyOnElmtId(t13);
        this.__moreAppsAreaHigh.purgeDependencyOnElmtId(t13);
        this.__recommendAppsVisiable.purgeDependencyOnElmtId(t13);
        this.__checkboxVisiable.purgeDependencyOnElmtId(t13);
        this.__moreHapListVisibility.purgeDependencyOnElmtId(t13);
        this.__totalHigh.purgeDependencyOnElmtId(t13);
        this.__isCheckSelected.purgeDependencyOnElmtId(t13);
        this.__action.purgeDependencyOnElmtId(t13);
        this.__hapListBackGround.purgeDependencyOnElmtId(t13);
        this.__hapListPngVisibility.purgeDependencyOnElmtId(t13);
        this.__moreHapListBackGround.purgeDependencyOnElmtId(t13);
        this.__moreHapListPngVisibility.purgeDependencyOnElmtId(t13);
        this.__isCheck.purgeDependencyOnElmtId(t13);
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
    set pcSelectorlist(s13: SelectorPc) {
        this.__pcSelectorlist.set(s13);
    }
    private __hapList: ObservedPropertyObjectPU<HapInfo[]>;
    get hapList() {
        return this.__hapList.get();
    }
    set hapList(r13: HapInfo[]) {
        this.__hapList.set(r13);
    }
    private __pcShowMoreHapList: ObservedPropertyObjectPU<HapInfo[]>;
    get pcShowMoreHapList() {
        return this.__pcShowMoreHapList.get();
    }
    set pcShowMoreHapList(q13: HapInfo[]) {
        this.__pcShowMoreHapList.set(q13);
    }
    private TAG;
    private __moreOpenModeVisiable: ObservedPropertySimplePU<number>;
    get moreOpenModeVisiable() {
        return this.__moreOpenModeVisiable.get();
    }
    set moreOpenModeVisiable(p13: number) {
        this.__moreOpenModeVisiable.set(p13);
    }
    private __moreOpenModeAreaHigh: ObservedPropertySimplePU<number>;
    get moreOpenModeAreaHigh() {
        return this.__moreOpenModeAreaHigh.get();
    }
    set moreOpenModeAreaHigh(o13: number) {
        this.__moreOpenModeAreaHigh.set(o13);
    }
    private __moreAppsVisiable: ObservedPropertySimplePU<number>;
    get moreAppsVisiable() {
        return this.__moreAppsVisiable.get();
    }
    set moreAppsVisiable(n13: number) {
        this.__moreAppsVisiable.set(n13);
    }
    private __moreAppsAreaHigh: ObservedPropertySimplePU<number>;
    get moreAppsAreaHigh() {
        return this.__moreAppsAreaHigh.get();
    }
    set moreAppsAreaHigh(m13: number) {
        this.__moreAppsAreaHigh.set(m13);
    }
    private __recommendAppsVisiable: ObservedPropertySimplePU<number>;
    get recommendAppsVisiable() {
        return this.__recommendAppsVisiable.get();
    }
    set recommendAppsVisiable(l13: number) {
        this.__recommendAppsVisiable.set(l13);
    }
    private __checkboxVisiable: ObservedPropertySimplePU<number>;
    get checkboxVisiable() {
        return this.__checkboxVisiable.get();
    }
    set checkboxVisiable(k13: number) {
        this.__checkboxVisiable.set(k13);
    }
    private __moreHapListVisibility: ObservedPropertySimplePU<number>;
    get moreHapListVisibility() {
        return this.__moreHapListVisibility.get();
    }
    set moreHapListVisibility(j13: number) {
        this.__moreHapListVisibility.set(j13);
    }
    private __totalHigh: ObservedPropertySimplePU<number>;
    get totalHigh() {
        return this.__totalHigh.get();
    }
    set totalHigh(i13: number) {
        this.__totalHigh.set(i13);
    }
    private __isCheckSelected: ObservedPropertySimplePU<boolean>;
    get isCheckSelected() {
        return this.__isCheckSelected.get();
    }
    set isCheckSelected(h13: boolean) {
        this.__isCheckSelected.set(h13);
    }
    private __action: ObservedPropertySimplePU<string>;
    get action() {
        return this.__action.get();
    }
    set action(g13: string) {
        this.__action.set(g13);
    }
    public __hapListBackGround: ObservedPropertyObjectPU<number[]>;
    get hapListBackGround() {
        return this.__hapListBackGround.get();
    }
    set hapListBackGround(f13: number[]) {
        this.__hapListBackGround.set(f13);
    }
    public __hapListPngVisibility: ObservedPropertyObjectPU<number[]>;
    get hapListPngVisibility() {
        return this.__hapListPngVisibility.get();
    }
    set hapListPngVisibility(e13: number[]) {
        this.__hapListPngVisibility.set(e13);
    }
    public __moreHapListBackGround: ObservedPropertyObjectPU<number[]>;
    get moreHapListBackGround() {
        return this.__moreHapListBackGround.get();
    }
    set moreHapListBackGround(d13: number[]) {
        this.__moreHapListBackGround.set(d13);
    }
    public __moreHapListPngVisibility: ObservedPropertyObjectPU<number[]>;
    get moreHapListPngVisibility() {
        return this.__moreHapListPngVisibility.get();
    }
    set moreHapListPngVisibility(c13: number[]) {
        this.__moreHapListPngVisibility.set(c13);
    }
    public moreHapListType: string;
    public __isCheck: ObservedPropertySimplePU<boolean>;
    get isCheck() {
        return this.__isCheck.get();
    }
    set isCheck(b13: boolean) {
        this.__isCheck.set(b13);
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
        for (let a13 = 0; a13 <= globalThis.pcShowHapList.length; a13++) {
            this.hapListBackGround[a13] = 0xffffff;
            this.hapListPngVisibility[a13] = 1;
        }
    }
    cleanSecondListColorAndVisibility() {
        for (let z12 = 0; z12 <= this.pcShowMoreHapList.length; z12++) {
            this.moreHapListBackGround[z12] = 0xffffff;
            this.moreHapListPngVisibility[z12] = 1;
        }
    }
    onSelectApp(t12) {
        globalThis.abilityWant.bundleName = t12.split('#')[0];
        globalThis.abilityWant.abilityName = t12.split('#')[1];
        globalThis.abilityWant.moduleName = t12.split('#')[4];
        globalThis.abilityWant.parameters['ohos.extra.param.key.appCloneIndex'] = Number(t12.split('#')[7]);
        globalThis.selectExtensionContext.startAbilityAsCaller(globalThis.abilityWant, (x12, y12) => {
            if (y12) {
                console.error(this.TAG + ' startAbility finish, error: ' + JSON.stringify(y12));
                return;
            }
            console.log(this.TAG + ' startAbility finish, data: ' + JSON.stringify(x12));
        });
        let u12 = t12.split('#')[6];
        let v12 = {
            bundleName: globalThis.abilityWant.bundleName,
            moduleName: globalThis.abilityWant.moduleName,
            abilityName: globalThis.abilityWant.abilityName,
        };
        if (this.isCheckSelected) {
            let w12 = t12.split('#')[5];
            console.log(this.TAG + 'setDefaultApplication typeName:' + JSON.stringify(w12));
            globalThis.defaultAppManager.setDefaultApplication(w12, v12);
        }
        globalThis.selectExtensionContext.terminateSelf();
    }
    onClickMoreApp() {
        try {
            globalThis.bundleManager.queryAbilityInfo({
                action: this.action,
                type: this.moreHapListType,
            }, 0, (q12, r12) => {
                if (r12 != undefined) {
                    for (let s12 = 0; s12 < r12.length; s12++) {
                        this.getHapResource(r12[s12], this.pcShowMoreHapList, s12);
                    }
                    this.getMoreHapListStyle();
                    console.debug('pcShowMoreHapList: ' + JSON.stringify(this.pcShowMoreHapList));
                }
            });
        }
        catch {
            p12 => {
                console.error('queryAbilityInfo error:' + JSON.stringify(p12));
            };
        }
    }
    getHapResource(s11, t11, u11) {
        this.moreHapListBackGround[u11] = 0xffffff;
        this.moreHapListPngVisibility[u11] = 1;
        let v11 = s11.bundleName;
        let w11 = s11.moduleName;
        let x11 = s11.name;
        let y11 = '';
        let z11 = '';
        let a12 = Number(s11.labelId);
        let b12 = s11.uri;
        let c12 = Number(s11.userId);
        let d12 = globalThis.selectExtensionContext.createModuleContext(v11, w11);
        d12.resourceManager.getString(a12).then(f12 => {
            y11 = f12;
            let g12 = Number(s11.iconId);
            d12.resourceManager.getMediaBase64(g12).then(i12 => {
                z11 = i12;
                if (z11.indexOf('image/json') > -1) {
                    try {
                        const m12 = d12.resourceManager.getDrawableDescriptor(g12);
                        if (m12 !== null && m12 !== undefined &&
                            m12 instanceof drawableDescriptor.LayeredDrawableDescriptor) {
                            let n12: drawableDescriptor.LayeredDrawableDescriptor = <drawableDescriptor.LayeredDrawableDescriptor>m12;
                            let o12: drawableDescriptor.DrawableDescriptor = n12.getForeground();
                            if (o12 !== null && o12 !== undefined) {
                                globalThis.jsonIconMap.set(v11 + ':' + w11 + ':' + x11 + ':' + s11.appIndex, o12.getPixelMap());
                            }
                            else {
                                console.error(this.TAG, 'get foregroundDescriptor is null');
                            }
                        }
                    }
                    catch (l12) {
                        console.error(this.TAG, 'get drawableDescriptor error:' + JSON.stringify(l12));
                    }
                }
                let j12 = '';
                j12 = v11 + x11 + w11;
                if (globalThis.params.hapList.find(k12 => String(k12.bundleName + k12.ability + k12.moduleName) ==
                    String(j12)) == undefined) {
                    t11.push(v11 + '#' + x11 + '#' + y11 +
                        '#' + z11 + '#' + w11 + '#' + b12 + '#' + c12);
                    this.getMoreHapListStyle();
                }
            }).catch(h12 => {
                console.error('getMediaBase64 error:' + JSON.stringify(h12));
            });
        }).catch(e12 => {
            console.error('getString error:' + JSON.stringify(e12));
        });
    }
    getMoreHapListStyle() {
        let n11 = 200;
        let o11 = 64;
        let p11 = 48;
        let q11 = 54;
        let r11 = 0;
        r11++;
        if (this.hapList.length == 2) {
            if (r11 == 1) {
                this.totalHigh = 447;
                n11 = 262;
            }
            else if (r11 >= 2) {
                this.totalHigh = 511;
                n11 = 326;
            }
            else {
                ;
            }
        }
        else if (this.hapList.length == 3) {
            this.totalHigh = 479;
            if (r11 >= 1) {
                this.totalHigh = 511;
                n11 = 326;
            }
        }
        else if (this.hapList.length == 4) {
            this.totalHigh = 543;
            n11 = this.hapList.length * o11 + p11 + q11;
            if (r11 != 0) {
            }
            else {
            }
        }
        else if (this.hapList.length > 4) {
            this.totalHigh = 543;
            n11 = 4 * o11 + p11 + q11 - 32;
        }
        this.pcSelectorlist = {
            width: '100%',
            height: n11 + 'vp',
        };
    }
    getHapListStyle() {
        let h11 = 0;
        let i11 = 64;
        let j11 = 48;
        let k11 = 54;
        let l11 = 56;
        let m11 = 81;
        if (globalThis.modelFlag == true) {
            this.recommendAppsVisiable = 2;
            this.moreOpenModeVisiable = 2;
            this.checkboxVisiable = 2;
            h11 = this.hapList.length * i11;
            this.totalHigh = this.hapList.length * i11 + l11 + m11;
            if (this.hapList.length > 6) {
                h11 = 6 * i11;
                this.totalHigh = 6 * i11 + l11 + m11;
            }
        }
        if (globalThis.modelFlag == false) {
            h11 = this.hapList.length * i11 + j11 + k11;
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
                h11 = 4 * i11 + j11 + k11 - 32;
            }
        }
        this.pcSelectorlist = {
            width: '100%',
            height: h11 + 'vp',
        };
    }
    initialRender() {
        this.observeComponentCreation((f11, g11) => {
            ViewStackProcessor.StartGetAccessRecordingFor(f11);
            Flex.create({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
            Flex.width('100%');
            Flex.height('100%');
            Flex.backgroundColor('#00ffffff');
            if (!g11) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((d11, e11) => {
            ViewStackProcessor.StartGetAccessRecordingFor(d11);
            Column.create();
            Column.height(this.totalHigh);
            Column.align(Alignment.Center);
            Column.borderRadius(24);
            Column.borderWidth(1);
            Column.borderColor('#e9e9e9');
            Column.width(394);
            Column.backgroundColor('#ffffffff');
            if (!e11) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((b11, c11) => {
            ViewStackProcessor.StartGetAccessRecordingFor(b11);
            Text.create({ "id": 33554462, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontSize(22);
            Text.fontWeight(FontWeight.Medium);
            Text.textAlign(TextAlign.Start);
            Text.margin({ left: 48 });
            Text.width(394);
            Text.height(56);
            if (!c11) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((z10, a11) => {
            ViewStackProcessor.StartGetAccessRecordingFor(z10);
            Scroll.create();
            Scroll.edgeEffect(EdgeEffect.None);
            Scroll.height(this.pcSelectorlist.height);
            Scroll.backgroundColor('#ffffff');
            Scroll.scrollBar(2);
            Scroll.width(346);
            if (!a11) {
                Scroll.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((x10, y10) => {
            ViewStackProcessor.StartGetAccessRecordingFor(x10);
            List.create();
            if (!y10) {
                List.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        {
            const m10 = (v10, w10) => {
                ViewStackProcessor.StartGetAccessRecordingFor(v10);
                n10(v10, w10);
                if (!w10) {
                    ListItem.pop();
                }
                ViewStackProcessor.StopGetAccessRecording();
            };
            const n10 = (t10, u10) => {
                ListItem.create(o10, true);
            };
            const o10 = (p10, q10) => {
                m10(p10, q10);
                this.updateFuncByElmtId.set(p10, m10);
                this.observeComponentCreation((r10, s10) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(r10);
                    Text.create({ "id": 33554459, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
                    Text.fontSize(14);
                    Text.fontWeight(FontWeight.Medium);
                    Text.width(394);
                    Text.height(48);
                    Text.visibility(this.recommendAppsVisiable);
                    if (!s10) {
                        Text.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                Text.pop();
                ListItem.pop();
            };
            this.observeComponentCreation(m10);
            ListItem.pop();
        }
        this.observeComponentCreation((c9, d9) => {
            ViewStackProcessor.StartGetAccessRecordingFor(c9);
            ForEach.create();
            const e9 = (g9, h9) => {
                const i9 = g9;
                {
                    const j9 = (k10, l10) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(k10);
                        k9(k10, l10);
                        if (!l10) {
                            ListItem.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    };
                    const k9 = (i10, j10) => {
                        ListItem.create(l9, true);
                        ListItem.backgroundColor(this.hapListBackGround[h9]);
                        ListItem.borderRadius(12);
                        ListItem.height(64);
                        ListItem.width(328);
                    };
                    const l9 = (m9, n9) => {
                        j9(m9, n9);
                        this.updateFuncByElmtId.set(m9, j9);
                        this.observeComponentCreation((g10, h10) => {
                            ViewStackProcessor.StartGetAccessRecordingFor(g10);
                            Flex.create({ direction: FlexDirection.Row });
                            Flex.onClick(() => {
                                this.cleanFirstListColorAndVisibility();
                                this.cleanSecondListColorAndVisibility();
                                this.selectItem = i9;
                                this.hapListBackGround[h9] = 0x1A0A59F7;
                                this.hapListPngVisibility[h9] = 0;
                                this.isCheck = true;
                            });
                            if (!h10) {
                                Flex.pop();
                            }
                            ViewStackProcessor.StopGetAccessRecording();
                        });
                        this.observeComponentCreation((w9, x9) => {
                            ViewStackProcessor.StartGetAccessRecordingFor(w9);
                            If.create();
                            if (i9.split('#')[3] != '') {
                                this.ifElseBranchUpdateFunction(0, () => {
                                    this.observeComponentCreation((a10, b10) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(a10);
                                        If.create();
                                        if (i9.split('#')[3].indexOf('image/json') > -1) {
                                            this.ifElseBranchUpdateFunction(0, () => {
                                                this.observeComponentCreation((e10, f10) => {
                                                    ViewStackProcessor.StartGetAccessRecordingFor(e10);
                                                    Image.create(globalThis.jsonIconMap.get(i9.split('#')[0] + ':' + i9.split('#')[4] +
                                                        ':' + i9.split('#')[1] + ':' + i9.split('#')[7]));
                                                    Image.height(48);
                                                    Image.width(48);
                                                    Image.alignSelf(ItemAlign.Center);
                                                    Image.margin({ left: 16 });
                                                    Image.borderRadius(12);
                                                    if (!f10) {
                                                        Image.pop();
                                                    }
                                                    ViewStackProcessor.StopGetAccessRecording();
                                                });
                                            });
                                        }
                                        else {
                                            this.ifElseBranchUpdateFunction(1, () => {
                                                this.observeComponentCreation((c10, d10) => {
                                                    ViewStackProcessor.StartGetAccessRecordingFor(c10);
                                                    Image.create(i9.split('#')[3]);
                                                    Image.height(48);
                                                    Image.width(48);
                                                    Image.alignSelf(ItemAlign.Center);
                                                    Image.margin({ left: 16 });
                                                    Image.borderRadius(12);
                                                    if (!d10) {
                                                        Image.pop();
                                                    }
                                                    ViewStackProcessor.StopGetAccessRecording();
                                                });
                                            });
                                        }
                                        if (!b10) {
                                            If.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    If.pop();
                                });
                            }
                            else {
                                this.ifElseBranchUpdateFunction(1, () => {
                                    this.observeComponentCreation((y9, z9) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(y9);
                                        Image.create({ "id": 33554433, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
                                        Image.height(48);
                                        Image.width(48);
                                        Image.alignSelf(ItemAlign.Center);
                                        Image.margin({ left: 16 });
                                        Image.borderRadius(12);
                                        if (!z9) {
                                            Image.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                });
                            }
                            if (!x9) {
                                If.pop();
                            }
                            ViewStackProcessor.StopGetAccessRecording();
                        });
                        If.pop();
                        this.observeComponentCreation((q9, r9) => {
                            ViewStackProcessor.StartGetAccessRecordingFor(q9);
                            If.create();
                            if (i9.split('#')[2] != '') {
                                this.ifElseBranchUpdateFunction(0, () => {
                                    this.observeComponentCreation((u9, v9) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(u9);
                                        Text.create(i9.split('#')[2]);
                                        Text.fontSize(16);
                                        Text.width('60%');
                                        Text.height(64);
                                        Text.alignSelf(ItemAlign.Center);
                                        Text.textOverflow({ overflow: TextOverflow.Ellipsis });
                                        Text.margin({ left: 16 });
                                        if (!v9) {
                                            Text.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    Text.pop();
                                });
                            }
                            else {
                                this.ifElseBranchUpdateFunction(1, () => {
                                    this.observeComponentCreation((s9, t9) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(s9);
                                        Text.create(i9.split('#')[0]);
                                        Text.fontSize(16);
                                        Text.width('60%');
                                        Text.height(64);
                                        Text.alignSelf(ItemAlign.Center);
                                        Text.textOverflow({ overflow: TextOverflow.Ellipsis });
                                        Text.margin({ left: 16 });
                                        if (!t9) {
                                            Text.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    Text.pop();
                                });
                            }
                            if (!r9) {
                                If.pop();
                            }
                            ViewStackProcessor.StopGetAccessRecording();
                        });
                        If.pop();
                        this.observeComponentCreation((o9, p9) => {
                            ViewStackProcessor.StartGetAccessRecordingFor(o9);
                            Image.create({ "id": 33554435, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
                            Image.height(24);
                            Image.width(24);
                            Image.alignSelf(ItemAlign.Center);
                            Image.margin({ right: 16 });
                            Image.visibility(this.hapListPngVisibility[h9]);
                            if (!p9) {
                                Image.pop();
                            }
                            ViewStackProcessor.StopGetAccessRecording();
                        });
                        Flex.pop();
                        ListItem.pop();
                    };
                    this.observeComponentCreation(j9);
                    ListItem.pop();
                }
            };
            this.forEachUpdateFunction(c9, globalThis.pcShowHapList, e9, f9 => f9, true, false);
            if (!d9) {
                ForEach.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        ForEach.pop();
        {
            const r8 = (a9, b9) => {
                ViewStackProcessor.StartGetAccessRecordingFor(a9);
                s8(a9, b9);
                if (!b9) {
                    ListItem.pop();
                }
                ViewStackProcessor.StopGetAccessRecording();
            };
            const s8 = (y8, z8) => {
                ListItem.create(t8, true);
                ListItem.visibility(this.moreOpenModeVisiable);
                ListItem.width(346);
                ListItem.height(this.moreOpenModeAreaHigh);
            };
            const t8 = (u8, v8) => {
                r8(u8, v8);
                this.updateFuncByElmtId.set(u8, r8);
                this.observeComponentCreation((w8, x8) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(w8);
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
                    if (!x8) {
                        Button.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                Button.pop();
                ListItem.pop();
            };
            this.observeComponentCreation(r8);
            ListItem.pop();
        }
        {
            const g8 = (p8, q8) => {
                ViewStackProcessor.StartGetAccessRecordingFor(p8);
                h8(p8, q8);
                if (!q8) {
                    ListItem.pop();
                }
                ViewStackProcessor.StopGetAccessRecording();
            };
            const h8 = (n8, o8) => {
                ListItem.create(i8, true);
                ListItem.width(346);
                ListItem.height(this.moreAppsAreaHigh);
            };
            const i8 = (j8, k8) => {
                g8(j8, k8);
                this.updateFuncByElmtId.set(j8, g8);
                this.observeComponentCreation((l8, m8) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(l8);
                    Text.create({ "id": 33554456, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
                    Text.fontSize(14);
                    Text.fontWeight(FontWeight.Medium);
                    Text.visibility(this.moreAppsVisiable);
                    Text.margin({ right: 234 });
                    if (!m8) {
                        Text.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                Text.pop();
                ListItem.pop();
            };
            this.observeComponentCreation(g8);
            ListItem.pop();
        }
        this.observeComponentCreation((w6, x6) => {
            ViewStackProcessor.StartGetAccessRecordingFor(w6);
            ForEach.create();
            const y6 = (a7, b7) => {
                const c7 = a7;
                {
                    const d7 = (e8, f8) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(e8);
                        e7(e8, f8);
                        if (!f8) {
                            ListItem.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    };
                    const e7 = (c8, d8) => {
                        ListItem.create(f7, true);
                        ListItem.backgroundColor(this.moreHapListBackGround[b7]);
                        ListItem.borderRadius(12);
                        ListItem.height(64);
                        ListItem.width(328);
                        ListItem.visibility(this.moreHapListVisibility);
                    };
                    const f7 = (g7, h7) => {
                        d7(g7, h7);
                        this.updateFuncByElmtId.set(g7, d7);
                        this.observeComponentCreation((a8, b8) => {
                            ViewStackProcessor.StartGetAccessRecordingFor(a8);
                            Flex.create({ direction: FlexDirection.Row });
                            Flex.onClick(() => {
                                this.cleanFirstListColorAndVisibility();
                                this.cleanSecondListColorAndVisibility();
                                this.selectItem = c7;
                                this.moreHapListBackGround[b7] = 0x1A0A59F7;
                                this.moreHapListPngVisibility[b7] = 0;
                                this.isCheck = true;
                            });
                            if (!b8) {
                                Flex.pop();
                            }
                            ViewStackProcessor.StopGetAccessRecording();
                        });
                        this.observeComponentCreation((q7, r7) => {
                            ViewStackProcessor.StartGetAccessRecordingFor(q7);
                            If.create();
                            if (c7.split('#')[3] != '') {
                                this.ifElseBranchUpdateFunction(0, () => {
                                    this.observeComponentCreation((u7, v7) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(u7);
                                        If.create();
                                        if (c7.split('#')[3].indexOf('image/json') > -1) {
                                            this.ifElseBranchUpdateFunction(0, () => {
                                                this.observeComponentCreation((y7, z7) => {
                                                    ViewStackProcessor.StartGetAccessRecordingFor(y7);
                                                    Image.create(globalThis.jsonIconMap.get(c7.split('#')[0] + ':' + c7.split('#')[4] +
                                                        ':' + c7.split('#')[1] + ':' + c7.split('#')[7]));
                                                    Image.height(48);
                                                    Image.width(48);
                                                    Image.alignSelf(ItemAlign.Center);
                                                    Image.margin({ left: 16 });
                                                    Image.borderRadius(12);
                                                    if (!z7) {
                                                        Image.pop();
                                                    }
                                                    ViewStackProcessor.StopGetAccessRecording();
                                                });
                                            });
                                        }
                                        else {
                                            this.ifElseBranchUpdateFunction(1, () => {
                                                this.observeComponentCreation((w7, x7) => {
                                                    ViewStackProcessor.StartGetAccessRecordingFor(w7);
                                                    Image.create(c7.split('#')[3]);
                                                    Image.height(48);
                                                    Image.width(48);
                                                    Image.alignSelf(ItemAlign.Center);
                                                    Image.margin({ left: 16 });
                                                    Image.borderRadius(12);
                                                    if (!x7) {
                                                        Image.pop();
                                                    }
                                                    ViewStackProcessor.StopGetAccessRecording();
                                                });
                                            });
                                        }
                                        if (!v7) {
                                            If.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    If.pop();
                                });
                            }
                            else {
                                this.ifElseBranchUpdateFunction(1, () => {
                                    this.observeComponentCreation((s7, t7) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(s7);
                                        Image.create({ "id": 33554433, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
                                        Image.height(48);
                                        Image.width(48);
                                        Image.alignSelf(ItemAlign.Center);
                                        Image.margin({ left: 16 });
                                        Image.borderRadius(12);
                                        if (!t7) {
                                            Image.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                });
                            }
                            if (!r7) {
                                If.pop();
                            }
                            ViewStackProcessor.StopGetAccessRecording();
                        });
                        If.pop();
                        this.observeComponentCreation((k7, l7) => {
                            ViewStackProcessor.StartGetAccessRecordingFor(k7);
                            If.create();
                            if (c7.split('#')[2] != '') {
                                this.ifElseBranchUpdateFunction(0, () => {
                                    this.observeComponentCreation((o7, p7) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(o7);
                                        Text.create(c7.split('#')[2]);
                                        Text.fontSize(16);
                                        Text.width('60%');
                                        Text.height(64);
                                        Text.alignSelf(ItemAlign.Center);
                                        Text.textOverflow({ overflow: TextOverflow.Ellipsis });
                                        Text.margin({ left: 16 });
                                        if (!p7) {
                                            Text.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    Text.pop();
                                });
                            }
                            else {
                                this.ifElseBranchUpdateFunction(1, () => {
                                    this.observeComponentCreation((m7, n7) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(m7);
                                        Text.create(c7.split('#')[0]);
                                        Text.fontSize(16);
                                        Text.width('60%');
                                        Text.height(64);
                                        Text.alignSelf(ItemAlign.Center);
                                        Text.height(56);
                                        Text.width(394);
                                        if (!n7) {
                                            Text.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    Text.pop();
                                });
                            }
                            if (!l7) {
                                If.pop();
                            }
                            ViewStackProcessor.StopGetAccessRecording();
                        });
                        If.pop();
                        this.observeComponentCreation((i7, j7) => {
                            ViewStackProcessor.StartGetAccessRecordingFor(i7);
                            Image.create({ "id": 33554435, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
                            Image.height(24);
                            Image.width(24);
                            Image.alignSelf(ItemAlign.Center);
                            Image.margin({ right: 16 });
                            Image.visibility(this.moreHapListPngVisibility[b7]);
                            if (!j7) {
                                Image.pop();
                            }
                            ViewStackProcessor.StopGetAccessRecording();
                        });
                        Flex.pop();
                        ListItem.pop();
                    };
                    this.observeComponentCreation(d7);
                    ListItem.pop();
                }
            };
            this.forEachUpdateFunction(w6, this.pcShowMoreHapList, y6, z6 => z6, true, false);
            if (!x6) {
                ForEach.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        ForEach.pop();
        List.pop();
        Scroll.pop();
        this.observeComponentCreation((u6, v6) => {
            ViewStackProcessor.StartGetAccessRecordingFor(u6);
            Flex.create({ direction: FlexDirection.Row });
            Flex.visibility(this.checkboxVisiable);
            Flex.height(48);
            Flex.width(394);
            if (!v6) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((r6, s6) => {
            ViewStackProcessor.StartGetAccessRecordingFor(r6);
            Checkbox.create({ name: 'checkbox' });
            Checkbox.select(this.isCheckSelected);
            Checkbox.selectedColor(0x39a2db);
            Checkbox.margin({ left: 8 });
            Checkbox.onChange((t6: boolean) => {
                this.isCheckSelected = t6;
            });
            if (!s6) {
                Checkbox.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Checkbox.pop();
        this.observeComponentCreation((p6, q6) => {
            ViewStackProcessor.StartGetAccessRecordingFor(p6);
            Text.create({ "id": 33554458, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontSize(14);
            Text.fontColor('#000000');
            Text.fontWeight(FontWeight.Regular);
            if (!q6) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Flex.pop();
        this.observeComponentCreation((n6, o6) => {
            ViewStackProcessor.StartGetAccessRecordingFor(n6);
            Flex.create({ direction: FlexDirection.Row });
            if (!o6) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((l6, m6) => {
            ViewStackProcessor.StartGetAccessRecordingFor(l6);
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
            if (!m6) {
                Button.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Button.pop();
        this.observeComponentCreation((f6, g6) => {
            ViewStackProcessor.StartGetAccessRecordingFor(f6);
            If.create();
            if (this.isCheck) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation((j6, k6) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(j6);
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
                        if (!k6) {
                            Button.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    Button.pop();
                });
            }
            else {
                this.ifElseBranchUpdateFunction(1, () => {
                    this.observeComponentCreation((h6, i6) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(h6);
                        Button.createWithLabel({ "id": 33554460, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" }, { type: ButtonType.Capsule, stateEffect: this.isCheck });
                        Button.backgroundColor('#EAEAEA');
                        Button.fontColor('#999999');
                        Button.fontWeight(FontWeight.Medium);
                        Button.width(157);
                        Button.height(40);
                        Button.fontSize(16);
                        Button.borderRadius(20);
                        Button.margin({ top: 16, left: 48 });
                        if (!i6) {
                            Button.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    Button.pop();
                });
            }
            if (!g6) {
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
}
ViewStackProcessor.StartGetAccessRecordingFor(ViewStackProcessor.AllocateNewElmetIdForNextComponent());
loadDocument(new SelectorPcDialog(undefined, {}));
ViewStackProcessor.StopGetAccessRecording();
