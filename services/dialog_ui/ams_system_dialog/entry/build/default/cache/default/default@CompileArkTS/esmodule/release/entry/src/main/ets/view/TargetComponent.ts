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
    constructor(b30, c30, d30, e30 = -1, f30 = undefined, g30) {
        super(b30, d30, e30, g30);
        if (typeof f30 === "function") {
            this.paramsGenerator_ = f30;
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
        this.setInitiallyProvidedValue(c30);
        this.declareWatch("currentLang", this.refreshLabel);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(a30: TargetComponent_Params) {
        if (a30.targetIcon !== undefined) {
            this.targetIcon = a30.targetIcon;
        }
        if (a30.targetLabel !== undefined) {
            this.targetLabel = a30.targetLabel;
        }
        if (a30.targetInfo !== undefined) {
            this.targetInfo = a30.targetInfo;
        }
        if (a30.storage !== undefined) {
            this.storage = a30.storage;
        }
    }
    updateStateVars(z29: TargetComponent_Params) {
    }
    purgeVariableDependenciesOnElmtId(y29) {
        this.__level2Page.purgeDependencyOnElmtId(y29);
        this.__openUIExtension.purgeDependencyOnElmtId(y29);
        this.__uiExtensionInfo.purgeDependencyOnElmtId(y29);
        this.__currentColorMode.purgeDependencyOnElmtId(y29);
        this.__currentLang.purgeDependencyOnElmtId(y29);
        this.__targetIcon.purgeDependencyOnElmtId(y29);
        this.__targetLabel.purgeDependencyOnElmtId(y29);
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
    set level2Page(x29: boolean) {
        this.__level2Page.set(x29);
    }
    private __openUIExtension: ObservedPropertyAbstractPU<boolean>;
    get openUIExtension() {
        return this.__openUIExtension.get();
    }
    set openUIExtension(w29: boolean) {
        this.__openUIExtension.set(w29);
    }
    private __uiExtensionInfo: ObservedPropertyAbstractPU<UIExtensionInfo | null>;
    get uiExtensionInfo() {
        return this.__uiExtensionInfo.get();
    }
    set uiExtensionInfo(v29: UIExtensionInfo | null) {
        this.__uiExtensionInfo.set(v29);
    }
    private __currentColorMode: ObservedPropertyAbstractPU<number>;
    get currentColorMode() {
        return this.__currentColorMode.get();
    }
    set currentColorMode(u29: number) {
        this.__currentColorMode.set(u29);
    }
    private __currentLang: ObservedPropertyAbstractPU<string>;
    get currentLang() {
        return this.__currentLang.get();
    }
    set currentLang(t29: string) {
        this.__currentLang.set(t29);
    }
    private __targetIcon: ObservedPropertyObjectPU<DrawableDescriptor | ResourceStr | PixelMap | null>;
    get targetIcon() {
        return this.__targetIcon.get();
    }
    set targetIcon(s29: DrawableDescriptor | ResourceStr | PixelMap | null) {
        this.__targetIcon.set(s29);
    }
    private __targetLabel: ObservedPropertyObjectPU<ResourceStr>;
    get targetLabel() {
        return this.__targetLabel.get();
    }
    set targetLabel(r29: ResourceStr) {
        this.__targetLabel.set(r29);
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
                }, (q29: Object[]) => {
                    if (q29) {
                        this.targetIcon = q29[0] as DrawableDescriptor;
                        this.targetLabel = q29[1] as string;
                    }
                });
            }
            catch (p29) {
                console.error(TAG, `Get service label and icon exception: ${p29}`);
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
                }, (o29: Object[]) => {
                    if (o29) {
                        this.targetIcon = o29[0] as DrawableDescriptor;
                        this.targetLabel = o29[1] as string;
                    }
                });
            }
            catch (n29) {
                console.error(TAG, `Get label and icon exception: ${JSON.stringify(n29)}`);
            }
        }
        else if (this.targetInfo?.targetAction?.type === TargetType.SHOW_ALL_TARGET) {
            this.targetLabel = 'All';
            this.targetIcon = { "id": 33554481, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" };
        }
    }
    initialRender() {
        this.observeComponentCreation((l29, m29) => {
            ViewStackProcessor.StartGetAccessRecordingFor(l29);
            Column.create();
            Column.justifyContent(FlexAlign.Start);
            if (!m29) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((j29, k29) => {
            ViewStackProcessor.StartGetAccessRecordingFor(j29);
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
            if (!k29) {
                Button.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((h29, i29) => {
            ViewStackProcessor.StartGetAccessRecordingFor(h29);
            Stack.create();
            if (!i29) {
                Stack.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((f29, g29) => {
            ViewStackProcessor.StartGetAccessRecordingFor(f29);
            Column.create();
            if (!g29) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((d29, e29) => {
            ViewStackProcessor.StartGetAccessRecordingFor(d29);
            Stack.create();
            Stack.alignContent(Alignment.Top);
            if (!e29) {
                Stack.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((b29, c29) => {
            ViewStackProcessor.StartGetAccessRecordingFor(b29);
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
            if (!c29) {
                Image.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((x28, y28) => {
            ViewStackProcessor.StartGetAccessRecordingFor(x28);
            If.create();
            if (this.targetInfo?.targetAction?.type === TargetType.SHOW_ALL_TARGET) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation((z28, a29) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(z28);
                        Image.create(this.currentColorMode === ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT ? { "id": 33554481, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" } : { "id": 33554482, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "entry" });
                        Image.objectFit(ImageFit.Fill);
                        Image.height(24);
                        Image.width(24);
                        Image.margin({ top: 12 });
                        Image.draggable(false);
                        if (!a29) {
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
            if (!y28) {
                If.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        If.pop();
        Stack.pop();
        this.observeComponentCreation((v28, w28) => {
            ViewStackProcessor.StartGetAccessRecordingFor(v28);
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
            if (!w28) {
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
    public __resetStateVarsOnReuse__Internal(u28: Object): void {
        this.__reInitializeConsume__Internal("level2Page", "level2Page");
        this.__reInitializeConsume__Internal("openUIExtension", "openUIExtension");
        this.__reInitializeConsume__Internal("uiExtensionInfo", "uiExtensionInfo");
        this.__currentColorMode.resetSource();
        this.__currentLang.resetSource();
    }
}
ViewStackProcessor.StartGetAccessRecordingFor(ViewStackProcessor.AllocateNewElmetIdForNextComponent());
loadDocument(new TargetComponent(undefined, {}));
ViewStackProcessor.StopGetAccessRecording();
