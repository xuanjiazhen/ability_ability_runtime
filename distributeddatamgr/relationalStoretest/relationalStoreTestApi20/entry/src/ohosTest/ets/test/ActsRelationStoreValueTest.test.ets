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

export default function ActsRelationStoreValueTest() {
  describe('ActsRelationStoreValueTest', () => {
    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_TYPE_UNLIMITED_INT_0100
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_TYPE_UNLIMITED_INT_0100
     * @tc.desc    : test enumeration value TYPE_UNLIMITED_INT
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_TYPE_UNLIMITED_INT_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_TYPE_UNLIMITED_INT_0100 start====");
          let result: number = testNapi.testEnumerate();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_TYPE_UNLIMITED_INT_0100 result====", result);
          expect(result).assertEqual(0);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_TYPE_UNLIMITED_INT_0100 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_Destroy_0200
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_Destroy_0200
     * @tc.desc    : test OH_Value_Destroy Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_Destroy_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_Destroy_0200 start====");
          let result: number = testNapi.destroy();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_Destroy_0200 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_Destroy_0200 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_PutNull_0300
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_PutNull_0300
     * @tc.desc    : test OH_Value_PutNull Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_PutNull_0300',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_PutNull_0300 start====");
          let result: number = testNapi.putNull();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_PutNull_0300 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_PutNull_0300 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_PutReal_0400
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_PutReal_0400
     * @tc.desc    : test OH_Value_PutReal Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_PutReal_0400',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_PutReal_0400 start====");
          let result: number = testNapi.putReal();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_PutReal_0400 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_PutReal_0400 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_PutText_0500
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_PutText_0500
     * @tc.desc    : test OH_Value_PutText Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_PutText_0500',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_PutText_0500 start====");
          let result: number = testNapi.putText();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_PutText_0500 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_PutText_0500 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_PutBlob_0600
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_PutBlob_0600
     * @tc.desc    : test OH_Value_PutBlob Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_PutBlob_0600',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_PutBlob_0600 start====");
          let result: number = testNapi.putBlob();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_PutBlob_0600 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_PutBlob_0600 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_PutAsset_0700
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_PutAsset_0700
     * @tc.desc    : test OH_Value_PutAsset Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_PutAsset_0700',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_PutAsset_0700 start====");
          let result: number = testNapi.putAsset();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_PutAsset_0700 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_PutAsset_0700 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_PutAssets_0800
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_PutAssets_0800
     * @tc.desc    : test OH_Value_PutAssets Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_PutAssets_0800',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_PutAssets_0800 start====");
          let result: number = testNapi.putAssets();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_PutAssets_0800 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_PutAssets_0800 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_PutUnlimitedInt_0900
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_PutUnlimitedInt_0900
     * @tc.desc    : test OH_Value_PutUnlimitedInt Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_PutUnlimitedInt_0900',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_PutUnlimitedInt_0900 start====");
          let result: number = testNapi.putUnlimitedInt();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_PutUnlimitedInt_0900 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_PutUnlimitedInt_0900 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetType_1000
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetType_1000
     * @tc.desc    : test OH_Value_GetType Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetType_1000',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetType_1000 start====");
          let result: number = testNapi.getType();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetType_1000 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetType_1000 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_IsNull_1100
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_IsNull_1100
     * @tc.desc    : test OH_Value_IsNull Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_IsNull_1100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_IsNull_1100 start====");
          let result: number = testNapi.isNull();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_IsNull_1100 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_IsNull_1100 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetInt_1200
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetInt_1200
     * @tc.desc    : test OH_Value_GetInt Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetInt_1200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetInt_1200 start====");
          let result: number = testNapi.testGetIntInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetInt_1200 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetInt_1200 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetInt_1300
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetInt_1300
     * @tc.desc    : test OH_Value_GetInt Test Data Types Do Not Match Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetInt_1300',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetInt_1300 start====");
          let result: number = testNapi.testGetIntTypeMismatch();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetInt_1300 result====", result);
          expect(result).assertEqual(RDB_E_TYPE_MISMATCH);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetInt_1300 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetInt_1400
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetInt_1400
     * @tc.desc    : test OH_Value_GetInt Test Storage Data Is Empty Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetInt_1400',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetInt_1400 start====");
          let result: number = testNapi.testGetIntDataTypeNull();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetInt_1400 result====", result);
          expect(result).assertEqual(RDB_E_DATA_TYPE_NULL);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetInt_1400 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetReal_1500
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetReal_1500
     * @tc.desc    : test OH_Value_GetReal Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetReal_1500',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetReal_1500 start====");
          let result: number = testNapi.testGetRealInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetReal_1500 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetReal_1500 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetText_1600
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetText_1600
     * @tc.desc    : test OH_Value_GetText Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetText_1600',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetText_1600 start====");
          let result: number = testNapi.testGetTextInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetText_1600 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetText_1600 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetText_1700
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetText_1700
     * @tc.desc    : test OH_Value_GetText Test Data Types Do Not Match Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetText_1700',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetText_1700 start====");
          let result: number = testNapi.testGetTextTypeMismatch();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetText_1700 result====", result);
          expect(result).assertEqual(RDB_E_TYPE_MISMATCH);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetText_1700 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetText_1800
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetText_1800
     * @tc.desc    : test OH_Value_GetText Test Storage Data Is Empty Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetText_1800',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetText_1800 start====");
          let result: number = testNapi.testGetTextDataTypeNull();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetText_1800 result====", result);
          expect(result).assertEqual(RDB_E_DATA_TYPE_NULL);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetText_1800 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetBlob_1900
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetBlob_1900
     * @tc.desc    : test OH_Value_GetBlob Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetBlob_1900',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetBlob_1900 start====");
          let result: number = testNapi.testGetBlobInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetBlob_1900 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetBlob_1900 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetBlob_2000
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetBlob_2000
     * @tc.desc    : test OH_Value_GetBlob Test Data Types Do Not Match Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetBlob_2000',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetBlob_2000 start====");
          let result: number = testNapi.testGetBlobTypeMismatch();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetBlob_2000 result====", result);
          expect(result).assertEqual(RDB_E_TYPE_MISMATCH);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetBlob_2000 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetBlob_2100
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetBlob_2100
     * @tc.desc    : test OH_Value_GetBlob Test Storage Data Is Empty Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetBlob_2100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetBlob_2100 start====");
          let result: number = testNapi.testGetBlobDataTypeNull();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetBlob_2100 result====", result);
          expect(result).assertEqual(RDB_E_DATA_TYPE_NULL);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetBlob_2100 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAsset_2200
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAsset_2200
     * @tc.desc    : test OH_Value_GetAsset Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAsset_2200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAsset_2200 start====");
          let result: number = testNapi.testGetAssetInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAsset_2200 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAsset_2200 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAsset_2300
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAsset_2300
     * @tc.desc    : test OH_Value_GetAsset Test Data Types Do Not Match Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAsset_2300',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAsset_2300 start====");
          let result: number = testNapi.testGetAssetTypeMismatch();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAsset_2300 result====", result);
          expect(result).assertEqual(RDB_E_TYPE_MISMATCH);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAsset_2300 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAsset_2400
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAsset_2400
     * @tc.desc    : test OH_Value_GetAsset Test Storage Data Is Empty Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAsset_2400',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAsset_2400 start====");
          let result: number = testNapi.testGetAssetDataTypeNull();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAsset_2400 result====", result);
          expect(result).assertEqual(RDB_E_DATA_TYPE_NULL);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAsset_2400 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssetsCount_2500
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssetsCount_2500
     * @tc.desc    : test OH_Value_GetAssetsCount Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssetsCount_2500',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssetsCount_2500 start====");
          let result: number = testNapi.testGetAssetsCountInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssetsCount_2500 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssetsCount_2500 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssetsCount_2600
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssetsCount_2600
     * @tc.desc    : test OH_Value_GetAssetsCount Test Data Types Do Not Match Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssetsCount_2600',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssetsCount_2600 start====");
          let result: number = testNapi.testGetAssetsCountTypeMismatch();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssetsCount_2600 result====", result);
          expect(result).assertEqual(RDB_E_TYPE_MISMATCH);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssetsCount_2600 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssetsCount_2700
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssetsCount_2700
     * @tc.desc    : test OH_Value_GetAssetsCount Test Storage Data Is Empty Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssetsCount_2700',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssetsCount_2700 start====");
          let result: number = testNapi.testGetAssetsCountDataTypeNull();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssetsCount_2700 result====", result);
          expect(result).assertEqual(RDB_E_DATA_TYPE_NULL);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssetsCount_2700 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssets_2800
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssets_2800
     * @tc.desc    : test OH_Value_GetAssets Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssets_2800',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssets_2800 start====");
          let result: number = testNapi.testGetAssetsInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssets_2800 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssets_2800 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssets_2900
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssets_2900
     * @tc.desc    : test OH_Value_GetAssets Test Data Types Do Not Match Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssets_2900',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssets_2900 start====");
          let result: number = testNapi.testGetAssetsTypeMismatch();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssets_2900 result====", result);
          expect(result).assertEqual(RDB_E_TYPE_MISMATCH);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssets_2900 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssets_3000
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssets_3000
     * @tc.desc    : test OH_Value_GetAssets Test Storage Data Is Empty Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssets_3000',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssets_3000 start====");
          let result: number = testNapi.testGetAssetsDataTypeNull();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssets_3000 result====", result);
          expect(result).assertEqual(RDB_E_DATA_TYPE_NULL);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetAssets_3000 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVectorCount_3100
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVectorCount_3100
     * @tc.desc    : test OH_Value_GetFloatVectorCount Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVectorCount_3100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVectorCount_3100 start====");
          let result: number = testNapi.testGetFloatVectorCountInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVectorCount_3100 result====",
            result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVectorCount_3100 catch err: " +
            err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVectorCount_3200
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVectorCount_3200
     * @tc.desc    : test OH_Value_GetFloatVectorCount Test Data Types Do Not Match Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVectorCount_3200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVectorCount_3200 start====");
          let result: number = testNapi.testGetFloatVectorCountTypeMismatch();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVectorCount_3200 result====",
            result);
          expect(result).assertEqual(RDB_E_TYPE_MISMATCH);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVectorCount_3200 catch err: " +
            err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVectorCount_3300
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVectorCount_3300
     * @tc.desc    : test OH_Value_GetFloatVectorCount Test Storage Data Is Empty Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVectorCount_3300',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVectorCount_3300 start====");
          let result: number = testNapi.testGetFloatVectorCountTypeNull();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVectorCount_3300 result====",
            result);
          expect(result).assertEqual(RDB_E_DATA_TYPE_NULL);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVectorCount_3300 catch err: " +
            err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVector_3400
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVector_3400
     * @tc.desc    : test OH_Value_GetFloatVector Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVector_3400',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVector_3400 start====");
          let result: number = testNapi.testGetFloatVectorInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVector_3400 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVector_3400 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVector_3500
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVector_3500
     * @tc.desc    : test OH_Value_GetFloatVector Test Data Types Do Not Match Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVector_3500',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVector_3500 start====");
          let result: number = testNapi.testGetFloatVectorTypeMismatch();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVector_3500 result====", result);
          expect(result).assertEqual(RDB_E_TYPE_MISMATCH);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVector_3500 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVector_3600
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVector_3600
     * @tc.desc    : test OH_Value_GetFloatVector Test Storage Data Is Empty Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVector_3600',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVector_3600 start====");
          let result: number = testNapi.testGetFloatVectorTypeNull();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVector_3600 result====", result);
          expect(result).assertEqual(RDB_E_DATA_TYPE_NULL);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetFloatVector_3600 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedIntBand_3700
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedIntBand_3700
     * @tc.desc    : test OH_Value_GetUnlimitedIntBand Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedIntBand_3700',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedIntBand_3700 start====");
          let result: number = testNapi.testGetUnlimitedIntBandInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedIntBand_3700 result====",
            result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedIntBand_3700 catch err: " +
            err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedIntBand_3800
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedIntBand_3800
     * @tc.desc    : test OH_Value_GetUnlimitedIntBand Test Data Types Do Not Match Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedIntBand_3800',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedIntBand_3800 start====");
          let result: number = testNapi.testGetUnlimitedIntBandTypeMismatch();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedIntBand_3800 result====",
            result);
          expect(result).assertEqual(RDB_E_TYPE_MISMATCH);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedIntBand_3800 catch err: " +
            err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedIntBand_3900
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedIntBand_3900
     * @tc.desc    : test OH_Value_GetUnlimitedIntBand Test Storage Data Is Empty Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedIntBand_3900',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedIntBand_3900 start====");
          let result: number = testNapi.testGetUnlimitedIntBandTypeNull();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedIntBand_3900 result====",
            result);
          expect(result).assertEqual(RDB_E_DATA_TYPE_NULL);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedIntBand_3900 catch err: " +
            err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedInt_4000
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedInt_4000
     * @tc.desc    : test OH_Value_GetUnlimitedInt Test Illegal Parameter Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedInt_4000',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedInt_4000 start====");
          let result: number = testNapi.testGetUnlimitedIntInvalidParam();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedInt_4000 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_ARGS);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedInt_4000 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedInt_4100
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedInt_4100
     * @tc.desc    : test OH_Value_GetUnlimitedInt Test Data Types Do Not Match Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedInt_4100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedInt_4100 start====");
          let result: number = testNapi.testGetUnlimitedIntTypeMismatch();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedInt_4100 result====", result);
          expect(result).assertEqual(RDB_E_TYPE_MISMATCH);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedInt_4100 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedInt_4200
     * @tc.name    : SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedInt_4200
     * @tc.desc    : test OH_Value_GetUnlimitedInt Test Storage Data Is Empty Scene
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedInt_4200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedInt_4200 start====");
          let result: number = testNapi.testGetUnlimitedIntTypeNull();
          console.info("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedInt_4200 result====", result);
          expect(result).assertEqual(RDB_E_DATA_TYPE_NULL);
          done();
        } catch (err) {
          console.error("====>SUB_DistributedData_RelationalStore_SDK_OH_Value_GetUnlimitedInt_4200 catch err: " + err);
        }
      })
  })
}
