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
import { MouseButton } from "@kit.TestKit";

export default function XComponentMouseTest() {
  describe('XComponentMouseTest', () => {
    beforeAll(async (done: Function) => {
      await Utils.pushPageNoClear(`NativeXComponent/MouseNativeXComponent`, done);
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTACTION_0100
     * @tc.name    : testMouseEventAction001
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testMouseEventAction001', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTACTION_0100 start<====`);
        Utils.driver.mouseLongClick({ x: 150, y: 150 }, MouseButton.MOUSE_BUTTON_LEFT, 0, 0, 200);
        Utils.driver.triggerKey(1);
        await Utils.simulateByShell('aa start -a MainAbility -b com.acts.keycodeevent');
        let Result: number = nativexcomponent.MouseEventAction(0);
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTACTION_0100 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTACTION_0200
     * @tc.name    : testMouseEventAction002
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testMouseEventAction002', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTACTION_0200 start<====`);
        await Utils.driver.mouseClick({ x: 150, y: 150 }, MouseButton.MOUSE_BUTTON_LEFT);
        let Result: number = nativexcomponent.MouseEventAction(1);
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTACTION_0200 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTACTION_0300
     * @tc.name    : testMouseEventAction003
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testMouseEventAction003', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTACTION_0100 start<====`);
        await Utils.driver.mouseClick({ x: 150, y: 150 }, MouseButton.MOUSE_BUTTON_LEFT);
        let Result: number = nativexcomponent.MouseEventAction(2);
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTACTION_0300 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTACTION_0400
     * @tc.name    : testMouseEventAction004
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testMouseEventAction004', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTACTION_0400 start<====`);
        await Utils.driver.mouseMoveTo({ x: 150, y: 150 });
        let Result: number = nativexcomponent.MouseEventAction(3);
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTACTION_0400 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTBUTTON_0200
     * @tc.name    : testMouseEventButton002
     * @tc.desc    : Test get native xcomopnent key event action invalid.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testMouseEventButton002', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTBUTTON_0200 start<====`);
        await Utils.driver.mouseClick({ x: 150, y: 150 }, MouseButton.MOUSE_BUTTON_LEFT);
        let Result: number = nativexcomponent.MouseEventButton(1);
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTBUTTON_0200 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTBUTTON_0300
     * @tc.name    : testMouseEventButton003
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testMouseEventButton003', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTBUTTON_0300 start<====`);
        await Utils.driver.mouseClick({ x: 150, y: 150 }, MouseButton.MOUSE_BUTTON_RIGHT);
        let Result: number = nativexcomponent.MouseEventButton(2);
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTBUTTON_0300 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTBUTTON_0400
     * @tc.name    : testMouseEventButton004
     * @tc.desc    : Test get native xcomopnent key event action invalid.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testMouseEventButton004', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTBUTTON_0400 start<====`);
        await Utils.driver.mouseClick({ x: 150, y: 150 }, MouseButton.MOUSE_BUTTON_MIDDLE);
        let Result: number = nativexcomponent.MouseEventButton(4);
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTBUTTON_0400 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTBUTTON_0500
     * @tc.name    : testMouseEventButton005
     * @tc.desc    : Test get native xcomopnent key event action works.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testMouseEventButton005', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTBUTTON_0500 start<====`);
        await Utils.simulateByShell('uinput -M -m 150 150 -d 6 -u 6');
        let Result: number = nativexcomponent.MouseEventButton(8);
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTBUTTON_0500 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTBUTTON_0600
     * @tc.name    : testMouseEventButton006
     * @tc.desc    : Test get native xcomopnent key event action invalid.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testMouseEventButton006', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTBUTTON_0600 start<====`);
        await Utils.simulateByShell('uinput -M -m 150 150 -d 5 -u 5');
        let Result: number = nativexcomponent.MouseEventButton(16);
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENTBUTTON_0600 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENT_TIMESTAMP_0100
     * @tc.name    : testMouseEventTimeStamp
     * @tc.desc    : Test SUB_ARKUI_ACE_ENGINE_GetTouchPointDisplayY function works fine.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testMouseEventTimeStamp', TestType.FUNCTION |
    Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info(`====>SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENT_TIMESTAMP_0100 start<====`);
        await Utils.simulateByShell('uinput -M -m 150 150');
        let Result: number = nativexcomponent.MouseEventTimeStamp();
        expect(1).assertEqual(Result);
        done()
      } catch (err) {
        console.error(`====>SUB_ARKUI_ACE_ENGINE_OH_NATIVEXCOMPONENT_MOUSEEVENT_TIMESTAMP_0100 catch err: ` + err);
        expect(false).assertEqual(err);
        done()
      }
    })
  })
}