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
import testNapi from 'librelationNdk.so'

const RDB_E_INVALID_ARGS = 14800001;
const RDB_E_DATA_TYPE_NULL = 14800063;
const RDB_E_TYPE_MISMATCH = 14800064;

export default function ActsRelationStoreValuesTest() {
  describe('ActsRelationStoreValuesTest', () => {
    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_Destroy_0100
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_Destroy_0100
     * @tc.desc    : test OH_Values_Destroy Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_Destroy_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_Destroy_0100 start====");
          let result: number = testNapi.testDestroyInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_Destroy_0100 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_Destroy_0100 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_PutNull_0200
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_PutNull_0200
     * @tc.desc    : test OH_Values_PutNull Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_PutNull_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutNull_0200 start====");
          let result: number = testNapi.testPutNullInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutNull_0200 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutNull_0200 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_PutInt_0300
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_PutInt_0300
     * @tc.desc    : test OH_Values_PutInt Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_PutInt_0300',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutInt_0300 start====");
          let result: number = testNapi.testPutIntInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutInt_0300 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutInt_0300 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_PutReal_0400
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_PutReal_0400
     * @tc.desc    : test OH_Values_PutReal Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_PutReal_0400',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutReal_0400 start====");
          let result: number = testNapi.testPutRealInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutReal_0400 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutReal_0400 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_PutText_0500
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_PutText_0500
     * @tc.desc    : test OH_Values_PutText Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_PutText_0500',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutText_0500 start====");
          let result: number = testNapi.testPutTextInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutText_0500 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutText_0500 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_PutBlob_0600
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_PutBlob_0600
     * @tc.desc    : test OH_Values_PutBlob Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_PutBlob_0600',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutBlob_0600 start====");
          let result: number = testNapi.testPutBlobInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutBlob_0600 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutBlob_0600 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_PutAsset_0700
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_PutAsset_0700
     * @tc.desc    : test OH_Values_PutAsset Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_PutAsset_0700',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutAsset_0700 start====");
          let result: number = testNapi.testPutAssetInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutAsset_0700 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutAsset_0700 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_PutAssets_0800
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_PutAssets_0800
     * @tc.desc    : test OH_Values_PutAssets Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_PutAssets_0800',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutAssets_0800 start====");
          let result: number = testNapi.testPutAssetsInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutAssets_0800 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutAssets_0800 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_PutUnlimitedInt_0900
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_PutUnlimitedInt_0900
     * @tc.desc    : test OH_Values_PutUnlimitedInt Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_PutUnlimitedInt_0900',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutUnlimitedInt_0900 start====");
          let result: number = testNapi.testPutUnlimitedIntInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutUnlimitedInt_0900 result====",
            result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_PutUnlimitedInt_0900 catch err: " +
            err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_Count_1000
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_Count_1000
     * @tc.desc    : test OH_Values_Count Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_Count_1000',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_Count_1000 start====");
          let result: number = testNapi.testCountInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_Count_1000 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_Count_1000 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetType_1100
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetType_1100
     * @tc.desc    : test OH_Values_GetType Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetType_1100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetType_1100 start====");
          let result: number = testNapi.testGetTypeInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetType_1100 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetType_1100 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_Get_1200
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_Get_1200
     * @tc.desc    : test OH_Values_Get Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_Get_1200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_Get_1200 start====");
          let result: number = testNapi.testGetInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_Get_1200 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_Get_1200 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_IsNull_1300
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_IsNull_1300
     * @tc.desc    : test OH_Values_IsNull Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_IsNull_1300',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_IsNull_1300 start====");
          let result: number = testNapi.testIsNullInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_IsNull_1300 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_IsNull_1300 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetInt_1400
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetInt_1400
     * @tc.desc    : test OH_Values_GetInt Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetInt_1400',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetInt_1400 start====");
          let result: number = testNapi.testGetIntInvalidParamA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetInt_1400 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetInt_1400 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetInt_1500
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetInt_1500
     * @tc.desc    : test OH_Values_GetInt Test Data Types Do Not Match Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetInt_1500',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetInt_1500 start====");
          let result: number = testNapi.testGetIntTypeMismatchA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetInt_1500 result====", result);
          expect(result).assertEqual(RDB_E_TYPE_MISMATCH);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetInt_1500 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetInt_1600
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetInt_1600
     * @tc.desc    : test OH_Values_GetInt Test Storage Data Is Empty Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetInt_1600',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetInt_1600 start====");
          let result: number = testNapi.testGetIntTypeNullA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetInt_1600 result====", result);
          expect(result).assertEqual(RDB_E_DATA_TYPE_NULL);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetInt_1600 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetReal_1700
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetReal_1700
     * @tc.desc    : test OH_Values_GetReal Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetReal_1700',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetReal_1700 start====");
          let result: number = testNapi.testGetRealInvalidParamA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetReal_1700 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetReal_1700 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetReal_1800
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetReal_1800
     * @tc.desc    : test OH_Values_GetReal Test Data Types Do Not Match Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetReal_1800',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetReal_1800 start====");
          let result: number = testNapi.testGetRealTypeMismatchA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetReal_1800 result====", result);
          expect(result).assertEqual(RDB_E_TYPE_MISMATCH);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetReal_1800 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetReal_1900
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetReal_1900
     * @tc.desc    : test OH_Values_GetReal Test Storage Data Is Empty Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetReal_1900',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetReal_1900 start====");
          let result: number = testNapi.testGetRealTypeNullA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetReal_1900 result====", result);
          expect(result).assertEqual(RDB_E_DATA_TYPE_NULL);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetReal_1900 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetText_2000
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetText_2000
     * @tc.desc    : test OH_Values_GetText Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetText_2000',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetText_2000 start====");
          let result: number = testNapi.testGetTextInvalidParamA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetText_2000 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetText_2000 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetText_2100
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetText_2100
     * @tc.desc    : test OH_Values_GetText Test Data Types Do Not Match Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetText_2100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetText_2100 start====");
          let result: number = testNapi.testGetTextTypeMismatchA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetText_2100 result====", result);
          expect(result).assertEqual(RDB_E_TYPE_MISMATCH);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetText_2100 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetText_2200
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetText_2200
     * @tc.desc    : test OH_Values_GetText Test Storage Data Is Empty Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetText_2200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetText_2200 start====");
          let result: number = testNapi.testGetTextTypeNullA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetText_2200 result====", result);
          expect(result).assertEqual(RDB_E_DATA_TYPE_NULL);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetText_2200 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetBlob_2300
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetBlob_2300
     * @tc.desc    : test OH_Values_GetBlob Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetBlob_2300',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetBlob_2300 start====");
          let result: number = testNapi.testGetBlobInvalidParamA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetBlob_2300 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetBlob_2300 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetBlob_2400
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetBlob_2400
     * @tc.desc    : test OH_Values_GetBlob Test Data Types Do Not Match Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetBlob_2400',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetBlob_2400 start====");
          let result: number = testNapi.testGetBlobTypeMismatchA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetBlob_2400 result====", result);
          expect(result).assertEqual(RDB_E_TYPE_MISMATCH);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetBlob_2400 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetBlob_2500
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetBlob_2500
     * @tc.desc    : test OH_Values_GetBlob Test Storage Data Is Empty Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetBlob_2500',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetBlob_2500 start====");
          let result: number = testNapi.testGetBlobTypeNullA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetBlob_2500 result====", result);
          expect(result).assertEqual(RDB_E_DATA_TYPE_NULL);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetBlob_2500 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAsset_2600
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAsset_2600
     * @tc.desc    : test OH_Values_GetAsset Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAsset_2600',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAsset_2600 start====");
          let result: number = testNapi.testGetAssetInvalidParamA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAsset_2600 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAsset_2600 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAsset_2700
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAsset_2700
     * @tc.desc    : test OH_Values_GetAsset Test Data Types Do Not Match Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAsset_2700',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAsset_2700 start====");
          let result: number = testNapi.testGetAssetTypeMismatchA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAsset_2700 result====", result);
          expect(result).assertEqual(RDB_E_TYPE_MISMATCH);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAsset_2700 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAsset_2800
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAsset_2800
     * @tc.desc    : test OH_Values_GetAsset Test Storage Data Is Empty Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAsset_2800',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {

        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAsset_2800 start====");
          let result: number = testNapi.testGetAssetTypeNullA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAsset_2800 result====", result);
          expect(result).assertEqual(RDB_E_DATA_TYPE_NULL);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAsset_2800 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssetsCount_2900
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssetsCount_2900
     * @tc.desc    : test OH_Values_GetAssetsCount Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssetsCount_2900',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssetsCount_2900 start====");
          let result: number = testNapi.testGetAssetsCountInvalidParamA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssetsCount_2900 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssetsCount_2900 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssetsCount_3000
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssetsCount_3000
     * @tc.desc    : test OH_Values_GetAssetsCount Test Data Types Do Not Match Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssetsCount_3000',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssetsCount_3000 start====");
          let result: number = testNapi.testGetAssetsCountTypeMismatchA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssetsCount_3000 result====", result);
          expect(result).assertEqual(RDB_E_TYPE_MISMATCH);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssetsCount_3000 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssetsCount_3100
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssetsCount_3100
     * @tc.desc    : test OH_Values_GetAssetsCount Test Storage Data Is Empty Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssetsCount_3100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssetsCount_3100 start====");
          let result: number = testNapi.testGetAssetsCountTypeNullA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssetsCount_3100 result====", result);
          expect(result).assertEqual(RDB_E_DATA_TYPE_NULL);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssetsCount_3100 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssets_3200
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssets_3200
     * @tc.desc    : test OH_Values_GetAssets Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssets_3200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssets_3200 start====");
          let result: number = testNapi.testGetAssetsInvalidParamA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssets_3200 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssets_3200 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssets_3300
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssets_3300
     * @tc.desc    : test OH_Values_GetAssets Test Data Types Do Not Match Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssets_3300',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssets_3300 start====");
          let result: number = testNapi.testGetAssetsTypeMismatchA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssets_3300 result====", result);
          expect(result).assertEqual(RDB_E_TYPE_MISMATCH);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssets_3300 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssets_3400
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssets_3400
     * @tc.desc    : test OH_Values_GetAssets Test Storage Data Is Empty Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssets_3400',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssets_3400 start====");
          let result: number = testNapi.testGetAssetsTypeNullA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssets_3400 result====", result);
          expect(result).assertEqual(RDB_E_DATA_TYPE_NULL);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetAssets_3400 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVectorCount_3500
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVectorCount_3500
     * @tc.desc    : test OH_Values_GetFloatVectorCount Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVectorCount_3500',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVectorCount_3500 start====");
          let result: number = testNapi.testGetFloatVectorCountInvalidParamA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVectorCount_3500 result====",
            result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVectorCount_3500 catch err: " +
            err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVectorCount_3600
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVectorCount_3600
     * @tc.desc    : test OH_Values_GetFloatVectorCount Test Data Types Do Not Match Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVectorCount_3600',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVectorCount_3600 start====");
          let result: number = testNapi.testGetFloatVectorCountTypeMismatchA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVectorCount_3600 result====",
            result);
          expect(result).assertEqual(RDB_E_TYPE_MISMATCH);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVectorCount_3600 catch err: " +
            err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVectorCount_3700
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVectorCount_3700
     * @tc.desc    : test OH_Values_GetFloatVectorCount Test Storage Data Is Empty Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVectorCount_3700',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVectorCount_3700 start====");
          let result: number = testNapi.testGetFloatVectorCountTypeNullA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVectorCount_3700 result====",
            result);
          expect(result).assertEqual(RDB_E_DATA_TYPE_NULL);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVectorCount_3700 catch err: " +
            err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVector_3800
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVector_3800
     * @tc.desc    : test OH_Values_GetFloatVector Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVector_3800',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVector_3800 start====");
          let result: number = testNapi.testGetFloatVectorInvalidParamA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVector_3800 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVector_3800 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVector_3900
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVector_3900
     * @tc.desc    : test OH_Values_GetFloatVector Test Data Types Do Not Match Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVector_3900',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVector_3900 start====");
          let result: number = testNapi.testGetFloatVectorTypeMismatchA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVector_3900 result====", result);
          expect(result).assertEqual(RDB_E_TYPE_MISMATCH);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVector_3900 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVector_4000
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVector_4000
     * @tc.desc    : test OH_Values_GetFloatVector Test Storage Data Is Empty Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVector_4000',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVector_4000 start====");
          let result: number = testNapi.testGetFloatVectorTypeNullA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVector_4000 result====", result);
          expect(result).assertEqual(RDB_E_DATA_TYPE_NULL);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetFloatVector_4000 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedIntBand_4100
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedIntBand_4100
     * @tc.desc    : test OH_Values_GetUnlimitedIntBand Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedIntBand_4100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedIntBand_4100 start====");
          let result: number = testNapi.testGetUnlimitedIntBandInvalidParamA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedIntBand_4100 result====",
            result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedIntBand_4100 catch err: " +
            err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedIntBand_4200
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedIntBand_4200
     * @tc.desc    : test OH_Values_GetUnlimitedIntBand Test Data Types Do Not Match Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedIntBand_4200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedIntBand_4200 start====");
          let result: number = testNapi.testGetUnlimitedIntBandTypeMismatchA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedIntBand_4200 result====",
            result);
          expect(result).assertEqual(RDB_E_TYPE_MISMATCH);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedIntBand_4200 catch err: " +
            err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedIntBand_4300
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedIntBand_4300
     * @tc.desc    : test OH_Values_GetUnlimitedIntBand Test Storage Data Is Empty Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedIntBand_4300',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedIntBand_4300 start====");
          let result: number = testNapi.testGetUnlimitedIntBandTypeNullA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedIntBand_4300 result====",
            result);
          expect(result).assertEqual(RDB_E_DATA_TYPE_NULL);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedIntBand_4300 catch err: " +
            err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedInt_4400
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedInt_4400
     * @tc.desc    : test OH_Values_GetUnlimitedInt Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedInt_4400',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedInt_4400 start====");
          let result: number = testNapi.testGetUnlimitedIntInvalidParamA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedInt_4400 result====",
            result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedInt_4400 catch err: " +
            err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedInt_4500
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedInt_4500
     * @tc.desc    : test OH_Values_GetUnlimitedInt Test Data Types Do Not Match Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedInt_4500',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedInt_4500 start====");
          let result: number = testNapi.testGetUnlimitedIntTypeMismatchA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedInt_4500 result====",
            result);
          expect(result).assertEqual(RDB_E_TYPE_MISMATCH);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedInt_4500 catch err: " +
            err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedInt_4600
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedInt_4600
     * @tc.desc    : test OH_Values_GetUnlimitedInt Test Storage Data Is Empty Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedInt_4600',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedInt_4600 start====");
          let result: number = testNapi.testGetUnlimitedIntTypeNullA();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedInt_4600 result====",
            result);
          expect(result).assertEqual(RDB_E_DATA_TYPE_NULL);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Values_GetUnlimitedInt_4600 catch err: " +
            err);
        }
      })
  })
}
