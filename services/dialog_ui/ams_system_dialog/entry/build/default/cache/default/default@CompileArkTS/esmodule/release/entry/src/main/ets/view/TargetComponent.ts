if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
interface TargetComponent_Params {
    level2Page?: boolean;
    openUIExtension?: boolean;
    uiExtensionInfo?: UIExtensionInfo | null;
    currentColorMode?: number;
    currentLang?: string;
    targetIcon?: DrawableDescriptor | ResourceStr | PixelMap | null;
    targetLabel?: ResourceStr;
    targetInfo?: TargetInfo;
    storage?: LocalStorage | null;
}
import { TargetType } from "@bundle:com.ohos.amsdialog/entry/ets/data/Model";
import type { TargetInfo, UIAbilityActionExecution, UIExtensionActionFunction, UIExtensionInfo } from "@bundle:com.ohos.amsdialog/entry/ets/data/Model";
import ConfigurationConstant from "@ohos:app.ability.ConfigurationConstant";
import { AbilityInfoUtils } from "@bundle:com.ohos.amsdialog/entry/ets/utils/AbilityInfoUtils";
import type UIExtensionAbility from "@ohos:app.ability.UIExtensionAbility";
import { localStorageKeyConstants } from "@bundle:com.ohos.amsdialog/entry/ets/utils/Constants";
const TAG = 'SelectorExtensionAbility.TargetComponent';
export class TargetComponent extends ViewPU {
    constructor(q29, r29, s29, t29 = -1, u29 = undefined, v29) {
        super(q29, s29, t29, v29);
        if (typeof u29 === "function") {
            this.paramsGenerator_ = u29;
        }
        this.__level2Page = this.initializeConsume("level2Page", "level2Page");
        this.__openUIExtension = this.initializeConsume("openUIExtension", "openUIExtension");
        this.__uiExtensionInfo = this.initializeConsume("uiExtensionInfo", "uiExtensionInfo");
        this.__currentColorMode = this.createStorageProp('currentColorMode', ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT, "currentColorMode");
        this.__currentLang = this.createStorageProp('currentLanguage', '', "currentLang");
        this.__targetIcon = new ObservedPropertyObjectPU(null, this, "targetIcon");
        this.__targetLabel = new ObservedPropertyObjectPU('', this, "targetLabel");
        this.targetInfo = undefined;
        this.storage = null;
        this.setInitiallyProvidedValue(r29);
        this.declareWatch("currentLang", this.refreshLabel);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(p29: TargetComponent_Params) {
        if (p29.targetIcon !== undefined) {
            this.targetIcon = p29.targetIcon;
        }
        if (p29.targetLabel !== undefined) {
            this.targetLabel = p29.targetLabel;
        }
        if (p29.targetInfo !== undefined) {
            this.targetInfo = p29.targetInfo;
        }
        if (p29.storage !== undefined) {
            this.storage = p29.storage;
        }
    }
    updateStateVars(o29: TargetComponent_Params) {
    }
    purgeVariableDependenciesOnElmtId(n29) {
        this.__level2Page.purgeDependencyOnElmtId(n29);
        this.__openUIExtension.purgeDependencyOnElmtId(n29);
        this.__uiExtensionInfo.purgeDependencyOnElmtId(n29);
        this.__currentColorMode.purgeDependencyOnElmtId(n29);
        this.__currentLang.purgeDependencyOnElmtId(n29);
        this.__targetIcon.purgeDependencyOnElmtId(n29);
        this.__targetLabel.purgeDependencyOnElmtId(n29);
    }
    aboutToBeDeleted() {
        this.__level2Page.aboutToBeDeleted();
        this.__openUIExtension.aboutToBeDeleted();
        this.__uiExtensionInfo.aboutToBeDeleted();
        this.__currentColorMode.aboutToBeDeleted();
        this.__currentLang.aboutToBeDeleted();
        this.__targetIcon.aboutToBeDeleted();
        this.__targetLabel.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal();
    }
    private __level2Page: ObservedPropertyAbstractPU<boolean>;
    get level2Page() {
        return this.__level2Page.get();
    }
    set level2Page(m29: boolean) {
        this.__level2Page.set(m29);
    }
    private __openUIExtension: ObservedPropertyAbstractPU<boolean>;
    get openUIExtension() {
        return this.__openUIExtension.get();
    }
    set openUIExtension(l29: boolean) {
        this.__openUIExtension.set(l29);
    }
    private __uiExtensionInfo: ObservedPropertyAbstractPU<UIExtensionInfo | null>;
    get uiExtensionInfo() {
        return this.__uiExtensionInfo.get();
    }
    set uiExtensionInfo(k29: UIExtensionInfo | null) {
        this.__uiExtensionInfo.set(k29);
    }
    private __currentColorMode: ObservedPropertyAbstractPU<number>;
    get currentColorMode() {
        return this.__currentColorMode.get();
    }
    set currentColorMode(j29: number) {
        this.__currentColorMode.set(j29);
    }
    private __currentLang: ObservedPropertyAbstractPU<string>;
    get currentLang() {
        return this.__currentLang.get();
    }
    set currentLang(i29: string) {
        this.__currentLang.set(i29);
    }
    private __targetIcon: ObservedPropertyObjectPU<DrawableDescriptor | ResourceStr | PixelMap | null>;
    get targetIcon() {
        return this.__targetIcon.get();
    }
    set targetIcon(h29: DrawableDescriptor | ResourceStr | PixelMap | null) {
        this.__targetIcon.set(h29);
    }
    private __targetLabel: ObservedPropertyObjectPU<ResourceStr>;
    get targetLabel() {
        return this.__targetLabel.get();
    }
    set targetLabel(g29: ResourceStr) {
        this.__targetLabel.set(g29);
    }
    private targetInfo?: TargetInfo;
    private storage: LocalStorage | null;
    aboutToAppear() {
        this.getIconAndLabel();
    }
    refreshLabel() {
        if (this.targetInfo && this.targetInfo.filterAbilityInfo) {
            try {
                AbilityInfoUtils.getAbilityLabelAndIcon(this.storage?.get<UIExtensionAbility>(localStorageKeyConstants.EXTENSION_ABILITY)?.context, {
                    bundleName: this.targetInfo.filterAbilityInfo.bundleName,
                    moduleName: this.targetInfo.filterAbilityInfo.moduleName,
                    abilityName: this.targetInfo.filterAbilityInfo.name,
                    abilityIconId: this.targetInfo.filterAbilityInfo.iconId,
                    abilityLabelId: this.targetInfo.filterAbilityInfo.labelId
                }, (f29: Object[]) => {
                    if (f29) {
                        this.targetIcon = f29[0] as DrawableDescriptor;
                        this.targetLabel = f29[1] as string;
                    }
                });
            }
            catch (e29) {
                console.error(TAG, `Get service label and icon exception: ${e29}`);
            }
        }
        else if (this.targetInfo?.targetAction?.type === TargetType.SHOW_ALL_TARGET) {
            this.targetLabel = 'All';
            this.targetIcon = { "id": 33554481, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" };
        }
    }
    getIconAndLabel() {
        if (this.targetInfo && this.targetInfo.filterAbilityInfo) {
            try {
                AbilityInfoUtils.getAbilityLabelAndIcon(this.storage?.get<UIExtensionAbility>(localStorageKeyConstants.EXTENSION_ABILITY)?.context, {
                    bundleName: this.targetInfo.filterAbilityInfo.bundleName,
                    moduleName: this.targetInfo.filterAbilityInfo.moduleName,
                    abilityName: this.targetInfo.filterAbilityInfo.name,
                    abilityIconId: this.targetInfo.filterAbilityInfo.iconId,
                    abilityLabelId: this.targetInfo.filterAbilityInfo.labelId
                }, (d29: Object[]) => {
                    if (d29) {
                        this.targetIcon = d29[0] as DrawableDescriptor;
                        this.targetLabel = d29[1] as string;
                    }
                });
            }
            catch (c29) {
                console.error(TAG, `Get label and icon exception: ${JSON.stringify(c29)}`);
            }
        }
        else if (this.targetInfo?.targetAction?.type === TargetType.SHOW_ALL_TARGET) {
            this.targetLabel = 'All';
            this.targetIcon = { "id": 33554481, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" };
        }
    }
    initialRender() {
        this.observeComponentCreation((a29, b29) => {
            ViewStackProcessor.StartGetAccessRecordingFor(a29);
            Column.create();
            Column.justifyContent(FlexAlign.Start);
            if (!b29) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((y28, z28) => {
            ViewStackProcessor.StartGetAccessRecordingFor(y28);
            Button.createWithChild({ type: ButtonType.Normal, stateEffect: true });
            Button.align(Alignment.Top);
            Button.constraintSize({ minHeight: 104 });
            Button.padding({
                top: 8,
                bottom: 8
            });
            Button.borderRadius({ "id": 125829706, "type": 10002, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Button.backgroundColor(Color.Transparent);
            Button.onClick(() => {
                if (this.targetInfo?.targetAction?.type === TargetType.SHOW_ALL_TARGET) {
                    this.level2Page = true;
                    return;
                }
                if (!this.storage) {
                    return;
                }
                if (this.targetInfo?.targetAction?.type === TargetType.UI_EXTENSION) {
                    this.uiExtensionInfo =
                        (this.targetInfo.targetAction.actionExecution as UIExtensionActionFunction)(this.storage);
                    this.openUIExtension = true;
                    return;
                }
                if (this.targetInfo?.targetAction?.type === TargetType.UI_ABILITY ||
                    this.targetInfo?.targetAction?.type === TargetType.DEEPLINK) {
                    (this.targetInfo.targetAction.actionExecution as UIAbilityActionExecution)(this.storage);
                    return;
                }
            });
            ViewStackProcessor.visualState("pressed");
            Button.backgroundColor({ "id": 125829165, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            ViewStackProcessor.visualState();
            if (!z28) {
                Button.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((w28, x28) => {
            ViewStackProcessor.StartGetAccessRecordingFor(w28);
            Stack.create();
            if (!x28) {
                Stack.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((u28, v28) => {
            ViewStackProcessor.StartGetAccessRecordingFor(u28);
            Column.create();
            if (!v28) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((s28, t28) => {
            ViewStackProcessor.StartGetAccessRecordingFor(s28);
            Stack.create();
            Stack.alignContent(Alignment.Top);
            if (!t28) {
                Stack.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((q28, r28) => {
            ViewStackProcessor.StartGetAccessRecordingFor(q28);
            Image.create(this.targetInfo?.targetAction?.type === TargetType.SHOW_ALL_TARGET ? '' : this.targetIcon);
            Image.alt({ "id": 125829156, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Image.objectFit(ImageFit.Auto);
            Image.backgroundColor(this.targetInfo?.targetAction?.type === TargetType.SHOW_ALL_TARGET ||
                this.targetIcon ? { "id": 125829555, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" } : { "id": 125829156, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Image.height(48);
            Image.width(48);
            Image.borderRadius(10);
            Image.draggable(false);
            Image.margin({
                left: 16,
                right: 16
            });
            if (!r28) {
                Image.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((m28, n28) => {
            ViewStackProcessor.StartGetAccessRecordingFor(m28);
            If.create();
            if (this.targetInfo?.targetAction?.type === TargetType.SHOW_ALL_TARGET) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation((o28, p28) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(o28);
                        Image.create(this.currentColorMode === ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT ? { "id": 33554481, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" } : { "id": 33554482, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
                        Image.objectFit(ImageFit.Fill);
                        Image.height(24);
                        Image.width(24);
                        Image.margin({ top: 12 });
                        Image.draggable(false);
                        if (!p28) {
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
            if (!n28) {
                If.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        If.pop();
        Stack.pop();
        this.observeComponentCreation((k28, l28) => {
            ViewStackProcessor.StartGetAccessRecordingFor(k28);
            Text.create(this.targetLabel);
            Text.fontSize({ "id": 125829686, "type": 10002, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.fontColor({ "id": 125829210, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
            Text.textAlign(TextAlign.Center);
            Text.align(Alignment.Top);
            Text.fontWeight(FontWeight.Regular);
            Text.width(64);
            Text.maxLines(2);
            Text.textOverflow({ overflow: TextOverflow.Ellipsis });
            Text.margin({ top: 8 });
            if (!l28) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Column.pop();
        Stack.pop();
        Button.pop();
        Column.pop();
    }
    rerender() {
        this.updateDirtyElements();
    }
}
ViewStackProcessor.StartGetAccessRecordingFor(ViewStackProcessor.AllocateNewElmetIdForNextComponent());
loadDocument(new TargetComponent(undefined, {}));
ViewStackProcessor.StopGetAccessRecording();
