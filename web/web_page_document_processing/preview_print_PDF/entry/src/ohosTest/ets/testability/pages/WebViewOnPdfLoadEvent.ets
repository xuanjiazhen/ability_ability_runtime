/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// xxx.ets
import { webview } from '@kit.ArkWeb';
import { waitForExist} from '../../test/WaitTest.test';
import events_emitter from '@ohos.events.emitter';
import Utils from '../../test/Utils';
import { Driver, MouseButton, UiWindow, ON, On  } from '@ohos.UiTest';

@Entry
@Component
struct WebComponent {
  controller: webview.WebviewController = new webview.WebviewController();
  @State eventInfoUrl:string = '';
  @State result:number = 0;
  @State emitKey: string = '';
  @State isReceive: number = -1;
  @State isPageEnd: number = -1;
  @State url: string = '';
  @State ScrollUrl:string = '';
  @State Scrollresult:number = 0;
  onPageShow() {
    let valueChangeEvent: events_emitter.InnerEvent = {
      eventId: 10,
      priority: events_emitter.EventPriority.LOW
    }
    events_emitter.on(valueChangeEvent, this.valueChangeCallBack)
  }

  private valueChangeCallBack = (eventData: events_emitter.EventData) => {
    if (eventData != null) {
      if (eventData.data?.ACTION != null) {
        this.emitKey = eventData.data?.ACTION;
        this.isReceive = 0;
      }
    }
  }

  build() {
    Column() {
      Row() {
        Button('web click')
          .key('WebViewOnPdfLoadEventKey')
          .onClick(async () => {
            await waitForExist(()=>this.isReceive, this.emitKey, 0);
            console.info('key==>' + this.emitKey);
            this.isReceive = -1;
            let webRect = Utils.getComponentRect('printTemplateKey');
            let X = Utils.getRectHorizontalCenterX(webRect)
            let Y = Utils.getRectVerticalCenterY(webRect)
            switch (this.emitKey) {
              case 'emitwebViewWebViewOnPdfLoadEvent001': {
                await waitForExist(()=>this.isPageEnd, this.emitKey, 0);
                console.log('eventInfoUrl' + this.eventInfoUrl)
                if(canIUse('SystemCapability.Security.DiaService')) {
                   Utils.emitEvent(this.eventInfoUrl,20250822)
                } else {
                    Utils.emitEvent('resource://rawfile/printTemplate.pdf',20250822)
                }
                break;
              }

              case 'emitwebViewWebViewOnPdfLoadEvent002': {
                await waitForExist(()=>this.isPageEnd, this.emitKey, 0);
                console.log('result' + this.result)
                 if(canIUse('SystemCapability.Security.DiaService')) {
                     Utils.emitEvent(this.result,20250823)
                 } else {
                     Utils.emitEvent(0,20250823)
                 }
                break;
              }

            }
          })
      }
      Web({ src: $rawfile('printTemplate.pdf'), controller: this.controller })
        .key('printTemplateKey')
        .onPdfLoadEvent((eventInfo: OnPdfLoadEvent) => {
          console.info(`Load event callback called. url: ${eventInfo.url}, result: ${eventInfo.result}`)
          this.eventInfoUrl = eventInfo.url;
          this.result = eventInfo.result;
        })
        .onPdfScrollAtBottom((eventInfo: OnPdfScrollEvent) => {
          console.error(`arkwebpdfdebug: scroll at bottom cb called! url: ${eventInfo.url}`)
          this.ScrollUrl = eventInfo.url;
        })
        .onPageEnd((event) => {
          console.log('onPageEnd url：' + event.url);
          this.isPageEnd = 0;
          this.url = event.url;
        })
    }
  }
}