import { describe, beforeAll, it, expect, TestType, Size, Level } from '@ohos/hypium'
import arkuikeycodendk from 'libarkuikeycodendk.so'
import { Driver } from '@kit.TestKit';
import Utils from '../common/Utils';

const KEY_CODE_EVENT_OK = 1;
const KEY_CODE_EVENT_ERROR = 0;

export default function ArkUIKeyCodeNdkTest2() {
  describe('ArkUIKeyCodeNdkTest2', () => {
    beforeAll(async (done: Function) => {
      await Utils.pushPage(`ArkUIKeyCodeTest/ArkUIKeyCodeTest2`, done);
    });

    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_J_0100
     * @tc.name    : testArkuiKeycodeJ0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_J are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeJ0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeJ0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2026);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventJ();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeJ0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })

    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_K_0100
     * @tc.name    : testArkuiKeycodeK0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_K are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeK0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeK0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2027);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventK();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeK0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_L_0100
     * @tc.name    : testArkuiKeycodeL0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_L are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeL0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeL0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2028);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventL();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeL0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_M_0100
     * @tc.name    : testArkuiKeycodeM0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_M are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeM0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeM0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2029);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventM();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeM0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_N_0100
     * @tc.name    : testArkuiKeycodeN0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_N are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeN0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeN0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2030);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventN();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeN0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_O_0100
     * @tc.name    : testArkuiKeycodeO0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_O are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeO0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeO0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2031);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventO();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeO0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_P_0100
     * @tc.name    : testArkuiKeycodeP0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_P are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeP0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeP0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2032);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventP();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeP0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_Q_0100
     * @tc.name    : testArkuiKeycodeQ0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_Q are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeQ0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeQ0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2033);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventQ();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeQ0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_R_0100
     * @tc.name    : testArkuiKeycodeR0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_R are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeR0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeR0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2034);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventR();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeR0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_S_0100
     * @tc.name    : testArkuiKeycodeS0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_S are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeS0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeS0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2035);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventS();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeS0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_T_0100
     * @tc.name    : testArkuiKeycodeT0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_T are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeT0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeT0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2036);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventT();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeT0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_U_0100
     * @tc.name    : testArkuiKeycodeU0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_U are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeU0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeU0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2037);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventU();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeU0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_V_0100
     * @tc.name    : testArkuiKeycodeV0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_V are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeV0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeV0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2038);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventV();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeV0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_W_0100
     * @tc.name    : testArkuiKeycodeW0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_W are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeW0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeW0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2039);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventW();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeW0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_X_0100
     * @tc.name    : testArkuiKeycodeX0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_X are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeX0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeX0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2040);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventX();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeX0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_Y_0100
     * @tc.name    : testArkuiKeycodeY0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_Y are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeY0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeY0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2041);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventY();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeY0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_Z_0100
     * @tc.name    : testArkuiKeycodeZ0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_Z are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeZ0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeZ0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2042);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventZ();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeZ0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_COMMA_0100
     * @tc.name    : testArkuiKeycodeComma0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_COMMA are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeComma0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeComma0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2043);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventComma();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeComma0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_PERIOD_0100
     * @tc.name    : testArkuiKeycodePeriod0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_PERIOD are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodePeriod0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodePeriod0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2044);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventPeriod();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodePeriod0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_ALT_LEFT_0100
     * @tc.name    : testArkuiKeycodeAltLeft0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_ALT_LEFT are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeAltLeft0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeAltLeft0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2045);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventAltLeft();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeAltLeft0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_ALT_RIGHT_0100
     * @tc.name    : testArkuiKeycodeAltRight0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_ALT_RIGHT are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeAltRight0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeAltRight0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2046);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventAltRight();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeAltRight0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_SHIFT_LEFT_0100
     * @tc.name    : testArkuiKeycodeShiftLeft0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_SHIFT_LEFT are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeShiftLeft0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeShiftLeft0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2047);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventShiftLeft();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeShiftLeft0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_SHIFT_RIGHT_0100
     * @tc.name    : testArkuiKeycodeShiftRight0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_SHIFT_RIGHT are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeShiftRight0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeShiftRight0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2048);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventShiftRight();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeShiftRight0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_TAB_0100
     * @tc.name    : testArkuiKeycodeTab0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_TAB are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeTab0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeTab0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2049);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventTab();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeTab0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_SPACE_0100
     * @tc.name    : testArkuiKeycodeSpace0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_SPACE are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeSpace0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeSpace0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2050);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventSpace();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeSpace0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_SYM_0100
     * @tc.name    : testArkuiKeycodeSym0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_SYM are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeSym0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeSym0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2051);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventSym();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeSym0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_EXPLORER_0100
     * @tc.name    : testArkuiKeycodeExplorer0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_EXPLORER are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeExplorer0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeExplorer0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2052);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventExplorer();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeExplorer0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_ENVELOPE_0100
     * @tc.name    : testArkuiKeycodeEnvelope0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_ENVELOPE are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeEnvelope0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeEnvelope0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2053);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventEnvelope();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeEnvelope0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_ENTER_0100
     * @tc.name    : testArkuiKeycodeEnter0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_ENTER are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeEnter0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeEnter0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2054);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventEnter();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeEnter0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_DEL_0100
     * @tc.name    : testArkuiKeycodeDel0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_DEL are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeDel0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeDel0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2055);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventDel();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeDel0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_GRAVE_0100
     * @tc.name    : testArkuiKeycodeGrave0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_GRAVE are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeGrave0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeGrave0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2056);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventGrave();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeGrave0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_MINUS_0100
     * @tc.name    : testArkuiKeycodeMinus0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_MINUS are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeMinus0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeMinus0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2057);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventMinus();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeMinus0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_EQUALS_0100
     * @tc.name    : testArkuiKeycodeEquals0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_EQUALS are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeEquals0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeEquals0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2058);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventEquals();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeEquals0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_LEFT_BRACKET_0100
     * @tc.name    : testArkuiKeycodeLeftBracket0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_LEFT_BRACKET are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeLeftBracket0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeLeftBracket0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2059);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventLeftBracket();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeLeftBracket0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_RIGHT_BRACKET_0100
     * @tc.name    : testArkuiKeycodeRightBracket0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_RIGHT_BRACKET are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeRightBracket0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeRightBracket0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2060);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventRightBracket();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeRightBracket0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })
    /**
     * @tc.number  : SUB_ARKUI_ACE_ENGINE_ARKUI_KEYCODE_BACKSLASH_0100
     * @tc.name    : testArkuiKeycodeBacksLash0100
     * @tc.desc    : Test whether the key ARKUI_KEYCODE_BACKSLASH are working properly.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkuiKeycodeBacksLash0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkuiKeycodeBacksLash0100 start====");
          let driver = await Driver.create()
          Utils.sleep(1000)
          await driver.triggerKey(2061);
          Utils.sleep(1000)
          let result: number = arkuikeycodendk.GetArkUIKeyCodeEventBackslash();
          expect(KEY_CODE_EVENT_OK).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>testArkuiKeycodeBacksLash0100 catch err: " + err);
          expect(false).assertEqual(KEY_CODE_EVENT_ERROR);
        }
      })


  })
}