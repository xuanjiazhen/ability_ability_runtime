// import { insightIntentDriver } from '@kit.AbilityKit';
import insightIntentDriver from '@ohos.app.ability.insightIntentDriver';
import { BusinessError } from '@kit.BasicServicesKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import application from '@ohos.app.ability.application';
import { FormPage } from './FormPage';
import { jsClone } from './formate';

export interface ItemInfo {
  title: string,
  brief: string
}
export interface IntentInfo extends insightIntentDriver.InsightIntentInfo {
  bundleNameCN: string;// 扩展接口（添加新属性）
}

export interface IntentGrout {
  string : IntentInfo[]
}



const TypeIntentIcon: Record<string, string> = {
  '闹钟1': 'app.media.ic_ohos',
  '睡眠': 'app.media.ic_camera',
  '邮件1': 'app.media.ic_webstorm',
  '邮件2': 'app.media.ic_vv',
}

@Component
export struct IntentContent{
  @Consume('articlePathStack') articlePathStack: NavPathStack;

  @State title: string = '所有意图'
  // @State listMaterial: Record<string, insightIntentDriver.InsightIntentInfo[]> | null = null

  @State listMaterial: IntentInfo[] | null = null
  @State groupMaterial1: Record<string, IntentInfo[]> = {}
  @State groupMaterial: Record<string, IntentInfo[]> = {}
  // @State groupMaterial: Promise<Record<string, insightIntentDriver.InsightIntentInfo[]>> | null= null

  aboutToAppear() {
    /*
    this.listMaterial = [{
      bundleName:'test',moduleName:'test',intentName:'test',domain:'test',intentVersion:'test',displayName:'test',displayDescription:'test',schema:'test',icon:'test',llmDescription:'test',keywords:['test'],intentType:'test',subIntentInfo:'test',
      parameters:{
        "playbackSpeed": 'test',
        "playbackProgress": 'test'
      }
    }]


    this.groupMaterial =this.listMaterial.reduce((acc, item) => {
      const domain: string = item.domain;
      if (!acc[domain]) {
        acc[domain] = [];
      }
      acc[domain].push(item);
      return acc;
      // acc[item.domain] = [...(acc[item.domain] || []), item];
    }, {} as Record< string , insightIntentDriver.InsightIntentInfo[]>)
    */
    this.getInfos()
    // this.fetchData()
  }

  async getInfos() {
    // let  _this = this;
    try {
      insightIntentDriver.getAllInsightIntentInfo(insightIntentDriver.GetInsightIntentFlag.GET_FULL_INSIGHT_INTENT).then((data) => {
        // insightIntentDriver.getAllInsightIntentInfo(1).then((data) => {
        // console.log("getAllInsightIntentInfo")
        // console.log(JSON.stringify(data))
        // hilog.info(0x0000, 'testTag', 'getAllInsightIntentInfo return %{public}s', JSON.stringify(data));
        console.log(`getAllInsightIntentInfo = ${JSON.stringify(data)}`)
        // const changeData = data as IntentInfo[];
        this.listMaterial =  data as IntentInfo[];

        this.groupMaterial1 = this.listMaterial.reduce((acc, item) => {
          const bundleName: string = item.bundleName;
          if (!acc[bundleName]) {
            acc[bundleName] = [];
          }
          acc[bundleName].push(item);
          return acc;
          // acc[item.domain] = [...(acc[item.domain] || []), item];
        }, {} as Record< string , IntentInfo[]>)

        Object.keys(this.groupMaterial1).forEach((key:string, index:number)=>{

          this.setAppName(this.groupMaterial1[key][0]).then((value)=>{
            console.log(`getAllInsightIntentInfo2 =  ${value} + ${JSON.stringify(this.groupMaterial1[key])}`)
            this.groupMaterial[value] = this.groupMaterial1[key]
            console.log(`getAllInsightIntentInfo3 = ${JSON.stringify(this.groupMaterial[value])}`)
          })
          if(index = Object.keys(this.groupMaterial1).length -1){
            this.groupMaterial = jsClone(this.groupMaterial)
          }
        })
/*
        const _this = this;

        setTimeout(()=>{

          this.listMaterial = changeData.map((currentValue: IntentInfo, index: number) => {
            // 对 currentValue 进行处理
            // currentValue.bundleNameCN = this.setAppName(currentValue);
            // currentValue.bundleNameCN = this.getAppName(currentValue);
            this.setAppName(currentValue).then((value)=>{
              currentValue.bundleNameCN = value
            })
            return currentValue; // 返回新数组的元素
          });

          this.groupMaterial = this.listMaterial.reduce((acc, item) => {
            const bundleNameCN: string = item.bundleNameCN;
            if (!acc[bundleNameCN]) {
              acc[bundleNameCN] = [];
            }
            acc[bundleNameCN].push(item);
            return acc;
            // acc[item.domain] = [...(acc[item.domain] || []), item];
          }, {} as Record< string , IntentInfo[]>)

          console.log(`getAllInsightIntentInfo2 = ${JSON.stringify(this.listMaterial)}`)
        },0);
*/

        /*
        //根据 domain 分类
        this.groupMaterial =this.listMaterial.reduce((acc, item) => {
          const domain: string = item.domain;
          if (!acc[domain]) {
            acc[domain] = [];
          }
          acc[domain].push(item);
          return acc;
          // acc[item.domain] = [...(acc[item.domain] || []), item];
        }, {} as Record< string , insightIntentDriver.InsightIntentInfo[]>)
*/
      }).catch((err: BusinessError) => {
        console.log(`BusinessError :err.code = ${err.code}`)
        console.log(`BusinessError :err.message = ${err.message}`)
        hilog.info(0x0000, 'testTag', 'getAllInsightIntentInfo errCode: %{public}d', err.code);
        hilog.info(0x0000, 'testTag', 'getAllInsightIntentInfo errMessage: %{public}s', err.message);
      });
    } catch (error) {
      hilog.error(0x0000, 'testTag', 'getAllInsightIntentInfo error caught %{public}s', JSON.stringify(error));
      console.log(`getAllInsightIntentInfo BusinessError : JSON.stringify(error)`)
    }
  }
//方法二
  /*
  async fetchData() {
    try {
      this.listMaterial = await insightIntentDriver.getAllInsightIntentInfo(1);
      // this.listMaterial = [{
      //   bundleName:'test',moduleName:'test',intentName:'test',domain:'test',intentVersion:'test',displayName:'test',displayDescription:'test',schema:'test',icon:'test',llmDescription:'test',keywords:['test'],intentType:'test',subIntentInfo:'test',
      //   parameters:{
      //     "playbackSpeed": 'test',
      //     "playbackProgress": 'test'
      //   }
      // }]

      // const group: Record<string, insightIntentDriver.InsightIntentInfo[] | []> = this.listMaterial.reduce((acc, item) => {
      this.groupMaterial =this.listMaterial.reduce((acc, item) => {
          const domain: string = item.domain;
        if (!acc[domain]) {
          acc[domain] = [];
        }
        acc[domain].push(item);
        return acc;
        // acc[item.domain] = [...(acc[item.domain] || []), item];
      }, {} as Record< string , insightIntentDriver.InsightIntentInfo[]>)

    } catch (error) {
      console.log(`getAllInsightIntentInfo BusinessError : JSON.stringify(error)`)
      hilog.error(0x0000, 'testTag', 'getAllInsightIntentInfo error caught %{public}s', JSON.stringify(error));
    }

    // const respons: Record<string, insightIntentDriver.InsightIntentInfo[]> = intentSession.reduce((acc, item) => {
    //   acc[item.category] = [...(acc[item.category] || []), item];
    //   return acc;
    // }, {} as { [key: string]: insightIntentDriver.InsightIntentInfo[] })

  }
*/
  async getAppName(intentDetail: IntentInfo) {
    const bundleNameCN: string = await this.setAppName(intentDetail); // 等待 Promise 完成
    return bundleNameCN
  }

  async setAppName(intentDetail: IntentInfo): Promise<string>{
    let bundleName: string = intentDetail.bundleName;
    let moduleName: string = intentDetail.moduleName;
    let context = AppStorage.get('context') as Context;

    const bundleContext: Context = await application.createModuleContext(context, bundleName, moduleName);
    // const bundleNameCN : string = bundleContext.resourceManager.getStringSync(bundleContext.applicationInfo.labelId)
    const bundleNameCN : string = bundleContext.resourceManager.getStringSync(bundleContext.applicationInfo.labelId)
// console.log(`setAppName = ${bundleName}、${JSON.stringify(bundleContext)}、${moduleName}、${bundleNameCN}`)
    console.log(`setAppName = ${bundleName}、${moduleName}、${bundleNameCN}`)
    return  bundleNameCN
  }
  @Builder
  IntentBlock(domainType:string, material: IntentInfo[]) {
      Column() {
        Text(domainType)
          .fontFamily('HarmonyHeiTi-Medium')
          .fontSize(16)
          .fontWeight(500)
          .fontColor("#67686A")
          .margin({ left: 10,  bottom: 8 })
          .textAlign(TextAlign.Start)
          .width('100%')

        List({ space: 12 }) {
          ForEach(material, (item: IntentInfo, index: number) => {
            ListItem(){
              this.IntentBlockLine(item)
            }
            .onClick(() => {
                this.articlePathStack.pushPathByName('intentDetail', item)
                // NavPathStack.pushPathByName(name: 'intentDetail', param: item, animated?: boolean | undefined): void (+1 overload)
            })
          }, (item: IntentInfo, index: number) => item.intentName)
        }
        .backgroundColor(Color.White)
        .borderRadius(16)
        .padding({ top:4, left: 12, right: 12 ,bottom:4})
        .divider({
          strokeWidth: 1,
          startMargin: 38,
          endMargin: 0,
          color: 'rgba(0,0,0,.2)'
        })

      }
      .width('100%')
      .margin({ top: 10, bottom: 18 })
      .alignItems(HorizontalAlign.Center)
  }

  @Builder
  IntentBlockLine(intentDetail: IntentInfo){
    Row() {
      // Image(intentDetail.icon)
      Image($r('app.media.ic_webstorm'))
        .width(24)
        .height(24)
        .backgroundColor("#cfcfcf")

      Column() {
        Text(intentDetail.displayName)
          .fontFamily('HarmonyHeiTi-Medium')
          .fontSize(16)
          .fontColor("#494947")
          .fontWeight(500)
          .margin({bottom:6})
        Text(intentDetail.displayDescription)
          .fontFamily('HarmonyHeiTi')
          .fontSize(14)
          .fontWeight(400)
          .fontColor("#777775")
      }
      .alignItems(HorizontalAlign.Start)
      .margin({ left: 18 })

      Blank()
      Image($r('app.media.ic_arrow'))
        .width(12)
        .height(24)
    }
    .width('100%')
    .height(64)
    .alignItems(VerticalAlign.Center)
  }

  build() {
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
      .alignItems(VerticalAlign.Center)
      .width('100%')
      .height(56)
      .padding({ top:14.5,right:16,left:16,bottom:14.5 })
      // .backgroundColor(Color.White)
      .onClick(() => {
        this.articlePathStack.pop()
      })

      Column() {
        // Text(JSON.stringify(this.groupMaterial))

        ForEach(Object.keys(this.groupMaterial), (bundleNameCN: string) => {
          Text(JSON.stringify(this.groupMaterial.bundleNameCN))
          this.IntentBlock(bundleNameCN, this.groupMaterial[bundleNameCN])
          //this.IntentBlock(domainType, this.groupMaterial[domainType])
        }, (bundleNameCN: string) => bundleNameCN)
      }
      .padding(20)
    }

  }
}

