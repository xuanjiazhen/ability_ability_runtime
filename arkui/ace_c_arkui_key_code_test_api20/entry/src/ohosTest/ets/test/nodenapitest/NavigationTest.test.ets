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

import { describe, it, expect, TestType, Size, Level, beforeAll } from '@ohos/hypium'
import nativerenderndk from 'libnoderenderndk.so'
import Utils from '../common/Utils';

const ARKUI_ERROR_CODE_NO_ERROR = 0;
const ARKUI_ERROR_CODE_ERROR = 1;
const ARKUI_ERROR_CODE_PARAM_INVALID = 401;
const ARKUI_ERROR_CODE_GET_INFO_FAILED = 106201;
const ARKUI_ERROR_CODE_BUFFER_SIZE_ERROR = 106202;

export default function NavigationTest() {
  describe('NavigationTest', () => {
    beforeAll(async (done: Function) => {
      await Utils.pushPage(`nodenapitest/navigationTest`, done);
    });

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavigationId_0200
       * @tc.name       : testOHArkUIGetNavigationId0200
       * @tc.desc       : testing OH_ArkUI_GetNavigationId interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavigationId0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavigationId0200 start====");
          let result: number = nativerenderndk.GetNavigationIdTest_002();
          console.info(`====>testOHArkUIGetNavigationId0200 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavigationId0200 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavigationId_0300
       * @tc.name       : testOHArkUIGetNavigationId0300
       * @tc.desc       : testing OH_ArkUI_GetNavigationId interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavigationId0300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavigationId0300 start====");
          let result: number = nativerenderndk.GetNavigationIdTest_003();
          console.info(`====>testOHArkUIGetNavigationId0300 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_GET_INFO_FAILED);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavigationId0300 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavigationId_0500
       * @tc.name       : testOHArkUIGetNavigationId0500
       * @tc.desc       : testing OH_ArkUI_GetNavigationId interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavigationId0500', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavigationId0500 start====");
          let result: number = nativerenderndk.GetNavigationIdTest_005();
          console.info(`====>testOHArkUIGetNavigationId0500 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavigationId0500 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavigationId_0600
       * @tc.name       : testOHArkUIGetNavigationId0600
       * @tc.desc       : testing OH_ArkUI_GetNavigationId interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavigationId0600', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavigationId0600 start====");
          let result: number = nativerenderndk.GetNavigationIdTest_006();
          console.info(`====>testOHArkUIGetNavigationId0600 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID );
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavigationId0600 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

  })
}