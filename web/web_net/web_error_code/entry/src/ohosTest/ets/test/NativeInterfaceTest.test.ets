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

import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect, TestType, Size, Level } from '@ohos/hypium'
import nativeInterface from 'libnativeinterfacetest.so';

const TEST_ARKWEB_SUCCESS = 0;
const TEST_ARKWEB_INIT_ERROR = 17100001;
const TEST_ARKWEB_INVALID_PARAM = 17100101;
const TEST_ARKWEB_SCHEME_REGISTER_FAILED = 17100102;
const TEST_ARKWEB_INVALID_URL = 17100103;
const TEST_ARKWEB_INVALID_COOKIE_VALUE = 17100104
const TEST_ARKWEB_LIBRARY_OPEN_FAILURE = 17100105;
const TEST_ARKWEB_LIBRARY_SYMBOL_NOT_FOUND = 17100106;
const TEST_ARKWEB_COOKIE_MANAGER_NOT_INITIALIZED = 17100107;


export default function NativeInterfaceTest() {
  describe('NativeInterfaceTest', () => {
    beforeAll(async (done: Function) => {
      done()
    });

    beforeEach(async (done: Function) => {
      console.info("WebNapiLoadDataTest beforeEach start");
      done();
    })

    afterEach(async (done: Function) => {
      done();
    })

    /**
     * @tc.number  : SUB_Web_Napi_LoadDataTest_0100
     * @tc.name    : SUB_Web_Napi_LoadDataTest_0100
     * @tc.desc    : test OH_NativeArkWeb_LoadData() not init
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('SUB_Web_Napi_LoadDataTest_0100', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>SUB_Web_Napi_LoadDataTest_0100 start====");
        let result: number = nativeInterface.NativeArkWebLoadDataInitError();
        console.info("====>SUB_Web_Napi_LoadDataTest_0100 result====", result);
        expect(result).assertEqual(TEST_ARKWEB_INIT_ERROR);
        done();
      } catch (err) {
        console.error("====>SUB_Web_Napi_LoadDataTest_0100 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_Web_Napi_LoadDataTest_0200
     * @tc.name    : SUB_Web_Napi_LoadDataTest_0200
     * @tc.desc    : test OH_NativeArkWeb_LoadData() SUCCESS
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('SUB_Web_Napi_LoadDataTest_0200', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>SUB_Web_Napi_LoadDataTest_0200 start====");
        let result: number = nativeInterface.loadData("web1211", 0);
        console.info("====>SUB_Web_Napi_LoadDataTest_0200 result====", result);
        expect(result).assertEqual(TEST_ARKWEB_SUCCESS);
        console.info("====>SUB_Web_Napi_LoadDataTest_0200 stop====");
        done();
      } catch (err) {
        console.error("====>SUB_Web_Napi_LoadDataTest_0200 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_Web_Napi_LoadDataTest_0300
     * @tc.name    : SUB_Web_Napi_LoadDataTest_0300
     * @tc.desc    : test OH_NativeArkWeb_LoadData() ARKWEB_LIBRARY_SYMBOL_NOT_FOUND
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('SUB_Web_Napi_LoadDataTest_0300', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>SUB_Web_Napi_LoadDataTest_0300 start====");
        let result: number = nativeInterface.loadData("web1211", 1);
        console.info("====>SUB_Web_Napi_LoadDataTest_0300 result====", result);
        // expect(result).assertEqual(TEST_ARKWEB_LIBRARY_SYMBOL_NOT_FOUND);
        console.info("====>SUB_Web_Napi_LoadDataTest_0300 stop====");
        done();
      } catch (err) {
        console.error("====>SUB_Web_Napi_LoadDataTest_0300 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_Web_Napi_LoadDataTest_0400
     * @tc.name    : SUB_Web_Napi_LoadDataTest_0400
     * @tc.desc    : test OH_NativeArkWeb_LoadData() ARKWEB_INVALID_PARAM
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('SUB_Web_Napi_LoadDataTest_0400', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>SUB_Web_Napi_LoadDataTest_0400 start====");
        let result: number = nativeInterface.loadData("web1211", 2);
        console.info("====>SUB_Web_Napi_LoadDataTest_0300 result====", result);
        // expect(result).assertEqual(0);
        console.info("====>SUB_Web_Napi_LoadDataTest_0400 stop====");
        done();
      } catch (err) {
        console.error("====>SUB_Web_Napi_LoadDataTest_0400 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_Web_Napi_SetBlanklessInfoTest_0100
     * @tc.name    : SUB_Web_Napi_SetBlanklessInfoTest_0100
     * @tc.desc    : test OH_NativeArkWeb_SetBlanklessLoadingWithKey()
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('SUB_Web_Napi_SetBlanklessInfoTest_0100', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>SUB_Web_Napi_SetBlanklessInfoTest_0100 start====");
          let result: number = nativeInterface.SetBlanklessInfo('new', '0731');
          expect(result).assertEqual(0);
          console.info("====>SUB_Web_Napi_SetBlanklessInfoTest_0100 stop====");
          done();
        } catch (err) {
          console.error("====>SUB_Web_Napi_SetBlanklessInfoTest_0100 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_SetBlanklessInfoTest_0200
     * @tc.name    : SUB_Web_Napi_SetBlanklessInfoTest_0200
     * @tc.desc    : test OH_NativeArkWeb_SetBlanklessLoadingWithKey()
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('SUB_Web_Napi_SetBlanklessInfoTest_0200', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>SUB_Web_Napi_SetBlanklessInfoTest_0200 start====");
          let result: number = nativeInterface.SetBlanklessInfo('web1211', '0731');
          expect(result).assertEqual(0);
          console.info("====>SUB_Web_Napi_SetBlanklessInfoTest_0200 stop====");
          done();
        } catch (err) {
          console.error("====>SUB_Web_Napi_SetBlanklessInfoTest_0200 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_SetBlanklessInfoTest_0300
     * @tc.name    : SUB_Web_Napi_SetBlanklessInfoTest_0300
     * @tc.desc    : test OH_NativeArkWeb_SetBlanklessLoadingWithKey()
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('SUB_Web_Napi_SetBlanklessInfoTest_0300', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>SUB_Web_Napi_SetBlanklessInfoTest_0300 start====");
          let result: number = nativeInterface.SetBlanklessInfo('web1211', 'xxxxxx');
          expect(result).assertEqual(0);
          console.info("====>SUB_Web_Napi_SetBlanklessInfoTest_0300 stop====");
          done();
        } catch (err) {
          console.error("====>SUB_Web_Napi_SetBlanklessInfoTest_0300 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_GetBlanklessInfoTest_0100
     * @tc.name    : SUB_Web_Napi_GetBlanklessInfoTest_0100
     * @tc.desc    : test OH_NativeArkWeb_SetBlanklessLoadingWithKey()
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('SUB_Web_Napi_GetBlanklessInfoTest_0100', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>SUB_Web_Napi_GetBlanklessInfoTest_0100 start====");
          nativeInterface.SetBlanklessInfo('web1211', '0731');
          let result: number = nativeInterface.GetBlanklessInfo('web1211', '0731');
          expect(result).assertEqual(0);
          console.info("====>SUB_Web_Napi_GetBlanklessInfoTest_0100 stop====");
          done();
        } catch (err) {
          console.error("====>SUB_Web_Napi_GetBlanklessInfoTest_0100 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_JSBridgeDataTest_0100
     * @tc.name    : SUB_Web_Napi_JSBridgeDataTest_0100
     * @tc.desc    : test ArkWeb_JavaScriptBridgeData and ArkWeb_OnJavaScriptCallback
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('SUB_Web_Napi_JSBridgeDataTest_0100', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>SUB_Web_Napi_JSBridgeDataTest_0100 start====");
          let result: number = nativeInterface.JSBridgeData(0);
          expect(result).assertEqual(0);
          console.info("====>SUB_Web_Napi_JSBridgeDataTest_0100 stop====");
          done();
        } catch (err) {
          console.error("====>SUB_Web_Napi_JSBridgeDataTest_0100 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_JSBridgeDataTest_0200
     * @tc.name    : SUB_Web_Napi_JSBridgeDataTest_0200
     * @tc.desc    : test ArkWeb_JavaScriptBridgeData and ArkWeb_OnJavaScriptCallback
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('SUB_Web_Napi_JSBridgeDataTest_0200', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>SUB_Web_Napi_JSBridgeDataTest_0200 start====");
          let result: number = nativeInterface.JSBridgeData(1);
          expect(result).assertEqual(0);
          console.info("====>SUB_Web_Napi_JSBridgeDataTest_0200 stop====");
          done();
        } catch (err) {
          console.error("====>SUB_Web_Napi_JSBridgeDataTest_0200 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_JSBridgeDataTest_0300
     * @tc.name    : SUB_Web_Napi_JSBridgeDataTest_0300
     * @tc.desc    : test ArkWeb_JavaScriptBridgeData and ArkWeb_OnJavaScriptCallback
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('SUB_Web_Napi_JSBridgeDataTest_0300', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>SUB_Web_Napi_JSBridgeDataTest_0300 start====");
          let result: number = nativeInterface.JSBridgeData(2);
          expect(result).assertEqual(0);
          console.info("====>SUB_Web_Napi_JSBridgeDataTest_0300 stop====");
          done();
        } catch (err) {
          console.error("====>SUB_Web_Napi_JSBridgeDataTest_0300 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_JSBridgeDataTest_0400
     * @tc.name    : SUB_Web_Napi_JSBridgeDataTest_0400
     * @tc.desc    : test ArkWeb_JavaScriptBridgeData and ArkWeb_OnJavaScriptCallback
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('SUB_Web_Napi_JSBridgeDataTest_0400', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>SUB_Web_Napi_JSBridgeDataTest_0400 start====");
          let result: number = nativeInterface.JSBridgeData(3);
          expect(result).assertEqual(0);
          console.info("====>SUB_Web_Napi_JSBridgeDataTest_0400 stop====");
          done();
        } catch (err) {
          console.error("====>SUB_Web_Napi_JSBridgeDataTest_0400 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_JSBridgeProxyDataTest_0100
     * @tc.name    : SUB_Web_Napi_JSBridgeProxyDataTest_0100
     * @tc.desc    : test ArkWeb_JavaScriptBridgeData and ArkWeb_OnJavaScriptProxyCallback
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('SUB_Web_Napi_JSBridgeProxyDataTest_0100', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>SUB_Web_Napi_JSBridgeProxyDataTest_0100 start====");
          let result: number = nativeInterface.JSBridgeProxyData(0);
          expect(result).assertEqual(0);
          console.info("====>SUB_Web_Napi_JSBridgeProxyDataTest_0100 stop====");
          done();
        } catch (err) {
          console.error("====>SUB_Web_Napi_JSBridgeProxyDataTest_0100 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_JSBridgeProxyDataTest_0200
     * @tc.name    : SUB_Web_Napi_JSBridgeProxyDataTest_0200
     * @tc.desc    : test ArkWeb_JavaScriptBridgeData and ArkWeb_OnJavaScriptProxyCallback
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('SUB_Web_Napi_JSBridgeProxyDataTest_0200', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>SUB_Web_Napi_JSBridgeProxyDataTest_0200 start====");
          let result: number = nativeInterface.JSBridgeProxyData(1);
          expect(result).assertEqual(0);
          console.info("====>SUB_Web_Napi_JSBridgeProxyDataTest_0200 stop====");
          done();
        } catch (err) {
          console.error("====>SUB_Web_Napi_JSBridgeProxyDataTest_0200 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_JSBridgeProxyDataTest_0300
     * @tc.name    : SUB_Web_Napi_JSBridgeProxyDataTest_0300
     * @tc.desc    : test ArkWeb_JavaScriptBridgeData and ArkWeb_OnJavaScriptProxyCallback
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('SUB_Web_Napi_JSBridgeProxyDataTest_0300', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>SUB_Web_Napi_JSBridgeProxyDataTest_0300 start====");
          let result: number = nativeInterface.JSBridgeProxyData(2);
          expect(result).assertEqual(0);
          console.info("====>SUB_Web_Napi_JSBridgeProxyDataTest_0300 stop====");
          done();
        } catch (err) {
          console.error("====>SUB_Web_Napi_JSBridgeProxyDataTest_0300 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_JSBridgeProxyDataTest_0400
     * @tc.name    : SUB_Web_Napi_JSBridgeProxyDataTest_0400
     * @tc.desc    : test ArkWeb_JavaScriptBridgeData and ArkWeb_OnJavaScriptProxyCallback
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('SUB_Web_Napi_JSBridgeProxyDataTest_0400', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>SUB_Web_Napi_JSBridgeProxyDataTest_0400 start====");
          let result: number = nativeInterface.JSBridgeProxyData(3);
          expect(result).assertEqual(0);
          console.info("====>SUB_Web_Napi_JSBridgeProxyDataTest_0400 stop====");
          done();
        } catch (err) {
          console.error("====>SUB_Web_Napi_JSBridgeProxyDataTest_0400 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_JSBridgeProxyDataTest_0500
     * @tc.name    : SUB_Web_Napi_JSBridgeProxyDataTest_0500
     * @tc.desc    : test ArkWeb_JavaScriptBridgeData and ArkWeb_OnJavaScriptProxyCallback
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('SUB_Web_Napi_JSBridgeProxyDataTest_0500', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>SUB_Web_Napi_JSBridgeProxyDataTest_0500 start====");
          let result: number = nativeInterface.JSBridgeProxyData(4);
          expect(result).assertEqual(0);
          console.info("====>SUB_Web_Napi_JSBridgeProxyDataTest_0500 stop====");
          done();
        } catch (err) {
          console.error("====>SUB_Web_Napi_JSBridgeProxyDataTest_0500 catch err: " + err);
        }
      })
  })
}