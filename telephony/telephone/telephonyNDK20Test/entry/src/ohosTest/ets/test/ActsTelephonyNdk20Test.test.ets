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
import osAccountNdk from 'libosaccountndk.so'

const OS_ACCOUNT_ERR_OK = 0;

export default function ActsTelephonyNdk20Test() {
  describe('ActsTelephonyNdk20Test', () => {
    /**
     * @tc.number  : SUB_TelephonyKit_OH_Telephony_GetNetworkState_test_0100
     * @tc.name    : SUB_TelephonyKit_OH_Telephony_GetNetworkState_test_0100
     * @tc.desc    : Gets the name of the OS account to which the caller process belongs.
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_TelephonyKit_OH_Telephony_GetNetworkState_test_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>SUB_TelephonyKit_OH_Telephony_GetNetworkState_test_0100 start====");
        let result = osAccountNdk.OHTelephonyGetNetworkState3Test();
        console.info("====>SUB_TelephonyKit_OH_Telephony_GetNetworkState_test_0100 result: " + result);
        expect(401).assertEqual(result);
        done()
      } catch (err) {
        console.error("====>SUB_TelephonyKit_OH_Telephony_GetNetworkState_test_0100 catch err: " + err);
        expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
        done()
      }
    })
  })
}
