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

import webCode from 'libweberrorcodetest.so'
import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect, TestType, Size, Level } from '@ohos/hypium'

const TEST_ARKWEB_SUCCESS = 0;
const TEST_ARKWEB_INIT_ERROR = 17100001;
const TEST_ARKWEB_ERROR_UNKNOWN = 17100100;
const TEST_ARKWEB_INVALID_PARAM = 17100101;
const TEST_ARKWEB_SCHEME_REGISTER_FAILED = 17100102;
const TEST_ARKWEB_INVALID_URL = 17100103;
const TEST_ARKWEB_INVALID_COOKIE_VALUE = 17100104
const TEST_ARKWEB_LIBRARY_OPEN_FAILURE = 17100105;
const TEST_ARKWEB_LIBRARY_SYMBOL_NOT_FOUND = 17100106;
const TEST_ARKWEB_COOKIE_MANAGER_NOT_INITIALIZED = 17100107;

const TEST_ARKWEB_BLANKLESS_SUCCESS = 0;
const TEST_ARKWEB_BLANKLESS_ERR_UNKNOWN = -1;
const TEST_ARKWEB_BLANKLESS_ERR_INVALID_ARGS = -2;
const TEST_ARKWEB_BLANKLESS_ERR_CONTROLLER_NOT_INITED = -3;
const TEST_ARKWEB_BLANKLESS_ERR_KEY_NOT_MATCH = -4;
const TEST_ARKWEB_BLANKLESS_ERR_SIGNIFICANT_CHANGE = -5;
const TEST_ARKWEB_BLANKLESS_ERR_DEVICE_NOT_SUPPORT = 801;


export default function ActsWebErrorCodeTest() {
  describe('ActsWebErrorCodeTest', () => {
    /**
     * @tc.number  : SUB_OH_Web_Error_Code_0100
     * @tc.name    : SUB_OH_Web_Error_Code_0100
     * @tc.desc    : test OH_Web_Error_Code Test Normal Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_Web_Error_Code_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_Web_Error_Code_0100 start====");
        let result: number = webCode.webErrCodeSuccess();
        console.info("====>SUB_OH_Web_Error_Code_0100 result====", result);
        expect(result).assertEqual(TEST_ARKWEB_SUCCESS);
        done();
      } catch (err) {
        console.error("====>SUB_OH_Web_Error_Code_0100 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_Web_Error_Code_0200
     * @tc.name    : SUB_OH_Web_Error_Code_0200
     * @tc.desc    : test OH_Web_Error_Code Test Normal Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_Web_Error_Code_0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_Web_Error_Code_0200 start====");
        let result: number = webCode.webInitError();
        console.info("====>SUB_OH_Web_Error_Code_0200 result====", result);
        expect(result).assertEqual(TEST_ARKWEB_INIT_ERROR);
        done();
      } catch (err) {
        console.error("====>SUB_OH_Web_Error_Code_0200 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_Web_Error_Code_0300
     * @tc.name    : SUB_OH_Web_Error_Code_0300
     * @tc.desc    : test OH_Web_Error_Code Test UNKNOWN
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_Web_Error_Code_0300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_Web_Error_Code_0300 start====");
        let result: number = webCode.webErrorUnknow();
        console.info("====>SUB_OH_Web_Error_Code_0300 result====", result);
        expect(result).assertEqual(TEST_ARKWEB_ERROR_UNKNOWN);
        done();
      } catch (err) {
        console.error("====>SUB_OH_Web_Error_Code_0300 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_Web_Error_Code_0400
     * @tc.name    : SUB_OH_Web_Error_Code_0400
     * @tc.desc    : test OH_Web_Error_Code Test ARKWEB_INVALID_PARAM
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_Web_Error_Code_0400', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_Web_Error_Code_0400 start====");
        let result: number = webCode.webErrorInvalidParam();
        console.info("====>SUB_OH_Web_Error_Code_0400 result====", result);
        expect(result).assertEqual(TEST_ARKWEB_INVALID_PARAM);
        done();
      } catch (err) {
        console.error("====>SUB_OH_Web_Error_Code_0400 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_Web_Error_Code_0500
     * @tc.name    : SUB_OH_Web_Error_Code_0500
     * @tc.desc    : test OH_Web_Error_Code Test ARKWEB_SCHEME_REGISTER_FAILED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_Web_Error_Code_0500', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_Web_Error_Code_0500 start====");
        let result: number = webCode.webErrorSchemeRegitsterFailed();
        console.info("====>SUB_OH_Web_Error_Code_0500 result====", result);
        expect(result).assertEqual(TEST_ARKWEB_SCHEME_REGISTER_FAILED);
        done();
      } catch (err) {
        console.error("====>SUB_OH_Web_Error_Code_0500 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_Web_Error_Code_0600
     * @tc.name    : SUB_OH_Web_Error_Code_0600
     * @tc.desc    : test OH_Web_Error_Code Test ARKWEB_INVALID_URL
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_Web_Error_Code_0600', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_Web_Error_Code_0600 start====");
        let result: number = webCode.webErrorInvaledURL();
        console.info("====>SUB_OH_Web_Error_Code_0600 result====", result);
        expect(result).assertEqual(TEST_ARKWEB_INVALID_URL);
        done();
      } catch (err) {
        console.error("====>SUB_OH_Web_Error_Code_0600 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_Web_Error_Code_0700
     * @tc.name    : SUB_OH_Web_Error_Code_0700
     * @tc.desc    : test OH_Web_Error_Code Test ARKWEB_INVALID_COOKIE_VALUE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_Web_Error_Code_0700', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_Web_Error_Code_0700 start====");
        let result: number = webCode.webErrorInvaledCookieValue();
        console.info("====>SUB_OH_Web_Error_Code_0700 result====", result);
        expect(result).assertEqual(TEST_ARKWEB_INVALID_COOKIE_VALUE);
        done();
      } catch (err) {
        console.error("====>SUB_OH_Web_Error_Code_0700 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_Web_Error_Code_0800
     * @tc.name    : SUB_OH_Web_Error_Code_0800
     * @tc.desc    : test OH_Web_Error_Code Test ARKWEB_LIBRARY_OPEN_FAILURE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_Web_Error_Code_0800', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_Web_Error_Code_0800 start====");
        let result: number = webCode.webErrorLibraryOpenFailure();
        console.info("====>SUB_OH_Web_Error_Code_0800 result====", result);
        expect(result).assertEqual(TEST_ARKWEB_LIBRARY_OPEN_FAILURE);
        done();
      } catch (err) {
        console.error("====>SUB_OH_Web_Error_Code_0800 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_Web_Error_Code_0900
     * @tc.name    : SUB_OH_Web_Error_Code_0900
     * @tc.desc    : test OH_Web_Error_Code Test ARKWEB_LIBRARY_SYMBOL_NOT_FOUND
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_Web_Error_Code_0900', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_Web_Error_Code_0900 start====");
        let result: number = webCode.webErrorLibraySymbolNotFound();
        console.info("====>SUB_OH_Web_Error_Code_0900 result====", result);
        expect(result).assertEqual(TEST_ARKWEB_LIBRARY_SYMBOL_NOT_FOUND);
        done();
      } catch (err) {
        console.error("====>SUB_OH_Web_Error_Code_0900 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_Web_Error_Code_1000
     * @tc.name    : SUB_OH_Web_Error_Code_1000
     * @tc.desc    : test OH_Web_Error_Code Test ARKWEB_COOKIE_MANAGER_NOT_INITIALIZED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_Web_Error_Code_1000', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_Web_Error_Code_1000 start====");
        let result: number = webCode.webErrorCookeManagerNotInitialized();
        console.info("====>SUB_OH_Web_Error_Code_1000 result====", result);
        expect(result).assertEqual(TEST_ARKWEB_COOKIE_MANAGER_NOT_INITIALIZED);
        done();
      } catch (err) {
        console.error("====>SUB_OH_Web_Error_Code_1000 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_BLANKLESS_Code_0100
     * @tc.name    : SUB_OH_BLANKLESS_Code_0100
     * @tc.desc    : test OH_Web_Error_Code Test ARKWEB_BLANKLESS_SUCCESS
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_BLANKLESS_Code_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_BLANKLESS_Code_0100 start====");
        let result: number = webCode.webBlanklessSuccess();
        console.info("====>SUB_OH_BLANKLESS_Code_0100 result====", result);
        expect(result).assertEqual(TEST_ARKWEB_BLANKLESS_SUCCESS);
        done();
      } catch (err) {
        console.error("====>SUB_OH_BLANKLESS_Code_0100 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_BLANKLESS_Code_0200
     * @tc.name    : SUB_OH_BLANKLESS_Code_0200
     * @tc.desc    : test OH_Web_Error_Code Test ARKWEB_BLANKLESS_SUCCESS
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_BLANKLESS_Code_0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_BLANKLESS_Code_0200 start====");
        let result: number = webCode.webBlanklessErrUnknown();
        console.info("====>SUB_OH_BLANKLESS_Code_0200 result====", result);
        expect(result).assertEqual(TEST_ARKWEB_BLANKLESS_ERR_UNKNOWN);
        done();
      } catch (err) {
        console.error("====>SUB_OH_BLANKLESS_Code_0200 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_BLANKLESS_Code_0300
     * @tc.name    : SUB_OH_BLANKLESS_Code_0300
     * @tc.desc    : test OH_Web_Error_Code Test ARKWEB_BLANKLESS_ERR_INVALID_ARGS
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_BLANKLESS_Code_0300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_BLANKLESS_Code_0300 start====");
        let result: number = webCode.webBlanklessErrInvalidArgs();
        console.info("====>SUB_OH_BLANKLESS_Code_0300 result====", result);
        expect(result).assertEqual(TEST_ARKWEB_BLANKLESS_ERR_INVALID_ARGS);
        done();
      } catch (err) {
        console.error("====>SUB_OH_BLANKLESS_Code_0300 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_BLANKLESS_Code_0400
     * @tc.name    : SUB_OH_BLANKLESS_Code_0400
     * @tc.desc    : test OH_Web_Error_Code Test ARKWEB_BLANKLESS_ERR_INVALID_ARGS
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_BLANKLESS_Code_0400', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_BLANKLESS_Code_0400 start====");
        let result: number = webCode.webBlanklessErrControllerNotInited();
        console.info("====>SUB_OH_BLANKLESS_Code_0400 result====", result);
        expect(result).assertEqual(TEST_ARKWEB_BLANKLESS_ERR_CONTROLLER_NOT_INITED);
        done();
      } catch (err) {
        console.error("====>SUB_OH_BLANKLESS_Code_0400 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_BLANKLESS_Code_0500
     * @tc.name    : SUB_OH_BLANKLESS_Code_0500
     * @tc.desc    : test OH_Web_Error_Code Test ARKWEB_BLANKLESS_ERR_INVALID_ARGS
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_BLANKLESS_Code_0500', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_BLANKLESS_Code_0500 start====");
        let result: number = webCode.webBlanklessErrKeyNotMatch();
        console.info("====>SUB_OH_BLANKLESS_Code_0500 result====", result);
        expect(result).assertEqual(TEST_ARKWEB_BLANKLESS_ERR_KEY_NOT_MATCH);
        done();
      } catch (err) {
        console.error("====>SUB_OH_BLANKLESS_Code_0500 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_BLANKLESS_Code_0600
     * @tc.name    : SUB_OH_BLANKLESS_Code_0600
     * @tc.desc    : test OH_Web_Error_Code Test ARKWEB_BLANKLESS_ERR_INVALID_ARGS
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_BLANKLESS_Code_0600', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_BLANKLESS_Code_0600 start====");
        let result: number = webCode.webBlanklessErrSignificantChange();
        console.info("====>SUB_OH_BLANKLESS_Code_0600 result====", result);
        expect(result).assertEqual(TEST_ARKWEB_BLANKLESS_ERR_SIGNIFICANT_CHANGE);
        done();
      } catch (err) {
        console.error("====>SUB_OH_BLANKLESS_Code_0600 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_BLANKLESS_Code_0700
     * @tc.name    : SUB_OH_BLANKLESS_Code_0700
     * @tc.desc    : test OH_Web_Error_Code Test ARKWEB_BLANKLESS_ERR_DEVICE_NOT_SUPPORT
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_BLANKLESS_Code_0700', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_BLANKLESS_Code_0700 start====");
        let result: number = webCode.webBlanklessDeviceNotSupport();
        console.info("====>SUB_OH_BLANKLESS_Code_0700 result====", result);
        expect(result).assertEqual(TEST_ARKWEB_BLANKLESS_ERR_DEVICE_NOT_SUPPORT);
        done();
      } catch (err) {
        console.error("====>SUB_OH_BLANKLESS_Code_0700 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_WebAnyNativeAPI_100
     * @tc.name    : SUB_OH_WebAnyNativeAPI_100
     * @tc.desc    : test ArkWeb_AnyNativeAPI Test size
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_WebAnyNativeAPI_100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_WebAnyNativeAPI_100 start====");
        let result: number = webCode.webAnyNativeAPI();
        console.info("====>SUB_OH_WebAnyNativeAPI_100 result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>SUB_OH_WebAnyNativeAPI_100 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_NATIVE_APIVariantKind_100
     * @tc.name    : SUB_OH_NATIVE_APIVariantKind_100
     * @tc.desc    : test ARKWEB_NATIVE_COMPONENT
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_NATIVE_APIVariantKind_100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_NATIVE_APIVariantKind_100 start====");
        let result: number = webCode.webNativeComponent();
        console.info("====>SUB_OH_NATIVE_APIVariantKind_100 result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>SUB_OH_NATIVE_APIVariantKind_100 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_NATIVE_APIVariantKind_200
     * @tc.name    : SUB_OH_NATIVE_APIVariantKind_200
     * @tc.desc    : test ARKWEB_NATIVE_CONTROLLER
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_NATIVE_APIVariantKind_200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_NATIVE_APIVariantKind_200 start====");
        let result: number = webCode.webNativeController();
        console.info("====>SUB_OH_NATIVE_APIVariantKind_200 result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>SUB_OH_NATIVE_APIVariantKind_200 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_NATIVE_APIVariantKind_300
     * @tc.name    : SUB_OH_NATIVE_APIVariantKind_300
     * @tc.desc    : test ARKWEB_NATIVE_WEB_MESSAGE_PORT
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_NATIVE_APIVariantKind_300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_NATIVE_APIVariantKind_300 start====");
        let result: number = webCode.webNativeWebMessagePort();
        console.info("====>SUB_OH_NATIVE_APIVariantKind_300 result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>SUB_OH_NATIVE_APIVariantKind_300 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_NATIVE_APIVariantKind_400
     * @tc.name    : SUB_OH_NATIVE_APIVariantKind_400
     * @tc.desc    : test ARKWEB_NATIVE_WEB_MESSAGE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_NATIVE_APIVariantKind_400', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_NATIVE_APIVariantKind_400 start====");
        let result: number = webCode.webNativeWebMessage();
        console.info("====>SUB_OH_NATIVE_APIVariantKind_400 result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>SUB_OH_NATIVE_APIVariantKind_400 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_NATIVE_APIVariantKind_500
     * @tc.name    : SUB_OH_NATIVE_APIVariantKind_500
     * @tc.desc    : test ARKWEB_NATIVE_WEB_MESSAGE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_NATIVE_APIVariantKind_500', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_NATIVE_APIVariantKind_500 start====");
        let result: number = webCode.webNativeCookieManger();
        console.info("====>SUB_OH_NATIVE_APIVariantKind_500 result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>SUB_OH_NATIVE_APIVariantKind_500 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_NATIVE_APIVariantKind_600
     * @tc.name    : SUB_OH_NATIVE_APIVariantKind_600
     * @tc.desc    : test ARKWEB_NATIVE_WEB_MESSAGE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_NATIVE_APIVariantKind_600', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_NATIVE_APIVariantKind_600 start====");
        let result: number = webCode.webNativeJavaScriptValue();
        console.info("====>SUB_OH_NATIVE_APIVariantKind_600 result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>SUB_OH_NATIVE_APIVariantKind_600 catch err: " + err);
      }
    })
  })
}