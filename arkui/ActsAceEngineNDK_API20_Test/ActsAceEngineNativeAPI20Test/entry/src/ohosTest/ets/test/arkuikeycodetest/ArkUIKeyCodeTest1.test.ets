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
import { Driver, ON } from '@kit.TestKit';
import Utils from '../common/Utils';

const KEY_CODE_EVENT_OK = 1;
const KEY_CODE_EVENT_ERROR = 0;


export default function ArkUIKeyCodeNdkTest1() {
  describe('ArkUIKeyCodeNdkTest1', () => {
    beforeAll(async (done: Function) => {
      await Utils.pushPage(`ArkUIKeyCodeTest/ArkUIKeyCodeTest1`, done);
    });

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_VOLUME_UP_0100
     * @tc.name    : testArkUIKeycodeVolumeUp0100
     * @tc.desc    : testing ARKUI_KEYCODE_VOLUME_UP_0100 interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeycodeVolumeUp0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeycodeVolumeUp0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(16);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventVolumeUp();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeycodeVolumeUp0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_VOLUME_DOWN_0100
     * @tc.name    : testArkUIKeyCodeVolumeDown0100
     * @tc.desc    : testing ARKUI_KEYCODE_VOLUME_DOWN interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeVolumeDown0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeVolumeDown0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          let com = await Utils.driver.findComponent(ON.id("ArkUIKeyCodeTest1"));
          await com.click();
          await com.click();
          await driver.triggerKey(17);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventVolumeDown();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeVolumeDown0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })

    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_CAMERA_0100
     * @tc.name    : testArkUIKeyCodeCamera0100
     * @tc.desc    : testing ARKUI_KEYCODE_CAMERA interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeCamera0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeCamera0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(19);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventCamera();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeCamera0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_VOLUME_MUTE_0100
     * @tc.name    : testArkUIKeyCodeVolumeMute0100
     * @tc.desc    : testing ARKUI_KEYCODE_VOLUME_MUTE interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeVolumeMute0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeVolumeMute0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(22);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventVolumeMute();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeVolumeMute0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_MUTE_0100
     * @tc.name    : testArkUIKeyCodeMute0100
     * @tc.desc    : testing ARKUI_KEYCODE_MUTE_0100 interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeMute0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeMute0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(23);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventMute();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeMute0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_BRIGHTNESS_UP_0100
     * @tc.name    : testArkUIKeyCodeBrightnessUp0100
     * @tc.desc    : testing ARKUI_KEYCODE_BRIGHTNESS_UP interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeBrightnessUp0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeBrightnessUp0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(40);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventBrightnessUp();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeBrightnessUp0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_BRIGHTNESS_DOWN_0100
     * @tc.name    : testArkUIKeyCodeBrightnessDown0100
     * @tc.desc    : testing ARKUI_KEYCODE_BRIGHTNESS_DOWN interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeBrightnessDown0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeBrightnessDown0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(41);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventBrightnessDown();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeBrightnessDown0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_0_0100
     * @tc.name    : testArkUIKeyCode0_0100
     * @tc.desc    : testing ARKUI_KEYCODE_0 interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCode0_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCode0_0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2000);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEvent0();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCode0_0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_1_0100
     * @tc.name    : testArkUIKeyCode1_0100
     * @tc.desc    : testing ARKUI_KEYCODE_1 interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCode1_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCode1_0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2001);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEvent1();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCode1_0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_2_0100
     * @tc.name    : testArkUIKeyCode2_0100
     * @tc.desc    : testing ARKUI_KEYCODE_2 interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCode2_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCode2_0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2002);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEvent2();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCode2_0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_3_0100
     * @tc.name    : testArkUIKeyCode3_0100
     * @tc.desc    : testing ARKUI_KEYCODE_3 interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCode3_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCode3_0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2003);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEvent3();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCode3_0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_4_0100
     * @tc.name    : testArkUIKeyCode4_0100
     * @tc.desc    : testing ARKUI_KEYCODE_4 interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCode4_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCode4_0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2004);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEvent4();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCode4_0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_5_0100
     * @tc.name    : testArkUIKeyCode5_0100
     * @tc.desc    : testing ARKUI_KEYCODE_5 interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCode5_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCode5_0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2005);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEvent5();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCode5_0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_6_0100
     * @tc.name    : testArkUIKeyCode6_0100
     * @tc.desc    : testing ARKUI_KEYCODE_6 interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCode6_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCode6_0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2006);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEvent6();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCode6_0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_7_0100
     * @tc.name    : testArkUIKeyCode7_0100
     * @tc.desc    : testing ARKUI_KEYCODE_7 interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCode7_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCode7_0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2007);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEvent7();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCode7_0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_8_0100
     * @tc.name    : testArkUIKeyCode8_0100
     * @tc.desc    : testing ARKUI_KEYCODE_8 interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCode8_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCode8_0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2008);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEvent8();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCode8_0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_9_0100
     * @tc.name    : testArkUIKeyCode9_0100
     * @tc.desc    : testing ARKUI_KEYCODE_9 interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCode9_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCode9_0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2009);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEvent9();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCode9_0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_STAR_0100
     * @tc.name    : testArkUIKeyCodeStar0100
     * @tc.desc    : testing ARKUI_KEYCODE_STAR interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeStar0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeStar0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2010);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventStar();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeStar0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_POUND_0100
     * @tc.name    : testArkUIKeyCodePound0100
     * @tc.desc    : testing ARKUI_KEYCODE_POUND interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodePound0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodePound0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2011);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventPound();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodePound0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_DPAD_UP_0100
     * @tc.name    : testArkUIKeyCodeDpadUp0100
     * @tc.desc    : testing ARKUI_KEYCODE_DPAD_UP interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeDpadUp0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeDpadUp0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2012);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventDpadUp();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeDpadUp0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_DPAD_DOWN_0100
     * @tc.name    : testArkUIKeyCodeDpadDown0100
     * @tc.desc    : testing ARKUI_KEYCODE_DPAD_DOWN interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeDpadDown0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeDpadDown0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2013);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventDpadDown();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeDpadDown0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_DPAD_LEFT_0100
     * @tc.name    : testArkUIKeyCodeDpadLeft0100
     * @tc.desc    : testing ARKUI_KEYCODE_DPAD_LEFT interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeDpadLeft0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeDpadLeft0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2014);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventDpadLeft();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeDpadLeft0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_DPAD_RIGHT_0100
     * @tc.name    : testArkUIKeyCodeDpadRight0100
     * @tc.desc    : testing ARKUI_KEYCODE_DPAD_RIGHT interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeDpadRight0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeDpadRight0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2015);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventDpadRight();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeDpadRight0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_DPAD_CENTER_0100
     * @tc.name    : testArkUIKeyCodeDpadCenter0100
     * @tc.desc    : testing ARKUI_KEYCODE_DPAD_CENTER interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeyCodeDpadCenter0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeyCodeDpadCenter0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2016);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventDpadCenter();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeyCodeDpadCenter0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_A_0100
     * @tc.name    : testArkUIKeycodeA0100
     * @tc.desc    : testing ARKUI_KEYCODE_A interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeycodeA0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeycodeA0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2017);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventA();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeycodeA0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_B_0100
     * @tc.name    : testArkUIKeycodeB0100
     * @tc.desc    : testing ARKUI_KEYCODE_B interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeycodeB0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeycodeB0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2018);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventB();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeycodeB0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_C_0100
     * @tc.name    : testArkUIKeycodeC0100
     * @tc.desc    : testing ARKUI_KEYCODE_C interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeycodeC0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeycodeC0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2019);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventC();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeycodeC0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_D_0100
     * @tc.name    : testArkUIKeycodeD0100
     * @tc.desc    : testing ARKUI_KEYCODE_D interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeycodeD0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeycodeD0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2020);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventD();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeycodeD0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_E_0100
     * @tc.name    : testArkUIKeycodeE0100
     * @tc.desc    : testing ARKUI_KEYCODE_E interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeycodeE0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeycodeE0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2021);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventE();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeycodeE0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_F_0100
     * @tc.name    : testArkUIKeycodeF0100
     * @tc.desc    : testing ARKUI_KEYCODE_F interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeycodeF0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeycodeF0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2022);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventF();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeycodeF0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_G_0100
     * @tc.name    : testArkUIKeycodeG0100
     * @tc.desc    : testing ARKUI_KEYCODE_G interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeycodeG0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeycodeG0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2023);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventG();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeycodeG0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_H_0100
     * @tc.name    : testArkUIKeycodeH0100
     * @tc.desc    : testing ARKUI_KEYCODE_H interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeycodeH0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeycodeH0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2024);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventH();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeycodeH0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_I_0100
     * @tc.name    : testArkUIKeycodeI0100
     * @tc.desc    : testing ARKUI_KEYCODE_I interface
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkUIKeycodeI0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkUIKeycodeI0100 start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2025);
          Utils.sleep(1000);
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventI();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done();
        } catch (err) {
          console.error("====>testArkUIKeycodeI0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
  })
}
