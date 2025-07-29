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

import { describe, it, expect, Level } from '@ohos/hypium'
import osAccountNdk from 'libosaccountndk.so'

export default function ActsOsAccountNdkTest() {
  describe('ActsOsAccountNdkTest', () => {
    /**
     * @tc.number  : SUB_Oh_Print_0100
     * @tc.name    : SUB_Oh_Print_0100
     * @tc.desc    : Gets the name of the OS account to which the caller process belongs.
     * @tc.level   : Level3
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Oh_Print_0100', Level.LEVEL1, async (done: Function) => {
      let result: number = await osAccountNdk.OhPrint1();
      expect(result).assertEqual(0);
      done();
    })

    /**
     * @tc.number  : SUB_Oh_Print_0200
     * @tc.name    : SUB_Oh_Print_0200
     * @tc.desc    : Gets the name of the OS account to which the caller process belongs.
     * @tc.level   : Level3
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Oh_Print_0200', Level.LEVEL1, async (done: Function) => {
      let result: number = await osAccountNdk.OhPrint2();
      console.log("---- xie3 ---",result);
      expect(result).assertEqual(0);
      done();
    })

    /**
     * @tc.number  : SUB_Oh_Print_0300
     * @tc.name    : SUB_Oh_Print_0300
     * @tc.desc    : Gets the name of the OS account to which the caller process belongs.
     * @tc.level   : Level3
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Oh_Print_0300', Level.LEVEL1, async (done: Function) => {
      console.log("---- xie1 ---")
      let result: number = await osAccountNdk.OhPrint3();
      console.log("---- xie2 ---",JSON.stringify(result))
      expect(result).assertEqual(401);
      done();
    })

    /**
     * @tc.number  : SUB_Oh_Print_0400
     * @tc.name    : SUB_Oh_Print_0400
     * @tc.desc    : Gets the name of the OS account to which the caller process belongs.
     * @tc.level   : Level3
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Oh_Print_0400', Level.LEVEL1, async (done: Function) => {
      let result: number = await osAccountNdk.OhPrint4();
      expect(result).assertEqual(24300001);
      done();
    })

    /**
     * @tc.number  : SUB_Oh_Print_0500
     * @tc.name    : SUB_Oh_Print_0500
     * @tc.desc    : Gets the name of the OS account to which the caller process belongs.
     * @tc.level   : Level3
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Oh_Print_0500', Level.LEVEL1, async (done: Function) => {
      let result: number = await osAccountNdk.OhPrint5();
      expect(result).assertEqual(24300006);
      done();
    })

    /**
     * @tc.number  : SUB_Oh_Print_0600
     * @tc.name    : SUB_Oh_Print_0600
     * @tc.desc    : Gets the name of the OS account to which the caller process belongs.
     * @tc.level   : Level3
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Oh_Print_0600', Level.LEVEL1, async (done: Function) => {
      let result: number = await osAccountNdk.OhPrint6();
      expect(result).assertEqual(24300005);
      done();
    })
  })
}
