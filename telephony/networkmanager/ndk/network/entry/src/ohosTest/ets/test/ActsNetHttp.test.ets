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
import NetHttpTest from 'libnethttptest.so'

const CASE_INDEX_1 = 1;
const CASE_INDEX_2 = 2;
const CASE_INDEX_3 = 3;
const CASE_INDEX_4 = 4;
const CASE_INDEX_5 = 4;
const CASE_INDEX_6 = 6;
const CASE_INDEX_7 = 7;
const CASE_INDEX_8 = 8;

const OH_HTTP_OK = 0;
const NETCONN_PERMISSION_DENIED = 201;
const OH_HTTP_PARAMETER_ERROR = 401;
const OH_HTTP_OUT_OF_MEMORY = 2300027;
const NETCONN_INTERNAL_ERROR = 2100003;

export default function ActsNetHttpTest() {
  describe('ActsNetHttpTest', () => {

  /**
   * @tc.number  : SUB_NetworkManager_Http_CreateHeaders_0100
   * @tc.name    : testNetworkManagerHttpCreateHeaders0100
   * @tc.desc    : Verify creation of HTTP headers
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
   it('testNetworkManagerHttpCreateHeaders0100', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerHttpCreateHeaders0100';
    try {
      console.info(`${caseName} ---start`);
      let result: number = NetHttpTest.Http_CreateHeaders();
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_OK);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }

  })

  /**
    * @tc.number  : SUB_NetworkManager_Http_SetHeaderValue_0100
    * @tc.name    : testNetworkManagerHttpSetHeaderValue0100
    * @tc.desc    : Verify setting a header value in HTTP headers
    * @tc.level   : Level2
    * @tc.size    : SmallTest
    * @tc.type    : Function
   */

  it('testNetworkManagerHttpSetHeaderValue0100', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerHttpSetHeaderValue0100';
    try {
      console.info(`${caseName} ---start`);
      let result: number = NetHttpTest.Http_SetHeaderValue(CASE_INDEX_1);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_PARAMETER_ERROR);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /**
    * @tc.number  : SUB_NetworkManager_Http_SetHeaderValue_0200
    * @tc.name    : testNetworkManagerHttpSetHeaderValue0200
    * @tc.desc    : Verify setting a header value in HTTP headers
    * @tc.level   : Level2
    * @tc.size    : SmallTest
    * @tc.type    : Function
   */
   it('testNetworkManagerHttpSetHeaderValue0200', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerHttpSetHeaderValue0200';
    try {
      console.info(`${caseName} ---start`);
      let result: number = NetHttpTest.Http_SetHeaderValue(CASE_INDEX_2);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_PARAMETER_ERROR);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /**
   * @tc.number  : SUB_NetworkManager_Http_SetHeaderValue_0300
   * @tc.name    : testNetworkManagerHttpSetHeaderValue0300
   * @tc.desc    : Verify setting a header value in HTTP headers
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
  */
   it('testNetworkManagerHttpSetHeaderValue0300', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerHttpSetHeaderValue0300';
    try {
      console.info(`${caseName} ---start`);
      let result: number = NetHttpTest.Http_SetHeaderValue(CASE_INDEX_3);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_OK);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /**
   * @tc.number  : SUB_NetworkManager_Http_SetHeaderValue_0400
   * @tc.name    : testNetworkManagerHttpSetHeaderValue0400
   * @tc.desc    : Verify setting a header value in HTTP headers
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
  */
  it('testNetworkManagerHttpSetHeaderValue0400', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerHttpSetHeaderValue0400';
    try {
      console.info(`${caseName} ---start`);
      let result: number = NetHttpTest.Http_SetHeaderValue(CASE_INDEX_4);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_PARAMETER_ERROR);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /**
   * @tc.number  : SUB_NetworkManager_Http_SetHeaderValue_0500
   * @tc.name    : testNetworkManagerHttpSetHeaderValue0500
   * @tc.desc    : Verify setting a header value in HTTP headers
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
  */
  it('testNetworkManagerHttpSetHeaderValue0500', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerHttpSetHeaderValue0500';
    try {
      console.info(`${caseName} ---start`);
      let result: number = NetHttpTest.Http_SetHeaderValue(CASE_INDEX_5);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_PARAMETER_ERROR);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /**
   * @tc.number  : SUB_NetworkManager_Http_SetHeaderValue_0600
   * @tc.name    : testNetworkManagerHttpSetHeaderValue0600
   * @tc.desc    : Verify setting a header value in HTTP headers
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
  */
  it('testNetworkManagerHttpSetHeaderValue0600', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerHttpSetHeaderValue0600';
    try {
      console.info(`${caseName} ---start`);
      let result: number = NetHttpTest.Http_SetHeaderValue(CASE_INDEX_6);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_PARAMETER_ERROR);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /**
   * @tc.number  : SUB_NetworkManager_Http_GetHeaderValue_0100
   * @tc.name    : testNetworkManagerHttpGetHeaderValue0100
   * @tc.desc    : Verify getting a header value in HTTP headers
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerHttpGetHeaderValue0100', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerHttpGetHeaderValue0100';
    try {
      console.info(`${caseName} ---start`);
      let result: number = NetHttpTest.Http_GetHeaderValue(CASE_INDEX_1);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_OK);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })
  /**
   * @tc.number  : SUB_NetworkManager_Http_GetHeaderValue_0200
   * @tc.name    : testNetworkManagerHttpGetHeaderValue0200
   * @tc.desc    : Verify getting a header value in HTTP headers
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerHttpGetHeaderValue0200', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerHttpGetHeaderValue0200';
    try {
      console.info(`${caseName} ---start`);
      let result: number = NetHttpTest.Http_GetHeaderValue(CASE_INDEX_2);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_OK);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }

  })

  /**
   * @tc.number  : SUB_NetworkManager_Http_GetHeaderValue_0300
   * @tc.name    : testNetworkManagerHttpGetHeaderValue0300
   * @tc.desc    : Verify getting a header value in HTTP headers
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerHttpGetHeaderValue0300', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerHttpGetHeaderValue0300';
    try {
      console.info(`${caseName} ---start`);
      let result: number = NetHttpTest.Http_GetHeaderValue(CASE_INDEX_3);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_OK);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /**
   * @tc.number  : SUB_NetworkManager_Http_GetHeaderValue_0400
   * @tc.name    : testNetworkManagerHttpGetHeaderValue0400
   * @tc.desc    : Verify getting a header value in HTTP headers
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerHttpGetHeaderValue0400', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerHttpGetHeaderValue0400';
    try {
      console.info(`${caseName} ---start`);
      let result: number = NetHttpTest.Http_GetHeaderValue(CASE_INDEX_4);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_OK);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }

  })

  /**
   * @tc.number  : SUB_NetworkManager_Http_GetHeaderEntries_0100
   * @tc.name    : testNetworkManagerHttpGetHeaderEntries0100
   * @tc.desc    : Verify getting header entries in HTTP headers
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerHttpGetHeaderEntries0100', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerHttpGetHeaderEntries0100';
    try {
      console.info(`${caseName} ---start`);
      let result: number = NetHttpTest.Http_GetHeaderEntries(CASE_INDEX_1);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_OK);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /**
   * @tc.number  : SUB_NetworkManager_Http_GetHeaderEntries_0200
   * @tc.name    : testNetworkManagerHttpGetHeaderEntries0200
   * @tc.desc    : Verify getting header entries in HTTP headers
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerHttpGetHeaderEntries0200', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerHttpGetHeaderEntries0200';
    try {
      console.info(`${caseName} ---start`);
      let result: number = NetHttpTest.Http_GetHeaderEntries(CASE_INDEX_2);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_OK);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }

  })

  /**
   * @tc.number  : SUB_NetworkManager_Http_DestroyHeaderEntries_0100
   * @tc.name    : testNetworkManagerHttpDestroyHeaderEntries0100
   * @tc.desc    : Verify proper cleanup of HTTP header entries
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerHttpDestroyHeaderEntries0100', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerHttpDestroyHeaderEntries0100';
    try {
      console.info(`${caseName} ---start`);
      let result: number = NetHttpTest.Http_DestroyHeaderEntries(CASE_INDEX_1);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_OK);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /**
   * @tc.number  : SUB_NetworkManager_Http_DestroyHeaderEntries_0200
   * @tc.name    : testNetworkManagerHttpDestroyHeaderEntries0200
   * @tc.desc    : Verify proper cleanup of HTTP header entries
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerHttpDestroyHeaderEntries0200', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerHttpDestroyHeaderEntries0200';
    try {
      console.info(`${caseName} ---start`);
      let result: number = NetHttpTest.Http_DestroyHeaderEntries(CASE_INDEX_2);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_OK);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /*
    * @tc.number  : SUB_NetworkManager_Http_CreateRequest_0100
    * @tc.name    : testNetworkManagerHttpCreateRequest0100
    * @tc.desc    : Verify creation of HTTP request
    * @tc.level   : Level2
    * @tc.size    : SmallTest
    * @tc.type    : Function
    */

  it('testNetworkManagerHttpCreateRequest0100', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerHttpCreateRequest0100';
    try {
      console.info(`${caseName} ---start`);
      let result: number = NetHttpTest.Http_CreateRequest("");
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_OK);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }

  })

    /*
    * @tc.number  : SUB_NetworkManager_Http_CreateRequest_0200
    * @tc.name    : testNetworkManagerHttpCreateRequest0200
    * @tc.desc    : Verify creation of HTTP request
    * @tc.level   : Level2
    * @tc.size    : SmallTest
    * @tc.type    : Function
    */

  it('testNetworkManagerHttpCreateRequest0200', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerHttpCreateRequest0200';
    try {
      console.info(`${caseName} ---start`);
      let result: number = NetHttpTest.Http_CreateRequest("www.baidu.com");
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_OK);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /**
   * @tc.number  : SUB_NetworkManager_Http_Request_0100
   * @tc.name    : testNetworkManagerHttpRequest0100
   * @tc.desc    : Verify HTTP request execution
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerHttpRequest0100', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerHttpRequest0100';
    try {
      console.info(`${caseName} ---start`);
      let result: number = NetHttpTest.Http_RequestRequest(CASE_INDEX_1, "www.baidu.com");
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(NETCONN_PERMISSION_DENIED);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /**
   * @tc.number  : SUB_NetworkManager_Http_Request_0200
   * @tc.name    : testNetworkManagerHttpRequest0200
   * @tc.desc    : Verify HTTP request execution with invalid URL
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerHttpRequest0200', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerHttpRequest0200';
    try {
      console.info(`${caseName} ---start`);
      let result: number = NetHttpTest.Http_RequestRequest(CASE_INDEX_2, "www.baidu.com");
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_OUT_OF_MEMORY);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /**
   * @tc.number  : SUB_NetworkManager_Http_Destroy_0100
   * @tc.name    : testNetworkManagerHttpDestroy0100
   * @tc.desc    : Verify proper cleanup of HTTP resources
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerHttpDestroy0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerHttpDestroy0100';
    try {
      console.info(`${caseName} ---start`);
      let result: number = NetHttpTest.Http_Destroy(CASE_INDEX_1);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_OK);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

      /**
   * @tc.number  : SUB_NetworkManager_Http_Destroy_0200
   * @tc.name    : testNetworkManagerHttpDestroy0200
   * @tc.desc    : Verify proper cleanup of HTTP resources
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerHttpDestroy0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerHttpDestroy0200';
    try {
      console.info(`${caseName} ---start`);
      let result: number = NetHttpTest.Http_Destroy(CASE_INDEX_2);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_OK);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })
  })
}
