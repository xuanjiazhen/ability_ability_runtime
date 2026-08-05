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
    constructor(e19, f19, g19, h19 = -1, i19 = undefined, j19) {
        super(e19, g19, h19, j19);
        if (typeof i19 === "function") {
            this.paramsGenerator_ = i19;
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
        this.setInitiallyProvidedValue(f19);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(d19: IntentDetailPage_Params) {
        if (d19.intentDetail !== undefined) {
            this.intentDetail = d19.intentDetail;
        }
        if (d19.intentDetailMap !== undefined) {
            this.intentDetailMap = d19.intentDetailMap;
        }
        if (d19.intentDetailEntries !== undefined) {
            this.intentDetailEntries = d19.intentDetailEntries;
        }
        if (d19.intentParameters !== undefined) {
            this.intentParameters = d19.intentParameters;
        }
        if (d19.scroller !== undefined) {
            this.scroller = d19.scroller;
        }
        if (d19.required !== undefined) {
            this.required = d19.required;
        }
        if (d19.formData !== undefined) {
            this.formData = d19.formData;
        }
        if (d19.properties !== undefined) {
            this.properties = d19.properties;
        }
        if (d19.formFields !== undefined) {
            this.formFields = d19.formFields;
        }
        if (d19.abilityName !== undefined) {
            this.abilityName = d19.abilityName;
        }
        if (d19.uri !== undefined) {
            this.uri = d19.uri;
        }
        if (d19.formName !== undefined) {
            this.formName = d19.formName;
        }
        if (d19.countArr !== undefined) {
            this.countArr = d19.countArr;
        }
        if (d19.result !== undefined) {
            this.result = d19.result;
        }
        if (d19.isShow !== undefined) {
            this.isShow = d19.isShow;
        }
        if (d19.isShowEntry2 !== undefined) {
            this.isShowEntry2 = d19.isShowEntry2;
        }
        if (d19.isShowForm !== undefined) {
            this.isShowForm = d19.isShowForm;
        }
        if (d19.want !== undefined) {
            this.want = d19.want;
        }
        if (d19.inputType !== undefined) {
            this.inputType = d19.inputType;
        }
        if (d19.InputTypeData !== undefined) {
            this.InputTypeData = d19.InputTypeData;
        }
    }
    updateStateVars(c19: IntentDetailPage_Params) {
    }
    purgeVariableDependenciesOnElmtId(b19) {
        this.__articlePathStack.purgeDependencyOnElmtId(b19);
        this.__intentDetail.purgeDependencyOnElmtId(b19);
        this.__intentDetailMap.purgeDependencyOnElmtId(b19);
        this.__intentDetailEntries.purgeDependencyOnElmtId(b19);
        this.__intentParameters.purgeDependencyOnElmtId(b19);
        this.__required.purgeDependencyOnElmtId(b19);
        this.__formData.purgeDependencyOnElmtId(b19);
        this.__properties.purgeDependencyOnElmtId(b19);
        this.__formFields.purgeDependencyOnElmtId(b19);
        this.__abilityName.purgeDependencyOnElmtId(b19);
        this.__uri.purgeDependencyOnElmtId(b19);
        this.__formName.purgeDependencyOnElmtId(b19);
        this.__countArr.purgeDependencyOnElmtId(b19);
        this.__result.purgeDependencyOnElmtId(b19);
        this.__isShow.purgeDependencyOnElmtId(b19);
        this.__isShowEntry2.purgeDependencyOnElmtId(b19);
        this.__isShowForm.purgeDependencyOnElmtId(b19);
        this.__want.purgeDependencyOnElmtId(b19);
        this.__InputTypeData.purgeDependencyOnElmtId(b19);
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
    set articlePathStack(a19: NavPathStack) {
        this.__articlePathStack.set(a19);
    }
    private __intentDetail: ObservedPropertyObjectPU<IntentInfo | null>;
    get intentDetail() {
        return this.__intentDetail.get();
    }
    set intentDetail(z18: IntentInfo | null) {
        this.__intentDetail.set(z18);
    }
    private __intentDetailMap: ObservedPropertyObjectPU<Map<string, string> | null>;
    get intentDetailMap() {
        return this.__intentDetailMap.get();
    }
    set intentDetailMap(y18: Map<string, string> | null) {
        this.__intentDetailMap.set(y18);
    }
    private __intentDetailEntries: ObservedPropertyObjectPU<Array<[
        string,
        string | string[] | object
    ]> | [
    ]>;
    get intentDetailEntries() {
        return this.__intentDetailEntries.get();
    }
    set intentDetailEntries(x18: Array<[
        string,
        string | string[] | object
    ]> | [
    ]) {
        this.__intentDetailEntries.set(x18);
    }
    private __intentParameters: ObservedPropertyObjectPU<object | null>;
    get intentParameters() {
        return this.__intentParameters.get();
    }
    set intentParameters(w18: object | null) {
        this.__intentParameters.set(w18);
    }
    private scroller: Scroller;
    private __required: ObservedPropertyObjectPU<string[]>;
    get required() {
        return this.__required.get();
    }
    set required(v18: string[]) {
        this.__required.set(v18);
    }
    private __formData: ObservedPropertyObjectPU<FormDataType>;
    get formData() {
        return this.__formData.get();
    }
    set formData(u18: FormDataType) {
        this.__formData.set(u18);
    }
    private __properties: ObservedPropertyObjectPU<Record<string, propertiesInfo>>;
    get properties() {
        return this.__properties.get();
    }
    set properties(t18: Record<string, propertiesInfo>) {
        this.__properties.set(t18);
    }
    private __formFields: ObservedPropertyObjectPU<FormField[]>;
    get formFields() {
        return this.__formFields.get();
    }
    set formFields(s18: FormField[]) {
        this.__formFields.set(s18);
    }
    private __abilityName: ObservedPropertySimplePU<string>;
    get abilityName() {
        return this.__abilityName.get();
    }
    set abilityName(r18: string) {
        this.__abilityName.set(r18);
    }
    private __uri: ObservedPropertySimplePU<string>;
    get uri() {
        return this.__uri.get();
    }
    set uri(q18: string) {
        this.__uri.set(q18);
    }
    private __formName: ObservedPropertySimplePU<string>;
    get formName() {
        return this.__formName.get();
    }
    set formName(p18: string) {
        this.__formName.set(p18);
    }
    private __countArr: ObservedPropertyObjectPU<number[]>;
    get countArr() {
        return this.__countArr.get();
    }
    set countArr(o18: number[]) {
        this.__countArr.set(o18);
    }
    private __result: ObservedPropertySimplePU<string>;
    get result() {
        return this.__result.get();
    }
    set result(n18: string) {
        this.__result.set(n18);
    }
    private __isShow: ObservedPropertySimplePU<boolean>;
    get isShow() {
        return this.__isShow.get();
    }
    set isShow(m18: boolean) {
        this.__isShow.set(m18);
    }
    private __isShowEntry2: ObservedPropertySimplePU<boolean>;
    get isShowEntry2() {
        return this.__isShowEntry2.get();
    }
    set isShowEntry2(l18: boolean) {
        this.__isShowEntry2.set(l18);
    }
    private __isShowForm: ObservedPropertySimplePU<boolean>;
    get isShowForm() {
        return this.__isShowForm.get();
    }
    set isShowForm(k18: boolean) {
        this.__isShowForm.set(k18);
    }
    private __want: ObservedPropertyObjectPU<Want>;
    get want() {
        return this.__want.get();
    }
    set want(j18: Want) {
        this.__want.set(j18);
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
        let i18 = AppStorage.get('context') as common.UIAbilityContext;
        i18.terminateSelf();
    }
    private __InputTypeData: ObservedPropertyObjectPU<Record<string, InputType>>;
    get InputTypeData() {
        return this.__InputTypeData.get();
    }
    set InputTypeData(h18: Record<string, InputType>) {
        this.__InputTypeData.set(h18);
    }
    getAttribute(f18: object | undefined, g18: string): string | string[] {
        if (f18 && Object.keys(f18).includes(g18)) {
            return f18[g18];
        }
        else {
            return '';
        }
    }
    async getUIExtensionType(r17: string, s17: string, t17: string): Promise<UIExtensionInfo> {
        hilog.info(DOMAIN, TAG, `getUIExtensionType start: bundleName=${r17}, moduleName=${s17}, abilityName=${t17}`);
        const u17 = await bundleManager.getBundleInfo(r17, bundleManager.BundleFlag.GET_BUNDLE_INFO_WITH_HAP_MODULE |
            bundleManager.BundleFlag.GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY);
        hilog.info(DOMAIN, TAG, `getBundleInfo success: bundleName=${u17.name}`);
        const v17 = u17.hapModulesInfo;
        if (!v17) {
            const e18 = `getUIExtensionType failed: hapModulesInfo is null for bundleName=${r17}`;
            hilog.error(DOMAIN, TAG, e18);
            throw new Error(e18);
        }
        for (let x17 = 0; x17 < v17.length; x17++) {
            const y17 = v17[x17];
            if (y17.name === s17) {
                const z17 = y17.extensionAbilitiesInfo;
                if (!z17) {
                    hilog.warn(DOMAIN, TAG, `extensionAbilitiesInfo is null for module: ${s17}`);
                    continue;
                }
                for (let a18 = 0; a18 < z17.length; a18++) {
                    const b18 = z17[a18];
                    const c18 = b18.extensionAbilityTypeName;
                    if (c18 && c18 !== '') {
                        hilog.info(DOMAIN, TAG, `getUIExtensionType success: found UIExtensionAbility ${b18.name}, type=${c18}`);
                        const d18: UIExtensionInfo = {
                            type: c18,
                            name: b18.name
                        };
                        return d18;
                    }
                }
            }
        }
        const w17 = `getUIExtensionType failed: UIExtensionAbility not found for bundleName=${r17}, moduleName=${s17}, abilityName=${t17}`;
        hilog.error(DOMAIN, TAG, w17);
        throw new Error(w17);
    }
    changeProperties(b17: Record<string, propertiesInfo>, c17?: string, d17?: string) {
        const e17: FormField[] = [];
        Object.keys(b17).forEach((f17: string, g17: number) => {
            const h17 = b17[f17];
            let i17 = f17;
            if (c17) {
                if (d17 && d17 === 'array') {
                    i17 = `${c17}.array0.${f17}`;
                }
                else {
                    i17 = `${c17}.${f17}`;
                }
            }
            const j17 = f17;
            const k17 = h17.type;
            const l17 = h17.enum ? 'select' : this.inputType[k17];
            let m17: FormField[] | FormField[][] = [];
            if (k17 === 'object' && Object.keys(h17).includes('properties')) {
                const q17 = this.findProperties(h17);
                m17 = this.changeProperties(q17, f17) as FormField[];
            }
            else if (k17 === 'array' && Object.keys(h17).includes('items')) {
                if (g17 === 0) {
                    this.countArr = [0];
                }
                let p17 = this.findProperties(h17);
                if (Object.keys(p17).length === 0) {
                    p17 = { 'items': { name: '', type: h17.items?.type ?? 'string' } as propertiesInfo };
                }
                m17 = [this.changeProperties(p17, f17, 'array')] as FormField[][];
            }
            const n17 = h17.description;
            const o17: FormField = {
                key: i17,
                name: j17,
                type: k17,
                inputType: l17,
                description: n17,
            };
            if (h17.enum) {
                o17.options = h17.enum;
            }
            if (h17.maxLength) {
                o17.maxLength = h17.maxLength;
            }
            if (h17.default) {
                o17.default = h17.default;
            }
            if (c17) {
                o17.fatherKey = c17;
            }
            if (d17) {
                o17.fatherType = d17;
                o17.index = 0;
            }
            if (this.required?.includes(f17)) {
                o17.required = true;
            }
            if (m17.length > 0) {
                if (d17 && d17 === 'array') {
                    o17.subProperties = m17;
                }
                else {
                    o17.subProperties = m17;
                }
            }
            e17.push(o17);
        });
        return e17;
    }
    findProperties(y16: object): Record<string, propertiesInfo> {
        let z16: Record<string, propertiesInfo> = {};
        Object.keys(y16).forEach((a17: string) => {
            if (a17 === 'properties') {
                z16 = y16['properties'];
            }
            else if (typeof y16[a17] === 'object' && a17 !== 'propertyNames') {
                z16 = this.findProperties(y16[a17]);
            }
        });
        return z16;
    }
    async executeLinkIntent(t16: Record<string, Object>) {
        let u16: insightIntentDriver.ExecuteParam = {
            bundleName: this.intentDetail?.bundleName ?? '',
            moduleName: this.intentDetail?.moduleName ?? '',
            insightIntentName: this.intentDetail?.intentName ?? '',
            insightIntentParam: t16,
            executeMode: insightIntent.ExecuteMode.UI_ABILITY_FOREGROUND,
            abilityName: this.abilityName
        };
        if (this.intentDetail?.intentType === '@InsightIntentLink') {
            u16.uris = [this.uri];
        }
        if (this.intentDetail?.intentType === '@InsightIntentLink' ||
            this.intentDetail?.intentType === '@InsightIntentPage') {
            u16.executeMode = insightIntent.ExecuteMode.UI_ABILITY_FOREGROUND;
        }
        if (this.intentDetail?.intentType === '@InsightIntentFunctionMethod') {
            u16.executeMode = insightIntent.ExecuteMode.UI_ABILITY_BACKGROUND;
        }
        if (this.intentDetail?.intentType === '@InsightIntentEntry') {
            u16.executeMode = this.intentDetail.subIntentInfo['executeMode'][0];
            hilog.info(DOMAIN, TAG, `InsightIntentEntry detected: intentName=${this.intentDetail.intentName}, executeMode=${u16.executeMode}, subIntentInfo=${JSON.stringify(this.intentDetail.subIntentInfo)}`);
        }
        if (this.intentDetail?.intentType === '@InsightIntentEntry' &&
            this.intentDetail.subIntentInfo['executeMode'] && this.intentDetail.subIntentInfo['executeMode'][0] === 2) {
            const x16 = await this.getUIExtensionType(u16.bundleName, u16.moduleName, u16.abilityName);
            hilog.info(DOMAIN, TAG, `IntentName: ${this.intentDetail.intentName}, uiExtensionType: ${x16.type}, uiExtensionName: ${x16.name}`);
            this.want = {
                bundleName: u16.bundleName,
                abilityName: u16.abilityName,
                moduleName: u16.moduleName,
                parameters: {
                    'ability.want.params.uiExtensionType': x16.type,
                    'ohos.insightIntent.executeParam.name': this.intentDetail.intentName,
                    'ohos.insightIntent.executeParam.param': u16.insightIntentParam,
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
                let w16: insightIntent.ExecuteResult = await insightIntentDriver.execute(u16);
                this.result = 'code: ' + w16.code + '\n' + 'result: ' + JSON.stringify(w16?.result, null, 2);
                if (this.result != '') {
                    this.isShow = true;
                }
            }
            catch (v16) {
                this.result = 'code: ' + JSON.stringify(v16.code) + '\n' + JSON.stringify(u16, null, 2);
                if (this.result != '') {
                    this.isShow = true;
                }
            }
        }
    }
    submitForm() {
        const q16 = format(this.formData);
        let r16: boolean = true;
        this.required?.forEach((s16: string) => {
            if (q16[s16] === undefined || q16[s16] === null) {
                if (r16) {
                    r16 = false;
                    hilog.error(DOMAIN, TAG, `submitForm validation failed: required field '${s16}' is missing`);
                }
            }
        });
        if (r16) {
            this.executeLinkIntent(q16 as Record<string, Object>);
        }
    }
    resultModule(b16 = null) {
        this.observeComponentCreation((o16, p16) => {
            ViewStackProcessor.StartGetAccessRecordingFor(o16);
            Column.create();
            Column.width('100%');
            Column.height('100%');
            if (!p16) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((c16, d16) => {
            ViewStackProcessor.StartGetAccessRecordingFor(c16);
            If.create();
            if (this.isShowEntry2) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation((m16, n16) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(m16);
                        Column.create();
                        Column.height('100%');
                        Column.width('100%');
                        Column.padding({
                            top: 28,
                            left: 22,
                            right: 22,
                            bottom: 48
                        });
                        if (!n16) {
                            Column.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    this.observeComponentCreation((k16, l16) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(k16);
                        UIExtensionComponent.create(this.want);
                        __UIExtensionComponent__extStyle();
                        if (!l16) {
                            UIExtensionComponent.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    Column.pop();
                });
            }
            else if (this.isShowForm) {
                this.ifElseBranchUpdateFunction(1, () => {
                    this.observeComponentCreation((i16, j16) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(i16);
                        Column.create();
                        Column.padding({
                            top: 28,
                            left: 22,
                            right: 22,
                            bottom: 48
                        });
                        Column.height('100%');
                        Column.width('100%');
                        if (!j16) {
                            Column.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    this.observeComponentCreation((g16, h16) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(g16);
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
                        if (!h16) {
                            FormComponent.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    Column.pop();
                });
            }
            else {
                this.ifElseBranchUpdateFunction(2, () => {
                    this.observeComponentCreation((e16, f16) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(e16);
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
                        if (!f16) {
                            Text.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    Text.pop();
                });
            }
            if (!d16) {
                If.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        If.pop();
        Column.pop();
    }
    buildTextInput(k15: FormField, l15 = null) {
        this.observeComponentCreation((z15, a16) => {
            ViewStackProcessor.StartGetAccessRecordingFor(z15);
            Flex.create({ direction: FlexDirection.Row, alignItems: ItemAlign.Start });
            Flex.margin({ bottom: 12 });
            if (!a16) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((x15, y15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(x15);
            Text.create(k15.name);
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
            if (!y15) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((v15, w15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(v15);
            Column.create();
            Column.flexGrow(1);
            if (!w15) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((s15, t15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(s15);
            TextInput.create({
                placeholder: `${k15.type} ${k15 && k15.description ? '，' + k15.description : ''}`,
                text: `${this.formData[k15.key] ?? ''}`
            });
            TextInput.maxLength(k15.maxLength);
            TextInput.width('100%');
            TextInput.height(40);
            TextInput.fontSize(16);
            TextInput.fontColor({ "id": 16777249, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            TextInput.padding({ right: 16, left: 16 });
            TextInput.backgroundColor({ "id": 16777239, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            TextInput.onChange((u15: string) => {
                this.formData[k15.key] = u15;
            });
            if (!t15) {
                TextInput.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((m15, n15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(m15);
            If.create();
            if (k15.required) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation((q15, r15) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(q15);
                        Row.create();
                        Row.width('100%');
                        Row.padding({ top: 5, right: 10 });
                        if (!r15) {
                            Row.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    this.observeComponentCreation((o15, p15) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(o15);
                        Text.create({ "id": 16777234, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                        Text.fontSize(12);
                        Text.lineHeight(16);
                        Text.fontColor({ "id": 16777246, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                        Text.padding({ left: 10 });
                        if (!p15) {
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
            if (!n15) {
                If.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        If.pop();
        Column.pop();
        Flex.pop();
    }
    buildNumberInput(t14: FormField, u14 = null) {
        this.observeComponentCreation((i15, j15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(i15);
            Flex.create({ direction: FlexDirection.Row, alignItems: ItemAlign.Start });
            Flex.height(50);
            Flex.margin({ bottom: 15 });
            if (!j15) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((g15, h15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(g15);
            Text.create(t14.name);
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
            if (!h15) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((e15, f15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(e15);
            Column.create();
            Column.flexGrow(1);
            if (!f15) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((b15, c15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(b15);
            TextInput.create({
                placeholder: `${t14.type} ${t14 && t14.description ? '，' + t14.description : ''}`,
                text: `${this.formData[t14.key] ?? ''}`
            });
            TextInput.maxLength(t14.maxLength);
            TextInput.type(InputType.Number);
            TextInput.width('100%');
            TextInput.height(40);
            TextInput.fontSize(16);
            TextInput.fontColor({ "id": 16777249, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            TextInput.padding({ right: 16, left: 16 });
            TextInput.backgroundColor({ "id": 16777239, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            TextInput.onChange((d15: string) => {
                this.formData[t14.key] = parseInt(d15);
            });
            if (!c15) {
                TextInput.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((v14, w14) => {
            ViewStackProcessor.StartGetAccessRecordingFor(v14);
            If.create();
            if (t14.required) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation((z14, a15) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(z14);
                        Row.create();
                        Row.width('100%');
                        Row.padding({ top: 5, right: 10 });
                        if (!a15) {
                            Row.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    this.observeComponentCreation((x14, y14) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(x14);
                        Text.create({ "id": 16777234, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                        Text.fontSize(12);
                        Text.lineHeight(16);
                        Text.fontColor({ "id": 16777246, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                        Text.padding({ left: 10 });
                        if (!y14) {
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
            if (!w14) {
                If.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        If.pop();
        Column.pop();
        Flex.pop();
    }
    buildSelectInput(w13: FormField, x13 = null) {
        this.observeComponentCreation((r14, s14) => {
            ViewStackProcessor.StartGetAccessRecordingFor(r14);
            Flex.create({ direction: FlexDirection.Row, alignItems: ItemAlign.Start });
            Flex.margin({ bottom: 20 });
            if (!s14) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((p14, q14) => {
            ViewStackProcessor.StartGetAccessRecordingFor(p14);
            Text.create(w13.name);
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
            if (!q14) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((n14, o14) => {
            ViewStackProcessor.StartGetAccessRecordingFor(n14);
            Column.create();
            Column.flexGrow(1);
            if (!o14) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((g14, h14) => {
            ViewStackProcessor.StartGetAccessRecordingFor(g14);
            Select.create(w13.options?.map(l14 => {
                const m14: SelectOption = { value: l14.toString() };
                return m14;
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
            Select.onSelect((i14: number, j14: string) => {
                const k14 = w13.type === 'string' ? j14 : parseInt(j14);
                this.formData[w13.key] = k14;
            });
            if (!h14) {
                Select.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Select.pop();
        this.observeComponentCreation((e14, f14) => {
            ViewStackProcessor.StartGetAccessRecordingFor(e14);
            Row.create();
            Row.width('100%');
            Row.padding({ top: 5, right: 10 });
            if (!f14) {
                Row.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((a14, b14) => {
            ViewStackProcessor.StartGetAccessRecordingFor(a14);
            If.create();
            if (w13.required) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation((c14, d14) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(c14);
                        Text.create({ "id": 16777234, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                        Text.fontSize(12);
                        Text.lineHeight(16);
                        Text.fontColor({ "id": 16777246, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                        Text.padding({ left: 10 });
                        if (!d14) {
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
            if (!b14) {
                If.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        If.pop();
        this.observeComponentCreation((y13, z13) => {
            ViewStackProcessor.StartGetAccessRecordingFor(y13);
            Text.create(w13.description);
            Text.fontSize(12);
            Text.lineHeight(16);
            Text.fontColor({ "id": 16777247, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.padding({ left: 10 });
            if (!z13) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Row.pop();
        Column.pop();
        Flex.pop();
    }
    buildFormFields(f12: FormField[] | FormField[][], g12 = null) {
        this.observeComponentCreation((u13, v13) => {
            ViewStackProcessor.StartGetAccessRecordingFor(u13);
            Column.create();
            if (!v13) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((h12, i12) => {
            ViewStackProcessor.StartGetAccessRecordingFor(h12);
            ForEach.create();
            const j12 = (k12, l12: number) => {
                const m12 = k12;
                this.observeComponentCreation((n12, o12) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(n12);
                    If.create();
                    if (m12.inputType === 'text') {
                        this.ifElseBranchUpdateFunction(0, () => {
                            this.buildTextInput.bind(this)(m12);
                        });
                    }
                    else if (m12.inputType === 'number') {
                        this.ifElseBranchUpdateFunction(1, () => {
                            this.buildNumberInput.bind(this)(m12);
                        });
                    }
                    else if (m12.inputType === 'select') {
                        this.ifElseBranchUpdateFunction(2, () => {
                            this.buildSelectInput.bind(this)(m12);
                        });
                    }
                    else if (m12.inputType === 'object') {
                        this.ifElseBranchUpdateFunction(3, () => {
                            this.observeComponentCreation((m13, n13) => {
                                ViewStackProcessor.StartGetAccessRecordingFor(m13);
                                If.create();
                                if (m12.inputType === 'object' && !m12.subProperties) {
                                    this.ifElseBranchUpdateFunction(0, () => {
                                        this.buildTextInput.bind(this)(m12);
                                    });
                                }
                                else {
                                    this.ifElseBranchUpdateFunction(1, () => {
                                        this.observeComponentCreation((s13, t13) => {
                                            ViewStackProcessor.StartGetAccessRecordingFor(s13);
                                            Column.create();
                                            if (!t13) {
                                                Column.pop();
                                            }
                                            ViewStackProcessor.StopGetAccessRecording();
                                        });
                                        this.observeComponentCreation((q13, r13) => {
                                            ViewStackProcessor.StartGetAccessRecordingFor(q13);
                                            Row.create();
                                            Row.margin({ bottom: 8, top: 16 });
                                            if (!r13) {
                                                Row.pop();
                                            }
                                            ViewStackProcessor.StopGetAccessRecording();
                                        });
                                        this.observeComponentCreation((o13, p13) => {
                                            ViewStackProcessor.StartGetAccessRecordingFor(o13);
                                            Text.create(m12.name);
                                            Text.fontSize(14);
                                            Text.lineHeight(20);
                                            Text.fontWeight(500);
                                            Text.fontColor({ "id": 16777248, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                            Text.width('100%');
                                            Text.padding({ left: 12 });
                                            if (!p13) {
                                                Text.pop();
                                            }
                                            ViewStackProcessor.StopGetAccessRecording();
                                        });
                                        Text.pop();
                                        Row.pop();
                                        this.buildFormFields.bind(this)(m12.subProperties as FormField[]);
                                        Column.pop();
                                    });
                                }
                                if (!n13) {
                                    If.pop();
                                }
                                ViewStackProcessor.StopGetAccessRecording();
                            });
                            If.pop();
                        });
                    }
                    else if (m12.inputType === 'array') {
                        this.ifElseBranchUpdateFunction(4, () => {
                            this.observeComponentCreation((p12, q12) => {
                                ViewStackProcessor.StartGetAccessRecordingFor(p12);
                                ForEach.create();
                                const r12 = (s12, t12: number) => {
                                    const u12 = s12;
                                    this.observeComponentCreation((k13, l13) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(k13);
                                        Column.create();
                                        if (!l13) {
                                            Column.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    this.observeComponentCreation((i13, j13) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(i13);
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
                                        if (!j13) {
                                            Flex.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    this.observeComponentCreation((g13, h13) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(g13);
                                        Text.create(m12.name);
                                        Text.fontSize(14);
                                        Text.fontWeight(500);
                                        Text.fontColor({ "id": 16777248, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                        Text.width('100%');
                                        Text.lineHeight(20);
                                        if (!h13) {
                                            Text.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    Text.pop();
                                    this.observeComponentCreation((v12, w12) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(v12);
                                        If.create();
                                        if (t12 === 0) {
                                            this.ifElseBranchUpdateFunction(0, () => {
                                                this.observeComponentCreation((c13, d13) => {
                                                    ViewStackProcessor.StartGetAccessRecordingFor(c13);
                                                    Text.create({ "id": 16777222, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                                    Text.fontSize(14);
                                                    Text.fontColor('rgb(10,89,247)');
                                                    Text.fontWeight(500);
                                                    Text.lineHeight(19);
                                                    Text.onClick(() => {
                                                        if (m12.maxLength && m12.subProperties && (m12.subProperties?.length >= m12.maxLength)) {
                                                            this.getUIContext().showAlertDialog({
                                                                message: `最大节点数不超过${m12.maxLength}个`,
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
                                                            const e13: FormField[][] = addArrayParams(m12.subProperties, m12.name, this.countArr);
                                                            let f13 = jsClone(this.formFields) as FormField[];
                                                            f13[l12].subProperties = e13;
                                                            this.formFields = f13;
                                                        }
                                                    });
                                                    if (!d13) {
                                                        Text.pop();
                                                    }
                                                    ViewStackProcessor.StopGetAccessRecording();
                                                });
                                                Text.pop();
                                            });
                                        }
                                        else {
                                            this.ifElseBranchUpdateFunction(1, () => {
                                                this.observeComponentCreation((x12, y12) => {
                                                    ViewStackProcessor.StartGetAccessRecordingFor(x12);
                                                    Text.create({ "id": 16777221, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                                    Text.fontSize(14);
                                                    Text.fontColor('rgb(10,89,247)');
                                                    Text.fontWeight(500);
                                                    Text.lineHeight(19);
                                                    Text.onClick(() => {
                                                        const z12: FormDataType = removeFormData(this.formData, m12.name, u12[0].index);
                                                        this.formData = z12;
                                                        const a13: FormField[][] = removeArrayParams(m12.subProperties, m12.name, t12);
                                                        let b13 = jsClone(this.formFields) as FormField[];
                                                        b13[l12].subProperties = a13;
                                                        this.formFields = b13;
                                                    });
                                                    if (!y12) {
                                                        Text.pop();
                                                    }
                                                    ViewStackProcessor.StopGetAccessRecording();
                                                });
                                                Text.pop();
                                            });
                                        }
                                        if (!w12) {
                                            If.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    If.pop();
                                    Flex.pop();
                                    this.buildFormFields.bind(this)(u12 as FormField[]);
                                    Column.pop();
                                };
                                this.forEachUpdateFunction(p12, m12.subProperties, r12, undefined, true, false);
                                if (!q12) {
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
                    if (!o12) {
                        If.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                If.pop();
            };
            this.forEachUpdateFunction(h12, f12, j12, undefined, true, false);
            if (!i12) {
                ForEach.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        ForEach.pop();
        Column.pop();
    }
    initialRender() {
        this.observeComponentCreation((p9, q9) => {
            ViewStackProcessor.StartGetAccessRecordingFor(p9);
            NavDestination.create(() => {
                this.observeComponentCreation((d12, e12) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(d12);
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
                    if (!e12) {
                        Row.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                this.observeComponentCreation((b12, c12) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(b12);
                    Column.create();
                    Column.width(40);
                    Column.height(40);
                    Column.borderRadius(20);
                    Column.backgroundColor({ "id": 16777245, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                    Column.justifyContent(FlexAlign.Center);
                    if (!c12) {
                        Column.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                this.observeComponentCreation((z11, a12) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(z11);
                    Image.create({ "id": 16777255, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                    Image.width(8.68);
                    Image.height(16.48);
                    Image.opacity(0.6);
                    if (!a12) {
                        Image.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                Column.pop();
                this.observeComponentCreation((x11, y11) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(x11);
                    Text.create({ "id": 16777228, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                    Text.margin({ left: 8 });
                    Text.fontSize(20);
                    Text.fontWeight(700);
                    Text.fontColor({ "id": 16777249, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                    if (!y11) {
                        Text.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                Text.pop();
                Row.pop();
                this.observeComponentCreation((v11, w11) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(v11);
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
                    if (!w11) {
                        Scroll.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                this.observeComponentCreation((t11, u11) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(t11);
                    Column.create();
                    Column.width('100%');
                    Column.alignItems(HorizontalAlign.Start);
                    if (!u11) {
                        Column.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                {
                    this.observeComponentCreation((p11, q11) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(p11);
                        if (q11) {
                            let r11 = new DetailItem(this, { intentDetail: this.intentDetail }, undefined, p11, () => { }, { page: "feature/src/main/ets/view/IntentDetailPage.ets", line: 731, col: 11 });
                            ViewPU.create(r11);
                            let s11 = () => {
                                return {
                                    intentDetail: this.intentDetail
                                };
                            };
                            r11.paramsGenerator_ = s11;
                        }
                        else {
                            this.updateStateVarsOfChildByElmtId(p11, {
                                intentDetail: this.intentDetail
                            });
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                }
                this.observeComponentCreation((n11, o11) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(n11);
                    Column.create();
                    Column.padding({ bottom: 86 });
                    Column.margin({ left: 16, right: 16 });
                    if (!o11) {
                        Column.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                this.observeComponentCreation((l11, m11) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(l11);
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
                    if (!m11) {
                        Text.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                Text.pop();
                this.observeComponentCreation((w9, x9) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(w9);
                    If.create();
                    if (this.intentParameters !== null) {
                        this.ifElseBranchUpdateFunction(0, () => {
                            this.observeComponentCreation((y9, z9) => {
                                ViewStackProcessor.StartGetAccessRecordingFor(y9);
                                ForEach.create();
                                const a10 = (b10, c10: number) => {
                                    const d10 = b10;
                                    this.observeComponentCreation((e10, f10) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(e10);
                                        If.create();
                                        if (d10.inputType === 'text') {
                                            this.ifElseBranchUpdateFunction(0, () => {
                                                this.buildTextInput.bind(this)(d10);
                                            });
                                        }
                                        else if (d10.inputType === 'number') {
                                            this.ifElseBranchUpdateFunction(1, () => {
                                                this.buildNumberInput.bind(this)(d10);
                                            });
                                        }
                                        else if (d10.inputType === 'select') {
                                            this.ifElseBranchUpdateFunction(2, () => {
                                                this.buildSelectInput.bind(this)(d10);
                                            });
                                        }
                                        else if (d10.inputType === 'object') {
                                            this.ifElseBranchUpdateFunction(3, () => {
                                                this.observeComponentCreation((d11, e11) => {
                                                    ViewStackProcessor.StartGetAccessRecordingFor(d11);
                                                    If.create();
                                                    if (d10.inputType === 'object' && !d10.subProperties) {
                                                        this.ifElseBranchUpdateFunction(0, () => {
                                                            this.buildTextInput.bind(this)(d10);
                                                        });
                                                    }
                                                    else {
                                                        this.ifElseBranchUpdateFunction(1, () => {
                                                            this.observeComponentCreation((j11, k11) => {
                                                                ViewStackProcessor.StartGetAccessRecordingFor(j11);
                                                                Column.create();
                                                                if (!k11) {
                                                                    Column.pop();
                                                                }
                                                                ViewStackProcessor.StopGetAccessRecording();
                                                            });
                                                            this.observeComponentCreation((h11, i11) => {
                                                                ViewStackProcessor.StartGetAccessRecordingFor(h11);
                                                                Row.create();
                                                                Row.margin({ bottom: 8, top: 16 });
                                                                if (!i11) {
                                                                    Row.pop();
                                                                }
                                                                ViewStackProcessor.StopGetAccessRecording();
                                                            });
                                                            this.observeComponentCreation((f11, g11) => {
                                                                ViewStackProcessor.StartGetAccessRecordingFor(f11);
                                                                Text.create(d10.name);
                                                                Text.fontSize(14);
                                                                Text.lineHeight(20);
                                                                Text.fontWeight(500);
                                                                Text.fontColor({ "id": 16777248, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                                                Text.width('100%');
                                                                Text.padding({ left: 12 });
                                                                if (!g11) {
                                                                    Text.pop();
                                                                }
                                                                ViewStackProcessor.StopGetAccessRecording();
                                                            });
                                                            Text.pop();
                                                            Row.pop();
                                                            this.buildFormFields.bind(this)(d10.subProperties as FormField[]);
                                                            Column.pop();
                                                        });
                                                    }
                                                    if (!e11) {
                                                        If.pop();
                                                    }
                                                    ViewStackProcessor.StopGetAccessRecording();
                                                });
                                                If.pop();
                                            });
                                        }
                                        else if (d10.inputType === 'array') {
                                            this.ifElseBranchUpdateFunction(4, () => {
                                                this.observeComponentCreation((g10, h10) => {
                                                    ViewStackProcessor.StartGetAccessRecordingFor(g10);
                                                    ForEach.create();
                                                    const i10 = (j10, k10: number) => {
                                                        const l10 = j10;
                                                        this.observeComponentCreation((b11, c11) => {
                                                            ViewStackProcessor.StartGetAccessRecordingFor(b11);
                                                            Column.create();
                                                            if (!c11) {
                                                                Column.pop();
                                                            }
                                                            ViewStackProcessor.StopGetAccessRecording();
                                                        });
                                                        this.observeComponentCreation((z10, a11) => {
                                                            ViewStackProcessor.StartGetAccessRecordingFor(z10);
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
                                                            if (!a11) {
                                                                Flex.pop();
                                                            }
                                                            ViewStackProcessor.StopGetAccessRecording();
                                                        });
                                                        this.observeComponentCreation((x10, y10) => {
                                                            ViewStackProcessor.StartGetAccessRecordingFor(x10);
                                                            Text.create(d10.name);
                                                            Text.fontSize(14);
                                                            Text.fontColor({ "id": 16777248, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                                            Text.width('50%');
                                                            Text.lineHeight(20);
                                                            Text.fontWeight(500);
                                                            if (!y10) {
                                                                Text.pop();
                                                            }
                                                            ViewStackProcessor.StopGetAccessRecording();
                                                        });
                                                        Text.pop();
                                                        this.observeComponentCreation((m10, n10) => {
                                                            ViewStackProcessor.StartGetAccessRecordingFor(m10);
                                                            If.create();
                                                            if (k10 === 0) {
                                                                this.ifElseBranchUpdateFunction(0, () => {
                                                                    this.observeComponentCreation((t10, u10) => {
                                                                        ViewStackProcessor.StartGetAccessRecordingFor(t10);
                                                                        Text.create({ "id": 16777222, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                                                        Text.fontSize(14);
                                                                        Text.fontColor('rgb(10,89,247)');
                                                                        Text.fontWeight(500);
                                                                        Text.lineHeight(19);
                                                                        Text.onClick(() => {
                                                                            if (d10.maxLength && d10.subProperties &&
                                                                                (d10.subProperties?.length >= d10.maxLength)) {
                                                                                this.getUIContext().showAlertDialog({
                                                                                    message: `最大节点不超过${d10.maxLength}个`,
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
                                                                                const v10: FormField[][] = addArrayParams(d10.subProperties, d10.name, this.countArr);
                                                                                let w10 = jsClone(this.formFields) as FormField[];
                                                                                w10[c10].subProperties = v10;
                                                                                this.formFields = w10;
                                                                            }
                                                                        });
                                                                        if (!u10) {
                                                                            Text.pop();
                                                                        }
                                                                        ViewStackProcessor.StopGetAccessRecording();
                                                                    });
                                                                    Text.pop();
                                                                });
                                                            }
                                                            else {
                                                                this.ifElseBranchUpdateFunction(1, () => {
                                                                    this.observeComponentCreation((o10, p10) => {
                                                                        ViewStackProcessor.StartGetAccessRecordingFor(o10);
                                                                        Text.create({ "id": 16777221, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                                                        Text.fontSize(14);
                                                                        Text.fontColor('rgb(10,89,247)');
                                                                        Text.fontWeight(500);
                                                                        Text.lineHeight(19);
                                                                        Text.onClick(() => {
                                                                            const q10: FormDataType = removeFormData(this.formData, d10.name, l10[0].index);
                                                                            this.formData = q10;
                                                                            const r10: FormField[][] = removeArrayParams(d10.subProperties, d10.name, k10);
                                                                            let s10 = jsClone(this.formFields) as FormField[];
                                                                            s10[c10].subProperties = r10;
                                                                            this.formFields = s10;
                                                                        });
                                                                        if (!p10) {
                                                                            Text.pop();
                                                                        }
                                                                        ViewStackProcessor.StopGetAccessRecording();
                                                                    });
                                                                    Text.pop();
                                                                });
                                                            }
                                                            if (!n10) {
                                                                If.pop();
                                                            }
                                                            ViewStackProcessor.StopGetAccessRecording();
                                                        });
                                                        If.pop();
                                                        Flex.pop();
                                                        this.buildFormFields.bind(this)(l10 as FormField[]);
                                                        Column.pop();
                                                    };
                                                    this.forEachUpdateFunction(g10, d10.subProperties, i10, undefined, true, false);
                                                    if (!h10) {
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
                                        if (!f10) {
                                            If.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    If.pop();
                                };
                                this.forEachUpdateFunction(y9, this.formFields, a10, undefined, true, false);
                                if (!z9) {
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
                    if (!x9) {
                        If.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                If.pop();
                Column.pop();
                Column.pop();
                Scroll.pop();
                this.observeComponentCreation((u9, v9) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(u9);
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
                    if (!v9) {
                        Row.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                this.observeComponentCreation((r9, s9) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(r9);
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
                    Button.bindSheet({ value: this.isShow, changeEvent: t9 => { this.isShow = t9; } }, { builder: () => {
                            this.resultModule.call(this);
                        } }, {
                        height: SheetSize.MEDIUM,
                        blurStyle: BlurStyle.Thick,
                        title: { title: { "id": 16777225, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" } },
                        preferType: SheetType.BOTTOM
                    });
                    if (!s9) {
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
            if (!q9) {
                NavDestination.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        NavDestination.pop();
    }
    rerender() {
        this.updateDirtyElements();
    }
}
