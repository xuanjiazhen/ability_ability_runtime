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
import relationalstoretest from 'librelationalstoretest.so'

const RDB_OK = 0;

export default function ActsRelationalStoreTest() {
  describe('ActsRelationalStoreTest', () => {
    /**
     * @tc.number  : SUB_OH_Data_Asset_GetUri_0100
     * @tc.name    : SUB_OH_Data_Asset_GetUri_0100
     * @tc.desc    : test OH_Data_Asset_GetUri Test Normal Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_Data_Asset_GetUri_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_Data_Asset_GetUri_0100 start====");
        let result: number = relationalstoretest.getURISuccess();
        console.info("====>SUB_OH_Data_Asset_GetUri_0100 result====", result);
        expect(result).assertEqual(RDB_OK);
        done();
      } catch (err) {
        console.error("====>SUB_OH_Data_Asset_GetUri_0100 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_Data_Asset_GetPath_0100
     * @tc.name    : SUB_OH_Data_Asset_GetPath_0100
     * @tc.desc    : test OH_Data_Asset_GetPath Test Normal Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_Data_Asset_GetPath_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_Data_Asset_GetPath_0100 start====");
        let result: number = relationalstoretest.getPathSuccess();
        console.info("====>SUB_OH_Data_Asset_GetPath_0100 result====", result);
        expect(result).assertEqual(RDB_OK);
        done();
      } catch (err) {
        console.error("====>SUB_OH_Data_Asset_GetPath_0100 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_Data_Asset_GetCreateTime_0100
     * @tc.name    : SUB_OH_Data_Asset_GetCreateTime_0100
     * @tc.desc    : test OH_Data_Asset_GetCreateTime Test Normal Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_Data_Asset_GetCreateTime_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_OH_Data_Asset_GetCreateTime_0100 start====");
          let result: number = relationalstoretest.getCreateTimeSuccess();
          console.info("====>SUB_OH_Data_Asset_GetCreateTime_0100 result====", result);
          expect(result).assertEqual(RDB_OK);
          done();
        } catch (err) {
          console.error("====>SUB_OH_Data_Asset_GetCreateTime_0100 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_OH_Data_Asset_GetModifyTime_0100
     * @tc.name    : SUB_OH_Data_Asset_GetModifyTime_0100
     * @tc.desc    : test OH_Data_Asset_GetModifyTime Test Normal Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_Data_Asset_GetModifyTime_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_OH_Data_Asset_GetModifyTime_0100 start====");
          let result: number = relationalstoretest.getModifyTimeSuccess();
          console.info("====>SUB_OH_Data_Asset_GetModifyTime_0100 result====", result);
          expect(result).assertEqual(RDB_OK);
          done();
        } catch (err) {
          console.error("====>SUB_OH_Data_Asset_GetModifyTime_0100 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_OH_Data_Asset_GetSize_0100
     * @tc.name    : SUB_OH_Data_Asset_GetSize_0100
     * @tc.desc    : test OH_Data_Asset_GetSize Test Normal Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_Data_Asset_GetSize_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_OH_Data_Asset_GetSize_0100 start====");
        let result: number = relationalstoretest.getSizeSuccess();
        console.info("====>SUB_OH_Data_Asset_GetSize_0100 result====", result);
        expect(result).assertEqual(RDB_OK);
        done();
      } catch (err) {
        console.error("====>SUB_OH_Data_Asset_GetSize_0100 catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_OH_Data_Asset_GetStatus_0100
     * @tc.name    : SUB_OH_Data_Asset_GetStatus_0100
     * @tc.desc    : test OH_Data_Asset_GetStatus Test Normal Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_Data_Asset_GetStatus_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_OH_Data_Asset_GetStatus_0100 start====");
          let result: number = relationalstoretest.getStatusSuccess();
          console.info("====>SUB_OH_Data_Asset_GetStatus_0100 result====", result);
          expect(result).assertEqual(RDB_OK);
          done();
        } catch (err) {
          console.error("====>SUB_OH_Data_Asset_GetStatus_0100 catch err: " + err);
        }
      })
  })
}
