/*
* Copyright (c) 2024 SwanLink (Jiangsu) Technology Development Co., LTD.
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

import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect, TestType, Size, Level } from "@ohos/hypium"
import napitest from 'libnapitestex.so'

async function sleep(time: number) {
  return new Promise((resolve: (value: string) => void) => {
    setTimeout(() => {
      resolve("ok")
    }, time)
  }).then(() => {
    console.info(`sleep ${time} over...`)
  })
}

export default function napiApiTestEx() {
  describe('napiApiTestEx', () => {
    /**
     * run after testcase
     */
    afterEach(async () => {
      console.info('[napiApiTestEx] after each called')
    });

    /**
    * @tc.number    SUB_ACE_BASIC_ETS_NAPI_EX_0001
    * @tc.name      napiTestEx0001
    * @tc.desc      napiTestEx
    */
    it('napiTestEx0001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let buffer: Object = napitest.CreateBufferCopy();
      expect(napitest.IsBuffer(buffer)).assertTrue();
      expect(napitest.GetBufferInfo(buffer)).assertTrue();
    });
  })
}
