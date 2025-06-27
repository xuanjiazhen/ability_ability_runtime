import { IntentPage } from 'intent'

@Entry
@Component
struct Index {
  build() {
    Column(){
      IntentPage()
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#F1F3F5')
    .padding(0)
  }
}