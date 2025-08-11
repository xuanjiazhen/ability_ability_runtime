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
     * @tc.number  : SUB_WEB_NET_NAPI_LoadData_0100
     * @tc.name    : testWebNapiLoadDataNotInit
     * @tc.desc    : test OH_NativeArkWeb_LoadData() not init
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiLoadDataNotInit', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>testWebNapiLoadDataNotInit start====");
        let result: number = nativeInterface.NativeArkWebLoadDataInitError();
        console.info("====>testWebNapiLoadDataNotInit result====", result);
        expect(result).assertEqual(TEST_ARKWEB_INIT_ERROR);
        done();
      } catch (err) {
        console.error("====>testWebNapiLoadDataNotInit catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_NAPI_LoadData_0200
     * @tc.name    : testWebNapiLoadDataSuccess
     * @tc.desc    : test OH_NativeArkWeb_LoadData() SUCCESS
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiLoadDataSuccess', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>testWebNapiLoadDataSuccess start====");
        let result: number = nativeInterface.loadData("web1211", 0);
        console.info("====>testWebNapiLoadDataSuccess result====", result);
        expect(result).assertEqual(TEST_ARKWEB_SUCCESS);
        console.info("====>testWebNapiLoadDataSuccess stop====");
        done();
      } catch (err) {
        console.error("====>testWebNapiLoadDataSuccess catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_NAPI_LoadData_0300
     * @tc.name    : testWebNapiLoadDataSymbolNotFound
     * @tc.desc    : test OH_NativeArkWeb_LoadData() ARKWEB_LIBRARY_SYMBOL_NOT_FOUND
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiLoadDataSymbolNotFound', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>testWebNapiLoadDataSymbolNotFound start====");
        let result: number = nativeInterface.loadData("web1211", 1);
        console.info("====>testWebNapiLoadDataSymbolNotFound result====", result);
        // expect(result).assertEqual(TEST_ARKWEB_LIBRARY_SYMBOL_NOT_FOUND);
        console.info("====>testWebNapiLoadDataSymbolNotFound stop====");
        done();
      } catch (err) {
        console.error("====>testWebNapiLoadDataSymbolNotFound catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_NAPI_LoadData_0400
     * @tc.name    : testWebNapiLoadDataInvalidParam
     * @tc.desc    : test OH_NativeArkWeb_LoadData() ARKWEB_INVALID_PARAM
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiLoadDataInvalidParam', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>testWebNapiLoadDataInvalidParam start====");
        let result: number = nativeInterface.loadData("web1211", 2);
        console.info("====>testWebNapiLoadDataInvalidParam result====", result);
        // expect(result).assertEqual(0);
        console.info("====>testWebNapiLoadDataInvalidParam stop====");
        done();
      } catch (err) {
        console.error("====>testWebNapiLoadDataInvalidParam catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_NAPI_BlankInfo_0100
     * @tc.name    : testWebNapiSetBlanklessInfoNotInit
     * @tc.desc    : test OH_NativeArkWeb_SetBlanklessLoadingWithKey()
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiSetBlanklessInfoNotInit', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiSetBlanklessInfoNotInit start====");
          let result: number = nativeInterface.SetBlanklessInfo('new', '0731');
          expect(result).assertEqual(0);
          console.info("====>testWebNapiSetBlanklessInfoNotInit stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiSetBlanklessInfoNotInit catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_WEB_NET_NAPI_BlankInfo_0200
     * @tc.name    : testWebNapiSetBlanklessInfoSuccess
     * @tc.desc    : test OH_NativeArkWeb_SetBlanklessLoadingWithKey()
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiSetBlanklessInfoSuccess', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiSetBlanklessInfoSuccess start====");
          let result: number = nativeInterface.SetBlanklessInfo('web1211', '0731');
          expect(result).assertEqual(0);
          console.info("====>testWebNapiSetBlanklessInfoSuccess stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiSetBlanklessInfoSuccess catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_WEB_NET_NAPI_BlankInfo_0300
     * @tc.name    : testWebNapiSetBlanklessInfoNokey
     * @tc.desc    : test OH_NativeArkWeb_SetBlanklessLoadingWithKey()
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiSetBlanklessInfoNokey', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiSetBlanklessInfoNokey start====");
          let result: number = nativeInterface.SetBlanklessInfo('web1211', 'xxxxxx');
          expect(result).assertEqual(0);
          console.info("====>testWebNapiSetBlanklessInfoNokey stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiSetBlanklessInfoNokey catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_WEB_NET_NAPI_BlankInfo_0400
     * @tc.name    : testWebNapiGetBlanklessInfo
     * @tc.desc    : test OH_NativeArkWeb_SetBlanklessLoadingWithKey()
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiGetBlanklessInfo', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiGetBlanklessInfo start====");
          nativeInterface.SetBlanklessInfo('web1211', '0731');
          let result: number = nativeInterface.GetBlanklessInfo('web1211', '0731');
          expect(result).assertEqual(0);
          console.info("====>testWebNapiGetBlanklessInfo stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiGetBlanklessInfo catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_WEB_NET_NAPI_JSBridgeData_0100
     * @tc.name    : testWebNapiJSBridgeDataCallback
     * @tc.desc    : test ArkWeb_JavaScriptBridgeData and ArkWeb_OnJavaScriptCallback
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiJSBridgeDataCallback', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiJSBridgeDataCallback start====");
          let result: number = nativeInterface.JSBridgeData(0);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiJSBridgeDataCallback stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiJSBridgeDataCallback catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_WEB_NET_NAPI_JSBridgeData_0200
     * @tc.name    : testWebNapiJSBridgeCallbackNullUser
     * @tc.desc    : test ArkWeb_JavaScriptBridgeData and ArkWeb_OnJavaScriptCallback
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiJSBridgeCallbackNullUser', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiJSBridgeCallbackNullUser start====");
          let result: number = nativeInterface.JSBridgeData(1);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiJSBridgeCallbackNullUser stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiJSBridgeCallbackNullUser catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_WEB_NET_NAPI_JSBridgeData_0300
     * @tc.name    : testWebNapiJSBridgeCallbackNullData
     * @tc.desc    : test ArkWeb_JavaScriptBridgeData and ArkWeb_OnJavaScriptCallback
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiJSBridgeCallbackNullData', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiJSBridgeCallbackNullData start====");
          let result: number = nativeInterface.JSBridgeData(2);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiJSBridgeCallbackNullData stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiJSBridgeCallbackNullData catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_WEB_NET_NAPI_JSBridgeData_0400
     * @tc.name    : testWebNapiJSBridgeCallbackNullWeb
     * @tc.desc    : test ArkWeb_JavaScriptBridgeData and ArkWeb_OnJavaScriptCallback
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiJSBridgeCallbackNullWeb', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiJSBridgeCallbackNullWeb start====");
          let result: number = nativeInterface.JSBridgeData(3);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiJSBridgeCallbackNullWeb stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiJSBridgeCallbackNullWeb catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_WEB_NET_NAPI_JSBridgeProxy_0100
     * @tc.name    : testWebNapiJSBridgeProxyData
     * @tc.desc    : test ArkWeb_JavaScriptBridgeData and ArkWeb_OnJavaScriptProxyCallback
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiJSBridgeProxyData', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiJSBridgeProxyData start====");
          let result: number = nativeInterface.JSBridgeProxyData(0);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiJSBridgeProxyData stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiJSBridgeProxyData catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_WEB_NET_NAPI_JSBridgeProxy_0200
     * @tc.name    : testWebNapiJSBridgeProxyDataLen
     * @tc.desc    : test ArkWeb_JavaScriptBridgeData and ArkWeb_OnJavaScriptProxyCallback
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiJSBridgeProxyDataLen', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiJSBridgeProxyData start====");
          let result: number = nativeInterface.JSBridgeProxyData(1);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiJSBridgeProxyData stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiJSBridgeProxyData catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_WEB_NET_NAPI_JSBridgeProxy_0300
     * @tc.name    : testWebNapiJSBridgeProxyNullData
     * @tc.desc    : test ArkWeb_JavaScriptBridgeData and ArkWeb_OnJavaScriptProxyCallback
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiJSBridgeProxyNullData', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiJSBridgeProxyNullData start====");
          let result: number = nativeInterface.JSBridgeProxyData(2);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiJSBridgeProxyNullData stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiJSBridgeProxyNullData catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_WEB_NET_NAPI_JSBridgeProxy_0400
     * @tc.name    : testWebNapiJSBridgeProxyNullUser
     * @tc.desc    : test ArkWeb_JavaScriptBridgeData and ArkWeb_OnJavaScriptProxyCallback
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiJSBridgeProxyNullUser', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiJSBridgeProxyNullUser start====");
          let result: number = nativeInterface.JSBridgeProxyData(3);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiJSBridgeProxyNullUser stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiJSBridgeProxyNullUser catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_WEB_NET_NAPI_JSBridgeProxy_0500
     * @tc.name    : testWebNapiJSBridgeProxyNullWeb
     * @tc.desc    : test ArkWeb_JavaScriptBridgeData and ArkWeb_OnJavaScriptProxyCallback
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiJSBridgeProxyNullWeb', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiJSBridgeProxyNullWeb start====");
          let result: number = nativeInterface.JSBridgeProxyData(4);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiJSBridgeProxyNullWeb stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiJSBridgeProxyNullWeb catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_WebMsgTypeTest_0100
     * @tc.name    : testWebNapiWebMsgTypeEnumNone
     * @tc.desc    : test ArkWeb_WebMessageType ARKWEB_NONE
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiWebMsgTypeEnumNone', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiWebMsgTypeEnumNone start====");
          let result: number = nativeInterface.WebMsgType(0);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiWebMsgTypeEnumNone stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiWebMsgTypeEnumNone catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_WebMsgTypeTest_0200
     * @tc.name    : testWebNapiWebMsgTypeEnumString
     * @tc.desc    : test ArkWeb_WebMessageType ARKWEB_STRING
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiWebMsgTypeEnumString', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiWebMsgTypeEnumString start====");
          let result: number = nativeInterface.WebMsgType(1);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiWebMsgTypeEnumString stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiWebMsgTypeEnumString catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_WebMsgTypeTest_0300
     * @tc.name    : testWebNapiWebMsgTypeEnumBuffer
     * @tc.desc    : test ArkWeb_WebMessageType ARKWEB_BUFFER
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiWebMsgTypeEnumBuffer', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiWebMsgTypeEnumBuffer start====");
          let result: number = nativeInterface.WebMsgType(2);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiWebMsgTypeEnumBuffer stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiWebMsgTypeEnumBuffer catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_WebMsgApiTest_0100
     * @tc.name    : testWebNapiWebMsgApiSize
     * @tc.desc    : test ArkWeb_WebMessageAPI interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiWebMsgApiSize', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiWebMsgApiSize start====");
          let result: number = nativeInterface.WebMsgAPI(0);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiWebMsgApiSize stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiWebMsgApiSize catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_WebMsgApiTest_0200
     * @tc.name    : testWebNapiWebMsgApiCreate
     * @tc.desc    : test ArkWeb_WebMessageAPI interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiWebMsgApiCreate', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiWebMsgApiCreate start====");
          let result: number = nativeInterface.WebMsgAPI(1);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiWebMsgApiCreate stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiWebMsgApiCreate catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_WebMsgApiTest_0300
     * @tc.name    : testWebNapiWebMsgApiDestroy
     * @tc.desc    : test ArkWeb_WebMessageAPI interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiWebMsgApiDestroy', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiWebMsgApiDestroy start====");
          let result: number = nativeInterface.WebMsgAPI(2);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiWebMsgApiDestroy stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiWebMsgApiDestroy catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_WebMsgApiTest_0400
     * @tc.name    : testWebNapiWebMsgApiSetType
     * @tc.desc    : test ArkWeb_WebMessageAPI interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiWebMsgApiSetType', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiWebMsgApiSetType start====");
          let result: number = nativeInterface.WebMsgAPI(3);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiWebMsgApiSetType stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiWebMsgApiSetType catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_WebMsgApiTest_0500
     * @tc.name    : testWebNapiWebMsgApiGetType
     * @tc.desc    : test ArkWeb_WebMessageAPI interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiWebMsgApiGetType', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiWebMsgApiGetType start====");
          let result: number = nativeInterface.WebMsgAPI(4);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiWebMsgApiGetType stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiWebMsgApiGetType catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_WebMsgApiTest_0600
     * @tc.name    : testWebNapiWebMsgApiSetData
     * @tc.desc    : test ArkWeb_WebMessageAPI interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiWebMsgApiSetData', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiWebMsgApiSetData start====");
          let result: number = nativeInterface.WebMsgAPI(5);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiWebMsgApiSetData stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiWebMsgApiSetData catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_WebMsgApiTest_0700
     * @tc.name    : testWebNapiWebMsgApiGetData
     * @tc.desc    : test ArkWeb_WebMessageAPI interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiWebMsgApiGetData', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiWebMsgApiGetData start====");
          let result: number = nativeInterface.WebMsgAPI(6);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiWebMsgApiGetData stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiWebMsgApiGetData catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_WebJSValueApiTest_0100
     * @tc.name    : testWebNapiWebJSValueApiSize
     * @tc.desc    : test ArkWeb_JavaScriptValueAPI interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiWebJSValueApiSize', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiWebJSValueApiSize start====");
          let result: number = nativeInterface.WebJSValueAPI(0);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiWebJSValueApiSize stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiWebJSValueApiSize catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_WebJSValueApiTest_0200
     * @tc.name    : testWebNapiWebJSValueApiCreate
     * @tc.desc    : test ArkWeb_JavaScriptValueAPI interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiWebJSValueApiCreate', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiWebJSValueApiCreate start====");
          let result: number = nativeInterface.WebJSValueAPI(1);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiWebJSValueApiCreate stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiWebJSValueApiCreate catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_WebJSValue_0100
     * @tc.name    : testWebNapiWebJSValueEnumNone
     * @tc.desc    : test ArkWeb_JavaScriptValueType ARKWEB_JAVASCRIPT_NONE
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiWebJSValueEnumNone', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiWebJSValueEnumNone start====");
          let result: number = nativeInterface.WebJSValueType(0);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiWebJSValueEnumNone stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiWebJSValueEnumNone catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_WebJSValue_0200
     * @tc.name    : testWebNapiWebJSValueEnumString
     * @tc.desc    : test ArkWeb_JavaScriptValueType ARKWEB_JAVASCRIPT_STRING
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiWebJSValueEnumString', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiWebJSValueEnumString start====");
          let result: number = nativeInterface.WebJSValueType(1);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiWebJSValueEnumString stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiWebJSValueEnumString catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_WebJSValue_0300
     * @tc.name    : testWebNapiWebJSValueEnumBool
     * @tc.desc    : test ArkWeb_JavaScriptValueType ARKWEB_JAVASCRIPT_BOOL
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiWebJSValueEnumBool', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiWebJSValueEnumBool start====");
          let result: number = nativeInterface.WebJSValueType(2);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiWebJSValueEnumBool stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiWebJSValueEnumBool catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_WebCtrlAPI_0100
     * @tc.name    : testWebNapiControllerAPISize
     * @tc.desc    : test ArkWeb_ControllerAPI interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiControllerAPISize', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiControllerAPISize start====");
          let result: number = nativeInterface.WebControllerAPI(0);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiControllerAPISize stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiControllerAPISize catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_WebCtrlAPI_0200
     * @tc.name    : testWebNapiControllerAPIRunJS
     * @tc.desc    : test ArkWeb_ControllerAPI interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiControllerAPIRunJS', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiControllerAPIRunJS start====");
          let result: number = nativeInterface.WebControllerAPI(1);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiControllerAPIRunJS stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiControllerAPIRunJS catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_WebCtrlAPI_0300
     * @tc.name    : testWebNapiControllerRegJSProxy
     * @tc.desc    : test ArkWeb_ControllerAPI interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiControllerRegJSProxy', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiControllerRegJSProxy start====");
          let result: number = nativeInterface.WebControllerAPI(2);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiControllerRegJSProxy stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiControllerRegJSProxy catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_WebCtrlAPI_0400
     * @tc.name    : testWebNapiControllerDelJSProxy
     * @tc.desc    : test ArkWeb_ControllerAPI interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiControllerDelJSProxy', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiControllerDelJSProxy start====");
          let result: number = nativeInterface.WebControllerAPI(3);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiControllerDelJSProxy stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiControllerRegJSProxy catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_WebCtrlAPI_0500
     * @tc.name    : testWebNapiControllerRefresh
     * @tc.desc    : test ArkWeb_ControllerAPI interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiControllerRefresh', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiControllerRefresh start====");
          let result: number = nativeInterface.WebControllerAPI(4);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiControllerRefresh stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiControllerRefresh catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_WebCtrlAPI_0600
     * @tc.name    : testWebNapiControllerRegAsyncJSProxy
     * @tc.desc    : test ArkWeb_ControllerAPI interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiControllerRegAsyncJSProxy', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiControllerRegAsyncJSProxy start====");
          let result: number = nativeInterface.WebControllerAPI(5);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiControllerRegAsyncJSProxy stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiControllerRegAsyncJSProxy catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_WebCtrlAPI_0700
     * @tc.name    : testWebNapiControllerCreateWebMsgPort
     * @tc.desc    : test ArkWeb_ControllerAPI interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiControllerCreateWebMsgPort', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiControllerCreateWebMsgPort start====");
          let result: number = nativeInterface.WebControllerAPI(6);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiControllerCreateWebMsgPort stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiControllerCreateWebMsgPort catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_WebCtrlAPI_0800
     * @tc.name    : testWebNapiControllerDestroyWebMsgPort
     * @tc.desc    : test ArkWeb_ControllerAPI interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiControllerDestroyWebMsgPort', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiControllerDestroyWebMsgPort start====");
          let result: number = nativeInterface.WebControllerAPI(7);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiControllerDestroyWebMsgPort stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiControllerDestroyWebMsgPort catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_WebCtrlAPI_0900
     * @tc.name    : testWebNapiControllerPostWebMsg
     * @tc.desc    : test ArkWeb_ControllerAPI interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiControllerPostWebMsg', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiControllerPostWebMsg start====");
          let result: number = nativeInterface.WebControllerAPI(8);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiControllerPostWebMsg stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiControllerPostWebMsg catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_WebCtrlAPI_1000
     * @tc.name    : testWebNapiControllerGetLast
     * @tc.desc    : test ArkWeb_ControllerAPI interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiControllerGetLast', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiControllerGetLast start====");
          let result: number = nativeInterface.WebControllerAPI(9);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiControllerGetLast stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiControllerGetLast catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_WebCtrlAPI_1100
     * @tc.name    : testWebNapiControllerRegJSProxyEX
     * @tc.desc    : test ArkWeb_ControllerAPI interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiControllerRegJSProxyEX', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiControllerRegJSProxyEX start====");
          let result: number = nativeInterface.WebControllerAPI(10);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiControllerRegJSProxyEX stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiControllerRegJSProxyEX catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_WebCtrlAPI_1200
     * @tc.name    : testWebNapiControllerRegAsyncJSProxyEX
     * @tc.desc    : test ArkWeb_ControllerAPI interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiControllerRegAsyncJSProxyEX', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiControllerRegAsyncJSProxyEX start====");
          let result: number = nativeInterface.WebControllerAPI(11);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiControllerRegAsyncJSProxyEX stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiControllerRegAsyncJSProxyEX catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_Web_Napi_JSObject_0100
     * @tc.name    : testWebNapiJSObjectSize
     * @tc.desc    : test ArkWeb_JavaScriptObject interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiJSObjectSize', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiJSObjectSize start====");
          let result: number = nativeInterface.WebJSObject(0);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiJSObjectSize stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiJSObjectSize catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_JSObject_0200
     * @tc.name    : testWebNapiJSObjectBuffer
     * @tc.desc    : test ArkWeb_JavaScriptObject interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiJSObjectBuffer', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiJSObjectBuffer start====");
          let result: number = nativeInterface.WebJSObject(1);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiJSObjectBuffer stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiJSObjectBuffer catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_JSObject_0300
     * @tc.name    : testWebNapiJSObjectCallback
     * @tc.desc    : test ArkWeb_JavaScriptObject interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiJSObjectCallback', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiJSObjectCallback start====");
          let result: number = nativeInterface.WebJSObject(2);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiJSObjectCallback stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiJSObjectCallback catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_JSObject_0400
     * @tc.name    : testWebNapiJSObjectUserData
     * @tc.desc    : test ArkWeb_JavaScriptObject interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiJSObjectUserData', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiJSObjectUserData start====");
          let result: number = nativeInterface.WebJSObject(3);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiJSObjectUserData stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiJSObjectUserData catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_ProxyMethod_0100
     * @tc.name    : testWebNapiProxyMethodName
     * @tc.desc    : test ArkWeb_ProxyMethod struct interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiProxyMethodName', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiProxyMethodName start====");
          let result: number = nativeInterface.WebStructProxyMethod(0);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiProxyMethodName stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiProxyMethodName catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_ProxyMethod_0200
     * @tc.name    : testWebNapiProxyMethodCallback
     * @tc.desc    : test ArkWeb_ProxyMethod struct interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiProxyMethodCallback', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiProxyMethodCallback start====");
          let result: number = nativeInterface.WebStructProxyMethod(1);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiProxyMethodCallback stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiProxyMethodCallback catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_ProxyMethod_0300
     * @tc.name    : testWebNapiProxyMethodUserdata
     * @tc.desc    : test ArkWeb_ProxyMethod struct interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiProxyMethodUserdata', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiProxyMethodUserdata start====");
          let result: number = nativeInterface.WebStructProxyMethod(2);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiProxyMethodUserdata stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiProxyMethodUserdata catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_ProxyMethodWithResult_0100
     * @tc.name    : testWebNapiProxyMethodWithResultName
     * @tc.desc    : test ArkWeb_ProxyMethodWithResult struct interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiProxyMethodWithResultName', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiProxyMethodWithResultName start====");
          let result: number = nativeInterface.WebStructProxyMethodWithResult(0);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiProxyMethodWithResultName stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiProxyMethodWithResultName catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_ProxyMethodWithResult_0200
     * @tc.name    : testWebNapiProxyMethodWithResultCallback
     * @tc.desc    : test ArkWeb_ProxyMethodWithResult struct interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiProxyMethodWithResultCallback', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiProxyMethodWithResultCallback start====");
          let result: number = nativeInterface.WebStructProxyMethodWithResult(1);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiProxyMethodWithResultCallback stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiProxyMethodWithResultCallback catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_ProxyMethodWithResult_0300
     * @tc.name    : testWebNapiProxyMethodWithResultUserdata
     * @tc.desc    : test ArkWeb_ProxyMethodWithResult struct interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiProxyMethodWithResultUserdata', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiProxyMethodWithResultUserdata start====");
          let result: number = nativeInterface.WebStructProxyMethodWithResult(2);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiProxyMethodWithResultUserdata stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiProxyMethodWithResultUserdata catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_ProxyObject_0100
     * @tc.name    : testWebNapiProxyObjectName
     * @tc.desc    : test ArkWeb_ProxyObject struct interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiProxyObjectName', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiProxyObjectName start====");
          let result: number = nativeInterface.WebStructProxyObject(0);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiProxyObjectName stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiProxyObjectName catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_ProxyObject_0200
     * @tc.name    : testWebNapiProxyObjectMethodList
     * @tc.desc    : test ArkWeb_ProxyObject struct interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiProxyObjectMethodList', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiProxyObjectMethodList start====");
          let result: number = nativeInterface.WebStructProxyObject(1);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiProxyObjectMethodList stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiProxyObjectMethodList catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_ProxyObject_0300
     * @tc.name    : testWebNapiProxyObjectSize
     * @tc.desc    : test ArkWeb_ProxyObject struct interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiProxyObjectSize', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiProxyObjectSize start====");
          let result: number = nativeInterface.WebStructProxyObject(2);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiProxyObjectSize stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiProxyObjectSize catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_ProxyObjectWithResult_0100
     * @tc.name    : testWebNapiProxyObjectWithResultName
     * @tc.desc    : test ArkWeb_ProxyObject struct interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiProxyObjectWithResultName', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiProxyObjectWithResultName start====");
          let result: number = nativeInterface.ArkWeb_ProxyObjectWithResult(0);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiProxyObjectWithResultName stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiProxyObjectWithResultName catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_ProxyObjectWithResult_0200
     * @tc.name    : testWebNapiProxyObjectWithResultMethodList
     * @tc.desc    : test ArkWeb_ProxyObject struct interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiProxyObjectWithResultMethodList', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiProxyObjectWithResultMethodList start====");
          let result: number = nativeInterface.ArkWeb_ProxyObjectWithResult(1);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiProxyObjectWithResultMethodList stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiProxyObjectWithResultMethodList catch err: " + err);
        }
      })
    /**
     * @tc.number  : SUB_Web_Napi_ProxyObjectWithResult_0300
     * @tc.name    : testWebNapiProxyObjectWithResultSize
     * @tc.desc    : test ArkWeb_ProxyObject struct interface
     * @tc.level   : Level2
     * @tc.size    : SMALLTEST
     * @tc.type    : Function
     */
    it('testWebNapiProxyObjectWithResultSize', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2,
      async (done: Function) => {
        try {
          console.info("====>testWebNapiProxyObjectWithResultSize start====");
          let result: number = nativeInterface.ArkWeb_ProxyObjectWithResult(2);
          expect(result).assertEqual(0);
          console.info("====>testWebNapiProxyObjectWithResultSize stop====");
          done();
        } catch (err) {
          console.error("====>testWebNapiProxyObjectWithResultSize catch err: " + err);
        }
      })


  })
}