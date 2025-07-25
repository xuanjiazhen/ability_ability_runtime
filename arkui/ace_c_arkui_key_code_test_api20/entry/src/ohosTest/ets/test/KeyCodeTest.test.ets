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
import keyCodeNdk from 'libkeycodendk.so'
import { Driver } from '@kit.TestKit';
import Utils from './common/Utils';

const KEY_CODE_EVENT_OK = 1;
const KEY_CODE_EVENT_ERROR = 0;

export default function KeyCodeNdkTest() {
  describe('KeyCodeNdkTest', () => {
    beforeAll(async (done: Function) => {
      await Utils.pushPage(`KeyCodeTest/KeyCodeTest`, done);
    });

    /*
     * @tc.number     : KEY_APOSTROPHE_TEST
     * @tc.name       : KEY_APOSTROPHE_TEST
     * @tc.desc       : testing KEY_APOSTROPHE interface
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 1
     */
    it('KEY_APOSTROPHE_TEST', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>KEY_APOSTROPHE_TEST start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2063);
          Utils.sleep(1000);
          let a: number = keyCodeNdk.GetKeyCodeEventKeyApostrophe();
          expect(KEY_CODE_EVENT_OK).assertEqual(a);
          done()
        } catch (err) {
          console.error("====>KEY_APOSTROPHE_TEST catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
          done()
        }
      })

    /*
     * @tc.number     : KEY_GRAVE_TEST
     * @tc.name       : KEY_GRAVE_TEST
     * @tc.desc       : testing KEY_GRAVE interface
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 1
     */
    it('KEY_GRAVE_TEST', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>KEY_GRAVE_TEST start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2056);
          Utils.sleep(1000);
          let a: number = keyCodeNdk.GetKeyCodeEventKeyGrave();
          expect(KEY_CODE_EVENT_OK).assertEqual(a);
          done()
        } catch (err) {
          console.error("====>KEY_GRAVE_TEST catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
          done()
        }
      })

    /*
     * @tc.number     : KEY_MINUS_TEST
     * @tc.name       : KEY_MINUS_TEST
     * @tc.desc       : testing KEY_MINUS interface
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 1
     */
    it('KEY_MINUS_TEST', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>KEY_MINUS_TEST start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2057);
          Utils.sleep(1000);
          let a: number = keyCodeNdk.GetKeyCodeEventKeyMinus();
          expect(KEY_CODE_EVENT_OK).assertEqual(a);
          done()
        } catch (err) {
          console.error("====>KEY_MINUS_TEST catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
          done()
        }
      })

    /*
     * @tc.number     : KEY_EQUALS_TEST
     * @tc.name       : KEY_EQUALS_TEST
     * @tc.desc       : testing KEY_EQUALS interface
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 1
     */
    it('KEY_EQUALS_TEST', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>KEY_MINUS_TEST start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2058);
          Utils.sleep(1000);
          let a: number = keyCodeNdk.GetKeyCodeEventKeyEquals();
          expect(KEY_CODE_EVENT_OK).assertEqual(a);
          done()
        } catch (err) {
          console.error("====>KEY_MINUS_TEST catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
          done()
        }
      })

    /*
     * @tc.number     : KEY_LEFT_BRACKET_TEST
     * @tc.name       : KEY_LEFT_BRACKET_TEST
     * @tc.desc       : testing KEY_LEFT_BRACKET interface
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 1
     */
    it('KEY_LEFT_BRACKET_TEST', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>KEY_LEFT_BRACKET_TEST start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2059);
          Utils.sleep(1000);
          let a: number = keyCodeNdk.GetKeyCodeEventKeyLeftbracket();
          expect(KEY_CODE_EVENT_OK).assertEqual(a);
          done()
        } catch (err) {
          console.error("====>KEY_LEFT_BRACKET_TEST catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
          done()
        }
      })

    /*
     * @tc.number     : KEY_RIGHT_BRACKET_TEST
     * @tc.name       : KEY_RIGHT_BRACKET_TEST
     * @tc.desc       : testing KEY_RIGHT_BRACKET interface
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 1
     */
    it('KEY_RIGHT_BRACKET_TEST', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>KEY_RIGHT_BRACKET_TEST start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2060);
          Utils.sleep(1000);
          let a: number = keyCodeNdk.GetKeyCodeEventKeyRightbracket();
          expect(KEY_CODE_EVENT_OK).assertEqual(a);
          done()
        } catch (err) {
          console.error("====>KEY_RIGHT_BRACKET_TEST catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
          done()
        }
      })

    /*
     * @tc.number     : KEY_BACKSLASH_TEST
     * @tc.name       : KEY_BACKSLASH_TEST
     * @tc.desc       : testing KEY_BACKSLASH interface
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 1
     */
    it('KEY_BACKSLASH_TEST', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>KEY_BACKSLASH_TEST start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2061);
          Utils.sleep(1000);
          let a: number = keyCodeNdk.GetKeyCodeEventKeyBackslash();
          expect(KEY_CODE_EVENT_OK).assertEqual(a);
          done()
        } catch (err) {
          console.error("====>KEY_BACKSLASH_TEST catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
          done()
        }
      })

    /*
     * @tc.number     : KEY_SEMICOLON_TEST
     * @tc.name       : KEY_SEMICOLON_TEST
     * @tc.desc       : testing KEY_SEMICOLON interface
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 1
     */
    it('KEY_SEMICOLON_TEST', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>KEY_SEMICOLON_TEST start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2062);
          Utils.sleep(1000);
          let a: number = keyCodeNdk.GetKeyCodeEventKeySemicolon();
          expect(KEY_CODE_EVENT_OK).assertEqual(a);
          done()
        } catch (err) {
          console.error("====>KEY_SEMICOLON_TEST catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
          done()
        }
      })

    /*
     * @tc.number     : KEY_SLASH_TEST
     * @tc.name       : KEY_SLASH_TEST
     * @tc.desc       : testing KEY_SLASH interface
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 1
     */
    it('KEY_SLASH_TEST', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>KEY_SLASH_TEST start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2064);
          Utils.sleep(1000);
          let a: number = keyCodeNdk.GetKeyCodeEventKeySlash();
          expect(KEY_CODE_EVENT_OK).assertEqual(a);
          done()
        } catch (err) {
          console.error("====>KEY_SLASH_TEST catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
          done()
        }
      })

    /*
     * @tc.number     : KEY_AT_TEST
     * @tc.name       : KEY_AT_TEST
     * @tc.desc       : testing KEY_AT interface
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 1
     */
    it('KEY_AT_TEST', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>KEY_AT_TEST start====");
          let driver = await Driver.create();
          Utils.sleep(1000);
          await driver.triggerKey(2065);
          Utils.sleep(1000);
          let a: number = keyCodeNdk.GetKeyCodeEventKeyAt();
          expect(KEY_CODE_EVENT_OK).assertEqual(a);
          done()
        } catch (err) {
          console.error("====>KEY_AT_TEST catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
          done()
        }
      })
  })
}
