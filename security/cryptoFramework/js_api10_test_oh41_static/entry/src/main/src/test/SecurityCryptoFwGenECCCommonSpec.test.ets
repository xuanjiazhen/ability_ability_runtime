import { describe, it, expect, TestType, Size, Level, beforeAll} from "../../../hypium/index";
import hilog from '@ohos.hilog'
import abilityDelegatorRegistry from '@ohos.app.ability.abilityDelegatorRegistry';
import common from '@ohos.app.ability.common';
import UIAbility from '@ohos.app.ability.UIAbility';
import { BusinessError } from '@ohos.base';
import cryptoFramework from '@ohos.security.cryptoFramework';
import abilityDelegatorRegistry from '@ohos.app.ability.abilityDelegatorRegistry';
import Want from '@ohos.app.ability.Want';
import Utils from './Util.test';
import { compareECCParams, comparePairParams, } from './utils/keyGenerate/genECCSpecFunc.test';
import * as asyKeyGenCallbackFunc from './utils/keyGenerate/asyKeyGenCallbackFunc.test';
import * as asyKeyGenPromiseFunc from './utils/keyGenerate/asyKeyGenPromiseFunc.test';
import * as cipherCallbackFunc from './utils/cipher/cipherAsyCallbackFunc.test';
import * as cipherPromiseFunc from './utils/cipher/cipherAsyPromiseFunc.test';
import * as signVerifyPromiseFunc from './utils/signVerify/signVerifyAsyPromiseFunc.test';
import * as signVerifyCallbackFunc from './utils/signVerify/signVerifyAsyCallbackFunc.test';
import * as agreementAsyCallbackFunc from './utils/agreement/agreementAsyCallbackFunc.test';
import * as agreementAsyPromiseFunc from './utils/agreement/agreementAsyPromiseFunc.test';
let domain: number = 0x0000; //日志标识,0x0000作为测试框架的业务标识
let tag: string = 'testTag'; //日志标识字符串,作为tag标识当前runner类下的测试行为
let testAbilityContext:common.UIAbilityContext;

export default function SecurityCryptoFwGenECCCommonSpecTestUnit() {

  describe("SecurityCryptoFwGenECCCommonSpecTestUnit", (): void => {
    hilog.info(domain, tag, '%{public}s', 'describe start');

    /**
     * @tc.number Security_CryptoFramework_genECCCommonSpec_Func_0100
     * @tc.name Security_CryptoFramework_genECCCommonSpec_Func_0100
     * @tc.desc 1. genECCCommonParamSpec with curveName 'NID_brainpoolP160r1'
     * @tc.size Medium
     * @tc.type Func
     * @tc.level Level4
     */
    it("Security_CryptoFramework_genECCCommonSpec_Func_0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL4, async (done: () => void): Promise<void> => {
      console.log(`>>Security_CryptoFramework_genECCCommonSpec_Func_0100: started.`);
      let curveName: string = "NID_brainpoolP160r1";
      try {
        let commonSpec: cryptoFramework.ECCCommonParamsSpec = cryptoFramework.ECCKeyUtil.genECCCommonParamsSpec(curveName);
        await asyKeyGenPromiseFunc.genKeyPair(true, commonSpec, curveName);
        console.log(`Security_CryptoFramework_genECCCommonSpec_Func_0100: ` + curveName + ` generate keyPair succeed.`);
      } catch (err: BusinessError) {
        console.error(`Security_CryptoFramework_genECCCommonSpec_Func_0100: ` + curveName + ` catch error: “${err}“, error code: ${err.code}`);
        expect(null).assertFail();
      }
      console.log(`Security_CryptoFramework_genECCCommonSpec_Func_0100: succeed.`);
      done();
    });

    /**
     * @tc.number Security_CryptoFramework_genECCCommonSpec_Func_0200
     * @tc.name Security_CryptoFramework_genECCCommonSpec_Func_0200
     * @tc.desc 1. genECCCommonParamSpec with curveName 'NID_brainpoolP160t1'
     * @tc.size Medium
     * @tc.type Func
     * @tc.level Level4
     */
    it("Security_CryptoFramework_genECCCommonSpec_Func_0200", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL4, async (done: () => void): Promise<void> => {
      console.log(`>>Security_CryptoFramework_genECCCommonSpec_Func_0200: started.`);
      let curveName: string = "NID_brainpoolP160t1";
      try {
        let commonSpec: cryptoFramework.ECCCommonParamsSpec = cryptoFramework.ECCKeyUtil.genECCCommonParamsSpec(curveName);
        await asyKeyGenPromiseFunc.genKeyPair(true, commonSpec, curveName);
        console.log(`Security_CryptoFramework_genECCCommonSpec_Func_0200: ` + curveName + ` generate keyPair succeed.`);
      } catch (err: BusinessError) {
        console.error(`Security_CryptoFramework_genECCCommonSpec_Func_0200: ` + curveName + ` catch error: “${err}“, error code: ${err.code}`);
        expect(null).assertFail();
      }
      console.log(`Security_CryptoFramework_genECCCommonSpec_Func_0200: succeed.`);
      done();
    });

    /**
     * @tc.number Security_CryptoFramework_genECCCommonSpec_Func_0300
     * @tc.name Security_CryptoFramework_genECCCommonSpec_Func_0300
     * @tc.desc 1. genECCCommonParamSpec with curveName 'NID_brainpoolP192r1'
     * @tc.size Medium
     * @tc.type Func
     * @tc.level Level4
     */
    it("Security_CryptoFramework_genECCCommonSpec_Func_0300", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL4, async (done: () => void): Promise<void> => {
      console.log(`>>Security_CryptoFramework_genECCCommonSpec_Func_0300: started.`);
      let curveName: string = "NID_brainpoolP192r1";
      try {
        let commonSpec: cryptoFramework.ECCCommonParamsSpec = cryptoFramework.ECCKeyUtil.genECCCommonParamsSpec(curveName);
        await asyKeyGenPromiseFunc.genKeyPair(true, commonSpec, curveName);
        console.log(`Security_CryptoFramework_genECCCommonSpec_Func_0300: ` + curveName + ` generate keyPair succeed.`);
      } catch (err: BusinessError) {
        console.error(`Security_CryptoFramework_genECCCommonSpec_Func_0300: ` + curveName + ` catch error: “${err}“, error code: ${err.code}`);
        expect(null).assertFail();
      }
      console.log(`Security_CryptoFramework_genECCCommonSpec_Func_0300: succeed.`);
      done();
    });

    /**
     * @tc.number Security_CryptoFramework_genECCCommonSpec_Func_0400
     * @tc.name Security_CryptoFramework_genECCCommonSpec_Func_0400
     * @tc.desc 1. genECCCommonParamSpec with curveName 'NID_brainpoolP192t1'
     * @tc.size Medium
     * @tc.type Func
     * @tc.level Level4
     */
    it("Security_CryptoFramework_genECCCommonSpec_Func_0400", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL4, async (done: () => void): Promise<void> => {
      console.log(`>>Security_CryptoFramework_genECCCommonSpec_Func_0400: started.`);
      let curveName: string = "NID_brainpoolP192t1";
      try {
        let commonSpec: cryptoFramework.ECCCommonParamsSpec = cryptoFramework.ECCKeyUtil.genECCCommonParamsSpec(curveName);
        await asyKeyGenPromiseFunc.genKeyPair(true, commonSpec, curveName);
        console.log(`Security_CryptoFramework_genECCCommonSpec_Func_0400: ` + curveName + ` generate keyPair succeed.`);
      } catch (err: BusinessError) {
        console.error(`Security_CryptoFramework_genECCCommonSpec_Func_0400: ` + curveName + ` catch error: “${err}“, error code: ${err.code}`);
        expect(null).assertFail();
      }
      console.log(`Security_CryptoFramework_genECCCommonSpec_Func_0400: succeed.`);
      done();
    });

    /**
     * @tc.number Security_CryptoFramework_genECCCommonSpec_Func_0500
     * @tc.name Security_CryptoFramework_genECCCommonSpec_Func_0500
     * @tc.desc 1. genECCCommonParamSpec with curveName 'NID_brainpoolP224r1'
     * @tc.size Medium
     * @tc.type Func
     * @tc.level Level4
     */
    it("Security_CryptoFramework_genECCCommonSpec_Func_0500", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL4, async (done: () => void): Promise<void> => {
      console.log(`>>Security_CryptoFramework_genECCCommonSpec_Func_0500: started.`);
      let curveName: string = "NID_brainpoolP224r1";
      try {
        let commonSpec: cryptoFramework.ECCCommonParamsSpec = cryptoFramework.ECCKeyUtil.genECCCommonParamsSpec(curveName);
        await asyKeyGenPromiseFunc.genKeyPair(true, commonSpec, curveName);
        console.log(`Security_CryptoFramework_genECCCommonSpec_Func_0500: ` + curveName + ` generate keyPair succeed.`);
      } catch (err: BusinessError) {
        console.error(`Security_CryptoFramework_genECCCommonSpec_Func_0500: ` + curveName + ` catch error: “${err}“, error code: ${err.code}`);
        expect(null).assertFail();
      }
      console.log(`Security_CryptoFramework_genECCCommonSpec_Func_0500: succeed.`);
      done();
    });

    /**
     * @tc.number Security_CryptoFramework_genECCCommonSpec_Func_0600
     * @tc.name Security_CryptoFramework_genECCCommonSpec_Func_0600
     * @tc.desc 1. genECCCommonParamSpec with curveName 'NID_brainpoolP224t1'
     * @tc.size Medium
     * @tc.type Func
     * @tc.level Level4
     */
    it("Security_CryptoFramework_genECCCommonSpec_Func_0600", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL4, async (done: () => void): Promise<void> => {
      console.log(`>>Security_CryptoFramework_genECCCommonSpec_Func_0600: started.`);
      let curveName: string = "NID_brainpoolP224t1";
      try {
        let commonSpec: cryptoFramework.ECCCommonParamsSpec = cryptoFramework.ECCKeyUtil.genECCCommonParamsSpec(curveName);
        await asyKeyGenPromiseFunc.genKeyPair(true, commonSpec, curveName);
        console.log(`Security_CryptoFramework_genECCCommonSpec_Func_0600: ` + curveName + ` generate keyPair succeed.`);
      } catch (err: BusinessError) {
        console.error(`Security_CryptoFramework_genECCCommonSpec_Func_0600: ` + curveName + ` catch error: “${err}“, error code: ${err.code}`);
        expect(null).assertFail();
      }
      console.log(`Security_CryptoFramework_genECCCommonSpec_Func_0600: succeed.`);
      done();
    });

    /**
     * @tc.number Security_CryptoFramework_genECCCommonSpec_Func_0700
     * @tc.name Security_CryptoFramework_genECCCommonSpec_Func_0700
     * @tc.desc 1. genECCCommonParamSpec with curveName 'NID_brainpoolP256r1'
     * @tc.size Medium
     * @tc.type Func
     * @tc.level Level4
     */
    it("Security_CryptoFramework_genECCCommonSpec_Func_0700", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL4, async (done: () => void): Promise<void> => {
      console.log(`>>Security_CryptoFramework_genECCCommonSpec_Func_0700: started.`);
      let curveName: string = "NID_brainpoolP256r1";
      try {
        let commonSpec: cryptoFramework.ECCCommonParamsSpec = cryptoFramework.ECCKeyUtil.genECCCommonParamsSpec(curveName);
        await asyKeyGenPromiseFunc.genKeyPair(true, commonSpec, curveName);
        console.log(`Security_CryptoFramework_genECCCommonSpec_Func_0700: ` + curveName + ` generate keyPair succeed.`);
      } catch (err: BusinessError) {
        console.error(`Security_CryptoFramework_genECCCommonSpec_Func_0700: ` + curveName + ` catch error: “${err}“, error code: ${err.code}`);
        expect(null).assertFail();
      }
      console.log(`Security_CryptoFramework_genECCCommonSpec_Func_0700: succeed.`);
      done();
    });

    /**
     * @tc.number Security_CryptoFramework_genECCCommonSpec_Func_0800
     * @tc.name Security_CryptoFramework_genECCCommonSpec_Func_0800
     * @tc.desc 1. genECCCommonParamSpec with curveName 'NID_brainpoolP256t1'
     * @tc.size Medium
     * @tc.type Func
     * @tc.level Level3
     */
    it("Security_CryptoFramework_genECCCommonSpec_Func_0800", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, async (done: () => void): Promise<void> => {
      console.log(`>>Security_CryptoFramework_genECCCommonSpec_Func_0800: started.`);
      let curveName: string = "NID_brainpoolP256t1";
      try {
        let commonSpec: cryptoFramework.ECCCommonParamsSpec = cryptoFramework.ECCKeyUtil.genECCCommonParamsSpec(curveName);
        await asyKeyGenPromiseFunc.genKeyPair(true, commonSpec, curveName);
        console.log(`Security_CryptoFramework_genECCCommonSpec_Func_0800: ` + curveName + ` generate keyPair succeed.`);
      } catch (err: BusinessError) {
        console.error(`Security_CryptoFramework_genECCCommonSpec_Func_0800: ` + curveName + ` catch error: “${err}“, error code: ${err.code}`);
        expect(null).assertFail();
      }
      console.log(`Security_CryptoFramework_genECCCommonSpec_Func_0800: succeed.`);
      done();
    });

    /**
     * @tc.number Security_CryptoFramework_genECCCommonSpec_Func_0900
     * @tc.name Security_CryptoFramework_genECCCommonSpec_Func_0900
     * @tc.desc 1. genECCCommonParamSpec with curveName 'NID_brainpoolP320r1'
     * @tc.size Medium
     * @tc.type Func
     * @tc.level Level3
     */
    it("Security_CryptoFramework_genECCCommonSpec_Func_0900", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, async (done: () => void): Promise<void> => {
      console.log(`>>Security_CryptoFramework_genECCCommonSpec_Func_0900: started.`);
      let curveName: string = "NID_brainpoolP320r1";
      try {
        let commonSpec: cryptoFramework.ECCCommonParamsSpec = cryptoFramework.ECCKeyUtil.genECCCommonParamsSpec(curveName);
        await asyKeyGenPromiseFunc.genKeyPair(true, commonSpec, curveName);
        console.log(`Security_CryptoFramework_genECCCommonSpec_Func_0900: ` + curveName + ` generate keyPair succeed.`);
      } catch (err: BusinessError) {
        console.error(`Security_CryptoFramework_genECCCommonSpec_Func_0900: ` + curveName + ` catch error: “${err}“, error code: ${err.code}`);
        expect(null).assertFail();
      }
      console.log(`Security_CryptoFramework_genECCCommonSpec_Func_0900: succeed.`);
      done();
    });

    /**
     * @tc.number Security_CryptoFramework_genECCCommonSpec_Func_1000
     * @tc.name Security_CryptoFramework_genECCCommonSpec_Func_1000
     * @tc.desc 1. genECCCommonParamSpec with curveName 'NID_brainpoolP320t1'
     * @tc.size Medium
     * @tc.type Func
     * @tc.level Level3
     */
    it("Security_CryptoFramework_genECCCommonSpec_Func_1000", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, async (done: () => void): Promise<void> => {
      console.log(`>>Security_CryptoFramework_genECCCommonSpec_Func_1000: started.`);
      let curveName: string = "NID_brainpoolP320t1";
      try {
        let commonSpec: cryptoFramework.ECCCommonParamsSpec = cryptoFramework.ECCKeyUtil.genECCCommonParamsSpec(curveName);
        await asyKeyGenPromiseFunc.genKeyPair(true, commonSpec, curveName);
        console.log(`Security_CryptoFramework_genECCCommonSpec_Func_1000: ` + curveName + ` generate keyPair succeed.`);
      } catch (err: BusinessError) {
        console.error(`Security_CryptoFramework_genECCCommonSpec_Func_1000: ` + curveName + ` catch error: “${err}“, error code: ${err.code}`);
        expect(null).assertFail();
      }
      console.log(`Security_CryptoFramework_genECCCommonSpec_Func_1000: succeed.`);
      done();
    });

    /**
     * @tc.number Security_CryptoFramework_genECCCommonSpec_Func_1100
     * @tc.name Security_CryptoFramework_genECCCommonSpec_Func_1100
     * @tc.desc 1. genECCCommonParamSpec with curveName 'NID_brainpoolP384r1'
     * @tc.size Medium
     * @tc.type Func
     * @tc.level Level3
     */
    it("Security_CryptoFramework_genECCCommonSpec_Func_1100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, async (done: () => void): Promise<void> => {
      console.log(`>>Security_CryptoFramework_genECCCommonSpec_Func_1100: started.`);
      let curveName: string = "NID_brainpoolP384r1";
      try {
        let commonSpec: cryptoFramework.ECCCommonParamsSpec = cryptoFramework.ECCKeyUtil.genECCCommonParamsSpec(curveName);
        await asyKeyGenPromiseFunc.genKeyPair(true, commonSpec, curveName);
        console.log(`Security_CryptoFramework_genECCCommonSpec_Func_1100: ` + curveName + ` generate keyPair succeed.`);
      } catch (err: BusinessError) {
        console.error(`Security_CryptoFramework_genECCCommonSpec_Func_1100: ` + curveName + ` catch error: “${err}“, error code: ${err.code}`);
        expect(null).assertFail();
      }
      console.log(`Security_CryptoFramework_genECCCommonSpec_Func_1100: succeed.`);
      done();
    });

    /**
     * @tc.number Security_CryptoFramework_genECCCommonSpec_Func_1200
     * @tc.name Security_CryptoFramework_genECCCommonSpec_Func_1200
     * @tc.desc 1. genECCCommonParamSpec with curveName 'NID_brainpoolP384t1'
     * @tc.size Medium
     * @tc.type Func
     * @tc.level Level3
     */
    it("Security_CryptoFramework_genECCCommonSpec_Func_1200", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, async (done: () => void): Promise<void> => {
      console.log(`>>Security_CryptoFramework_genECCCommonSpec_Func_1200: started.`);
      let curveName: string = "NID_brainpoolP384t1";
      try {
        let commonSpec: cryptoFramework.ECCCommonParamsSpec = cryptoFramework.ECCKeyUtil.genECCCommonParamsSpec(curveName);
        await asyKeyGenPromiseFunc.genKeyPair(true, commonSpec, curveName);
        console.log(`Security_CryptoFramework_genECCCommonSpec_Func_1200: ` + curveName + ` generate keyPair succeed.`);
      } catch (err: BusinessError) {
        console.error(`Security_CryptoFramework_genECCCommonSpec_Func_1200: ` + curveName + ` catch error: “${err}“, error code: ${err.code}`);
        expect(null).assertFail();
      }
      console.log(`Security_CryptoFramework_genECCCommonSpec_Func_1200: succeed.`);
      done();
    });

    /**
     * @tc.number Security_CryptoFramework_genECCCommonSpec_Func_1300
     * @tc.name Security_CryptoFramework_genECCCommonSpec_Func_1300
     * @tc.desc 1. genECCCommonParamSpec with curveName 'NID_brainpoolP512r1'
     * @tc.size Medium
     * @tc.type Func
     * @tc.level Level3
     */
    it("Security_CryptoFramework_genECCCommonSpec_Func_1300", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, async (done: () => void): Promise<void> => {
      console.log(`>>Security_CryptoFramework_genECCCommonSpec_Func_1300: started.`);
      let curveName: string = "NID_brainpoolP512r1";
      try {
        let commonSpec: cryptoFramework.ECCCommonParamsSpec = cryptoFramework.ECCKeyUtil.genECCCommonParamsSpec(curveName);
        await asyKeyGenPromiseFunc.genKeyPair(true, commonSpec, curveName);
        console.log(`Security_CryptoFramework_genECCCommonSpec_Func_1300: ` + curveName + ` generate keyPair succeed.`);
      } catch (err: BusinessError) {
        console.error(`Security_CryptoFramework_genECCCommonSpec_Func_1300: ` + curveName + ` catch error: “${err}“, error code: ${err.code}`);
        expect(null).assertFail();
      }
      console.log(`Security_CryptoFramework_genECCCommonSpec_Func_1300: succeed.`);
      done();
    });

    /**
     * @tc.number Security_CryptoFramework_genECCCommonSpec_Func_1400
     * @tc.name Security_CryptoFramework_genECCCommonSpec_Func_1400
     * @tc.desc 1. genECCCommonParamSpec with curveName 'NID_brainpoolP512t1'
     * @tc.size Medium
     * @tc.type Func
     * @tc.level Level3
     */
    it("Security_CryptoFramework_genECCCommonSpec_Func_1400", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, async (done: () => void): Promise<void> => {
      console.log(`>>Security_CryptoFramework_genECCCommonSpec_Func_1400: started.`);
      let curveName: string = "NID_brainpoolP512t1";
      try {
        let commonSpec: cryptoFramework.ECCCommonParamsSpec = cryptoFramework.ECCKeyUtil.genECCCommonParamsSpec(curveName);
        await asyKeyGenPromiseFunc.genKeyPair(true, commonSpec, curveName);
        console.log(`Security_CryptoFramework_genECCCommonSpec_Func_1400: ` + curveName + ` generate keyPair succeed.`);
      } catch (err: BusinessError) {
        console.error(`Security_CryptoFramework_genECCCommonSpec_Func_1400: ` + curveName + ` catch error: “${err}“, error code: ${err.code}`);
        expect(null).assertFail();
      }
      console.log(`Security_CryptoFramework_genECCCommonSpec_Func_1400: succeed.`);
      done();
    });

    /**
     * @tc.number Security_CryptoFramework_genECCCommonSpec_Func_1500
     * @tc.name Security_CryptoFramework_genECCCommonSpec_Func_1500
     * @tc.desc 1. genECCCommonParamSpec with curveName 'NID_secp224r1'
     * @tc.size Medium
     * @tc.type Func
     * @tc.level Level2
     */
    it("Security_CryptoFramework_genECCCommonSpec_Func_1500", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: () => void): Promise<void> => {
      console.log(`>>Security_CryptoFramework_genECCCommonSpec_Func_1500: started.`);
      let curveName: string = "NID_secp224r1";
      try {
        let commonSpec: cryptoFramework.ECCCommonParamsSpec = cryptoFramework.ECCKeyUtil.genECCCommonParamsSpec(curveName);
        await asyKeyGenPromiseFunc.genKeyPair(true, commonSpec, curveName);
        console.log(`Security_CryptoFramework_genECCCommonSpec_Func_1500: ` + curveName + ` generate keyPair succeed.`);
      } catch (err: BusinessError) {
        console.error(`Security_CryptoFramework_genECCCommonSpec_Func_1500: ` + curveName + ` catch error: “${err}“, error code: ${err.code}`);
        expect(null).assertFail();
      }
      console.log(`Security_CryptoFramework_genECCCommonSpec_Func_1500: succeed.`);
      done();
    });

    /**
     * @tc.number Security_CryptoFramework_genECCCommonSpec_Func_1600
     * @tc.name Security_CryptoFramework_genECCCommonSpec_Func_1600
     * @tc.desc 1. genECCCommonParamSpec with curveName 'NID_secp384r1'
     * @tc.size Medium
     * @tc.type Func
     * @tc.level Level2
     */
    it("Security_CryptoFramework_genECCCommonSpec_Func_1600", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: () => void): Promise<void> => {
      console.log(`>>Security_CryptoFramework_genECCCommonSpec_Func_1600: started.`);
      let curveName: string = "NID_secp384r1";
      try {
        let commonSpec: cryptoFramework.ECCCommonParamsSpec = cryptoFramework.ECCKeyUtil.genECCCommonParamsSpec(curveName);
        await asyKeyGenPromiseFunc.genKeyPair(true, commonSpec, curveName);
        console.log(`Security_CryptoFramework_genECCCommonSpec_Func_1600: ` + curveName + ` generate keyPair succeed.`);
      } catch (err: BusinessError) {
        console.error(`Security_CryptoFramework_genECCCommonSpec_Func_1600: ` + curveName + ` catch error: “${err}“, error code: ${err.code}`);
        expect(null).assertFail();
      }
      console.log(`Security_CryptoFramework_genECCCommonSpec_Func_1600: succeed.`);
      done();
    });

    /**
     * @tc.number Security_CryptoFramework_genECCCommonSpec_Func_1700
     * @tc.name Security_CryptoFramework_genECCCommonSpec_Func_1700
     * @tc.desc 1. genECCCommonParamSpec with curveName 'NID_secp521r1'
     * @tc.size Medium
     * @tc.type Func
     * @tc.level Level2
     */
    it("Security_CryptoFramework_genECCCommonSpec_Func_1700", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: () => void): Promise<void> => {
      console.log(`>>Security_CryptoFramework_genECCCommonSpec_Func_1700 started.`);
      let curveName: string = "NID_secp521r1";
      try {
        let commonSpec: cryptoFramework.ECCCommonParamsSpec = cryptoFramework.ECCKeyUtil.genECCCommonParamsSpec(curveName);
        await asyKeyGenPromiseFunc.genKeyPair(true, commonSpec, curveName);
        console.log(`Security_CryptoFramework_genECCCommonSpec_Func_1700: ` + curveName + ` generate keyPair succeed.`);
      } catch (err: BusinessError) {
        console.error(`Security_CryptoFramework_genECCCommonSpec_Func_1700: ` + curveName + ` catch error: “${err}“, error code: ${err.code}`);
        expect(null).assertFail();
      }
      console.log(`Security_CryptoFramework_genECCCommonSpec_Func_1700: succeed.`);
      done();
    });

    /**
     * @tc.number Security_CryptoFramework_genECCCommonSpec_Func_1800
     * @tc.name Security_CryptoFramework_genECCCommonSpec_Func_1800
     * @tc.desc 1. genECCCommonParamSpec with error curveName 'RSA512'
     * @tc.size Medium
     * @tc.type Func
     * @tc.level Level3
     */
    it("Security_CryptoFramework_genECCCommonSpec_Func_1800", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, async (done: () => void): Promise<void> => {
      console.log(`>>Security_CryptoFramework_genECCCommonSpec_Func_1800 started.`);
      let curveName: string = "RSA512";
      try {
        let commonSpec: cryptoFramework.ECCCommonParamsSpec = cryptoFramework.ECCKeyUtil.genECCCommonParamsSpec(curveName);
        await asyKeyGenPromiseFunc.genKeyPair(true, commonSpec, curveName);
        console.log(`Security_CryptoFramework_genECCCommonSpec_Func_1800: ` + curveName + ` generate keyPair succeed.`);
        expect(null).assertFail();
      } catch (err: BusinessError) {
        expect(err.code).assertEqual(401);
        console.error(`Security_CryptoFramework_genECCCommonSpec_Func_1800: ` + curveName + ` catch error: “${err}“, error code: ${err.code}`);
      }
      console.log(`Security_CryptoFramework_genECCCommonSpec_Func_1800: succeed.`);
      done();
    });

    /**
     * @tc.number Security_CryptoFramework_genECCCommonSpec_Func_2100
     * @tc.name Security_CryptoFramework_genECCCommonSpec_Func_2100
     * @tc.desc 1. genECCCommonParamSpec with curveName 'NID_sm2'
     * @tc.size Medium
     * @tc.type Func
     * @tc.level Level4
     */
    it("Security_CryptoFramework_genECCCommonSpec_Func_2100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL4, async (done: () => void): Promise<void> => {
      console.log(`>>Security_CryptoFramework_genECCCommonSpec_Func_2100 started.`);
      let curveName: string = "NID_sm2";
      let signVerifyAlgName: string = "SM2_256|SM3";
      try {
        let commonSpec: cryptoFramework.ECCCommonParamsSpec = cryptoFramework.ECCKeyUtil.genECCCommonParamsSpec(curveName);
        let res: boolean = await signVerifyPromiseFunc.normalProcess(signVerifyAlgName, true, commonSpec, curveName);
        expect(res).assertTrue();
        console.log(`Security_CryptoFramework_genECCCommonSpec_Func_2100: signVerify succeed.`);
      } catch (err: BusinessError) {
        console.error(`Security_CryptoFramework_genECCCommonSpec_Func_2100: catch error: “${err}“, error code: ${err.code}`);
        expect(null).assertFail();
      }
      console.log(`Security_CryptoFramework_genECCCommonSpec_Func_2100: succeed.`);
      done();
    });


  })

}