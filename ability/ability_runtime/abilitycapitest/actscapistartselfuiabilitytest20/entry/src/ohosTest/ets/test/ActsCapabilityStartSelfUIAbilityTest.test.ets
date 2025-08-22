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
import AbilityKitTest from 'libentry.so'

const ABILITY_ERR_OK = 0;

export default function ActsCapabilityStartSelfUIAbilityTest() {
  describe('ActsCapabilityStartSelfUIAbilityTest', () => {

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_DestroyChildProcessConfigs_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_DestroyChildProcessConfigs_0100
     * @tc.desc    : Testing DestroyChildProcessConfigs returns an invalid parameter error code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_DestroyChildProcessConfigs_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_DestroyChildProcessConfigs_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.DestroyChildProcessConfigsParamInvalid();
          expect(401).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_DestroyChildProcessConfigs_0200
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_DestroyChildProcessConfigs_0200
     * @tc.desc    : Test DestroyChildProcessConfigs and return the correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_DestroyChildProcessConfigs_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_DestroyChildProcessConfigs_0200`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.DestroyChildProcessConfigsNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_ChildProcessConfigsSetIsolationMode_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_ChildProcessConfigsSetIsolationMode_0100
     * @tc.desc    : Testing ChildProcessConfigsSetIsolationMode returns an invalid parameter error code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_ChildProcessConfigsSetIsolationMode_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_ChildProcessConfigsSetIsolationMode_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ChildProcessConfigsSetIsolationModeInvalidParam();
          expect(401).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_ChildProcessConfigsSetIsolationMode_0200
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_ChildProcessConfigsSetIsolationMode_0200
     * @tc.desc    : Testing ChildProcessConfigsSetIsolationMode returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_ChildProcessConfigsSetIsolationMode_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_ChildProcessConfigsSetIsolationMode_0200`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ChildProcessConfigsSetIsolationModeNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_ChildProcessConfigsSetProcessName_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_ChildProcessConfigsSetProcessName_0100
     * @tc.desc    : Testing ChildProcessConfigsSetProcessName returns an invalid parameter error code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_ChildProcessConfigsSetProcessName_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_ChildProcessConfigsSetProcessName_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ChildProcessConfigsSetProcessNameInvalidParam();
          expect(401).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_ChildProcessConfigsSetProcessName_0200
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_ChildProcessConfigsSetProcessName_0200
     * @tc.desc    : Testing ChildProcessConfigsSetProcessName returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_ChildProcessConfigsSetProcessName_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_ChildProcessConfigsSetProcessName_0200`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ChildProcessConfigsSetProcessNameNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_RegisterNativeChildProcessExitCallback_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_RegisterNativeChildProcessExitCallback_0100
     * @tc.desc    : Testing RegisterNativeChildProcessExitCallback returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_RegisterNativeChildProcessExitCallback_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_RegisterNativeChildProcessExitCallback_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.RegisterNativeChildProcessExitCallbackNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_RegisterNativeChildProcessExitCallback_0200
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_RegisterNativeChildProcessExitCallback_0200
     * @tc.desc    : Testing RegisterNativeChildProcessExitCallback returns an invalid parameter error code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_RegisterNativeChildProcessExitCallback_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_RegisterNativeChildProcessExitCallback_0200`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.RegisterNativeChildProcessExitCallbackInvPar();
          expect(401).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_UnregisterNativeChildProcessExitCallback_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_UnregisterNativeChildProcessExitCallback_0100
     * @tc.desc    : Testing UnregisterNativeChildProcessExitCallback returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_UnregisterNativeChildProcessExitCallback_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_UnregisterNativeChildProcessExitCallback_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.UnregisterNativeChildProcessExitCallbackNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_UnregisterNativeChildProcessExitCallback_0200
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_UnregisterNativeChildProcessExitCallback_0200
     * @tc.desc    : Testing UnregisterNativeChildProcessExitCallback returns an invalid parameter error code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_UnregisterNativeChildProcessExitCallback_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_UnregisterNativeChildProcessExitCallback_0200`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.UnregisterNativeChildProcessExitCallbackInvPar();
          expect(401).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_UnRegisterNativeChildProcessExitCallback_0300
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_UnRegisterNativeChildProcessExitCallback_0300
     * @tc.desc    : Test UnregisterNativeChildProcessExitCallback and return the error code that there is no callback.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_UnRegisterNativeChildProcessExitCallback_0300',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_UnRegisterNativeChildProcessExitCallback_0300`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.UnregisterNativeChildProcessExitCallbackNotExist();
          expect(16010009).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityBase_DestroyWant_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityBase_DestroyWant_0100
     * @tc.desc    : Testing DestroyWant returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityBase_DestroyWant_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityBase_DestroyWant_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.DestroyWantNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityBase_SetWantElement_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityBase_SetWantElement_0100
     * @tc.desc    : Testing SetWantElement returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityBase_SetWantElement_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityBase_SetWantElement_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.SetWantElementNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityBase_GetWantElement_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityBase_GetWantElement_0100
     * @tc.desc    : Testing GetWantElement returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityBase_GetWantElement_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityBase_GetWantElement_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.GetWantElementNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityBase_SetWantCharParam_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityBase_SetWantCharParam_0100
     * @tc.desc    : Testing SetWantCharParam returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityBase_SetWantCharParam_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityBase_SetWantCharParam_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.SetWantCharParamNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityBase_GetWantCharParam_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityBase_GetWantCharParam_0100
     * @tc.desc    : Testing GetWantCharParam returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityBase_GetWantCharParam_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityBase_GetWantCharParam_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.GetWantCharParamNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityBase_AddWantFd_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityBase_AddWantFd_0100
     * @tc.desc    : Testing AddWantFd returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityBase_AddWantFd_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityBase_AddWantFd_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.AddWantFdNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityBase_SetWantUri_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityBase_SetWantUri_0100
     * @tc.desc    : Testing SetWantUri returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityBase_SetWantUri_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityBase_SetWantUri_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.SetWantUriNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityBase_GetWantUri_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityBase_GetWantUri_0100
     * @tc.desc    : Testing GetWantUri returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityBase_GetWantUri_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityBase_GetWantUri_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.GetWantUriNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityBase_SetWantInt32Param_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityBase_SetWantInt32Param_0100
     * @tc.desc    : Testing SetWantInt32Param returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityBase_SetWantInt32Param_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityBase_SetWantInt32Param_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.SetWantInt32ParamNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityBase_GetWantInt32Param_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityBase_GetWantInt32Param_0100
     * @tc.desc    : Testing GetWantInt32Param returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityBase_GetWantInt32Param_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityBase_GetWantInt32Param_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.GetWantInt32ParamNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityBase_SetWantBoolParam_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityBase_SetWantBoolParam_0100
     * @tc.desc    : Testing SetWantBoolParam returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityBase_SetWantBoolParam_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityBase_SetWantBoolParam_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.SetWantBoolParamNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityBase_GetWantBoolParam_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityBase_GetWantBoolParam_0100
     * @tc.desc    : Testing GetWantBoolParam returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityBase_GetWantBoolParam_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityBase_GetWantBoolParam_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.GetWantBoolParamNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityBase_SetWantDoubleParam_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityBase_SetWantDoubleParam_0100
     * @tc.desc    : Testing SetWantDoubleParam returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityBase_SetWantDoubleParam_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityBase_SetWantDoubleParam_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.SetWantDoubleParamNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityBase_GetWantDoubleParam_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityBase_GetWantDoubleParam_0100
     * @tc.desc    : Testing GetWantDoubleParam returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityBase_GetWantDoubleParam_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityBase_GetWantDoubleParam_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.GetWantDoubleParamNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_AbilityRuntimeAreaModeEl2_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_AbilityRuntimeAreaModeEl2_0100
     * @tc.desc    : Testing AbilityRuntimeAreaModeEl2 returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_AbilityRuntimeAreaModeEl2_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_AbilityRuntimeAreaModeEl2_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.AbilityRuntimeAreaModeEl2Code();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_AbilityRuntimeAreaModeEl3_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_AbilityRuntimeAreaModeEl3_0100
     * @tc.desc    : Testing AbilityRuntimeAreaModeEl3 returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_AbilityRuntimeAreaModeEl3_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_AbilityRuntimeAreaModeEl3_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.AbilityRuntimeAreaModeEl3Code();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_AbilityRuntimeAreaModeEl4_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_AbilityRuntimeAreaModeEl4_0100
     * @tc.desc    : Testing AbilityRuntimeAreaModeEl4 returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_AbilityRuntimeAreaModeEl4_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_AbilityRuntimeAreaModeEl4_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.AbilityRuntimeAreaModeEl4Code();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_AbilityRuntimeAreaModeEl5_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_AbilityRuntimeAreaModeEl5_0100
     * @tc.desc    : Testing AbilityRuntimeAreaModeEl5 returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_AbilityRuntimeAreaModeEl5_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_AbilityRuntimeAreaModeEl5_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.AbilityRuntimeAreaModeEl5Code();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_AbilityRuntimeHideUponStart_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_AbilityRuntimeHideUponStart_0100
     * @tc.desc    : Testing AbilityRuntimeHideUponStart returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_AbilityRuntimeHideUponStart_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_AbilityRuntimeHideUponStart_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.AbilityRuntimeHideUponStartCode();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_DestroyStartOptions_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_DestroyStartOptions_0100
     * @tc.desc    : Testing DestroyStartOptions returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_DestroyStartOptions_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_DestroyStartOptions_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.DestroyStartOptionsNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWindowMode_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWindowMode_0100
     * @tc.desc    : Testing SetStartOptionsWindowMode returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWindowMode_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWindowMode_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.SetStartOptionsWindowModeNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWindowMode_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWindowMode_0100
     * @tc.desc    : Testing GetStartOptionsWindowMode returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWindowMode_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWindowMode_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.GetStartOptionsWindowModeNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsDisplayId_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsDisplayId_0100
     * @tc.desc    : Testing SetStartOptionsDisplayId returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsDisplayId_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsDisplayId_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.SetStartOptionsDisplayIdNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsDisplayId_0200
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsDisplayId_0200
     * @tc.desc    : Testing SetStartOptionsDisplayId returns an invalid parameter error code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsDisplayId_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsDisplayId_0200`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.SetStartOptionsDisplayIdParamInvalid();
          expect(401).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsDisplayId_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsDisplayId_0100
     * @tc.desc    : Testing GetStartOptionsDisplayId returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsDisplayId_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsDisplayId_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.GetStartOptionsDisplayIdNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWithAnimation_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWithAnimation_0100
     * @tc.desc    : Testing SetStartOptionsWithAnimation returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWithAnimation_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWithAnimation_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.SetStartOptionsWithAnimationNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWithAnimation_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWithAnimation_0100
     * @tc.desc    : Testing GetStartOptionsWithAnimation returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWithAnimation_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWithAnimation_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.GetStartOptionsWithAnimationNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWindowLeft_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWindowLeft_0100
     * @tc.desc    : Testing SetStartOptionsWindowLeft returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWindowLeft_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWindowLeft_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.SetStartOptionsWindowLeftNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWindowLeft_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWindowLeft_0100
     * @tc.desc    : Testing GetStartOptionsWindowLeft returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWindowLeft_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWindowLeft_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.GetStartOptionsWindowLeftNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWindowTop_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWindowTop_0100
     * @tc.desc    : Testing SetStartOptionsWindowTop returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWindowTop_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWindowTop_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.SetStartOptionsWindowTopNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWindowTop_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWindowTop_0100
     * @tc.desc    : Testing GetStartOptionsWindowTop returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWindowTop_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWindowTop_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.GetStartOptionsWindowTopNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWindowHeight_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWindowHeight_0100
     * @tc.desc    : Testing SetStartOptionsWindowHeight returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWindowHeight_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWindowHeight_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.SetStartOptionsWindowHeightNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWindowHeight_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWindowHeight_0100
     * @tc.desc    : Testing GetStartOptionsWindowHeight returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWindowHeight_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWindowHeight_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.GetStartOptionsWindowHeightNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWindowWidth_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWindowWidth_0100
     * @tc.desc    : Testing SetStartOptionsWindowWidth returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWindowWidth_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsWindowWidth_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.SetStartOptionsWindowWidthNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWindowWidth_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWindowWidth_0100
     * @tc.desc    : Testing GetStartOptionsWindowWidth returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWindowWidth_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsWindowWidth_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.GetStartOptionsWindowWidthNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsStartVisibility_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsStartVisibility_0100
     * @tc.desc    : Testing SetStartOptionsStartVisibility returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsStartVisibility_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsStartVisibility_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.SetStartOptionsStartVisibilityNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsStartVisibility_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsStartVisibility_0100
     * @tc.desc    : Testing GetStartOptionsStartVisibility returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsStartVisibility_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsStartVisibility_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.GetStartOptionsStartVisibilityNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsStartWindowIcon_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsStartWindowIcon_0100
     * @tc.desc    : Testing SetStartOptionsStartWindowIcon returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsStartWindowIcon_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsStartWindowIcon_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.SetStartOptionsStartWindowIconNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsStartWindowIcon_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsStartWindowIcon_0100
     * @tc.desc    : Testing GetStartOptionsStartWindowIcon returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsStartWindowIcon_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsStartWindowIcon_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.GetStartOptionsStartWindowIconNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsStartWindowBackgroundColor_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsStartWindowBackgroundColor_0100
     * @tc.desc    : Testing SetStartOptionsStartWindowBackgroundColor returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsStartWindowBackgroundColor_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsStartWindowBackgroundColor_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.SetStartOptionsStartWindowBackgroundColorNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsStartWindowBackgroundColor_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsStartWindowBackgroundColor_0100
     * @tc.desc    : Testing GetStartOptionsStartWindowBackgroundColor returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsStartWindowBackgroundColor_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsStartWindowBackgroundColor_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.GetStartOptionsStartWindowBackgroundColorNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsSupportedWindowModes_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsSupportedWindowModes_0100
     * @tc.desc    : Testing SetStartOptionsSupportedWindowModes returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsSupportedWindowModes_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsSupportedWindowModes_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.SetStartOptionsSupportedWindowModesNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsSupportedWindowModes_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsSupportedWindowModes_0100
     * @tc.desc    : Testing GetStartOptionsSupportedWindowModes returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsSupportedWindowModes_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsSupportedWindowModes_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.GetStartOptionsSupportedWindowModesNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsMinWindowWidth_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsMinWindowWidth_0100
     * @tc.desc    : Testing SetStartOptionsMinWindowWidth returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsMinWindowWidth_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsMinWindowWidth_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.SetStartOptionsMinWindowWidthNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsMinWindowWidth_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsMinWindowWidth_0100
     * @tc.desc    : Testing GetStartOptionsMinWindowWidth returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsMinWindowWidth_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsMinWindowWidth_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.GetStartOptionsMinWindowWidthNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsMaxWindowWidth_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsMaxWindowWidth_0100
     * @tc.desc    : Testing SetStartOptionsMaxWindowWidth returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsMaxWindowWidth_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsMaxWindowWidth_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.SetStartOptionsMaxWindowWidthNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsMaxWindowWidth_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsMaxWindowWidth_0100
     * @tc.desc    : Testing GetStartOptionsMaxWindowWidth returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsMaxWindowWidth_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsMaxWindowWidth_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.GetStartOptionsMaxWindowWidthNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsMinWindowHeight_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsMinWindowHeight_0100
     * @tc.desc    : Testing SetStartOptionsMinWindowHeight returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsMinWindowHeight_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsMinWindowHeight_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.SetStartOptionsMinWindowHeightNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsMinWindowHeight_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsMinWindowHeight_0100
     * @tc.desc    : Testing GetStartOptionsMinWindowHeight returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsMinWindowHeight_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsMinWindowHeight_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.GetStartOptionsMinWindowHeightNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsMinWindowHeight_0200
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsMinWindowHeight_0200
     * @tc.desc    : Testing GetStartOptionsMinWindowHeight returns an invalid parameter error code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsMinWindowHeight_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsMinWindowHeight_0200`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.GetStartOptionsMinWindowHeightParamInvalid();
          expect(401).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsMaxWindowHeight_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsMaxWindowHeight_0100
     * @tc.desc    : Testing SetStartOptionsMaxWindowHeight returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsMaxWindowHeight_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_SetStartOptionsMaxWindowHeight_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.SetStartOptionsMaxWindowHeightNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsMaxWindowHeight_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsMaxWindowHeight_0100
     * @tc.desc    : Testing GetStartOptionsMaxWindowHeight returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsMaxWindowHeight_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeAbilityRuntime_GetStartOptionsMaxWindowHeight_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.GetStartOptionsMaxWindowHeightNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_CreateNativeChildProcess_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_CreateNativeChildProcess_0100
     * @tc.desc    : Testing CreateNativeChildProcess returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_CreateNativeChildProcess_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_CreateNativeChildProcess_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result : number = AbilityKitTest.CreateNativeChildProcessNoError();
          if(result == 801 || result == 16010004 || result == 16010006){
            done();
            return;
          }
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_CreateNativeChildProcess_0200
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_CreateNativeChildProcess_0200
     * @tc.desc    : Testing CreateNativeChildProcess returns an invalid parameter error code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_CreateNativeChildProcess_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_CreateNativeChildProcess_0200`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result : number = AbilityKitTest.CreateNativeChildProcessInvalidParam();
          expect(401).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_CreateNativeChildProcess_0300
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_CreateNativeChildProcess_0300
     * @tc.desc    : Testing CreateNativeChildProcess returns 16010007.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_CreateNativeChildProcess_0300',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_CreateNativeChildProcess_0300`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result : number = AbilityKitTest.CreateNativeChildProcessLibLoadingFailed();
          if(result == 801 || result == 16010004 || result == 16010006){
            done();
            return;
          }
          expect(16010007).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_CreateNativeChildProcessWithConfigs_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_CreateNativeChildProcessWithConfigs_0100
     * @tc.desc    : Testing CreateNativeChildProcessWithConfigs returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_CreateNativeChildProcessWithConfigs_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_CreateNativeChildProcessWithConfigs_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result : number = AbilityKitTest.CreateNativeChildProcessWithConfigsNoError();
          if(result == 801 || result == 16010004 || result == 16010006){
            done();
            return;
          }
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_CreateNativeChildProcessWithConfigs_0200
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_CreateNativeChildProcessWithConfigs_0200
     * @tc.desc    : Testing CreateNativeChildProcessWithConfigs returns an invalid parameter error code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_CreateNativeChildProcessWithConfigs_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_CreateNativeChildProcessWithConfigs_0200`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result : number = AbilityKitTest.CreateNativeChildProcessWithConfigsInvalidParam();
          expect(401).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_StartNativeChildProcess_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_StartNativeChildProcess_0100
     * @tc.desc    : Testing StartNativeChildProcess returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_StartNativeChildProcess_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_StartNativeChildProcess_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result : number = AbilityKitTest.StartNativeChildProcessNoError();
          if(result == 801 || result == 16010004 || result == 16010006){
            done();
            return;
          }
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_StartNativeChildProcess_0200
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_StartNativeChildProcess_0200
     * @tc.desc    : Testing StartNativeChildProcess returns an invalid parameter error code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_StartNativeChildProcess_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_StartNativeChildProcess_0200`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result : number = AbilityKitTest.StartNativeChildProcessInvalidParam();
          expect(401).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_StartNativeChildProcessWithConfigs_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_StartNativeChildProcessWithConfigs_0100
     * @tc.desc    : Testing StartNativeChildProcessWithConfigs returns a correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_StartNativeChildProcessWithConfigs_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_StartNativeChildProcessWithConfigs_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result : number = AbilityKitTest.StartNativeChildProcessWithConfigsNoError();
          if(result == 801 || result == 16010004 || result == 16010006){
            done();
            return;
          }
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_StartNativeChildProcessWithConfigs_0200
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_StartNativeChildProcessWithConfigs_0200
     * @tc.desc    : Testing StartNativeChildProcessWithConfigs returns an invalid parameter error code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_StartNativeChildProcessWithConfigs_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_StartNativeChildProcessWithConfigs_0200`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result : number = AbilityKitTest.StartNativeChildProcessWithConfigsInvalidParam();
          expect(401).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_StartNativeChildProcessWithConfigs_0300
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_StartNativeChildProcessWithConfigs_0300
     * @tc.desc    : Testing StartNativeChildProcessWithConfigs returns 16010003.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_StartNativeChildProcessWithConfigs_0300',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_StartNativeChildProcessWithConfigs_0300`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result : number = AbilityKitTest.StartNativeChildProcessWithConfigsServiceError();
          if(result == 801 || result == 16010004 || result == 16010006){
            done();
            return;
          }
          expect(16010003).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_StartSelfUIAbility_0100
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_StartSelfUIAbility_0100
     * @tc.desc    : Test StartSelfUIAbility and return 16000079.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_StartSelfUIAbility_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_StartSelfUIAbility_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result : number = AbilityKitTest.StartSelfUIAbilityInstanceKeyNotSupport();
          if(result == 801){
            console.info(`====>${LOG_NAME} result: ` + result);
            expect(801).assertEqual(result);
            done();
            return;
          }
          expect(16000079).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_StartSelfUIAbility_0200
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_StartSelfUIAbility_0200
     * @tc.desc    : Test StartSelfUIAbility and return the correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_StartSelfUIAbility_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_StartSelfUIAbility_0200`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result : number = AbilityKitTest.StartSelfUIAbilityNoError();
          if(result == 801){
            console.info(`====>${LOG_NAME} result: ` + result);
            expect(801).assertEqual(result);
            done();
            return;
          }
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NativeChildProcess_StartSelfUIAbility_0300
     * @tc.name    : SUB_Ability_AbilityBase_NativeChildProcess_StartSelfUIAbility_0300
     * @tc.desc    : Test StartSelfUIAbility and return 16000077.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NativeChildProcess_StartSelfUIAbility_0300',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NativeChildProcess_StartSelfUIAbility_0300`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result : number = AbilityKitTest.StartSelfUIAbilityUpperLimitReached();
          if(result == 801){
            console.info(`====>${LOG_NAME} result: ` + result);
            expect(801).assertEqual(result);
            done();
            return;
          }
          expect(16000077).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })


    /**
     * @tc.number  : SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0100
     * @tc.name    : SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0100
     * @tc.desc    : Test StartSelfUIAbilityWithStartOptions and return 16000079.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result : number = AbilityKitTest.StartSelfUIAbilityWithStartOptionsInstanceKeyNotSupport();
          if(result == 801){
            console.info(`====>${LOG_NAME} result: ` + result);
            expect(801).assertEqual(result);
            done();
            return;
          }
          expect(16000079).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0200
     * @tc.name    : SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0200
     * @tc.desc    : Test StartSelfUIAbilityWithStartOptions and return the correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0200`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result : number = AbilityKitTest.StartSelfUIAbilityWithStartOptionsNoError();
          if(result == 801){
            console.info(`====>${LOG_NAME} result: ` + result);
            expect(801).assertEqual(result);
            done();
            return;
          }
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0300
     * @tc.name    : SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0300
     * @tc.desc    : Test StartSelfUIAbilityWithStartOptions and return 16000077.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0300',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0300`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result : number = AbilityKitTest.StartSelfUIAbilityWithStartOptionsUpperLimitReached();
          if(result == 801){
            console.info(`====>${LOG_NAME} result: ` + result);
            expect(801).assertEqual(result);
            done();
            return;
          }
          expect(16000077).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0500
     * @tc.name    : SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0500
     * @tc.desc    : Test StartSelfUIAbilityWithStartOptions and return 16000001.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0500',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0500`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result : number = AbilityKitTest.StartSelfUIAbilityWithStartOptionsNoSuchAbility();
          if(result == 801){
            console.info(`====>${LOG_NAME} result: ` + result);
            expect(801).assertEqual(result);
            done();
            return;
          }
          expect(16000001).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0600
     * @tc.name    : SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0600
     * @tc.desc    : Test StartSelfUIAbilityWithStartOptions and return 16000018.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0600',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0600`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result : number = AbilityKitTest.StartSelfUIAbilityWithStartOptionsCodeCrossApp();
          if(result == 801){
            console.info(`====>${LOG_NAME} result: ` + result);
            expect(801).assertEqual(result);
            done();
            return;
          }
          expect(16000018).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0700
     * @tc.name    : SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0700
     * @tc.desc    : Test StartSelfUIAbilityWithStartOptions and return 16000050.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0700',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_0700`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result : number = AbilityKitTest.StartSelfUIAbilityWithStartOptionsCodeInternal();
          if(result == 801){
            console.info(`====>${LOG_NAME} result: ` + result);
            expect(801).assertEqual(result);
            done();
            return;
          }
          expect(16000050).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_1000
     * @tc.name    : SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_1000
     * @tc.desc    : Test StartSelfUIAbilityWithStartOptions and return 16000076.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_1000',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_1000`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result : number = AbilityKitTest.StartSelfUIAbilityWithStartOptionsInvalidAppInstanceKey();
          if(result == 801){
            console.info(`====>${LOG_NAME} result: ` + result);
            expect(801).assertEqual(result);
            done();
            return;
          }
          expect(16000076).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_1100
     * @tc.name    : SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_1100
     * @tc.desc    : Test StartSelfUIAbilityWithStartOptions and return 16000067.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_1100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_AbilityRuntime_StartSelfUIAbilityWithStartOptions_1100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result : number = AbilityKitTest.StartSelfUIAbilityWithStartOptionsVisibilitySettingDisabled();
          if(result == 801){
            console.info(`====>${LOG_NAME} result: ` + result);
            expect(801).assertEqual(result);
            done();
            return;
          }
          expect(16000067).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done();
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done();
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_ApplicationContextGetCacheDir_0100
     * @tc.name    : SUB_Ability_AbilityBase_ApplicationContextGetCacheDir_0100
     * @tc.desc    : Test ApplicationContextGetCacheDir and return the correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_ApplicationContextGetCacheDir_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_ApplicationContextGetCacheDir_0100`;
        try {
          console.info(`${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ApplicationContextGetCacheDirNoError();
          expect(0).assertEqual(result);
          console.info(`${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_ApplicationContextGetCacheDir_0200
     * @tc.name    : SUB_Ability_AbilityBase_ApplicationContextGetCacheDir_0200
     * @tc.desc    : Testing ApplicationContextGetCacheDir returns an invalid parameter error code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_ApplicationContextGetCacheDir_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_ApplicationContextGetCacheDir_0200`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ApplicationContextGetCacheDirInvPar();
          expect(401).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_ApplicationContextGetAreaMode_0100
     * @tc.name    : SUB_Ability_AbilityBase_ApplicationContextGetAreaMode_0100
     * @tc.desc    : Test ApplicationContextGetAreaMode and return the correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_ApplicationContextGetAreaMode_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_ApplicationContextGetAreaMode_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ApplicationContextGetAreaModeNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_ApplicationContextGetAreaMode_0200
     * @tc.name    : SUB_Ability_AbilityBase_ApplicationContextGetAreaMode_0200
     * @tc.desc    : Testing ApplicationContextGetAreaMode returns an invalid parameter error code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_ApplicationContextGetAreaMode_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_ApplicationContextGetAreaMode_0200`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ApplicationContextGetAreaModeInvPar();
          expect(401).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_ApplicationContextGetBundleName_0100
     * @tc.name    : SUB_Ability_AbilityBase_ApplicationContextGetBundleName_0100
     * @tc.desc    : Test ApplicationContextGetBundleName and return the correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_ApplicationContextGetBundleName_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_ApplicationContextGetBundleName_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ApplicationContextGetBundleNameNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_ApplicationContextGetBundleName_0200
     * @tc.name    : SUB_Ability_AbilityBase_ApplicationContextGetBundleName_0200
     * @tc.desc    : Testing ApplicationContextGetBundleName returns an invalid parameter error code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_ApplicationContextGetBundleName_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_ApplicationContextGetBundleName_0200`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ApplicationContextGetBundleNameInvPar();
          expect(401).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_ApplicationContextGetTempDir_0100
     * @tc.name    : SUB_Ability_AbilityBase_ApplicationContextGetTempDir_0100
     * @tc.desc    : Test ApplicationContextGetTempDir and return the correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_ApplicationContextGetTempDir_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_ApplicationContextGetTempDir_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ApplicationContextGetTempDirNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_ApplicationContextGetTempDir_0200
     * @tc.name    : SUB_Ability_AbilityBase_ApplicationContextGetTempDir_0200
     * @tc.desc    : Testing ApplicationContextGetTempDir returns an invalid parameter error code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_ApplicationContextGetTempDir_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_ApplicationContextGetTempDir_0200`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ApplicationContextGetTempDirInvPar();
          expect(401).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_ApplicationContextGetFilesDir_0100
     * @tc.name    : SUB_Ability_AbilityBase_ApplicationContextGetFilesDir_0100
     * @tc.desc    : Test ApplicationContextGetFilesDir and return the correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_ApplicationContextGetFilesDir_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_ApplicationContextGetFilesDir_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ApplicationContextGetFilesDirNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_ApplicationContextGetFilesDir_0200
     * @tc.name    : SUB_Ability_AbilityBase_ApplicationContextGetFilesDir_0200
     * @tc.desc    : Testing ApplicationContextGetFilesDir returns an invalid parameter error code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_ApplicationContextGetFilesDir_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_ApplicationContextGetFilesDir_0200`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ApplicationContextGetFilesDirInvPar();
          expect(401).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_ApplicationContextGetDatabaseDir_0100
     * @tc.name    : SUB_Ability_AbilityBase_ApplicationContextGetDatabaseDir_0100
     * @tc.desc    : Test ApplicationContextGetDatabaseDir and return the correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_ApplicationContextGetDatabaseDir_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_ApplicationContextGetDatabaseDir_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ApplicationContextGetDatabaseDirNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_ApplicationContextGetDatabaseDir_0200
     * @tc.name    : SUB_Ability_AbilityBase_ApplicationContextGetDatabaseDir_0200
     * @tc.desc    : Testing ApplicationContextGetDatabaseDir returns an invalid parameter error code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_ApplicationContextGetDatabaseDir_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_ApplicationContextGetDatabaseDir_0200`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ApplicationContextGetDatabaseDirInvPar();
          expect(401).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_ApplicationContextGetPreferencesDir_0100
     * @tc.name    : SUB_Ability_AbilityBase_ApplicationContextGetPreferencesDir_0100
     * @tc.desc    : Test ApplicationContextGetPreferencesDir and return the correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_ApplicationContextGetPreferencesDir_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_ApplicationContextGetPreferencesDir_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ApplicationContextGetPreferencesDirNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_ApplicationContextGetPreferencesDir_0200
     * @tc.name    : SUB_Ability_AbilityBase_ApplicationContextGetPreferencesDir_0200
     * @tc.desc    : Testing ApplicationContextGetPreferencesDir returns an invalid parameter error code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_ApplicationContextGetPreferencesDir_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_ApplicationContextGetPreferencesDir_0200`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ApplicationContextGetPreferencesDirInvPar();
          expect(401).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_ApplicationContextGetBundleCodeDir_0100
     * @tc.name    : SUB_Ability_AbilityBase_ApplicationContextGetBundleCodeDir_0100
     * @tc.desc    : Test ApplicationContextGetBundleCodeDir and return the correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_ApplicationContextGetBundleCodeDir_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_ApplicationContextGetBundleCodeDir_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ApplicationContextGetBundleCodeDirNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_ApplicationContextGetBundleCodeDir_0200
     * @tc.name    : SUB_Ability_AbilityBase_ApplicationContextGetBundleCodeDir_0200
     * @tc.desc    : Testing ApplicationContextGetBundleCodeDir returns an invalid parameter error code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_ApplicationContextGetBundleCodeDir_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_ApplicationContextGetBundleCodeDir_0200`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ApplicationContextGetBundleCodeDirInvPar();
          expect(401).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_ApplicationContextGetDistributedFilesDir_0100
     * @tc.name    : SUB_Ability_AbilityBase_ApplicationContextGetDistributedFilesDir_0100
     * @tc.desc    : Test ApplicationContextGetDistributedFilesDir and return the correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_ApplicationContextGetDistributedFilesDir_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_ApplicationContextGetDistributedFilesDir_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ApplicationContextGetDistributedFilesDirNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_ApplicationContextGetDistributedFilesDir_0200
     * @tc.name    : SUB_Ability_AbilityBase_ApplicationContextGetDistributedFilesDir_0200
     * @tc.desc    : Testing ApplicationContextGetDistributedFilesDir returns an invalid parameter error code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_ApplicationContextGetDistributedFilesDir_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_ApplicationContextGetDistributedFilesDir_0200`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ApplicationContextGetDistributedFilesDirInvPar();
          expect(401).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_ApplicationContextGetCloudFileDir_0100
     * @tc.name    : SUB_Ability_AbilityBase_ApplicationContextGetCloudFileDir_0100
     * @tc.desc    : Test ApplicationContextGetCloudFileDir and return the correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_ApplicationContextGetCloudFileDir_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_ApplicationContextGetCloudFileDir_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ApplicationContextGetCloudFileDirNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_ApplicationContextGetCloudFileDir_0200
     * @tc.name    : SUB_Ability_AbilityBase_ApplicationContextGetCloudFileDir_0200
     * @tc.desc    : Testing ApplicationContextGetCloudFileDir returns an invalid parameter error code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_ApplicationContextGetCloudFileDir_0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_ApplicationContextGetCloudFileDir_0200`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ApplicationContextGetCloudFileDirInvPar();
          expect(401).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_ApplicationContextGetResourceDir_0100
     * @tc.name    : SUB_Ability_AbilityBase_ApplicationContextGetResourceDir_0100
     * @tc.desc    : Test ApplicationContextGetResourceDir and return the correct code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_ApplicationContextGetResourceDir_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_ApplicationContextGetResourceDir_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.ApplicationContextGetResourceDirNoError();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NcpErrorInvalidParam_0100
     * @tc.name    : SUB_Ability_AbilityBase_NcpErrorInvalidParam_0100
     * @tc.desc    : Test NcpErrorInvalidParam Error Code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NcpErrorInvalidParam_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NcpErrorInvalidParam_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.NcpErrorInvalidParamCode();
          expect(401).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NcpErrorServiceError_0100
     * @tc.name    : SUB_Ability_AbilityBase_NcpErrorServiceError_0100
     * @tc.desc    : Test NcpErrorServiceError Error Code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NcpErrorServiceError_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NcpErrorServiceError_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.NcpErrorServiceErrorCode();
          if(result == 801 || result == 16010004 || result == 16010006){
            done();
            return;
          }
          expect(16010003).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NcpErrorLibLoadingFailed_0100
     * @tc.name    : SUB_Ability_AbilityBase_NcpErrorLibLoadingFailed_0100
     * @tc.desc    : Test NcpErrorLibLoadingFailed Error Code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NcpErrorLibLoadingFailed_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NcpErrorLibLoadingFailed_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.NcpErrorLibLoadingFailedCode();
          if(result == 801 || result == 16010004 || result == 16010006){
            done();
            return;
          }
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_NcpErrCallBackNotExist_0100
     * @tc.name    : SUB_Ability_AbilityBase_NcpErrCallBackNotExist_0100
     * @tc.desc    : Test NcpErrCallBackNotExist Error Code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_NcpErrCallBackNotExist_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_NcpErrCallBackNotExist_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.NcpErrCallBackNotExistCode();
          expect(16010009).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_AbilityBaseErrorCodeNoError_0100
     * @tc.name    : SUB_Ability_AbilityBase_AbilityBaseErrorCodeNoError_0100
     * @tc.desc    : Test AbilityBaseErrorCodeNoError Error Code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_AbilityBaseErrorCodeNoError_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_AbilityBaseErrorCodeNoError_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.AbilityBaseErrorCodeNoErrorCode();
          expect(0).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_AbilityBaseErrorCodeParamInvalid_0100
     * @tc.name    : SUB_Ability_AbilityBase_AbilityBaseErrorCodeParamInvalid_0100
     * @tc.desc    : Test AbilityBaseErrorCodeParamInvalid Error Code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_AbilityBaseErrorCodeParamInvalid_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_AbilityBaseErrorCodeParamInvalid_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.AbilityBaseErrorCodeParamInvalidCode();
          if(result == 801){
            expect(801).assertEqual(result);
            done();
            return;
          }
          expect(401).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_AbilityRuntimeErrorCodeNoSuchAbility_0100
     * @tc.name    : SUB_Ability_AbilityBase_AbilityRuntimeErrorCodeNoSuchAbility_0100
     * @tc.desc    : Test AbilityRuntimeErrorCodeNoSuchAbility Error Code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_AbilityRuntimeErrorCodeNoSuchAbility_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_AbilityRuntimeErrorCodeNoSuchAbility_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.AbilityRuntimeErrorCodeNoSuchAbilityCode();
          if(result == 801){
            expect(801).assertEqual(result);
            done();
            return;
          }
          expect(16000001).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_AbilityRuntimeErrorCodeCrossApp_0100
     * @tc.name    : SUB_Ability_AbilityBase_AbilityRuntimeErrorCodeCrossApp_0100
     * @tc.desc    : Test AbilityRuntimeErrorCodeCrossApp Error Code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_AbilityRuntimeErrorCodeCrossApp_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_AbilityRuntimeErrorCodeCrossApp_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.AbilityRuntimeErrorCodeCrossAppCode();
          if(result == 801){
            expect(801).assertEqual(result);
            done();
            return;
          }
          expect(16000018).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_AbilityRuntimeErrorCodeInternal_0100
     * @tc.name    : SUB_Ability_AbilityBase_AbilityRuntimeErrorCodeInternal_0100
     * @tc.desc    : Test AbilityRuntimeErrorCodeInternal Error Code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_AbilityRuntimeErrorCodeInternal_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_AbilityRuntimeErrorCodeInternal_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.AbilityRuntimeErrorCodeInternalCode();
          if(result == 801){
            expect(801).assertEqual(result);
            done();
            return;
          }
          expect(16000050).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_AbilityRuntimeErrorVisibilitySettingDisabled_0100
     * @tc.name    : SUB_Ability_AbilityBase_AbilityRuntimeErrorVisibilitySettingDisabled_0100
     * @tc.desc    : Test AbilityRuntimeErrorVisibilitySettingDisabled Error Code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_AbilityRuntimeErrorVisibilitySettingDisabled_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_AbilityRuntimeErrorVisibilitySettingDisabled_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.AbilityRuntimeErrorVisibilitySettingDisabledCode();
          if(result == 801){
            expect(801).assertEqual(result);
            done();
            return;
          }
          expect(16000067).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_AbilityRuntimeErrorCodeInvalidAppInstanceKey_0100
     * @tc.name    : SUB_Ability_AbilityBase_AbilityRuntimeErrorCodeInvalidAppInstanceKey_0100
     * @tc.desc    : Test AbilityRuntimeErrorCodeInvalidAppInstanceKey Error Code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_AbilityRuntimeErrorCodeInvalidAppInstanceKey_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_AbilityRuntimeErrorCodeInvalidAppInstanceKey_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.AbilityRuntimeErrorCodeInvalidAppInstanceKeyCode();
          if(result == 801){
            expect(801).assertEqual(result);
            done();
            return;
          }
          expect(16000076).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_AbilityRuntimeErrorCodeUpperLimitReached_0100
     * @tc.name    : SUB_Ability_AbilityBase_AbilityRuntimeErrorCodeUpperLimitReached_0100
     * @tc.desc    : Test AbilityRuntimeErrorCodeUpperLimitReached Error Code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_AbilityRuntimeErrorCodeUpperLimitReached_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_AbilityRuntimeErrorCodeUpperLimitReached_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.AbilityRuntimeErrorCodeUpperLimitReachedCode();
          if(result == 801){
            expect(801).assertEqual(result);
            done();
            return;
          }
          expect(16000077).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_AbilityRuntimeErrorAppInstanceKeyNotSupport_0100
     * @tc.name    : SUB_Ability_AbilityBase_AbilityRuntimeErrorAppInstanceKeyNotSupport_0100
     * @tc.desc    : Test AbilityRuntimeErrorAppInstanceKeyNotSupport Error Code.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Ability_AbilityBase_AbilityRuntimeErrorAppInstanceKeyNotSupport_0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        const LOG_NAME = `SUB_Ability_AbilityBase_AbilityRuntimeErrorAppInstanceKeyNotSupport_0100`;
        try {
          console.info(`====>${LOG_NAME} start====`);
          let result:number = AbilityKitTest.AbilityRuntimeErrorAppInstanceKeyNotSupportCode();
          if(result == 801){
            expect(801).assertEqual(result);
            done();
            return;
          }
          expect(16000079).assertEqual(result);
          console.info(`====>${LOG_NAME} result: ` + result);
          done()
        } catch (err) {
          console.error(`====>${LOG_NAME} catch err: ` + err);
          expect(false).assertEqual(ABILITY_ERR_OK);
          done()
        }
      })
  })
}
