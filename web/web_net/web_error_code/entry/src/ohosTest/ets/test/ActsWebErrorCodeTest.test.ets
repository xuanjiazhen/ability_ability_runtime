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
import weberrorCode from 'libweberrorcodetest.so'

const TEST_ARKWEB_SUCCESS = 0;
const TEST_ARKWEB_INIT_ERROR = 17100001;
const TEST_ARKWEB_ERROR_UNKNOWN = 17100100;
const TEST_ARKWEB_INVALID_PARAM = 17100101;
const TEST_ARKWEB_SCHEME_REGISTER_FAILED = 17100102;
const TEST_ARKWEB_INVALID_URL = 17100103;


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
        let result: number = weberrorCode.webErrCodeSuccess();
        console.info("====>SUB_OH_Web_Error_Code_0100 result====", result);
        expect(result).assertEqual(TEST_ARKWEB_SUCCESS);
        done();
      } catch (err) {
        console.error("====>SUB_OH_Web_Error_Code_0100 catch err: " + err);
      }
    })



  })
}