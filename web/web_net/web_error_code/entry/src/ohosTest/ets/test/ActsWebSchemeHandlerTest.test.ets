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

import { describe, it, expect, TestType, Size, Level } from '@ohos/hypium'
import webCode from 'libweberrorcodetest.so'

export default function ActsWebSchemeHandlerTest() {
  describe('ActsWebSchemeHandlerTest', () => {
    /**
     * @tc.number  : SUB_WEB_CUSTOM_SCHEME_OPTION_0100
     * @tc.name    : testArkWebSchemeOptionNone
     * @tc.desc    : test ArkWeb_CustomSchemeOption OH_ARKWEB_SCHEME_OPTION_NONE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebSchemeOptionNone', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        let result: number = webCode.webRegisterCustomSchemes("OH_ARKWEB_SCHEME_OPTION_NONE");
        console.info("====>testArkWebSchemeOptionNone result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebSchemeOptionNone catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_CUSTOM_SCHEME_OPTION_0200
     * @tc.name    : testArkWebSchemeOptionSecure
     * @tc.desc    : test ArkWeb_CustomSchemeOption ARKWEB_SCHEME_OPTION_SECURE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebSchemeOptionSecure', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        let result: number = webCode.webRegisterCustomSchemes("ARKWEB_SCHEME_OPTION_SECURE");
        console.info("====>testArkWebSchemeOptionSecure result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebSchemeOptionSecure catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_CUSTOM_SCHEME_OPTION_0300
     * @tc.name    : testArkWebSchemeOptionCodeCacheEnabled
     * @tc.desc    : test ArkWeb_CustomSchemeOption ARKWEB_SCHEME_OPTION_CODE_CACHE_ENABLED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebSchemeOptionCodeCacheEnabled', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        let result: number = webCode.webRegisterCustomSchemes("ARKWEB_SCHEME_OPTION_CODE_CACHE_ENABLED");
        console.info("====>testArkWebSchemeOptionCodeCacheEnabled result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebSchemeOptionCodeCacheEnabled catch err: " + err);
      }
    })

  })
}