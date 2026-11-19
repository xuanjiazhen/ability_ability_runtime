import type bundleManager from "@ohos:bundle.bundleManager";
export type UIAbilityActionExecution = (storage?: LocalStorage) => void;
export type UIExtensionActionFunction = (storage?: LocalStorage) => UIExtensionInfo;
export type CustomActionFunction = () => void;
export class UIExtensionInfo {
    public readonly bundleName: string;
    public readonly abilityName: string;
    public readonly parameters?: Record<string, Object>;
    constructor(k: string, l: string, m?: Record<string, Object>) {
        this.bundleName = k;
        this.abilityName = l;
        this.parameters = m;
    }
}
export enum TargetType {
    UI_EXTENSION = 1,
    UI_ABILITY = 2,
    DEEPLINK = 3,
    SHOW_ALL_TARGET = 4
}
export enum PickerType {
    PHOTO_EDIT = 1,
    APP_SELECTOR = 2
}
export enum IntentType {
    PHOTO_EDIT = 1,
    GENERAL_APP_SELECTOR = 2
}
export class TargetAction {
    public readonly type: TargetType;
    public readonly actionExecution: UIAbilityActionExecution | UIExtensionActionFunction | CustomActionFunction | null;
    constructor(i: TargetType, j: UIAbilityActionExecution | UIExtensionActionFunction | CustomActionFunction | null) {
        this.type = i;
        this.actionExecution = j;
    }
}
export class TargetInfo {
    public readonly filterAbilityInfo: bundleManager.ExtensionAbilityInfo | null;
    public readonly targetAction: TargetAction | null;
    public readonly visible: boolean;
    constructor(f: bundleManager.ExtensionAbilityInfo | null, g: TargetAction | null, h: boolean = true) {
        this.filterAbilityInfo = f;
        this.targetAction = g;
        this.visible = h;
    }
}
export class AbilityInfoParam {
    public bundleName: string;
    public moduleName: string;
    public abilityName: string | null;
    public abilityIconId: number;
    public abilityLabelId: number;
    constructor(a: string, b: string, c: string, d: number, e: number) {
        this.bundleName = a;
        this.moduleName = b;
        this.abilityName = c;
        this.abilityIconId = d;
        this.abilityLabelId = e;
    }
}
export interface IconAndLabelContent {
    icon: PixelMap;
    label: string;
}
