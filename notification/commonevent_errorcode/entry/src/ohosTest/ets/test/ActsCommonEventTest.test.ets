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

import { describe, it, expect, Level } from '@ohos/hypium'
import commoneventerror from 'libcommoneventerrorcode.so'
const Success = 0;
const InvalidParameter = 401

export default function ActsCommonEventErrorTest() {
  describe('ActsCommonEventErrorTest', () => {
    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_Notification_Ces_CommonEvent_InvalidParameter_0100', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_Notification_Ces_CommonEvent_InvalidParameter_0100 start====");
        let result: number = commoneventerror.testCommonEventInvalidParameter();
        expect(result).assertEqual(InvalidParameter);
        console.info("====>Sub_Notification_Ces_CommonEvent_InvalidParameter_0100 result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_Notification_Ces_CommonEvent_InvalidParameter_0100 catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_SetPublisherPermissionSuccess_0200
     * @tc.name    : testNotificationCesCommonEventSetPublisherPermissionSuccess0200.
     * @tc.desc    : Test SetPublisherPermissionSuccess
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_Notification_Ces_CommonEvent_SetPublisherPermissionSuccess_0200', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_Notification_Ces_CommonEvent_SetPublisherPermissionSuccess_0200 start====");
        let result: number = commoneventerror.SetPublisherPermissionSuccess();
        expect(result).assertEqual(Success);
        console.info("====>Sub_Notification_Ces_CommonEvent_SetPublisherPermissionSuccess_0200 result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_Notification_Ces_CommonEvent_SetPublisherPermissionSuccess_0200 catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_SetPublisherPermissionError_0300
     * @tc.name    : testNotificationCesCommonEventSetPublisherPermissionError0300.
     * @tc.desc    : Test SetPublisherPermissionError
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_Notification_Ces_CommonEvent_SetPublisherPermissionError_0300', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_Notification_Ces_CommonEvent_SetPublisherPermissionError_0300 start====");
        let result: number = commoneventerror.SetPublisherPermissionError();
        expect(result).assertEqual(InvalidParameter);
        console.info("====>Sub_Notification_Ces_CommonEvent_SetPublisherPermissionError_0300 result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_Notification_Ces_CommonEvent_SetPublisherPermissionError_0300 catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_SetPublisherBundleNameSuccess_0400
     * @tc.name    : testNotificationCesCommonEventSetPublisherBundleNameSuccess0400.
     * @tc.desc    : Test SetPublisherBundleNameSuccess
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_Notification_Ces_CommonEvent_SetPublisherBundleNameSuccess_0400', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_Notification_Ces_CommonEvent_SetPublisherBundleNameSuccess_0400 start====");
        let result: number = commoneventerror.SetPublisherBundleNameSuccess();
        expect(result).assertEqual(Success);
        console.info("====>Sub_Notification_Ces_CommonEvent_SetPublisherBundleNameSuccess_0400 result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_Notification_Ces_CommonEvent_SetPublisherBundleNameSuccess_0400 catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_SetPublisherBundleNameError_0500
     * @tc.name    : testNotificationCesCommonEventSetPublisherBundleNameError0500.
     * @tc.desc    : Test SetPublisherBundleNameError
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_Notification_Ces_CommonEvent_SetPublisherBundleNameError_0500', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_Notification_Ces_CommonEvent_SetPublisherBundleNameError_0500 start====");
        let result = commoneventerror.SetPublisherBundleNameError();
        console.info("====>Sub_Notification_Ces_CommonEvent_SetPublisherBundleNameError_0500 result: " + result);
        expect(result).assertEqual(InvalidParameter);
        done()
      } catch (err) {
        console.error("====>Sub_Notification_Ces_CommonEvent_SetPublisherBundleNameError_0500 catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0600
     * @tc.name    : testNotificationCesCommonEventNoError0600.
     * @tc.desc    : Test SetPublishInfoBundleNameInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetPublishInfoBundleNameInvalidParameter', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetPublishInfoBundleName start====");
        let result = commoneventerror.SetPublishInfoBundleNameInvalidParameter();
        expect(result).assertEqual(InvalidParameter);
        console.info("====>Sub_CommonEvent_SetPublishInfoBundleName result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetPublishInfoBundleName catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_CommonEventSetPublishInfoPermissionSuccess_0700
     * @tc.name    : testNotificationCesCommonEventNoError0700.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetPublishInfoPermissionSuccess', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetPublishInfoPermissionSuccess start====");
        let result: number = commoneventerror.SetPublishInfoPermissionSuccess();
        expect(result).assertEqual(Success);
        console.info("====>Sub_CommonEvent_SetPublishInfoPermissionSuccess result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetPublishInfoPermissionSuccess catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetPublishInfoPermissionError', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetPublishInfoPermissionError start====");
        let result: number = commoneventerror.SetPublishInfoPermissionError();
        expect(result).assertEqual(InvalidParameter);
        console.info("====>Sub_CommonEvent_SetPublishInfoPermissionError result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetPublishInfoPermissionError catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetPublishInfoCodeSuccess', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetPublishInfoCodeSuccess start====");
        let result: number = commoneventerror.SetPublishInfoCodeSuccess();
        expect(result).assertEqual(Success);
        console.info("====>Sub_CommonEvent_SetPublishInfoCodeSuccess result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetPublishInfoCodeSuccess catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetPublishInfoCodeError', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetPublishInfoCodeError start====");
        let result: number = commoneventerror.SetPublishInfoCodeError();
        expect(result).assertEqual(InvalidParameter);
        console.info("====>Sub_CommonEvent_SetPublishInfoCodeError result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetPublishInfoCodeError catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetPublishInfoDataSuccess', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetPublishInfoDataSuccess start====");
        let result: number = commoneventerror.SetPublishInfoDataSuccess();
        expect(result).assertEqual(Success);
        console.info("====>Sub_CommonEvent_SetPublishInfoDataSuccess result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetPublishInfoDataSuccess catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetPublishInfoDataError', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetPublishInfoDataError start====");
        let result: number = commoneventerror.SetPublishInfoDataError();
        expect(result).assertEqual(InvalidParameter);
        console.info("====>Sub_CommonEvent_SetPublishInfoDataError result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetPublishInfoDataError catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetPublishInfoParameterSuccess', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetPublishInfoParameterSuccess start====");
        let result: number = commoneventerror.SetPublishInfoParameterSuccess();
        expect(result).assertEqual(Success);
        console.info("====>Sub_CommonEvent_SetPublishInfoParameterSuccess result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetPublishInfoParameterSuccess catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetPublishInfoParameterError', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetPublishInfoParameterError start====");
        let result: number = commoneventerror.SetPublishInfoParameterError();
        expect(result).assertEqual(InvalidParameter);
        console.info("====>Sub_CommonEvent_SetPublishInfoParameterError result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetPublishInfoParameterError catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetIntToParameterSuccess', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetIntToParameterSuccess start====");
        let result: number = commoneventerror.SetIntToParameterSuccess();
        expect(result).assertEqual(Success);
        console.info("====>Sub_CommonEvent_SetIntToParameterSuccess result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetIntToParameterSuccess catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetIntToParameterError', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetIntToParameterError start====");
        let result: number = commoneventerror.SetIntToParameterError();
        expect(result).assertEqual(InvalidParameter);
        console.info("====>Sub_CommonEvent_SetIntToParameterError result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetIntToParameterError catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetIntArrayToParameterSuccess', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetIntArrayToParameterSuccess start====");
        let result: number = commoneventerror.SetIntArrayToParameterSuccess();
        expect(result).assertEqual(Success);
        console.info("====>Sub_CommonEvent_SetIntArrayToParameterSuccess result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetIntArrayToParameterSuccess catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetIntArrayToParameterError', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetIntArrayToParameterError start====");
        let result: number = commoneventerror.SetIntArrayToParameterError();
        expect(result).assertEqual(InvalidParameter);
        console.info("====>Sub_CommonEvent_SetIntArrayToParameterError result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetIntArrayToParameterError catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetLongToParameterSuccess', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetLongToParameterSuccess start====");
        let result: number = commoneventerror.SetLongToParameterSuccess();
        expect(result).assertEqual(Success);
        console.info("====>Sub_CommonEvent_SetLongToParameterSuccess result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetLongToParameterSuccess catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetLongToParameterError', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetLongToParameterError start====");
        let result: number = commoneventerror.SetLongToParameterError();
        expect(result).assertEqual(InvalidParameter);
        console.info("====>Sub_CommonEvent_SetLongToParameterError result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetLongToParameterError catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetLongArrayToParameterSuccess', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetLongArrayToParameterSuccess start====");
        let result: number = commoneventerror.SetLongArrayToParameterSuccess();
        expect(result).assertEqual(Success);
        console.info("====>Sub_CommonEvent_SetLongArrayToParameterSuccess result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetLongArrayToParameterSuccess catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetLongArrayToParameterError', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetLongArrayToParameterError start====");
        let result: number = commoneventerror.SetLongArrayToParameterError();
        expect(result).assertEqual(InvalidParameter);
        console.info("====>Sub_CommonEvent_SetLongArrayToParameterError result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetLongArrayToParameterError catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetBoolToParameterSuccess', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetBoolToParameterSuccess start====");
        let result: number = commoneventerror.SetBoolToParameterSuccess();
        expect(result).assertEqual(Success);
        console.info("====>Sub_CommonEvent_SetBoolToParameterSuccess result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetBoolToParameterSuccess catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetBoolToParameterError', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetBoolToParameterError start====");
        let result: number = commoneventerror.SetBoolToParameterError();
        expect(result).assertEqual(InvalidParameter);
        console.info("====>Sub_CommonEvent_SetBoolToParameterError result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetBoolToParameterError catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetBoolArrayToParameterSuccess', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetBoolArrayToParameterSuccess start====");
        let result: number = commoneventerror.SetBoolArrayToParameterSuccess();
        expect(result).assertEqual(Success);
        console.info("====>Sub_CommonEvent_SetBoolArrayToParameterSuccess result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetBoolArrayToParameterSuccess catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetBoolArrayToParameterError', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetBoolArrayToParameterError start====");
        let result: number = commoneventerror.SetBoolArrayToParameterError();
        expect(result).assertEqual(InvalidParameter);
        console.info("====>Sub_CommonEvent_SetBoolArrayToParameterError result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetBoolArrayToParameterError catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetCharToParameterSuccess', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetCharToParameterSuccess start====");
        let result: number = commoneventerror.SetCharToParameterSuccess();
        expect(result).assertEqual(Success);
        console.info("====>Sub_CommonEvent_SetCharToParameterSuccess result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetCharToParameterSuccess catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetCharToParameterError', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetCharToParameterError start====");
        let result: number = commoneventerror.SetCharToParameterError();
        expect(result).assertEqual(InvalidParameter);
        console.info("====>Sub_CommonEvent_SetCharToParameterError result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetCharToParameterError catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetCharArrayToParameterSuccess', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetCharArrayToParameterSuccess start====");
        let result: number = commoneventerror.SetCharArrayToParameterSuccess();
        expect(result).assertEqual(Success);
        console.info("====>Sub_CommonEvent_SetCharArrayToParameterSuccess result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetCharArrayToParameterSuccess catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetCharArrayToParameterError', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetCharArrayToParameterError start====");
        let result: number = commoneventerror.SetCharArrayToParameterError();
        expect(result).assertEqual(InvalidParameter);
        console.info("====>Sub_CommonEvent_SetCharArrayToParameterError result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetCharArrayToParameterError catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetDoubleToParameterSuccess', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetDoubleToParameterSuccess start====");
        let result: number = commoneventerror.SetDoubleToParameterSuccess();
        expect(result).assertEqual(Success);
        console.info("====>Sub_CommonEvent_SetDoubleToParameterSuccess result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetDoubleToParameterSuccess catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetDoubleToParameterError', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetDoubleToParameterError start====");
        let result: number = commoneventerror.SetDoubleToParameterError();
        expect(result).assertEqual(InvalidParameter);
        console.info("====>Sub_CommonEvent_SetDoubleToParameterError result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetDoubleToParameterError catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetDoubleArrayToParameterSuccess', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetDoubleArrayToParameterSuccess start====");
        let result: number = commoneventerror.SetDoubleArrayToParameterSuccess();
        expect(result).assertEqual(Success);
        console.info("====>Sub_CommonEvent_SetDoubleArrayToParameterSuccess result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetDoubleArrayToParameterSuccess catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_SetDoubleArrayToParameterError', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_SetDoubleArrayToParameterError start====");
        let result: number = commoneventerror.SetDoubleArrayToParameterError();
        expect(result).assertEqual(InvalidParameter);
        console.info("====>Sub_CommonEvent_SetDoubleArrayToParameterError result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_SetDoubleArrayToParameterError catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_CommonEventPublishSuccess', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_CommonEventPublishSuccess start====");
        let result: number = commoneventerror.PublishSuccess();
        expect(result).assertEqual(Success);
        console.info("====>Sub_CommonEvent_CommonEventPublishSuccess result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_CommonEventPublishSuccess catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_PublishInvalidParameter', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_PublishInvalidParameter start====");
        let result: number = commoneventerror.PublishInvalidParameter();
        expect(result).assertEqual(InvalidParameter);
        console.info("====>Sub_CommonEvent_PublishInvalidParameter result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_PublishInvalidParameter catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_PublishWithInfoSuccess', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_PublishWithInfoSuccess start====");
        let result: number = commoneventerror.PublishWithInfoSuccess();
        expect(result).assertEqual(Success);
        console.info("====>Sub_CommonEvent_PublishWithInfoSuccess result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_PublishWithInfoSuccess catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : Sub_Notification_Ces_CommonEvent_InvalidParameter_0100
     * @tc.name    : testNotificationCesCommonEventNoError0100.
     * @tc.desc    : Test testCommonEventInvalidParameter
     * @tc.level   : Level2
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('Sub_CommonEvent_PublishWithInfoInvalidParameter', Level.LEVEL2, async (done: Function) => {
      try {
        console.info("====>Sub_CommonEvent_PublishWithInfoInvalidParameter start====");
        let result: number = commoneventerror.PublishWithInfoInvalidParameter();
        expect(result).assertEqual(InvalidParameter);
        console.info("====>Sub_CommonEvent_PublishWithInfoInvalidParameter result: " + result);
        done()
      } catch (err) {
        console.error("====>Sub_CommonEvent_PublishWithInfoInvalidParameter catch err: " + err);
        done()
      }
    });
  })
}
