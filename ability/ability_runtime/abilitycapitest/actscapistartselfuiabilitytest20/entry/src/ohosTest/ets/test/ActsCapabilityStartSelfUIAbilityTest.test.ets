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
import { Driver } from '@kit.TestKit';

const OS_ACCOUNT_ERR_OK = 0;

export default function ActsCapabilityStartSelfUIAbilityTest() {
  describe('ActsCapabilityStartSelfUIAbilityTest', () => {

    /**
     * @tc.number  : Acts_Capability_Start_Self_UIAbility_Test
     * @tc.name    : Acts_Capability_Start_Self_UIAbility_Test
     * @tc.desc    : Gets the name of the OS account to which the caller process belongs.
     * @tc.level   : Level0
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Acts_Capability_Start_Self_UIAbility_Test', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0, async (done: Function) => {
      try {
        console.info("====>Acts_Capability_Start_Self_UIAbility_Test start====");
        let result = osAccountNdk.DestroyChildProcessConfigsInvalid();
        expect(401).assertEqual(result);
        console.info("====>Acts_Capability_Start_Self_UIAbility_Test result: " + result);
        done()
      } catch (err) {
        console.error("====>Acts_Capability_Start_Self_UIAbility_Test catch err: " + err);
        expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
        done()
      }
    })

  })
}
