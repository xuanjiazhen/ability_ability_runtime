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

import { describe, it, expect, TestType, Size, Level, beforeAll } from '@ohos/hypium'
import nativerenderndk from 'libnoderenderndk.so'
import Utils from '../common/Utils';
import { Driver } from '@kit.TestKit';

const ARKUI_ERROR_CODE_NO_ERROR = 0;
const ARKUI_ERROR_CODE_ERROR = 1;
const ARKUI_ERROR_CODE_NODE_EVENT_PARAM_INDEX_OUT_OF_RANGE = 106107
const ARKUI_ERROR_CODE_NODE_EVENT_PARAM_INVALID = 106108;
const ARKUI_ERROR_CODE_NODE_EVENT_NO_RETURN = 106109;

export default function NodeEventTest() {
  describe('NodeEventTest', () => {
    beforeAll(async (done: Function) => {
      await Utils.pushPage(`NodeEventTest/NodeEventTest`, done);
    });

    /*
        * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeEventGetNumberValue_0300
        * @tc.name       : testOHArkUINodeEventGetNumberValue0300
        * @tc.desc       : testing OH_ArkUI_NodeEvent_GetNumberValue interface
        * @tc.size       : MediumTest
        * @tc.type       : Function
        * @tc.level      : Level 1
        */
    it('testOHArkUINodeEventGetNumberValue0300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeEventGetNumberValue0300 start====");
          let driver = Driver.create();
          await driver.triggerKey(2022);
          Utils.sleep(1000);
          let result: number = nativerenderndk.GetNumberValueTest_003();
          console.info(`====>testOHArkUINodeEventGetNumberValue0300 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NODE_EVENT_PARAM_INVALID);;
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeEventGetNumberValue0300 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
        * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeEventGetStringValue_0300
        * @tc.name       : testOHArkUINodeEventGetStringValue0300
        * @tc.desc       : testing OH_ArkUI_NodeEvent_GetStringValue interface
        * @tc.size       : MediumTest
        * @tc.type       : Function
        * @tc.level      : Level 1
        */
    it('testOHArkUINodeEventGetStringValue0300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeEventGetStringValue0300 start====");
          let driver = Driver.create();
          await driver.triggerKey(2022);
          Utils.sleep(1000);
          let result: number = nativerenderndk.GetStringValueTest_003();
          console.info(`====>testOHArkUINodeEventGetStringValue0300 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NODE_EVENT_PARAM_INVALID);;
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeEventGetStringValue0300 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
        * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeEventSetReturnNumberValue_0300
        * @tc.name       : testOHArkUINodeEventSetReturnNumberValue0300
        * @tc.desc       : testing OH_ArkUI_NodeEvent_SetReturnNumberValue interface
        * @tc.size       : MediumTest
        * @tc.type       : Function
        * @tc.level      : Level 1
        */
    it('testOHArkUINodeEventSetReturnNumberValue0300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeEventSetReturnNumberValue0300 start====");
          let driver = Driver.create();
          await driver.triggerKey(2022);
          Utils.sleep(1000);
          let result: number = nativerenderndk.SetReturnNumberValueTest_003();
          console.info(`====>testOHArkUINodeEventSetReturnNumberValue0300 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NODE_EVENT_PARAM_INVALID);;
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeEventSetReturnNumberValue0300 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

  })
}