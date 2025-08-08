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
import NetConnectionAuthTest from 'libnetconnectionauthtest.so'

const OH_HTTP_RESULT_OK = 0;
const CASE_INDEX_1 = 1;
const CASE_INDEX_2 = 2;
const CASE_INDEX_3 = 3;
const CASE_INDEX_4 = 4;

export default function ActsNetConnectionAuth() {
  describe('ActsNetConnectionAuth', () => {
    /**
     * @tc.number  : SUB_NetworkManager_Http_Request_Auth_0100
     * @tc.name    : testNetworkManagerHttpRequestAuth0100
     * @tc.desc    : test Http_Request error code
     * @tc.level   : Level2
     * @tc.size    : SmallTest
     * @tc.type    : Function
     */
    it('testNetworkManagerHttpRequestAuth0100', TestType.FUNCTION | Size.SMALLTEST | Level.LEVEL2, async (done: Function) => {
      let caseName: string = 'testNetworkManagerHttpRequestAuth0100';
      try {
        console.info(`${caseName} ---start`);
        let result : number = NetConnectionAuthTest.Http_RequestRequest(CASE_INDEX_1, "http://www.baidu.com");
        console.info(`${caseName} result is: ` + JSON.stringify(result));
        expect(result).assertEqual(OH_HTTP_RESULT_OK);
        done();
      } catch (err) {
        console.error(`${caseName} catch err: ` + JSON.stringify(err));
        expect(false).assertEqual(true);
        done();
      }
    })

  })
}
