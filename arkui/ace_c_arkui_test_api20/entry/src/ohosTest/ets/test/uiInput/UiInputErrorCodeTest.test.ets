/*
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

import { beforeAll, describe, expect, it, Level, afterEach } from '@ohos/hypium';
import Utils from '../common/Utils';
import { Driver, ON } from '@ohos.UiTest';

export default function uiInputErrorCodeTest() {

  describe('UiInputErrorCodeTest', () => {

    let PAGE_TAG = 'UiInputErrorCodeTest';

    beforeAll(async (done: Function) => {
      await Utils.pushPage(`uiInput/${PAGE_TAG}`, done);
    });

    /*
    * @tc.number     : errorCode_001
    * @tc.name       : errorCode_001
    * @tc.desc       : test OH_ArkUI_AddSupportedUIStates return 180006
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('errorCode_001', Level.LEVEL1, async (done: Function) => {
      let driver = Driver.create();
      let component = await driver.findComponent(ON.id('ErrorCodeButton'));
      await Utils.sleep(2000);
      await component.click();
      await Utils.sleep(2000);
      let strJson = getInspectorByKey('ErrorCodetextStates');
      await Utils.sleep(1000);
      let obj: ESObject = JSON.parse(strJson);
      console.info("errorCode_001 content is: " + JSON.stringify(obj.$attrs.content));
      expect(obj.$attrs.content).assertEqual('180006');
      done();
    })

    /*
    * @tc.number     : errorCode_002
    * @tc.name       : errorCode_002
    * @tc.desc       : test OH_ArkUI_AddSupportedUIStates return 0
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('errorCode_002', Level.LEVEL1, async (done: Function) => {
      let driver = Driver.create();
      let component = await driver.findComponent(ON.id('ErrorCodeButton'));
      await Utils.sleep(2000);
      await component.click();
      await Utils.sleep(2000);
      let strJson = getInspectorByKey('ErrorCodetextStates1');
      await Utils.sleep(1000);
      let obj: ESObject = JSON.parse(strJson);
      console.info("errorCode_002 content is: " + JSON.stringify(obj.$attrs.content));
      expect(obj.$attrs.content).assertEqual('0');
      done();
    })

    /*
    * @tc.number     : errorCode_003
    * @tc.name       : errorCode_003
    * @tc.desc       : test OH_ArkUI_AddSupportedUIStates return 401
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('errorCode_003', Level.LEVEL1, async (done: Function) => {
      let driver = Driver.create();
      let component = await driver.findComponent(ON.id('ErrorCodeButton'));
      await Utils.sleep(2000);
      await component.click();
      await Utils.sleep(2000);
      let strJson = getInspectorByKey('ErrorCodetextStates2');
      await Utils.sleep(1000);
      let obj: ESObject = JSON.parse(strJson);
      console.info("errorCode_003 content is: " + JSON.stringify(obj.$attrs.content));
      expect(obj.$attrs.content).assertEqual('401');
      done();
    })
  })
}