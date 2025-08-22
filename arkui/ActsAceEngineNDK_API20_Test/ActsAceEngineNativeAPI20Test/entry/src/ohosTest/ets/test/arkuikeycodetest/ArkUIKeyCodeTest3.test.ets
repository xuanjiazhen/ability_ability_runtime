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

import { describe, beforeAll, it, expect, TestType, Size, Level } from '@ohos/hypium'
import arkuikeycodendk from 'libarkuikeycodendk.so'
import { Driver } from '@kit.TestKit';
import Utils from '../common/Utils';

const KEY_CODE_EVENT_OK = 1;
const KEY_CODE_EVENT_ERROR = 0;

export default function ArkUIKeyCodeNdkTest3() {
  describe('ArkUIKeyCodeNdkTest3', () => {
    beforeAll(async (done: Function) => {
      await Utils.pushPage(`ArkUIKeyCodeTest/ArkUIKeyCodeTest3`, done);
    });

    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_SEMICOLON_0100
 * @tc.name    : testARKUIKEYCODESEMICOLON_0100
 * @tc.desc    : testing ARKUI_KEYCODE_SEMICOLON interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODESEMICOLON_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODESEMICOLON_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2062);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventSemicolon();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODESEMICOLON_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_APOSTROPHE_0100
 * @tc.name    : testARKUIKEYCODEAPOSTROPHE_0100
 * @tc.desc    : testing ARKUI_KEYCODE_APOSTROPHE interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEAPOSTROPHE_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEAPOSTROPHE_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2063);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventApostrophe();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEAPOSTROPHE_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_SLASH_0100
 * @tc.name    : testARKUIKEYCODESLASH_0100
 * @tc.desc    : testing ARKUI_KEYCODE_SLASH interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODESLASH_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODESLASH_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2064);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventSlash();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODESLASH_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_AT_0100
 * @tc.name    : testARKUIKEYCODEAT_0100
 * @tc.desc    : testing ARKUI_KEYCODE_AT interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEAT_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEAT_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2065);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventAt();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEAT_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_PLUS_0100
 * @tc.name    : testARKUIKEYCODEPLUS_0100
 * @tc.desc    : testing ARKUI_KEYCODE_PLU interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEPLUS_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEPLUS_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2066);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventPlus();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEPLUS_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_MENU_0100
 * @tc.name    : testARKUIKEYCODEMENU_0100
 * @tc.desc    : testing ARKUI_KEYCODE_MENU interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEMENU_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEMENU_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2067);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventMenu();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEMENU_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_PAGE_UP_0100
 * @tc.name    : testARKUIKEYCODEPAGEUP_0100
 * @tc.desc    : testing ARKUI_KEYCODE_PAGE_UP interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEPAGEUP_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEPAGEUP_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2068);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventPageUp();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEPAGEUP_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_PAGE_DOWN_0100
 * @tc.name    : testARKUIKEYCODEPAGEDOWN_0100
 * @tc.desc    : testing ARKUI_KEYCODE_PAGE_DOWN interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEPAGEDOWN_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEPAGEDOWN_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2069);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventPageDown();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEPAGEDOWN_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_ESCAPE_0100
 * @tc.name    : testARKUIKEYCODEESCAPE_0100
 * @tc.desc    : testing ARKUI_KEYCODE_ESCAPE interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEESCAPE_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEESCAPE_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2070);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventEscape();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEESCAPE_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_FORWARD_DEL_0100
 * @tc.name    : testARKUIKEYCODEFORWARDDEL_0100
 * @tc.desc    : testing ARKUI_KEYCODE_FORWARD_DEL interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEFORWARDDEL_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEFORWARDDEL_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2071);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventForwardDel();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEFORWARDDEL_0100  catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_CTRL_LEFT_0100
 * @tc.name    : testARKUIKEYCODECTRLLEFT_0100
 * @tc.desc    : testing ARKUI_KEYCODE_CTRL_LEFT interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODECTRLLEFT_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODECTRLLEFT_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2072);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventCtrlLeft();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODECTRLLEFT_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_CTRL_RIGHT_0100
 * @tc.name    : testARKUIKEYCODECTRLRIGHT_0100
 * @tc.desc    : testing ARKUI_KEYCODE_CTRL_RIGHT interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODECTRLRIGHT_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODECTRLRIGHT_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2073);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventCtrlRight();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODECTRLRIGHT_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_CAPS_LOCK_0100
 * @tc.name    : testARKUIKEYCODECAPSLOCK_0100
 * @tc.desc    : testing ARKUI_KEYCODE_CAPS_LOCK interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODECAPSLOCK_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODECAPSLOCK_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2074);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventCapsLock();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODECAPSLOCK_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_SCROLL_LOCK_0100
 * @tc.name    : testARKUIKEYCODESCROLLLOCK_0100
 * @tc.desc    : testing ARKUI_KEYCODE_SCROLL_LOCK interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODESCROLLLOCK_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODESCROLLLOCK_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2075);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventScrollLock();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODESCROLLLOCK_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_META_LEFT_0100
 * @tc.name    : testARKUIKEYCODEMETALEFT_0100
 * @tc.desc    : testing SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_META_LEFT interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEMETALEFT_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEMETALEFT_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2076);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventMetaLeft();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEMETALEFT_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_META_RIGHT_0100
 * @tc.name    : testARKUIKEYCODEMETARIGHT_0100
 * @tc.desc    : testing SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_META_RIGHT interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEMETARIGHT_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEMETARIGHT_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2077);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventMetaRight();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEMETARIGHT_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_FUNCTION_0100
 * @tc.name    : testARKUIKEYCODEFUNCTION_0100
 * @tc.desc    : testing SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_FUNCTION interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEFUNCTION_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEFUNCTION_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2078);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventFunction();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEFUNCTION_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_SYSRQ_0100
 * @tc.name    : testARKUIKEYCODESYSRQ_0100
 * @tc.desc    : testing ARKUI_KEYCODE_SYSRQ interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODESYSRQ_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODESYSRQ_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2079);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventSysrq();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODESYSRQ_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_BREAK_0100
 * @tc.name    : testARKUIKEYCODEBREAK_0100
 * @tc.desc    : testing ARKUI_KEYCODE_BREAK interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEBREAK_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEBREAK_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2080);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventBreak();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEBREAK_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_MOVE_HOME_0100
 * @tc.name    : testARKUIKEYCODEMOVEHOME_0100
 * @tc.desc    : testing ARKUI_KEYCODE_MOVE_HOME interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEMOVEHOME_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEMOVEHOME_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2081);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventMoveHome();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEMOVEHOME_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_MOVE_END_0100
 * @tc.name    : testARKUIKEYCODEMOVEEND_0100
 * @tc.desc    : testing ARKUI_KEYCODE_MOVE_END interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEMOVEEND_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEMOVEEND_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2082);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventMoveEnd();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEMOVEEND_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_INSERT_0100
 * @tc.name    : testARKUIKEYCODEINSERT_0100
 * @tc.desc    : testing ARKUI_KEYCODE_INSERT interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEINSERT_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEINSERT_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2083);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventInsert();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEINSERT_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_FORWARD_0100
 * @tc.name    : testARKUIKEYCODEFORWARD_0100
 * @tc.desc    : testing ARKUI_KEYCODE_FORWARD interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEFORWARD_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEFORWARD_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2084);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventForward();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEFORWARD_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_MEDIA_PLAY_0100
 * @tc.name    : testARKUIKEYCODEMEDIAPLAY_0100
 * @tc.desc    : testing ARKUI_KEYCODE_MEDIA_PLAY interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEMEDIAPLAY_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEMEDIAPLAY_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2085);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventMediaPlay();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEMEDIAPLAY_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_MEDIA_PAUSE_0100
 * @tc.name    : testARKUIKEYCODEMEDIAPAUSE_0100
 * @tc.desc    : testing ARKUI_KEYCODE_MEDIA_PAUSE interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEMEDIAPAUSE_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEMEDIAPAUSE_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2086);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventMediaPause();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEMEDIAPAUSE_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_MEDIA_CLOSE_0100
 * @tc.name    : testARKUIKEYCODEMEDIACLOSE_0100
 * @tc.desc    : testing ARKUI_KEYCODE_MEDIA_CLOSE interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEMEDIACLOSE_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEMEDIACLOSE_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2087);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventMediaClose();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEMEDIACLOSE_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_MEDIA_EJECT_0100
 * @tc.name    : testARKUIKEYCODEMEDIAEJECT_0100
 * @tc.desc    : testing ARKUI_KEYCODE_MEDIA_EJECT interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEMEDIAEJECT_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEMEDIAEJECT_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2088);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventMediaEject();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEMEDIAEJECT_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_MEDIA_RECORD_0100
 * @tc.name    : testARKUIKEYCODEMEDIARECORD_0100
 * @tc.desc    : testing ARKUI_KEYCODE_MEDIA_RECORD interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEMEDIARECORD_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEMEDIARECORD_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2089);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventMediaRecord();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEMEDIARECORD_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_F1_0100
 * @tc.name    : testARKUIKEYCODEF1_0100
 * @tc.desc    : testing ARKUI_KEYCODE_F1 interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEF1_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEF1_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2090);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventF1();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEF1_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_F2_0100
 * @tc.name    : testARKUIKEYCODEF2_0100
 * @tc.desc    : testing ARKUI_KEYCODE_F2 interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEF2_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEF2_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2091);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventF2();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEF2_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_F3_0100
 * @tc.name    : testARKUIKEYCODEF3_0100
 * @tc.desc    : testing ARKUI_KEYCODE_F3 interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEF3_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEF3_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2092);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventF3();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEF3_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_F4_0100
 * @tc.name    : testARKUIKEYCODEF4_0100
 * @tc.desc    : testing ARKUI_KEYCODE_F4 interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEF4_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEF4_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2093);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventF4();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEF4_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_F5_0100
 * @tc.name    : testARKUIKEYCODEF5_0100
 * @tc.desc    : testing ARKUI_KEYCODE_F5 interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEF5_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEF5_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2094);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventF5();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEF5_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_F6_0100
 * @tc.name    : testARKUIKEYCODEF6_0100
 * @tc.desc    : testing ARKUI_KEYCODE_F6 interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEF6_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEF6_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2095);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventF6();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEF6_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_F7_0100
 * @tc.name    : testARKUIKEYCODEF7_0100
 * @tc.desc    : testing ARKUI_KEYCODE_F7 interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEF7_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEF7_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2096);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventF7();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEF7_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_F8_0100
 * @tc.name    : testARKUIKEYCODEF8_0100
 * @tc.desc    : testing ARKUI_KEYCODE_F8 interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEF8_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEF8_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2097);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventF8();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEF8_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })
    /**
 * @tc.number  : SUB_ArkUI_ACE_ENGINE_ARKUI_KEYCODE_F9_0100
 * @tc.name    : testARKUIKEYCODEF9_0100
 * @tc.desc    : testing ARKUI_KEYCODE_F9 interface
 * @tc.level   : LEVEL1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('testARKUIKEYCODEF9_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
    async (done: Function) => {
        try {
            console.info("====>testARKUIKEYCODEF9_0100 start====");
            let driver = await Driver.create();
            Utils.sleep(1000);
            await driver.triggerKey(2098);
            Utils.sleep(1000);
            let result: number = arkuikeycodendk.GetArkUIKeyCodeEventF9();
            expect(KEY_CODE_EVENT_OK).assertEqual(result);
            done();
        } catch (err) {
            console.error("====>testARKUIKEYCODEF9_0100 catch err: " + err);
            expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
    })


  })
}
