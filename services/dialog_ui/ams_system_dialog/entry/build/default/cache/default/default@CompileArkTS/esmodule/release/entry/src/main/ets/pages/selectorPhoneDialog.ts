if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
interface SelectorPhoneDialog_Params {
    phoneShowData?: SelectorDataSource;
    oversizeHeight?: string;
    selector?: SelectorPhone;
    hapList?: HapInfo[];
    TAG?;
}
interface SwiperPhone {
    height: string;
    indicator: boolean;
    contentHeight: string;
    gridColumns: string;
    gridRows: string;
}
interface SelectorBtn {
    marginTop: string;
}
interface SelectorPhone {
    swiper: SwiperPhone;
    btn: SelectorBtn;
}
interface HapInfo {
    label: string;
    icon: boolean;
    bundle: string;
    ability: string;
    module: string;
}
class SelectorDataSource implements IDataSource {
    private list: string[] = [];
    private listener: DataChangeListener;
    constructor(g17: string[]) {
        this.list = g17;
    }
    totalCount(): number {
        return this.list.length;
    }
    getData(f17: number): string {
        return this.list[f17];
    }
    registerDataChangeListener(e17: DataChangeListener): void {
        this.listener = e17;
    }
    unregisterDataChangeListener() {
    }
}
class SelectorPhoneDialog extends ViewPU {
    constructor(y16, z16, a17, b17 = -1, c17 = undefined, d17) {
        super(y16, a17, b17, d17);
        if (typeof c17 === "function") {
            this.paramsGenerator_ = c17;
        }
        this.phoneShowData = new SelectorDataSource([]);
        this.__oversizeHeight = this.createStorageLink('oversizeHeight', 'false', "oversizeHeight");
        this.__selector = new ObservedPropertyObjectPU({
            swiper: {
                height: '100vp',
                indicator: false,
                contentHeight: '100vp',
                gridColumns: '1fr 1fr',
                gridRows: '1fr',
            },
            btn: {
                marginTop: '20vp',
            }
        }, this, "selector");
        this.__hapList = new ObservedPropertyObjectPU([], this, "hapList");
        this.TAG = 'SelectorDialog_Phone_Page';
        this.setInitiallyProvidedValue(z16);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(x16: SelectorPhoneDialog_Params) {
        if (x16.phoneShowData !== undefined) {
            this.phoneShowData = x16.phoneShowData;
        }
        if (x16.selector !== undefined) {
            this.selector = x16.selector;
        }
        if (x16.hapList !== undefined) {
            this.hapList = x16.hapList;
        }
        if (x16.TAG !== undefined) {
            this.TAG = x16.TAG;
        }
    }
    updateStateVars(w16: SelectorPhoneDialog_Params) {
    }
    purgeVariableDependenciesOnElmtId(v16) {
        this.__oversizeHeight.purgeDependencyOnElmtId(v16);
        this.__selector.purgeDependencyOnElmtId(v16);
        this.__hapList.purgeDependencyOnElmtId(v16);
    }
    aboutToBeDeleted() {
        this.__oversizeHeight.aboutToBeDeleted();
        this.__selector.aboutToBeDeleted();
        this.__hapList.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal();
    }
    private phoneShowData: SelectorDataSource;
    private __oversizeHeight: ObservedPropertyAbstractPU<string>;
    get oversizeHeight() {
        return this.__oversizeHeight.get();
    }
    set oversizeHeight(u16: string) {
        this.__oversizeHeight.set(u16);
    }
    private __selector: ObservedPropertyObjectPU<SelectorPhone>;
    get selector() {
        return this.__selector.get();
    }
    set selector(t16: SelectorPhone) {
        this.__selector.set(t16);
    }
    private __hapList: ObservedPropertyObjectPU<HapInfo[]>;
    get hapList() {
        return this.__hapList.get();
    }
    set hapList(s16: HapInfo[]) {
        this.__hapList.set(s16);
    }
    private TAG;
    elementIconCombine(h16: string, i16: string, j16 = null) {
        this.observeComponentCreation((q16, r16) => {
            ViewStackProcessor.StartGetAccessRecordingFor(q16);
            RelativeContainer.create();
            RelativeContainer.height(60);
            RelativeContainer.width(60);
            RelativeContainer.alignSelf(ItemAlign.Center);
            if (!r16) {
                RelativeContainer.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((o16, p16) => {
            ViewStackProcessor.StartGetAccessRecordingFor(o16);
            Image.create(h16);
            Image.id('iconLab');
            Image.height(60);
            Image.width(60);
            Image.alignSelf(ItemAlign.Center);
            if (!p16) {
                Image.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((k16, l16) => {
            ViewStackProcessor.StartGetAccessRecordingFor(k16);
            If.create();
            if (i16 !== '0') {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation((m16, n16) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(m16);
                        Image.create({ "id": -1, "type": -1, params: ['app.media.app_icon_clone_index_' + i16], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
                        Image.height(18);
                        Image.width(18);
                        Image.alignRules({
                            end: { anchor: 'iconLab', align: HorizontalAlign.End },
                            bottom: { anchor: 'iconLab', align: VerticalAlign.Bottom }
                        });
                        if (!n16) {
                            Image.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                });
            }
            else {
                this.ifElseBranchUpdateFunction(1, () => {
                });
            }
            if (!l16) {
                If.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        If.pop();
        RelativeContainer.pop();
    }
    aboutToAppear(): void {
        console.log(this.TAG, 'dialog page appears');
        this.hapList = globalThis.params.hapList;
        this.getHapListStyle();
    }
    onPageShow() {
        console.info(this.TAG, 'onPageShow called');
        this.getHapListStyle();
    }
    onSelectApp(e16) {
        console.log(this.TAG, 'onSelectApp - start ');
        globalThis.abilityWant.bundleName = e16.split('#')[0];
        globalThis.abilityWant.abilityName = e16.split('#')[1];
        globalThis.abilityWant.moduleName = e16.split('#')[4];
        globalThis.abilityWant.parameters['ohos.extra.param.key.appCloneIndex'] = Number(e16.split('#')[7]);
        globalThis.selectExtensionContext.startAbilityAsCaller(globalThis.abilityWant, (f16, g16) => {
            if (g16) {
                console.error(this.TAG + ' startAbility finish, error: ' + JSON.stringify(g16));
                return;
            }
            console.log(this.TAG + ' startAbility finish, data: ' + JSON.stringify(f16));
        });
        globalThis.selectExtensionContext.terminateSelf();
    }
    getHapListStyle() {
        if (this.oversizeHeight === 'false') {
            if (this.hapList.length > 8) {
                this.selector.swiper.height = '220vp';
                this.selector.swiper.indicator = true;
                this.selector.swiper.contentHeight = '200vp';
                this.selector.swiper.gridColumns = '1fr 1fr 1fr 1fr';
                this.selector.swiper.gridRows = '1fr 1fr';
                this.selector.btn.marginTop = '10vp';
            }
            else if (this.hapList.length > 4) {
                this.selector.swiper.height = '200vp';
                this.selector.swiper.contentHeight = '200vp';
                this.selector.swiper.gridColumns = '1fr 1fr 1fr 1fr';
                this.selector.swiper.gridRows = '1fr 1fr';
            }
            else if (this.hapList.length > 3) {
                this.selector.swiper.gridColumns = '1fr 1fr 1fr 1fr';
            }
            else if (this.hapList.length > 2) {
                this.selector.swiper.gridColumns = '1fr 1fr 1fr';
            }
            else {
                ;
            }
            this.phoneShowData = new SelectorDataSource(globalThis.phoneShowHapList);
        }
        else {
            this.selector.swiper.gridColumns = '1fr 1fr 1fr 1fr';
            this.selector.swiper.gridRows = '1fr';
            this.phoneShowData = new SelectorDataSource(globalThis.signalRowPhoneShowHapList);
        }
    }
    initialRender() {
        this.observeComponentCreation((c16, d16) => {
            ViewStackProcessor.StartGetAccessRecordingFor(c16);
            Flex.create({ direction: FlexDirection.Column, alignItems: ItemAlign.Center });
            Flex.borderRadius(24);
            Flex.borderWidth(1);
            Flex.align(Alignment.Center);
            Flex.borderColor('#e9e9e9');
            Flex.backgroundColor('#ffffff');
            Flex.width('100%');
            Flex.height('100%');
            if (!d16) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((a16, b16) => {
            ViewStackProcessor.StartGetAccessRecordingFor(a16);
            Flex.create({ direction: FlexDirection.Column, alignItems: ItemAlign.Start });
            Flex.height(100);
            if (!b16) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((y15, z15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(y15);
            Text.create({ "id": 33554464, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontSize(22);
            Text.fontWeight(FontWeight.Medium);
            Text.textAlign(TextAlign.Start);
            Text.margin({ top: 25, left: 20 });
            if (!z15) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Flex.pop();
        this.observeComponentCreation((w15, x15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(w15);
            Flex.create({ direction: FlexDirection.Column, alignItems: ItemAlign.Center });
            if (!x15) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((u15, v15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(u15);
            Swiper.create();
            Swiper.indicatorStyle({ color: '#bebdc0', selectedColor: '#ff326Ce9', size: 4 });
            Swiper.align(Alignment.Center);
            Swiper.width('90%');
            Swiper.indicator(this.phoneShowData.totalCount() > 1);
            if (!v15) {
                Swiper.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        {
            const m14 = p14 => {
                const q14 = p14;
                this.observeComponentCreation((s15, t15) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(s15);
                    Grid.create();
                    Grid.columnsTemplate(this.selector.swiper.gridColumns);
                    Grid.rowsTemplate(this.selector.swiper.gridRows);
                    Grid.columnsGap(12);
                    Grid.rowsGap(12);
                    Grid.margin({ top: 10 });
                    Grid.height(300);
                    if (!t15) {
                        Grid.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                this.observeComponentCreation((r14, s14) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(r14);
                    ForEach.create();
                    const t14 = v14 => {
                        const w14 = v14;
                        {
                            const x14 = (q15, r15) => {
                                ViewStackProcessor.StartGetAccessRecordingFor(q15);
                                y14(q15, r15);
                                if (!r15) {
                                    GridItem.pop();
                                }
                                ViewStackProcessor.StopGetAccessRecording();
                            };
                            const y14 = (o15, p15) => {
                                GridItem.create(() => { }, false);
                                GridItem.margin({ top: 10 });
                                GridItem.align(Alignment.Center);
                            };
                            const z14 = () => {
                                this.observeComponentCreation(x14);
                                this.observeComponentCreation((m15, n15) => {
                                    ViewStackProcessor.StartGetAccessRecordingFor(m15);
                                    Flex.create({ direction: FlexDirection.Column, alignItems: ItemAlign.Center });
                                    Flex.height(180);
                                    Flex.onClick(() => {
                                        this.onSelectApp(w14);
                                    });
                                    if (!n15) {
                                        Flex.pop();
                                    }
                                    ViewStackProcessor.StopGetAccessRecording();
                                });
                                this.observeComponentCreation((g15, h15) => {
                                    ViewStackProcessor.StartGetAccessRecordingFor(g15);
                                    If.create();
                                    if (w14.split('#')[3] != '') {
                                        this.ifElseBranchUpdateFunction(0, () => {
                                            this.observeComponentCreation((k15, l15) => {
                                                ViewStackProcessor.StartGetAccessRecordingFor(k15);
                                                If.create();
                                                if (w14.split('#')[3].indexOf('image/json') > -1) {
                                                    this.ifElseBranchUpdateFunction(0, () => {
                                                        this.elementIconCombine.bind(this)(globalThis.jsonIconMap.get(w14.split('#')[0] + ':' +
                                                            w14.split('#')[4] + ':' + w14.split('#')[1] + ':' + w14.split('#')[7]), w14.split('#')[7]);
                                                    });
                                                }
                                                else {
                                                    this.ifElseBranchUpdateFunction(1, () => {
                                                        this.elementIconCombine.bind(this)(w14.split('#')[3], w14.split('#')[7]);
                                                    });
                                                }
                                                if (!l15) {
                                                    If.pop();
                                                }
                                                ViewStackProcessor.StopGetAccessRecording();
                                            });
                                            If.pop();
                                        });
                                    }
                                    else {
                                        this.ifElseBranchUpdateFunction(1, () => {
                                            this.observeComponentCreation((i15, j15) => {
                                                ViewStackProcessor.StartGetAccessRecordingFor(i15);
                                                Image.create({ "id": 33554433, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
                                                Image.height(60);
                                                Image.width(60);
                                                Image.alignSelf(ItemAlign.Center);
                                                if (!j15) {
                                                    Image.pop();
                                                }
                                                ViewStackProcessor.StopGetAccessRecording();
                                            });
                                        });
                                    }
                                    if (!h15) {
                                        If.pop();
                                    }
                                    ViewStackProcessor.StopGetAccessRecording();
                                });
                                If.pop();
                                this.observeComponentCreation((a15, b15) => {
                                    ViewStackProcessor.StartGetAccessRecordingFor(a15);
                                    If.create();
                                    if (w14.split('#')[2] != '') {
                                        this.ifElseBranchUpdateFunction(0, () => {
                                            this.observeComponentCreation((e15, f15) => {
                                                ViewStackProcessor.StartGetAccessRecordingFor(e15);
                                                Text.create(w14.split('#')[2]);
                                                Text.fontSize(16);
                                                Text.width(80);
                                                Text.height(23);
                                                Text.textAlign(TextAlign.Center);
                                                Text.alignSelf(ItemAlign.Center);
                                                Text.textOverflow({ overflow: TextOverflow.Ellipsis });
                                                if (!f15) {
                                                    Text.pop();
                                                }
                                                ViewStackProcessor.StopGetAccessRecording();
                                            });
                                            Text.pop();
                                        });
                                    }
                                    else {
                                        this.ifElseBranchUpdateFunction(1, () => {
                                            this.observeComponentCreation((c15, d15) => {
                                                ViewStackProcessor.StartGetAccessRecordingFor(c15);
                                                Text.create(w14.split('#')[0]);
                                                Text.fontSize(16);
                                                Text.width(80);
                                                Text.height(23);
                                                Text.textAlign(TextAlign.Center);
                                                Text.alignSelf(ItemAlign.Center);
                                                Text.textOverflow({ overflow: TextOverflow.Ellipsis });
                                                if (!d15) {
                                                    Text.pop();
                                                }
                                                ViewStackProcessor.StopGetAccessRecording();
                                            });
                                            Text.pop();
                                        });
                                    }
                                    if (!b15) {
                                        If.pop();
                                    }
                                    ViewStackProcessor.StopGetAccessRecording();
                                });
                                If.pop();
                                Flex.pop();
                                GridItem.pop();
                            };
                            z14();
                        }
                    };
                    this.forEachUpdateFunction(r14, q14, t14, u14 => u14, false, false);
                    if (!s14) {
                        ForEach.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                ForEach.pop();
                Grid.pop();
            };
            const n14 = o14 => o14;
            LazyForEach.create("1", this, this.phoneShowData, m14, n14);
            LazyForEach.pop();
        }
        Swiper.pop();
        Flex.pop();
        this.observeComponentCreation((k14, l14) => {
            ViewStackProcessor.StartGetAccessRecordingFor(k14);
            Flex.create({ alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center });
            Flex.width('90%');
            Flex.height(100);
            Flex.backgroundColor('#ffffff');
            Flex.borderRadius(20);
            Flex.margin({ bottom: 20, top: 10 });
            Flex.onClick(() => {
                globalThis.selectExtensionContext.terminateSelf();
            });
            if (!l14) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((i14, j14) => {
            ViewStackProcessor.StartGetAccessRecordingFor(i14);
            Text.create(({ "id": 33554452, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" }));
            Text.fontSize(20);
            Text.fontColor('#0A59F7');
            Text.fontWeight(FontWeight.Regular);
            if (!j14) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Flex.pop();
        Flex.pop();
    }
    rerender() {
        this.updateDirtyElements();
    }
    public __resetStateVarsOnReuse__Internal(h14: Object): void {
    }
}
ViewStackProcessor.StartGetAccessRecordingFor(ViewStackProcessor.AllocateNewElmetIdForNextComponent());
loadDocument(new SelectorPhoneDialog(undefined, {}));
ViewStackProcessor.StopGetAccessRecording();
