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

import { describe, beforeAll, it, expect, TestType, Size, Level, afterAll } from '@ohos/hypium'
import keyCodeNdk from 'libkeycodendk.so'
import Utils from '../common/Utils';

export default function KeyCodeNdkTest() {
  describe('KeyCodeNdkTest', () => {
    beforeAll(async (done: Function) => {
      await Utils.pushPageNoClear(`KeyCodeTest/KeyCodeTest`, done);
    });

    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_APOSTROPHE_0100
     * @tc.name    : testGetKeyCodeEventApostrophe0100
     * @tc.desc    : testing KEY_APOSTROPHE interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventApostrophe0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2063, keyCodeNdk.GetKeyCodeEventApostrophe)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_GRAVE_0100
     * @tc.name    : testGetKeyCodeEventGrave0100
     * @tc.desc    : testing KEY_GRAVE interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventGrave0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2056, keyCodeNdk.GetKeyCodeEventGrave)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_MINUS_0100
     * @tc.name    : testGetKeyCodeEventMinus0100
     * @tc.desc    : testing KEY_MINUS interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventMinus0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2057, keyCodeNdk.GetKeyCodeEventMinus)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_EQUALS_0100
     * @tc.name    : testGetKeyCodeEventEquals0100
     * @tc.desc    : testing KEY_EQUALS interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventEquals0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2058, keyCodeNdk.GetKeyCodeEventEquals)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_LEFT_BRACKET_0100
     * @tc.name    : testGetKeyCodeEventLeftBracket0100
     * @tc.desc    : testing KEY_LEFT_BRACKET interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventLeftBracket0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2059, keyCodeNdk.GetKeyCodeEventLeftBracket)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_RIGHT_BRACKET_0100
     * @tc.name    : testGetKeyCodeEventRightBracket0100
     * @tc.desc    : testing KEY_RIGHT_BRACKET interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventRightBracket0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2060, keyCodeNdk.GetKeyCodeEventRightBracket)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_BACKSLASH_0100
     * @tc.name    : testGetKeyCodeEventBackslash0100
     * @tc.desc    : testing KEY_BACKSLASH interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventBackslash0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2061, keyCodeNdk.GetKeyCodeEventBackslash)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_SEMICOLON_0100
     * @tc.name    : testGetKeyCodeEventSemicolon0100
     * @tc.desc    : testing KEY_SEMICOLON interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventSemicolon0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2062, keyCodeNdk.GetKeyCodeEventSemicolon)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_SLASH_0100
     * @tc.name    : testGetKeyCodeEventSlash0100
     * @tc.desc    : testing KEY_SLASH interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventSlash0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2064, keyCodeNdk.GetKeyCodeEventSlash)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_AT_0100
     * @tc.name    : testGetKeyCodeEventAt0100
     * @tc.desc    : testing KEY_AT interface
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('testGetKeyCodeEventAt0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2065, keyCodeNdk.GetKeyCodeEventAt)  })
}
