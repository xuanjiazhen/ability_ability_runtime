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
    constructor(a17: string[]) {
        this.list = a17;
    }
    totalCount(): number {
        return this.list.length;
    }
    getData(z16: number): string {
        return this.list[z16];
    }
    registerDataChangeListener(y16: DataChangeListener): void {
        this.listener = y16;
    }
    unregisterDataChangeListener() {
    }
}
class SelectorPhoneDialog extends ViewPU {
    constructor(s16, t16, u16, v16 = -1, w16 = undefined, x16) {
        super(s16, u16, v16, x16);
        if (typeof w16 === "function") {
            this.paramsGenerator_ = w16;
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
        this.setInitiallyProvidedValue(t16);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(r16: SelectorPhoneDialog_Params) {
        if (r16.phoneShowData !== undefined) {
            this.phoneShowData = r16.phoneShowData;
        }
        if (r16.selector !== undefined) {
            this.selector = r16.selector;
        }
        if (r16.hapList !== undefined) {
            this.hapList = r16.hapList;
        }
        if (r16.TAG !== undefined) {
            this.TAG = r16.TAG;
        }
    }
    updateStateVars(q16: SelectorPhoneDialog_Params) {
    }
    purgeVariableDependenciesOnElmtId(p16) {
        this.__oversizeHeight.purgeDependencyOnElmtId(p16);
        this.__selector.purgeDependencyOnElmtId(p16);
        this.__hapList.purgeDependencyOnElmtId(p16);
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
    set oversizeHeight(o16: string) {
        this.__oversizeHeight.set(o16);
    }
    private __selector: ObservedPropertyObjectPU<SelectorPhone>;
    get selector() {
        return this.__selector.get();
    }
    set selector(n16: SelectorPhone) {
        this.__selector.set(n16);
    }
    private __hapList: ObservedPropertyObjectPU<HapInfo[]>;
    get hapList() {
        return this.__hapList.get();
    }
    set hapList(m16: HapInfo[]) {
        this.__hapList.set(m16);
    }
    private TAG;
    elementIconCombine(b16: string, c16: string, d16 = null) {
        this.observeComponentCreation((k16, l16) => {
            ViewStackProcessor.StartGetAccessRecordingFor(k16);
            RelativeContainer.create();
            RelativeContainer.height(60);
            RelativeContainer.width(60);
            RelativeContainer.alignSelf(ItemAlign.Center);
            if (!l16) {
                RelativeContainer.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((i16, j16) => {
            ViewStackProcessor.StartGetAccessRecordingFor(i16);
            Image.create(b16);
            Image.id('iconLab');
            Image.height(60);
            Image.width(60);
            Image.alignSelf(ItemAlign.Center);
            if (!j16) {
                Image.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((e16, f16) => {
            ViewStackProcessor.StartGetAccessRecordingFor(e16);
            If.create();
            if (c16 !== '0') {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation((g16, h16) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(g16);
                        Image.create({ "id": -1, "type": -1, params: ['app.media.app_icon_clone_index_' + c16], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
                        Image.height(18);
                        Image.width(18);
                        Image.alignRules({
                            end: { anchor: 'iconLab', align: HorizontalAlign.End },
                            bottom: { anchor: 'iconLab', align: VerticalAlign.Bottom }
                        });
                        if (!h16) {
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
            if (!f16) {
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
    onSelectApp(y15) {
        console.log(this.TAG, 'onSelectApp - start ');
        globalThis.abilityWant.bundleName = y15.split('#')[0];
        globalThis.abilityWant.abilityName = y15.split('#')[1];
        globalThis.abilityWant.moduleName = y15.split('#')[4];
        globalThis.abilityWant.parameters['ohos.extra.param.key.appCloneIndex'] = Number(y15.split('#')[7]);
        globalThis.selectExtensionContext.startAbilityAsCaller(globalThis.abilityWant, (z15, a16) => {
            if (a16) {
                console.error(this.TAG + ' startAbility finish, error: ' + JSON.stringify(a16));
                return;
            }
            console.log(this.TAG + ' startAbility finish, data: ' + JSON.stringify(z15));
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
        this.observeComponentCreation((w15, x15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(w15);
            Flex.create({ direction: FlexDirection.Column, alignItems: ItemAlign.Center });
            Flex.borderRadius(24);
            Flex.borderWidth(1);
            Flex.align(Alignment.Center);
            Flex.borderColor('#e9e9e9');
            Flex.backgroundColor('#ffffff');
            Flex.width('100%');
            Flex.height('100%');
            if (!x15) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((u15, v15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(u15);
            Flex.create({ direction: FlexDirection.Column, alignItems: ItemAlign.Start });
            Flex.height(100);
            if (!v15) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((s15, t15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(s15);
            Text.create({ "id": 33554464, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontSize(22);
            Text.fontWeight(FontWeight.Medium);
            Text.textAlign(TextAlign.Start);
            Text.margin({ top: 25, left: 20 });
            if (!t15) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Flex.pop();
        this.observeComponentCreation((q15, r15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(q15);
            Flex.create({ direction: FlexDirection.Column, alignItems: ItemAlign.Center });
            if (!r15) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((o15, p15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(o15);
            Swiper.create();
            Swiper.indicatorStyle({ color: '#bebdc0', selectedColor: '#ff326Ce9', size: 4 });
            Swiper.align(Alignment.Center);
            Swiper.width('90%');
            Swiper.indicator(this.phoneShowData.totalCount() > 1);
            if (!p15) {
                Swiper.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        {
            const g14 = j14 => {
                const k14 = j14;
                this.observeComponentCreation((m15, n15) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(m15);
                    Grid.create();
                    Grid.columnsTemplate(this.selector.swiper.gridColumns);
                    Grid.rowsTemplate(this.selector.swiper.gridRows);
                    Grid.columnsGap(12);
                    Grid.rowsGap(12);
                    Grid.margin({ top: 10 });
                    Grid.height(300);
                    if (!n15) {
                        Grid.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                this.observeComponentCreation((l14, m14) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(l14);
                    ForEach.create();
                    const n14 = p14 => {
                        const q14 = p14;
                        {
                            const r14 = (k15, l15) => {
                                ViewStackProcessor.StartGetAccessRecordingFor(k15);
                                s14(k15, l15);
                                if (!l15) {
                                    GridItem.pop();
                                }
                                ViewStackProcessor.StopGetAccessRecording();
                            };
                            const s14 = (i15, j15) => {
                                GridItem.create(() => { }, false);
                                GridItem.margin({ top: 10 });
                                GridItem.align(Alignment.Center);
                            };
                            const t14 = () => {
                                this.observeComponentCreation(r14);
                                this.observeComponentCreation((g15, h15) => {
                                    ViewStackProcessor.StartGetAccessRecordingFor(g15);
                                    Flex.create({ direction: FlexDirection.Column, alignItems: ItemAlign.Center });
                                    Flex.height(180);
                                    Flex.onClick(() => {
                                        this.onSelectApp(q14);
                                    });
                                    if (!h15) {
                                        Flex.pop();
                                    }
                                    ViewStackProcessor.StopGetAccessRecording();
                                });
                                this.observeComponentCreation((a15, b15) => {
                                    ViewStackProcessor.StartGetAccessRecordingFor(a15);
                                    If.create();
                                    if (q14.split('#')[3] != '') {
                                        this.ifElseBranchUpdateFunction(0, () => {
                                            this.observeComponentCreation((e15, f15) => {
                                                ViewStackProcessor.StartGetAccessRecordingFor(e15);
                                                If.create();
                                                if (q14.split('#')[3].indexOf('image/json') > -1) {
                                                    this.ifElseBranchUpdateFunction(0, () => {
                                                        this.elementIconCombine.bind(this)(globalThis.jsonIconMap.get(q14.split('#')[0] + ':' +
                                                            q14.split('#')[4] + ':' + q14.split('#')[1] + ':' + q14.split('#')[7]), q14.split('#')[7]);
                                                    });
                                                }
                                                else {
                                                    this.ifElseBranchUpdateFunction(1, () => {
                                                        this.elementIconCombine.bind(this)(q14.split('#')[3], q14.split('#')[7]);
                                                    });
                                                }
                                                if (!f15) {
                                                    If.pop();
                                                }
                                                ViewStackProcessor.StopGetAccessRecording();
                                            });
                                            If.pop();
                                        });
                                    }
                                    else {
                                        this.ifElseBranchUpdateFunction(1, () => {
                                            this.observeComponentCreation((c15, d15) => {
                                                ViewStackProcessor.StartGetAccessRecordingFor(c15);
                                                Image.create({ "id": 33554433, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
                                                Image.height(60);
                                                Image.width(60);
                                                Image.alignSelf(ItemAlign.Center);
                                                if (!d15) {
                                                    Image.pop();
                                                }
                                                ViewStackProcessor.StopGetAccessRecording();
                                            });
                                        });
                                    }
                                    if (!b15) {
                                        If.pop();
                                    }
                                    ViewStackProcessor.StopGetAccessRecording();
                                });
                                If.pop();
                                this.observeComponentCreation((u14, v14) => {
                                    ViewStackProcessor.StartGetAccessRecordingFor(u14);
                                    If.create();
                                    if (q14.split('#')[2] != '') {
                                        this.ifElseBranchUpdateFunction(0, () => {
                                            this.observeComponentCreation((y14, z14) => {
                                                ViewStackProcessor.StartGetAccessRecordingFor(y14);
                                                Text.create(q14.split('#')[2]);
                                                Text.fontSize(16);
                                                Text.width(80);
                                                Text.height(23);
                                                Text.textAlign(TextAlign.Center);
                                                Text.alignSelf(ItemAlign.Center);
                                                Text.textOverflow({ overflow: TextOverflow.Ellipsis });
                                                if (!z14) {
                                                    Text.pop();
                                                }
                                                ViewStackProcessor.StopGetAccessRecording();
                                            });
                                            Text.pop();
                                        });
                                    }
                                    else {
                                        this.ifElseBranchUpdateFunction(1, () => {
                                            this.observeComponentCreation((w14, x14) => {
                                                ViewStackProcessor.StartGetAccessRecordingFor(w14);
                                                Text.create(q14.split('#')[0]);
                                                Text.fontSize(16);
                                                Text.width(80);
                                                Text.height(23);
                                                Text.textAlign(TextAlign.Center);
                                                Text.alignSelf(ItemAlign.Center);
                                                Text.textOverflow({ overflow: TextOverflow.Ellipsis });
                                                if (!x14) {
                                                    Text.pop();
                                                }
                                                ViewStackProcessor.StopGetAccessRecording();
                                            });
                                            Text.pop();
                                        });
                                    }
                                    if (!v14) {
                                        If.pop();
                                    }
                                    ViewStackProcessor.StopGetAccessRecording();
                                });
                                If.pop();
                                Flex.pop();
                                GridItem.pop();
                            };
                            t14();
                        }
                    };
                    this.forEachUpdateFunction(l14, k14, n14, o14 => o14, false, false);
                    if (!m14) {
                        ForEach.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                ForEach.pop();
                Grid.pop();
            };
            const h14 = i14 => i14;
            LazyForEach.create("1", this, this.phoneShowData, g14, h14);
            LazyForEach.pop();
        }
        Swiper.pop();
        Flex.pop();
        this.observeComponentCreation((e14, f14) => {
            ViewStackProcessor.StartGetAccessRecordingFor(e14);
            Flex.create({ alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center });
            Flex.width('90%');
            Flex.height(100);
            Flex.backgroundColor('#ffffff');
            Flex.borderRadius(20);
            Flex.margin({ bottom: 20, top: 10 });
            Flex.onClick(() => {
                globalThis.selectExtensionContext.terminateSelf();
            });
            if (!f14) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((c14, d14) => {
            ViewStackProcessor.StartGetAccessRecordingFor(c14);
            Text.create(({ "id": 33554452, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" }));
            Text.fontSize(20);
            Text.fontColor('#0A59F7');
            Text.fontWeight(FontWeight.Regular);
            if (!d14) {
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
}
ViewStackProcessor.StartGetAccessRecordingFor(ViewStackProcessor.AllocateNewElmetIdForNextComponent());
loadDocument(new SelectorPhoneDialog(undefined, {}));
ViewStackProcessor.StopGetAccessRecording();
