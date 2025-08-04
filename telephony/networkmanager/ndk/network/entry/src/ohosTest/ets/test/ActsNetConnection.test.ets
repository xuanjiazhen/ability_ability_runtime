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

import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect, TestType, Size, Level } from '@ohos/hypium'
import NetConnectionTest from 'libnetconnectiontest.so'

const OH_HTTP_OK = 0;
const NETCONN_PERMISSION_DENIED = 201;
const OH_HTTP_PARAMETER_ERROR = 401;
const OH_HTTP_OUT_OF_MEMORY = 2300027;
const NETCONN_INTERNAL_ERROR = 2100003;
const CASE_INDEX_1 = 1;
const CASE_INDEX_2 = 2;
const CASE_INDEX_3 = 3;
const CASE_INDEX_4 = 4;
const CASE_INDEX_5 = 4;
const CASE_INDEX_6 = 6;
const CASE_INDEX_7 = 7;
const CASE_INDEX_8 = 8;

export default function ActsNetConnectionTest() {
  describe('ActsNetConnectionTest', () => {

  /**
   * @tc.number  : SUB_NetworkManager_Net_HasDefaultNet_NoP_0100
   * @tc.name    : testSUBNetworkManagerNetHasDefaultNetNoP0100
   * @tc.desc    : Test whether the right of interface is denied.
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testSUBNetworkManagerNetHasDefaultNetNoP0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testSUBNetworkManagerNetHasDefaultNetNoP0100';
    try {
      console.info(`${caseName} ---start`);
      let result : number = NetConnectionTest.NetConn_HasDefaultNet();
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(NETCONN_PERMISSION_DENIED);
      done();
    } catch (err) {
      console.error(`${caseName} cache error is: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /**
   * @tc.number  : SUB_NetworkManager_Net_GetDefaultNet_NoP_0100
   * @tc.name    : testNetworkManagerNetGetDefaultNetNoP0100
   * @tc.desc    : Get the default network connection.
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerNetGetDefaultNetNoP0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerNetGetDefaultNetNoP0100';
    try {
      console.info(`${caseName} ---start`);
      let result : number = NetConnectionTest.NetConn_GetDefaultNet();
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(NETCONN_PERMISSION_DENIED);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /**
   * @tc.number  : SUB_NetworkManager_Net_IsDefaultNetMetered_NoP_0100
   * @tc.name    : testNetworkManagerNetIsDefaultNetMeteredNoP0100
   * @tc.desc    : Check if the default network connection is metered.
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerNetIsDefaultNetMeteredNoP0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerNetIsDefaultNetMeteredNoP0100';
    try {
      console.info(`${caseName} ---start`);
      let result : number = NetConnectionTest.NetConn_IsDefaultNetMetered();
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(NETCONN_PERMISSION_DENIED);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /**
   * @tc.number  : SUB_NetworkManager_Net_GetConnectionProperties_NoP_0100
   * @tc.name    : testNetworkManagerNetGetConnectionPropertiesNoP0100
   * @tc.desc    : Get the connection properties of the default network.
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerNetGetConnectionPropertiesNoP0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerNetGetConnectionPropertiesNoP0100';
    try {
      console.info(`${caseName} ---start`);
      let result : number = NetConnectionTest.NetConn_GetConnectionProperties();
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(NETCONN_PERMISSION_DENIED);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /**
   * @tc.number  : SUB_NetworkManager_Net_GetNetCapabilities_NoP_0100
   * @tc.name    : testNetworkManagerNetGetNetCapabilitiesNoP0100
   * @tc.desc    : Get the network capabilities of the default network.
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerNetGetNetCapabilitiesNoP0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerNetGetNetCapabilitiesNoP0100';
    try {
      console.info(`${caseName} ---start`);
      let result : number = NetConnectionTest.NetConn_GetNetCapabilities();
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(NETCONN_PERMISSION_DENIED);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /**
   * @tc.number  : SUB_NetworkManager_Net_GetAddrInfo_NoP_0100
   * @tc.name    : testNetworkManagerNetGetAddrInfoNoP0100
   * @tc.desc    : Get the address information of the network.
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerNetGetAddrInfoNoP0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerNetGetAddrInfoNoP0100';
    let host = 'www.baidu.com';
    let serv = 'https';
    try {
      console.info(`${caseName} ---start`);
      let result : number = NetConnectionTest.NetConn_GetAddrInfo(host, serv);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(NETCONN_PERMISSION_DENIED);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /**
   * @tc.number  : SUB_NetworkManager_Net_GetAllNets_NoP_0100
   * @tc.name    : testNetworkManagerNetGetAllNetsNoP0100
   * @tc.desc    : Get all available network connections.
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerNetGetAllNetsNoP0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerNetGetAllNetsNoP0100';
    try {
      console.info(`${caseName} ---start`);
      let result : number = NetConnectionTest.NetConn_GetAllNets();
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(NETCONN_PERMISSION_DENIED);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /**
   * @tc.number  : SUB_NetworkManager_Net_SetPacUrl_NoP_0100
   * @tc.name    : testNetworkManagerNetSetPacUrlNoP0100
   * @tc.desc    : Set the PAC URL of the network.
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerNetSetPacUrlNoP0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerNetSetPacUrlNoP0100';
    try {
      console.info(`${caseName} ---start`);
      let result : number = NetConnectionTest.NetConn_SetPacUrl("test");
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(NETCONN_PERMISSION_DENIED);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })
  
      /**
   * @tc.number  : SUB_NetworkManager_Net_SetPacUrl_NoP_0200
   * @tc.name    : testNetworkManagerNetSetPacUrlNoP0200
   * @tc.desc    : Set the PAC URL of the network.
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerNetSetPacUrlNoP0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerNetSetPacUrlNoP0200';
    try {
      console.info(`${caseName} ---start`);
      let result : number = NetConnectionTest.NetConn_SetPacUrl("");
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(NETCONN_PERMISSION_DENIED);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

      /**
   * @tc.number  : SUB_NetworkManager_Net_GetPacUrl_NoP_0100
   * @tc.name    : testNetworkManagerNetGetPacUrlNoP0100
   * @tc.desc    : Get the PAC URL of the network.
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerNetGetPacUrlNoP0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerNetGetPacUrlNoP0100';
    try {
      console.info(`${caseName} ---start`);
      let result : number = NetConnectionTest.NetConn_GetPacUrl(CASE_INDEX_1);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(NETCONN_INTERNAL_ERROR);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

    /**
   * @tc.number  : SUB_NetworkManager_Net_GetPacUrl_NoP_0200
   * @tc.name    : testNetworkManagerNetGetPacUrlNoP0200
   * @tc.desc    : Get the PAC URL of the network.
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerNetGetPacUrlNoP0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerNetGetPacUrlNoP0200';
    try {
      console.info(`${caseName} ---start`);
      let result : number = NetConnectionTest.NetConn_GetPacUrl(0);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_PARAMETER_ERROR);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /**
   * @tc.number  : SUB_NetworkManager_Net_Capabilities_0100
   * @tc.name    : testNetworkManagerNetCapabilities0100
   * @tc.desc    : Test NetCapabilities netCaps interface.
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerNetCapabilities0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerNetCapabilities0100';
    try {
      console.info(`${caseName} ---start`);
      let result : number = NetConnectionTest.NetConn_NetCapabilities_netCaps(CASE_INDEX_1);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_OK);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })
    /**
   * @tc.number  : SUB_NetworkManager_Net_Capabilities_0200
   * @tc.name    : testNetworkManagerNetCapabilities0200
   * @tc.desc    : Test NetCapabilities netCaps interface.
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerNetCapabilities0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerNetCapabilities0200';
    try {
      console.info(`${caseName} ---start`);
      let result : number = NetConnectionTest.NetConn_NetCapabilities_netCaps(CASE_INDEX_2);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_OK);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

      /**
   * @tc.number  : SUB_NetworkManager_Net_Capabilities_0300
   * @tc.name    : testNetworkManagerNetCapabilities0300
   * @tc.desc    : Test NetCapabilities netCaps interface.
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerNetCapabilities0300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerNetCapabilities0300';
    try {
      console.info(`${caseName} ---start`);
      let result : number = NetConnectionTest.NetConn_NetCapabilities_netCaps(CASE_INDEX_3);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_OK);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /**
   * @tc.number  : SUB_NetworkManager_Net_Capabilities_0400
   * @tc.name    : testNetworkManagerNetCapabilities0400
   * @tc.desc    : Test NetCapabilities netCaps interface.
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerNetCapabilities0400', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerNetCapabilities0400';
    try {
      console.info(`${caseName} ---start`);
      let result : number = NetConnectionTest.NetConn_NetCapabilities_netCaps(CASE_INDEX_4);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_OK);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /**
   * @tc.number  : SUB_NetworkManager_Net_Capabilities_0500
   * @tc.name    : testNetworkManagerNetCapabilities0500
   * @tc.desc    : Test NetCapabilities netCaps interface.
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerNetCapabilities0500', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerNetCapabilities0500';
    try {
      console.info(`${caseName} ---start`);
      let result : number = NetConnectionTest.NetConn_NetCapabilities_netCaps(CASE_INDEX_5);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_OK);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

  /**
   * @tc.number  : SUB_NetworkManager_Net_Capabilities_0600
   * @tc.name    : testNetworkManagerNetCapabilities0600
   * @tc.desc    : Test NetCapabilities netCaps interface.
   * @tc.level   : Level2
   * @tc.size    : SmallTest
   * @tc.type    : Function
   */
  it('testNetworkManagerNetCapabilities0600', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: Function) => {
    let caseName: string = 'testNetworkManagerNetCapabilities0600';
    try {
      console.info(`${caseName} ---start`);
      let result : number = NetConnectionTest.NetConn_NetCapabilities_netCaps(CASE_INDEX_6);
      console.info(`${caseName} result is: ` + JSON.stringify(result));
      expect(result).assertEqual(OH_HTTP_OK);
      done();
    } catch (err) {
      console.error(`${caseName} catch err: ` + JSON.stringify(err));
      expect(false).assertEqual(true);
      done();
    }
  })

    /**
     * @tc.number  : SUB_NetworkManager_Net_Capabilities_0700
     * @tc.name    : testNetworkManagerNetCapabilities0700
     * @tc.desc    : Test NetCapabilities netCaps interface.
     * @tc.level   : Level2
     * @tc.size    : SmallTest
     * @tc.type    : Function
     */
    it('testNetworkManagerNetCapabilities0700', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL2, async (done: Function) => {
      let caseName: string = 'testNetworkManagerNetCapabilities0700';
      try {
        console.info(`${caseName} ---start`);
        let result : number = NetConnectionTest.NetConn_NetCapabilities_netCaps(CASE_INDEX_7);
        console.info(`${caseName} result is: ` + JSON.stringify(result));
        expect(result).assertEqual(OH_HTTP_OK);
        done();
      } catch (err) {
        console.error(`${caseName} catch err: ` + JSON.stringify(err));
        expect(false).assertEqual(true);
        done();
      }
    })
  })
}
