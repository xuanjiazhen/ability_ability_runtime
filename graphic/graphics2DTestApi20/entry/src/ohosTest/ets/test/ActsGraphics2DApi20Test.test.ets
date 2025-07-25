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

import { describe, it, expect, TestType, Size, Level } from '@ohos/hypium'
import libGraphics2DApi20 from 'libgraphicsApiNdk.so'
const OH_DRAWING_SUCCESS = 0;
const OH_DRAWING_ERROR_INVALID_PARAMETER = 401;
export default function ActsNativeBufferColorSpaceTest() {
  describe('ActsGraphics2DApi20Test', () => {
    /**
     * @tc.number  : SUB_OH_Data_Asset_GetAlphaFloat_0100
     * @tc.name    : SUB_OH_Data_Asset_GetAlphaFloat_0100
     * @tc.desc    : test OH_Data_Asset_GetAlphaFloat Test Normal Scene
     * @tc.level   : Level0
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_Data_Asset_GetAlphaFloat_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, async (done: Function) => {
      try {
        console.info("====>SUB_OH_Data_Asset_GetAlphaFloat_0100 start====");
        let result: number = libGraphics2DApi20.drawingBrushGetAlphaFloatSuccess();
        console.info("====>SUB_OH_Data_Asset_GetAlphaFloat_0100 result====", result);
        expect(result).assertEqual(OH_DRAWING_SUCCESS);
        done();
      } catch (err) {
        console.error("====>SUB_OH_Data_Asset_GetAlphaFloat_0100 catch err: " + err);
        done();
      }
    })

    /**
     * @tc.number  : SUB_OH_Data_Asset_GetAlphaFloat_0200
     * @tc.name    : SUB_OH_Data_Asset_GetAlphaFloat_0200
     * @tc.desc    : test OH_Data_Asset_GetAlphaFloat Test Normal Scene
     * @tc.level   : Level0
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_OH_Data_Asset_GetAlphaFloat_0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, async (done: Function) => {
      try {
        console.info("====>SUB_OH_Data_Asset_GetAlphaFloat_0200 start====");
        let result: number = libGraphics2DApi20.drawingBrushGetAlphaFloatFail();
        console.info("====>SUB_OH_Data_Asset_GetAlphaFloat_0200 result====", result);
        expect(result).assertEqual(OH_DRAWING_ERROR_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error("====>SUB_OH_Data_Asset_GetAlphaFloat_0100 catch err: " + err);
        done();
      }
    })
  })
}
