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
import { afterEach, beforeAll, beforeEach, describe, it, expect , TestType, Size, Level } from '@ohos/hypium';
import events_emitter from '@ohos.events.emitter';
import Utils from './Utils';
import router from '@ohos.router';
import emitter from '@ohos.events.emitter';
import { Driver, ON } from '@ohos.UiTest';
import { accessibility, config } from '@kit.AccessibilityKit';
const CAPACITY: accessibility.Capability[] = ['retrieve', 'gesture'];
const BUNDLE_NAME = 'com.example.myaccessibilityapp/AccessibilityExtAbility';
// @ts-nocheck
let emitKey = ''

export default function ActsAceWebViewOnPdfLoadEvent() {
  describe('ActsAcewebViewWebViewOnPdfLoadEvent', () => {

    beforeAll(async (done: Function) => {
      let options : router.RouterOptions = {
        url: 'testability/pages/WebViewOnPdfLoadEvent',
      }

      try {
        await Utils.sleep(1000)
        router.clear();
        let pages = router.getState();
        console.info("get WebViewOnPdfLoadEvent state success " + JSON.stringify(pages));
        if (!("WebViewOnPdfLoadEvent" == pages.name)) {
          console.info("get WebViewOnPdfLoadEvent state success " + JSON.stringify(pages.name));
          let result = await router.pushUrl(options);
          await Utils.sleep(1000);
          console.info("push WebViewOnPdfLoadEvent state success " + JSON.stringify(result));
        }
      } catch (err) {
        console.error("push WebViewOnPdfLoadEvent page error:" + err);
      }
      let driver: Driver = Driver.create();
      await driver.waitForComponent(ON.id("WebViewOnPdfLoadEventKey"),10000);
      done()
    });

    beforeEach(async (done: Function) => {
      config.enableAbility(BUNDLE_NAME, CAPACITY).then(() => {
        console.info(`enable success`);
      }).catch((err: object) => {
        console.error(`failed to enable, ${err}`);
      })
      await Utils.sleep(2000);
      done();
    })

    afterEach(async (done: Function) => {
      try {
        let backData: events_emitter.EventData = {
          data: {
            "ACTION": emitKey
          }
        }
        let backEvent : events_emitter.InnerEvent = {
          eventId: 10,
          priority: events_emitter.EventPriority.LOW
        }
        console.info("start send emitKey");
        events_emitter.emit(backEvent, backData);
      } catch (err) {
        console.error("emit emitKey error:" + JSON.stringify(err));
      }
      await Utils.sleep(1000);
      done();
    })


    /*
      * @tc.number      : SUB_ACE_WEB_WEBVIEWWEBVIEWONPDFLOADEVENT_API_0001
      * @tc.name        : emitwebViewWebViewOnPdfLoadEvent001
      * @tc.desc        : emitwebViewWebViewOnPdfLoadEvent001
      * @tc.size        : MediumTest
      * @tc.type        : Function
      * @tc.level       : Level 0
      */
    it('emitwebViewWebViewOnPdfLoadEvent001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0, async (done: Function) => {
      Utils.emitEvent("emitwebViewWebViewOnPdfLoadEvent001", 10)
      await Utils.sleep(2000);
      Utils.registerEvent("emitwebViewWebViewOnPdfLoadEvent001", 'resource://rawfile/printTemplate.pdf', 20250822, done);
      sendEventByKey('WebViewOnPdfLoadEventKey', 10, '');
    })

    /*
       * @tc.number      : SUB_ACE_WEB_WEBVIEWWEBVIEWONPDFLOADEVENT_API_0002
       * @tc.name        : emitwebViewWebViewOnPdfLoadEvent002
       * @tc.desc        : emitwebViewWebViewOnPdfLoadEvent002
       * @tc.size        : MediumTest
       * @tc.type        : Function
       * @tc.level       : Level 0
       */
    it('emitwebViewWebViewOnPdfLoadEvent002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0, async (done: Function) => {
      Utils.emitEvent("emitwebViewWebViewOnPdfLoadEvent002", 10)
      await Utils.sleep(2000);
      Utils.registerEvent("emitwebViewWebViewOnPdfLoadEvent002", 0, 20250823, done);
      sendEventByKey('WebViewOnPdfLoadEventKey', 10, '');
    })

    /*
   * @tc.number      : SUB_ACE_WEB_WEBVIEWWEBVIEWONPDFLOADEVENT_API_0003
   * @tc.name        : emitwebViewWebViewOnPdfLoadEvent003
   * @tc.desc        : emitwebViewWebViewOnPdfLoadEvent003
   * @tc.size        : MediumTest
   * @tc.type        : Function
   * @tc.level       : Level 0
   */
    it('emitwebViewWebViewOnPdfLoadEvent003', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0, async (done: Function) => {
      Utils.emitEvent("emitwebViewWebViewOnPdfLoadEvent003", 10)
      await Utils.sleep(2000);
      Utils.registerEvent("emitwebViewWebViewOnPdfLoadEvent003", "resource://rawfile/small.pdf", 20250824, done);
      sendEventByKey('WebViewOnPdfLoadEventKey', 10, '');
    })

  })
}
