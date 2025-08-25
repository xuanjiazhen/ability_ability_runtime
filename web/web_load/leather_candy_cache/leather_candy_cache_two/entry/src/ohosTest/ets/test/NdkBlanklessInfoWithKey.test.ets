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

import { beforeAll, beforeEach, describe, it, TestType, Size, Level, expect } from '@ohos/hypium';
import router from '@ohos.router';
import Utils from './Utils';
import { Component, Driver, ON } from '@kit.TestKit';
import events_emitter from '@ohos.events.emitter';

const driver = Driver.create();

export default function ndkBlanklessInfoWithKeyTest() {

  describe('ndkBlanklessInfoWithKeyTest', () => {

    beforeAll(async (done: Function) => {
      let options: router.RouterOptions = {
        url: 'testability/pages/NdkBlanklessInfoWithKeyTest',
      }
      try {
        router.clear();
        let pages = router.getState();
        console.info('get ndkBlanklessInfoWithKeyTest state success ' + JSON.stringify(pages));
        if (!("blankLessInfoWithKeyTest" == pages.name)) {
          console.info('get ndkBlanklessInfoWithKeyTest state success ' + JSON.stringify(pages.name));
          let result = await router.pushUrl(options);
          console.info('push ndkBlanklessInfoWithKeyTest page success ' + JSON.stringify(result));
        }
      } catch (err) {
        console.error('push ndkBlanklessInfoWithKeyTest page error: ' + err);
      }
      done()
    });

    /*
    * @tc.number     : SUB_WEB_WEBVIEWCONTROLLER_NDKBLANKLESSINFOWITHKEYTEST_0100
    * @tc.name       : testndkBlanklessInfoWithKey001
    * @tc.desc       : test ndkBlanklessInfoWithKey
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testndkBlanklessInfoWithKey001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      Utils.emitEvent("testndkBlanklessInfoWithKey001", 148101);
      Utils.registerBaseEvent("testndkBlanklessInfoWithKey001", 0, 148110, done,
        (backData: events_emitter.EventData, expected: Object) => {
          expect(backData?.data?.ACTION).assertEqual(expected)
        });
      Utils.waitAndClickComponent('ndkBlanklessInfoWithKeyTestButton', driver);
    })
    /*
    * @tc.number     : SUB_WEB_WEBVIEWCONTROLLER_NDKBLANKLESSINFOWITHKEYTEST_0200
    * @tc.name       : testndkBlanklessInfoWithKey002
    * @tc.desc       : test ndkBlanklessInfoWithKey
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testndkBlanklessInfoWithKey002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      Utils.emitEvent("testndkBlanklessInfoWithKey002", 148101);
      Utils.registerBaseEvent("testndkBlanklessInfoWithKey002", -2, 148120, done,
        (backData: events_emitter.EventData, expected: Object) => {
          expect(backData?.data?.ACTION).assertEqual(expected)
        });
      Utils.waitAndClickComponent('ndkBlanklessInfoWithKeyTestButton', driver);
    })
    /*
    * @tc.number     : SUB_WEB_WEBVIEWCONTROLLER_NDKBLANKLESSINFOWITHKEYTEST_0300
    * @tc.name       : testndkBlanklessInfoWithKey003
    * @tc.desc       : test ndkBlanklessInfoWithKey
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testndkBlanklessInfoWithKey003', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      Utils.emitEvent("testndkBlanklessInfoWithKey003", 148101);
      Utils.registerBaseEvent("testndkBlanklessInfoWithKey003", -3, 148130, done,
        (backData: events_emitter.EventData, expected: Object) => {
          expect(backData?.data?.ACTION).assertEqual(expected)
        });
      Utils.waitAndClickComponent('ndkBlanklessInfoWithKeyTestButton', driver);
    })

    /*
    * @tc.number     : SUB_WEB_WEBVIEWCONTROLLER_NDKBLANKLESSINFOWITHKEYTEST_0400
    * @tc.name       : testndkBlanklessInfoWithKey004
    * @tc.desc       : test ndkBlanklessInfoWithKey
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testndkBlanklessInfoWithKey004', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      Utils.emitEvent("testndkBlanklessInfoWithKey004", 148101)
      Utils.registerBaseEvent("testndkBlanklessInfoWithKey004", -4, 148140, done,
        (backData: events_emitter.EventData, expected: Object) => {
          expect(backData?.data?.ACTION).assertEqual(expected)
        });
      Utils.waitAndClickComponent('ndkBlanklessInfoWithKeyTestButton', driver);
    })
    /*
    * @tc.number     : SUB_WEB_WEBVIEWCONTROLLER_NDKBLANKLESSINFOWITHKEYTEST_0500
    * @tc.name       : testndkBlanklessInfoWithKey005
    * @tc.desc       : test ndkBlanklessInfoWithKey
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testndkBlanklessInfoWithKey005', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      Utils.emitEvent("testndkBlanklessInfoWithKey005", 148101);
      Utils.registerBaseEvent("testndkBlanklessInfoWithKey005", -5, 148150, done,
        (backData: events_emitter.EventData, expected: Object) => {
          expect(backData?.data?.ACTION).assertEqual(expected)
        });
      Utils.waitAndClickComponent('ndkBlanklessInfoWithKeyTestButton', driver);
    })
  })
}
