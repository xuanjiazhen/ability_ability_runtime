/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

import { describe, expect, it, TestType, Size, Level } from '@ohos/hypium';
import mouse from 'libmousendk.so';

export default function MultimodalMouseNdkTest() {
  describe('MultimodalMouseNdkTest', () => {

    const ExpectTrue = (n: boolean) => {
      try {
        expect(n).assertTrue();
      } catch (err) {
        console.info("expectInfo", `test failed`);
      }
    }
    const ExpectFalse = (n: boolean) => {
      try {
        expect(n).assertFail();
      } catch (err) {
        console.info("expectInfo", `test failed`);
      }
    }

    /**
     * @tc.number: SUB_MMI_Input_Api_GetPointerLocation_0100
     * @tc.name: SUB_MMI_Input_Api_GetPointerLocation_0100
     * @tc.desc: OH_Input_GetPointerLocation Interface testing
     * @tc.level 0
     * @tc.type: Function
     * @tc.size: SmallTest
     */
    it('SUB_MMI_Input_Api_GetPointerLocation_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0, async (done: Function) => {
      let caseName = 'SUB_MMI_Input_Api_GetPointerLocation_0100';
      console.info(`${caseName} start to test OH_Input_GetPointerLocation`);
      try {
        let data = mouse.GetPointerLocation1();
        console.info(`${caseName} success, test NAPI data= ${JSON.stringify(data)}`);
        ExpectTrue(data == 1);
        done();
      } catch (err) {
        console.info(`${caseName} fail, error is ${JSON.stringify(err)}`);
        ExpectFalse(true);
        done();
      }
      console.info(`${caseName} end to test OH_Input_GetPointerLocation`);
    });
  });
}