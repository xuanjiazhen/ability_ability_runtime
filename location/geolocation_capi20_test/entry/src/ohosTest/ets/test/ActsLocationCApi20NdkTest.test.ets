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
import LocationCApi20Ndk from 'liblocationcapi20ndk.so'
import { Driver } from '@kit.TestKit';

const OS_ACCOUNT_ERR_OK = 0;

export default function ActsLocationCApi20NdkTest() {
  describe('ActsLocationCApi20NdkTest', () => {
    /**
     * @tc.number  : OHLocationInfoGetAdditionalInfo_test
     * @tc.name    : OHLocationInfoGetAdditionalInfo_test
     * @tc.desc    : Gets the name of the OS account to which the caller process belongs.
     * @tc.level   : Level3
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('OHLocationInfoGetAdditionalInfo_test', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3,
      async (done: Function) => {
        try {
          console.info("====>OHLocationInfoGetAdditionalInfo_test start====");
          let result: number = LocationCApi20Ndk.OHLocationInfoGetAdditionalInfo_test();
          console.info("====>OHLocationInfoGetAdditionalInfo_test result: " + result);
          expect(401).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>OHLocationInfoGetAdditionalInfo_test catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })
  })
}
