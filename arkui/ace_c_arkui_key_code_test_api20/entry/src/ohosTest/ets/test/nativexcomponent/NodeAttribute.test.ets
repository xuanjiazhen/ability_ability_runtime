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

import { beforeAll, describe, expect, it, Level, Size, TestType } from "@ohos/hypium";
import nativexcomponent from 'libnativexcomponent.so';
import Utils from "../common/Utils";

export default function NodeAttribute() {
  describe('NodeAttribute', () => {
    beforeAll(async (done: Function) => {
      await Utils.pushPageNoClear(`NativeXComponent/NodeAttributeNativeXComponent`, done);
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_HALF_LEADING_0100
     * @tc.name    : testCheckNodeTextHalfLeading
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testCheckNodeTextHalfLeading', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_HALF_LEADING_0100 start<====`);
        let Result: number = nativexcomponent.CheckNodeTextHalfLeading();
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_HALF_LEADING_0100 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_LINE_COUNT_0100
     * @tc.name    : testCheckNodeTextLineCount
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testCheckNodeTextLineCount', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_LINE_COUNT_0100 start<====`);
        let Result: number = nativexcomponent.CheckNodeTextLineCount();
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_LINE_COUNT_0100 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_OPTIMIZE_TRAILING_SPACE_0100
     * @tc.name    : testCheckNodeTextOptimizeTrailingSpace
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testCheckNodeTextOptimizeTrailingSpace', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_OPTIMIZE_TRAILING_SPACE_0100 start<====`);
        let Result: number = nativexcomponent.CheckNodeTextOptimizeTrailingSpace();
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_OPTIMIZE_TRAILING_SPACE_0100 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_LINEAR_GRADIENT_0100
     * @tc.name    : testCheckNodeTextLinearGradient
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testCheckNodeTextLinearGradient', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_LINEAR_GRADIENT_0100 start<====`);
        let Result: number = nativexcomponent.CheckNodeTextLinearGradient();
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_LINEAR_GRADIENT_0100 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_RADIAL_GRADIENT_0100
     * @tc.name    : testCheckNodeTextRadialGradient
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testCheckNodeTextRadialGradient', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_RADIAL_GRADIENT_0100 start<====`);
        let Result: number = nativexcomponent.CheckNodeTextRadialGradient();
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_RADIAL_GRADIENT_0100 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_VERTICAL_ALIGN_0100
     * @tc.name    : testCheckNodeTextVerticalAlign
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testCheckNodeTextVerticalAlign', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_VERTICAL_ALIGN_0100 start<====`);
        let Result: number = nativexcomponent.CheckNodeTextVerticalAlign();
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_VERTICAL_ALIGN_0100 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    // /**
    //  * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_INPUT_HALF_LEADING_0100
    //  * @tc.name    : testCheckNodeTextInputHalfLeading
    //  * @tc.desc    : Test get native xcomopnent key event action works.
    //  * @tc.level   : LEVEL1
    //  * @tc.size    : MediumTest
    //  * @tc.type    : Function
    //  */
    // it('testCheckNodeTextInputHalfLeading', TestType.FUNCTION |
    // Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
    //   try {
    //     console.info(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_INPUT_HALF_LEADING_0100 start<====`);
    //     let Result: number = nativexcomponent.CheckNodeTextInputHalfLeading();
    //     expect(1).assertEqual(Result);
    //     done()
    //   } catch (err) {
    //     console.error(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_INPUT_HALF_LEADING_0100 catch err: ` + err);
    //     expect(false).assertEqual(err);
    //     done()
    //   }
    // })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_INPUT_KEYBOARD_APPEARANCE_0100
     * @tc.name    : testCheckNodeTextInputKeyboardAppearance
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testCheckNodeTextInputKeyboardAppearance', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_INPUT_KEYBOARD_APPEARANCE_0100 start<====`);
        let Result: number = nativexcomponent.CheckNodeTextInputKeyboardAppearance();
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_INPUT_KEYBOARD_APPEARANCE_0100 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_INPUT_ENABLE_FILL_ANIMATION_0100
     * @tc.name    : testCheckNodeTextInputEnableFillAnimation
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testCheckNodeTextInputEnableFillAnimation', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_INPUT_ENABLE_FILL_ANIMATION_0100 start<====`);
        let Result: number = nativexcomponent.CheckNodeTextInputEnableFillAnimation();
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_INPUT_ENABLE_FILL_ANIMATION_0100 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_INPUT_LINE_HEIGHT_0100
     * @tc.name    : testCheckNodeTextInputLineHeight
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testCheckNodeTextInputLineHeight', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_INPUT_LINE_HEIGHT_0100 start<====`);
        let Result: number = nativexcomponent.CheckNodeTextInputLineHeight();
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_INPUT_LINE_HEIGHT_0100 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_INPUT_FILTER_0100
     * @tc.name    : testCheckNodeTextAreaInputFilter
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testCheckNodeTextAreaInputFilter', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_INPUT_FILTER_0100 start<====`);
        let Result: number = nativexcomponent.CheckNodeTextAreaInputFilter();
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_INPUT_FILTER_0100 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_CONTENT_TYPE_0100
     * @tc.name    : testCheckNodeTextAreaContentType
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testCheckNodeTextAreaContentType', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_CONTENT_TYPE_0100 start<====`);
        let Result: number = nativexcomponent.CheckNodeTextAreaContentType();
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_CONTENT_TYPE_0100 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    // /**
    //  * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_HALF_LEADING_0100
    //  * @tc.name    : testCheckNodeTextAreaHalfLeading
    //  * @tc.desc    : Test get native xcomopnent key event action works.
    //  * @tc.level   : LEVEL1
    //  * @tc.size    : MediumTest
    //  * @tc.type    : Function
    //  */
    // it('testCheckNodeTextAreaHalfLeading', TestType.FUNCTION |
    // Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
    //   try {
    //     console.info(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_HALF_LEADING_0100 start<====`);
    //     let Result: number = nativexcomponent.CheckNodeTextAreaHalfLeading();
    //     expect(1).assertEqual(Result);
    //     done()
    //   } catch (err) {
    //     console.error(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_HALF_LEADING_0100 catch err: ` + err);
    //     expect(false).assertEqual(err);
    //     done()
    //   }
    // })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_KEYBOARD_APPEARANCE_0100
     * @tc.name    : testCheckNodeTextAreaKeyboardAppearance
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testCheckNodeTextAreaKeyboardAppearance', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_KEYBOARD_APPEARANCE_0100 start<====`);
        let Result: number = nativexcomponent.CheckNodeTextAreaKeyboardAppearance();
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_KEYBOARD_APPEARANCE_0100 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_MAX_LINES_0100
     * @tc.name    : testCheckNodeTextAreaMaxLines
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testCheckNodeTextAreaMaxLines', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_MAX_LINES_0100 start<====`);
        let Result: number = nativexcomponent.CheckNodeTextAreaMaxLines();
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_MAX_LINES_0100 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_LINE_SPACING_0100
     * @tc.name    : testCheckNodeTextAreaLineSpacing
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testCheckNodeTextAreaLineSpacing', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_LINE_SPACING_0100 start<====`);
        let Result: number = nativexcomponent.CheckNodeTextAreaLineSpacing();
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_LINE_SPACING_0100 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_MIN_LINES_0100
     * @tc.name    : testCheckNodeTextAreaMinLines
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testCheckNodeTextAreaMinLines', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_MIN_LINES_0100 start<====`);
        let Result: number = nativexcomponent.CheckNodeTextAreaMinLines();
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_MIN_LINES_0100 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_MAX_LINES_WITH_SCROLL_0100
     * @tc.name    : testCheckNodeTextAreaMaxLinesWithScroll
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testCheckNodeTextAreaMaxLinesWithScroll', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_MAX_LINES_WITH_SCROLL_0100 start<====`);
        let Result: number = nativexcomponent.CheckNodeTextAreaMaxLinesWithScroll();
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_MAX_LINES_WITH_SCROLL_0100 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_LINE_HEIGHT_0100
     * @tc.name    : testCheckNodeTextAreaLineHeight
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testCheckNodeTextAreaLineHeight', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_LINE_HEIGHT_0100 start<====`);
        let Result: number = nativexcomponent.CheckNodeTextAreaLineHeight();
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_AREA_LINE_HEIGHT_0100 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_PICKER_SELECTED_BACKGROUND_STYLE_0100
     * @tc.name    : testCheckNodeTextPickSelectedBackgroundStyle
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testCheckNodeTextPickSelectedBackgroundStyle', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_PICKER_SELECTED_BACKGROUND_STYLE_0100 start<====`);
        let Result: number = nativexcomponent.CheckNodeTextPickSelectedBackgroundStyle();
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_ARKUI_NODEATTRIBUTETYPE_NODE_TEXT_PICKER_SELECTED_BACKGROUND_STYLE_0100 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
  })
}