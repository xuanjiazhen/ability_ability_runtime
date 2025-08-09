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

export default function KeyCodeNdkTest1() {
  describe('KeyCodeNdkTest1', () => {
    beforeAll(async (done: Function) => {
      await Utils.pushPageNoClear(`KeyCodeTest/KeyCodeTest1`, done);
    });
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_BACK_0100
     * @tc.name    : testGetKeyCodeEventBack0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_BACK_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2, keyCodeNdk.GetKeyCodeEventBack)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_MEDIA_PLAY_PAUSE_0100
     * @tc.name    : testGetKeyCodeEventMediaPlayPause0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_MEDIA_PLAY_PAUSE_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 10, keyCodeNdk.GetKeyCodeEventMediaPlayPause)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_MEDIA_STOP_0100
     * @tc.name    : testGetKeyCodeEventMediaStop0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_MEDIA_STOP_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 11, keyCodeNdk.GetKeyCodeEventMediaStop)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_MEDIA_NEXT_0100
     * @tc.name    : testGetKeyCodeEventMediaNext0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_MEDIA_NEXT_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 12, keyCodeNdk.GetKeyCodeEventMediaNext)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_MEDIA_PREVIOUS_0100
     * @tc.name    : testGetKeyCodeEventMediaPrevious0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_MEDIA_PREVIOUS_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 13, keyCodeNdk.GetKeyCodeEventMediaPrevious)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_MEDIA_REWIND_0100
     * @tc.name    : testGetKeyCodeEventMediaRewind0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_MEDIA_REWIND_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 14, keyCodeNdk.GetKeyCodeEventMediaRewind)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_MEDIA_FAST_FORWARD_0100
     * @tc.name    : testGetKeyCodeEventMediaFastForward0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_MEDIA_FAST_FORWARD_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 15, keyCodeNdk.GetKeyCodeEventMediaFastForward)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_VOLUME_UP_0100
     * @tc.name    : testGetKeyCodeEventVolumeUp0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_VOLUME_UP_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 16, keyCodeNdk.GetKeyCodeEventVolumeUp)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_VOLUME_DOWN_0100
     * @tc.name    : testGetKeyCodeEventVolumeDown0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_VOLUME_DOWN_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 17, keyCodeNdk.GetKeyCodeEventVolumeDown)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_CAMERA_0100
     * @tc.name    : testGetKeyCodeEventCamera0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_CAMERA_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 19, keyCodeNdk.GetKeyCodeEventCamera)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_VOLUME_MUTE_0100
     * @tc.name    : testGetKeyCodeEventVolumeMute0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_VOLUME_MUTE_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 22, keyCodeNdk.GetKeyCodeEventVolumeMute)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_MUTE_0100
     * @tc.name    : testGetKeyCodeEventMute0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_MUTE_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 23, keyCodeNdk.GetKeyCodeEventMute)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_BRIGHTNESS_UP_0100
     * @tc.name    : testGetKeyCodeEventBrightnessUp0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_BRIGHTNESS_UP_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 40, keyCodeNdk.GetKeyCodeEventBrightnessUp)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_BRIGHTNESS_DOWN_0100
     * @tc.name    : testGetKeyCodeEventBrightnessDown0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_BRIGHTNESS_DOWN_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 41, keyCodeNdk.GetKeyCodeEventBrightnessDown)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_0_0100
     * @tc.name    : testGetKeyCodeEvent01000
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_0_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2000, keyCodeNdk.GetKeyCodeEvent0)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_1_0100
     * @tc.name    : testGetKeyCodeEvent01001
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_1_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2001, keyCodeNdk.GetKeyCodeEvent1)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_2_0100
     * @tc.name    : testGetKeyCodeEvent01002
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_2_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2002, keyCodeNdk.GetKeyCodeEvent2)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_3_0100
     * @tc.name    : testGetKeyCodeEvent01003
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_3_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2003, keyCodeNdk.GetKeyCodeEvent3)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_4_0100
     * @tc.name    : testGetKeyCodeEvent01004
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_4_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2004, keyCodeNdk.GetKeyCodeEvent4)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_5_0100
     * @tc.name    : testGetKeyCodeEvent01005
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_5_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2005, keyCodeNdk.GetKeyCodeEvent5)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_6_0100
     * @tc.name    : testGetKeyCodeEvent01006
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_6_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2006, keyCodeNdk.GetKeyCodeEvent6)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_7_0100
     * @tc.name    : testGetKeyCodeEvent01007
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_7_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2007, keyCodeNdk.GetKeyCodeEvent7)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_8_0100
     * @tc.name    : testGetKeyCodeEvent01008
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_8_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2008, keyCodeNdk.GetKeyCodeEvent8)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_9_0100
     * @tc.name    : testGetKeyCodeEvent01009
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_9_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2009, keyCodeNdk.GetKeyCodeEvent9)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_STAR_0100
     * @tc.name    : testGetKeyCodeEventStar0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_STAR_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2010, keyCodeNdk.GetKeyCodeEventStar)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_POUND_0100
     * @tc.name    : testGetKeyCodeEventPound0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_POUND_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2011, keyCodeNdk.GetKeyCodeEventPound)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_DPAD_UP_0100
     * @tc.name    : testGetKeyCodeEventDpadUp0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_DPAD_UP_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2012, keyCodeNdk.GetKeyCodeEventDpadUp)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_DPAD_DOWN_0100
     * @tc.name    : testGetKeyCodeEventDpadDown0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_DPAD_DOWN_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2013, keyCodeNdk.GetKeyCodeEventDpadDown)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_DPAD_LEFT_0100
     * @tc.name    : testGetKeyCodeEventDpadLeft0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_DPAD_LEFT_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2014, keyCodeNdk.GetKeyCodeEventDpadLeft)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_DPAD_RIGHT_0100
     * @tc.name    : testGetKeyCodeEventDpadRight0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_DPAD_RIGHT_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2015, keyCodeNdk.GetKeyCodeEventDpadRight)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_DPAD_CENTER_0100
     * @tc.name    : testGetKeyCodeEventDpadCenter0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_DPAD_CENTER_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2016, keyCodeNdk.GetKeyCodeEventDpadCenter)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_A_0100
     * @tc.name    : testGetKeyCodeEventA0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_A_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2017, keyCodeNdk.GetKeyCodeEventA)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_B_0100
     * @tc.name    : testGetKeyCodeEventB0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_B_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2018, keyCodeNdk.GetKeyCodeEventB)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_C_0100
     * @tc.name    : testGetKeyCodeEventC0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_C_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2019, keyCodeNdk.GetKeyCodeEventC)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_D_0100
     * @tc.name    : testGetKeyCodeEventD0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_D_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2020, keyCodeNdk.GetKeyCodeEventD)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_E_0100
     * @tc.name    : testGetKeyCodeEventE0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_E_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2021, keyCodeNdk.GetKeyCodeEventE)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_F_0100
     * @tc.name    : testGetKeyCodeEventF0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_F_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2022, keyCodeNdk.GetKeyCodeEventF)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_G_0100
     * @tc.name    : testGetKeyCodeEventG0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_G_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2023, keyCodeNdk.GetKeyCodeEventG)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_H_0100
     * @tc.name    : testGetKeyCodeEventH0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_H_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2024, keyCodeNdk.GetKeyCodeEventH)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_I_0100
     * @tc.name    : testGetKeyCodeEventI0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_I_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2025, keyCodeNdk.GetKeyCodeEventI)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_J_0100
     * @tc.name    : testGetKeyCodeEventJ0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_J_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2026, keyCodeNdk.GetKeyCodeEventJ)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_K_0100
     * @tc.name    : testGetKeyCodeEventK0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_K_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2027, keyCodeNdk.GetKeyCodeEventK)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_L_0100
     * @tc.name    : testGetKeyCodeEventL0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_L_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2028, keyCodeNdk.GetKeyCodeEventL)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_M_0100
     * @tc.name    : testGetKeyCodeEventM0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_M_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2029, keyCodeNdk.GetKeyCodeEventM)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_N_0100
     * @tc.name    : testGetKeyCodeEventN0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_N_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2030, keyCodeNdk.GetKeyCodeEventN)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_O_0100
     * @tc.name    : testGetKeyCodeEventO0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_O_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2031, keyCodeNdk.GetKeyCodeEventO)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_P_0100
     * @tc.name    : testGetKeyCodeEventP0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_P_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2032, keyCodeNdk.GetKeyCodeEventP)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_Q_0100
     * @tc.name    : testGetKeyCodeEventQ0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_Q_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2033, keyCodeNdk.GetKeyCodeEventQ)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_R_0100
     * @tc.name    : testGetKeyCodeEventR0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_R_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2034, keyCodeNdk.GetKeyCodeEventR)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_S_0100
     * @tc.name    : testGetKeyCodeEventS0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_S_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2035, keyCodeNdk.GetKeyCodeEventS)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_T_0100
     * @tc.name    : testGetKeyCodeEventT0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_T_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2036, keyCodeNdk.GetKeyCodeEventT)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_U_0100
     * @tc.name    : testGetKeyCodeEventU0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_U_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2037, keyCodeNdk.GetKeyCodeEventU)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_V_0100
     * @tc.name    : testGetKeyCodeEventV0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_V_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2038, keyCodeNdk.GetKeyCodeEventV)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_W_0100
     * @tc.name    : testGetKeyCodeEventW0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_W_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2039, keyCodeNdk.GetKeyCodeEventW)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_X_0100
     * @tc.name    : testGetKeyCodeEventX0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_X_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2040, keyCodeNdk.GetKeyCodeEventX)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_Y_0100
     * @tc.name    : testGetKeyCodeEventY0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_Y_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2041, keyCodeNdk.GetKeyCodeEventY)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_Z_0100
     * @tc.name    : testGetKeyCodeEventZ0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_Z_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2042, keyCodeNdk.GetKeyCodeEventZ)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_COMMA_0100
     * @tc.name    : testGetKeyCodeEventComma0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_COMMA_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2043, keyCodeNdk.GetKeyCodeEventComma)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_PERIOD_0100
     * @tc.name    : testGetKeyCodeEventPeriod0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_PERIOD_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2044, keyCodeNdk.GetKeyCodeEventPeriod)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_ALT_LEFT_0100
     * @tc.name    : testGetKeyCodeEventAltLeft0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_ALT_LEFT_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2045, keyCodeNdk.GetKeyCodeEventAltLeft)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_ALT_RIGHT_0100
     * @tc.name    : testGetKeyCodeEventAltRight0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_ALT_RIGHT_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2046, keyCodeNdk.GetKeyCodeEventAltRight)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_SHIFT_LEFT_0100
     * @tc.name    : testGetKeyCodeEventShiftLeft0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_SHIFT_LEFT_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2047, keyCodeNdk.GetKeyCodeEventShiftLeft)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_SHIFT_RIGHT_0100
     * @tc.name    : testGetKeyCodeEventShiftRight0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_SHIFT_RIGHT_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2048, keyCodeNdk.GetKeyCodeEventShiftRight)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_TAB_0100
     * @tc.name    : testGetKeyCodeEventTab0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_TAB_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2049, keyCodeNdk.GetKeyCodeEventTab)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_SPACE_0100
     * @tc.name    : testGetKeyCodeEventSpace0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_SPACE_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2050, keyCodeNdk.GetKeyCodeEventSpace)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_SYM_0100
     * @tc.name    : testGetKeyCodeEventSym0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_SYM_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2051, keyCodeNdk.GetKeyCodeEventSym)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_EXPLORER_0100
     * @tc.name    : testGetKeyCodeEventExplorer0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_EXPLORER_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2052, keyCodeNdk.GetKeyCodeEventExplorer)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_ENVELOPE_0100
     * @tc.name    : testGetKeyCodeEventEnvelope0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_ENVELOPE_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2053, keyCodeNdk.GetKeyCodeEventEnvelope)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_ENTER_0100
     * @tc.name    : testGetKeyCodeEventEnter0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_ENTER_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2054, keyCodeNdk.GetKeyCodeEventEnter)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_DEL_0100
     * @tc.name    : testGetKeyCodeEventDel0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_DEL_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2055, keyCodeNdk.GetKeyCodeEventDel)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_PLUS_0100
     * @tc.name    : testGetKeyCodeEventPlus0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_PLUS_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2066, keyCodeNdk.GetKeyCodeEventPlus)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_MENU_0100
     * @tc.name    : testGetKeyCodeEventMenu0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_MENU_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2067, keyCodeNdk.GetKeyCodeEventMenu)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_PAGE_UP_0100
     * @tc.name    : testGetKeyCodeEventPageUp0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_PAGE_UP_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2068, keyCodeNdk.GetKeyCodeEventPageUp)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_PAGE_DOWN_0100
     * @tc.name    : testGetKeyCodeEventPageDown0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_PAGE_DOWN_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2069, keyCodeNdk.GetKeyCodeEventPageDown)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_ESCAPE_0100
     * @tc.name    : testGetKeyCodeEventEscape0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_ESCAPE_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2070, keyCodeNdk.GetKeyCodeEventEscape)
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_KEY_FORWARD_DEL_0100
     * @tc.name    : testGetKeyCodeEventForwardDel0100
     * @tc.desc    : Test whether the key KEY_ADDRESSBOOK are working properly.
     * @tc.level   : LEVEL1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    Utils.keydo('SUB_ARKUI_ACE_ENGINE_KEY_FORWARD_DEL_0100', TestType.FUNCTION |
        Size.MEDIUMTEST | Level.LEVEL1, 2071, keyCodeNdk.GetKeyCodeEventForwardDel)
  })
}
