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

import { afterAll, afterEach, beforeAll, beforeEach, describe, expect, it, Level, Size, TestType } from '@ohos/hypium';
import {
  testApplyValidateArgs22,
  testConstructModifyInstance23,
  testDeepProxy1,
  testGetAccessorInvariant40,
  testGetInterceptArrayPush6,
  testGetLazyInitialization8,
  testGetReturnThis19,
  testGetSymbolIterator33,
  testGetVsGetPrototypeOf78,
  testHasForInLoop21,
  testNestedProxyThis2,
  testOwnKeysInvalidKeyType41,
  testOwnKeysJsonStringify14,
  testOwnKeysVsHas80,
  testProxyAccessControl88,
  testProxyAccessFrequency64,
  testProxyAdapter61,
  testProxyArrayAt90,
  testProxyArrayPop26,
  testProxyArraySplice27,
  testProxyAsyncFunction12,
  testProxyAsyncIterator43,
  testProxyBufferTransfer53,
  testProxyClassExtends56,
  testProxyClassStaticBlock55,
  testProxyClassStaticInheritance46,
  testProxyCommandPattern89,
  testProxyDataBinding87,
  testProxyDataValidator16,
  testProxyDataView52,
  testProxyDebounce38,
  testProxyDecorator60,
  testProxyErrorCapture69,
  testProxyErrorCause92,
  testProxyErrorRetry71,
  testProxyErrorStack28,
  testProxyGenerator13,
  testProxyHasInstance67,
  testProxyIntlFormatter29,
  testProxyLazyLoad65,
  testProxyLRUCache63,
  testProxyMapIterator72,
  testProxyMediator62,
  testProxyObjectHasOwn91,
  testProxyObserver17,
  testProxyPrivateMethod54,
  testProxyPromiseAll57,
  testProxyPromiseCatch44,
  testProxyPropertyCache37,
  testProxyProxyConstructor49,
  testProxyReadOnlyView18,
  testProxyRecursive95,
  testProxySandbox100,
  testProxySetAdd3,
  testProxyStructuredClone15,
  testProxySymbolSpecies68,
  testProxySyncAsyncError70,
  testProxyToStringTag66,
  testProxyUint8Array51,
  testReflectApplyLength35,
  testReflectDefineAccessor36,
  testReflectEnumerateLike9,
  testReflectGetPrototypeNull11,
  testReflectGetSymbolAccessor34,
  testReflectPreventExtensions10,
  testRevocableReleaseResource24,
  testRevokedProxyTypeof48,
  testSetArrayLengthLimit7,
  testSetPrototypeProperty20,
  testSetToPrimitive32,
  testSetVsDefineProperty79,
  testSetWritableInvariant42
} from './Proxy2';

function sleep(time: number) {
  return new Promise<void>((resolve) => setTimeout(resolve, time));
}


export default function proxy2Test() {
  describe('proxy2Test', () => {
    // Defines a test suite. Two parameters are supported: test suite name and test suite function.
    beforeAll(() => {
      // Presets an action, which is performed only once before all test cases of the test suite start.
      // This API supports only one parameter: preset action function.
    })
    beforeEach(async () => {
      await sleep(100);
      // Presets an action, which is performed before each unit test case starts.
      // The number of execution times is the same as the number of test cases defined by **it**.
      // This API supports only one parameter: preset action function.
    })
    afterEach(() => {
      // Presets a clear action, which is performed after each unit test case ends.
      // The number of execution times is the same as the number of test cases defined by **it**.
      // This API supports only one parameter: clear action function.
    })
    afterAll(() => {
      // Presets a clear action, which is performed after all test cases of the test suite end.
      // This API supports only one parameter: clear action function.
    })

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_0100
     * @tc.name proxy2Test001
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testDeepProxy1();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_0200
     * @tc.name proxy2Test002
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testNestedProxyThis2();
      expect(result).assertEqual(true);
    });
    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_0300
     * @tc.name proxy2Test003
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest003', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result: boolean = testProxySetAdd3();
      expect(result).assertEqual(false);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_0600
     * @tc.name proxy2Test006
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest006', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result: boolean = testGetInterceptArrayPush6();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_0700
     * @tc.name proxy2Test007
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest007', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testSetArrayLengthLimit7();
      expect(result).assertEqual(2);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_0800
     * @tc.name proxy2Test008
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest008', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result: boolean = testGetLazyInitialization8();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_0900
     * @tc.name proxy2Test009
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest009', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testReflectEnumerateLike9();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_1000
     * @tc.name proxy2Test010
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest010', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testReflectPreventExtensions10();
      expect(result).assertEqual(2);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_1100
     * @tc.name proxy2Test011
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest011', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testReflectGetPrototypeNull11();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_1200
     * @tc.name proxy2Test012
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest012', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async () => {
      let result = await testProxyAsyncFunction12();
      expect(result).assertEqual(1);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_1300
     * @tc.name proxy2Test013
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest013', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyGenerator13();
      expect(result).assertEqual(2);

    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_1400
     * @tc.name proxy2Test014
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest014', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testOwnKeysJsonStringify14();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_1500
     * @tc.name proxy2Test015
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest015', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyStructuredClone15();
      expect(result).assertEqual(1);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_1600
     * @tc.name proxy2Test016
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest016', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyDataValidator16();
      expect(result).assertEqual(1);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_1700
     * @tc.name proxy2Test017
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest017', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyObserver17();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_1800
     * @tc.name proxy2Test018
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest018', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyReadOnlyView18();
      expect(result).assertEqual(2);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_1900
     * @tc.name proxy2Test019
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest019', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testGetReturnThis19();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_2000
     * @tc.name proxy2Test020
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest020', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testSetPrototypeProperty20();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_2100
     * @tc.name proxy2Test021
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest021', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testHasForInLoop21();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_2200
     * @tc.name proxy2Test022
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest022', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testApplyValidateArgs22();
      expect(result).assertEqual(1);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_2300
     * @tc.name proxy2Test023
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest023', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testConstructModifyInstance23();
      expect(result).assertEqual(false);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_2400
     * @tc.name proxy2Test024
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest024', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testRevocableReleaseResource24();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_2600
     * @tc.name proxy2Test026
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest026', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyArrayPop26();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_2700
     * @tc.name proxy2Test027
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest027', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyArraySplice27();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_2800
     * @tc.name proxy2Test028
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest028', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyErrorStack28();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_2900
     * @tc.name proxy2Test029
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest029', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result: boolean = testProxyIntlFormatter29();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_3200
     * @tc.name proxy2Test032
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest032', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testSetToPrimitive32();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_3300
     * @tc.name proxy2Test033
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest033', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testGetSymbolIterator33();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_3400
     * @tc.name proxy2Test034
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest034', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testReflectGetSymbolAccessor34();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_3500
     * @tc.name proxy2Test035
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest035', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testReflectApplyLength35();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_3600
     * @tc.name proxy2Test036
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest036', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testReflectDefineAccessor36();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_3700
     * @tc.name proxy2Test037
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest037', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyPropertyCache37();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_3800
     * @tc.name proxy2Test038
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest038', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async () => {
      let result = await testProxyDebounce38();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_4000
     * @tc.name proxy2Test040
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest040', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testGetAccessorInvariant40();
      expect(result).assertEqual(1);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_4100
     * @tc.name proxy2Test041
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest041', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testOwnKeysInvalidKeyType41();
      expect(result).assertEqual(1);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_4200
     * @tc.name proxy2Test042
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest042', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testSetWritableInvariant42();
      expect(result).assertEqual(1);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_4300
     * @tc.name proxy2Test043
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest043', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async () => {
      let result = await testProxyAsyncIterator43();
      expect(result).assertEqual(1);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_4400
     * @tc.name proxy2Test044
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest044', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyPromiseCatch44();
      expect(result).assertEqual(0);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_4600
     * @tc.name proxy2Test046
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest046', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyClassStaticInheritance46();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_4800
     * @tc.name proxy2Test048
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest048', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testRevokedProxyTypeof48();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_4900
     * @tc.name proxy2Test049
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest049', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyProxyConstructor49();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_5100
     * @tc.name proxy2Test051
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest051', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyUint8Array51();
      expect(result).assertEqual(1);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_5200
     * @tc.name proxy2Test052
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest052', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyDataView52();
      expect(result).assertEqual(false);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_5300
     * @tc.name proxy2Test053
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest053', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyBufferTransfer53();
      expect(result).assertEqual(1);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_5400
     * @tc.name proxy2Test054
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest054', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyPrivateMethod54();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_5500
     * @tc.name proxy2Test055
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest055', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyClassStaticBlock55();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_5600
     * @tc.name proxy2Test056
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest056', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyClassExtends56();
      expect(result).assertEqual(1);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_5700
     * @tc.name proxy2Test057
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest057', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async () => {
      let result = await testProxyPromiseAll57();
      expect(result).assertEqual(1);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_6000
     * @tc.name proxy2Test060
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest060', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyDecorator60();
      expect(result).assertEqual(true);
    });
    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_6100
     * @tc.name proxy2Test061
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest061', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyAdapter61();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_6200
     * @tc.name proxy2Test062
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest062', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyMediator62();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_6300
     * @tc.name proxy2Test063
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest063', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyLRUCache63();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_6400
     * @tc.name proxy2Test064
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest064', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyAccessFrequency64();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_6500
     * @tc.name proxy2Test065
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest065', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyLazyLoad65();
      expect(result).assertEqual(1);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_6600
     * @tc.name proxy2Test066
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest066', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyToStringTag66();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_6700
     * @tc.name proxy2Test067
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest067', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyHasInstance67();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_6800
     * @tc.name proxy2Test068
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest068', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxySymbolSpecies68();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_6900
     * @tc.name proxy2Test069
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest069', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyErrorCapture69();
      expect(result).assertEqual(1);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_7000
     * @tc.name proxy2Test070
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest070', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async () => {
      let result = await testProxySyncAsyncError70();
      expect(result).assertEqual(2);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_7100
     * @tc.name proxy2Test071
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest071', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async () => {
      let result = await testProxyErrorRetry71();
      expect(result).assertEqual(1);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_7200
     * @tc.name proxy2Test072
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest072', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyMapIterator72();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_7800
     * @tc.name proxy2Test078
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest078', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testGetVsGetPrototypeOf78();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_7900
     * @tc.name proxy2Test079
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest079', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testSetVsDefineProperty79();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_8000
     * @tc.name proxy2Test080
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest080', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testOwnKeysVsHas80();
      expect(result).assertEqual(1);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_8700
     * @tc.name proxy2Test087
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest087', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyDataBinding87();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_8800
     * @tc.name proxy2Test088
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest088', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyAccessControl88();
      expect(result).assertEqual(1);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_8900
     * @tc.name proxy2Test089
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest089', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyCommandPattern89();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_9000
     * @tc.name proxy2Test090
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest090', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyArrayAt90();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_9100
     * @tc.name proxy2Test091
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest091', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyObjectHasOwn91();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_9200
     * @tc.name proxy2Test092
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest092', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyErrorCause92();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_9500
     * @tc.name proxy2Test095
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest095', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxyRecursive95();
      expect(result).assertEqual(true);
    });

    /**
     * @tc.number SUB_BUILTINS_PROXY2_TEST_10000
     * @tc.name proxy2Test100
     * @tc.desc test proxy
     * @tc.size MediumTest
     * @tc.type Function
     * @tc.level Level 1
     */
    it('proxyTest100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let result = testProxySandbox100();
      expect(result).assertEqual(1);
    });

  })
}