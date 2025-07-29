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
import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect, TestType, Level, Size } from '@ohos/hypium';
import { Driver, ON } from '@ohos.UiTest';
import mechanicManager from '@ohos.distributedHardware.mechanicManager'

export default function MechanicManagerAPI() {
  describe('MechanicManagerAPI', () => {
    let sleep = (ms: number) => {
      return new Promise<string>(resolve => setTimeout(resolve, ms));
    }

    beforeAll(() => {
      console.log(('beforeAll called'));
      await sleep(1000);
      done();
    })
    beforeEach(() => {
      // Presets an action, which is performed before each unit test case starts.
      // The number of execution times is the same as the number of test cases defined by **it**.
      // This API supports only one parameter: preset action function.
    })
    afterEach(() => {
      // Presets a clear action, which is performed after each unit test case ends.
      // The number of execution times is the same as the number of test cases defined by **it**.
      // This API supports only one parameter: clear action function.
    })
    afterAll(() => {
      // Presets a clear action, which is performed after all test case of the test suite end.
      // This API supports only one parameter: clear action function.
    })
    /**
     * @tc.number     : SUB_DH_MechanicManager_on_0100
     * @tc.name       : SUB_DH_MechanicManager_on_0100
     * @tc.desc       : Subscribes to device attachment state change events,mechanicManager.on接口调用
     * @tc.size       : MediumTest
     * @tc.type       : FUNCTION
     * @tc.level      : Level 0
     */
    it('SUB_DH_MechanicManager_on_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0, async () => {
      console.log("----------------SUB_DH_MechanicManager_on_0100 begin----------------");
      try {
        mechanicManager.on("attachStateChange", (data) => {
          console.log("SUB_DH_MechanicManager_on_0100 mechanicManager.on success, data:" + JSON.stringify(data));
        });
        console.log("SUB_DH_MechanicManager_on_0100 mechanicManager.on success");
        expect(true).assertTrue();
      }catch (error) {
        console.error(`SUB_DH_MechanicManager_on_0100 failed, code is ${error.code}, message is ${error.message}`);
        expect(false).assertFail();
      }
      console.log("----------------SUB_DH_MechanicManager_on_0100 end----------------");
    });
    /**
     * @tc.number     : SUB_DH_MechanicManager_on_0200
     * @tc.name       : SUB_DH_MechanicManager_on_0200
     * @tc.desc       : Subscribes to device attachment state change events,mechanicManager.on接口调用callback为null
     * @tc.size       : MediumTest
     * @tc.type       : FUNCTION
     * @tc.level      : Level 3
     */
    it('SUB_DH_MechanicManager_on_0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, async () => {
      console.log("----------------SUB_DH_MechanicManager_on_0200 begin----------------");
      try {
        mechanicManager.on("attachStateChange", null);
        console.log("SUB_DH_MechanicManager_on_0200 mechanicManager.on success" + ".expect fail");
        expect().assertFalse();
      }catch (error) {
        console.error(`SUB_DH_MechanicManager_on_0200 failed, code is ${error.code}, message is ${error.message}`);
        expect(error.code == 401).assertTrue();
      }
      console.log("----------------SUB_DH_MechanicManager_on_0200 end----------------");
    });
    /**
     * @tc.number     : SUB_DH_MechanicManager_on_0300
     * @tc.name       : SUB_DH_MechanicManager_on_0300
     * @tc.desc       : Subscribes to device attachment state change events,mechanicManager.on接口调用callback为underfined
     * @tc.size       : MediumTest
     * @tc.type       : FUNCTION
     * @tc.level      : Level 3
     */
    it('SUB_DH_MechanicManager_on_0300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, async () => {
      console.log("----------------SUB_DH_MechanicManager_on_0300 begin----------------");
      try {
        mechanicManager.on("attachStateChange", underfined);
        console.log("SUB_DH_MechanicManager_on_0300 mechanicManager.on success" + ".expect fail");
        expect().assertFalse();
      }catch (error) {
        console.error(`SUB_DH_MechanicManager_on_0300 failed, code is ${error.code}, message is ${error.message}`);
        expect(error.code == 401).assertTrue();
      }
      console.log("----------------SUB_DH_MechanicManager_on_0300 end----------------");
    });
    /**
     * @tc.number     : SUB_DH_MechanicManager_off_0100
     * @tc.name       : SUB_DH_MechanicManager_off_0100
     * @tc.desc       : Unsubscribes from device attachment state change events,mechanicManager.off接口调用
     * @tc.size       : MediumTest
     * @tc.type       : FUNCTION
     * @tc.level      : Level 0
     */
    it('SUB_DH_MechanicManager_off_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0, async () => {
      console.log("----------------SUB_DH_MechanicManager_off_0100 begin----------------");
      try {
        mechanicManager.off("attachStateChange", (data) => {
          console.log("SUB_DH_MechanicManager_off_0100 mechanicManager.off success, data:" + JSON.stringify(data));
        });
        console.log("SUB_DH_MechanicManager_off_0100 mechanicManager.off success");
        expect(true).assertTrue();
      }catch (error) {
        console.error(`SUB_DH_MechanicManager_off_0100 failed, code is ${error.code}, message is ${error.message}`);
        expect(false).assertFail();
      }
      console.log("----------------SUB_DH_MechanicManager_off_0100 end----------------");
    });
    /**
     * @tc.number     : SUB_DH_MechanicManager_off_0200
     * @tc.name       : SUB_DH_MechanicManager_off_0200
     * @tc.desc       : Unsubscribes from device attachment state change events,mechanicManager.off接口回调为空
     * @tc.size       : MediumTest
     * @tc.type       : FUNCTION
     * @tc.level      : Level 0
     */
    it('SUB_DH_MechanicManager_off_0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0, async () => {
      console.log("----------------SUB_DH_MechanicManager_off_0200 begin----------------");
      try {
        mechanicManager.off("attachStateChange");
        console.log("SUB_DH_MechanicManager_off_0200 mechanicManager.off success");
        expect(false).assertFail();
      }catch (error) {
        console.error(`SUB_DH_MechanicManager_off_0200 failed, code is ${error.code}, message is ${error.message}`);
        expect(error.code == 33300001).assertTrue();
      }
      console.log("----------------SUB_DH_MechanicManager_off_0200 end----------------");
    });
    /**
     * @tc.number     : SUB_DH_MechanicManager_off_0300
     * @tc.name       : SUB_DH_MechanicManager_off_0300
     * @tc.desc       : Unsubscribes from device attachment state change events,mechanicManager.off接口回调callback为null
     * @tc.size       : MediumTest
     * @tc.type       : FUNCTION
     * @tc.level      : Level 3
     */
    it('SUB_DH_MechanicManager_off_0300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, async () => {
      console.log("----------------SUB_DH_MechanicManager_off_0300 begin----------------");
      try {
        mechanicManager.off("attachStateChange", null);
        console.log("SUB_DH_MechanicManager_off_0300 mechanicManager.off success" + ".expect fail");
        expect(false).assertFail();
      }catch (error) {
        console.error(`SUB_DH_MechanicManager_off_0300 failed, code is ${error.code}, message is ${error.message}`);
        expect(error.code == 401).assertTrue();
      }
      console.log("----------------SUB_DH_MechanicManager_off_0300 end----------------");
    });
    /**
     * @tc.number     : SUB_DH_MechanicManager_off_0400
     * @tc.name       : SUB_DH_MechanicManager_off_0400
     * @tc.desc       : Unsubscribes from device attachment state change events,mechanicManager.off接口回调callback为undefined
     * @tc.size       : MediumTest
     * @tc.type       : FUNCTION
     * @tc.level      : Level 3
     */
    it('SUB_DH_MechanicManager_off_0400', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, async () => {
      console.log("----------------SUB_DH_MechanicManager_off_0400 begin----------------");
      try {
        mechanicManager.off("attachStateChange", undefined);
        console.log("SUB_DH_MechanicManager_off_0400 mechanicManager.off success" + ".expect fail");
        expect(false).assertFail();
      }catch (error) {
        console.error(`SUB_DH_MechanicManager_off_0400 failed, code is ${error.code}, message is ${error.message}`);
        expect(error.code == 401).assertTrue();
      }
      console.log("----------------SUB_DH_MechanicManager_off_0400 end----------------");
    });
    /**
     * @tc.number     : SUB_DH_MechanicManager_getAttachedMechDevices_0100
     * @tc.name       : SUB_DH_MechanicManager_getAttachedMechDevices_0100
     * @tc.desc       : Obtain the list of connected mechanical devices,getAttachedMechDevices接口调用
     * @tc.size       : MediumTest
     * @tc.type       : FUNCTION
     * @tc.level      : Level 0
     */
    it('SUB_DH_MechanicManager_getAttachedMechDevices_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0, async () => {
      console.log("----------------SUB_DH_MechanicManager_getAttachedMechDevices_0100 begin----------------");
      try {
        let mechInfos: mechanicManager.MechInfo[] = mechanicManager.getAttachedMechDevices();
        console.log("SUB_DH_MechanicManager_getAttachedMechDevices_0100 设备列表：" + JSON.stringify(mechInfos));
        expect(false).assertFail();
      }catch (error) {
        console.error(`SUB_DH_MechanicManager_getAttachedMechDevices_0100 查询失败, code is ${error.code}, message is ${error.message}`);
        expect(error.code == 33300001).assertTrue();
      }
      console.log("----------------SUB_DH_MechanicManager_getAttachedMechDevices_0100 end----------------");
    });
    /**
     * @tc.number     : SUB_DH_MechanicManager_setCameraTrackingEnabled_0100
     * @tc.name       : SUB_DH_MechanicManager_setCameraTrackingEnabled_0100
     * @tc.desc       : Enables or disables camera tracking,启用当前机械体设备摄像头跟踪
     * @tc.size       : MediumTest
     * @tc.type       : FUNCTION
     * @tc.level      : Level 0
     */
    it('SUB_DH_MechanicManager_setCameraTrackingEnabled_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0, async () => {
      console.log("----------------SUB_DH_MechanicManager_setCameraTrackingEnabled_0100 begin----------------");
      try {
        mechanicManager.setCameraTrackingEnabled(true);
        console.log("SUB_DH_MechanicManager_setCameraTrackingEnabled_0100 Enable Tracking Success");
        expect(false).assertFail();
      }catch (error) {
        console.error(`SUB_DH_MechanicManager_setCameraTrackingEnabled_0100 failed, code is ${error.code}, message is ${error.message}`);
        expect(error.code == 33300001).assertTrue();
      }
      console.log("----------------SUB_DH_MechanicManager_setCameraTrackingEnabled_0100 end----------------");
    });
    /**
     * @tc.number     : SUB_DH_MechanicManager_setCameraTrackingEnabled_0200
     * @tc.name       : SUB_DH_MechanicManager_setCameraTrackingEnabled_0200
     * @tc.desc       : Enables or disables camera tracking,禁用当前机械体设备摄像头跟踪
     * @tc.size       : MediumTest
     * @tc.type       : FUNCTION
     * @tc.level      : Level 0
     */
    it('SUB_DH_MechanicManager_setCameraTrackingEnabled_0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0, async () => {
      console.log("----------------SUB_DH_MechanicManager_setCameraTrackingEnabled_0200 begin----------------");
      try {
        mechanicManager.setCameraTrackingEnabled(false);
        console.log("SUB_DH_MechanicManager_setCameraTrackingEnabled_0200 disables Tracking Success");
        expect(false).assertFail();
      }catch (error) {
        console.error(`SUB_DH_MechanicManager_setCameraTrackingEnabled_0200 failed, code is ${error.code}, message is ${error.message}`);
        expect(error.code == 33300001).assertTrue();
      }
      console.log("----------------SUB_DH_MechanicManager_setCameraTrackingEnabled_0200 end----------------");
    });
    /**
     * @tc.number     : SUB_DH_MechanicManager_setCameraTrackingEnabled_0300
     * @tc.name       : SUB_DH_MechanicManager_setCameraTrackingEnabled_0300
     * @tc.desc       : Enables or disables camera tracking,setCameraTrackingEnabled参数为null
     * @tc.size       : MediumTest
     * @tc.type       : FUNCTION
     * @tc.level      : Level 3
     */
    it('SUB_DH_MechanicManager_setCameraTrackingEnabled_0300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, async () => {
      console.log("----------------SUB_DH_MechanicManager_setCameraTrackingEnabled_0300 begin----------------");
      try {
        mechanicManager.setCameraTrackingEnabled(null);
        console.log("SUB_DH_MechanicManager_setCameraTrackingEnabled_0300 disables Tracking Success" + ".expect fail" );
        expect(false).assertFail();
      }catch (error) {
        console.error(`SUB_DH_MechanicManager_setCameraTrackingEnabled_0300 failed, code is ${error.code}, message is ${error.message}`);
        expect(error.code == 401).assertTrue();
      }
      console.log("----------------SUB_DH_MechanicManager_setCameraTrackingEnabled_0300 end----------------");
    });
    /**
     * @tc.number     : SUB_DH_MechanicManager_setCameraTrackingEnabled_0400
     * @tc.name       : SUB_DH_MechanicManager_setCameraTrackingEnabled_0400
     * @tc.desc       : Enables or disables camera tracking,setCameraTrackingEnabled参数为undefined
     * @tc.size       : MediumTest
     * @tc.type       : FUNCTION
     * @tc.level      : Level 3
     */
    it('SUB_DH_MechanicManager_setCameraTrackingEnabled_0400', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, async () => {
      console.log("----------------SUB_DH_MechanicManager_setCameraTrackingEnabled_0400 begin----------------");
      try {
        mechanicManager.setCameraTrackingEnabled(undefined);
        console.log("SUB_DH_MechanicManager_setCameraTrackingEnabled_0400 disables Tracking Success" + ".expect fail");
        expect(false).assertFail();
      }catch (error) {
        console.error(`SUB_DH_MechanicManager_setCameraTrackingEnabled_0400 failed, code is ${error.code}, message is ${error.message}`);
        expect(error.code == 401).assertTrue();
      }
      console.log("----------------SUB_DH_MechanicManager_setCameraTrackingEnabled_0400 end----------------");
    });
    /**
     * @tc.number     : SUB_DH_MechanicManager_getCameraTrackingEnabled_0100
     * @tc.name       : SUB_DH_MechanicManager_getCameraTrackingEnabled_0100
     * @tc.desc       : Checks whether camera tracking is enabled for this mechanical device,getCameraTrackingEnabled接口调用
     * @tc.size       : MediumTest
     * @tc.type       : FUNCTION
     * @tc.level      : Level 0
     */
    it('SUB_DH_MechanicManager_getCameraTrackingEnabled_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0, async () => {
      console.log("----------------SUB_DH_MechanicManager_getCameraTrackingEnabled_0100 begin----------------");
      try {
        let enabled = mechanicManager.getCameraTrackingEnabled();
        console.log(`'SUB_DH_MechanicManager_getCameraTrackingEnabled_0100 current tracking status:' ${enabled}`);
        expect(false).assertFail();
      }catch (error) {
        console.error(`SUB_DH_MechanicManager_getCameraTrackingEnabled_0100 failed, code is ${error.code}, message is ${error.message}`);
        expect(error.code == 33300001).assertTrue();
      }
      console.log("----------------SUB_DH_MechanicManager_getCameraTrackingEnabled_0100 end----------------");
    });
    /**
     * @tc.number     : SUB_DH_MechanicManager_on_0500
     * @tc.name       : SUB_DH_MechanicManager_on_0500
     * @tc.desc       : Subscribes to tracking events,mechanicManager.on接口回调callback为null
     * @tc.size       : MediumTest
     * @tc.type       : FUNCTION
     * @tc.level      : Level 3
     */
    it('SUB_DH_MechanicManager_on_0500', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, async () => {
      console.log("----------------SUB_DH_MechanicManager_on_0500 begin----------------");
      try {
        mechanicManager.on("trackingStateChange", null);
        console.log("SUB_DH_MechanicManager_on_0500 mechanicManager.on success" + ".expect fail");
        expect().assertFalse();
      }catch (error) {
        console.error(`SUB_DH_MechanicManager_on_0500 failed, code is ${error.code}, message is ${error.message}`);
        expect(error.code == 401).assertTrue();
      }
      console.log("----------------SUB_DH_MechanicManager_on_0500 end----------------");
    });
    /**
     * @tc.number     : SUB_DH_MechanicManager_on_0600
     * @tc.name       : SUB_DH_MechanicManager_on_0600
     * @tc.desc       : Subscribes to tracking events,mechanicManager.on接口回调callback为undefined
     * @tc.size       : MediumTest
     * @tc.type       : FUNCTION
     * @tc.level      : Level 3
     */
    it('SUB_DH_MechanicManager_on_0600', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, async () => {
      console.log("----------------SUB_DH_MechanicManager_on_0600 begin----------------");
      try {
        mechanicManager.on("trackingStateChange", undefined);
        console.log("SUB_DH_MechanicManager_on_0600 mechanicManager.on success" + ".expect fail");
        expect().assertFalse();
      }catch (error) {
        console.error(`SUB_DH_MechanicManager_on_0600 failed, code is ${error.code}, message is ${error.message}`);
        expect(error.code == 401).assertTrue();
      }
      console.log("----------------SUB_DH_MechanicManager_on_0600 end----------------");
    });
    /**
     * @tc.number     : SUB_DH_MechanicManager_off_0700
     * @tc.name       : SUB_DH_MechanicManager_off_0700
     * @tc.desc       : Unsubscribes from tracking events,mechanicManager.off接口回调callback为null
     * @tc.size       : MediumTest
     * @tc.type       : FUNCTION
     * @tc.level      : Level 3
     */
    it('SUB_DH_MechanicManager_off_0700', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, async () => {
      console.log("----------------SUB_DH_MechanicManager_off_0700 begin----------------");
      try {
        mechanicManager.on("trackingStateChange", null);
        console.log("SUB_DH_MechanicManager_off_0700 mechanicManager.on success" + ".expect fail");
        expect().assertFalse();
      }catch (error) {
        console.error(`SUB_DH_MechanicManager_off_0700 failed, code is ${error.code}, message is ${error.message}`);
        expect(error.code == 401).assertTrue();
      }
      console.log("----------------SUB_DH_MechanicManager_off_0700 end----------------");
    });
    /**
     * @tc.number     : SUB_DH_MechanicManager_off_0800
     * @tc.name       : SUB_DH_MechanicManager_off_0800
     * @tc.desc       : Unsubscribes from tracking events,mechanicManager.off接口回调callback为undefined
     * @tc.size       : MediumTest
     * @tc.type       : FUNCTION
     * @tc.level      : Level 3
     */
    it('SUB_DH_MechanicManager_off_0800', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, async () => {
      console.log("----------------SUB_DH_MechanicManager_off_0800 begin----------------");
      try {
        mechanicManager.on("trackingStateChange", undefined);
        console.log("SUB_DH_MechanicManager_off_0800 mechanicManager.on success" + ".expect fail");
        expect().assertFalse();
      }catch (error) {
        console.error(`SUB_DH_MechanicManager_off_0800 failed, code is ${error.code}, message is ${error.message}`);
        expect(error.code == 401).assertTrue();
      }
      console.log("----------------SUB_DH_MechanicManager_off_0800 end----------------");
    });
    /**
     * @tc.number     : SUB_DH_MechanicManager_getCameraTrackingLayout_0100
     * @tc.name       : SUB_DH_MechanicManager_getCameraTrackingLayout_0100
     * @tc.desc       : Obtains the camera tracking layout of this mechanical device,getCameraTrackingLayout接口调用
     * @tc.size       : MediumTest
     * @tc.type       : FUNCTION
     * @tc.level      : Level 0
     */
    it('SUB_DH_MechanicManager_getCameraTrackingLayout_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0, async () => {
      console.log("----------------SUB_DH_MechanicManager_getCameraTrackingLayout_0100 begin----------------");
      try {
        let layout = mechanicManager.getCameraTrackingLayout();
        console.log(`'SUB_DH_MechanicManager_getCameraTrackingLayout_0100 Query layout successful:' ${layout}`);
        expect(false).assertFail();
      }catch (error) {
        console.error(`SUB_DH_MechanicManager_getCameraTrackingLayout_0100 failed, code is ${error.code}, message is ${error.message}`);
        expect(error.code == 33300001).assertTrue();
      }
      console.log("----------------SUB_DH_MechanicManager_getCameraTrackingLayout_0100 end----------------");
    });
    /**
     * @tc.number     : SUB_DH_MechanicManager_MechDeviceType_0100
     * @tc.name       : SUB_DH_MechanicManager_MechDeviceType_0100
     * @tc.desc       : Enumerates the mechanical device types,机械体设备类型的枚举
     * @tc.size       : MediumTest
     * @tc.type       : FUNCTION
     * @tc.level      : Level 2
     */
    it('SUB_DH_MechanicManager_MechDeviceType_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async () => {
      console.log("----------------SUB_DH_MechanicManager_MechDeviceType_0100 begin----------------");
      try {
        let GIMBAL_DEVICE = mechanicManager.MechDeviceType.GIMBAL_DEVICE;
        console.log(`'SUB_DH_MechanicManager_MechDeviceType_0100 GIMBAL_DEVICE:' ${GIMBAL_DEVICE}`);
        expect(GIMBAL_DEVICE).assertEqual(0);
      }catch (error) {
        console.error(`SUB_DH_MechanicManager_MechDeviceType_0100 failed, code is ${error.code}, message is ${error.message}`);
        expect().assertFalse();
      }
      console.log("----------------SUB_DH_MechanicManager_MechDeviceType_0100 end----------------");
    });
    /**
     * @tc.number     : SUB_DH_MechanicManager_AttachState_0100
     * @tc.name       : SUB_DH_MechanicManager_AttachState_0100
     * @tc.desc       : Device attach states,设备连接状态的枚举
     * @tc.size       : MediumTest
     * @tc.type       : FUNCTION
     * @tc.level      : Level 2
     */
    it('SUB_DH_MechanicManager_AttachState_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async () => {
      console.log("----------------SUB_DH_MechanicManager_AttachState_0100 begin----------------");
      try {
        let ATTACHED = mechanicManager.AttachState.ATTACHED;
        let DETACHED = mechanicManager.AttachState.DETACHED;
        console.log(`'SUB_DH_MechanicManager_AttachState_0100 ATTACHED:' ${ATTACHED}`);
        console.log(`'SUB_DH_MechanicManager_AttachState_0100 DETACHED:' ${DETACHED}`);
        expect(ATTACHED).assertEqual(0);
        expect(DETACHED).assertEqual(1);
      }catch (error) {
        console.error(`SUB_DH_MechanicManager_AttachState_0100 failed, code is ${error.code}, message is ${error.message}`);
        expect().assertFalse();
      }
      console.log("----------------SUB_DH_MechanicManager_AttachState_0100 end----------------");
    });
    /**
     * @tc.number     : SUB_DH_MechanicManager_CameraTrackingLayout_0100
     * @tc.name       : SUB_DH_MechanicManager_CameraTrackingLayout_0100
     * @tc.desc       : Enumerates the camera tracking layouts,摄像头跟踪布局的枚举
     * @tc.size       : MediumTest
     * @tc.type       : FUNCTION
     * @tc.level      : Level 2
     */
    it('SUB_DH_MechanicManager_CameraTrackingLayout_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async () => {
      console.log("----------------SUB_DH_MechanicManager_CameraTrackingLayout_0100 begin----------------");
      try {
        let DEFAULT = mechanicManager.CameraTrackingLayout.DEFAULT;
        let LEFT = mechanicManager.CameraTrackingLayout.LEFT;
        let MIDDLE = mechanicManager.CameraTrackingLayout.MIDDLE;
        let RIGHT = mechanicManager.CameraTrackingLayout.RIGHT;
        console.log(`'SUB_DH_MechanicManager_CameraTrackingLayout_0100 DEFAULT:' ${DEFAULT}`);
        console.log(`'SUB_DH_MechanicManager_CameraTrackingLayout_0100 LEFT:' ${LEFT}`);
        console.log(`'SUB_DH_MechanicManager_CameraTrackingLayout_0100 MIDDLE:' ${MIDDLE}`);
        console.log(`'SUB_DH_MechanicManager_CameraTrackingLayout_0100 RIGHT:' ${RIGHT}`);
        expect(DEFAULT).assertEqual(0);
        expect(LEFT).assertEqual(1);
        expect(MIDDLE).assertEqual(2);
        expect(RIGHT).assertEqual(3);
      }catch (error) {
        console.error(`SUB_DH_MechanicManager_CameraTrackingLayout_0100 failed, code is ${error.code}, message is ${error.message}`);
        expect().assertFalse();
      }
      console.log("----------------SUB_DH_MechanicManager_CameraTrackingLayout_0100 end----------------");
    });
  })
}