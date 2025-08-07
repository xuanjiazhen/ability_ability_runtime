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
import locationcapi20 from 'liblocationcapi20.so'

const OS_LOCATION_ERR_OK = 0;

export default function ActsLocationCApi20Test() {
  describe('ActsLocationCApi20Test', () => {
    /**
     * @tc.number  : SUB_LocationKit_GetAdditionalInfo_Test_0100
     * @tc.name    : SUB_LocationKit_GetAdditionalInfo_Test_0100
     * @tc.desc    : test OHLocationInfoGetAdditionalInfo.
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_LocationKit_GetAdditionalInfo_Test_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>SUB_LocationKit_GetAdditionalInfo_Test_0100 start====");
          let result: number = locationcapi20.OHLocationInfoGetAdditionalInfo_test();
          console.info("====>SUB_LocationKit_GetAdditionalInfo_Test_0100 result: " + result);
          expect(401).assertEqual(result);
          done()
        } catch (err) {
          console.error("====>SUB_LocationKit_GetAdditionalInfo_Test_0100 catch err: " + err);
          expect(false).assertEqual(OS_LOCATION_ERR_OK);
          done()
        }
      })
      /**
 * @tc.number  : SUB_LocationKit_OHLocationInfoGetAdditionalInfo_Test_0100
 * @tc.name    : testLocationKitOHLocationInfoGetAdditionalInfoTest0100
 * @tc.desc    : test OHLocationInfoGetAdditionalInfo.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_OHLocationInfoGetAdditionalInfo_Test_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
  async (done: Function) => {
    try {
      console.info("====>SUB_LocationKit_OHLocationInfoGetAdditionalInfo_Test_0100 start====");
      let result = locationcapi20.OHLocationInfoGetAdditionalInfo();
      console.info("====>SUB_LocationKit_OHLocationInfoGetAdditionalInfo_Test_0100 result: " + result);
      expect(401).assertEqual(result);
      done()
    } catch (err) {
      console.error("====>SUB_LocationKit_OHLocationInfoGetAdditionalInfo_Test_0100 catch err: " + err);
      expect(false).assertEqual(OS_LOCATION_ERR_OK);
      done()
    }
  })

/**
 * @tc.number  : SUB_LocationKit_OH_Location_IsLocatingEnabled_Test_0100
 * @tc.name    : testLocationKitOH_Location_IsLocatingEnabledTest0100
 * @tc.desc    : test OH_Location_IsLocatingEnabled1.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_OH_Location_IsLocatingEnabled_Test_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
  try {
    console.info("====>SUB_LocationKit_OH_Location_IsLocatingEnabled_Test_0100 start====");
    let result = locationcapi20.OH_Location_IsLocatingEnabled1();
    console.info("====>SUB_LocationKit_OH_Location_IsLocatingEnabled_Test_0100 result: " + result);
    expect(0).assertEqual(result);
    done()
  } catch (err) {
    console.error("====>SUB_LocationKit_OH_Location_IsLocatingEnabled_Test_0100 catch err: " + err);
    expect(false).assertEqual(OS_LOCATION_ERR_OK);
    done()
  }
})

/**
 * @tc.number  : SUB_LocationKit_OH_Location_IsLocatingEnabled_Test_0101
 * @tc.name    : testLocationKitOH_Location_IsLocatingEnabledTest0101
 * @tc.desc    : test OH_Location_IsLocatingEnabled2.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_OH_Location_IsLocatingEnabled_Test_0101', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
  try {
    console.info("====>SUB_LocationKit_OH_Location_IsLocatingEnabled_Test_0101 start====");
    let result = locationcapi20.OH_Location_IsLocatingEnabled2();
    console.info("====>SUB_LocationKit_OH_Location_IsLocatingEnabled_Test_0101 result: " + result);
    expect(401).assertEqual(result);
    done()
  } catch (err) {
    console.error("====>SUB_LocationKit_OH_Location_IsLocatingEnabled_Test_0101 catch err: " + err);
    expect(false).assertEqual(OS_LOCATION_ERR_OK);
    done()
  }
})

/**
 * @tc.number  : SUB_LocationKit_OH_Location_StartLocating_Test_0100
 * @tc.name    : testLocationKitOH_Location_StartLocatingTest0100
 * @tc.desc    : test OH_Location_StartLocating1.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_OH_Location_StartLocating_Test_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
          try {
            console.info("====>SUB_LocationKit_OH_Location_StartLocating_Test_0100 start====");
            let result = locationcapi20.OH_Location_StartLocating1();
            console.info("====>SUB_LocationKit_OH_Location_StartLocating_Test_0100 result: " + result);
            expect(3301100).assertEqual(result);
            done()
          } catch (err) {
            console.error("====>SUB_LocationKit_OH_Location_StartLocating_Test_0100 catch err: " + err);
            expect(false).assertEqual(OS_LOCATION_ERR_OK);
            done()
          }
        }
)
/**
 * @tc.number  : SUB_LocationKit_OH_Location_StartLocating_Test_0101
 * @tc.name    : testLocationKitOH_Location_StartLocatingTest0101
 * @tc.desc    : test OH_Location_StartLocating2.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_OH_Location_StartLocating_Test_0101', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
  try {
    console.info("====>SUB_LocationKit_OH_Location_StartLocating_Test_0101 start====");
    let result = locationcapi20.OH_Location_StartLocating2();
    console.info("====>SUB_LocationKit_OH_Location_StartLocating_Test_0101 result: " + result);
    expect(401).assertEqual(result);
    done()
  } catch (err) {
    console.error("====>SUB_LocationKit_OH_Location_StartLocating_Test_0101 catch err: " + err);
    expect(false).assertEqual(OS_LOCATION_ERR_OK);
    done()
  }
})

/**
 * @tc.number  : SUB_LocationKit_OH_Location_StopLocating_Test_0100
 * @tc.name    : testLocationKitOH_Location_StopLocatingTest0100
 * @tc.desc    : test OH_Location_StopLocating2.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_OH_Location_StopLocating_Test_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
  try {
    console.info("====>SUB_LocationKit_OH_Location_StopLocating_Test_0100 start====");
    let result = locationcapi20.OH_Location_StopLocating2();
    console.info("====>SUB_LocationKit_OH_Location_StopLocating_Test_0100 result: " + result);
    expect(401).assertEqual(result);
    done()
  } catch (err) {
    console.error("====>SUB_LocationKit_OH_Location_StopLocating_Test_0100 catch err: " + err);
    expect(false).assertEqual(OS_LOCATION_ERR_OK);
    done()
  }
})

/**
 * @tc.number  : SUB_LocationKit_OH_LocationRequestConfig_SetUseScene_Test_0100
 * @tc.name    : testLocationKitOH_LocationRequestConfig_SetUseSceneTest0100
 * @tc.desc    : test OH_LocationRequestConfig_SetUseScene0100.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_OH_LocationRequestConfig_SetUseScene_Test_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
  async (done: Function) => {
    try {
      console.info("====>SUB_LocationKit_OH_LocationRequestConfig_SetUseScene_Test_0100 start====");
      let result = locationcapi20.OH_LocationRequestConfig_SetUseScene0100();
      console.info("====>SUB_LocationKit_OH_LocationRequestConfig_SetUseScene_Test_0100 result: " + result);
      expect(result).assertEqual(0);
      done()
    } catch (err) {
      console.error("====>SUB_LocationKit_OH_LocationRequestConfig_SetUseScene_Test_0100 catch err: " + err);
      expect(false).assertEqual(OS_LOCATION_ERR_OK);
      done()
    }
  })

/**
 * @tc.number  : SUB_LocationKit_OH_LocationRequestConfig_SetUseScene_Test_0101
 * @tc.name    : testLocationKitOH_LocationRequestConfig_SetUseSceneTest0101
 * @tc.desc    : test OH_LocationRequestConfig_SetUseScene0101.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_OH_LocationRequestConfig_SetUseScene_Test_0101', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
  async (done: Function) => {
    try {
      console.info("====>SUB_LocationKit_OH_LocationRequestConfig_SetUseScene_Test_0101 start====");
      let result = locationcapi20.OH_LocationRequestConfig_SetUseScene0101();
      console.info("====>SUB_LocationKit_OH_LocationRequestConfig_SetUseScene_Test_0101 result: " + result);
      expect(result).assertEqual(0);
      done()
    } catch (err) {
      console.error("====>SUB_LocationKit_OH_LocationRequestConfig_SetUseScene_Test_0101 catch err: " + err);
      expect(false).assertEqual(OS_LOCATION_ERR_OK);
      done()
    }
  })

/**
 * @tc.number  : SUB_LocationKit_OH_LocationRequestConfig_SetUseScene_Test_0102
 * @tc.name    : testLocationKitOH_LocationRequestConfig_SetUseSceneTest0102
 * @tc.desc    : test OH_LocationRequestConfig_SetUseScene0102.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_OH_LocationRequestConfig_SetUseScene_Test_0102', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
  async (done: Function) => {
    try {
      console.info("====>SUB_LocationKit_OH_LocationRequestConfig_SetUseScene_Test_0102 start====");
      let result = locationcapi20.OH_LocationRequestConfig_SetUseScene0102();
      console.info("====>SUB_LocationKit_OH_LocationRequestConfig_SetUseScene_Test_0102 result: " + result);
      expect(result).assertEqual(0);
      done()
    } catch (err) {
      console.error("====>SUB_LocationKit_OH_LocationRequestConfig_SetUseScene_Test_0102 catch err: " + err);
      expect(false).assertEqual(OS_LOCATION_ERR_OK);
      done()
    }
  })

/**
 * @tc.number  : SUB_LocationKit_OH_LocationRequestConfig_SetPowerConsumptionScene_Test_0100
 * @tc.name    : testLocationKitOH_LocationRequestConfig_SetPowerConsumptionSceneTest0100
 * @tc.desc    : test OH_LocationRequestConfig_SetPowerConsumptionScene0100.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_OH_LocationRequestConfig_SetPowerConsumptionScene_Test_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
  async (done: Function) => {
    try {
      console.info("====>SUB_LocationKit_OH_LocationRequestConfig_SetPowerConsumptionScene_Test_0100 start====");
      let result = locationcapi20.OH_LocationRequestConfig_SetPowerConsumptionScene0100();
      console.info("====>SUB_LocationKit_OH_LocationRequestConfig_SetPowerConsumptionScene_Test_0100 result: " + result);
      expect(result).assertEqual(0);
      done()
    } catch (err) {
      console.error("====>SUB_LocationKit_OH_LocationRequestConfig_SetPowerConsumptionScene_Test_0100 catch err: " + err);
      expect(false).assertEqual(OS_LOCATION_ERR_OK);
      done()
    }
  })

/**
 * @tc.number  : SUB_LocationKit_OH_LocationRequestConfig_SetPowerConsumptionScene_Test_0101
 * @tc.name    : testLocationKitOH_LocationRequestConfig_SetPowerConsumptionSceneTest0101
 * @tc.desc    : test OH_LocationRequestConfig_SetPowerConsumptionScene0101.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_OH_LocationRequestConfig_SetPowerConsumptionScene_Test_0101', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
  async (done: Function) => {
    try {
      console.info("====>SUB_LocationKit_OH_LocationRequestConfig_SetPowerConsumptionScene_Test_0101 start====");
      let result = locationcapi20.OH_LocationRequestConfig_SetPowerConsumptionScene0101();
      console.info("====>SUB_LocationKit_OH_LocationRequestConfig_SetPowerConsumptionScene_Test_0101 result: " + result);
      expect(result).assertEqual(0);
      done()
    } catch (err) {
      console.error("====>SUB_LocationKit_OH_LocationRequestConfig_SetPowerConsumptionScene_Test_0101 catch err: " + err);
      expect(false).assertEqual(OS_LOCATION_ERR_OK);
      done()
    }
  })

/**
 * @tc.number  : SUB_LocationKit_Location_BasicInfo_Test_0100
 * @tc.name    : testLocationKitLocation_BasicInfoTest0100
 * @tc.desc    : test Location_BasicInfo0100.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_Location_BasicInfo_Test_0100', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
  try {
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0100 start====");
    let result = locationcapi20.Location_BasicInfo0100();
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0100 result: " + result);
    expect(result).assertEqual(1);
    done()
  } catch (err) {
    console.error("====>SUB_LocationKit_Location_BasicInfo_Test_0100 catch err: " + err);
    expect(false).assertEqual(OS_LOCATION_ERR_OK);
    done()
  }
})

/**
 * @tc.number  : SUB_LocationKit_Location_BasicInfo_Test_0101
 * @tc.name    : testLocationKitLocation_BasicInfoTest0101
 * @tc.desc    : test Location_BasicInfo0101.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_Location_BasicInfo_Test_0101', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
  try {
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0101 start====");
    let result = locationcapi20.Location_BasicInfo0101();
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0101 result: " + result);
    expect(result).assertEqual(1);
    done()
  } catch (err) {
    console.error("====>SUB_LocationKit_Location_BasicInfo_Test_0101 catch err: " + err);
    expect(false).assertEqual(OS_LOCATION_ERR_OK);
    done()
  }
})

/**
 * @tc.number  : SUB_LocationKit_Location_BasicInfo_Test_0102
 * @tc.name    : testLocationKitLocation_BasicInfoTest0102
 * @tc.desc    : test Location_BasicInfo0102.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_Location_BasicInfo_Test_0102', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
  try {
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0102 start====");
    let result = locationcapi20.Location_BasicInfo0102();
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0102 result: " + result);
    expect(result).assertEqual(1);
    done()
  } catch (err) {
    console.error("====>SUB_LocationKit_Location_BasicInfo_Test_0102 catch err: " + err);
    expect(false).assertEqual(OS_LOCATION_ERR_OK);
    done()
  }
})

/**
 * @tc.number  : SUB_LocationKit_Location_BasicInfo_Test_0103
 * @tc.name    : testLocationKitLocation_BasicInfoTest0103
 * @tc.desc    : test Location_BasicInfo0103.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_Location_BasicInfo_Test_0103', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
  try {
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0103 start====");
    let result = locationcapi20.Location_BasicInfo0103();
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0103 result: " + result);
    expect(result).assertEqual(1);
    done()
  } catch (err) {
    console.error("====>SUB_LocationKit_Location_BasicInfo_Test_0103 catch err: " + err);
    expect(false).assertEqual(OS_LOCATION_ERR_OK);
    done()
  }
})

/**
 * @tc.number  : SUB_LocationKit_Location_BasicInfo_Test_0104
 * @tc.name    : testLocationKitLocation_BasicInfoTest0104
 * @tc.desc    : test Location_BasicInfo0104.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_Location_BasicInfo_Test_0104', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
  try {
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0104 start====");
    let result = locationcapi20.Location_BasicInfo0104();
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0104 result: " + result);
    expect(result).assertEqual(1);
    done()
  } catch (err) {
    console.error("====>SUB_LocationKit_Location_BasicInfo_Test_0104 catch err: " + err);
    expect(false).assertEqual(OS_LOCATION_ERR_OK);
    done()
  }
})

/**
 * @tc.number  : SUB_LocationKit_Location_BasicInfo_Test_0105
 * @tc.name    : testLocationKitLocation_BasicInfoTest0105
 * @tc.desc    : test Location_BasicInfo0105.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_Location_BasicInfo_Test_0105', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
  try {
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0105 start====");
    let result = locationcapi20.Location_BasicInfo0105();
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0105 result: " + result);
    expect(result).assertEqual(1);
    done()
  } catch (err) {
    console.error("====>SUB_LocationKit_Location_BasicInfo_Test_0105 catch err: " + err);
    expect(false).assertEqual(OS_LOCATION_ERR_OK);
    done()
  }
})

/**
 * @tc.number  : SUB_LocationKit_Location_BasicInfo_Test_0106
 * @tc.name    : testLocationKitLocation_BasicInfoTest0106
 * @tc.desc    : test Location_BasicInfo0106.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_Location_BasicInfo_Test_0106', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
  try {
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0106 start====");
    let result = locationcapi20.Location_BasicInfo0106();
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0106 result: " + result);
    expect(result).assertEqual(1);
    done()
  } catch (err) {
    console.error("====>SUB_LocationKit_Location_BasicInfo_Test_0106 catch err: " + err);
    expect(false).assertEqual(OS_LOCATION_ERR_OK);
    done()
  }
})

/**
 * @tc.number  : SUB_LocationKit_Location_BasicInfo_Test_0107
 * @tc.name    : testLocationKitLocation_BasicInfoTest0107
 * @tc.desc    : test Location_BasicInfo0107.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_Location_BasicInfo_Test_0107', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
  try {
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0107 start====");
    let result = locationcapi20.Location_BasicInfo0107();
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0107 result: " + result);
    expect(result).assertEqual(1);
    done()
  } catch (err) {
    console.error("====>SUB_LocationKit_Location_BasicInfo_Test_0107 catch err: " + err);
    expect(false).assertEqual(OS_LOCATION_ERR_OK);
    done()
  }
})

/**
 * @tc.number  : SUB_LocationKit_Location_BasicInfo_Test_0108
 * @tc.name    : testLocationKitLocation_BasicInfoTest0108
 * @tc.desc    : test Location_BasicInfo0108.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_Location_BasicInfo_Test_0108', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
  try {
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0108 start====");
    let result = locationcapi20.Location_BasicInfo0108();
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0108 result: " + result);
    expect(result).assertEqual(1);
    done()
  } catch (err) {
    console.error("====>SUB_LocationKit_Location_BasicInfo_Test_0108 catch err: " + err);
    expect(false).assertEqual(OS_LOCATION_ERR_OK);
    done()
  }
})

/**
 * @tc.number  : SUB_LocationKit_Location_BasicInfo_Test_0109
 * @tc.name    : testLocationKitLocation_BasicInfoTest0109
 * @tc.desc    : test Location_BasicInfo0109.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_Location_BasicInfo_Test_0109', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
  try {
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0109 start====");
    let result = locationcapi20.Location_BasicInfo0109();
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0109 result: " + result);
    expect(result).assertEqual(1);
    done()
  } catch (err) {
    console.error("====>SUB_LocationKit_Location_BasicInfo_Test_0109 catch err: " + err);
    expect(false).assertEqual(OS_LOCATION_ERR_OK);
    done()
  }
})

/**
 * @tc.number  : SUB_LocationKit_Location_BasicInfo_Test_0110
 * @tc.name    : testLocationKitLocation_BasicInfoTest0110
 * @tc.desc    : test Location_BasicInfo0110.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_Location_BasicInfo_Test_0110', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
  try {
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0110 start====");
    let result = locationcapi20.Location_BasicInfo0110();
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0110 result: " + result);
    expect(result).assertEqual(1);
    done()
  } catch (err) {
    console.error("====>SUB_LocationKit_Location_BasicInfo_Test_0110 catch err: " + err);
    expect(false).assertEqual(OS_LOCATION_ERR_OK);
    done()
  }
})

/**
 * @tc.number  : SUB_LocationKit_Location_BasicInfo_Test_0111
 * @tc.name    : testLocationKitLocation_BasicInfoTest0111
 * @tc.desc    : test Location_BasicInfo0111.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_Location_BasicInfo_Test_0111', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
  try {
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0111 start====");
    let result = locationcapi20.Location_BasicInfo0111();
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0111 result: " + result);
    expect(result).assertEqual(1);
    done()
  } catch (err) {
    console.error("====>SUB_LocationKit_Location_BasicInfo_Test_0111 catch err: " + err);
    expect(false).assertEqual(OS_LOCATION_ERR_OK);
    done()
  }
})

/**
 * @tc.number  : SUB_LocationKit_Location_BasicInfo_Test_0112
 * @tc.name    : testLocationKitLocation_BasicInfoTest0112
 * @tc.desc    : test Location_BasicInfo0112.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_Location_BasicInfo_Test_0112', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
  try {
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0112 start====");
    let result = locationcapi20.Location_BasicInfo0112();
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0112 result: " + result);
    expect(result).assertEqual(1);
    done()
  } catch (err) {
    console.error("====>SUB_LocationKit_Location_BasicInfo_Test_0112 catch err: " + err);
    expect(false).assertEqual(OS_LOCATION_ERR_OK);
    done()
  }
})

/**
 * @tc.number  : SUB_LocationKit_Location_BasicInfo_Test_0113
 * @tc.name    : testLocationKitLocation_BasicInfoTest0113
 * @tc.desc    : test Location_BasicInfo0113.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_Location_BasicInfo_Test_0113', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
  try {
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0113 start====");
    let result = locationcapi20.Location_BasicInfo0113();
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0113 result: " + result);
    expect(result).assertEqual(1);
    done()
  } catch (err) {
    console.error("====>SUB_LocationKit_Location_BasicInfo_Test_0113 catch err: " + err);
    expect(false).assertEqual(OS_LOCATION_ERR_OK);
    done()
  }
})

/**
 * @tc.number  : SUB_LocationKit_Location_BasicInfo_Test_0114
 * @tc.name    : testLocationKitLocation_BasicInfoTest0114
 * @tc.desc    : test Location_BasicInfo0114.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_Location_BasicInfo_Test_0114', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
  try {
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0114 start====");
    let result = locationcapi20.Location_BasicInfo0114();
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0114 result: " + result);
    expect(result).assertEqual(1);
    done()
  } catch (err) {
    console.error("====>SUB_LocationKit_Location_BasicInfo_Test_0114 catch err: " + err);
    expect(false).assertEqual(OS_LOCATION_ERR_OK);
    done()
  }
})

/**
 * @tc.number  : SUB_LocationKit_Location_BasicInfo_Test_0115
 * @tc.name    : testLocationKitLocation_BasicInfoTest0115
 * @tc.desc    : test Location_BasicInfo0115.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_Location_BasicInfo_Test_0115', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
  try {
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0115 start====");
    let result = locationcapi20.Location_BasicInfo0115();
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0115 result: " + result);
    expect(result).assertEqual(1);
    done()
  } catch (err) {
    console.error("====>SUB_LocationKit_Location_BasicInfo_Test_0115 catch err: " + err);
    expect(false).assertEqual(OS_LOCATION_ERR_OK);
    done()
  }
})

/**
 * @tc.number  : SUB_LocationKit_Location_BasicInfo_Test_0116
 * @tc.name    : testLocationKitLocation_BasicInfoTest0116
 * @tc.desc    : test Location_BasicInfo0116.
 * @tc.level   : Level1
 * @tc.size    : MediumTest
 * @tc.type    : Function
 */
it('SUB_LocationKit_Location_BasicInfo_Test_0116', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
  try {
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0116 start====");
    let result = locationcapi20.Location_BasicInfo0116();
    console.info("====>SUB_LocationKit_Location_BasicInfo_Test_0116 result: " + result);
    expect(result).assertEqual(1);
    done()
  } catch (err) {
    console.error("====>SUB_LocationKit_Location_BasicInfo_Test_0116 catch err: " + err);
    expect(false).assertEqual(OS_LOCATION_ERR_OK);
    done()
  }
})

  })
}
