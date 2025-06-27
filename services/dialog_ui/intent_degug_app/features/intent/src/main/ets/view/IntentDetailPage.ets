import { IntentInfo } from './IntentContent'
import { DetailItem } from './DetailItem';
import { insightIntentDriver, insightIntent } from '@kit.AbilityKit';
import { formate, jsClone, addArrayParams, removeArrayParams } from './formate'


type PropType = Record<string, Object>;
type parameType = Record<string, string>;
interface parameGroup{
  parameters: Record<string, Object>;
}
const IntentDetailLabel: string[] = ['bundleName','moduleName','intentName','domain','intentVersion','displayName','displayDescription','schema','icon','llmDescription','intentType']


// 定义动态表单字段类型
interface propertiesInfo{
  name: string
  type: string
  description?: string
  enum?: Array<string | number>
  subItem?: propertiesInfo
  maxLength?: number
  default?: number | Resource
}
interface FormField {
  key: string // 字段唯一标识
  fatherKey?: string
  name: string // 字段标签
  type: string //'text' | 'number' | 'select'| 'object'  // 字段类型
  fatherType?: string
  inputType: string //'text' | 'number' | 'select'| 'object'  // 字段类型
  description?: string
  options?: Array<string | number>//{ label: string,value: string | number}[]
  subProperties?: FormField[] | FormField[][]
  maxLength?: number
  default?: number | Resource
  value?: string | number
  index?: number
}

type ObjVal = string | number | null;
type ObjType = Record<string,object>
type ArrType = ObjType[]

type FormDataType = Record<string, ObjVal | ObjType | ArrType>
type ObjType2 = Record<string,Record<string, ObjVal>>
type ArrType2 = Record<string,Record<string, ObjVal>>[]

@Extend(UIExtensionComponent)
function extStyle() {
  .width('100%')
  .height('100%')
  .onReceive((data) => {
    console.info(`onReceive, data:${JSON.stringify(data)}`)
  })
  .onTerminated((info) => {
    console.info(`onTerminated, info:${JSON.stringify(info)}`);
  })
  .onRemoteReady((proxy) => {
    console.info(`onRemoteReady, info:${JSON.stringify(proxy)}`)
  })
  .onError((data) => {
    console.info(`onError, data:${JSON.stringify(data)}`)
  })
}

@Component
export struct IntentDetailPage{
  @Consume('articlePathStack') articlePathStack: NavPathStack;
  title:string ='执行意图';
  title2:string ='意图参数';

  @State intentDetail: IntentInfo | null = null;
  // @State intentDetailBlock: object | null = null;
  @State intentDetailMap: Map<string, string> | null = null;
  @State intentDetailEntries: Array<[string, string|string[]|object]> | [] = []
  @State intentParameters: object | null = null;
  scroller: Scroller = new Scroller();


  // @State formData: Record<string, string | number | object | object[]> = {}
  // @State formData: Record<string, string | number | object | object[]> = {}
  @State formData: FormDataType = {}
  @State properties: Record< string, propertiesInfo > = {};
  @State formFields: FormField[] = [] // | FormField[][] = []
  @State abilityName: string = "";
  @State uri: string = "";
  // @State entryExecuteMode: number[] = [0];
  @State result: string = "";
  @State isShow: boolean = false;
  @State isShowEntry2: boolean = false;
  @State want: Want = {
    bundleName: 'com.ohos.tools.myapplication',  // 意图里的bundleName
    abilityName: 'EntryUIExtAbility',          // 需要是意图里的abilityName: subIntentInfo
    // abilityName: 'EntryShareExtAbility1', // 并未绑定对应的意图，预期不会被拉起
    moduleName: 'entry',   // moduleName
    parameters: {
      'ability.want.params.uiExtensionType': 'share',
      "ohos.insightIntent.executeParam.name": "TestIntentNameEntry",   // intentName
      "ohos.insightIntent.executeParam.param": {
        // 意图参数
      },
    },
  };

  inputType: Record<string, string> = {
    'string': 'text',
    'number': 'number',
    'enum': 'select',
    'object': 'object',
    'array': 'array',
  }



  aboutToAppear(): void {
    this.intentDetail = this.articlePathStack.getParamByName('intentDetail')[0] as IntentInfo;
    console.log(`this.intentDetail = ${JSON.stringify(this.intentDetail)}`);
    this.intentParameters = this.intentDetail.parameters ?? null;
    this.intentDetailMap = new Map(Object.entries(this.intentDetail));
    this.intentDetailEntries = Array.from(this.intentDetailMap.entries());

    //FormPage({ parameters: this.intentParameters, intentDetail: this.intentDetail})
    // this.abilityName = this.getAbilityName(this.intentDetail?.subIntentInfo);
    this.abilityName = this.getAttribute(this.intentDetail?.subIntentInfo, 'abilityName');
    this.uri =  this.getAttribute(this.intentDetail?.subIntentInfo, 'uri');
    // this.entryExecuteMode =  this.getAttribute(this.intentDetail?.subIntentInfo, 'executeMode');
    if(this.intentParameters){
      // this.properties = this.findProperties(this.parameters);
      this.properties = this.intentParameters["properties"]; //this.parameters["properties"];
      this.formFields = this.changeProperties(this.properties); //格式化表单数据结构
    }
  }

  @Provide InputTypeData: Record< string, InputType> = {
    'string': InputType.Normal,
    'number': InputType.Number
  }


  getAttribute(subInfo:object | undefined, attributeName: string): string{
    if(subInfo && Object.keys(subInfo).includes(attributeName)){
      return subInfo[attributeName]
    } else {
      return ""
    }
    /*
    if(subInfo){
      if(Object.keys(subInfo).includes("abilityName")){
        return subInfo["abilityName"]
      }
      else if(Object.keys(subInfo).includes("uiAbility")){
        return subInfo["uiAbility"]
      } else{
        return ""
      }
    }else{
      return ""
    }
    * */
  }

  changeProperties(properties:Record< string, propertiesInfo >,fatherKey?:string,fatherType?:string){
    const formArr: FormField[] = []
    Object.keys(properties).forEach((propKey:string, index:number)=>{
      const obj = properties[propKey];
      let  key = propKey;
      if(fatherKey){
        if(fatherType && fatherType == "array"){
          key = `${fatherKey}.array0.${propKey}`;
        }else{
          key = `${fatherKey}.${propKey}`
        }
      }
      const name = propKey;
      const type = obj.type;
      const inputType = obj.enum ? 'select' : this.inputType[type];
      let  subItem: FormField[] | FormField[][] = [];
      if(type === "object" && Object.keys(obj).includes('properties')){
        const subPropties =  this.findProperties(obj);
        subItem= this.changeProperties(subPropties, propKey) as FormField[];
      }else if(type === "array" && Object.keys(obj).includes("items")){
        const subPropties = this.findProperties(obj);
        subItem= [this.changeProperties(subPropties, propKey, "array")] as FormField[][];
        // this.formData[propKey] = new Array();
      }
      const description = obj.description;
      const formObj: FormField = {
        key,
        name,
        type,
        inputType,
        description,
      }

      if(obj.enum) formObj.options = obj.enum;
      if(obj.maxLength) formObj.maxLength = obj.maxLength;
      if(obj.default) formObj.default = obj.default;
      if(fatherKey) formObj.fatherKey = fatherKey;
      if(fatherType){
        formObj.fatherType = fatherType;
        formObj.index = 0;
      }
      //if(arrIndex) formObj.index = 0;
      if(subItem.length>0){
        if(fatherType && fatherType == "array"){
          formObj.subProperties = subItem;// as FormField[][]
        }else{
          formObj.subProperties = subItem;// as FormField[]
        }
      }
      formArr.push(formObj)
    })
    return formArr;
  }

  findProperties(obj:object): Record< string, propertiesInfo >{
    let prop: Record< string, propertiesInfo >={}
    Object.keys(obj).forEach((key:string) => {
      if( key == 'properties'){
        prop = obj['properties'];
      }else if(typeof obj[key] == 'object' && key !== "propertyNames"){
        prop = this.findProperties(obj[key])
      }
    })
    return prop
  }

  async executeLinkIntent(insightIntentParam: Record<string, Object>) {

    let param: insightIntentDriver.ExecuteParam = {
      bundleName: this.intentDetail?.bundleName ?? '', //'com.acts.intentxtslinkhelper',
      moduleName: this.intentDetail?.moduleName ?? '', //'entry',
      insightIntentName: this.intentDetail?.intentName ?? '',
      // uris: ["link://www.linkAbility.com"], //只有link现需要传这个参数 uris：undefined
      insightIntentParam: insightIntentParam, //{"cabin": "经济舱", "entityId": 176839410},
      executeMode: insightIntent.ExecuteMode.UI_ABILITY_FOREGROUND,
      //executeMode——entry:0 | link/page: insightIntent.ExecuteMode.UI_ABILITY_FOREGROUND | form:不用执行 | function ： 后台：insightIntent.ExecuteMode.UI_ABILITY_BACKGROUND |
      abilityName: this.abilityName //"EntryAbility1"
    };

    if(this.intentDetail?.intentType == "@InsightIntentLink"){
      param.uris = [this.uri];
    }
    if(this.intentDetail?.intentType == "@InsightIntentLink" || this.intentDetail?.intentType == "@InsightIntentPage"){
      param.executeMode = insightIntent.ExecuteMode.UI_ABILITY_FOREGROUND;
    }
    if(this.intentDetail?.intentType == "@InsightIntentFunctionMethod"){
      param.executeMode = insightIntent.ExecuteMode.UI_ABILITY_BACKGROUND
    }
    console.log("insightIntentLink ExecuteParame: " + JSON.stringify(param))

    if(this.intentDetail?.intentType == "@InsightIntentEntry" && this.intentDetail.subIntentInfo['executeMode'][0]){
      param.executeMode = this.intentDetail.subIntentInfo['executeMode'][0] ?? 0;
      if(param.executeMode == 2)
        this.want = {
          bundleName: param.bundleName,  // 意图里的bundleName
          abilityName: param.abilityName,          // 需要是意图里的abilityName: subIntentInfo
          // abilityName: 'EntryShareExtAbility1', // 并未绑定对应的意图，预期不会被拉起
          moduleName: param.moduleName,   // moduleName
          parameters: {
            'ability.want.params.uiExtensionType': 'share',
            "ohos.insightIntent.executeParam.name": this.intentDetail.intentName,//"TestIntentNameEntry",   // intentName
            "ohos.insightIntent.executeParam.param": param.insightIntentParam,
          },
        };
        this.isShow = true;
        this.isShowEntry2 = true;
    }else {
      try {
        //通过接口校验写入的意图
        let resultData: insightIntent.ExecuteResult = await insightIntentDriver.execute(param);
        this.result = JSON.stringify(resultData);
        if(this.result!=""){
          this.isShow = true;
        }
        // console.log("insightIntentLink execute SUCCESS: " + JSON.stringify(resultData))
      } catch (error) {
        console.log("insightIntentLink execute ERROR: " + JSON.stringify(error));
        this.result = JSON.stringify(error);
        if(this.result!=""){
          this.isShow = true;
        }
      }
    }
  }

  submitForm(){
    const excuteParam = formate(this.formData);
    this.executeLinkIntent(excuteParam as Record<string, Object>);
    // AlertDialog.show({
    //   title: '提交成功',
    //   message: `用户名: ${this.formData.username}, 年龄: ${this.formData.age}, 性别: ${this.formData.gender}`,
    //   confirm: {
    //     value: '确定',
    //     action: () => {
    //       console.info('用户确认提交')
    //     }
    //   }
    // })
  }

  @Builder
  resultModule(){
    Column(){
      if(this.isShowEntry2){
        Column(){
          // Text(JSON.stringify(this.want))
          UIExtensionComponent(this.want)
            .extStyle()
        }
        .height('100%')
        .width('100%')
      }else {
        Text(this.result)
          .width("100%")
          .textAlign(TextAlign.Start)
      }
    }
    .width('100%')
    .padding({ top:30,left:25,right:25,bottom:20})
  }

  @Builder
  buildTextInput(field: FormField) {
    Flex({direction:FlexDirection.Row,alignItems:ItemAlign.Start}) {
      Text(field.name)
        .fontSize(16)
        .fontColor('rgba(0,0,0,0.4)')
        .width(165)
        .height(40)
        .borderRadius(20)
        .padding({right: 16, left: 16 })
        .margin({right:8})
        .backgroundColor(Color.White)
        .textOverflow({overflow:TextOverflow.Ellipsis})
        .maxLines(1)
      TextInput({placeholder: `${field.type} ${field.description ? ',' + field.description : ''}` })
        .maxLength(field.maxLength)
        .width('100%')
        .height(40)
        .fontSize(16)
        .fontColor('rgba(0,0,0,0.9)')
        .padding({right: 16, left: 16 })
        .flexGrow(1)
        .backgroundColor(Color.White)
        .onChange((value: string) => {
          this.formData[field.key] = value
          /*
          if(field.fatherKey){
            this.setNestedProperty(this.formData, field.key.split("."), value);
            this.formData[field.fatherKey]= this.setNestedProperty(this.formData, field.key.split("."), value) as ObjType;
          }else {
            this.formData[field.key] = value
          }
          * */
        })
    }
    // .width('100%')
    // .height(50)
    .margin({bottom:12})
    // .padding({left:10,right:10})
  }

  @Builder
  buildNumberInput(field: FormField) {
    Flex({direction:FlexDirection.Row,alignItems:ItemAlign.Start}) {
      Text(field.name)
        .fontSize(16)
        .fontColor('rgba(0,0,0,0.4)')
        .width(165)
        .height(40)
        .borderRadius(20)
        .padding({right: 16, left: 16 })
        .margin({right:8})
        .backgroundColor(Color.White)
        .textOverflow({overflow:TextOverflow.Ellipsis})
        .maxLines(1)
      TextInput({placeholder: `${field.type} ${field.description ? ',' + field.description : ''}` })
        .maxLength(field.maxLength)
        .type(InputType.Number)
        .width('100%')
        .height(40)
        .fontSize(16)
        .fontColor('rgba(0,0,0,0.9)')
        .padding({right: 16, left: 16 })
        .flexGrow(1)
        .backgroundColor(Color.White)
        .onChange((value: string) => {
          this.formData[field.key] = parseInt(value);
          /*
        if(field.fatherKey){
          this.setNestedProperty(this.formData, field.key.split("."), value);
          this.formData[field.fatherKey]=this.setNestedProperty(this.formData, field.key.split("."), value) as ObjType;
        }else {
          this.formData[field.key] = parseInt(value)
        }
          * */
        })
    }
    .height(50)
    .margin({bottom:15})
    // .padding({left:10,right:10})
    // .alignItems(VerticalAlign.Center)
    // .justifyContent(FlexAlign.Start)
  }

  @Builder
  buildSelectInput(field: FormField) {
    Flex({direction:FlexDirection.Row,alignItems:ItemAlign.Start}) {
      Text(field.name)
        .fontSize(16)
        .fontColor('rgba(0,0,0,0.4)')
        .width(165)
        .height(40)
        .borderRadius(20)
        .padding({right: 16, left: 16 })
        .margin({right:8})
        .backgroundColor(Color.White)
        .textOverflow({overflow:TextOverflow.Ellipsis})
        .maxLines(1)
      Column(){
        Select(field.options?.map(option => {
          const opt:SelectOption = {value: option.toString()}
          return opt
        }))
          .selected(field.default)
          .width('100%')
          .height(40)
          .fontColor('rgba(0,0,0,0.9)')
          // .fontWeight(400)
          .padding({right: 16, left: 16 })
          .backgroundColor(Color.White)
          .onSelect((index: number, value: string) => {
            const selVal = field.type==='string'? value : parseInt(value)
            this.formData[field.key] = selVal
          })
        Text(field.description)
          .fontSize(12)
          .fontColor('rgba(0,0,0,0.4)')
          .width('100%')
          .padding({ top: 5, left: 10, right: 10 })
      }
      .flexGrow(1)
    }
    .margin({bottom:15})
  }

  @Builder
  buildFormFields(formFields:FormField[]|FormField[][]){
    Column() {
      ForEach(formFields,(field: FormField, index: number)=>{
        if(field.inputType == "text"){
          this.buildTextInput(field)
        }else if(field.inputType == "number"){
          this.buildNumberInput(field)
        }else if(field.inputType == "select"){
          this.buildSelectInput(field)
        }else if(field.inputType == "object"){
          if(field.inputType == "object" && !field.subProperties){
            this.buildTextInput(field)
          }else {
            Column(){
              Row(){
                Text(field.name)
                  .fontColor('rgba(0,0,0,0.6)')
                  .fontWeight(500)
                  .width('100%')
                  .padding({top:10, right:10, bottom:11, left: 20})
                  .borderRadius({topLeft:20,topRight: 20})
                  .backgroundColor('#e0e0e0')
              }
              .margin({bottom: 15,right:-10,left:-10 })
              // const subFormFields: FormField[] = this.changeProperties(formFields.properties)
              this.buildFormFields(field.subProperties as FormField[])
            }
            .margin(15)
            .borderRadius(20)
            .margin({bottom: 20})
            .padding({left:10,right:10})
            .backgroundColor('#f0f0f1')
          }
        }else if(field.inputType == "array"){
          ForEach(field.subProperties,(fieldItem: FormField[], arrIndex: number)=>{
            Column() {
              Flex({direction:FlexDirection.Row,justifyContent:FlexAlign.SpaceBetween,alignItems:ItemAlign.Center}) {
                Text(field.name+(1+arrIndex))
                  .fontColor('rgba(0,0,0,0.6)')
                  .width('100%')
                  .padding({
                    top: 10,
                    right: 10,
                    bottom: 10,
                    left: 20
                  })
                  .fontWeight(500)
                if(arrIndex == 0){
                  Text("+")
                    .fontSize(24)
                    .fontColor('rgba(0,0,0,0.6)')
                    .fontWeight(500)
                    .margin({ right: 20})
                    .padding(10)
                    .onClick(()=>{
                      //方法二：返回 FormField[]
                      // const arrItem: FormField[] = addArrayParams(field.subProperties, field.name);
                      // let fieldArr = jsClone(this.formFields) as FormField[];
                      // (fieldArr[index].subProperties as FormField[][]).push(arrItem);
                      // if(Array.isArray(fieldArr[index].subProperties)){
                      //   (fieldArr[index].subProperties as FormField[][]).push(arrItem);
                      // }
                      //方法一：返回 FormField[][]
                      const arrItem: FormField[][] = addArrayParams(field.subProperties, field.name);
                      let fieldArr = jsClone(this.formFields) as FormField[];
                      fieldArr[index].subProperties = arrItem;
                      this.formFields = fieldArr;
                      console.log("addArrayParams2 = "+JSON.stringify(this.formFields[index].subProperties))
                    })
                }else {
                  Text("-")
                    .fontSize(24)
                    .fontColor('rgba(0,0,0,0.6)')
                    .fontWeight(500)
                    .margin({ right: 20})
                    .padding(10)
                    .onClick(()=>{
                      const arrItem: FormField[][] = removeArrayParams(field.subProperties, field.name, arrIndex);
                      let fieldArr = jsClone(this.formFields) as FormField[];
                      fieldArr[index].subProperties = arrItem;
                      this.formFields = fieldArr;
                      console.log("addArrayParams2 = "+JSON.stringify(this.formFields[index].subProperties))
                    })
                }
              }
              .borderRadius({ topLeft: 20, topRight: 20 })
              .backgroundColor('#e0e0e0')
              .margin({bottom: 15,right:-10,left:-10 })

              this.buildFormFields(fieldItem as FormField[])
            }
            .margin(15)
            .borderRadius(20)
            .margin({bottom: 20})
            .padding({left:10,right:10})
            .backgroundColor('#f0f0f1')
          })
        }
      })
    }
  }

  build() {
    NavDestination() {
      Scroll(this.scroller) {
        Column() {
          Row(){
            Column(){
              Image($r('app.media.ic_arrow_left'))
                .width(8.68)
                .height(16.48)
                .opacity(0.6)
                .id('btn-back')
            }
            .width(40)
            .height(40)
            .borderRadius(20)
            .backgroundColor('rgba(0,0,0,0.05)')
            .justifyContent(FlexAlign.Center)

            Text(this.title)
              .margin({ left: 8 })
              .fontSize(20)
              .fontWeight(700)
              .fontColor('rgba(0,0,0,0.9)')
          }
          .width('100%')
          .height(56)
          .padding({ top:14.5,right:16,left:16,bottom:14.5 })
          .onClick(() => {
              this.articlePathStack.pop()
            })

          DetailItem({intentDetail: this.intentDetail, intentDetailAll: this.intentDetailEntries})
          //FormPage({ parameters: this.intentParameters, intentDetail: this.intentDetail})

          Column(){
            Text(this.title2)
              .fontFamily('HarmonyHeiTi-Medium')
              .fontSize(14)
              .fontWeight(500)
              .fontColor('rgba(0,0,0,0.6)')
              .padding({top:10,right:16,bottom:10,left:16})
              .textAlign(TextAlign.Start)
              .width('100%')

            if(this.intentParameters !== null){
              // this.buildFormFields(this.formFields)
              ForEach(this.formFields,(field: FormField, index: number)=>{
                if(field.inputType == "text"){
                  this.buildTextInput(field)
                }else if(field.inputType == "number"){
                  this.buildNumberInput(field)
                }else if(field.inputType == "select"){
                  this.buildSelectInput(field)
                }else if(field.inputType == "object"){
                  if(field.inputType == "object" && !field.subProperties){
                    this.buildTextInput(field)
                  }else {
                    Column(){
                      Row(){
                        Text(field.name)
                          .fontColor('rgba(0,0,0,.6)')
                          .fontWeight(500)
                          .width('100%')
                          .padding({top:10, right:10, bottom:11, left: 20})
                          .borderRadius({topLeft:20,topRight: 20})
                          .backgroundColor('#e0e0e0')
                      }
                      .margin({bottom: 15,right:-10,left:-10 })
                      // const subFormFields: FormField[] = this.changeProperties(formFields.properties)
                      this.buildFormFields(field.subProperties as FormField[])
                    }
                    // .margin(15)
                    .borderRadius(20)
                    .margin({bottom: 20})
                    .padding({left:10,right:10})
                    .backgroundColor('#f0f0f1')
                  }
                }else if(field.inputType == "array"){
                  ForEach(field.subProperties,(fieldItem: FormField[], arrIndex: number)=>{
                    Column() {
                      Flex({direction:FlexDirection.Row,justifyContent:FlexAlign.SpaceBetween,alignItems:ItemAlign.Center}) {
                        Text(field.name+(1+arrIndex))
                          .fontColor('rgba(0,0,0,0.6)')
                          .width('100%')
                          .padding({
                            top: 10,
                            right: 10,
                            bottom: 10,
                            left: 20
                          })
                          .fontWeight(500)
                        if(arrIndex == 0){
                          Text("+")
                            .fontSize(24)
                            .fontColor('rgba(0,0,0,0.6)')
                            .fontWeight(500)
                            .margin({ right: 20})
                            .padding(10)
                            .onClick(()=>{
                              //方法二：返回 FormField[]
                              // const arrItem: FormField[] = addArrayParams(field.subProperties, field.name);
                              // let fieldArr = jsClone(this.formFields) as FormField[];
                              // (fieldArr[index].subProperties as FormField[][]).push(arrItem);
                              // if(Array.isArray(fieldArr[index].subProperties)){
                              //   (fieldArr[index].subProperties as FormField[][]).push(arrItem);
                              // }
                              //方法一：返回 FormField[][]
                              const arrItem: FormField[][] = addArrayParams(field.subProperties, field.name);
                              let fieldArr = jsClone(this.formFields) as FormField[];
                              fieldArr[index].subProperties = arrItem;
                              this.formFields = fieldArr;
                              console.log("addArrayParams2 = "+JSON.stringify(this.formFields[index].subProperties))
                            })
                        }else {
                          Text("-")
                            .fontSize(24)
                            .fontColor('rgba(0,0,0,0.6)')
                            .fontWeight(500)
                            .margin({ right: 20})
                            .padding(10)
                            .onClick(()=>{
                              const arrItem: FormField[][] = removeArrayParams(field.subProperties, field.name, arrIndex);
                              let fieldArr = jsClone(this.formFields) as FormField[];
                              fieldArr[index].subProperties = arrItem;
                              this.formFields = fieldArr;
                              console.log("addArrayParams2 = "+JSON.stringify(this.formFields[index].subProperties))
                            })
                        }
                      }
                      .borderRadius({ topLeft: 20, topRight: 20 })
                      .backgroundColor('#e0e0e0')
                      .margin({bottom: 15,right:-10,left:-10 })

                      this.buildFormFields(fieldItem as FormField[])
                    }
                    // .margin(15)
                    .borderRadius(20)
                    .margin({bottom: 20})
                    .padding({left:10,right:10})
                    .backgroundColor('#f0f0f1')
                  })
                }
              })
            }

          }
          .padding({bottom:70})
          .margin(16)
        }
        .width('100%')
        .alignItems(HorizontalAlign.Start)
      }
      .align(Alignment.TopStart)
      .constraintSize({ minHeight: '100%' })
      .edgeEffect(EdgeEffect.Spring)
      .scrollable(ScrollDirection.Vertical)
      // .scrollBar(BarState.Auto)
      .scrollBar(BarState.Off)
      .backgroundColor('#F2F3F5')
      .width('100%')
      .height('100%')
      .margin({ bottom: 0 })  // 确保底部无margin
      .padding({ bottom: 0 }) // 确保底部无padding

      Row(){
        Button('执行意图', {stateEffect: true })
          .width('100%')
          .fontSize(16)
          .fontWeight(500)
          .backgroundColor('#0x317aff')
          .borderRadius(20)
          .height(40)
            // .align(Alignment.Bottom) // 对齐到底部
            // .zIndex(100)
          .onClick(() => {
            this.submitForm()
          })
          .bindSheet(this.isShow, this.resultModule(), {
            height: SheetSize.MEDIUM,
            blurStyle: BlurStyle.Thick,
            // showClose: false,
            title: { title: "执行结果"},//, subtitle: "subtitle"
            preferType: SheetType.BOTTOM,
            shouldDismiss:(()=>{
              if(this.isShowEntry2) this.isShowEntry2=!this.isShowEntry2;
              this.isShow = false;
            }),
            onWillDismiss: ((DismissSheetAction: DismissSheetAction) => {
              if (DismissSheetAction.reason == DismissReason.SLIDE_DOWN) {
                DismissSheetAction.dismiss(); //注册dismiss行为
              }
            }),
            onWillSpringBackWhenDismiss: ((SpringBackAction: SpringBackAction) => {
              //没有注册springBack，下拉半模态页面无回弹行为
              SpringBackAction.springBack();
            }),
          })
      }
      .width('100%')
      .padding(16)
      .position({bottom:28,left:0})
    }
    .hideTitleBar(true)
  }
}

