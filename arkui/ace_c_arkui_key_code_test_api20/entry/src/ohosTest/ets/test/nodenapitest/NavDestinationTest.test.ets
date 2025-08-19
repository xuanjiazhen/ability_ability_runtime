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
const ARKUI_ERROR_CODE_NODE_INDEX_INVALID = 106200

export default function NavDestinationTest() {
  describe('NavDestinationTest', () => {
    beforeAll(async (done: Function) => {
      await Utils.pushPage(`nodenapitest/navdestinationTest`, done);
    });

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavDestinationName_0200
       * @tc.name       : testOHArkUIGetNavDestinationName0200
       * @tc.desc       : testing OH_ArkUI_GetNavDestinationName interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavDestinationName0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavDestinationName0200 start====");
          let result: number = nativerenderndk.GetNavDestinationNameTest_002();
          console.info(`====>testOHArkUIGetNavDestinationName0200 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavDestinationName0200 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavDestinationName_0300
       * @tc.name       : testOHArkUIGetNavDestinationName0300
       * @tc.desc       : testing OH_ArkUI_GetNavDestinationName interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavDestinationName0300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavDestinationName0300 start====");
          let result: number = nativerenderndk.GetNavDestinationNameTest_003();
          console.info(`====>testOHArkUIGetNavDestinationName0300 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_GET_INFO_FAILED);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavDestinationName0300 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavDestinationName_0500
       * @tc.name       : testOHArkUIGetNavDestinationName0500
       * @tc.desc       : testing OH_ArkUI_GetNavDestinationName interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavDestinationName0500', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavDestinationName0500 start====");
          let result: number = nativerenderndk.GetNavDestinationNameTest_005();
          console.info(`====>testOHArkUIGetNavDestinationName0500 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavDestinationName0500 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavDestinationName_0600
       * @tc.name       : testOHArkUIGetNavDestinationName0600
       * @tc.desc       : testing OH_ArkUI_GetNavDestinationName interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavDestinationName0600', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavDestinationName0600 start====");
          let result: number = nativerenderndk.GetNavDestinationNameTest_006();
          console.info(`====>testOHArkUIGetNavDestinationName0600 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavDestinationName0600 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavStackLength_0200
       * @tc.name       : testOHArkUIGetNavStackLength0200
       * @tc.desc       : testing OH_ArkUI_GetNavStackLength interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavStackLength0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavStackLength0200 start====");
          let result: number = nativerenderndk.GetNavStackLengthTest_002();
          console.info(`====>testOHArkUIGetNavStackLength0200 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavStackLength0200 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavStackLength_0300
       * @tc.name       : testOHArkUIGetNavStackLength0300
       * @tc.desc       : testing OH_ArkUI_GetNavStackLength interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavStackLength0300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavStackLength0300 start====");
          let result: number = nativerenderndk.GetNavStackLengthTest_003();
          console.info(`====>testOHArkUIGetNavStackLength0300 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_GET_INFO_FAILED);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavStackLength0300 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavDestinationNameByIndex_0300
       * @tc.name       : testOHArkUIGetNavDestinationNameByIndex0300
       * @tc.desc       : testing OH_ArkUI_GetNavDestinationNameByIndex interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavDestinationNameByIndex0300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavDestinationNameByIndex0300 start====");
          let result: number = nativerenderndk.GetNavDestinationNameByIndexTest_003();
          console.info(`====>testOHArkUIGetNavDestinationNameByIndex0300 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_GET_INFO_FAILED);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavDestinationNameByIndex0300 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavDestinationNameByIndex_0500
       * @tc.name       : testOHArkUIGetNavDestinationNameByIndex0500
       * @tc.desc       : testing OH_ArkUI_GetNavDestinationNameByIndex interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavDestinationNameByIndex0500', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavDestinationNameByIndex0500 start====");
          let result: number = nativerenderndk.GetNavDestinationNameByIndexTest_005();
          console.info(`====>testOHArkUIGetNavDestinationNameByIndex0500 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavDestinationNameByIndex0500 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavDestinationNameByIndex_0600
       * @tc.name       : testOHArkUIGetNavDestinationNameByIndex0600
       * @tc.desc       : testing OH_ArkUI_GetNavDestinationNameByIndex interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavDestinationNameByIndex0600', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavDestinationNameByIndex0600 start====");
          let result: number = nativerenderndk.GetNavDestinationNameByIndexTest_006();
          console.info(`====>testOHArkUIGetNavDestinationNameByIndex0600 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavDestinationNameByIndex0600 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavDestinationNameByIndex_0700
       * @tc.name       : testOHArkUIGetNavDestinationNameByIndex0700
       * @tc.desc       : testing OH_ArkUI_GetNavDestinationNameByIndex interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavDestinationNameByIndex0700', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavDestinationNameByIndex0700 start====");
          let result: number = nativerenderndk.GetNavDestinationNameByIndexTest_007();
          console.info(`====>testOHArkUIGetNavDestinationNameByIndex0700 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavDestinationNameByIndex0700 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavDestinationId_0200
       * @tc.name       : testOHArkUIGetNavDestinationId0200
       * @tc.desc       : testing OH_ArkUI_GetNavDestinationId interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavDestinationId0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavDestinationId0200 start====");
          let result: number = nativerenderndk.GetNavDestinationIdTest_002();
          console.info(`====>testOHArkUIGetNavDestinationId0200 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_GET_INFO_FAILED);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavDestinationId0200 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavDestinationId_0400
       * @tc.name       : testOHArkUIGetNavDestinationId0400
       * @tc.desc       : testing OH_ArkUI_GetNavDestinationId interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavDestinationId0400', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavDestinationId0400 start====");
          let result: number = nativerenderndk.GetNavDestinationIdTest_004();
          console.info(`====>testOHArkUIGetNavDestinationId0400 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavDestinationId0400 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavDestinationId_0500
       * @tc.name       : testOHArkUIGetNavDestinationId0500
       * @tc.desc       : testing OH_ArkUI_GetNavDestinationId interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavDestinationId0500', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavDestinationId0500 start====");
          let result: number = nativerenderndk.GetNavDestinationIdTest_005();
          console.info(`====>testOHArkUIGetNavDestinationId0500 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavDestinationId0500 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavDestinationId_0600
       * @tc.name       : testOHArkUIGetNavDestinationId0600
       * @tc.desc       : testing OH_ArkUI_GetNavDestinationId interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavDestinationId0600', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavDestinationId0600 start====");
          let result: number = nativerenderndk.GetNavDestinationIdTest_006();
          console.info(`====>testOHArkUIGetNavDestinationId0600 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavDestinationId0600 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavDestinationState_0200
       * @tc.name       : testOHArkUIGetNavDestinationState0200
       * @tc.desc       : testing OH_ArkUI_GetNavDestinationState interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavDestinationState0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavDestinationState0200 start====");
          let result: number = nativerenderndk.GetNavDestinationStateTest_002();
          console.info(`====>testOHArkUIGetNavDestinationState0200 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavDestinationState0200 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

   /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavDestinationState_0300
       * @tc.name       : testOHArkUIGetNavDestinationState0300
       * @tc.desc       : testing OH_ArkUI_GetNavDestinationState interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavDestinationState0300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavDestinationState0300 start====");
          let result: number = nativerenderndk.GetNavDestinationStateTest_003();
          console.info(`====>testOHArkUIGetNavDestinationState0300 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavDestinationState0300 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

   /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavDestinationState_0400
       * @tc.name       : testOHArkUIGetNavDestinationState0400
       * @tc.desc       : testing OH_ArkUI_GetNavDestinationState interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavDestinationState0400', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavDestinationState0400 start====");
          let result: number = nativerenderndk.GetNavDestinationStateTest_004();
          console.info(`====>testOHArkUIGetNavDestinationState0400 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_GET_INFO_FAILED);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavDestinationState0400 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavDestinationIndex_0200
       * @tc.name       : testOHArkUIGetNavDestinationIndex0200
       * @tc.desc       : testing OH_ArkUI_GetNavDestinationIndex interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavDestinationIndex0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavDestinationIndex0200 start====");
          let result: number = nativerenderndk.GetNavDestinationIndexTest_002();
          console.info(`====>testOHArkUIGetNavDestinationIndex0200 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavDestinationIndex0200 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

   /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavDestinationIndex_0300
       * @tc.name       : testOHArkUIGetNavDestinationIndex0300
       * @tc.desc       : testing OH_ArkUI_GetNavDestinationIndex interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavDestinationIndex0300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavDestinationIndex0300 start====");
          let result: number = nativerenderndk.GetNavDestinationIndexTest_003();
          console.info(`====>testOHArkUIGetNavDestinationIndex0300 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavDestinationIndex0300 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

   /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetNavDestinationIndex_0400
       * @tc.name       : testOHArkUIGetNavDestinationIndex0400
       * @tc.desc       : testing OH_ArkUI_GetNavDestinationIndex interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetNavDestinationIndex0400', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetNavDestinationIndex0400 start====");
          let result: number = nativerenderndk.GetNavDestinationIndexTest_004();
          console.info(`====>testOHArkUIGetNavDestinationIndex0400 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_GET_INFO_FAILED);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetNavDestinationIndex0400 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

  })
}