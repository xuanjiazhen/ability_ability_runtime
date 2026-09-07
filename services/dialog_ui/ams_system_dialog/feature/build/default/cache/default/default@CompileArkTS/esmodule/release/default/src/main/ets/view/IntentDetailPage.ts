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
    constructor(m19, n19, o19, p19 = -1, q19 = undefined, r19) {
        super(m19, o19, p19, r19);
        if (typeof q19 === "function") {
            this.paramsGenerator_ = q19;
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
        this.setInitiallyProvidedValue(n19);
        this.finalizeConstruction();
    }
    setInitiallyProvidedValue(l19: IntentDetailPage_Params) {
        if (l19.intentDetail !== undefined) {
            this.intentDetail = l19.intentDetail;
        }
        if (l19.intentDetailMap !== undefined) {
            this.intentDetailMap = l19.intentDetailMap;
        }
        if (l19.intentDetailEntries !== undefined) {
            this.intentDetailEntries = l19.intentDetailEntries;
        }
        if (l19.intentParameters !== undefined) {
            this.intentParameters = l19.intentParameters;
        }
        if (l19.scroller !== undefined) {
            this.scroller = l19.scroller;
        }
        if (l19.required !== undefined) {
            this.required = l19.required;
        }
        if (l19.formData !== undefined) {
            this.formData = l19.formData;
        }
        if (l19.properties !== undefined) {
            this.properties = l19.properties;
        }
        if (l19.formFields !== undefined) {
            this.formFields = l19.formFields;
        }
        if (l19.abilityName !== undefined) {
            this.abilityName = l19.abilityName;
        }
        if (l19.uri !== undefined) {
            this.uri = l19.uri;
        }
        if (l19.formName !== undefined) {
            this.formName = l19.formName;
        }
        if (l19.countArr !== undefined) {
            this.countArr = l19.countArr;
        }
        if (l19.result !== undefined) {
            this.result = l19.result;
        }
        if (l19.isShow !== undefined) {
            this.isShow = l19.isShow;
        }
        if (l19.isShowEntry2 !== undefined) {
            this.isShowEntry2 = l19.isShowEntry2;
        }
        if (l19.isShowForm !== undefined) {
            this.isShowForm = l19.isShowForm;
        }
        if (l19.want !== undefined) {
            this.want = l19.want;
        }
        if (l19.inputType !== undefined) {
            this.inputType = l19.inputType;
        }
        if (l19.InputTypeData !== undefined) {
            this.InputTypeData = l19.InputTypeData;
        }
    }
    updateStateVars(k19: IntentDetailPage_Params) {
    }
    purgeVariableDependenciesOnElmtId(j19) {
        this.__articlePathStack.purgeDependencyOnElmtId(j19);
        this.__intentDetail.purgeDependencyOnElmtId(j19);
        this.__intentDetailMap.purgeDependencyOnElmtId(j19);
        this.__intentDetailEntries.purgeDependencyOnElmtId(j19);
        this.__intentParameters.purgeDependencyOnElmtId(j19);
        this.__required.purgeDependencyOnElmtId(j19);
        this.__formData.purgeDependencyOnElmtId(j19);
        this.__properties.purgeDependencyOnElmtId(j19);
        this.__formFields.purgeDependencyOnElmtId(j19);
        this.__abilityName.purgeDependencyOnElmtId(j19);
        this.__uri.purgeDependencyOnElmtId(j19);
        this.__formName.purgeDependencyOnElmtId(j19);
        this.__countArr.purgeDependencyOnElmtId(j19);
        this.__result.purgeDependencyOnElmtId(j19);
        this.__isShow.purgeDependencyOnElmtId(j19);
        this.__isShowEntry2.purgeDependencyOnElmtId(j19);
        this.__isShowForm.purgeDependencyOnElmtId(j19);
        this.__want.purgeDependencyOnElmtId(j19);
        this.__InputTypeData.purgeDependencyOnElmtId(j19);
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
    set articlePathStack(i19: NavPathStack) {
        this.__articlePathStack.set(i19);
    }
    private __intentDetail: ObservedPropertyObjectPU<IntentInfo | null>;
    get intentDetail() {
        return this.__intentDetail.get();
    }
    set intentDetail(h19: IntentInfo | null) {
        this.__intentDetail.set(h19);
    }
    private __intentDetailMap: ObservedPropertyObjectPU<Map<string, string> | null>;
    get intentDetailMap() {
        return this.__intentDetailMap.get();
    }
    set intentDetailMap(g19: Map<string, string> | null) {
        this.__intentDetailMap.set(g19);
    }
    private __intentDetailEntries: ObservedPropertyObjectPU<Array<[
        string,
        string | string[] | object
    ]> | [
    ]>;
    get intentDetailEntries() {
        return this.__intentDetailEntries.get();
    }
    set intentDetailEntries(f19: Array<[
        string,
        string | string[] | object
    ]> | [
    ]) {
        this.__intentDetailEntries.set(f19);
    }
    private __intentParameters: ObservedPropertyObjectPU<object | null>;
    get intentParameters() {
        return this.__intentParameters.get();
    }
    set intentParameters(e19: object | null) {
        this.__intentParameters.set(e19);
    }
    private scroller: Scroller;
    private __required: ObservedPropertyObjectPU<string[]>;
    get required() {
        return this.__required.get();
    }
    set required(d19: string[]) {
        this.__required.set(d19);
    }
    private __formData: ObservedPropertyObjectPU<FormDataType>;
    get formData() {
        return this.__formData.get();
    }
    set formData(c19: FormDataType) {
        this.__formData.set(c19);
    }
    private __properties: ObservedPropertyObjectPU<Record<string, propertiesInfo>>;
    get properties() {
        return this.__properties.get();
    }
    set properties(b19: Record<string, propertiesInfo>) {
        this.__properties.set(b19);
    }
    private __formFields: ObservedPropertyObjectPU<FormField[]>;
    get formFields() {
        return this.__formFields.get();
    }
    set formFields(a19: FormField[]) {
        this.__formFields.set(a19);
    }
    private __abilityName: ObservedPropertySimplePU<string>;
    get abilityName() {
        return this.__abilityName.get();
    }
    set abilityName(z18: string) {
        this.__abilityName.set(z18);
    }
    private __uri: ObservedPropertySimplePU<string>;
    get uri() {
        return this.__uri.get();
    }
    set uri(y18: string) {
        this.__uri.set(y18);
    }
    private __formName: ObservedPropertySimplePU<string>;
    get formName() {
        return this.__formName.get();
    }
    set formName(x18: string) {
        this.__formName.set(x18);
    }
    private __countArr: ObservedPropertyObjectPU<number[]>;
    get countArr() {
        return this.__countArr.get();
    }
    set countArr(w18: number[]) {
        this.__countArr.set(w18);
    }
    private __result: ObservedPropertySimplePU<string>;
    get result() {
        return this.__result.get();
    }
    set result(v18: string) {
        this.__result.set(v18);
    }
    private __isShow: ObservedPropertySimplePU<boolean>;
    get isShow() {
        return this.__isShow.get();
    }
    set isShow(u18: boolean) {
        this.__isShow.set(u18);
    }
    private __isShowEntry2: ObservedPropertySimplePU<boolean>;
    get isShowEntry2() {
        return this.__isShowEntry2.get();
    }
    set isShowEntry2(t18: boolean) {
        this.__isShowEntry2.set(t18);
    }
    private __isShowForm: ObservedPropertySimplePU<boolean>;
    get isShowForm() {
        return this.__isShowForm.get();
    }
    set isShowForm(s18: boolean) {
        this.__isShowForm.set(s18);
    }
    private __want: ObservedPropertyObjectPU<Want>;
    get want() {
        return this.__want.get();
    }
    set want(r18: Want) {
        this.__want.set(r18);
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
        let q18 = AppStorage.get('context') as common.UIAbilityContext;
        if (q18) {
            q18.terminateSelf();
        }
    }
    private __InputTypeData: ObservedPropertyObjectPU<Record<string, InputType>>;
    get InputTypeData() {
        return this.__InputTypeData.get();
    }
    set InputTypeData(p18: Record<string, InputType>) {
        this.__InputTypeData.set(p18);
    }
    getAttribute(n18: object | undefined, o18: string): string | string[] {
        if (n18 && Object.keys(n18).includes(o18)) {
            return n18[o18];
        }
        else {
            return '';
        }
    }
    async getUIExtensionType(z17: string, a18: string, b18: string): Promise<UIExtensionInfo> {
        hilog.info(DOMAIN, TAG, `getUIExtensionType start: bundleName=${z17}, moduleName=${a18}, abilityName=${b18}`);
        const c18 = await bundleManager.getBundleInfo(z17, bundleManager.BundleFlag.GET_BUNDLE_INFO_WITH_HAP_MODULE |
            bundleManager.BundleFlag.GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY);
        hilog.info(DOMAIN, TAG, 'getBundleInfo success: bundleName=%{public}s', c18.name);
        const d18 = c18.hapModulesInfo;
        if (!d18) {
            const m18 = `getUIExtensionType failed: hapModulesInfo is null for bundleName=${z17}`;
            hilog.error(DOMAIN, TAG, m18);
            throw new Error(m18);
        }
        for (let f18 = 0; f18 < d18.length; f18++) {
            const g18 = d18[f18];
            if (g18.name === a18) {
                const h18 = g18.extensionAbilitiesInfo;
                if (!h18) {
                    hilog.warn(DOMAIN, TAG, `extensionAbilitiesInfo is null for module: ${a18}`);
                    continue;
                }
                for (let i18 = 0; i18 < h18.length; i18++) {
                    const j18 = h18[i18];
                    const k18 = j18.extensionAbilityTypeName;
                    if (k18 && k18 !== '') {
                        hilog.info(DOMAIN, TAG, `getUIExtensionType success: found UIExtensionAbility ${j18.name}, type=${k18}`);
                        const l18: UIExtensionInfo = {
                            type: k18,
                            name: j18.name
                        };
                        return l18;
                    }
                }
            }
        }
        const e18 = `getUIExtensionType failed: UIExtensionAbility not found for bundleName=${z17}, moduleName=${a18}, abilityName=${b18}`;
        hilog.error(DOMAIN, TAG, e18);
        throw new Error(e18);
    }
    changeProperties(j17: Record<string, propertiesInfo>, k17?: string, l17?: string) {
        const m17: FormField[] = [];
        Object.keys(j17).forEach((n17: string, o17: number) => {
            const p17 = j17[n17];
            let q17 = n17;
            if (k17) {
                if (l17 && l17 === 'array') {
                    q17 = `${k17}.array0.${n17}`;
                }
                else {
                    q17 = `${k17}.${n17}`;
                }
            }
            const r17 = n17;
            const s17 = p17.type;
            const t17 = p17.enum ? 'select' : this.inputType[s17];
            let u17: FormField[] | FormField[][] = [];
            if (s17 === 'object' && Object.keys(p17).includes('properties')) {
                const y17 = this.findProperties(p17);
                u17 = this.changeProperties(y17, n17) as FormField[];
            }
            else if (s17 === 'array' && Object.keys(p17).includes('items')) {
                if (o17 === 0) {
                    this.countArr = [0];
                }
                let x17 = this.findProperties(p17);
                if (Object.keys(x17).length === 0) {
                    x17 = { 'items': { name: '', type: p17.items?.type ?? 'string' } as propertiesInfo };
                }
                u17 = [this.changeProperties(x17, n17, 'array')] as FormField[][];
            }
            const v17 = p17.description;
            const w17: FormField = {
                key: q17,
                name: r17,
                type: s17,
                inputType: t17,
                description: v17,
            };
            if (p17.enum) {
                w17.options = p17.enum;
            }
            if (p17.maxLength) {
                w17.maxLength = p17.maxLength;
            }
            if (p17.default) {
                w17.default = p17.default;
            }
            if (k17) {
                w17.fatherKey = k17;
            }
            if (l17) {
                w17.fatherType = l17;
                w17.index = 0;
            }
            if (this.required?.includes(n17)) {
                w17.required = true;
            }
            if (u17.length > 0) {
                if (l17 && l17 === 'array') {
                    w17.subProperties = u17;
                }
                else {
                    w17.subProperties = u17;
                }
            }
            m17.push(w17);
        });
        return m17;
    }
    findProperties(g17: object): Record<string, propertiesInfo> {
        let h17: Record<string, propertiesInfo> = {};
        Object.keys(g17).forEach((i17: string) => {
            if (i17 === 'properties') {
                h17 = g17['properties'];
            }
            else if (typeof g17[i17] === 'object' && i17 !== 'propertyNames') {
                h17 = this.findProperties(g17[i17]);
            }
        });
        return h17;
    }
    async executeLinkIntent(b17: Record<string, Object>) {
        let c17: insightIntentDriver.ExecuteParam = {
            bundleName: this.intentDetail?.bundleName ?? '',
            moduleName: this.intentDetail?.moduleName ?? '',
            insightIntentName: this.intentDetail?.intentName ?? '',
            insightIntentParam: b17,
            executeMode: insightIntent.ExecuteMode.UI_ABILITY_FOREGROUND,
            abilityName: this.abilityName
        };
        if (this.intentDetail?.intentType === '@InsightIntentLink') {
            c17.uris = [this.uri];
        }
        if (this.intentDetail?.intentType === '@InsightIntentLink' ||
            this.intentDetail?.intentType === '@InsightIntentPage') {
            c17.executeMode = insightIntent.ExecuteMode.UI_ABILITY_FOREGROUND;
        }
        if (this.intentDetail?.intentType === '@InsightIntentFunctionMethod') {
            c17.executeMode = insightIntent.ExecuteMode.UI_ABILITY_BACKGROUND;
        }
        if (this.intentDetail?.intentType === '@InsightIntentEntry') {
            c17.executeMode = this.intentDetail.subIntentInfo['executeMode'][0];
            hilog.info(DOMAIN, TAG, `InsightIntentEntry detected: intentName=${this.intentDetail.intentName}, executeMode=${c17.executeMode}, subIntentInfo=${JSON.stringify(this.intentDetail.subIntentInfo)}`);
        }
        if (this.intentDetail?.intentType === '@InsightIntentEntry' &&
            this.intentDetail.subIntentInfo['executeMode'] && this.intentDetail.subIntentInfo['executeMode'][0] === 2) {
            const f17 = await this.getUIExtensionType(c17.bundleName, c17.moduleName, c17.abilityName);
            hilog.info(DOMAIN, TAG, `IntentName: ${this.intentDetail.intentName}, uiExtensionType: ${f17.type}, uiExtensionName: ${f17.name}`);
            this.want = {
                bundleName: c17.bundleName,
                abilityName: c17.abilityName,
                moduleName: c17.moduleName,
                parameters: {
                    'ability.want.params.uiExtensionType': f17.type,
                    'ohos.insightIntent.executeParam.name': this.intentDetail.intentName,
                    'ohos.insightIntent.executeParam.param': c17.insightIntentParam,
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
                let e17: insightIntent.ExecuteResult = await insightIntentDriver.execute(c17);
                this.result = 'code: ' + e17.code + '\n' + 'result: ' + JSON.stringify(e17?.result, null, 2);
                if (this.result != '') {
                    this.isShow = true;
                }
            }
            catch (d17) {
                this.result = 'code: ' + JSON.stringify(d17.code) + '\n' + JSON.stringify(c17, null, 2);
                if (this.result != '') {
                    this.isShow = true;
                }
            }
        }
    }
    submitForm() {
        const y16 = format(this.formData);
        let z16: boolean = true;
        this.required?.forEach((a17: string) => {
            if (y16[a17] === undefined || y16[a17] === null ||
                y16[a17] === '' || Number.isNaN(y16[a17] as number)) {
                if (z16) {
                    z16 = false;
                    hilog.error(DOMAIN, TAG, `submitForm validation failed: required field '${a17}' is missing`);
                }
            }
        });
        if (!z16) {
            this.getUIContext().showAlertDialog({
                message: { "id": 16777237, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" },
                autoCancel: true,
                alignment: DialogAlignment.Center,
                offset: { dx: 0, dy: -20 },
                gridCount: 3,
                confirm: {
                    value: { "id": 16777220, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" },
                    action: () => {
                    }
                }
            });
            return;
        }
        this.executeLinkIntent(y16 as Record<string, Object>);
    }
    resultModule(j16 = null) {
        this.observeComponentCreation((w16, x16) => {
            ViewStackProcessor.StartGetAccessRecordingFor(w16);
            Column.create();
            Column.width('100%');
            Column.height('100%');
            if (!x16) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((k16, l16) => {
            ViewStackProcessor.StartGetAccessRecordingFor(k16);
            If.create();
            if (this.isShowEntry2) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation((u16, v16) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(u16);
                        Column.create();
                        Column.height('100%');
                        Column.width('100%');
                        Column.padding({
                            top: 28,
                            left: 22,
                            right: 22,
                            bottom: 48
                        });
                        if (!v16) {
                            Column.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    this.observeComponentCreation((s16, t16) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(s16);
                        UIExtensionComponent.create(this.want);
                        __UIExtensionComponent__extStyle();
                        if (!t16) {
                            UIExtensionComponent.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    Column.pop();
                });
            }
            else if (this.isShowForm) {
                this.ifElseBranchUpdateFunction(1, () => {
                    this.observeComponentCreation((q16, r16) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(q16);
                        Column.create();
                        Column.padding({
                            top: 28,
                            left: 22,
                            right: 22,
                            bottom: 48
                        });
                        Column.height('100%');
                        Column.width('100%');
                        if (!r16) {
                            Column.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    this.observeComponentCreation((o16, p16) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(o16);
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
                        if (!p16) {
                            FormComponent.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    Column.pop();
                });
            }
            else {
                this.ifElseBranchUpdateFunction(2, () => {
                    this.observeComponentCreation((m16, n16) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(m16);
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
                        if (!n16) {
                            Text.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    Text.pop();
                });
            }
            if (!l16) {
                If.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        If.pop();
        Column.pop();
    }
    buildTextInput(s15: FormField, t15 = null) {
        this.observeComponentCreation((h16, i16) => {
            ViewStackProcessor.StartGetAccessRecordingFor(h16);
            Flex.create({ direction: FlexDirection.Row, alignItems: ItemAlign.Start });
            Flex.margin({ bottom: 12 });
            if (!i16) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((f16, g16) => {
            ViewStackProcessor.StartGetAccessRecordingFor(f16);
            Text.create(s15.name);
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
            if (!g16) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((d16, e16) => {
            ViewStackProcessor.StartGetAccessRecordingFor(d16);
            Column.create();
            Column.flexGrow(1);
            if (!e16) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((a16, b16) => {
            ViewStackProcessor.StartGetAccessRecordingFor(a16);
            TextInput.create({
                placeholder: `${s15.type} ${s15 && s15.description ? '，' + s15.description : ''}`,
                text: `${this.formData[s15.key] ?? ''}`
            });
            TextInput.maxLength(s15.maxLength);
            TextInput.width('100%');
            TextInput.height(40);
            TextInput.fontSize(16);
            TextInput.fontColor({ "id": 16777249, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            TextInput.padding({ right: 16, left: 16 });
            TextInput.backgroundColor({ "id": 16777239, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            TextInput.onChange((c16: string) => {
                this.formData[s15.key] = c16;
            });
            if (!b16) {
                TextInput.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((u15, v15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(u15);
            If.create();
            if (s15.required) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation((y15, z15) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(y15);
                        Row.create();
                        Row.width('100%');
                        Row.padding({ top: 5, right: 10 });
                        if (!z15) {
                            Row.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    this.observeComponentCreation((w15, x15) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(w15);
                        Text.create({ "id": 16777234, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                        Text.fontSize(12);
                        Text.lineHeight(16);
                        Text.fontColor({ "id": 16777246, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                        Text.padding({ left: 10 });
                        if (!x15) {
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
            if (!v15) {
                If.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        If.pop();
        Column.pop();
        Flex.pop();
    }
    buildNumberInput(b15: FormField, c15 = null) {
        this.observeComponentCreation((q15, r15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(q15);
            Flex.create({ direction: FlexDirection.Row, alignItems: ItemAlign.Start });
            Flex.height(50);
            Flex.margin({ bottom: 15 });
            if (!r15) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((o15, p15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(o15);
            Text.create(b15.name);
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
            if (!p15) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((m15, n15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(m15);
            Column.create();
            Column.flexGrow(1);
            if (!n15) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((j15, k15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(j15);
            TextInput.create({
                placeholder: `${b15.type} ${b15 && b15.description ? '，' + b15.description : ''}`,
                text: `${this.formData[b15.key] ?? ''}`
            });
            TextInput.maxLength(b15.maxLength);
            TextInput.type(InputType.Number);
            TextInput.width('100%');
            TextInput.height(40);
            TextInput.fontSize(16);
            TextInput.fontColor({ "id": 16777249, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            TextInput.padding({ right: 16, left: 16 });
            TextInput.backgroundColor({ "id": 16777239, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            TextInput.onChange((l15: string) => {
                this.formData[b15.key] = parseInt(l15);
            });
            if (!k15) {
                TextInput.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((d15, e15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(d15);
            If.create();
            if (b15.required) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation((h15, i15) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(h15);
                        Row.create();
                        Row.width('100%');
                        Row.padding({ top: 5, right: 10 });
                        if (!i15) {
                            Row.pop();
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                    this.observeComponentCreation((f15, g15) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(f15);
                        Text.create({ "id": 16777234, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                        Text.fontSize(12);
                        Text.lineHeight(16);
                        Text.fontColor({ "id": 16777246, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                        Text.padding({ left: 10 });
                        if (!g15) {
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
            if (!e15) {
                If.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        If.pop();
        Column.pop();
        Flex.pop();
    }
    buildSelectInput(e14: FormField, f14 = null) {
        this.observeComponentCreation((z14, a15) => {
            ViewStackProcessor.StartGetAccessRecordingFor(z14);
            Flex.create({ direction: FlexDirection.Row, alignItems: ItemAlign.Start });
            Flex.margin({ bottom: 20 });
            if (!a15) {
                Flex.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((x14, y14) => {
            ViewStackProcessor.StartGetAccessRecordingFor(x14);
            Text.create(e14.name);
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
            if (!y14) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        this.observeComponentCreation((v14, w14) => {
            ViewStackProcessor.StartGetAccessRecordingFor(v14);
            Column.create();
            Column.flexGrow(1);
            if (!w14) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((o14, p14) => {
            ViewStackProcessor.StartGetAccessRecordingFor(o14);
            Select.create(e14.options?.map(t14 => {
                const u14: SelectOption = { value: t14.toString() };
                return u14;
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
            Select.onSelect((q14: number, r14: string) => {
                const s14 = e14.type === 'string' ? r14 : parseInt(r14);
                this.formData[e14.key] = s14;
            });
            if (!p14) {
                Select.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Select.pop();
        this.observeComponentCreation((m14, n14) => {
            ViewStackProcessor.StartGetAccessRecordingFor(m14);
            Row.create();
            Row.width('100%');
            Row.padding({ top: 5, right: 10 });
            if (!n14) {
                Row.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((i14, j14) => {
            ViewStackProcessor.StartGetAccessRecordingFor(i14);
            If.create();
            if (e14.required) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation((k14, l14) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(k14);
                        Text.create({ "id": 16777234, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                        Text.fontSize(12);
                        Text.lineHeight(16);
                        Text.fontColor({ "id": 16777246, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                        Text.padding({ left: 10 });
                        if (!l14) {
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
            if (!j14) {
                If.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        If.pop();
        this.observeComponentCreation((g14, h14) => {
            ViewStackProcessor.StartGetAccessRecordingFor(g14);
            Text.create(e14.description);
            Text.fontSize(12);
            Text.lineHeight(16);
            Text.fontColor({ "id": 16777247, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
            Text.padding({ left: 10 });
            if (!h14) {
                Text.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        Text.pop();
        Row.pop();
        Column.pop();
        Flex.pop();
    }
    buildFormFields(n12: FormField[] | FormField[][], o12 = null) {
        this.observeComponentCreation((c14, d14) => {
            ViewStackProcessor.StartGetAccessRecordingFor(c14);
            Column.create();
            if (!d14) {
                Column.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        this.observeComponentCreation((p12, q12) => {
            ViewStackProcessor.StartGetAccessRecordingFor(p12);
            ForEach.create();
            const r12 = (s12, t12: number) => {
                const u12 = s12;
                this.observeComponentCreation((v12, w12) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(v12);
                    If.create();
                    if (u12.inputType === 'text') {
                        this.ifElseBranchUpdateFunction(0, () => {
                            this.buildTextInput.bind(this)(u12);
                        });
                    }
                    else if (u12.inputType === 'number') {
                        this.ifElseBranchUpdateFunction(1, () => {
                            this.buildNumberInput.bind(this)(u12);
                        });
                    }
                    else if (u12.inputType === 'select') {
                        this.ifElseBranchUpdateFunction(2, () => {
                            this.buildSelectInput.bind(this)(u12);
                        });
                    }
                    else if (u12.inputType === 'object') {
                        this.ifElseBranchUpdateFunction(3, () => {
                            this.observeComponentCreation((u13, v13) => {
                                ViewStackProcessor.StartGetAccessRecordingFor(u13);
                                If.create();
                                if (u12.inputType === 'object' && !u12.subProperties) {
                                    this.ifElseBranchUpdateFunction(0, () => {
                                        this.buildTextInput.bind(this)(u12);
                                    });
                                }
                                else {
                                    this.ifElseBranchUpdateFunction(1, () => {
                                        this.observeComponentCreation((a14, b14) => {
                                            ViewStackProcessor.StartGetAccessRecordingFor(a14);
                                            Column.create();
                                            if (!b14) {
                                                Column.pop();
                                            }
                                            ViewStackProcessor.StopGetAccessRecording();
                                        });
                                        this.observeComponentCreation((y13, z13) => {
                                            ViewStackProcessor.StartGetAccessRecordingFor(y13);
                                            Row.create();
                                            Row.margin({ bottom: 8, top: 16 });
                                            if (!z13) {
                                                Row.pop();
                                            }
                                            ViewStackProcessor.StopGetAccessRecording();
                                        });
                                        this.observeComponentCreation((w13, x13) => {
                                            ViewStackProcessor.StartGetAccessRecordingFor(w13);
                                            Text.create(u12.name);
                                            Text.fontSize(14);
                                            Text.lineHeight(20);
                                            Text.fontWeight(500);
                                            Text.fontColor({ "id": 16777248, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                            Text.width('100%');
                                            Text.padding({ left: 12 });
                                            if (!x13) {
                                                Text.pop();
                                            }
                                            ViewStackProcessor.StopGetAccessRecording();
                                        });
                                        Text.pop();
                                        Row.pop();
                                        this.buildFormFields.bind(this)(u12.subProperties as FormField[]);
                                        Column.pop();
                                    });
                                }
                                if (!v13) {
                                    If.pop();
                                }
                                ViewStackProcessor.StopGetAccessRecording();
                            });
                            If.pop();
                        });
                    }
                    else if (u12.inputType === 'array') {
                        this.ifElseBranchUpdateFunction(4, () => {
                            this.observeComponentCreation((x12, y12) => {
                                ViewStackProcessor.StartGetAccessRecordingFor(x12);
                                ForEach.create();
                                const z12 = (a13, b13: number) => {
                                    const c13 = a13;
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
                                        if (!r13) {
                                            Flex.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    this.observeComponentCreation((o13, p13) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(o13);
                                        Text.create(u12.name);
                                        Text.fontSize(14);
                                        Text.fontWeight(500);
                                        Text.fontColor({ "id": 16777248, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                        Text.width('100%');
                                        Text.lineHeight(20);
                                        if (!p13) {
                                            Text.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    Text.pop();
                                    this.observeComponentCreation((d13, e13) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(d13);
                                        If.create();
                                        if (b13 === 0) {
                                            this.ifElseBranchUpdateFunction(0, () => {
                                                this.observeComponentCreation((k13, l13) => {
                                                    ViewStackProcessor.StartGetAccessRecordingFor(k13);
                                                    Text.create({ "id": 16777222, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                                    Text.fontSize(14);
                                                    Text.fontColor('rgb(10,89,247)');
                                                    Text.fontWeight(500);
                                                    Text.lineHeight(19);
                                                    Text.onClick(() => {
                                                        if (u12.maxLength && u12.subProperties && (u12.subProperties?.length >= u12.maxLength)) {
                                                            this.getUIContext().showAlertDialog({
                                                                message: `最大节点数不超过${u12.maxLength}个`,
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
                                                            const m13: FormField[][] = addArrayParams(u12.subProperties, u12.name, this.countArr);
                                                            let n13 = jsClone(this.formFields) as FormField[];
                                                            n13[t12].subProperties = m13;
                                                            this.formFields = n13;
                                                        }
                                                    });
                                                    if (!l13) {
                                                        Text.pop();
                                                    }
                                                    ViewStackProcessor.StopGetAccessRecording();
                                                });
                                                Text.pop();
                                            });
                                        }
                                        else {
                                            this.ifElseBranchUpdateFunction(1, () => {
                                                this.observeComponentCreation((f13, g13) => {
                                                    ViewStackProcessor.StartGetAccessRecordingFor(f13);
                                                    Text.create({ "id": 16777221, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                                    Text.fontSize(14);
                                                    Text.fontColor('rgb(10,89,247)');
                                                    Text.fontWeight(500);
                                                    Text.lineHeight(19);
                                                    Text.onClick(() => {
                                                        const h13: FormDataType = removeFormData(this.formData, u12.name, c13[0].index);
                                                        this.formData = h13;
                                                        const i13: FormField[][] = removeArrayParams(u12.subProperties, u12.name, b13);
                                                        let j13 = jsClone(this.formFields) as FormField[];
                                                        j13[t12].subProperties = i13;
                                                        this.formFields = j13;
                                                    });
                                                    if (!g13) {
                                                        Text.pop();
                                                    }
                                                    ViewStackProcessor.StopGetAccessRecording();
                                                });
                                                Text.pop();
                                            });
                                        }
                                        if (!e13) {
                                            If.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    If.pop();
                                    Flex.pop();
                                    this.buildFormFields.bind(this)(c13 as FormField[]);
                                    Column.pop();
                                };
                                this.forEachUpdateFunction(x12, u12.subProperties, z12, undefined, true, false);
                                if (!y12) {
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
                    if (!w12) {
                        If.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                If.pop();
            };
            this.forEachUpdateFunction(p12, n12, r12, undefined, true, false);
            if (!q12) {
                ForEach.pop();
            }
            ViewStackProcessor.StopGetAccessRecording();
        });
        ForEach.pop();
        Column.pop();
    }
    initialRender() {
        this.observeComponentCreation((x9, y9) => {
            ViewStackProcessor.StartGetAccessRecordingFor(x9);
            NavDestination.create(() => {
                this.observeComponentCreation((l12, m12) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(l12);
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
                    if (!m12) {
                        Row.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                this.observeComponentCreation((j12, k12) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(j12);
                    Column.create();
                    Column.width(40);
                    Column.height(40);
                    Column.borderRadius(20);
                    Column.backgroundColor({ "id": 16777245, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                    Column.justifyContent(FlexAlign.Center);
                    if (!k12) {
                        Column.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                this.observeComponentCreation((h12, i12) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(h12);
                    Image.create({ "id": 16777255, "type": 20000, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                    Image.width(8.68);
                    Image.height(16.48);
                    Image.opacity(0.6);
                    if (!i12) {
                        Image.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                Column.pop();
                this.observeComponentCreation((f12, g12) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(f12);
                    Text.create({ "id": 16777228, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                    Text.margin({ left: 8 });
                    Text.fontSize(20);
                    Text.fontWeight(700);
                    Text.fontColor({ "id": 16777249, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                    if (!g12) {
                        Text.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                Text.pop();
                Row.pop();
                this.observeComponentCreation((d12, e12) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(d12);
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
                    if (!e12) {
                        Scroll.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                this.observeComponentCreation((b12, c12) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(b12);
                    Column.create();
                    Column.width('100%');
                    Column.alignItems(HorizontalAlign.Start);
                    if (!c12) {
                        Column.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                {
                    this.observeComponentCreation((x11, y11) => {
                        ViewStackProcessor.StartGetAccessRecordingFor(x11);
                        if (y11) {
                            let z11 = new DetailItem(this, { intentDetail: this.intentDetail }, undefined, x11, () => { }, { page: "feature/src/main/ets/view/IntentDetailPage.ets", line: 747, col: 11 });
                            ViewPU.create(z11);
                            let a12 = () => {
                                return {
                                    intentDetail: this.intentDetail
                                };
                            };
                            z11.paramsGenerator_ = a12;
                        }
                        else {
                            this.updateStateVarsOfChildByElmtId(x11, {
                                intentDetail: this.intentDetail
                            });
                        }
                        ViewStackProcessor.StopGetAccessRecording();
                    });
                }
                this.observeComponentCreation((v11, w11) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(v11);
                    Column.create();
                    Column.padding({ bottom: 86 });
                    Column.margin({ left: 16, right: 16 });
                    if (!w11) {
                        Column.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                this.observeComponentCreation((t11, u11) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(t11);
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
                    if (!u11) {
                        Text.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                Text.pop();
                this.observeComponentCreation((e10, f10) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(e10);
                    If.create();
                    if (this.intentParameters !== null) {
                        this.ifElseBranchUpdateFunction(0, () => {
                            this.observeComponentCreation((g10, h10) => {
                                ViewStackProcessor.StartGetAccessRecordingFor(g10);
                                ForEach.create();
                                const i10 = (j10, k10: number) => {
                                    const l10 = j10;
                                    this.observeComponentCreation((m10, n10) => {
                                        ViewStackProcessor.StartGetAccessRecordingFor(m10);
                                        If.create();
                                        if (l10.inputType === 'text') {
                                            this.ifElseBranchUpdateFunction(0, () => {
                                                this.buildTextInput.bind(this)(l10);
                                            });
                                        }
                                        else if (l10.inputType === 'number') {
                                            this.ifElseBranchUpdateFunction(1, () => {
                                                this.buildNumberInput.bind(this)(l10);
                                            });
                                        }
                                        else if (l10.inputType === 'select') {
                                            this.ifElseBranchUpdateFunction(2, () => {
                                                this.buildSelectInput.bind(this)(l10);
                                            });
                                        }
                                        else if (l10.inputType === 'object') {
                                            this.ifElseBranchUpdateFunction(3, () => {
                                                this.observeComponentCreation((l11, m11) => {
                                                    ViewStackProcessor.StartGetAccessRecordingFor(l11);
                                                    If.create();
                                                    if (l10.inputType === 'object' && !l10.subProperties) {
                                                        this.ifElseBranchUpdateFunction(0, () => {
                                                            this.buildTextInput.bind(this)(l10);
                                                        });
                                                    }
                                                    else {
                                                        this.ifElseBranchUpdateFunction(1, () => {
                                                            this.observeComponentCreation((r11, s11) => {
                                                                ViewStackProcessor.StartGetAccessRecordingFor(r11);
                                                                Column.create();
                                                                if (!s11) {
                                                                    Column.pop();
                                                                }
                                                                ViewStackProcessor.StopGetAccessRecording();
                                                            });
                                                            this.observeComponentCreation((p11, q11) => {
                                                                ViewStackProcessor.StartGetAccessRecordingFor(p11);
                                                                Row.create();
                                                                Row.margin({ bottom: 8, top: 16 });
                                                                if (!q11) {
                                                                    Row.pop();
                                                                }
                                                                ViewStackProcessor.StopGetAccessRecording();
                                                            });
                                                            this.observeComponentCreation((n11, o11) => {
                                                                ViewStackProcessor.StartGetAccessRecordingFor(n11);
                                                                Text.create(l10.name);
                                                                Text.fontSize(14);
                                                                Text.lineHeight(20);
                                                                Text.fontWeight(500);
                                                                Text.fontColor({ "id": 16777248, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                                                Text.width('100%');
                                                                Text.padding({ left: 12 });
                                                                if (!o11) {
                                                                    Text.pop();
                                                                }
                                                                ViewStackProcessor.StopGetAccessRecording();
                                                            });
                                                            Text.pop();
                                                            Row.pop();
                                                            this.buildFormFields.bind(this)(l10.subProperties as FormField[]);
                                                            Column.pop();
                                                        });
                                                    }
                                                    if (!m11) {
                                                        If.pop();
                                                    }
                                                    ViewStackProcessor.StopGetAccessRecording();
                                                });
                                                If.pop();
                                            });
                                        }
                                        else if (l10.inputType === 'array') {
                                            this.ifElseBranchUpdateFunction(4, () => {
                                                this.observeComponentCreation((o10, p10) => {
                                                    ViewStackProcessor.StartGetAccessRecordingFor(o10);
                                                    ForEach.create();
                                                    const q10 = (r10, s10: number) => {
                                                        const t10 = r10;
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
                                                            if (!i11) {
                                                                Flex.pop();
                                                            }
                                                            ViewStackProcessor.StopGetAccessRecording();
                                                        });
                                                        this.observeComponentCreation((f11, g11) => {
                                                            ViewStackProcessor.StartGetAccessRecordingFor(f11);
                                                            Text.create(l10.name);
                                                            Text.fontSize(14);
                                                            Text.fontColor({ "id": 16777248, "type": 10001, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                                            Text.width('50%');
                                                            Text.lineHeight(20);
                                                            Text.fontWeight(500);
                                                            if (!g11) {
                                                                Text.pop();
                                                            }
                                                            ViewStackProcessor.StopGetAccessRecording();
                                                        });
                                                        Text.pop();
                                                        this.observeComponentCreation((u10, v10) => {
                                                            ViewStackProcessor.StartGetAccessRecordingFor(u10);
                                                            If.create();
                                                            if (s10 === 0) {
                                                                this.ifElseBranchUpdateFunction(0, () => {
                                                                    this.observeComponentCreation((b11, c11) => {
                                                                        ViewStackProcessor.StartGetAccessRecordingFor(b11);
                                                                        Text.create({ "id": 16777222, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                                                        Text.fontSize(14);
                                                                        Text.fontColor('rgb(10,89,247)');
                                                                        Text.fontWeight(500);
                                                                        Text.lineHeight(19);
                                                                        Text.onClick(() => {
                                                                            if (l10.maxLength && l10.subProperties &&
                                                                                (l10.subProperties?.length >= l10.maxLength)) {
                                                                                this.getUIContext().showAlertDialog({
                                                                                    message: `最大节点不超过${l10.maxLength}个`,
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
                                                                                const d11: FormField[][] = addArrayParams(l10.subProperties, l10.name, this.countArr);
                                                                                let e11 = jsClone(this.formFields) as FormField[];
                                                                                e11[k10].subProperties = d11;
                                                                                this.formFields = e11;
                                                                            }
                                                                        });
                                                                        if (!c11) {
                                                                            Text.pop();
                                                                        }
                                                                        ViewStackProcessor.StopGetAccessRecording();
                                                                    });
                                                                    Text.pop();
                                                                });
                                                            }
                                                            else {
                                                                this.ifElseBranchUpdateFunction(1, () => {
                                                                    this.observeComponentCreation((w10, x10) => {
                                                                        ViewStackProcessor.StartGetAccessRecordingFor(w10);
                                                                        Text.create({ "id": 16777221, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" });
                                                                        Text.fontSize(14);
                                                                        Text.fontColor('rgb(10,89,247)');
                                                                        Text.fontWeight(500);
                                                                        Text.lineHeight(19);
                                                                        Text.onClick(() => {
                                                                            const y10: FormDataType = removeFormData(this.formData, l10.name, t10[0].index);
                                                                            this.formData = y10;
                                                                            const z10: FormField[][] = removeArrayParams(l10.subProperties, l10.name, s10);
                                                                            let a11 = jsClone(this.formFields) as FormField[];
                                                                            a11[k10].subProperties = z10;
                                                                            this.formFields = a11;
                                                                        });
                                                                        if (!x10) {
                                                                            Text.pop();
                                                                        }
                                                                        ViewStackProcessor.StopGetAccessRecording();
                                                                    });
                                                                    Text.pop();
                                                                });
                                                            }
                                                            if (!v10) {
                                                                If.pop();
                                                            }
                                                            ViewStackProcessor.StopGetAccessRecording();
                                                        });
                                                        If.pop();
                                                        Flex.pop();
                                                        this.buildFormFields.bind(this)(t10 as FormField[]);
                                                        Column.pop();
                                                    };
                                                    this.forEachUpdateFunction(o10, l10.subProperties, q10, undefined, true, false);
                                                    if (!p10) {
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
                                        if (!n10) {
                                            If.pop();
                                        }
                                        ViewStackProcessor.StopGetAccessRecording();
                                    });
                                    If.pop();
                                };
                                this.forEachUpdateFunction(g10, this.formFields, i10, undefined, true, false);
                                if (!h10) {
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
                    if (!f10) {
                        If.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                If.pop();
                Column.pop();
                Column.pop();
                Scroll.pop();
                this.observeComponentCreation((c10, d10) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(c10);
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
                    if (!d10) {
                        Row.pop();
                    }
                    ViewStackProcessor.StopGetAccessRecording();
                });
                this.observeComponentCreation((z9, a10) => {
                    ViewStackProcessor.StartGetAccessRecordingFor(z9);
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
                    Button.bindSheet({ value: this.isShow, changeEvent: b10 => { this.isShow = b10; } }, { builder: () => {
                            this.resultModule.call(this);
                        } }, {
                        height: SheetSize.MEDIUM,
                        blurStyle: BlurStyle.Thick,
                        title: { title: { "id": 16777225, "type": 10003, params: [], "bundleName": "com.ohos.amsdialog", "moduleName": "default" } },
                        preferType: SheetType.BOTTOM
                    });
                    if (!a10) {
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
            if (!y9) {
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
