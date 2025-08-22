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

export default function GetRouterTest() {
  describe('GetRouterTest', () => {
    beforeAll(async (done: Function) => {
      await Utils.pushPage(`nodenapitest/getrouterTest`, done);
    });

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetRouterPageIndex_0200
       * @tc.name       : testOHArkUIGetRouterPageIndex0200
       * @tc.desc       : testing OH_ArkUI_GetRouterPageIndex interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetRouterPageIndex0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetRouterPageIndex0200 start====");
          let result: number = nativerenderndk.GetRouterPageIndexTest_002();
          console.info(`====>testOHArkUIGetRouterPageIndex0200 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_GET_INFO_FAILED);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetRouterPageIndex0200 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetRouterPageIndex_0300
       * @tc.name       : testOHArkUIGetRouterPageIndex0300
       * @tc.desc       : testing OH_ArkUI_GetRouterPageIndex interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetRouterPageIndex0300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetRouterPageIndex0300 start====");
          let result: number = nativerenderndk.GetRouterPageIndexTest_003();
          console.info(`====>testOHArkUIGetRouterPageIndex0300 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetRouterPageIndex0300 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetRouterPageIndex_0400
       * @tc.name       : testOHArkUIGetRouterPageIndex0400
       * @tc.desc       : testing OH_ArkUI_GetRouterPageIndex interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetRouterPageIndex0400', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetRouterPageIndex0400 start====");
          let result: number = nativerenderndk.GetRouterPageIndexTest_004();
          console.info(`====>testOHArkUIGetRouterPageIndex0400 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetRouterPageIndex0400 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetRouterPageName_0200
       * @tc.name       : testOHArkUIGetRouterPageName0200
       * @tc.desc       : testing OH_ArkUI_GetRouterPageName interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetRouterPageName0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetRouterPageName0200 start====");
          let result: number = nativerenderndk.GetRouterPageNameTest_002();
          console.info(`====>testOHArkUIGetRouterPageName0200 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_GET_INFO_FAILED);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetRouterPageName0200 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })
    
    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetRouterPageName_0400
    * @tc.name       : testOHArkUIGetRouterPageName0400
    * @tc.desc       : testing OH_ArkUI_GetRouterPageName interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUIGetRouterPageName0400', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetRouterPageName0400 start====");
          let result: number = nativerenderndk.GetRouterPageNameTest_004();
          console.info(`====>testOHArkUIGetRouterPageName0400 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetRouterPageName0400 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetRouterPageName_0500
       * @tc.name       : testOHArkUIGetRouterPageName0500
       * @tc.desc       : testing OH_ArkUI_GetRouterPageName interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetRouterPageName0500', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetRouterPageName0500 start====");
          let result: number = nativerenderndk.GetRouterPageNameTest_005();
          console.info(`====>testOHArkUIGetRouterPageName0500 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetRouterPageName0500 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetRouterPageName_0600
       * @tc.name       : testOHArkUIGetRouterPageName0600
       * @tc.desc       : testing OH_ArkUI_GetRouterPageName interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetRouterPageName0600', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetRouterPageName0600 start====");
          let result: number = nativerenderndk.GetRouterPageNameTest_006();
          console.info(`====>testOHArkUIGetRouterPageName0600 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetRouterPageName0600 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetRouterPagePath_0200
       * @tc.name       : testOHArkUIGetRouterPagePath0200
       * @tc.desc       : testing OH_ArkUI_GetRouterPagePath interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetRouterPagePath0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetRouterPagePath0200 start====");
          let result: number = nativerenderndk.GetRouterPagePathTest_002();
          console.info(`====>testOHArkUIGetRouterPagePath0200 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_GET_INFO_FAILED);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetRouterPagePath0200 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetRouterPagePath_0400
       * @tc.name       : testOHArkUIGetRouterPagePath0400
       * @tc.desc       : testing OH_ArkUI_GetRouterPagePath interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetRouterPagePath0400', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetRouterPagePath0400 start====");
          let result: number = nativerenderndk.GetRouterPagePathTest_004();
          console.info(`====>testOHArkUIGetRouterPagePath0400 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetRouterPagePath0400 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetRouterPagePath_0500
       * @tc.name       : testOHArkUIGetRouterPagePath0500
       * @tc.desc       : testing OH_ArkUI_GetRouterPagePath interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetRouterPagePath0500', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetRouterPagePath0500 start====");
          let result: number = nativerenderndk.GetRouterPagePathTest_005();
          console.info(`====>testOHArkUIGetRouterPagePath0500 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetRouterPagePath0500 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetRouterPagePath_0600
       * @tc.name       : testOHArkUIGetRouterPagePath0600
       * @tc.desc       : testing OH_ArkUI_GetRouterPagePath interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetRouterPagePath0600', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetRouterPagePath0600 start====");
          let result: number = nativerenderndk.GetRouterPagePathTest_006();
          console.info(`====>testOHArkUIGetRouterPagePath0600 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetRouterPagePath0600 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetRouterPageState_0200
       * @tc.name       : testOHArkUIGetRouterPageState0200
       * @tc.desc       : testing OH_ArkUI_GetRouterPageState interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetRouterPageState0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetRouterPageState0200 start====");
          let result: number = nativerenderndk.GetRouterPageStateTest_002();
          console.info(`====>testOHArkUIGetRouterPageState0200 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_GET_INFO_FAILED);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetRouterPageState0200 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetRouterPageState_0300
       * @tc.name       : testOHArkUIGetRouterPageState0300
       * @tc.desc       : testing OH_ArkUI_GetRouterPageState interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetRouterPageState0300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetRouterPageState0300 start====");
          let result: number = nativerenderndk.GetRouterPageStateTest_003();
          console.info(`====>testOHArkUIGetRouterPageState0300 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetRouterPageState0300 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetRouterPageState_0400
       * @tc.name       : testOHArkUIGetRouterPageState0400
       * @tc.desc       : testing OH_ArkUI_GetRouterPageState interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetRouterPageState0400', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetRouterPageState0400 start====");
          let result: number = nativerenderndk.GetRouterPageStateTest_004();
          console.info(`====>testOHArkUIGetRouterPageState0400 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetRouterPageState0400 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetRouterPageId_0200
       * @tc.name       : testOHArkUIGetRouterPageId0200
       * @tc.desc       : testing OH_ArkUI_GetRouterPageId interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetRouterPageId0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetRouterPageId0200 start====");
          let result: number = nativerenderndk.GetRouterPageIdTest_002();
          console.info(`====>testOHArkUIGetRouterPageId0200 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_GET_INFO_FAILED);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetRouterPageId0200 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

   /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetRouterPageId_0400
       * @tc.name       : testOHArkUIGetRouterPageId0400
       * @tc.desc       : testing OH_ArkUI_GetRouterPageId interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetRouterPageId0400', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetRouterPageId0400 start====");
          let result: number = nativerenderndk.GetRouterPageIdTest_004();
          console.info(`====>testOHArkUIGetRouterPageId0400 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetRouterPageId0400 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

   /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetRouterPageId_0500
       * @tc.name       : testOHArkUIGetRouterPageId0500
       * @tc.desc       : testing OH_ArkUI_GetRouterPageId interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetRouterPageId0500', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetRouterPageId0500 start====");
          let result: number = nativerenderndk.GetRouterPageIdTest_005();
          console.info(`====>testOHArkUIGetRouterPageId0500 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetRouterPageId0500 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

   /*
       * @tc.number     : SUB_ArkUI_ACE_ENGINE_OH_ArkUI_GetRouterPageId_0600
       * @tc.name       : testOHArkUIGetRouterPageId0600
       * @tc.desc       : testing OH_ArkUI_GetRouterPageId interface
       * @tc.size       : MediumTest
       * @tc.type       : Function
       * @tc.level      : Level 1
       */
    it('testOHArkUIGetRouterPageId0600', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUIGetRouterPageId0600 start====");
          let result: number = nativerenderndk.GetRouterPageIdTest_006();
          console.info(`====>testOHArkUIGetRouterPageId0600 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_PARAM_INVALID);
          done();
        } catch (err) {
          console.error("====>testOHArkUIGetRouterPageId0600 err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

  })
}