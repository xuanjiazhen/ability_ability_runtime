import { IntentInfo } from './IntentContent'
@Component
export struct DetailItem{
  @Prop intentDetail: IntentInfo | null = null;
  @Prop intentDetailAll: Array<[string, string|string[]|object]> | [] = []
  title: string = "意图信息";
  // @State intentDetailEntries: Array<[string, string|string[]|object]> | [] = []

  aboutToAppear(): void {
    // this.intentDetail =
  }
  @Builder
  DetailBlock(entry: string|string[]|object) {
    if(entry[0]=="keywords"){
      this.DetailBox(entry[0], entry[1].join(','))
    }
    else if(entry[0]=="subIntentInfo"){
      // this.DetailBox(entry[0], entry[1].abilityName)
    }else {
      this.DetailBox(entry[0], entry[1])
    }
  }

  @Builder
  DetailBox(label:string, val: string = ""){
    Column(){
      Text(label)
        .fontSize(16)
        .fontWeight(500)
        .fontColor('rgba(0,0,0,0.9)')
        .fontFamily('HarmonyHeiTi-Medium')
        .lineHeight(22)
        .width('100%')

      Text(val)
        .fontSize(14)
        .fontWeight(400)
        .fontColor('rgba(0,0,0,0.6)')
        .fontFamily('HarmonyHeiTi')
        .lineHeight(20)
        .maxLines(1)
        .constraintSize({maxWidth: '100%'})
        .textOverflow({ overflow: TextOverflow.Ellipsis })
        .width('100%')
    }
    .width('100%')
    .padding({top: 10, bottom:10 })
    .alignItems(HorizontalAlign.Start)
  }

  build() {
    Column(){

      Text(this.title)
        .fontFamily('HarmonyHeiTi-Medium')
        .fontSize(14)
        .fontWeight(500)
        .fontColor('rgba(0,0,0,0.6)')
        .textAlign(TextAlign.Start)
        .lineHeight(20)
        .width('100%')
        .padding({top:28,bottom:8,left:30,right:30})

      Column(){
        List({ space: 0 }) {
          ListItem(){
            this.DetailBox("应用包名",this.intentDetail?.bundleName)
          }
          ListItem(){
            this.DetailBox("意图名称",this.intentDetail?.intentName)
          }
          ListItem(){
            this.DetailBox("执行模式",this.intentDetail?.intentType)
          }
          /*
          //一下是循环打印所有详情元素
          ForEach(this.intentDetailEntries, (entry: [string, string]) => {

            if(entry[0] != "parameters"){
              ListItem(){
                this.DetailBlock(entry)
              }
            }

          },(key: string) => key)
          * */
        }
        .scrollBar(BarState.Off)
        .divider({
          strokeWidth: 0,
          startMargin: 0,
          endMargin: 0,
          color: '#F2F2F2'
        })
      }
      .margin({ left:18,right:18,bottom:10 })
      .padding({top:3,right:12,bottom:12,left:12})
      .alignItems(HorizontalAlign.Start)
      .backgroundColor(Color.White)
      .borderRadius(20)
    }
  }

}