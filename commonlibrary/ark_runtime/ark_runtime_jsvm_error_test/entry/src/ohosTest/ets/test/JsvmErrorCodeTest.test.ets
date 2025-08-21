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

import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect, TestType, Size, Level} from "@ohos/hypium"

import napitest from 'libentry.so'

function sleep(time: number) {
  return new Promise<void>((resolve, reject) => {
    setTimeout(() => {
      resolve()
    }, time)
  }).then(() => {
    console.info( `sleep ${time} over...`)
  })
}

export default function JsvmJsunit() {

  describe('JsvmTest',() => {
    const empty = 'hh';
    let value: string;

    /**
    * run after testcase
    */
    afterEach(async (done:Function) => {
      console.info('[JsvmTest] after each called')
      value == '';
      await sleep(50);
      done();
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0001
     * @tc.name      JsvmErrorCodeTest001
     * @tc.desc      CreateVM,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testCreateVMJSOK();
      console.info('JsvmErrorCodeTest001 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0002
     * @tc.name      JsvmErrorCodeTest002
     * @tc.desc      OpenVMScope,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testOpenVMScopeJSOK();
      console.info('JsvmErrorCodeTest002 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0003
     * @tc.name      JsvmErrorCodeTest003
     * @tc.desc      CloseVMScope,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest003', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testCloseVMScopeJSOK();
      console.info('JsvmErrorCodeTest003 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0004
     * @tc.name      JsvmErrorCodeTest004
     * @tc.desc      CreateProxy,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest004', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testCreateProxyJSOK();
      console.info('JsvmErrorCodeTest004 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });
    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0005
     * @tc.name      JsvmErrorCodeTest005
     * @tc.desc      CreateProxy if the any of the input arguments is NULL,return JSVM_INVALID_ARG
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest005', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testCreateProxyInvalid();
      console.info('JsvmErrorCodeTest005 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0006
     * @tc.name      JsvmErrorCodeTest006
     * @tc.desc      CreateProxy if target or handler is not Javascript Object,return JSVM_OBJECT_EXPECTED
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest006', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testCreateProxyObejct();
      console.info('JsvmErrorCodeTest006 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0007
     * @tc.name      JsvmErrorCodeTest007
     * @tc.desc      CreateProxy if an exception occurs,return JSVM_PENDING_EXCEPTION
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest007', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      try {
        let rst:number = napitest.testCreateProxyPending();
        console.info('JsvmErrorCodeTest007 testString result is:' + JSON.stringify(rst));
        expect(rst).assertEqual(0);
      }catch (e){
        expect(false).assertEqual(true);
        console.info('JsvmErrorCodeTest007 testString result is:' + JSON.stringify(e));
      }
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0008
     * @tc.name      JsvmErrorCodeTest008
     * @tc.desc      CreateEnv,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest008', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testCreateEnvJSOK();
      console.info('JsvmErrorCodeTest008 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);


    });
    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0009
     * @tc.name      JsvmErrorCodeTest009
     * @tc.desc      CreateEnvFromSnapshot,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest009', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testCreateEnvFromSnapshotJSOK();
      console.info('JsvmErrorCodeTest009 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);

    });
    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0010
     * @tc.name      JsvmErrorCodeTest010
     * @tc.desc      DestroyEnv,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest010', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testDestroyEnvJSOK();
      console.info('JsvmErrorCodeTest010 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);

    });
    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0011
     * @tc.name      JsvmErrorCodeTest011
     * @tc.desc      OpenEnvScope,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest011', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testOpenEnvScopeJSOK();
      console.info('JsvmErrorCodeTest011 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0012
     * @tc.name      JsvmErrorCodeTest012
     * @tc.desc      CloseEnvScope,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest012', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testCloseEnvScopeJSOK();
      console.info('JsvmErrorCodeTest012 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0013
     * @tc.name      JsvmErrorCodeTest013
     * @tc.desc      GetVM,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest013', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testGetVMJSOK();
      console.info('JsvmErrorCodeTest013 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0014
     * @tc.name      JsvmErrorCodeTest014
     * @tc.desc      IsProxy,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest014', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testIsProxyJSOK();
      console.info('JsvmErrorCodeTest014 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0015
     * @tc.name      JsvmErrorCodeTest015
     * @tc.desc      IsProxy,return JSVM_INVALID_ARG
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest015', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testIsProxyInvalid();
      console.info('JsvmErrorCodeTest015 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0016
     * @tc.name      JsvmErrorCodeTest016
     * @tc.desc      ProxyGetTarget,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest016', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testProxyGetTargetJSOK();
      console.info('JsvmErrorCodeTest016 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0017
     * @tc.name      JsvmErrorCodeTest017
     * @tc.desc      ProxyGetTarget,return JSVM_INVALID_ARG
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest017', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testProxyGetTargetInvalid();
      console.info('JsvmErrorCodeTest017 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0018
     * @tc.name      JsvmErrorCodeTest018
     * @tc.desc      ProxyGetTarget,return JSVM_INVALID_TYPE
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest018', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testProxyGetTargetInvalidType();
      console.info('JsvmErrorCodeTest018 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0019
     * @tc.name      JsvmErrorCodeTest019
     * @tc.desc      CompileScript,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest019', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testCompileScriptJSOK();
      console.info('JsvmErrorCodeTest019 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0020
     * @tc.name      JsvmErrorCodeTest020
     * @tc.desc      CreateCodeCache,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest020', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testCreateCodeCacheJSOK();
      console.info('JsvmErrorCodeTest020 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0021
     * @tc.name      JsvmErrorCodeTest021
     * @tc.desc      OH_JSVM_Throw,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest021', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      try {
        let rst: number = napitest.testThrowJSOK();
        console.info('JsvmErrorCodeTest021 testString result is:' + JSON.stringify(rst));
        expect(rst).assertEqual(0);
      }catch (e) {
        expect(false).assertEqual(true);
      }
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0022
     * @tc.name      JsvmErrorCodeTest022
     * @tc.desc      OpenHandleScope,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest022', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testOpenHandleScopeJSOK();
      console.info('JsvmErrorCodeTest022 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0023
     * @tc.name      JsvmErrorCodeTest023
     * @tc.desc      OpenEscapableHandleScope,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest023', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testOpenEscapableHandleScopeJSOK();
      console.info('JsvmErrorCodeTest023 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0024
     * @tc.name      JsvmErrorCodeTest024
     * @tc.desc      CloseEscapableHandleScope,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest024', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testCloseEscapableHandleScopeJSOK();
      console.info('JsvmErrorCodeTest024 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0025
     * @tc.name      JsvmErrorCodeTest025
     * @tc.desc      GetReferenceValue,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest025', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testGetReferenceValueJSOK();
      console.info('JsvmErrorCodeTest025 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0026
     * @tc.name      JsvmErrorCodeTest026
     * @tc.desc      CreateArray,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest026', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testCreateArrayJSOK();
      console.info('JsvmErrorCodeTest026 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0027
     * @tc.name      JsvmErrorCodeTest027
     * @tc.desc      AllocateArrayBufferBackingStoreData,return JSVM_INVALID_ARG
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest027', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testAllocateArrayBufferBackingStoreDataInvalid();
      console.info('JsvmErrorCodeTest027 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0028
     * @tc.name      JsvmErrorCodeTest028
     * @tc.desc      AllocateArrayBufferBackingStoreData,return JSVM_GENERIC_FAILURE
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest028', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testAllocateArrayBufferBackingStoreDataGenericFailure();
      console.info('JsvmErrorCodeTest028 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0029
     * @tc.name      JsvmErrorCodeTest029
     * @tc.desc      FreeArrayBufferBackingStoreData,return JSVM_INVALID_ARG
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest029', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testFreeArrayBufferBackingStoreDataInvalid();
      console.info('JsvmErrorCodeTest029 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0030
     * @tc.name      JsvmErrorCodeTest030
     * @tc.desc      CreateArrayBufferFromBackingStoreData,return JSVM_INVALID_ARG
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest030', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testCreateArrayBufferFromBackingStoreDataInvalid();
      console.info('JsvmErrorCodeTest030 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0031
     * @tc.name      JsvmErrorCodeTest031
     * @tc.desc      CreateDate,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest031', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testCreateDateJSOK();
      console.info('JsvmErrorCodeTest031 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0032
     * @tc.name      JsvmErrorCodeTest032
     * @tc.desc      CreateExternal,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest032', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testCreateExternalJSOK();
      console.info('JsvmErrorCodeTest032 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0033
     * @tc.name      JsvmErrorCodeTest033
     * @tc.desc      CreateSymbol,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest033', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testCreateSymbolJSOK();
      console.info('JsvmErrorCodeTest033 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0034
     * @tc.name      JsvmErrorCodeTest034
     * @tc.desc      CreateBigintInt64,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest034', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testCreateBigintInt64JSOK();
      console.info('JsvmErrorCodeTest034 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0035
     * @tc.name      JsvmErrorCodeTest035
     * @tc.desc      GetArrayLength,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest035', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testGetArrayLengthJSOK();
      console.info('JsvmErrorCodeTest035 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0036
     * @tc.name      JsvmErrorCodeTest036
     * @tc.desc      GetPrototype,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest036', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testGetPrototypeJSOK();
      console.info('JsvmErrorCodeTest036 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0037
     * @tc.name      JsvmErrorCodeTest037
     * @tc.desc      GetDateValue,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest037', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testGetDateValueJSOK();
      console.info('JsvmErrorCodeTest037 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0038
     * @tc.name      JsvmErrorCodeTest038
     * @tc.desc      GetDateValue,return JSVM_DATE_EXPECTED
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest038', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testGetDateValueExpected();
      console.info('JsvmErrorCodeTest038 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0039
     * @tc.name      JsvmErrorCodeTest039
     * @tc.desc      GetGetValueBool,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest039', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testGetValueBoolJSOK();
      console.info('JsvmErrorCodeTest039 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0040
     * @tc.name      JsvmErrorCodeTest040
     * @tc.desc      GetValueDouble,return JSVM_NUMBER_EXPECTED
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest040', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testGetValueDoubleExpected();
      console.info('JsvmErrorCodeTest040 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0041
     * @tc.name      JsvmErrorCodeTest041
     * @tc.desc      GetValueBigintInt64,return JSVM_BIGINT_EXPECTED
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest041', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testGetValueBigintInt64Expected();
      console.info('JsvmErrorCodeTest041 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0042
     * @tc.name      JsvmErrorCodeTest042
     * @tc.desc      GetValueBigintUint64,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest042', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testGetValueBigintUint64JSOK();
      console.info('JsvmErrorCodeTest042 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0043
     * @tc.name      JsvmErrorCodeTest043
     * @tc.desc      GetValueBigintUint64,return JSVM_BIGINT_EXPECTED
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest043', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testGetValueBigintUint64Expected();
      console.info('JsvmErrorCodeTest043 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0044
     * @tc.name      JsvmErrorCodeTest044
     * @tc.desc      GetValueExternal,return JSVM_INVALID_ARG
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest044', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testGetValueExternalInvalid();
      console.info('JsvmErrorCodeTest044 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0045
     * @tc.name      JsvmErrorCodeTest045
     * @tc.desc      GetValueInt32,return JSVM_NUMBER_EXPECTED
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest045', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testGetValueInt32Expected();
      console.info('JsvmErrorCodeTest045 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0046
     * @tc.name      JsvmErrorCodeTest046
     * @tc.desc      GetValueInt64,return JSVM_NUMBER_EXPECTED
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest046', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testGetValueInt64Expected();
      console.info('JsvmErrorCodeTest046 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0047
     * @tc.name      JsvmErrorCodeTest047
     * @tc.desc      GetValueStringLatin1,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest047', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testGetValueStringLatin1JSOK();
      console.info('JsvmErrorCodeTest047 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0048
     * @tc.name      JsvmErrorCodeTest048
     * @tc.desc      GetValueStringLatin1,return JSVM_NUMBER_EXPECTED
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest048', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testGetValueStringLatin1Expected();
      console.info('JsvmErrorCodeTest048 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0049
     * @tc.name      JsvmErrorCodeTest049
     * @tc.desc      GetValueStringUtf8,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest049', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testGetValueStringUtf8JSOK();
      console.info('JsvmErrorCodeTest049 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0050
     * @tc.name      JsvmErrorCodeTest050
     * @tc.desc      GetValueStringUtf8,return JSVM_NUMBER_EXPECTED
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest050', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testGetValueStringUtf8Expected();
      console.info('JsvmErrorCodeTest050 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0051
     * @tc.name      JsvmErrorCodeTest051
     * @tc.desc      GetValueStringUtf16,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest051', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testGetValueStringUtf16JSOK();
      console.info('JsvmErrorCodeTest051 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0052
     * @tc.name      JsvmErrorCodeTest052
     * @tc.desc      GetValueStringUtf16,return JSVM_NUMBER_EXPECTED
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest052', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testGetValueStringUtf16Expected();
      console.info('JsvmErrorCodeTest052 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0053
     * @tc.name      JsvmErrorCodeTest053
     * @tc.desc      GetValueUint32,return JSVM_OK
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest053', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testGetValueUint32JSOK();
      console.info('JsvmErrorCodeTest053 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

    /**
     * @tc.number    SUB_ACE_ERROR_ETS_JSVM_0054
     * @tc.name      JsvmErrorCodeTest054
     * @tc.desc      GetValueUint32,return JSVM_NUMBER_EXPECTED
     * @tc.type      Function
     * @tc.level     Level 1
     */
    it('JsvmErrorCodeTest054', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let rst: number = napitest.testGetValueUint32Expected();
      console.info('JsvmErrorCodeTest054 testString result is:' + JSON.stringify(rst));
      expect(rst).assertEqual(0);
    });

  });
}
