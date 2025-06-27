import { IntentContent } from '../view/IntentContent'
import { IntentDetailPage } from '../view/IntentDetailPage'
import { IntentInfo } from '../view/IntentContent'
import { StartOptions } from '@kit.AbilityKit'

// @Entry
@Component
export struct IntentPage {
  @Provide('articlePathStack') articlePathStack: NavPathStack = new NavPathStack();

  scroller: Scroller = new Scroller();
  @State title: string ="所有意图"

  @Builder
  intentRouter(name: string, param?: IntentInfo) {
    // intentRouter(name: string) {
    if (name === 'intentDetail') {
      IntentDetailPage()
    }
  }

  build() {
    Navigation(this.articlePathStack) {
      Column() {
        Scroll(this.scroller) {
          Column() {
            IntentContent()
          }
          .width('100%')
          // .backgroundColor('#f1f3f5')

        }
        .align(Alignment.TopStart)
        .constraintSize({ minHeight: '100%' })
        .edgeEffect(EdgeEffect.Spring)
        .scrollable(ScrollDirection.Vertical)
        .scrollBar(BarState.Off)
      }
      .width('100%')
      .height('100%')
      .backgroundColor('#F1F3F5')
    }
    .navDestination(this.intentRouter)
    .hideTitleBar(true)
    .mode(NavigationMode.Stack)
  }
}
