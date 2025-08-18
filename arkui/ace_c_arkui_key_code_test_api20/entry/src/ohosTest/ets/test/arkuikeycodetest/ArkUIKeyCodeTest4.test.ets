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

import { describe, beforeAll, it, expect, TestType, Size, Level } from '@ohos/hypium'
import arkuikeycodendk from 'libarkuikeycodendk.so'
import { Driver } from '@kit.TestKit';
import Utils from '../common/Utils';

const KEY_CODE_EVENT_OK = 1;
const KEY_CODE_EVENT_ERROR = 0;

export default function ArkUIKeyCodeNdkTest4() {
  describe('ArkUIKeyCodeNdkTest4', () => {
    beforeAll(async (done: Function) => {
      await Utils.pushPage(`ArkUIKeyCodeTest/ArkUIKeyCodeTest4`, done);
    });

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_F10_0100
     * @tc.name    : testArkuiKeyCodeF100100
     * @tc.desc    : Test ARKUI_KEYCODE_F10 interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeyCodeF100100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("testArkuiKeyCodeF100100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2099);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventF10();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("testArkuiKeyCodeF100100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_F11_0100
     * @tc.name    : testArkUIKeyCodeF110100
     * @tc.desc    : testing ARKUI_KEYCODE_F11 interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeF110100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeF110100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2100);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventF11();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeF110100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_F12_0100
     * @tc.name    : testArkUIKeyCodeF120100
     * @tc.desc    : testing ARKUI_KEYCODE_F12 interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeF120100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeF120100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2101);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventF12();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeF120100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_NUM_LOCK_0100
     * @tc.name    : testArkUIKeyCodeNumLock0100
     * @tc.desc    : testing ARKUI_KEYCODE_NUM_LOCK interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeNumLock0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeNumLock0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2102);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventNumLock();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeNumLock0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_NUMPAD_0_0100
     * @tc.name    : testArkUIKeyCodeNumpad00100
     * @tc.desc    : testing ARKUI_KEYCODE_NUMPAD_0 interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeNumpad00100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeNumpad00100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2103);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventNumpad0();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeNumpad00100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_NUMPAD_1_0100
     * @tc.name    : testArkUIKeyCodeNumpad10100
     * @tc.desc    : testing ARKUI_KEYCODE_NUMPAD_1 interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeNumpad10100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeNumpad10100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2104);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventNumpad1();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeNumpad10100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_NUMPAD_2_0100
     * @tc.name    : testArkUIKeyCodeNumpad20100
     * @tc.desc    : testing ARKUI_KEYCODE_NUMPAD_2 interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeNumpad20100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeNumpad20100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2105);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventNumpad2();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeNumpad20100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_NUMPAD_3_0100
     * @tc.name    : testArkUIKeyCodeNumpad30100
     * @tc.desc    : testing ARKUI_KEYCODE_NUMPAD_3 interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeNumpad30100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeNumpad30100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2106);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventNumpad3();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeNumpad30100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_NUMPAD_4_0100
     * @tc.name    : testArkUIKeyCodeNumpad40100
     * @tc.desc    : testing ARKUI_KEYCODE_NUMPAD_4 interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeNumpad40100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeNumpad40100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2107);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventNumpad4();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeNumpad40100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_NUMPAD_5_0100
     * @tc.name    : testArkUIKeyCodeNumpad50100
     * @tc.desc    : testing ARKUI_KEYCODE_NUMPAD_5 interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeNumpad50100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeNumpad50100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2108);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventNumpad5();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeNumpad50100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_NUMPAD_6_0100
     * @tc.name    : testArkUIKeyCodeNumpad60100
     * @tc.desc    : testing ARKUI_KEYCODE_NUMPAD_6 interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeNumpad60100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeNumpad60100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2109);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventNumpad6();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeNumpad60100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_NUMPAD_7_0100
     * @tc.name    : testArkUIKeyCodeNumpad70100
     * @tc.desc    : testing ARKUI_KEYCODE_NUMPAD_7 interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeNumpad70100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeNumpad70100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2110);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventNumpad7();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeNumpad70100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_NUMPAD_8_0100
     * @tc.name    : testArkUIKeyCodeNumpad80100
     * @tc.desc    : testing ARKUI_KEYCODE_NUMPAD_8 interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeNumpad80100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeNumpad80100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2111);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventNumpad8();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeNumpad80100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_NUMPAD_9_0100
     * @tc.name    : testArkUIKeyCodeNumpad90100
     * @tc.desc    : testing ARKUI_KEYCODE_NUMPAD_9 interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeNumpad90100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeNumpad90100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2112);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventNumpad9();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeNumpad90100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_NUMPAD_DIVIDE_0100
     * @tc.name    : testArkUIKeyCodeNumpadDivide0100
     * @tc.desc    : testing ARKUI_KEYCODE_NUMPAD_DIVIDE interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeNumpadDivide0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeNumpadDivide0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2113);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventNumpadDivide();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeNumpadDivide0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_NUMPAD_MULTIPLY_0100
     * @tc.name    : testArkUIKeyCodeNumpadMultiply0100
     * @tc.desc    : testing ARKUI_KEYCODE_NUMPAD_MULTIPLY interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeNumpadMultiply0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeNumpadMultiply0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2114);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventNumpadMultiply();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeNumpadMultiply0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_NUMPAD_SUBTRACT_0100
     * @tc.name    : testArkUIKeyCodeNumpadSubtract0100
     * @tc.desc    : testing ARKUI_KEYCODE_NUMPAD_SUBTRACT interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeNumpadSubtract0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeNumpadSubtract0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2115);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventNumpadSubtract();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeNumpadSubtract0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_NUMPAD_ADD_0100
     * @tc.name    : testArkUIKeyCodeNumpadAdd0100
     * @tc.desc    : testing ARKUI_KEYCODE_NUMPAD_ADD interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeNumpadAdd0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeNumpadAdd0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2116);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventNumpadAdd();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeNumpadAdd0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_NUMPAD_DOT_0100
     * @tc.name    : testArkUIKeyCodeNumpadDot0100
     * @tc.desc    : testing ARKUI_KEYCODE_NUMPAD_DOT interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeNumpadDot0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeNumpadDot0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2117);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventNumpadDot();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeNumpadDot0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_NUMPAD_COMMA_0100
     * @tc.name    : testArkUIKeyCodeNumpadComma0100
     * @tc.desc    : testing ARKUI_KEYCODE_NUMPAD_COMMA interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeNumpadComma0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeNumpadComma0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2118);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventNumpadComma();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeNumpadComma0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_NUMPAD_ENTER_0100
     * @tc.name    : testArkUIKeyCodeNumpadEnter0100
     * @tc.desc    : testing ARKUI_KEYCODE_NUMPAD_ENTER interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeNumpadEnter0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeNumpadEnter0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2119);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventNumpadEnter();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeNumpadEnter0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_NUMPAD_EQUALS_0100
     * @tc.name    : testArkUIKeyCodeNumpadEquals0100
     * @tc.desc    : testing ARKUI_KEYCODE_NUMPAD_EQUALS interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeNumpadEquals0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeNumpadEquals0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2120);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventNumpadEquals();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeNumpadEquals0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_NUMPAD_LEFT_PAREN_0100
     * @tc.name    : testArkUIKeyCodeNumpadLeftParen0100
     * @tc.desc    : testing ARKUI_KEYCODE_NUMPAD_LEFT_PAREN interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeNumpadLeftParen0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeNumpadLeftParen0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2121);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventNumpadLeftParen();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeNumpadLeftParen0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_NUMPAD_RIGHT_PAREN_0100
     * @tc.name    : testArkUIKeyCodeNumpadRightParen0100
     * @tc.desc    : testing ARKUI_KEYCODE_NUMPAD_RIGHT_PAREN interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeNumpadRightParen0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeNumpadRightParen0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2122);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventNumpadRightParen();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeNumpadRightParen0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_BUTTON_A_0100
     * @tc.name    : testArkUIKeyCodeButtonA0100
     * @tc.desc    : testing ARKUI_KEYCODE_BUTTON_A interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeButtonA0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeButtonA0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2301);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventButtonA();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeButtonA0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_BUTTON_B_0100
     * @tc.name    : testArkUIKeyCodeButtonB0100
     * @tc.desc    : testing ARKUI_KEYCODE_BUTTON_B interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeButtonB0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeButtonB0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2302);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventButtonB();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeButtonB0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_BUTTON_X_0100
     * @tc.name    : testArkUIKeyCodeButtonX0100
     * @tc.desc    : testing ARKUI_KEYCODE_BUTTON_X interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeButtonX0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeButtonX0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2304);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventButtonX();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeButtonX0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_BUTTON_Y_0100
     * @tc.name    : testArkUIKeyCodeButtonY0100
     * @tc.desc    : testing ARKUI_KEYCODE_BUTTON_Y interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeButtonY0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeButtonY0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2305);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventButtonY();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeButtonY0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_BUTTON_L1_0100
     * @tc.name    : testArkUIKeyCodeButtonL10100
     * @tc.desc    : testing ARKUI_KEYCODE_BUTTON_L1 interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeButtonL10100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeButtonL10100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2307);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventButtonL1();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeButtonL10100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_BUTTON_R1_0100
     * @tc.name    : testArkUIKeyCodeButtonR10100
     * @tc.desc    : testing ARKUI_KEYCODE_BUTTON_R1 interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeButtonR10100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeButtonR10100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2308);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventButtonR1();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeButtonR10100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_BUTTON_L2_0100
     * @tc.name    : testArkUIKeyCodeButtonL20100
     * @tc.desc    : testing ARKUI_KEYCODE_BUTTON_L2 interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeButtonL20100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeButtonL20100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2309);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventButtonL2();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeButtonL20100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_BUTTON_R2_0100
     * @tc.name    : testArkUIKeyCodeButtonR20100
     * @tc.desc    : testing ARKUI_KEYCODE_BUTTON_R2 interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeButtonR20100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeButtonR20100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2310);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventButtonR2();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeButtonR20100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_BUTTON_SELECT_0100
     * @tc.name    : testArkUIKeyCodeButtonSelect0100
     * @tc.desc    : testing ARKUI_KEYCODE_BUTTON_SELECT interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeButtonSelect0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeButtonSelect0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2311);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventButtonSelect();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeButtonSelect0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_BUTTON_START_0100
     * @tc.name    : testArkUIKeyCodeButtonStart0100
     * @tc.desc    : testing ARKUI_KEYCODE_BUTTON_START interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeButtonStart0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeButtonStart0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2312);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventButtonStart();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeButtonStart0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_BUTTON_MODE_0100
     * @tc.name    : testArkUIKeyCodeButtonMode0100
     * @tc.desc    : testing ARKUI_KEYCODE_BUTTON_MODE interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeButtonMode0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeButtonMode0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2313);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventButtonMode();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeButtonMode0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_BUTTON_THUMBL_0100
     * @tc.name    : testArkUIKeyCodeButtonThumbl0100
     * @tc.desc    : testing ARKUI_KEYCODE_BUTTON_THUMBL interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeButtonThumbl0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeButtonThumbl0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2314);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventButtonThumbl();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeButtonThumbl0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_BUTTON_THUMBR_0100
     * @tc.name    : testArkUIKeyCodeButtonThumbr0100
     * @tc.desc    : testing ARKUI_KEYCODE_BUTTON_THUMBR interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeButtonThumbr0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeButtonThumbr0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2315);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventButtonThumbr();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeButtonThumbr0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);;
        }
    })
  })
}