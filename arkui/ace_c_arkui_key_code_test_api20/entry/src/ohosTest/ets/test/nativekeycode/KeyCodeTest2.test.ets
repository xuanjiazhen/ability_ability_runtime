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

import { describe, beforeAll, TestType, Size, Level, afterAll } from '@ohos/hypium'
import keyCodeNdk from 'libkeycodendk.so'
import Utils from '../common/Utils';

export default function KeyCodeNdkTest2() {
  describe('KeyCodeNdkTest2', () => {
    beforeAll(async (done: Function) => {
      await Utils.pushPageNoClear(`KeyCodeTest/KeyCodeTest2`, done);
    });
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_CTRL_LEFT_0100
     * @tc.name    : testGetKeyCodeEventCtrlLeft0100
     * @tc.desc    : Test whether the key KEY_CTRL_LEFT are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventCtrlLeft0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2072, keyCodeNdk.GetKeyCodeEventCtrlLeft)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_CTRL_RIGHT_0100
     * @tc.name    : testGetKeyCodeEventCtrlRight0100
     * @tc.desc    : Test whether the key KEY_CTRL_RIGHT are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventCtrlRight0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2073, keyCodeNdk.GetKeyCodeEventCtrlRight)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_CAPS_LOCK_0100
     * @tc.name    : testGetKeyCodeEventCapsLock0100
     * @tc.desc    : Test whether the key KEY_CAPS_LOCK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventCapsLock0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2074, keyCodeNdk.GetKeyCodeEventCapsLock)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_SCROLL_LOCK_0100
     * @tc.name    : testGetKeyCodeEventScrollLock0100
     * @tc.desc    : Test whether the key KEY_SCROLL_LOCK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventScrollLock0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2075, keyCodeNdk.GetKeyCodeEventScrollLock)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_META_LEFT_0100
     * @tc.name    : testGetKeyCodeEventMetaLeft0100
     * @tc.desc    : Test whether the key KEY_META_LEFT are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventMetaLeft0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2076, keyCodeNdk.GetKeyCodeEventMetaLeft)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_META_RIGHT_0100
     * @tc.name    : testGetKeyCodeEventMetaRight0100
     * @tc.desc    : Test whether the key KEY_META_RIGHT are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventMetaRight0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2077, keyCodeNdk.GetKeyCodeEventMetaRight)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_FUNCTION_0100
     * @tc.name    : testGetKeyCodeEventFunction0100
     * @tc.desc    : Test whether the key KEY_FUNCTION are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventFunction0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2078, keyCodeNdk.GetKeyCodeEventFunction)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_SYSRQ_0100
     * @tc.name    : testGetKeyCodeEventSysRq0100
     * @tc.desc    : Test whether the key KEY_SYSRQ are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventSysRq0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2079, keyCodeNdk.GetKeyCodeEventSysRq)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_BREAK_0100
     * @tc.name    : testGetKeyCodeEventBreak0100
     * @tc.desc    : Test whether the key KEY_BREAK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventBreak0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2080, keyCodeNdk.GetKeyCodeEventBreak)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_MOVE_HOME_0100
     * @tc.name    : testGetKeyCodeEventMoveHome0100
     * @tc.desc    : Test whether the key KEY_MOVE_HOME are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventMoveHome0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2081, keyCodeNdk.GetKeyCodeEventMoveHome)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_MOVE_END_0100
     * @tc.name    : testGetKeyCodeEventMoveEnd0100
     * @tc.desc    : Test whether the key KEY_MOVE_END are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventMoveEnd0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2082, keyCodeNdk.GetKeyCodeEventMoveEnd)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_INSERT_0100
     * @tc.name    : testGetKeyCodeEventInsert0100
     * @tc.desc    : Test whether the key KEY_INSERT are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventInsert0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2083, keyCodeNdk.GetKeyCodeEventInsert)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_FORWARD_0100
     * @tc.name    : testGetKeyCodeEventForWord0100
     * @tc.desc    : Test whether the key KEY_FORWARD are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventForWord0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2084, keyCodeNdk.GetKeyCodeEventForward)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_MEDIA_PLAY_0100
     * @tc.name    : testGetKeyCodeEventMediaPlay0100
     * @tc.desc    : Test whether the key KEY_MEDIA_PLAY are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventMediaPlay0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2085, keyCodeNdk.GetKeyCodeEventMediaPlay)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_MEDIA_PAUSE_0100
     * @tc.name    : testGetKeyCodeEventMediaPause0100
     * @tc.desc    : Test whether the key KEY_MEDIA_PAUSE are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventMediaPause0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2086, keyCodeNdk.GetKeyCodeEventMediaPause)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_MEDIA_CLOSE_0100
     * @tc.name    : testGetKeyCodeEventMediaClose0100
     * @tc.desc    : Test whether the key KEY_MEDIA_CLOSE are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventMediaClose0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2087, keyCodeNdk.GetKeyCodeEventMediaClose)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_MEDIA_EJECT_0100
     * @tc.name    : testGetKeyCodeEventMediaEject0100
     * @tc.desc    : Test whether the key KEY_MEDIA_EJECT are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventMediaEject0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2088, keyCodeNdk.GetKeyCodeEventMediaEject)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_MEDIA_RECORD_0100
     * @tc.name    : testGetKeyCodeEventMediaRecord0100
     * @tc.desc    : Test whether the key KEY_MEDIA_RECORD are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventMediaRecord0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2089, keyCodeNdk.GetKeyCodeEventMediaRecord)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_F1_0100
     * @tc.name    : testGetKeyCodeEventF10100
     * @tc.desc    : Test whether the key KEY_F1 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventF01001', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2090, keyCodeNdk.GetKeyCodeEventF1)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_F2_0100
     * @tc.name    : testGetKeyCodeEventF20100
     * @tc.desc    : Test whether the key KEY_F2 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventF01002', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2091, keyCodeNdk.GetKeyCodeEventF2)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_F3_0100
     * @tc.name    : testGetKeyCodeEventF30100
     * @tc.desc    : Test whether the key KEY_F3 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventF01003', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2092, keyCodeNdk.GetKeyCodeEventF3)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_F4_0100
     * @tc.name    : testGetKeyCodeEventF40100
     * @tc.desc    : Test whether the key KEY_F4 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventF01004', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2093, keyCodeNdk.GetKeyCodeEventF4)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_F5_0100
     * @tc.name    : testGetKeyCodeEventF50100
     * @tc.desc    : Test whether the key KEY_F5 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventF01005', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2094, keyCodeNdk.GetKeyCodeEventF5)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_F6_0100
     * @tc.name    : testGetKeyCodeEventF60100
     * @tc.desc    : Test whether the key KEY_F6 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventF01006', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2095, keyCodeNdk.GetKeyCodeEventF6)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_F7_0100
     * @tc.name    : testGetKeyCodeEventF70100
     * @tc.desc    : Test whether the key KEY_F7 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventF01007', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2096, keyCodeNdk.GetKeyCodeEventF7)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_F8_0100
     * @tc.name    : testGetKeyCodeEventF80100
     * @tc.desc    : Test whether the key KEY_F8 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventF01008', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2097, keyCodeNdk.GetKeyCodeEventF8)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_F9_0100
     * @tc.name    : testGetKeyCodeEventF90100
     * @tc.desc    : Test whether the key KEY_F9 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventF01009', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2098, keyCodeNdk.GetKeyCodeEventF9)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_F10_0100
     * @tc.name    : testGetKeyCodeEventF100100
     * @tc.desc    : Test whether the key KEY_F10 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventF010010', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2099, keyCodeNdk.GetKeyCodeEventF10)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_F11_0100
     * @tc.name    : testGetKeyCodeEventF110100
     * @tc.desc    : Test whether the key KEY_F11 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventF010011', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2100, keyCodeNdk.GetKeyCodeEventF11)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_F12_0100
     * @tc.name    : testGetKeyCodeEventF120100
     * @tc.desc    : Test whether the key KEY_F12 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventF010012', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2101, keyCodeNdk.GetKeyCodeEventF12)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_NUM_LOCK_0100
     * @tc.name    : testGetKeyCodeEventNumLock0100
     * @tc.desc    : Test whether the key KEY_NUM_LOCK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventNumLock0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2102, keyCodeNdk.GetKeyCodeEventNumLock)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_NUMPAD_0_0100
     * @tc.name    : testGetKeyCodeEventNumpad00100
     * @tc.desc    : Test whether the key KEY_NUMPAD_0 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventNumpad01000', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2103, keyCodeNdk.GetKeyCodeEventNumpad0)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_NUMPAD_1_0100
     * @tc.name    : testGetKeyCodeEventNumpad10100
     * @tc.desc    : Test whether the key KEY_NUMPAD_1 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventNumpad01001', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2104, keyCodeNdk.GetKeyCodeEventNumpad1)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_NUMPAD_2_0100
     * @tc.name    : testGetKeyCodeEventNumpad20100
     * @tc.desc    : Test whether the key KEY_NUMPAD_2 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventNumpad01002', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2105, keyCodeNdk.GetKeyCodeEventNumpad2)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_NUMPAD_3_0100
     * @tc.name    : testGetKeyCodeEventNumpad30100
     * @tc.desc    : Test whether the key KEY_NUMPAD_3 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventNumpad01003', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2106, keyCodeNdk.GetKeyCodeEventNumpad3)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_NUMPAD_4_0100
     * @tc.name    : testGetKeyCodeEventNumpad40100
     * @tc.desc    : Test whether the key KEY_NUMPAD_4 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventNumpad01004', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2107, keyCodeNdk.GetKeyCodeEventNumpad4)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_NUMPAD_5_0100
     * @tc.name    : testGetKeyCodeEventNumpad50100
     * @tc.desc    : Test whether the key KEY_NUMPAD_5 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventNumpad01005', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2108, keyCodeNdk.GetKeyCodeEventNumpad5)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_NUMPAD_6_0100
     * @tc.name    : testGetKeyCodeEventNumpad60100
     * @tc.desc    : Test whether the key KEY_NUMPAD_6 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventNumpad01006', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2109, keyCodeNdk.GetKeyCodeEventNumpad6)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_NUMPAD_7_0100
     * @tc.name    : testGetKeyCodeEventNumpad70100
     * @tc.desc    : Test whether the key KEY_NUMPAD_7 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventNumpad01007', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2110, keyCodeNdk.GetKeyCodeEventNumpad7)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_NUMPAD_8_0100
     * @tc.name    : testGetKeyCodeEventNumpad80100
     * @tc.desc    : Test whether the key KEY_NUMPAD_8 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventNumpad01008', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2111, keyCodeNdk.GetKeyCodeEventNumpad8)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_NUMPAD_9_0100
     * @tc.name    : testGetKeyCodeEventNumpad90100
     * @tc.desc    : Test whether the key KEY_NUMPAD_9 are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventNumpad01009', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2112, keyCodeNdk.GetKeyCodeEventNumpad9)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_NUMPAD_DIVIDE_0100
     * @tc.name    : testGetKeyCodeEventNumpadDivide0100
     * @tc.desc    : Test whether the key KEY_NUMPAD_DIVIDE are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventNumpadDivide0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2113, keyCodeNdk.GetKeyCodeEventNumpadDivide)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_NUMPAD_MULTIPLY_0100
     * @tc.name    : testGetKeyCodeEventNumpadMultiply0100
     * @tc.desc    : Test whether the key KEY_NUMPAD_MULTIPLY are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventNumpadMultiply0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2114, keyCodeNdk.GetKeyCodeEventNumpadMultiply)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_NUMPAD_SUBTRACT_0100
     * @tc.name    : testGetKeyCodeEventNumpadSubtract0100
     * @tc.desc    : Test whether the key KEY_NUMPAD_SUBTRACT are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventNumpadSubtract0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2115, keyCodeNdk.GetKeyCodeEventNumpadSubtract)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_NUMPAD_ADD_0100
     * @tc.name    : testGetKeyCodeEventNumpadAdd0100
     * @tc.desc    : Test whether the key KEY_NUMPAD_ADD are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventNumpadAdd0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2116, keyCodeNdk.GetKeyCodeEventNumpadAdd)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_NUMPAD_DOT_0100
     * @tc.name    : testGetKeyCodeEventNumpadDot0100
     * @tc.desc    : Test whether the key KEY_NUMPAD_DOT are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventNumpadDot0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2117, keyCodeNdk.GetKeyCodeEventNumpadDot)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_NUMPAD_COMMA_0100
     * @tc.name    : testGetKeyCodeEventNumpadComma0100
     * @tc.desc    : Test whether the key KEY_NUMPAD_COMMA are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventNumpadComma0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2118, keyCodeNdk.GetKeyCodeEventNumpadComma)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_NUMPAD_ENTER_0100
     * @tc.name    : testGetKeyCodeEventNumpadEnter0100
     * @tc.desc    : Test whether the key KEY_NUMPAD_ENTER are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventNumpadEnter0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2119, keyCodeNdk.GetKeyCodeEventNumpadEnter)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_NUMPAD_EQUALS_0100
     * @tc.name    : testGetKeyCodeEventNumpadEquals0100
     * @tc.desc    : Test whether the key KEY_NUMPAD_EQUALS are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventNumpadEquals0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2120, keyCodeNdk.GetKeyCodeEventNumpadEquals)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_NUMPAD_LEFT_PAREN_0100
     * @tc.name    : testGetKeyCodeEventNumpadLeftParen0100
     * @tc.desc    : Test whether the key KEY_NUMPAD_LEFT_PAREN are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventNumpadLeftParen0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2121, keyCodeNdk.GetKeyCodeEventNumpadLeftParen)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_NUMPAD_RIGHT_PAREN_0100
     * @tc.name    : testGetKeyCodeEventNumpadRightParen0100
     * @tc.desc    : Test whether the key KEY_NUMPAD_RIGHT_PAREN are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventNumpadRightParen0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2122, keyCodeNdk.GetKeyCodeEventNumpadRightParen)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_VIRTUAL_MULTITASK_0100
     * @tc.name    : testGetKeyCodeEventVirtualMultitask0100
     * @tc.desc    : Test whether the key KEY_VIRTUAL_MULTITASK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventVirtualMultitask0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2210, keyCodeNdk.GetKeyCodeEventVirtualMultitask)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_SLEEP_0100
     * @tc.name    : testGetKeyCodeEventSleep0100
     * @tc.desc    : Test whether the key KEY_SLEEP are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventSleep0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2600, keyCodeNdk.GetKeyCodeEventSleep)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_ZENKAKU_HANKAKU_0100
     * @tc.name    : testGetKeyCodeEventZenkakuHanKaKu0100
     * @tc.desc    : Test whether the key KEY_ZENKAKU_HANKAKU are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventZenkakuHanKaKu0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2601, keyCodeNdk.GetKeyCodeEventZenkakuHankaku)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_102ND_0100
     * @tc.name    : testGetKeyCodeEvent0100102nd
     * @tc.desc    : Test whether the key KEY_102ND are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEvent0100102nd', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2602, keyCodeNdk.GetKeyCodeEvent102nd)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_RO_0100
     * @tc.name    : testGetKeyCodeEventRO0100
     * @tc.desc    : Test whether the key KEY_RO are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventRO0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2603, keyCodeNdk.GetKeyCodeEventRo)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_KATAKANA_0100
     * @tc.name    : testGetKeyCodeEventKatakana0100
     * @tc.desc    : Test whether the key KEY_KATAKANA are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventKatakana0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2604, keyCodeNdk.GetKeyCodeEventKatakana)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_HIRAGANA_0100
     * @tc.name    : testGetKeyCodeEventHiragana0100
     * @tc.desc    : Test whether the key KEY_HIRAGANA are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventHiragana0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2605, keyCodeNdk.GetKeyCodeEventHiragana)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_HENKAN_0100
     * @tc.name    : testGetKeyCodeEventHenKan0100
     * @tc.desc    : Test whether the key KEY_HENKAN are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventHenKan0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2606, keyCodeNdk.GetKeyCodeEventHenkan)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_KATAKANA_HIRAGANA_0100
     * @tc.name    : testGetKeyCodeEventKatakanaHiragana0100
     * @tc.desc    : Test whether the key KEY_KATAKANA_HIRAGANA are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventKatakanaHiragana0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2607, keyCodeNdk.GetKeyCodeEventKatakanaHiragana)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_MUHENKAN_0100
     * @tc.name    : testGetKeyCodeEventMuhenkan0100
     * @tc.desc    : Test whether the key KEY_MUHENKAN are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventMuhenkan0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2608, keyCodeNdk.GetKeyCodeEventMuhenkan)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_LINEFEED_0100
     * @tc.name    : testGetKeyCodeEventLinefeed0100
     * @tc.desc    : Test whether the key KEY_LINEFEED are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventLinefeed0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2609, keyCodeNdk.GetKeyCodeEventLinefeed)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_MACRO_0100
     * @tc.name    : testGetKeyCodeEventMacro0100
     * @tc.desc    : Test whether the key KEY_MACRO are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventMacro0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2610, keyCodeNdk.GetKeyCodeEventMacro)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_NUMPAD_PLUSMINUS_0100
     * @tc.name    : testGetKeyCodeEventNumpadPlusminus0100
     * @tc.desc    : Test whether the key KEY_NUMPAD_PLUSMINUS are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventNumpadPlusminus0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2611, keyCodeNdk.GetKeyCodeEventNumpadPlusminus)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_SCALE_0100
     * @tc.name    : testGetKeyCodeEventScale0100
     * @tc.desc    : Test whether the key KEY_SCALE are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventScale0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2612, keyCodeNdk.GetKeyCodeEventScale)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_HANGUEL_0100
     * @tc.name    : testGetKeyCodeEventHanguel0100
     * @tc.desc    : Test whether the key KEY_HANGUEL are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventHanguel0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2613, keyCodeNdk.GetKeyCodeEventHanguel)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_HANJA_0100
     * @tc.name    : testGetKeyCodeEventHanJa0100
     * @tc.desc    : Test whether the key KEY_HANJA are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventHanJa0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2614, keyCodeNdk.GetKeyCodeEventHanja)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_YEN_0100
     * @tc.name    : testGetKeyCodeEventYen0100
     * @tc.desc    : Test whether the key KEY_YEN are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventYen0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2615, keyCodeNdk.GetKeyCodeEventYen)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_STOP_0100
     * @tc.name    : testGetKeyCodeEventStop0100
     * @tc.desc    : Test whether the key KEY_STOP are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventStop0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2616, keyCodeNdk.GetKeyCodeEventStop)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_AGAIN_0100
     * @tc.name    : testGetKeyCodeEventAgain0100
     * @tc.desc    : Test whether the key KEY_AGAIN are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventAgain0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2617, keyCodeNdk.GetKeyCodeEventAgain)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_PROPS_0100
     * @tc.name    : testGetKeyCodeEventProps0100
     * @tc.desc    : Test whether the key KEY_PROPS are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventProps0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2618, keyCodeNdk.GetKeyCodeEventProps)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_UNDO_0100
     * @tc.name    : testGetKeyCodeEventUndo0100
     * @tc.desc    : Test whether the key KEY_UNDO are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventUndo0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2619, keyCodeNdk.GetKeyCodeEventUndo)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_COPY_0100
     * @tc.name    : testGetKeyCodeEventCopy0100
     * @tc.desc    : Test whether the key KEY_COPY are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventCopy0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2620, keyCodeNdk.GetKeyCodeEventCopy)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_OPEN_0100
     * @tc.name    : testGetKeyCodeEventOpen0100
     * @tc.desc    : Test whether the key KEY_OPEN are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventOpen0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2621, keyCodeNdk.GetKeyCodeEventOpen)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_PASTE_0100
     * @tc.name    : testGetKeyCodeEventPaste0100
     * @tc.desc    : Test whether the key KEY_PASTE are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventPaste0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2622, keyCodeNdk.GetKeyCodeEventPaste)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_FIND_0100
     * @tc.name    : testGetKeyCodeEventFind0100
     * @tc.desc    : Test whether the key KEY_FIND are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventFind0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2623, keyCodeNdk.GetKeyCodeEventFind)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_CUT_0100
     * @tc.name    : testGetKeyCodeEventCut0100
     * @tc.desc    : Test whether the key KEY_CUT are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventCut0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2624, keyCodeNdk.GetKeyCodeEventCut)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_HELP_0100
     * @tc.name    : testGetKeyCodeEventHelp0100
     * @tc.desc    : Test whether the key KEY_HELP are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventHelp0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2625, keyCodeNdk.GetKeyCodeEventHelp)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_CALC_0100
     * @tc.name    : testGetKeyCodeEventCalc0100
     * @tc.desc    : Test whether the key KEY_CALC are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventCalc0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2626, keyCodeNdk.GetKeyCodeEventCalc)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_FILE_0100
     * @tc.name    : testGetKeyCodeEventFile0100
     * @tc.desc    : Test whether the key KEY_FILE are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventFile0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2627, keyCodeNdk.GetKeyCodeEventFile)
  })
}
