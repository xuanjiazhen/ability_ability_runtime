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
import osAccountNdk from 'libosaccountndk.so'

const OS_ACCOUNT_ERR_OK = 0;

export default function ActsCapabilityStartSelfUIAbilityTest() {
  describe('ActsCapabilityStartSelfUIAbilityTest', () => {

    /**
     * @tc.number  : SUB_Ability_AbilityBase_DestroyChildProcessConfigs_0100
     * @tc.name    : testAbilityAbilityBaseDestroyChildProcessConfigs0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseDestroyChildProcessConfigs0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_DestroyChildProcessConfigs_0100 start====");
          let result = osAccountNdk.DestroyChildProcessConfigsParamInvalid();
          expect(401).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_DestroyChildProcessConfigs_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_DestroyChildProcessConfigs_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_DestroyChildProcessConfigs_0200
     * @tc.name    : testAbilityAbilityBaseDestroyChildProcessConfigs0200
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseDestroyChildProcessConfigs0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_DestroyChildProcessConfigs_0200 start====");
          let result = osAccountNdk.DestroyChildProcessConfigsNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_DestroyChildProcessConfigs_0200 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_DestroyChildProcessConfigs_0200 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetIsolationModeConfigs_0100
     * @tc.name    : testAbilityAbilityBaseSetIsolationModeConfigs0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetIsolationModeConfigs0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetIsolationModeConfigs_0100 start====");
          let result = osAccountNdk.SetIsolationModeConfigsInvalidParam();
          expect(401).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetIsolationModeConfigs_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetIsolationModeConfigs_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetIsolationModeConfigs_0200
     * @tc.name    : testAbilityAbilityBaseSetIsolationModeConfigs0200
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetIsolationModeConfigs0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetIsolationModeConfigs_0200 start====");
          let result = osAccountNdk.SetIsolationModeConfigsNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetIsolationModeConfigs_0200 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetIsolationModeConfigs_0200 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetProcessNameConfigs_0100
     * @tc.name    : testAbilityAbilityBaseSetProcessNameConfigs0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetProcessNameConfigs0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetProcessNameConfigs_0100 start====");
          let result = osAccountNdk.SetProcessNameConfigsInvalidParam();
          expect(401).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetProcessNameConfigs_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetProcessNameConfigs_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetProcessNameConfigs_0200
     * @tc.name    : testAbilityAbilityBaseSetProcessNameConfigs0200
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetProcessNameConfigs0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetProcessNameConfigs_0200 start====");
          let result = osAccountNdk.SetProcessNameConfigsNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetProcessNameConfigs_0200 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetProcessNameConfigs_0200 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_RegisterNativeChildProcessExitCallback_0100
     * @tc.name    : testAbilityAbilityBaseRegisterNativeChildProcessExitCallback0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseRegisterNativeChildProcessExitCallback0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_RegisterNativeChildProcessExitCallback_0100 start====");
          let result = osAccountNdk.RegisterNativeChildProcessExitCallbackNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_RegisterNativeChildProcessExitCallback_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_RegisterNativeChildProcessExitCallback_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_RegisterNativeChildProcessExitCallback_0200
     * @tc.name    : testAbilityAbilityBaseRegisterNativeChildProcessExitCallback0200
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseRegisterNativeChildProcessExitCallback0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_RegisterNativeChildProcessExitCallback_0200 start====");
          let result = osAccountNdk.RegisterNativeChildProcessExitCallbackInvalidParam();
          expect(401).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_RegisterNativeChildProcessExitCallback_0200 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_RegisterNativeChildProcessExitCallback_0200 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_UnregisterNativeChildProcessExitCallback_0100
     * @tc.name    : testAbilityAbilityBaseUnRegisterNativeChildProcessExitCallback0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseUnRegisterNativeChildProcessExitCallback0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_UnregisterNativeChildProcessExitCallback_0100 start====");
          let result = osAccountNdk.UnregisterNativeChildProcessExitCallbackNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_UnregisterNativeChildProcessExitCallback_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_UnregisterNativeChildProcessExitCallback_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_UnregisterNativeChildProcessExitCallback_0200
     * @tc.name    : testAbilityAbilityBaseUnRegisterNativeChildProcessExitCallback0200
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseUnRegisterNativeChildProcessExitCallback0200',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_UnregisterNativeChildProcessExitCallback_0200 start====");
          let result = osAccountNdk.UnregisterNativeChildProcessExitCallbackInvalidParam();
          expect(401).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_UnregisterNativeChildProcessExitCallback_0200 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_UnregisterNativeChildProcessExitCallback_0200 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_UnRegisterNativeChildProcessExitCallback_0300
     * @tc.name    : testAbilityAbilityBaseUnRegisterNativeChildProcessExitCallback0300
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseUnRegisterNativeChildProcessExitCallback0300',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_UnRegisterNativeChildProcessExitCallback_0300 start====");
          let result = osAccountNdk.UnregisterNativeChildProcessExitCallbackNotExist();
          expect(16010009).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_UnRegisterNativeChildProcessExitCallback_0300 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_UnRegisterNativeChildProcessExitCallback_0300 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_DestroyWant_0100
     * @tc.name    : testAbilityAbilityBaseDestroyWant0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseDestroyWant0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_DestroyWant_0100 start====");
          let result = osAccountNdk.DestroyWantNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_DestroyWant_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_DestroyWant_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetWantElement_0100
     * @tc.name    : testAbilityAbilityBaseSetWantElement0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetWantElement0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetWantElement_0100 start====");
          let result = osAccountNdk.SetWantElementNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetWantElement_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetWantElement_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_GetWantElement_0100
     * @tc.name    : testAbilityAbilityBaseGetWantElement0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseGetWantElement0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_GetWantElement_0100 start====");
          let result = osAccountNdk.GetWantElementNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_GetWantElement_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_GetWantElement_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetWantCharParam_0100
     * @tc.name    : testAbilityAbilityBaseSetWantCharParam0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetWantCharParam0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetWantCharParam_0100 start====");
          let result = osAccountNdk.SetWantCharParamNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetWantCharParam_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetWantCharParam_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_GetWantCharParam_0100
     * @tc.name    : testAbilityAbilityBaseGetWantCharParam0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseGetWantCharParam0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_GetWantCharParam_0100 start====");
          let result = osAccountNdk.GetWantCharParamNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_GetWantCharParam_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_GetWantCharParam_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_AddWantFd_0100
     * @tc.name    : testAbilityAbilityBaseAddWantFd0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseAddWantFd0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_AddWantFd_0100 start====");
          let result = osAccountNdk.AddWantFdNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_AddWantFd_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_AddWantFd_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetWantUri_0100
     * @tc.name    : testAbilityAbilityBaseSetWantUri0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetWantUri0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetWantUri_0100 start====");
          let result = osAccountNdk.SetWantUriNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetWantUri_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetWantUri_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_GetWantUri_0100
     * @tc.name    : testAbilityAbilityBaseGetWantUri0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseGetWantUri0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_GetWantUri_0100 start====");
          let result = osAccountNdk.GetWantUriNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_GetWantUri_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_GetWantUri_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetWantInt32Param_0100
     * @tc.name    : testAbilityAbilityBaseSetWantInt32Param0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetWantInt32Param0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetWantInt32Param_0100 start====");
          let result = osAccountNdk.SetWantInt32ParamNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetWantInt32Param_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetWantInt32Param_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_GetWantInt32Param_0100
     * @tc.name    : testAbilityAbilityBaseGetWantInt32Param0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseGetWantInt32Param0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_GetWantInt32Param_0100 start====");
          let result = osAccountNdk.GetWantInt32ParamNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_GetWantInt32Param_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_GetWantInt32Param_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetWantBoolParam_0100
     * @tc.name    : testAbilityAbilityBaseSetWantBoolParam0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetWantBoolParam0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetWantBoolParam_0100 start====");
          let result = osAccountNdk.SetWantBoolParamNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetWantBoolParam_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetWantBoolParam_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_GetWantBoolParam_0100
     * @tc.name    : testAbilityAbilityBaseGetWantBoolParam0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseGetWantBoolParam0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_GetWantBoolParam_0100 start====");
          let result = osAccountNdk.GetWantBoolParamNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_GetWantBoolParam_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_GetWantBoolParam_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetWantDoubleParam_0100
     * @tc.name    : testAbilityAbilityBaseSetWantDoubleParam0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetWantDoubleParam0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetWantDoubleParam_0100 start====");
          let result = osAccountNdk.SetWantDoubleParamNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetWantDoubleParam_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetWantDoubleParam_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_GetWantDoubleParam_0100
     * @tc.name    : testAbilityAbilityBaseGetWantDoubleParam0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseGetWantDoubleParam0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_GetWantDoubleParam_0100 start====");
          let result = osAccountNdk.GetWantDoubleParamNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_GetWantDoubleParam_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_GetWantDoubleParam_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_AbilityRuntimeAreaModeEl2_0100
     * @tc.name    : testAbilityAbilityBaseAbilityRuntimeAreaModeEl20100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseAbilityRuntimeAreaModeEl20100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_AbilityRuntimeAreaModeEl2_0100 start====");
          let result = osAccountNdk.AbilityRuntimeAreaModeEl2Code();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_AbilityRuntimeAreaModeEl2_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_AbilityRuntimeAreaModeEl2_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_AbilityRuntimeAreaModeEl3_0100
     * @tc.name    : testAbilityAbilityBaseAbilityRuntimeAreaModeEl30100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseAbilityRuntimeAreaModeEl30100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_AbilityRuntimeAreaModeEl3_0100 start====");
          let result = osAccountNdk.AbilityRuntimeAreaModeEl3Code();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_AbilityRuntimeAreaModeEl3_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_AbilityRuntimeAreaModeEl3_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_AbilityRuntimeAreaModeEl4_0100
     * @tc.name    : testAbilityAbilityBaseAbilityRuntimeAreaModeEl40100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseAbilityRuntimeAreaModeEl40100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_AbilityRuntimeAreaModeEl4_0100 start====");
          let result = osAccountNdk.AbilityRuntimeAreaModeEl4Code();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_AbilityRuntimeAreaModeEl4_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_AbilityRuntimeAreaModeEl4_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_AbilityRuntimeAreaModeEl5_0100
     * @tc.name    : testAbilityAbilityBaseAbilityRuntimeAreaModeEl50100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseAbilityRuntimeAreaModeEl50100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_AbilityRuntimeAreaModeEl5_0100 start====");
          let result = osAccountNdk.AbilityRuntimeAreaModeEl5Code();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_AbilityRuntimeAreaModeEl5_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_AbilityRuntimeAreaModeEl5_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_AbilityRuntimeHideUponStart_0100
     * @tc.name    : testAbilityAbilityBaseAbilityRuntimeHideUponStart0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseAbilityRuntimeHideUponStart0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_AbilityRuntimeHideUponStart_0100 start====");
          let result = osAccountNdk.AbilityRuntimeHideUponStartCode();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_AbilityRuntimeHideUponStart_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_AbilityRuntimeHideUponStart_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_DestroyStartOptions_0100
     * @tc.name    : testAbilityAbilityBaseDestroyStartOptions0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseDestroyStartOptions0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_DestroyStartOptions_0100 start====");
          let result = osAccountNdk.DestroyStartOptionsNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_DestroyStartOptions_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_DestroyStartOptions_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetStartOptionsWindowMode_0100
     * @tc.name    : testAbilityAbilityBaseSetStartOptionsWindowMode0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetStartOptionsWindowMode0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsWindowMode_0100 start====");
          let result = osAccountNdk.SetStartOptionsWindowModeNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsWindowMode_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetStartOptionsWindowMode_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_GetStartOptionsWindowMode_0100
     * @tc.name    : testAbilityAbilityBaseGetStartOptionsWindowMode0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseGetStartOptionsWindowMode0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsWindowMode_0100 start====");
          let result = osAccountNdk.GetStartOptionsWindowModeNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsWindowMode_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_GetStartOptionsWindowMode_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetStartOptionsDisplayId_0100
     * @tc.name    : testAbilityAbilityBaseSetStartOptionsDisplayId0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetStartOptionsDisplayId0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsDisplayId_0100 start====");
          let result = osAccountNdk.SetStartOptionsDisplayIdNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsDisplayId_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetStartOptionsDisplayId_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetStartOptionsDisplayId_0200
     * @tc.name    : testAbilityAbilityBaseSetStartOptionsDisplayId0200
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetStartOptionsDisplayId0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsDisplayId_0200 start====");
          let result = osAccountNdk.SetStartOptionsDisplayIdParamInvalid();
          expect(401).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsDisplayId_0200 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetStartOptionsDisplayId_0200 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_GetStartOptionsDisplayId_0100
     * @tc.name    : testAbilityAbilityBaseGetStartOptionsDisplayId0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseGetStartOptionsDisplayId0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsDisplayId_0100 start====");
          let result = osAccountNdk.GetStartOptionsDisplayIdNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsDisplayId_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_GetStartOptionsDisplayId_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetStartOptionsWithAnimation_0100
     * @tc.name    : testAbilityAbilityBaseSetStartOptionsWithAnimation0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetStartOptionsWithAnimation0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsWithAnimation_0100 start====");
          let result = osAccountNdk.SetStartOptionsWithAnimationNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsWithAnimation_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetStartOptionsWithAnimation_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_GetStartOptionsWithAnimation_0100
     * @tc.name    : testAbilityAbilityBaseGetStartOptionsWithAnimation0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseGetStartOptionsWithAnimation0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsWithAnimation_0100 start====");
          let result = osAccountNdk.GetStartOptionsWithAnimationNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsWithAnimation_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_GetStartOptionsWithAnimation_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetStartOptionsWindowLeft_0100
     * @tc.name    : testAbilityAbilityBaseSetStartOptionsWindowLeft0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetStartOptionsWindowLeft0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsWindowLeft_0100 start====");
          let result = osAccountNdk.SetStartOptionsWindowLeftNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsWindowLeft_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetStartOptionsWindowLeft_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_GetStartOptionsWindowLeft_0100
     * @tc.name    : testAbilityAbilityBaseGetStartOptionsWindowLeft0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseGetStartOptionsWindowLeft0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsWindowLeft_0100 start====");
          let result = osAccountNdk.GetStartOptionsWindowLeftNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsWindowLeft_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_GetStartOptionsWindowLeft_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetStartOptionsWindowTop_0100
     * @tc.name    : testAbilityAbilityBaseSetStartOptionsWindowTop0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetStartOptionsWindowTop0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsWindowTop_0100 start====");
          let result = osAccountNdk.SetStartOptionsWindowTopNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsWindowTop_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetStartOptionsWindowTop_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_GetStartOptionsWindowTop_0100
     * @tc.name    : testAbilityAbilityBaseGetStartOptionsWindowTop0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseGetStartOptionsWindowTop0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsWindowTop_0100 start====");
          let result = osAccountNdk.GetStartOptionsWindowTopNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsWindowTop_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_GetStartOptionsWindowTop_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetStartOptionsWindowHeight_0100
     * @tc.name    : testAbilityAbilityBaseSetStartOptionsWindowHeight0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetStartOptionsWindowHeight0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsWindowHeight_0100 start====");
          let result = osAccountNdk.SetStartOptionsWindowHeightNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsWindowHeight_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetStartOptionsWindowHeight_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_GetStartOptionsWindowHeight_0100
     * @tc.name    : testAbilityAbilityBaseGetStartOptionsWindowHeight0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseGetStartOptionsWindowHeight0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsWindowHeight_0100 start====");
          let result = osAccountNdk.GetStartOptionsWindowHeightNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsWindowHeight_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_GetStartOptionsWindowHeight_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetStartOptionsWindowWidth_0100
     * @tc.name    : testAbilityAbilityBaseSetStartOptionsWindowWidth0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetStartOptionsWindowWidth0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsWindowWidth_0100 start====");
          let result = osAccountNdk.SetStartOptionsWindowWidthNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsWindowWidth_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetStartOptionsWindowWidth_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_GetStartOptionsWindowWidth_0100
     * @tc.name    : testAbilityAbilityBaseGetStartOptionsWindowWidth0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseGetStartOptionsWindowWidth0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsWindowWidth_0100 start====");
          let result = osAccountNdk.GetStartOptionsWindowWidthNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsWindowWidth_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_GetStartOptionsWindowWidth_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetStartOptionsStartVisibility_0100
     * @tc.name    : testAbilityAbilityBaseSetStartOptionsStartVisibility0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetStartOptionsStartVisibility0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsStartVisibility_0100 start====");
          let result = osAccountNdk.SetStartOptionsStartVisibilityNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsStartVisibility_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetStartOptionsStartVisibility_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_GetStartOptionsStartVisibility_0100
     * @tc.name    : testAbilityAbilityBaseGetStartOptionsStartVisibility0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseGetStartOptionsStartVisibility0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsStartVisibility_0100 start====");
          let result = osAccountNdk.GetStartOptionsStartVisibilityNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsStartVisibility_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_GetStartOptionsStartVisibility_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetStartOptionsStartWindowIcon_0100
     * @tc.name    : testAbilityAbilityBaseSetStartOptionsStartWindowIcon0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetStartOptionsStartWindowIcon0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsStartWindowIcon_0100 start====");
          let result = osAccountNdk.SetStartOptionsStartWindowIconNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsStartWindowIcon_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetStartOptionsStartWindowIcon_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_GetStartOptionsStartWindowIcon_0100
     * @tc.name    : testAbilityAbilityBaseGetStartOptionsStartWindowIcon0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseGetStartOptionsStartWindowIcon0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsStartWindowIcon_0100 start====");
          let result = osAccountNdk.GetStartOptionsStartWindowIconNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsStartWindowIcon_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_GetStartOptionsStartWindowIcon_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetStartOptionsStartWindowBackgroundColor_0100
     * @tc.name    : testAbilityAbilityBaseSetStartOptionsStartWindowBackgroundColor0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetStartOptionsStartWindowBackgroundColor0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsStartWindowBackgroundColor_0100 start====");
          let result = osAccountNdk.SetStartOptionsStartWindowBackgroundColorNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsStartWindowBackgroundColor_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetStartOptionsStartWindowBackgroundColor_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_GetStartOptionsStartWindowBackgroundColor_0100
     * @tc.name    : testAbilityAbilityBaseGetStartOptionsStartWindowBackgroundColor0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseGetStartOptionsStartWindowBackgroundColor0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsStartWindowBackgroundColor_0100 start====");
          let result = osAccountNdk.GetStartOptionsStartWindowBackgroundColorNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsStartWindowBackgroundColor_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_GetStartOptionsStartWindowBackgroundColor_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetStartOptionsSupportedWindowModes_0100
     * @tc.name    : testAbilityAbilityBaseSetStartOptionsSupportedWindowModes0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetStartOptionsSupportedWindowModes0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsSupportedWindowModes_0100 start====");
          let result = osAccountNdk.SetStartOptionsSupportedWindowModesNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsSupportedWindowModes_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetStartOptionsSupportedWindowModes_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_GetStartOptionsSupportedWindowModes_0100
     * @tc.name    : testAbilityAbilityBaseGetStartOptionsSupportedWindowModes0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseGetStartOptionsSupportedWindowModes0100',
      TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsSupportedWindowModes_0100 start====");
          let result = osAccountNdk.GetStartOptionsSupportedWindowModesNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsSupportedWindowModes_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_GetStartOptionsSupportedWindowModes_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetStartOptionsMinWindowWidth_0100
     * @tc.name    : testAbilityAbilityBaseSetStartOptionsMinWindowWidth0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetStartOptionsMinWindowWidth0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsMinWindowWidth_0100 start====");
          let result = osAccountNdk.SetStartOptionsMinWindowWidthNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsMinWindowWidth_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetStartOptionsMinWindowWidth_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_GetStartOptionsMinWindowWidth_0100
     * @tc.name    : testAbilityAbilityBaseGetStartOptionsMinWindowWidth0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseGetStartOptionsMinWindowWidth0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsMinWindowWidth_0100 start====");
          let result = osAccountNdk.GetStartOptionsMinWindowWidthNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsMinWindowWidth_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_GetStartOptionsMinWindowWidth_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetStartOptionsMaxWindowWidth_0100
     * @tc.name    : testAbilityAbilityBaseSetStartOptionsMaxWindowWidth0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetStartOptionsMaxWindowWidth0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsMaxWindowWidth_0100 start====");
          let result = osAccountNdk.SetStartOptionsMaxWindowWidthNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsMaxWindowWidth_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetStartOptionsMaxWindowWidth_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_GetStartOptionsMaxWindowWidth_0100
     * @tc.name    : testAbilityAbilityBaseGetStartOptionsMaxWindowWidth0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseGetStartOptionsMaxWindowWidth0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsMaxWindowWidth_0100 start====");
          let result = osAccountNdk.GetStartOptionsMaxWindowWidthNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsMaxWindowWidth_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_GetStartOptionsMaxWindowWidth_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetStartOptionsMinWindowHeight_0100
     * @tc.name    : testAbilityAbilityBaseSetStartOptionsMinWindowHeight0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetStartOptionsMinWindowHeight0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsMinWindowHeight_0100 start====");
          let result = osAccountNdk.SetStartOptionsMinWindowHeightNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsMinWindowHeight_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetStartOptionsMinWindowHeight_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_GetStartOptionsMinWindowHeight_0100
     * @tc.name    : testAbilityAbilityBaseGetStartOptionsMinWindowHeight0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseGetStartOptionsMinWindowHeight0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsMinWindowHeight_0100 start====");
          let result = osAccountNdk.GetStartOptionsMinWindowHeightNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsMinWindowHeight_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_GetStartOptionsMinWindowHeight_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_GetStartOptionsMinWindowHeight_0200
     * @tc.name    : testAbilityAbilityBaseGetStartOptionsMinWindowHeight0200
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseGetStartOptionsMinWindowHeight0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsMinWindowHeight_0200 start====");
          let result = osAccountNdk.GetStartOptionsMinWindowHeightParamInvalid();
          expect(401).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsMinWindowHeight_0200 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_GetStartOptionsMinWindowHeight_0200 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_SetStartOptionsMaxWindowHeight_0100
     * @tc.name    : testAbilityAbilityBaseSetStartOptionsMaxWindowHeight0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseSetStartOptionsMaxWindowHeight0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsMaxWindowHeight_0100 start====");
          let result = osAccountNdk.SetStartOptionsMaxWindowHeightNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_SetStartOptionsMaxWindowHeight_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_SetStartOptionsMaxWindowHeight_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })

    /**
     * @tc.number  : SUB_Ability_AbilityBase_GetStartOptionsMaxWindowHeight_0100
     * @tc.name    : testAbilityAbilityBaseGetStartOptionsMaxWindowHeight0100
     * @tc.desc    : Meta-capability testing.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testAbilityAbilityBaseGetStartOptionsMaxWindowHeight0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsMaxWindowHeight_0100 start====");
          let result = osAccountNdk.GetStartOptionsMaxWindowHeightNoError();
          expect(0).assertEqual(result);
          console.info("====>SUB_Ability_AbilityBase_GetStartOptionsMaxWindowHeight_0100 result: " + result);
          done()
        } catch (err) {
          console.error("====>SUB_Ability_AbilityBase_GetStartOptionsMaxWindowHeight_0100 catch err: " + err);
          expect(false).assertEqual(OS_ACCOUNT_ERR_OK);
          done()
        }
      })
  })
}
