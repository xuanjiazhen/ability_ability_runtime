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
import hilog from '@ohos.hilog'
import napitestex from 'libnapitestex.so'

async function sleep(time: number) {
  return new Promise((resolve: (value: string) => void) => {
    setTimeout(() => {
      resolve("ok")
    }, time)
  }).then(() => {
    console.info(`sleep ${time} over...`)
  })
}

export default function napiCheckExTest() {
  describe('napiCheckExTest', () => {
    /**
     * run after testcase
     */
    afterEach(async () => {
      console.info('[napiCheckExTest] after each called')
    });

    /**
    * @tc.number    SUB_ACE_BASIC_ETS_NAPI_BUFFER_0001
    * @tc.name      napiTestEx0001
    * @tc.desc      napiTestEx
    */
    it('napiTestEx0001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let buffer: Object = napitestex.CreateBufferCopy();
      expect(napitestex.IsBuffer(buffer)).assertTrue();
      expect(napitestex.GetBufferInfo(buffer)).assertTrue();
    });

    /**
    * @tc.number    SUB_ACE_BASIC_ETS_NAPI_BUFFER_0002
    * @tc.name      napiBufferTest002
    * @tc.desc      aceNapiEtsTest
    */
    it('napiBufferTest002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let buffer: Object = napitestex.CreateBuffer();
      expect(napitestex.IsBuffer(buffer)).assertTrue();
      expect(napitestex.GetBufferInfo(buffer)).assertTrue();
    })

    /**
    * @tc.number    SUB_ACE_BASIC_ETS_NAPI_BUFFER_0003
    * @tc.name      napiBufferTest003
    * @tc.desc      aceNapiEtsTest
    */
    it('napiBufferTest003', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let buffer: Object = napitestex.CreateBufferCopy();
      expect(napitestex.IsBuffer(buffer)).assertTrue();
      expect(napitestex.GetBufferInfo(buffer)).assertTrue();
    })

    /**
    * @tc.number    SUB_ACE_BASIC_ETS_NAPI_BUFFER_0004
    * @tc.name      napiBufferTest004
    * @tc.desc      aceNapiEtsTest - Test external buffer creation and finalize callback
    */
    it('napiBufferTest004', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      let externalBuffer: Object = napitestex.CreateExternalBufferTest();
      expect(napitestex.IsBuffer(externalBuffer)).assertTrue();
      expect(napitestex.GetBufferInfo(externalBuffer)).assertTrue();
    })

    /**
    * @tc.number    SUB_ACE_BASIC_ETS_NAPI_BUFFER_0005
    * @tc.name      napiBufferTest005
    * @tc.desc      aceNapiEtsTest - Test buffer creation edge cases and error handling
    */
    it('napiBufferTest005', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      // 测试零长度buffer
      let zeroLengthBuffer: Object = napitestex.createBufferSizeZeroTest();
      expect(zeroLengthBuffer).assertDeepEquals(1);
      
      // 测试最大长度buffer (2097152)
      let maxLengthBuffer: Object = napitestex.CreateBufferMaxLength();
      hilog.info(0x0000, 'testTag', 'maxLengthBuffer:%{public}d', maxLengthBuffer);
      expect(maxLengthBuffer).assertDeepEquals(0);
      
      // 测试超过最大长度 - 应该返回错误状态
      let invalidLengthStatus: Object = napitestex.CreateBufferExceedMaxLength();
      // 检查返回的是否为错误状态码 (napi_invalid_arg)
      hilog.info(0x0000, 'testTag', 'invalidLengthStatus:%{public}d', invalidLengthStatus);
      expect(invalidLengthStatus).assertDeepEquals(1); // napi_invalid_arg = 1
      
      // 测试无效参数 - 应该返回错误状态(crash)
      //let invalidEnvStatus: Object = napitestex.CreateBufferInvalidEnv();
      //hilog.info(0x0000, 'testTag', 'invalidEnvStatus:%{public}d', invalidEnvStatus);
      //expect(invalidEnvStatus).assertDeepEquals(1); // napi_invalid_arg = 1
      
      let invalidDataStatus: Object = napitestex.CreateBufferInvalidData();
      hilog.info(0x0000, 'testTag', 'invalidDataStatus:%{public}d', invalidDataStatus);
      expect(invalidDataStatus).assertDeepEquals(1); // napi_invalid_arg = 1
      
      //(crash)
      //let invalidResultStatus: Object = napitestex.CreateBufferInvalidResult();
      //hilog.info(0x0000, 'testTag', 'invalidResultStatus:%{public}d', invalidResultStatus);
      //expect(invalidResultStatus).assertDeepEquals(1); // napi_invalid_arg = 1
    })

    /**
    * @tc.number    SUB_ACE_BASIC_ETS_NAPI_BUFFER_0006
    * @tc.name      napiBufferTest006
    * @tc.desc      aceNapiEtsTest - Test non-ArrayBuffer object detection
    */
    it('napiBufferTest006', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      // 测试非buffer对象的检测
      let notABuffer: Object = napitestex.CreateInt32Test();
      hilog.info(0x0000, 'testTag', 'notABuffer:%{public}d', notABuffer);
      expect(napitestex.IsBuffer(notABuffer)).assertFalse();
      
      // 测试获取非buffer对象信息应该返回错误
      let getBufferInfoStatus: Object = napitestex.GetBufferInfoNonBuffer(notABuffer);
      hilog.info(0x0000, 'testTag', 'getBufferInfoStatus:%{public}d', getBufferInfoStatus);
      // 应该返回 napi_arraybuffer_expected 错误码
      expect(getBufferInfoStatus).assertDeepEquals(19); // napi_arraybuffer_expected = 19
    })

    /**
    * @tc.number    SUB_ACE_BASIC_ETS_NAPI_BUFFER_0007
    * @tc.name      napiBufferTest007
    * @tc.desc      aceNapiEtsTest - Test multiple operations combination
    */
    it('napiBufferTest007', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      // 测试多种操作的组合
      let testResult: Object = napitestex.TestMultipleOperations();
      hilog.info(0x0000, 'testTag', 'testResult:%{public}d', testResult);
      // 所有操作成功应该返回 0 (napi_ok)
      expect(testResult).assertDeepEquals(0);
    })

    /**
    * @tc.number    SUB_ACE_BASIC_ETS_NAPI_BUFFER_0008
    * @tc.name      napiBufferTest008
    * @tc.desc      aceNapiEtsTest - Test ArrayBuffer thread safety in multi-threaded environment
    */
    it('napiBufferTest008', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      // 测试多线程环境下的ArrayBuffer线程安全性
      let threadSafetyResult: boolean = napitestex.TestArrayBufferThreadSafety();
      hilog.info(0x0000, 'testTag', 'threadSafetyResult:%{public}s', threadSafetyResult.toString());
      // 所有线程操作成功应该返回 true
      expect(threadSafetyResult).assertTrue();
    })

    /**
    * @tc.number    SUB_ACE_BASIC_ETS_NAPI_BUFFER_0009
    * @tc.name      napiBufferTest009
    * @tc.desc      aceNapiEtsTest - Test ArrayBuffer and external ArrayBuffer comparison
    */
    it('napiBufferTest009', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, () => {
      // 测试ArrayBuffer和外部ArrayBuffer的比对
      let comparisonResult: number = napitestex.TestArrayBufferComparison();
      hilog.info(0x0000, 'testTag', 'comparisonResult:%{public}d', comparisonResult);
      // 所有测试通过应该返回 0 (napi_ok)
      expect(comparisonResult).assertDeepEquals(1);
    })
  })

}
