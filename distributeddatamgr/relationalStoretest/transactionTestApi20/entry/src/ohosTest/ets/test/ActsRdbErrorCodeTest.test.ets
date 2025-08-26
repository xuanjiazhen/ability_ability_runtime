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
import testNapi from 'libentry.so'

const RDB_ERR = -1;
const RDB_E_EMPTY_TABLE_NAME = 14800005;
const RDB_E_INVALID_COLUMN_TYPE = 14800009;
const RDB_E_NO_TRANSACTION_IN_SESSION = 14800016;
const RDB_E_NO_ROW_IN_QUERY = 14800018;

export default function ActsRdbErrorCodeTest() {
  describe('ActsRdbErrorCodeTest', () => {
    /**
     * @tc.number  : SUB_OH_Rdb_ErrCode_RDB_ERR_0100
     * @tc.name    : SUB_OH_Rdb_ErrCode_RDB_ERR_0100
     * @tc.desc    : The execution error scene in the test error code information.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_Rdb_ErrCode_RDB_ERR_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_OH_Rdb_ErrCode_RDB_ERR_0100 start====");
          let result: number = testNapi.testRdbError();
          console.info("====>SUB_OH_Rdb_ErrCode_RDB_ERR_0100 result====", result);
          expect(result).assertEqual(RDB_ERR);
          done();
        } catch (err) {
          console.error("====>SUB_OH_Rdb_ErrCode_RDB_ERR_0100 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_OH_Rdb_ErrCode_RDB_E_EMPTY_TABLE_NAME_0200
     * @tc.name    : SUB_OH_Rdb_ErrCode_RDB_E_EMPTY_TABLE_NAME_0200
     * @tc.desc    : Failure scenario in which the table name in the test error code information is empty.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_Rdb_ErrCode_RDB_E_EMPTY_TABLE_NAME_0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_OH_Rdb_ErrCode_RDB_E_EMPTY_TABLE_NAME_0200 start====");
          let result: number = testNapi.testEmptyTableName();
          console.info("====>SUB_OH_Rdb_ErrCode_RDB_E_EMPTY_TABLE_NAME_0200 result====", result);
          expect(result).assertEqual(RDB_E_EMPTY_TABLE_NAME);
          done();
        } catch (err) {
          console.error("====>SUB_OH_Rdb_ErrCode_RDB_E_EMPTY_TABLE_NAME_0200 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_OH_Rdb_ErrCode_RDB_E_INVALID_COLUMN_TYPE_0300
     * @tc.name    : SUB_OH_Rdb_ErrCode_RDB_E_INVALID_COLUMN_TYPE_0300
     * @tc.desc    : Test the fault scenario where the column type in the error code information is illegal.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_Rdb_ErrCode_RDB_E_INVALID_COLUMN_TYPE_0300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_OH_Rdb_ErrCode_RDB_E_INVALID_COLUMN_TYPE_0300 start====");
          let result: number = testNapi.testInvalidColumnType();
          console.info("====>SUB_OH_Rdb_ErrCode_RDB_E_INVALID_COLUMN_TYPE_0300 result====", result);
          expect(result).assertEqual(RDB_E_INVALID_COLUMN_TYPE);
          done();
        } catch (err) {
          console.error("====>SUB_OH_Rdb_ErrCode_RDB_E_INVALID_COLUMN_TYPE_0300 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_OH_Rdb_ErrCode_RDB_E_NO_ROW_IN_QUERY_0400
     * @tc.name    : SUB_OH_Rdb_ErrCode_RDB_E_NO_ROW_IN_QUERY_0400
     * @tc.desc    : There is no recorded fault scenario in the result set obtained by testing the query in the error code information.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_Rdb_ErrCode_RDB_E_NO_ROW_IN_QUERY_0400', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_OH_Rdb_ErrCode_RDB_E_NO_ROW_IN_QUERY_0400 start====");
          let result: number = testNapi.testNoRowInQuery();
          console.info("====>SUB_OH_Rdb_ErrCode_RDB_E_NO_ROW_IN_QUERY_0400 result====", result);
          expect(result).assertEqual(RDB_E_NO_ROW_IN_QUERY);
          done();
        } catch (err) {
          console.error("====>SUB_OH_Rdb_ErrCode_RDB_E_NO_ROW_IN_QUERY_0400 catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_OH_Rdb_ErrCode_RDB_E_NO_TRANSACTION_IN_SESSION_0500
     * @tc.name    : SUB_OH_Rdb_ErrCode_RDB_E_NO_TRANSACTION_IN_SESSION_0500
     * @tc.desc    : Test the fault scenario in the error code information that there is no open transaction in the database session.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_Rdb_ErrCode_RDB_E_NO_TRANSACTION_IN_SESSION_0500', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_OH_Rdb_ErrCode_RDB_E_NO_TRANSACTION_IN_SESSION_0500 start====");
          let result: number = testNapi.testNoTransactionInSession();
          console.info("====>SUB_OH_Rdb_ErrCode_RDB_E_NO_TRANSACTION_IN_SESSION_0500 result====", result);
          expect(result).assertEqual(RDB_E_NO_TRANSACTION_IN_SESSION);
          done();
        } catch (err) {
          console.error("====>SUB_OH_Rdb_ErrCode_RDB_E_NO_TRANSACTION_IN_SESSION_0500 catch err: " + err);
        }
      })
  })
}
