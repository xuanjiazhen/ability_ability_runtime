// xxx.ets
import { webview } from '@kit.ArkWeb';
import { pasteboard } from '@kit.BasicServicesKit';
import events_emitter from '@ohos.events.emitter';
import { waitForExist} from '../../test/WaitTest.test';
import { Driver, ON } from '@ohos.UiTest';
import Utils from '../../test/Utils';

const TAG = 'ContextMenu';

@Entry
@Component
struct WebComponent {
  controller: webview.WebviewController = new webview.WebviewController();
  private result: WebContextMenuResult | undefined = undefined;
  @State linkUrl: string = '';
  @State offsetX: number = 0;
  @State offsetY: number = 0;
  @State showMenu: boolean = false;
  @State emitKey: string = '';
  @State isReceive: number = -1;
  @State str:string = ''
  @State MediaTypeArr:ContextMenuMediaType[] = [ContextMenuMediaType.None,ContextMenuMediaType.Image,ContextMenuMediaType.VIDEO,ContextMenuMediaType.AUDIO]
  @State getMediaType: number = 0

  @Builder
  // 构建自定义菜单及触发功能接口
  MenuBuilder() {
    // 以垂直列表形式显示的菜单。
    Menu() {
      // 展示菜单Menu中具体的item菜单项。
      MenuItem({
        content: '粘贴',
      })
        .width(100)
        .height(50)
        .onClick(() => {
          this.result?.paste();
          this.showMenu = false;
        })
      MenuItem({
        content: '复制链接',
      })
        .width(100)
        .height(50)
        .onClick(() => {
          let pasteData = pasteboard.createData('text/plain', this.linkUrl);
          pasteboard.getSystemPasteboard().setData(pasteData, (error) => {
            if (error) {
              return;
            }
          })
          this.showMenu = false;
        })
      MenuItem({
        content: '复制',
      })
        .width(100)
        .height(50)
        .onClick(() => {
          this.result?.copy();
          this.showMenu = false;
        })
    }
    .width(150)
    .height(450)
  }

  onPageShow() {
    let valueChangeEvent: events_emitter.InnerEvent = {
      eventId: 10,
      priority: events_emitter.EventPriority.LOW
    }
    events_emitter.on(valueChangeEvent, this.valueChangeCallBack)
  }

  private valueChangeCallBack = (eventData: events_emitter.EventData) => {
    if (eventData != null) {
      console.info('valueChangeCallBack:' + JSON.stringify(eventData));
      if (eventData.data != null && eventData.data.ACTION != null) {
        this.str = eventData.data.ACTION;
        this.isReceive = 0;
      }
    }
  }

  build() {
    Column() {
      Row() {
        Button("web click")
          .key("webviewOnContextMenuShowTwoButton")
          .onClick(async () => {
            console.info("key==>" + this.str)
            await waitForExist(()=>this.isReceive, this.str, 0);
            switch (this.str) {
              case "emitOnContextMenuShowTwo001": {
                try {
                  let driver = Driver.create();
                  let button = await driver.findComponent(ON.id('OnContextMenuShowTwoKey'));
                  await button.longClick();
                  await Utils.sleep(2000);
                  break;
                } catch (error) {
                  console.error(`ErrorCode: ${error.code},  Message: ${error.message}`);
                }
                break;
              }

            }
          })

      }

      Web({ src: $rawfile("OnContextMenuShowVideo.html"), controller: this.controller })
        .key('OnContextMenuShowTwoKey')
      // 触发自定义弹窗
        .onContextMenuShow((event) => {
          if (event) {
            this.result = event.result
            console.info("onContextMenuShow SourceUrl = " + event.param.getSourceUrl());
            console.info("onContextMenuShow MediaType = " + event.param.getMediaType());
            this.getMediaType = event.param.getMediaType();
            Utils.emitEvent(this.MediaTypeArr[this.getMediaType], 20250709)
          }
          console.info(TAG, `x: ${this.offsetX}, y: ${this.offsetY}`);
          this.showMenu = true;
          this.offsetX = 0;
          this.offsetY = Math.max(px2vp(event?.param.y() ?? 0) - 0, 0);
          return true;
        })
        .bindPopup(this.showMenu,
          {
            builder: this.MenuBuilder(),
            enableArrow: false,
            placement: Placement.LeftTop,
            // offset: { x: this.offsetX, y: this.offsetY },
            mask: false,
            onStateChange: (e) => {
              if (!e.isVisible) {
                this.showMenu = false;
                this.result!.closeContextMenu();
              }
            }
          })
    }
  }
}