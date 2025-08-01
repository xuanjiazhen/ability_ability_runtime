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
import webCode from 'libweberrorcodetest.so'


export default function ActsWebErrorCodeListTest() {
  describe('ActsWebErrorCodeListTest', () => {
    /**
     * @tc.number  : SUB_WEB_NET_ERROR_0100
     * @tc.name    : testArkWebErrIOPending
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_IO_PENDING
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrIOPending', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrIOPending start====");
        let result: number = webCode.webNetErrorDidFailWithError("ARKWEB_ERR_IO_PENDING");
        console.info("====>testArkWebErrIOPending result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrIOPending catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_0200
     * @tc.name    : testArkWebErrFailed
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_FAILED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrFailed', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrFailed start====");
        let result: number = webCode.webNetErrorDidFailWithError("ARKWEB_ERR_FAILED");
        console.info("====>testArkWebErrFailed result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrFailed catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_0300
     * @tc.name    : testArkWebErrAboorted
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_ABORTED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrAboorted', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrAboorted start====");
        let result: number = webCode.webNetErrorDidFailWithError("ARKWEB_ERR_ABORTED");
        console.info("====>testArkWebErrAboorted result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrAboorted catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_0400
     * @tc.name    : testArkWebErrInvaliedHandle
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_INVALID_HANDLE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrInvaliedHandle', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrInvaliedHandle start====");
        let result: number = webCode.webNetErrorDidFailWithError("ARKWEB_ERR_INVALID_HANDLE");
        console.info("====>testArkWebErrInvaliedHandle result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrInvaliedHandle catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_0500
     * @tc.name    : testArkWebErrTimedOut
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_TIMED_OUT
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrTimedOut', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrTimedOut start====");
        let result: number = webCode.webNetErrorDidFailWithError("ARKWEB_ERR_TIMED_OUT");
        console.info("====>testArkWebErrTimedOut result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrTimedOut catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_0600
     * @tc.name    : testArkWebErrFileTooLarge
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_FILE_TOO_LARGE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrFileTooLarge', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrFileTooLarge start====");
        let result: number = webCode.webNetErrorDidFailWithError("ARKWEB_ERR_FILE_TOO_LARGE");
        console.info("====>testArkWebErrFileTooLarge result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrFileTooLarge catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_0700
     * @tc.name    : testArkWebErrUnexpected
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_UNEXPECTED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrUnexpected', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrUnexpected start====");
        let result: number = webCode.webNetErrorDidFailWithError("ARKWEB_ERR_UNEXPECTED");
        console.info("====>testArkWebErrUnexpected result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrUnexpected catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_0800
     * @tc.name    : testArkWebErrAccessDenied
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_ACCESS_DENIED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrAccessDenied', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrAccessDenied start====");
        let result: number = webCode.webNetErrorDidFailWithError("ARKWEB_ERR_ACCESS_DENIED");
        console.info("====>testArkWebErrAccessDenied result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrAccessDenied catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_0900
     * @tc.name    : testArkWebErrNotImplemented
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_NOT_IMPLEMENTED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrNotImplemented', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrNotImplemented start====");
        let result: number = webCode.webNetErrorDidFailWithError("ARKWEB_ERR_NOT_IMPLEMENTED");
        console.info("====>testArkWebErrNotImplemented result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrNotImplemented catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_1000
     * @tc.name    : testArkWebErrInsufficientResources
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_INSUFFICIENT_RESOURCES
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrInsufficientResources', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrInsufficientResources start====");
        let result: number = webCode.webNetErrorDidFailWithError("ARKWEB_ERR_INSUFFICIENT_RESOURCES");
        console.info("====>testArkWebErrInsufficientResources result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrInsufficientResources catch err: " + err);
      }
    })
  })
}