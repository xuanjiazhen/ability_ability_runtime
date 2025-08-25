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

export default function NodeUtilsTest1() {
  describe('NodeUtilsTest1', () => {

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_0100
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo0100
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo0100 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_001()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo0100 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo0100 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_0200
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo0200
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo0200 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_002()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo0200 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo0200 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_0300
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo0300
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo0300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo0300 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_003()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo0300 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo0300 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_0400
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo0400
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo0400', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo0400 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_004()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo0400 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo0400 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_0500
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo0500
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo0500', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo0500 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_005()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo0500 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo0500 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_0600
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo0600
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo0600', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo0600 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_006()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo0600 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo0600 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_0700
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo0700
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo0700', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo0700 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_007()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo0700 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo0700 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_0800
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo0800
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo0800', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo0800 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_008()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo0800 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo0800 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_0900
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo0900
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo0900', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo0900 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_009()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo0900 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo0900 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_1000
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo1000
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo1000', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo1000 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_010()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo1000 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo1000 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_1100
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo1100
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo1100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo1100 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_011()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo1100 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo1100 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_1200
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo1200
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo1200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo1200 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_012()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo1200 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo1200 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_1300
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo1300
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo1300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo1300 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_013()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo1300 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo1300 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_1400
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo1400
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo1400', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo1400 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_014()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo1400 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo1400 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_1500
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo1500
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo1500', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo1500 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_015()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo1500 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo1500 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_1600
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo1600
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo1600', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo1600 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_016()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo1600 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo1600 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_1700
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo1700
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo1700', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo1700 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_017()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo1700 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo1700 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_1800
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo1800
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo1800', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo1800 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_018()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo1800 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo1800 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_1900
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo1900
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo1900', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo1900 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_019()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo1900 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo1900 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_2000
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo2000
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo2000', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo2000 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_020()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo2000 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo2000 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_2100
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo2100
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo2100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo2100 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_021()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo2100 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo2100 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_2200
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo2200
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo2200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo2200 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_022()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo2200 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo2200 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_2300
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo2300
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo2300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo2300 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_023()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo2300 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo2300 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_2400
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo2400
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo2400', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo2400 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_024()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo2400 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo2400 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_2500
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo2500
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo2500', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo2500 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_025()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo2500 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo2500 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_2600
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo2600
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo2600', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo2600 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_026()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo2600 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo2600 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_2700
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo2700
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo2700', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo2700 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_027()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo2700 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo2700 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_2800
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo2800
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo2800', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo2800 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_028()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo2800 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo2800 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_2900
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo2900
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo2900', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo2900 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_029()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo2900 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo2900 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_3000
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo3000
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo3000', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo3000 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_030()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo3000 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo3000 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_3100
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo3100
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo3100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo3100 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_031()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo3100 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo3100 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_3200
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo3200
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo3200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo3200 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_032()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo3200 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo3200 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_3300
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo3300
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo3300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo3300 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_033()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo3300 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo3300 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_3400
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo3400
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo3400', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo3400 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_034()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo3400 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo3400 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_3500
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo3500
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo3500', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo3500 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_035()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo3500 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo3500 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : testOHArkUINodeUtilsGetActiveChildrenInfo3600
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo3600
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo3600', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo3600 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_036()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo3600 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo3600 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_3700
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo3700
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo3700', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo3700 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_037()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo3700 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo3700 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_3800
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo3800
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo3800', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo3800 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_038()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo3800 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo3800 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

    /*
    * @tc.number     : SUB_ArkUI_ACE_ENGINE_OHArkUINodeUtilsGetNodeType_3900
    * @tc.name       : testOHArkUINodeUtilsGetActiveChildrenInfo3900
    * @tc.desc       : testing OH_ArkUI_NodeUtils_GetNodeType interface
    * @tc.size       : MediumTest
    * @tc.type       : Function
    * @tc.level      : Level 1
    */
    it('testOHArkUINodeUtilsGetActiveChildrenInfo3900', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testOHArkUINodeUtilsGetActiveChildrenInfo3900 start====");
          let result: number = nativerenderndk.GetNodeTypeTest_039()
          console.info(`====>testOHArkUINodeUtilsGetActiveChildrenInfo3900 result====${result}`);
          expect(result).assertEqual(ARKUI_ERROR_CODE_NO_ERROR);
          done();
        } catch (err) {
          console.error("====>testOHArkUINodeUtilsGetActiveChildrenInfo3900 catch err: " + err);
          expect(false).assertEqual(ARKUI_ERROR_CODE_ERROR);
        }
      })

  })
}