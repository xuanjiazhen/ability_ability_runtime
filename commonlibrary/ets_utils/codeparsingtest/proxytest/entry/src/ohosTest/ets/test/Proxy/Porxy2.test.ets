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
  testGetInterceptArrayPush6,
  testGetLazyInitialization8,
  testGetReturnThis19,
  testGetSymbolIterator33,
  testHasForInLoop21,
  testNestedProxyThis2,
  testOwnKeysJsonStringify14,
  testProxyArrayPop26,
  testProxyArraySplice27,
  testProxyAsyncFunction12,
  testProxyDataValidator16,
  testProxyErrorStack28,
  testProxyGenerator13,
  testProxyIntlFormatter29,
  testProxyObserver17,
  testProxyReadOnlyView18,
  testProxySetAdd3,
  testProxyStructuredClone15,
  testReflectEnumerateLike9,
  testReflectGetPrototypeNull11,
  testReflectPreventExtensions10,
  testRevocableReleaseResource24,
  testSetArrayLengthLimit7,
  testSetPrototypeProperty20,
  testSetToPrimitive32
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
  })
}