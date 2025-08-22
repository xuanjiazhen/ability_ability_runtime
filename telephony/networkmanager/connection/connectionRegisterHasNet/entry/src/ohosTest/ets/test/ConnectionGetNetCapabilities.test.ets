
/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License')
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http:www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import connection from '@ohos.net.connection';
import { describe, it, expect , Level, Size, TestType} from '@ohos/hypium';
import { BusinessError } from '@ohos.base';

const ExpectFail = () => {
  try {
    expect().assertFail();
  } catch (err) {
    console.info(`exportFail failed err: ${JSON.stringify(err)}`);
  }
}

const ExpectTrue = (n: boolean) => {
  try {
    expect(n).assertTrue();
  } catch (err) {
    console.info(`exportTrue failed err: ${JSON.stringify(err)}`);
  }
}
export default function ConnectionGetNetCapabilitiesTest() {
  describe("ConnectionGetNetCapabilitiesTest", () => {

    /**
     * @tc.number: SUB_NetManager_Connection_IsExcludedRoute_0100
     * @tc.name:  testNetManagerConnectionIsExcludedRoute0100
     * @tc.desc: getConnectionProperties  detection
     * @tc.level:Level 3
     * @tc.type:Function
     * @tc.size:MediumTest
     */
    it('testNetManagerConnectionIsExcludedRoute0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, async (done: Function) => {
      let caseName: string = "testNetManagerConnectionIsExcludedRoute0100";
      console.info(`${caseName}---start`);
      try {
        let netHandle = connection.getDefaultNetSync();
        connection.getConnectionProperties(netHandle, (error: BusinessError, data: connection.ConnectionProperties) => {
          if (error) {
            console.log(`${caseName}  get data fail: `+ JSON.stringify(error));
            ExpectFail();
            console.info(`${caseName}---End`);
            done();
          } else {
            console.log(`${caseName}  get value success: ${JSON.stringify(data)}`);
            ExpectTrue(data.routes[0].isExcludedRoute == undefined);
            console.info(`${caseName}---End`);
            done();
          }
        });
      } catch (error) {
        console.info(`${caseName} catch err ->${JSON.stringify(error)}`);
        ExpectFail();
        console.info(`${caseName}---End`);
        done();
      }
    });


    /**
     * @tc.number: SUB_NetManager_Connection_GetConnectionProperties_CallBack_0100
     * @tc.name:  testNetManagerConnectionGetConnectionPropertiesCallBack0100
     * @tc.desc: getConnectionProperties  detection
     * @tc.level:Level 3
     * @tc.type:Function
     * @tc.size:MediumTest
     */
    it('testNetManagerConnectionGetConnectionPropertiesCallBack0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, (done: Function) => {
      let caseName: string = "testNetManagerConnectionGetConnectionPropertiesCallBack0100";
      console.info(`${caseName}---start`);
      try {
        connection.getAllNets((error: BusinessError, value: connection.NetHandle[]) => {
          if (error) {
            console.log(`${caseName}  get data fail: `+ JSON.stringify(error));
            ExpectFail();
            console.info(`${caseName}---End`);
            done();
          } else {
            console.log(`${caseName} get value is : ${JSON.stringify(value)}`);
            connection.getConnectionProperties(value[0], (error: BusinessError, data: connection.ConnectionProperties) => {
              if (error) {
                console.log(`${caseName}  get data fail: `+ JSON.stringify(error));
                ExpectFail();
                console.info(`${caseName}---End`);
                done();
              } else {
                console.log(`${caseName}  get value success: ${JSON.stringify(data)}`);
                if (data.linkAddresses.length > 0) {
                  ExpectTrue(data.linkAddresses[0].address != undefined);
                }
                console.info(`${caseName}---End`);
                done();
              }
            });
          }
        });
      } catch (error) {
        console.info(`${caseName} catch err ->${JSON.stringify(error)}`);
        ExpectFail();
        console.info(`${caseName}---End`);
        done();
      }
    });

    /**
     * @tc.number: SUB_NetManager_Connection_GetNetCapabilities_0100
     * @tc.name:  testNetManagerConnectionGetNetCapabilities0100
     * @tc.desc: GetNetCapabilities  detection
     * @tc.level:Level 3
     * @tc.type:Function
     * @tc.size:MediumTest
     */
    it('testNetManagerConnectionGetNetCapabilities0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, (done: Function) => {
      let caseName: string = "testNetManagerConnectionGetNetCapabilities0100";
      console.info(`${caseName}---start`);
      try {
        connection.getAllNets((error: BusinessError, value: connection.NetHandle[]) => {
          if (error) {
            console.log(`${caseName}  get data fail: `+ JSON.stringify(error));
            ExpectFail();
            console.info(`${caseName}---End`);
            done();
          } else {
            console.log(`Telephony_connection getAllNets netID value : ${JSON.stringify(value)}`);
            connection.getNetCapabilities(value[0],(error: BusinessError, data: connection.NetCapabilities) => {
              if (error) {
                console.log(`${caseName}  get data fail: `+ JSON.stringify(error));
                ExpectFail();
                console.info(`${caseName}---End`);
                done();
              } else {
                console.log(`${caseName}  get value success: ${JSON.stringify(data)}`);
                if(data.networkCap){
                  ExpectTrue(data.networkCap.includes(connection.NetCap.NET_CAPABILITY_INTERNET));
                }
                done();
                console.info(`${caseName}---End`);
              }
            });
          }
        });
      } catch (error) {
        console.info(`${caseName} catch err ->${JSON.stringify(error)}`);
        ExpectFail();
        console.info(`${caseName}---End`);
        done();
      }
    });

    /**
     * @tc.number: SUB_NetManager_Connection_GetNetCapabilities_0200
     * @tc.name:  testNetManagerConnectionGetNetCapabilities0200
     * @tc.desc: getNetCapabilities  detection
     * @tc.level:Level 3
     * @tc.type:Function
     * @tc.size:MediumTest
     */
    it('testNetManagerConnectionGetNetCapabilities0200', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, (done: Function) => {
      let caseName: string = "testNetManagerConnectionGetNetCapabilities0200";
      console.info(`${caseName}---start`);
      try {
        connection.getAllNets((error: BusinessError, value: connection.NetHandle[]) => {
          if (error) {
            console.log(`${caseName}  get data fail: `+ JSON.stringify(error));
            ExpectFail();
            console.info(`${caseName}---End`);
            done();
          } else {
            console.log(`Telephony_connection getAllNets netID value : `+JSON.stringify(value));
            connection.getNetCapabilities(value[0], (error, data) => {
              if (error) {
                console.log(`${caseName}  get data fail: `+ JSON.stringify(error));
                ExpectFail();
                console.info(`${caseName}---End`);
                done();
              } else {
                console.log(`${caseName}  get value success: ${JSON.stringify(data.bearerTypes)}`);
                ExpectTrue( data.bearerTypes.includes(connection.NetBearType.BEARER_WIFI));
                done();
              }
            });
          }
        });
      } catch (error) {
        console.info(`${caseName} catch err ->${JSON.stringify(error)}`);
        ExpectFail();
        console.info(`${caseName}---End`);
        done();
      }

    });

    /**
     * @tc.number: SUB_NetManager_Connection_GetNetCapabilities_0300
     * @tc.name:  testNetManagerConnectionGetNetCapabilities0300
     * @tc.desc: getNetCapabilities  detection
     * @tc.level:Level 3
     * @tc.type:Function
     * @tc.size:MediumTest
     */
    it('testNetManagerConnectionGetNetCapabilities0300', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, (done: Function) => {
      let caseName: string = "testNetManagerConnectionGetNetCapabilities0300";
      console.info(`${caseName}---start`);
      try {
        connection.getAllNets().then((netHandle: connection.NetHandle[]) => {
          connection.getNetCapabilities(netHandle[0]).then((data: connection.NetCapabilities) => {
            console.log(`${caseName} getNetCapabilities promiss data suceess: ${JSON.stringify(data)}`);
            ExpectTrue(data.bearerTypes.includes(connection.NetBearType.BEARER_WIFI));
            console.info(`${caseName}---End`);
            done();
          }).catch((error:BusinessError) => {
            console.log(`${caseName} getNetCapabilities promiss fail: `+JSON.stringify(error));
            ExpectFail();
            console.info(`${caseName}---End`);
            done();
          });
        }).catch((error:BusinessError) => {
          console.log(`${caseName} getAllNets promiss error: `+ JSON.stringify(error));
          ExpectFail();
          console.info(`${caseName}---End`);
          done();
        });
      } catch (error) {
        console.info(`${caseName} catch err ->${JSON.stringify(error)}`);
        ExpectFail();
        console.info(`${caseName}---End`);
        done();
      }
    });

    /**
     * @tc.number: SUB_NetManager_Connection_GetNetCapabilities_0400
     * @tc.name:  testNetManagerConnectionGetNetCapabilities0400
     * @tc.desc: getNetCapabilities  detection
     * @tc.level:Level 3
     * @tc.type:Function
     * @tc.size:MediumTest
     */
    it('testNetManagerConnectionGetNetCapabilities0400', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, (done: Function) => {
      let caseName: string = "testNetManagerConnectionGetNetCapabilities0400";
      console.info(`${caseName}---start`);
      try {
        connection.getAllNets().then((netHandle: connection.NetHandle[]) => {
          connection.getNetCapabilities(netHandle[0]).then((data: connection.NetCapabilities) => {
            console.log(`${caseName} getNetCapabilities promiss data suceess: ${JSON.stringify(data)}`);
            if(data.networkCap){
              ExpectTrue(data.networkCap.includes(connection.NetCap.NET_CAPABILITY_INTERNET));
            }
            console.info(`${caseName}---End`);
            done();
          }).catch((error:BusinessError) => {
            console.log(`${caseName} getNetCapabilities promiss fail: `+JSON.stringify(error));
            ExpectFail();
            console.info(`${caseName}---End`);
            done();
          });
        }).catch((error:BusinessError) => {
          console.log(`${caseName} getAllNets promiss error: `+ JSON.stringify(error));
          ExpectFail();
          console.info(`${caseName}---End`);
          done();
        });
      } catch (error) {
        console.info(`${caseName} catch err ->${JSON.stringify(error)}`);
        ExpectFail();
        console.info(`${caseName}---End`);
        done();
      }
    });

    /**
     * @tc.number: SUB_NetManager_Connection_NetBearType_0100
     * @tc.name:  testNetManagerConnectionNetBearType0100
     * @tc.desc: GetNetCapabilities  detection
     * @tc.level:Level 3
     * @tc.type:Function
     * @tc.size:MediumTest
     */
    it("testNetManagerConnectionNetBearType0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, (done: Function) => {
      let caseName: string = "testNetManagerConnectionNetBearType0100";
      console.info(`${caseName}---start`);
      try {
        ExpectTrue(connection.NetBearType.BEARER_CELLULAR == 0);
        ExpectTrue(connection.NetBearType.BEARER_WIFI == 1);
        ExpectTrue(connection.NetBearType.BEARER_BLUETOOTH == 2);
        ExpectTrue(connection.NetBearType.BEARER_ETHERNET == 3);
        console.info(`${caseName}---End`);
        done();
      } catch (error) {
        console.info(`${caseName} catch err ->${JSON.stringify(error)}`);
        ExpectFail();
        console.info(`${caseName}---End`);
        done();
      }
    })
  })
}

