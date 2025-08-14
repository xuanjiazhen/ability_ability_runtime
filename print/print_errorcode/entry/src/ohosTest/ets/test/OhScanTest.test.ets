/*
* Copyright (c) 2024 Huawei Device Co., Ltd.
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
import printerror from 'libprinterrorcode.so'
const ERR_OK = 0;
const INVALID_PARAMETER = 401;
const UNSUPPORTED = 24300104;
const INVALID = 24300107;

export default function ActsScanErrorTest() {
  describe('ActsScanErrorTest', () => {
    /**
     * @tc.number  : SUB_Oh_Scan_NoError_0100
     * @tc.name    : SUB_Oh_Scan_NoError_0100
     * @tc.desc    : Test OHScanNoError
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Oh_Scan_NoError_0100', Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_Oh_Scan_NoError_0100 start====");
        let result = printerror.OHScanNoError();
        expect(result).assertEqual(ERR_OK);
        console.info("====>SUB_Oh_Scan_NoError_0100 result: " + result);
        done();
      } catch (err) {
        console.error("====>SUB_Oh_Scan_NoError_0100 catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : SUB_Oh_Scan_InvalidParameter_0200
     * @tc.name    : SUB_Oh_Scan_InvalidParameter_0200
     * @tc.desc    : Test OHScanInvalidParameter
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Oh_Scan_InvalidParameter_0200', Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_Oh_Scan_InvalidParameter_0200 start====");
        let result = printerror.OHScanInvalidParameter();
        expect(result).assertEqual(INVALID_PARAMETER);
        console.info("====>SUB_Oh_Scan_InvalidParameter_0200 result: " + result);
        done();
      } catch (err) {
        console.error("====>SUB_Oh_Scan_InvalidParameter_0200 catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : SUB_Oh_Scan_UnSupported_0300
     * @tc.name    : SUB_Oh_Scan_UnSupported_0300
     * @tc.desc    : Test OHScanUnSupported
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Oh_Scan_UnSupported_0300', Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_Oh_Scan_UnSupported_0300 start====");
        let result = printerror.OHScanUnSupported();
        expect(result).assertEqual(UNSUPPORTED);
        console.info("====>SUB_Oh_Scan_UnSupported_0300 result: " + result);
        done();
      } catch (err) {
        console.error("====>SUB_Oh_Scan_UnSupported_0300 catch err: " + err);
        done()
      }
    });

    /**
     * @tc.number  : SUB_Oh_Scan_INVALID_0400
     * @tc.name    : SUB_Oh_Scan_INVALID_0400
     * @tc.desc    : Test OHPrintINVALID
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('SUB_Oh_Scan_INVALID_0400', Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>SUB_Oh_Scan_INVALID_0400 start====");
        let result = printerror.OHPrintINVALID();
        expect(result).assertEqual(INVALID);
        console.info("====>SUB_Oh_Scan_INVALID_0400 result: " + result);
        done();
      } catch (err) {
        console.error("====>SUB_Oh_Scan_INVALID_0400 catch err: " + err);
        done()
      }
    });
  })
}

