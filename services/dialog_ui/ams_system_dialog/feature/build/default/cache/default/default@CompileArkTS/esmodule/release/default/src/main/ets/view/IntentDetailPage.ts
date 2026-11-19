if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
interface IntentDetailPage_Params {
    articlePathStack?: NavPathStack;
    intentDetail?: IntentInfo | null;
    intentDetailMap?: Map<string, string> | null;
    intentDetailEntries?: Array<[
        string,
        string | string[] | object
    ]> | [
    ];
    intentParameters?: object | null;
    scroller?: Scroller;
    required?: string[];
    formData?: FormDataType;
    properties?: Record<string, propertiesInfo>;
    formFields?: FormField[];
    abilityName?: string;
    uri?: string;
    formName?: string;
    countArr?: number[];
    result?: string;
    isShow?: boolean;
    isShowEntry2?: boolean;
    isShowForm?: boolean;
    want?: Want;
    inputType?: Record<string, string>;
    InputTypeData?: Record<string, InputType>;
}
import type { IntentInfo } from './IntentContent';
import { DetailItem } from "@bundle:com.ohos.amsdialog/default/ets/view/DetailItem";
import insightIntent from "@ohos:app.ability.insightIntent";
import insightIntentDriver from "@ohos:app.ability.insightIntentDriver";
import bundleManager from "@ohos:bundle.bundleManager";
import type common from "@ohos:app.ability.common";
import hilog from "@ohos:hilog";
import { format, jsClone, addArrayParams, removeArrayParams, removeFormData } from "@bundle:com.ohos.amsdialog/default/ets/view/format";
const TAG: string = 'IntentDetailPage';
const DOMAIN: number = 0xFF00;
interface ItemsInfo {
    type: string;
}
interface propertiesInfo {
    name: string;
    type: string;
    description?: string;
    enum?: Array<string | number>;
    subItem?: propertiesInfo;
    maxLength?: number;
    default?: number | Resource;
    required?: boolean;
    items?: ItemsInfo;
}
interface FormField {
    key: string;
    fatherKey?: string;
    name: string;
    type: string;
    fatherType?: string;
    inputType: string;
    description?: string;
    options?: Array<string | number>;
    subProperties?: FormField[] | FormField[][];
    maxLength?: number;
    default?: number | Resource;
    value?: string | number;
    index?: number;
    required?: boolean;
}
interface UIExtensionInfo {
    type: string;
    name: string;
}
type ObjVal = string | number | null;
type ObjType = Record<string, object>;
type ArrType = ObjType[];
type FormDataType = Record<string, ObjVal | ObjType | ArrType>;
function __UIExtensionComponent__extStyle(): void {
    UIExtensionComponent.width('100%');
    UIExtensionComponent.height('100%');
    UIExtensionComponent.borderRadius(20);
    UIExtensionComponent.shadow({
        radius: 60,
        color: '#33000000',
        offsetX: 0,
        offsetY: 10
    });
    UIExtensionComponent.clip(true);
}
export class IntentDetailPage extends ViewPU {
    constructor(q19, r19, s19, t19 = -1, u19 = undefined, v19) {
        super(q19, s19, t19, v19);
        if (typeof u19 === "function") {
            this.paramsGenerator_ = u19;
        }
        this.__articlePathStack = this.initializeConsume('articlePathStack', "articlePathStack");
        this.__intentDetail = new ObservedPropertyObjectPU(null, this, "intentDetail");
        this.__intentDetailMap = new ObservedPropertyObjectPU(null, this, "intentDetailMap");
        this.__intentDetailEntries = new ObservedPropertyObjectPU([], this, "intentDetailEntries");
        this.__intentParameters = new ObservedPropertyObjectPU(null, this, "intentParameters");
        this.scroller = new Scroller();
        this.__required = new ObservedPropertyObjectPU([], this, "required");
        this.__formData = new ObservedPropertyObjectPU({}, this, "formData");
        this.__properties = new ObservedPropertyObjectPU({}, this, "properties");
        this.__formFields = new ObservedPropertyObjectPU([], this, "formFields");
        this.__abilityName = new ObservedPropertySimplePU('', this, "abilityName");
        this.__uri = new ObservedPropertySimplePU('', this, "uri");
        this.__formName = new ObservedPropertySimplePU('', this, "formName");
        this.__countArr = new ObservedPropertyObjectPU([0], this, "countArr");
        this.__result = new ObservedPropertySimplePU('', this, "result");
        this.__isShow = new ObservedPropertySimplePU(false, this, "isShow");
        this.__isShowEntry2 = new ObservedPropertySimplePU(false, this, "isShowEntry2");
        this.__isShowForm = new ObservedPropertySimplePU(false, this, "isShowForm");
        this.__want = new ObservedPropertyObjectPU({
            bundleName: 'com.ohos.tools.myapplication',
            abilityName: 'EntryUIExtAbility',
            moduleName: 'entry',
            parameters: {
                'ability.want.params.uiExtensionType': 'share',
                'ohos.insightIntent.executeParam.name': 'TestIntentNameEntry',
                'ohos.insightIntent.executeParam.param': {}
            }
        }, this, "want");
        this.inputType = {
            'string': 'text',
            'number': 'number',
            'enum': 'select',
            'object': 'object',
            'array': 'array',
        };
        this.__InputTypeData = new ObservedPropertyObjectPU({
            'string': InputType.Normal,
            'number': InputType.Number
        }, this, "InputTypeData");
        this.addProvidedVar("InputTypeData", this.__InputTypeData, false);
        this.setInitiallyProvidedValue(r19);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(p19: IntentDetailPage_Params) {
        if (p19.intentDetail !== undefined) {
            this.intentDetail = p19.intentDetail;
        }
        if (p19.intentDetailMap !== undefined) {
            this.intentDetailMap = p19.intentDetailMap;
        }
        if (p19.intentDetailEntries !== undefined) {
            this.intentDetailEntries = p19.intentDetailEntries;
        }
        if (p19.intentParameters !== undefined) {
            this.intentParameters = p19.intentParameters;
        }
        if (p19.scroller !== undefined) {
            this.scroller = p19.scroller;
        }
        if (p19.required !== undefined) {
            this.required = p19.required;
        }
        if (p19.formData !== undefined) {
            this.formData = p19.formData;
        }
        if (p19.properties !== undefined) {
            this.properties = p19.properties;
        }
        if (p19.formFields !== undefined) {
            this.formFields = p19.formFields;
        }
        if (p19.abilityName !== undefined) {
            this.abilityName = p19.abilityName;
        }
        if (p19.uri !== undefined) {
            this.uri = p19.uri;
        }
        if (p19.formName !== undefined) {
            this.formName = p19.formName;
        }
        if (p19.countArr !== undefined) {
            this.countArr = p19.countArr;
        }
        if (p19.result !== undefined) {
            this.result = p19.result;
        }
        if (p19.isShow !== undefined) {
            this.isShow = p19.isShow;
        }
        if (p19.isShowEntry2 !== undefined) {
            this.isShowEntry2 = p19.isShowEntry2;
        }
        if (p19.isShowForm !== undefined) {
            this.isShowForm = p19.isShowForm;
        }
        if (p19.want !== undefined) {
            this.want = p19.want;
        }
        if (p19.inputType !== undefined) {
            this.inputType = p19.inputType;
        }
        if (p19.InputTypeData !== undefined) {
            this.InputTypeData = p19.InputTypeData;
        }
    }
    updateStateVars(o19: IntentDetailPage_Params) {
    }
    purgeVariableDependenciesOnElmtId(n19) {
        this.__articlePathStack.purgeDependencyOnElmtId(n19);
        this.__intentDetail.purgeDependencyOnElmtId(n19);
        this.__intentDetailMap.purgeDependencyOnElmtId(n19);
        this.__intentDetailEntries.purgeDependencyOnElmtId(n19);
        this.__intentParameters.purgeDependencyOnElmtId(n19);
        this.__required.purgeDependencyOnElmtId(n19);
        this.__formData.purgeDependencyOnElmtId(n19);
        this.__properties.purgeDependencyOnElmtId(n19);
        this.__formFields.purgeDependencyOnElmtId(n19);
        this.__abilityName.purgeDependencyOnElmtId(n19);
        this.__uri.purgeDependencyOnElmtId(n19);
        this.__formName.purgeDependencyOnElmtId(n19);
        this.__countArr.purgeDependencyOnElmtId(n19);
        this.__result.purgeDependencyOnElmtId(n19);
        this.__isShow.purgeDependencyOnElmtId(n19);
        this.__isShowEntry2.purgeDependencyOnElmtId(n19);
        this.__isShowForm.purgeDependencyOnElmtId(n19);
        this.__want.purgeDependencyOnElmtId(n19);
        this.__InputTypeData.purgeDependencyOnElmtId(n19);
    }
    aboutToBeDeleted() {
        this.__articlePathStack.aboutToBeDeleted();
        this.__intentDetail.aboutToBeDeleted();
        this.__intentDetailMap.aboutToBeDeleted();
        this.__intentDetailEntries.aboutToBeDeleted();
        this.__intentParameters.aboutToBeDeleted();
        this.__required.aboutToBeDeleted();
        this.__formData.aboutToBeDeleted();
        this.__properties.aboutToBeDeleted();
        this.__formFields.aboutToBeDeleted();
        this.__abilityName.aboutToBeDeleted();
        this.__uri.aboutToBeDeleted();
        this.__formName.aboutToBeDeleted();
        this.__countArr.aboutToBeDeleted();
        this.__result.aboutToBeDeleted();
        this.__isShow.aboutToBeDeleted();
        this.__isShowEntry2.aboutToBeDeleted();
        this.__isShowForm.aboutToBeDeleted();
        this.__want.aboutToBeDeleted();
        this.__InputTypeData.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal();
    }
    private __articlePathStack: ObservedPropertyAbstractPU<NavPathStack>;
    get articlePathStack() {
        return this.__articlePathStack.get();
    }
    set articlePathStack(m19: NavPathStack) {
        this.__articlePathStack.set(m19);
    }
    private __intentDetail: ObservedPropertyObjectPU<IntentInfo | null>;
    get intentDetail() {
        return this.__intentDetail.get();
    }
    set intentDetail(l19: IntentInfo | null) {
        this.__intentDetail.set(l19);
    }
    private __intentDetailMap: ObservedPropertyObjectPU<Map<string, string> | null>;
    get intentDetailMap() {
        return this.__intentDetailMap.get();
    }
    set intentDetailMap(k19: Map<string, string> | null) {
        this.__intentDetailMap.set(k19);
    }
    private __intentDetailEntries: ObservedPropertyObjectPU<Array<[
        string,
        string | string[] | object
    ]> | [
    ]>;
    get intentDetailEntries() {
        return this.__intentDetailEntries.get();
    }
    set intentDetailEntries(j19: Array<[
        string,
        string | string[] | object
    ]> | [
    ]) {
        this.__intentDetailEntries.set(j19);
    }
    private __intentParameters: ObservedPropertyObjectPU<object | null>;
    get intentParameters() {
        return this.__intentParameters.get();
    }
    set intentParameters(i19: object | null) {
        this.__intentParameters.set(i19);
    }
    private scroller: Scroller;
    private __required: ObservedPropertyObjectPU<string[]>;
    get required() {
        return this.__required.get();
    }
    set required(h19: string[]) {
        this.__required.set(h19);
    }
    private __formData: ObservedPropertyObjectPU<FormDataType>;
    get formData() {
        return this.__formData.get();
    }
    set formData(g19: FormDataType) {
        this.__formData.set(g19);
    }
    private __properties: ObservedPropertyObjectPU<Record<string, propertiesInfo>>;
    get properties() {
        return this.__properties.get();
    }
    set properties(f19: Record<string, propertiesInfo>) {
        this.__properties.set(f19);
    }
    private __formFields: ObservedPropertyObjectPU<FormField[]>;
    get formFields() {
        return this.__formFields.get();
    }
    set formFields(e19: FormField[]) {
        this.__formFields.set(e19);
    }
    private __abilityName: ObservedPropertySimplePU<string>;
    get abilityName() {
        return this.__abilityName.get();
    }
    set abilityName(d19: string) {
        this.__abilityName.set(d19);
    }
    private __uri: ObservedPropertySimplePU<string>;
    get uri() {
        return this.__uri.get();
    }
    set uri(c19: string) {
        this.__uri.set(c19);
    }
    private __formName: ObservedPropertySimplePU<string>;
    get formName() {
        return this.__formName.get();
    }
    set formName(b19: string) {
        this.__formName.set(b19);
    }
    private __countArr: ObservedPropertyObjectPU<number[]>;
    get countArr() {
        return this.__countArr.get();
    }
    set countArr(a19: number[]) {
        this.__countArr.set(a19);
    }
    private __result: ObservedPropertySimplePU<string>;
    get result() {
        return this.__result.get();
    }
    set result(z18: string) {
        this.__result.set(z18);
    }
    private __isShow: ObservedPropertySimplePU<boolean>;
    get isShow() {
        return this.__isShow.get();
    }
    set isShow(y18: boolean) {
        this.__isShow.set(y18);
    }
    private __isShowEntry2: ObservedPropertySimplePU<boolean>;
    get isShowEntry2() {
        return this.__isShowEntry2.get();
    }
    set isShowEntry2(x18: boolean) {
        this.__isShowEntry2.set(x18);
    }
    private __isShowForm: ObservedPropertySimplePU<boolean>;
    get isShowForm() {
        return this.__isShowForm.get();
    }
    set isShowForm(w18: boolean) {
        this.__isShowForm.set(w18);
    }
    private __want: ObservedPropertyObjectPU<Want>;
    get want() {
        return this.__want.get();
    }
    set want(v18: Want) {
        this.__want.set(v18);
    }
    private inputType: Record<string, string>;
    aboutToAppear(): void {
        this.intentDetail = this.articlePathStack.getParamByName('intentDetail')[0] as IntentInfo;
        this.intentParameters = this.intentDetail.parameters ?? null;
        this.intentDetailMap = new Map(Object.entries(this.intentDetail));
        this.intentDetailEntries = Array.from(this.intentDetailMap.entries());
        this.required = (this.getAttribute(this.intentDetail?.parameters, 'required') === '') ? [] :
            this.getAttribute(this.intentDetail?.parameters, 'required') as string[];
        this.abilityName = this.getAttribute(this.intentDetail?.subIntentInfo, 'abilityName') as string;
        if (this.intentDetail?.intentType === '@InsightIntentPage') {
            this.abilityName = this.getAttribute(this.intentDetail?.subIntentInfo, 'uiAbility') as string;
        }
        this.uri = this.getAttribute(this.intentDetail?.subIntentInfo, 'uri') as string;
        if (this.intentParameters) {
            this.properties = this.intentParameters['properties'];
            if (this.properties) {
                this.formFields = this.changeProperties(this.properties);
            }
        }
    }
    onBackPress() {
        let u18 = AppStorage.get('context') as common.UIAbilityContext;
        if (u18) {
            u18.terminateSelf();
        }
    }
    private __InputTypeData: ObservedPropertyObjectPU<Record<string, InputType>>;
    get InputTypeData() {
        return this.__InputTypeData.get();
    }
    set InputTypeData(t18: Record<string, InputType>) {
        this.__InputTypeData.set(t18);
    }
    getAttribute(r18: object | undefined, s18: string): string | string[] {
        if (r18 && Object.keys(r18).includes(s18)) {
            return r18[s18];
        }
        else {
            return '';
        }
    }
    async getUIExtensionType(d18: string, e18: string, f18: string): Promise<UIExtensionInfo> {
        hilog.info(DOMAIN, TAG, `getUIExtensionType start: bundleName=${d18}, moduleName=${e18}, abilityName=${f18}`);
        const g18 = await bundleManager.getBundleInfo(d18, bundleManager.BundleFlag.GET_BUNDLE_INFO_WITH_HAP_MODULE |
            bundleManager.BundleFlag.GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY);
        hilog.info(DOMAIN, TAG, 'getBundleInfo success: bundleName=%{public}s', g18.name);
        const h18 = g18.hapModulesInfo;
        if (!h18) {
            const q18 = `getUIExtensionType failed: hapModulesInfo is null for bundleName=${d18}`;
            hilog.error(DOMAIN, TAG, q18);
            throw new Error(q18);
        }
        for (let j18 = 0; j18 < h18.length; j18++) {
            const k18 = h18[j18];
            if (k18.name === e18) {
                const l18 = k18.extensionAbilitiesInfo;
                if (!l18) {
                    hilog.warn(DOMAIN, TAG, `extensionAbilitiesInfo is null for module: ${e18}`);
                    continue;
                }
                for (let m18 = 0; m18 < l18.length; m18++) {
                    const n18 = l18[m18];
                    const o18 = n18.extensionAbilityTypeName;
                    if (o18 && o18 !== '') {
                        hilog.info(DOMAIN, TAG, `getUIExtensionType success: found UIExtensionAbility ${n18.name}, type=${o18}`);
                        const p18: UIExtensionInfo = {
                            type: o18,
                            name: n18.name
                        };
                        return p18;
                    }
                }
            }
        }
        const i18 = `getUIExtensionType failed: UIExtensionAbility not found for bundleName=${d18}, moduleName=${e18}, abilityName=${f18}`;
        hilog.error(DOMAIN, TAG, i18);
        throw new Error(i18);
    }
    changeProperties(n17: Record<string, propertiesInfo>, o17?: string, p17?: string) {
        const q17: FormField[] = [];
        Object.keys(n17).forEach((r17: string, s17: number) => {
            const t17 = n17[r17];
            let u17 = r17;
            if (o17) {
                if (p17 && p17 === 'array') {
                    u17 = `${o17}.array0.${r17}`;
                }
                else {
                    u17 = `${o17}.${r17}`;
                }
            }
            const v17 = r17;
            const w17 = t17.type;
            const x17 = t17.enum ? 'select' : this.inputType[w17];
            let y17: FormField[] | FormField[][] = [];
            if (w17 === 'object' && Object.keys(t17).includes('properties')) {
                const c18 = this.findProperties(t17);
                y17 = this.changeProperties(c18, r17) as FormField[];
            }
            else if (w17 === 'array' && Object.keys(t17).includes('items')) {
                if (s17 === 0) {
                    this.countArr = [0];
                }
                let b18 = this.findProperties(t17);
                if (Object.keys(b18).length === 0) {
                    b18 = { 'items': { name: '', type: t17.items?.type ?? 'string' } as propertiesInfo };
                }
                y17 = [this.changeProperties(b18, r17, 'array')] as FormField[][];
            }
            const z17 = t17.description;
            const a18: FormField = {
                key: u17,
                name: v17,
                type: w17,
                inputType: x17,
                description: z17,
            };
            if (t17.enum) {
                a18.options = t17.enum;
            }
            if (t17.maxLength) {
                a18.maxLength = t17.maxLength;
            }
            if (t17.default) {
                a18.default = t17.default;
            }
            if (o17) {
                a18.fatherKey = o17;
            }
            if (p17) {
                a18.fatherType = p17;
                a18.index = 0;
            }
            if (this.required?.includes(r17)) {
                a18.required = true;
            }
            if (y17.length > 0) {
                if (p17 && p17 === 'array') {
                    a18.subProperties = y17;
                }
                else {
                    a18.subProperties = y17;
                }
            }
            q17.push(a18);
        });
        return q17;
    }
    findProperties(k17: object): Record<string, propertiesInfo> {
        let l17: Record<string, propertiesInfo> = {};
        Object.keys(k17).forEach((m17: string) => {
            if (m17 === 'properties') {
                l17 = k17['properties'];
            }
            else if (typeof k17[m17] === 'object' && m17 !== 'propertyNames') {
                l17 = this.findProperties(k17[m17]);
            }
        });
        return l17;
    }
    async executeLinkIntent(f17: Record<string, Object>) {
        let g17: insightIntentDriver.ExecuteParam = {
            bundleName: this.intentDetail?.bundleName ?? '',
            moduleName: this.intentDetail?.moduleName ?? '',
            insightIntentName: this.intentDetail?.intentName ?? '',
            insightIntentParam: f17,
            executeMode: insightIntent.ExecuteMode.UI_ABILITY_FOREGROUND,
            abilityName: this.abilityName
        };
        if (this.intentDetail?.intentType === '@InsightIntentLink') {
            g17.uris = [this.uri];
        }
        if (this.intentDetail?.intentType === '@InsightIntentLink' ||
            this.intentDetail?.intentType === '@InsightIntentPage') {
            g17.executeMode = insightIntent.ExecuteMode.UI_ABILITY_FOREGROUND;
        }
        if (this.intentDetail?.intentType === '@InsightIntentFunctionMethod') {
            g17.executeMode = insightIntent.ExecuteMode.UI_ABILITY_BACKGROUND;
        }
        if (this.intentDetail?.intentType === '@InsightIntentEntry') {
            g17.executeMode = this.intentDetail.subIntentInfo['executeMode'][0];
            hilog.info(DOMAIN, TAG, `InsightIntentEntry detected: intentName=${this.intentDetail.intentName}, executeMode=${g17.executeMode}, subIntentInfo=${JSON.stringify(this.intentDetail.subIntentInfo)}`);
        }
        if (this.intentDetail?.intentType === '@InsightIntentEntry' &&
            this.intentDetail.subIntentInfo['executeMode'] && this.intentDetail.subIntentInfo['executeMode'][0] === 2) {
            const j17 = await this.getUIExtensionType(g17.bundleName, g17.moduleName, g17.abilityName);
            hilog.info(DOMAIN, TAG, `IntentName: ${this.intentDetail.intentName}, uiExtensionType: ${j17.type}, uiExtensionName: ${j17.name}`);
            this.want = {
                bundleName: g17.bundleName,
                abilityName: g17.abilityName,
                moduleName: g17.moduleName,
                parameters: {
                    'ability.want.params.uiExtensionType': j17.type,
                    'ohos.insightIntent.executeParam.name': this.intentDetail.intentName,
                    'ohos.insightIntent.executeParam.param': g17.insightIntentParam,
                }
            };
            hilog.info(DOMAIN, TAG, `UIExtensionComponent want: ${JSON.stringify(this.want)}`);
            this.isShowEntry2 = true;
            this.isShow = true;
        }
        else if (this.intentDetail?.intentType === '@InsightIntentForm') {
            this.formName = this.getAttribute(this.intentDetail?.subIntentInfo, 'formName') as string;
            hilog.info(DOMAIN, TAG, `FormComponent: formName=${this.formName}`);
            this.isShow = true;
            this.isShowForm = true;
        }
        else {
            try {
                let i17: insightIntent.ExecuteResult = await insightIntentDriver.execute(g17);
                this.result = 'code: ' + i17.code + '\n' + 'result: ' + JSON.stringify(i17?.result, null, 2);
                if (this.result != '') {
                    this.isShow = true;
                }
            }
            catch (h17) {
                this.result = 'code: ' + JSON.stringify(h17.code) + '\n' + JSON.stringify(g17, null, 2);
                if (this.result != '') {
                    this.isShow = true;
                }
            }
        }
    }
    submitForm() {
        const c17 = format(this.formData);
        let d17: boolean = true;
        this.required?.forEach((e17: string) => {
            if (c17[e17] === undefined || c17[e17] === null) {
                if (d17) {
                    d17 = false;
                    hilog.error(DOMAIN, TAG, `submitForm validation failed: required field '${e17}' is missing`);
                }
            }
        });
        if (d17) {
            this.executeLinkIntent(c17 as Record<string, Object>);
        }
    }
    resultModule(n16 = null) {
        this.observeComponentCreation((a17, b17) => {
            ViewStackProcessor.StartGetAccessRecordingFor(a17);
            Column.create();
            Column.width('100%');
            Column.height('100%');
            if (!b17) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((o16, p16) => {
            ViewStackProcessor.StartGetAccessRecordingFor(o16);
            If.create();
            if (this.isShowEntry2) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation((y16, z16) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(y16);
                        Column.create();
                        Column.height('100%');
                        Column.width('100%');
                        Column.padding({
                            top: 28,
                            left: 22,
                            right: 22,
                            bottom: 48
                        });
                        if (!z16) {
                            Column.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    this.observeComponentCreation((w16, x16) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(w16);
                        UIExtensionComponent.create(this.want);
                        __UIExtensionComponent__extStyle();
                        if (!x16) {
                            UIExtensionComponent.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    Column.pop();
                });
            }
            else if (this.isShowForm) {
                this.ifElseBranchUpdateFunction(1, () => {
                    this.observeComponentCreation((u16, v16) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(u16);
                        Column.create();
                        Column.padding({
                            top: 28,
                            left: 22,
                            right: 22,
                            bottom: 48
                        });
                        Column.height('100%');
                        Column.width('100%');
                        if (!v16) {
                            Column.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    this.observeComponentCreation((s16, t16) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(s16);
                        FormComponent.create({
                            id: '0',
                            name: this.formName,
                            bundle: this.intentDetail?.bundleName,
                            ability: this.abilityName,
                            module: this.intentDetail?.moduleName,
                        });
                        FormComponent.width('100%');
                        FormComponent.height('80%');
                        FormComponent.borderRadius(20);
                        FormComponent.shadow({
                            radius: 60,
                            color: '#33000000',
                            offsetX: 0,
                            offsetY: 10
                        });
                        FormComponent.clip(true);
                        if (!t16) {
                            FormComponent.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    Column.pop();
                });
            }
            else {
                this.ifElseBranchUpdateFunction(2, () => {
                    this.observeComponentCreation((q16, r16) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(q16);
                        Text.create(this.result);
                        Text.width('100%');
                        Text.textAlign(TextAlign.Start);
                        Text.fontSize(14);
                        Text.fontColor({ "id": 16777249, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                        Text.lineHeight(18);
                        Text.padding({
                            top: 20,
                            left: 28,
                            right: 28,
                            bottom: 48
                        });
                        if (!r16) {
                            Text.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    Text.pop();
                });
            }
            if (!p16) {
                If.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        If.pop();
        Column.pop();
    }
    buildTextInput(w15: FormField, x15 = null) {
        this.observeComponentCreation((l16, m16) => {
            ViewStackProcessor.StartGetAccessRecordingFor(l16);
            Flex.create({ direction: FlexDirection.Row, alignItems: ItemAlign.Start });
            Flex.margin({ bottom: 12 });
            if (!m16) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((j16, k16) => {
            ViewStackProcessor.StartGetAccessRecordingFor(j16);
            Text.create(w15.name);
            Text.fontSize(16);
            Text.fontColor({ "id": 16777247, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.width(165);
            Text.height(40);
            Text.borderRadius(20);
            Text.padding({ right: 16, left: 16 });
            Text.margin({ right: 8 });
            Text.backgroundColor({ "id": 16777239, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.textOverflow({ overflow: TextOverflow.Ellipsis });
            Text.maxLines(1);
            if (!k16) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((h16, i16) => {
            ViewStackProcessor.StartGetAccessRecordingFor(h16);
            Column.create();
            Column.flexGrow(1);
            if (!i16) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((e16, f16) => {
            ViewStackProcessor.StartGetAccessRecordingFor(e16);
            TextInput.create({
                placeholder: `${w15.type} ${w15 && w15.description ? '，' + w15.description : ''}`,
                text: `${this.formData[w15.key] ?? ''}`
            });
            TextInput.maxLength(w15.maxLength);
            TextInput.width('100%');
            TextInput.height(40);
            TextInput.fontSize(16);
            TextInput.fontColor({ "id": 16777249, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            TextInput.padding({ right: 16, left: 16 });
            TextInput.backgroundColor({ "id": 16777239, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            TextInput.onChange((g16: string) => {
                this.formData[w15.key] = g16;
            });
            if (!f16) {
                TextInput.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((y15, z15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(y15);
            If.create();
            if (w15.required) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation((c16, d16) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(c16);
                        Row.create();
                        Row.width('100%');
                        Row.padding({ top: 5, right: 10 });
                        if (!d16) {
                            Row.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    this.observeComponentCreation((a16, b16) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(a16);
                        Text.create({ "id": 16777234, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                        Text.fontSize(12);
                        Text.lineHeight(16);
                        Text.fontColor({ "id": 16777246, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                        Text.padding({ left: 10 });
                        if (!b16) {
                            Text.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    Text.pop();
                    Row.pop();
                });
            }
            else {
                this.ifElseBranchUpdateFunction(1, () => {
                });
            }
            if (!z15) {
                If.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        If.pop();
        Column.pop();
        Flex.pop();
    }
    buildNumberInput(f15: FormField, g15 = null) {
        this.observeComponentCreation((u15, v15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(u15);
            Flex.create({ direction: FlexDirection.Row, alignItems: ItemAlign.Start });
            Flex.height(50);
            Flex.margin({ bottom: 15 });
            if (!v15) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((s15, t15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(s15);
            Text.create(f15.name);
            Text.fontSize(16);
            Text.fontColor({ "id": 16777247, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.width(165);
            Text.height(40);
            Text.borderRadius(20);
            Text.padding({ right: 16, left: 16 });
            Text.margin({ right: 8 });
            Text.backgroundColor({ "id": 16777239, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.textOverflow({ overflow: TextOverflow.Ellipsis });
            Text.maxLines(1);
            if (!t15) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((q15, r15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(q15);
            Column.create();
            Column.flexGrow(1);
            if (!r15) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((n15, o15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(n15);
            TextInput.create({
                placeholder: `${f15.type} ${f15 && f15.description ? '，' + f15.description : ''}`,
                text: `${this.formData[f15.key] ?? ''}`
            });
            TextInput.maxLength(f15.maxLength);
            TextInput.type(InputType.Number);
            TextInput.width('100%');
            TextInput.height(40);
            TextInput.fontSize(16);
            TextInput.fontColor({ "id": 16777249, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            TextInput.padding({ right: 16, left: 16 });
            TextInput.backgroundColor({ "id": 16777239, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            TextInput.onChange((p15: string) => {
                this.formData[f15.key] = parseInt(p15);
            });
            if (!o15) {
                TextInput.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((h15, i15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(h15);
            If.create();
            if (f15.required) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation((l15, m15) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(l15);
                        Row.create();
                        Row.width('100%');
                        Row.padding({ top: 5, right: 10 });
                        if (!m15) {
                            Row.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    this.observeComponentCreation((j15, k15) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(j15);
                        Text.create({ "id": 16777234, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                        Text.fontSize(12);
                        Text.lineHeight(16);
                        Text.fontColor({ "id": 16777246, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                        Text.padding({ left: 10 });
                        if (!k15) {
                            Text.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    Text.pop();
                    Row.pop();
                });
            }
            else {
                this.ifElseBranchUpdateFunction(1, () => {
                });
            }
            if (!i15) {
                If.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        If.pop();
        Column.pop();
        Flex.pop();
    }
    buildSelectInput(i14: FormField, j14 = null) {
        this.observeComponentCreation((d15, e15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(d15);
            Flex.create({ direction: FlexDirection.Row, alignItems: ItemAlign.Start });
            Flex.margin({ bottom: 20 });
            if (!e15) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((b15, c15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(b15);
            Text.create(i14.name);
            Text.fontSize(16);
            Text.fontColor({ "id": 16777247, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.width(165);
            Text.height(40);
            Text.borderRadius(20);
            Text.padding({ right: 16, left: 16 });
            Text.margin({ right: 8 });
            Text.backgroundColor({ "id": 16777239, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.textOverflow({ overflow: TextOverflow.Ellipsis });
            Text.maxLines(1);
            if (!c15) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((z14, a15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(z14);
            Column.create();
            Column.flexGrow(1);
            if (!a15) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((s14, t14) => {
            ViewStackProcessor.StartGetAccessRecordingFor(s14);
            Select.create(i14.options?.map(x14 => {
                const y14: SelectOption = { value: x14.toString() };
                return y14;
            }));
            Select.width('100%');
            Select.height(40);
            Select.font({ size: 16, weight: 400 });
            Select.fontColor({ "id": 16777249, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Select.padding({ right: 6, left: 8 });
            Select.backgroundColor({ "id": 16777239, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Select.optionWidth(224);
            Select.optionFont({ size: 16, weight: 500 });
            Select.optionFontColor({ "id": 16777249, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Select.onSelect((u14: number, v14: string) => {
                const w14 = i14.type === 'string' ? v14 : parseInt(v14);
                this.formData[i14.key] = w14;
            });
            if (!t14) {
                Select.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Select.pop();
        this.observeComponentCreation((q14, r14) => {
            ViewStackProcessor.StartGetAccessRecordingFor(q14);
            Row.create();
            Row.width('100%');
            Row.padding({ top: 5, right: 10 });
            if (!r14) {
                Row.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((m14, n14) => {
            ViewStackProcessor.StartGetAccessRecordingFor(m14);
            If.create();
            if (i14.required) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation((o14, p14) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(o14);
                        Text.create({ "id": 16777234, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                        Text.fontSize(12);
                        Text.lineHeight(16);
                        Text.fontColor({ "id": 16777246, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                        Text.padding({ left: 10 });
                        if (!p14) {
                            Text.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    Text.pop();
                });
            }
            else {
                this.ifElseBranchUpdateFunction(1, () => {
                });
            }
            if (!n14) {
                If.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        If.pop();
        this.observeComponentCreation((k14, l14) => {
            ViewStackProcessor.StartGetAccessRecordingFor(k14);
            Text.create(i14.description);
            Text.fontSize(12);
            Text.lineHeight(16);
            Text.fontColor({ "id": 16777247, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.padding({ left: 10 });
            if (!l14) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Row.pop();
        Column.pop();
        Flex.pop();
    }
    buildFormFields(r12: FormField[] | FormField[][], s12 = null) {
        this.observeComponentCreation((g14, h14) => {
            ViewStackProcessor.StartGetAccessRecordingFor(g14);
            Column.create();
            if (!h14) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((t12, u12) => {
            ViewStackProcessor.StartGetAccessRecordingFor(t12);
            ForEach.create();
            const v12 = (w12, x12: number) => {
                const y12 = w12;
                this.observeComponentCreation((z12, a13) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(z12);
                    If.create();
                    if (y12.inputType === 'text') {
                        this.ifElseBranchUpdateFunction(0, () => {
                            this.buildTextInput.bind(this)(y12);
                        });
                    }
                    else if (y12.inputType === 'number') {
                        this.ifElseBranchUpdateFunction(1, () => {
                            this.buildNumberInput.bind(this)(y12);
                        });
                    }
                    else if (y12.inputType === 'select') {
                        this.ifElseBranchUpdateFunction(2, () => {
                            this.buildSelectInput.bind(this)(y12);
                        });
                    }
                    else if (y12.inputType === 'object') {
                        this.ifElseBranchUpdateFunction(3, () => {
                            this.observeComponentCreation((y13, z13) => {
                                ViewStackProcessor.StartGetAccessRecordingFor(y13);
                                If.create();
                                if (y12.inputType === 'object' && !y12.subProperties) {
                                    this.ifElseBranchUpdateFunction(0, () => {
                                        this.buildTextInput.bind(this)(y12);
                                    });
                                }
                                else {
                                    this.ifElseBranchUpdateFunction(1, () => {
                                        this.observeComponentCreation((e14, f14) => {
                                            ViewStackProcessor.StartGetAccessRecordingFor(e14);
                                            Column.create();
                                            if (!f14) {
                                                Column.pop();
                                            }
                                            ViewStackProcessor.StopGetAccessRecording();
                                        });
                                        this.observeComponentCreation((c14, d14) => {
                                            ViewStackProcessor.StartGetAccessRecordingFor(c14);
                                            Row.create();
                                            Row.margin({ bottom: 8, top: 16 });
                                            if (!d14) {
                                                Row.pop();
                                            }
                                            ViewStackProcessor.StopGetAccessRecording();
                                        });
                                        this.observeComponentCreation((a14, b14) => {
                                            ViewStackProcessor.StartGetAccessRecordingFor(a14);
                                            Text.create(y12.name);
                                            Text.fontSize(14);
                                            Text.lineHeight(20);
                                            Text.fontWeight(500);
                                            Text.fontColor({ "id": 16777248, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                            Text.width('100%');
                                            Text.padding({ left: 12 });
                                            if (!b14) {
                                                Text.pop();
                                            }
                                            ViewStackProcessor.StopGetAccessRecording();
                                        });
                                        Text.pop();
                                        Row.pop();
                                        this.buildFormFields.bind(this)(y12.subProperties as FormField[]);
                                        Column.pop();
                                    });
                                }
                                if (!z13) {
                                    If.pop();
                                }
                                ViewStackProcessor.StopGetAccessRecording();
                            });
                            If.pop();
                        });
                    }
                    else if (y12.inputType === 'array') {
                        this.ifElseBranchUpdateFunction(4, () => {
                            this.observeComponentCreation((b13, c13) => {
                                ViewStackProcessor.StartGetAccessRecordingFor(b13);
                                ForEach.create();
                                const d13 = (e13, f13: number) => {
                                    const g13 = e13;
                                    this.observeComponentCreation((w13, x13) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(w13);
                                        Column.create();
                                        if (!x13) {
                                            Column.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    this.observeComponentCreation((u13, v13) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(u13);
                                        Flex.create({
                                            direction: FlexDirection.Row,
                                            justifyContent: FlexAlign.SpaceBetween,
                                            alignItems: ItemAlign.Center
                                        });
                                        Flex.margin({
                                            top: 16,
                                            bottom: 8,
                                            left: 12,
                                            right: 12
                                        });
                                        if (!v13) {
                                            Flex.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    this.observeComponentCreation((s13, t13) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(s13);
                                        Text.create(y12.name);
                                        Text.fontSize(14);
                                        Text.fontWeight(500);
                                        Text.fontColor({ "id": 16777248, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                        Text.width('100%');
                                        Text.lineHeight(20);
                                        if (!t13) {
                                            Text.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    Text.pop();
                                    this.observeComponentCreation((h13, i13) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(h13);
                                        If.create();
                                        if (f13 === 0) {
                                            this.ifElseBranchUpdateFunction(0, () => {
                                                this.observeComponentCreation((o13, p13) => {
                                                    ViewStackProcessor.StartGetAccessRecordingFor(o13);
                                                    Text.create({ "id": 16777222, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                                    Text.fontSize(14);
                                                    Text.fontColor('rgb(10,89,247)');
                                                    Text.fontWeight(500);
                                                    Text.lineHeight(19);
                                                    Text.onClick(() => {
                                                        if (y12.maxLength && y12.subProperties && (y12.subProperties?.length >= y12.maxLength)) {
                                                            this.getUIContext().showAlertDialog({
                                                                message: `最大节点数不超过${y12.maxLength}个`,
                                                                autoCancel: true,
                                                                alignment: DialogAlignment.Center,
                                                                offset: { dx: 0, dy: -20 },
                                                                gridCount: 3,
                                                                confirm: {
                                                                    value: { "id": 16777220, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" },
                                                                    action: () => {
                                                                        console.info('Button-clicking callback');
                                                                    }
                                                                },
                                                                cancel: () => {
                                                                    console.info('Closed callbacks');
                                                                }
                                                            });
                                                        }
                                                        else {
                                                            this.countArr.push(this.countArr.length);
                                                            const q13: FormField[][] = addArrayParams(y12.subProperties, y12.name, this.countArr);
                                                            let r13 = jsClone(this.formFields) as FormField[];
                                                            r13[x12].subProperties = q13;
                                                            this.formFields = r13;
                                                        }
                                                    });
                                                    if (!p13) {
                                                        Text.pop();
                                                    }
                                                    ViewStackProcessor.StopGetAccessRecording();
                                                });
                                                Text.pop();
                                            });
                                        }
                                        else {
                                            this.ifElseBranchUpdateFunction(1, () => {
                                                this.observeComponentCreation((j13, k13) => {
                                                    ViewStackProcessor.StartGetAccessRecordingFor(j13);
                                                    Text.create({ "id": 16777221, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                                    Text.fontSize(14);
                                                    Text.fontColor('rgb(10,89,247)');
                                                    Text.fontWeight(500);
                                                    Text.lineHeight(19);
                                                    Text.onClick(() => {
                                                        const l13: FormDataType = removeFormData(this.formData, y12.name, g13[0].index);
                                                        this.formData = l13;
                                                        const m13: FormField[][] = removeArrayParams(y12.subProperties, y12.name, f13);
                                                        let n13 = jsClone(this.formFields) as FormField[];
                                                        n13[x12].subProperties = m13;
                                                        this.formFields = n13;
                                                    });
                                                    if (!k13) {
                                                        Text.pop();
                                                    }
                                                    ViewStackProcessor.StopGetAccessRecording();
                                                });
                                                Text.pop();
                                            });
                                        }
                                        if (!i13) {
                                            If.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    If.pop();
                                    Flex.pop();
                                    this.buildFormFields.bind(this)(g13 as FormField[]);
                                    Column.pop();
                                };
                                this.forEachUpdateFunction(b13, y12.subProperties, d13, undefined, true, false);
                                if (!c13) {
                                    ForEach.pop();
                                }
                                ViewStackProcessor.StopGetAccessRecording();
                            });
                            ForEach.pop();
                        });
                    }
                    else {
                        this.ifElseBranchUpdateFunction(5, () => {
                        });
                    }
                    if (!a13) {
                        If.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                If.pop();
            };
            this.forEachUpdateFunction(t12, r12, v12, undefined, true, false);
            if (!u12) {
                ForEach.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        ForEach.pop();
        Column.pop();
    }
    initialRender() {
        this.observeComponentCreation((b10, c10) => {
            ViewStackProcessor.StartGetAccessRecordingFor(b10);
            NavDestination.create(() => {
                this.observeComponentCreation((p12, q12) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(p12);
                    Row.create();
                    Row.width('100%');
                    Row.height(56);
                    Row.padding({
                        top: 14.5,
                        right: 16,
                        left: 16,
                        bottom: 14.5
                    });
                    Row.backgroundColor({ "id": 16777238, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                    Row.position({ top: 0, left: 0 });
                    Row.zIndex(100);
                    Row.onClick(() => {
                        this.articlePathStack.pop();
                    });
                    if (!q12) {
                        Row.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                this.observeComponentCreation((n12, o12) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(n12);
                    Column.create();
                    Column.width(40);
                    Column.height(40);
                    Column.borderRadius(20);
                    Column.backgroundColor({ "id": 16777245, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                    Column.justifyContent(FlexAlign.Center);
                    if (!o12) {
                        Column.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                this.observeComponentCreation((l12, m12) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(l12);
                    Image.create({ "id": 16777255, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                    Image.width(8.68);
                    Image.height(16.48);
                    Image.opacity(0.6);
                    if (!m12) {
                        Image.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                Column.pop();
                this.observeComponentCreation((j12, k12) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(j12);
                    Text.create({ "id": 16777228, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                    Text.margin({ left: 8 });
                    Text.fontSize(20);
                    Text.fontWeight(700);
                    Text.fontColor({ "id": 16777249, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                    if (!k12) {
                        Text.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                Text.pop();
                Row.pop();
                this.observeComponentCreation((h12, i12) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(h12);
                    Scroll.create(this.scroller);
                    Scroll.align(Alignment.TopStart);
                    Scroll.constraintSize({ minHeight: '100%' });
                    Scroll.edgeEffect(EdgeEffect.Spring);
                    Scroll.scrollable(ScrollDirection.Vertical);
                    Scroll.scrollBar(BarState.Off);
                    Scroll.backgroundColor({ "id": 16777238, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                    Scroll.width('100%');
                    Scroll.height('100%');
                    Scroll.padding({ top: 56, bottom: 0 });
                    Scroll.clip(true);
                    if (!i12) {
                        Scroll.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                this.observeComponentCreation((f12, g12) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(f12);
                    Column.create();
                    Column.width('100%');
                    Column.alignItems(HorizontalAlign.Start);
                    if (!g12) {
                        Column.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                {
                    this.observeComponentCreation((b12, c12) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(b12);
                        if (c12) {
                            let d12 = new DetailItem(this, { intentDetail: this.intentDetail }, undefined, b12, () => { }, { page: "feature/src/main/ets/view/IntentDetailPage.ets", line: 733, col: 11 });
                            ViewPU.create(d12);
                            let e12 = () => {
                                return {
                                    intentDetail: this.intentDetail
                                };
                            };
                            d12.paramsGenerator_ = e12;
                        }
                        else {
                            this.updateStateVarsOfChildByElmtId(b12, {
                                intentDetail: this.intentDetail
                            });
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                }
                this.observeComponentCreation((z11, a12) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(z11);
                    Column.create();
                    Column.padding({ bottom: 86 });
                    Column.margin({ left: 16, right: 16 });
                    if (!a12) {
                        Column.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                this.observeComponentCreation((x11, y11) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(x11);
                    Text.create({ "id": 16777226, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                    Text.fontFamily('HarmonyHeiTi-Medium');
                    Text.fontSize(18);
                    Text.fontWeight(700);
                    Text.fontColor({ "id": 16777249, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                    Text.lineHeight(24);
                    Text.padding({
                        top: 24,
                        right: 16,
                        bottom: 8,
                        left: 16
                    });
                    Text.margin({ bottom: 12 });
                    Text.textAlign(TextAlign.Start);
                    Text.width('100%');
                    if (!y11) {
                        Text.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                Text.pop();
                this.observeComponentCreation((i10, j10) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(i10);
                    If.create();
                    if (this.intentParameters !== null) {
                        this.ifElseBranchUpdateFunction(0, () => {
                            this.observeComponentCreation((k10, l10) => {
                                ViewStackProcessor.StartGetAccessRecordingFor(k10);
                                ForEach.create();
                                const m10 = (n10, o10: number) => {
                                    const p10 = n10;
                                    this.observeComponentCreation((q10, r10) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(q10);
                                        If.create();
                                        if (p10.inputType === 'text') {
                                            this.ifElseBranchUpdateFunction(0, () => {
                                                this.buildTextInput.bind(this)(p10);
                                            });
                                        }
                                        else if (p10.inputType === 'number') {
                                            this.ifElseBranchUpdateFunction(1, () => {
                                                this.buildNumberInput.bind(this)(p10);
                                            });
                                        }
                                        else if (p10.inputType === 'select') {
                                            this.ifElseBranchUpdateFunction(2, () => {
                                                this.buildSelectInput.bind(this)(p10);
                                            });
                                        }
                                        else if (p10.inputType === 'object') {
                                            this.ifElseBranchUpdateFunction(3, () => {
                                                this.observeComponentCreation((p11, q11) => {
                                                    ViewStackProcessor.StartGetAccessRecordingFor(p11);
                                                    If.create();
                                                    if (p10.inputType === 'object' && !p10.subProperties) {
                                                        this.ifElseBranchUpdateFunction(0, () => {
                                                            this.buildTextInput.bind(this)(p10);
                                                        });
                                                    }
                                                    else {
                                                        this.ifElseBranchUpdateFunction(1, () => {
                                                            this.observeComponentCreation((v11, w11) => {
                                                                ViewStackProcessor.StartGetAccessRecordingFor(v11);
                                                                Column.create();
                                                                if (!w11) {
                                                                    Column.pop();
                                                                }
                                                                ViewStackProcessor.StopGetAccessRecording();
                                                            });
                                                            this.observeComponentCreation((t11, u11) => {
                                                                ViewStackProcessor.StartGetAccessRecordingFor(t11);
                                                                Row.create();
                                                                Row.margin({ bottom: 8, top: 16 });
                                                                if (!u11) {
                                                                    Row.pop();
                                                                }
                                                                ViewStackProcessor.StopGetAccessRecording();
                                                            });
                                                            this.observeComponentCreation((r11, s11) => {
                                                                ViewStackProcessor.StartGetAccessRecordingFor(r11);
                                                                Text.create(p10.name);
                                                                Text.fontSize(14);
                                                                Text.lineHeight(20);
                                                                Text.fontWeight(500);
                                                                Text.fontColor({ "id": 16777248, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                                                Text.width('100%');
                                                                Text.padding({ left: 12 });
                                                                if (!s11) {
                                                                    Text.pop();
                                                                }
                                                                ViewStackProcessor.StopGetAccessRecording();
                                                            });
                                                            Text.pop();
                                                            Row.pop();
                                                            this.buildFormFields.bind(this)(p10.subProperties as FormField[]);
                                                            Column.pop();
                                                        });
                                                    }
                                                    if (!q11) {
                                                        If.pop();
                                                    }
                                                    ViewStackProcessor.StopGetAccessRecording();
                                                });
                                                If.pop();
                                            });
                                        }
                                        else if (p10.inputType === 'array') {
                                            this.ifElseBranchUpdateFunction(4, () => {
                                                this.observeComponentCreation((s10, t10) => {
                                                    ViewStackProcessor.StartGetAccessRecordingFor(s10);
                                                    ForEach.create();
                                                    const u10 = (v10, w10: number) => {
                                                        const x10 = v10;
                                                        this.observeComponentCreation((n11, o11) => {
                                                            ViewStackProcessor.StartGetAccessRecordingFor(n11);
                                                            Column.create();
                                                            if (!o11) {
                                                                Column.pop();
                                                            }
                                                            ViewStackProcessor.StopGetAccessRecording();
                                                        });
                                                        this.observeComponentCreation((l11, m11) => {
                                                            ViewStackProcessor.StartGetAccessRecordingFor(l11);
                                                            Flex.create({
                                                                direction: FlexDirection.Row,
                                                                justifyContent: FlexAlign.SpaceBetween,
                                                                alignItems: ItemAlign.Center
                                                            });
                                                            Flex.margin({
                                                                top: 16,
                                                                bottom: 8,
                                                                left: 12,
                                                                right: 12
                                                            });
                                                            if (!m11) {
                                                                Flex.pop();
                                                            }
                                                            ViewStackProcessor.StopGetAccessRecording();
                                                        });
                                                        this.observeComponentCreation((j11, k11) => {
                                                            ViewStackProcessor.StartGetAccessRecordingFor(j11);
                                                            Text.create(p10.name);
                                                            Text.fontSize(14);
                                                            Text.fontColor({ "id": 16777248, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                                            Text.width('50%');
                                                            Text.lineHeight(20);
                                                            Text.fontWeight(500);
                                                            if (!k11) {
                                                                Text.pop();
                                                            }
                                                            ViewStackProcessor.StopGetAccessRecording();
                                                        });
                                                        Text.pop();
                                                        this.observeComponentCreation((y10, z10) => {
                                                            ViewStackProcessor.StartGetAccessRecordingFor(y10);
                                                            If.create();
                                                            if (w10 === 0) {
                                                                this.ifElseBranchUpdateFunction(0, () => {
                                                                    this.observeComponentCreation((f11, g11) => {
                                                                        ViewStackProcessor.StartGetAccessRecordingFor(f11);
                                                                        Text.create({ "id": 16777222, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                                                        Text.fontSize(14);
                                                                        Text.fontColor('rgb(10,89,247)');
                                                                        Text.fontWeight(500);
                                                                        Text.lineHeight(19);
                                                                        Text.onClick(() => {
                                                                            if (p10.maxLength && p10.subProperties &&
                                                                                (p10.subProperties?.length >= p10.maxLength)) {
                                                                                this.getUIContext().showAlertDialog({
                                                                                    message: `最大节点不超过${p10.maxLength}个`,
                                                                                    autoCancel: true,
                                                                                    alignment: DialogAlignment.Center,
                                                                                    offset: { dx: 0, dy: -20 },
                                                                                    gridCount: 3,
                                                                                    confirm: {
                                                                                        value: { "id": 16777220, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" },
                                                                                        action: () => {
                                                                                            console.info('Button-clicking callback');
                                                                                        }
                                                                                    },
                                                                                    cancel: () => {
                                                                                        console.info('Closed callbacks');
                                                                                    }
                                                                                });
                                                                            }
                                                                            else {
                                                                                this.countArr.push(this.countArr.length);
                                                                                const h11: FormField[][] = addArrayParams(p10.subProperties, p10.name, this.countArr);
                                                                                let i11 = jsClone(this.formFields) as FormField[];
                                                                                i11[o10].subProperties = h11;
                                                                                this.formFields = i11;
                                                                            }
                                                                        });
                                                                        if (!g11) {
                                                                            Text.pop();
                                                                        }
                                                                        ViewStackProcessor.StopGetAccessRecording();
                                                                    });
                                                                    Text.pop();
                                                                });
                                                            }
                                                            else {
                                                                this.ifElseBranchUpdateFunction(1, () => {
                                                                    this.observeComponentCreation((a11, b11) => {
                                                                        ViewStackProcessor.StartGetAccessRecordingFor(a11);
                                                                        Text.create({ "id": 16777221, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                                                        Text.fontSize(14);
                                                                        Text.fontColor('rgb(10,89,247)');
                                                                        Text.fontWeight(500);
                                                                        Text.lineHeight(19);
                                                                        Text.onClick(() => {
                                                                            const c11: FormDataType = removeFormData(this.formData, p10.name, x10[0].index);
                                                                            this.formData = c11;
                                                                            const d11: FormField[][] = removeArrayParams(p10.subProperties, p10.name, w10);
                                                                            let e11 = jsClone(this.formFields) as FormField[];
                                                                            e11[o10].subProperties = d11;
                                                                            this.formFields = e11;
                                                                        });
                                                                        if (!b11) {
                                                                            Text.pop();
                                                                        }
                                                                        ViewStackProcessor.StopGetAccessRecording();
                                                                    });
                                                                    Text.pop();
                                                                });
                                                            }
                                                            if (!z10) {
                                                                If.pop();
                                                            }
                                                            ViewStackProcessor.StopGetAccessRecording();
                                                        });
                                                        If.pop();
                                                        Flex.pop();
                                                        this.buildFormFields.bind(this)(x10 as FormField[]);
                                                        Column.pop();
                                                    };
                                                    this.forEachUpdateFunction(s10, p10.subProperties, u10, undefined, true, false);
                                                    if (!t10) {
                                                        ForEach.pop();
                                                    }
                                                    ViewStackProcessor.StopGetAccessRecording();
                                                });
                                                ForEach.pop();
                                            });
                                        }
                                        else {
                                            this.ifElseBranchUpdateFunction(5, () => {
                                            });
                                        }
                                        if (!r10) {
                                            If.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    If.pop();
                                };
                                this.forEachUpdateFunction(k10, this.formFields, m10, undefined, true, false);
                                if (!l10) {
                                    ForEach.pop();
                                }
                                ViewStackProcessor.StopGetAccessRecording();
                            });
                            ForEach.pop();
                        });
                    }
                    else {
                        this.ifElseBranchUpdateFunction(1, () => {
                        });
                    }
                    if (!j10) {
                        If.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                If.pop();
                Column.pop();
                Column.pop();
                Scroll.pop();
                this.observeComponentCreation((g10, h10) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(g10);
                    Row.create();
                    Row.width('100%');
                    Row.padding({
                        top: 16,
                        left: 16,
                        right: 16,
                        bottom: 6
                    });
                    Row.position({ bottom: 0, left: 0 });
                    Row.backgroundColor({ "id": 16777238, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                    if (!h10) {
                        Row.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                this.observeComponentCreation((d10, e10) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(d10);
                    Button.createWithLabel({ "id": 16777224, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" }, { stateEffect: true });
                    Button.width('100%');
                    Button.fontSize(16);
                    Button.fontWeight(500);
                    Button.backgroundColor({ "id": 16777244, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                    Button.borderRadius(20);
                    Button.height(40);
                    Button.onClick(() => {
                        this.submitForm();
                    });
                    Button.bindSheet({ value: this.isShow, changeEvent: f10 => { this.isShow = f10; } }, { builder: () => {
                            this.resultModule.call(this);
                        } }, {
                        height: SheetSize.MEDIUM,
                        blurStyle: BlurStyle.Thick,
                        title: { title: { "id": 16777225, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" } },
                        preferType: SheetType.BOTTOM
                    });
                    if (!e10) {
                        Button.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                Button.pop();
                Row.pop();
            }, { moduleName: "default", pagePath: "feature/src/main/ets/view/IntentDetailPage" });
            NavDestination.hideTitleBar(true);
            NavDestination.width('100%');
            NavDestination.height('100%');
            NavDestination.backgroundColor({ "id": 16777238, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            NavDestination.clip(true);
            if (!c10) {
                NavDestination.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        NavDestination.pop();
    }
    rerender() {
        this.updateDirtyElements();
    }
    public __resetStateVarsOnReuse__Internal(a10: Object): void {
        this.__reInitializeConsume__Internal('articlePathStack', "articlePathStack");
    }
}
