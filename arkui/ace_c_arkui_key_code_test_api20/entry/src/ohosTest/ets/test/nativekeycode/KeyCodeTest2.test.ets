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

import { describe, beforeAll, TestType, Size, Level } from '@ohos/hypium'
import keyCodeNdk from 'libkeycodendk.so'
import Utils from '../common/Utils';

export default function KeyCodeNdkTest2() {
  describe('KeyCodeNdkTest2', () => {
    beforeAll(async (done: Function) => {
      await Utils.pushPage(`KeyCodeTest/KeyCodeTest2`, done);
    });
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_CTRL_LEFT_0100
     * @tc.name    : GetKeyCodeEventCtrlLeft
     * @tc.desc    : Test whether the key KEY_CTRL_LEFT are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_CTRL_LEFT_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2072, keyCodeNdk.GetKeyCodeEventCtrlLeft)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_CTRL_RIGHT_0100
     * @tc.name    : GetKeyCodeEventCtrlRight
     * @tc.desc    : Test whether the key KEY_CTRL_RIGHT are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_CTRL_RIGHT_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2073, keyCodeNdk.GetKeyCodeEventCtrlRight)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_CAPS_LOCK_0100
     * @tc.name    : GetKeyCodeEventCapsLock
     * @tc.desc    : Test whether the key KEY_CAPS_LOCK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_CAPS_LOCK_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2074, keyCodeNdk.GetKeyCodeEventCapsLock)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_SCROLL_LOCK_0100
     * @tc.name    : GetKeyCodeEventScrollLock
     * @tc.desc    : Test whether the key KEY_SCROLL_LOCK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_SCROLL_LOCK_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2075, keyCodeNdk.GetKeyCodeEventScrollLock)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_META_LEFT_0100
     * @tc.name    : GetKeyCodeEventMetaLeft
     * @tc.desc    : Test whether the key KEY_META_LEFT are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_META_LEFT_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2076, keyCodeNdk.GetKeyCodeEventMetaLeft)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_META_RIGHT_0100
     * @tc.name    : GetKeyCodeEventMetaRight
     * @tc.desc    : Test whether the key KEY_META_RIGHT are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_META_RIGHT_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2077, keyCodeNdk.GetKeyCodeEventMetaRight)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_FUNCTION_0100
     * @tc.name    : GetKeyCodeEventFunction
     * @tc.desc    : Test whether the key KEY_FUNCTION are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_FUNCTION_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2078, keyCodeNdk.GetKeyCodeEventFunction)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_SYSRQ_0100
     * @tc.name    : GetKeyCodeEventSysRq
     * @tc.desc    : Test whether the key KEY_SYSRQ are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_SYSRQ_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2079, keyCodeNdk.GetKeyCodeEventSysRq)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_BREAK_0100
     * @tc.name    : GetKeyCodeEventBreak
     * @tc.desc    : Test whether the key KEY_BREAK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_BREAK_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2080, keyCodeNdk.GetKeyCodeEventBreak)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_MOVE_HOME_0100
     * @tc.name    : GetKeyCodeEventMoveHome
     * @tc.desc    : Test whether the key KEY_MOVE_HOME are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_MOVE_HOME_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2081, keyCodeNdk.GetKeyCodeEventMoveHome)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_MOVE_END_0100
     * @tc.name    : GetKeyCodeEventMoveEnd
     * @tc.desc    : Test whether the key KEY_MOVE_END are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_MOVE_END_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2082, keyCodeNdk.GetKeyCodeEventMoveEnd)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_INSERT_0100
     * @tc.name    : GetKeyCodeEventInsert
     * @tc.desc    : Test whether the key KEY_INSERT are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_INSERT_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2083, keyCodeNdk.GetKeyCodeEventInsert)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_FORWARD_0100
     * @tc.name    : GetKeyCodeEventForWord
     * @tc.desc    : Test whether the key KEY_FORWARD are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_FORWARD_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2084, keyCodeNdk.GetKeyCodeEventForward)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_MEDIA_PLAY_0100
     * @tc.name    : GetKeyCodeEventMediaPlay
     * @tc.desc    : Test whether the key KEY_MEDIA_PLAY are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_MEDIA_PLAY_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2085, keyCodeNdk.GetKeyCodeEventMediaPlay)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_MEDIA_PAUSE_0100
     * @tc.name    : GetKeyCodeEventMediaPause
     * @tc.desc    : Test whether the key KEY_MEDIA_PAUSE are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_MEDIA_PAUSE_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2086, keyCodeNdk.GetKeyCodeEventMediaPause)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_MEDIA_CLOSE_0100
     * @tc.name    : GetKeyCodeEventMediaClose
     * @tc.desc    : Test whether the key KEY_MEDIA_CLOSE are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_MEDIA_CLOSE_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2087, keyCodeNdk.GetKeyCodeEventMediaClose)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_MEDIA_EJECT_0100
     * @tc.name    : GetKeyCodeEventMediaEject
     * @tc.desc    : Test whether the key KEY_MEDIA_EJECT are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_MEDIA_EJECT_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2088, keyCodeNdk.GetKeyCodeEventMediaEject)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_MEDIA_RECORD_0100
     * @tc.name    : GetKeyCodeEventMediaRecord
     * @tc.desc    : Test whether the key KEY_MEDIA_RECORD are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_MEDIA_RECORD_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2089, keyCodeNdk.GetKeyCodeEventMediaRecord)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_F1_0100
     * @tc.name    : GetKeyCodeEventF1
     * @tc.desc    : Test whether the key KEY_F1 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_F1_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2090, keyCodeNdk.GetKeyCodeEventF1)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_F2_0100
     * @tc.name    : GetKeyCodeEventF2
     * @tc.desc    : Test whether the key KEY_F2 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_F2_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2091, keyCodeNdk.GetKeyCodeEventF2)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_F3_0100
     * @tc.name    : GetKeyCodeEventF3
     * @tc.desc    : Test whether the key KEY_F3 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_F3_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2092, keyCodeNdk.GetKeyCodeEventF3)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_F4_0100
     * @tc.name    : GetKeyCodeEventF4
     * @tc.desc    : Test whether the key KEY_F4 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_F4_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2093, keyCodeNdk.GetKeyCodeEventF4)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_F5_0100
     * @tc.name    : GetKeyCodeEventF5
     * @tc.desc    : Test whether the key KEY_F5 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_F5_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2094, keyCodeNdk.GetKeyCodeEventF5)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_F6_0100
     * @tc.name    : GetKeyCodeEventF6
     * @tc.desc    : Test whether the key KEY_F6 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_F6_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2095, keyCodeNdk.GetKeyCodeEventF6)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_F7_0100
     * @tc.name    : GetKeyCodeEventF7
     * @tc.desc    : Test whether the key KEY_F7 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_F7_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2096, keyCodeNdk.GetKeyCodeEventF7)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_F8_0100
     * @tc.name    : GetKeyCodeEventF8
     * @tc.desc    : Test whether the key KEY_F8 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_F8_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2097, keyCodeNdk.GetKeyCodeEventF8)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_F9_0100
     * @tc.name    : GetKeyCodeEventF9
     * @tc.desc    : Test whether the key KEY_F9 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_F9_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2098, keyCodeNdk.GetKeyCodeEventF9)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_F10_0100
     * @tc.name    : GetKeyCodeEventF10
     * @tc.desc    : Test whether the key KEY_F10 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_F10_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2099, keyCodeNdk.GetKeyCodeEventF10)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_F11_0100
     * @tc.name    : GetKeyCodeEventF11
     * @tc.desc    : Test whether the key KEY_F11 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_F11_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2100, keyCodeNdk.GetKeyCodeEventF11)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_F12_0100
     * @tc.name    : GetKeyCodeEventF12
     * @tc.desc    : Test whether the key KEY_F12 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_F12_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2101, keyCodeNdk.GetKeyCodeEventF12)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_NUM_LOCK_0100
     * @tc.name    : GetKeyCodeEventNumLock
     * @tc.desc    : Test whether the key KEY_NUM_LOCK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_NUM_LOCK_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2102, keyCodeNdk.GetKeyCodeEventNumLock)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_0_0100
     * @tc.name    : GetKeyCodeEventNumpad0
     * @tc.desc    : Test whether the key KEY_NUMPAD_0 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_0_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2103, keyCodeNdk.GetKeyCodeEventNumpad0)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_1_0100
     * @tc.name    : GetKeyCodeEventNumpad1
     * @tc.desc    : Test whether the key KEY_NUMPAD_1 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_1_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2104, keyCodeNdk.GetKeyCodeEventNumpad1)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_2_0100
     * @tc.name    : GetKeyCodeEventNumpad2
     * @tc.desc    : Test whether the key KEY_NUMPAD_2 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_2_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2105, keyCodeNdk.GetKeyCodeEventNumpad2)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_3_0100
     * @tc.name    : GetKeyCodeEventNumpad3
     * @tc.desc    : Test whether the key KEY_NUMPAD_3 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_3_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2106, keyCodeNdk.GetKeyCodeEventNumpad3)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_4_0100
     * @tc.name    : GetKeyCodeEventNumpad4
     * @tc.desc    : Test whether the key KEY_NUMPAD_4 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_4_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2107, keyCodeNdk.GetKeyCodeEventNumpad4)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_5_0100
     * @tc.name    : GetKeyCodeEventNumpad5
     * @tc.desc    : Test whether the key KEY_NUMPAD_5 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_5_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2108, keyCodeNdk.GetKeyCodeEventNumpad5)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_6_0100
     * @tc.name    : GetKeyCodeEventNumpad6
     * @tc.desc    : Test whether the key KEY_NUMPAD_6 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_6_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2109, keyCodeNdk.GetKeyCodeEventNumpad6)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_7_0100
     * @tc.name    : GetKeyCodeEventNumpad7
     * @tc.desc    : Test whether the key KEY_NUMPAD_7 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_7_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2110, keyCodeNdk.GetKeyCodeEventNumpad7)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_8_0100
     * @tc.name    : GetKeyCodeEventNumpad8
     * @tc.desc    : Test whether the key KEY_NUMPAD_8 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_8_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2111, keyCodeNdk.GetKeyCodeEventNumpad8)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_9_0100
     * @tc.name    : GetKeyCodeEventNumpad9
     * @tc.desc    : Test whether the key KEY_NUMPAD_9 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_9_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2112, keyCodeNdk.GetKeyCodeEventNumpad9)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_DIVIDE_0100
     * @tc.name    : GetKeyCodeEventNumpadDivide
     * @tc.desc    : Test whether the key KEY_NUMPAD_DIVIDE are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_DIVIDE_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2113, keyCodeNdk.GetKeyCodeEventNumpadDivide)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_MULTIPLY_0100
     * @tc.name    : GetKeyCodeEventNumpadMultiply
     * @tc.desc    : Test whether the key KEY_NUMPAD_MULTIPLY are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_MULTIPLY_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2114, keyCodeNdk.GetKeyCodeEventNumpadMultiply)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_SUBTRACT_0100
     * @tc.name    : GetKeyCodeEventNumpadSubtract
     * @tc.desc    : Test whether the key KEY_NUMPAD_SUBTRACT are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_SUBTRACT_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2115, keyCodeNdk.GetKeyCodeEventNumpadSubtract)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_ADD_0100
     * @tc.name    : GetKeyCodeEventNumpadAdd
     * @tc.desc    : Test whether the key KEY_NUMPAD_ADD are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_ADD_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2116, keyCodeNdk.GetKeyCodeEventNumpadAdd)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_DOT_0100
     * @tc.name    : GetKeyCodeEventNumpadDot
     * @tc.desc    : Test whether the key KEY_NUMPAD_DOT are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_DOT_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2117, keyCodeNdk.GetKeyCodeEventNumpadDot)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_COMMA_0100
     * @tc.name    : GetKeyCodeEventNumpadComma
     * @tc.desc    : Test whether the key KEY_NUMPAD_COMMA are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_COMMA_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2118, keyCodeNdk.GetKeyCodeEventNumpadComma)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_ENTER_0100
     * @tc.name    : GetKeyCodeEventNumpadEnter
     * @tc.desc    : Test whether the key KEY_NUMPAD_ENTER are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_ENTER_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2119, keyCodeNdk.GetKeyCodeEventNumpadEnter)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_EQUALS_0100
     * @tc.name    : GetKeyCodeEventNumpadEquals
     * @tc.desc    : Test whether the key KEY_NUMPAD_EQUALS are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_EQUALS_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2120, keyCodeNdk.GetKeyCodeEventNumpadEquals)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_LEFT_PAREN_0100
     * @tc.name    : GetKeyCodeEventNumpadLeftParen
     * @tc.desc    : Test whether the key KEY_NUMPAD_LEFT_PAREN are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_LEFT_PAREN_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2121, keyCodeNdk.GetKeyCodeEventNumpadLeftParen)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_RIGHT_PAREN_0100
     * @tc.name    : GetKeyCodeEventNumpadRightParen
     * @tc.desc    : Test whether the key KEY_NUMPAD_RIGHT_PAREN are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_RIGHT_PAREN_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2122, keyCodeNdk.GetKeyCodeEventNumpadRightParen)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_VIRTUAL_MULTITASK_0100
     * @tc.name    : GetKeyCodeEventVirtualMultitask
     * @tc.desc    : Test whether the key KEY_VIRTUAL_MULTITASK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_VIRTUAL_MULTITASK_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2210, keyCodeNdk.GetKeyCodeEventVirtualMultitask)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_SLEEP_0100
     * @tc.name    : GetKeyCodeEventSleep
     * @tc.desc    : Test whether the key KEY_SLEEP are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_SLEEP_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2600, keyCodeNdk.GetKeyCodeEventSleep)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_ZENKAKU_HANKAKU_0100
     * @tc.name    : GetKeyCodeEventZenkakuHanKaKu
     * @tc.desc    : Test whether the key KEY_ZENKAKU_HANKAKU are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_ZENKAKU_HANKAKU_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2601, keyCodeNdk.GetKeyCodeEventZenkakuHankaku)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_102ND_0100
     * @tc.name    : GetKeyCodeEvent102nd
     * @tc.desc    : Test whether the key KEY_102ND are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_102ND_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2602, keyCodeNdk.GetKeyCodeEvent102nd)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_RO_0100
     * @tc.name    : GetKeyCodeEventRO
     * @tc.desc    : Test whether the key KEY_RO are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_RO_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2603, keyCodeNdk.GetKeyCodeEventRo)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_KATAKANA_0100
     * @tc.name    : GetKeyCodeEventKatakana
     * @tc.desc    : Test whether the key KEY_KATAKANA are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_KATAKANA_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2604, keyCodeNdk.GetKeyCodeEventKatakana)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_HIRAGANA_0100
     * @tc.name    : GetKeyCodeEventHiragana
     * @tc.desc    : Test whether the key KEY_HIRAGANA are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_HIRAGANA_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2605, keyCodeNdk.GetKeyCodeEventHiragana)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_HENKAN_0100
     * @tc.name    : GetKeyCodeEventHenKan
     * @tc.desc    : Test whether the key KEY_HENKAN are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_HENKAN_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2606, keyCodeNdk.GetKeyCodeEventHenkan)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_KATAKANA_HIRAGANA_0100
     * @tc.name    : GetKeyCodeEventKatakanaHiragana
     * @tc.desc    : Test whether the key KEY_KATAKANA_HIRAGANA are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_KATAKANA_HIRAGANA_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2607, keyCodeNdk.GetKeyCodeEventKatakanaHiragana)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_MUHENKAN_0100
     * @tc.name    : GetKeyCodeEventMuhenkan
     * @tc.desc    : Test whether the key KEY_MUHENKAN are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_MUHENKAN_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2608, keyCodeNdk.GetKeyCodeEventMuhenkan)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_LINEFEED_0100
     * @tc.name    : GetKeyCodeEventLinefeed
     * @tc.desc    : Test whether the key KEY_LINEFEED are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_LINEFEED_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2609, keyCodeNdk.GetKeyCodeEventLinefeed)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_MACRO_0100
     * @tc.name    : GetKeyCodeEventMacro
     * @tc.desc    : Test whether the key KEY_MACRO are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_MACRO_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2610, keyCodeNdk.GetKeyCodeEventMacro)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_PLUSMINUS_0100
     * @tc.name    : GetKeyCodeEventNumpadPlusminus
     * @tc.desc    : Test whether the key KEY_NUMPAD_PLUSMINUS are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_NUMPAD_PLUSMINUS_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2611, keyCodeNdk.GetKeyCodeEventNumpadPlusminus)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_SCALE_0100
     * @tc.name    : GetKeyCodeEventScale
     * @tc.desc    : Test whether the key KEY_SCALE are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_SCALE_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2612, keyCodeNdk.GetKeyCodeEventScale)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_HANGUEL_0100
     * @tc.name    : GetKeyCodeEventHanguel
     * @tc.desc    : Test whether the key KEY_HANGUEL are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_HANGUEL_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2613, keyCodeNdk.GetKeyCodeEventHanguel)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_HANJA_0100
     * @tc.name    : GetKeyCodeEventHanJa
     * @tc.desc    : Test whether the key KEY_HANJA are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_HANJA_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2614, keyCodeNdk.GetKeyCodeEventHanja)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_YEN_0100
     * @tc.name    : GetKeyCodeEventYen
     * @tc.desc    : Test whether the key KEY_YEN are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_YEN_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2615, keyCodeNdk.GetKeyCodeEventYen)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_STOP_0100
     * @tc.name    : GetKeyCodeEventStop
     * @tc.desc    : Test whether the key KEY_STOP are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_STOP_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2616, keyCodeNdk.GetKeyCodeEventStop)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_AGAIN_0100
     * @tc.name    : GetKeyCodeEventAgain
     * @tc.desc    : Test whether the key KEY_AGAIN are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_AGAIN_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2617, keyCodeNdk.GetKeyCodeEventAgain)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_PROPS_0100
     * @tc.name    : GetKeyCodeEventProps
     * @tc.desc    : Test whether the key KEY_PROPS are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_PROPS_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2618, keyCodeNdk.GetKeyCodeEventProps)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_UNDO_0100
     * @tc.name    : GetKeyCodeEventUndo
     * @tc.desc    : Test whether the key KEY_UNDO are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_UNDO_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2619, keyCodeNdk.GetKeyCodeEventUndo)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_COPY_0100
     * @tc.name    : GetKeyCodeEventCopy
     * @tc.desc    : Test whether the key KEY_COPY are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_COPY_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2620, keyCodeNdk.GetKeyCodeEventCopy)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_OPEN_0100
     * @tc.name    : GetKeyCodeEventOpen
     * @tc.desc    : Test whether the key KEY_OPEN are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_OPEN_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2621, keyCodeNdk.GetKeyCodeEventOpen)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_PASTE_0100
     * @tc.name    : GetKeyCodeEventPaste
     * @tc.desc    : Test whether the key KEY_PASTE are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_PASTE_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2622, keyCodeNdk.GetKeyCodeEventPaste)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_FIND_0100
     * @tc.name    : GetKeyCodeEventFind
     * @tc.desc    : Test whether the key KEY_FIND are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_FIND_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2623, keyCodeNdk.GetKeyCodeEventFind)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_CUT_0100
     * @tc.name    : GetKeyCodeEventCut
     * @tc.desc    : Test whether the key KEY_CUT are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_CUT_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2624, keyCodeNdk.GetKeyCodeEventCut)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_HELP_0100
     * @tc.name    : GetKeyCodeEventHelp
     * @tc.desc    : Test whether the key KEY_HELP are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_HELP_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2625, keyCodeNdk.GetKeyCodeEventHelp)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_CALC_0100
     * @tc.name    : GetKeyCodeEventCalc
     * @tc.desc    : Test whether the key KEY_CALC are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_CALC_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2626, keyCodeNdk.GetKeyCodeEventCalc)
    /**
     * @tc.number  : SUB_MULTIMODALINPUT_KEYCODE_KEY_FILE_0100
     * @tc.name    : GetKeyCodeEventFile
     * @tc.desc    : Test whether the key KEY_FILE are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_MULTIMODALINPUT_KEYCODE_KEY_FILE_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2627, keyCodeNdk.GetKeyCodeEventFile)
  })
}
