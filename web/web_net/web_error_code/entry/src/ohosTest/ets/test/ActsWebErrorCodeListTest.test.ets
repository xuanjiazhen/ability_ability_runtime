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
        let result: number = webCode.WebArkWebResponseGetError("ARKWEB_ERR_IO_PENDING");
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
        let result: number = webCode.WebArkWebResponseGetError("ARKWEB_ERR_FAILED");
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
        let result: number = webCode.WebArkWebResponseGetError("ARKWEB_ERR_ABORTED");
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
        let result: number = webCode.WebArkWebResponseGetError("ARKWEB_ERR_INVALID_HANDLE");
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
        let result: number = webCode.WebArkWebResponseGetError("ARKWEB_ERR_TIMED_OUT");
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
        let result: number = webCode.WebArkWebResponseGetError("ARKWEB_ERR_FILE_TOO_LARGE");
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
        let result: number = webCode.WebArkWebResponseGetError("ARKWEB_ERR_UNEXPECTED");
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
        let result: number = webCode.WebArkWebResponseGetError("ARKWEB_ERR_ACCESS_DENIED");
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
        let result: number = webCode.WebArkWebResponseGetError("ARKWEB_ERR_NOT_IMPLEMENTED");
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
        let result: number = webCode.WebArkWebResponseGetError("ARKWEB_ERR_INSUFFICIENT_RESOURCES");
        console.info("====>testArkWebErrInsufficientResources result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrInsufficientResources catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_1100
     * @tc.name    : testArkWebErrOutofMemory
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_OUT_OF_MEMORY
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrOutofMemory', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrOutofMemory start====");
        let result: number = webCode.WebArkWebResponseGetError("ARKWEB_ERR_OUT_OF_MEMORY");
        console.info("====>testArkWebErrOutofMemory result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrOutofMemory catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_1200
     * @tc.name    : testArkWebErrUploadFileChanged
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_UPLOAD_FILE_CHANGED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrUploadFileChanged', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrUploadFileChanged start====");
        let result: number = webCode.WebArkWebResponseGetError("ARKWEB_ERR_UPLOAD_FILE_CHANGED");
        console.info("====>testArkWebErrUploadFileChanged result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrUploadFileChanged catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_1300
     * @tc.name    : testArkWebErrSocketNotConnected
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_SOCKET_NOT_CONNECTED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrSocketNotConnected', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkWebErrSocketNotConnected start====");
          let result: number = webCode.WebArkWebResponseGetError("ARKWEB_ERR_SOCKET_NOT_CONNECTED");
          console.info("====>testArkWebErrSocketNotConnected result====", result);
          expect(result).assertEqual(0);
          done();
        } catch (err) {
          console.error("====>testArkWebErrSocketNotConnected catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_1400
     * @tc.name    : testArkWebErrFileExists
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_FILE_EXISTS
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrFileExists', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrFileExists start====");
        let result: number = webCode.WebArkWebResponseGetError("ARKWEB_ERR_FILE_EXISTS");
        console.info("====>testArkWebErrFileExists result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrFileExists catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_1500
     * @tc.name    : testArkWebErrFilePathTooLong
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_FILE_PATH_TOO_LONG
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrFilePathTooLong', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrFilePathTooLong start====");
        let result: number = webCode.WebArkWebResponseGetError("ARKWEB_ERR_FILE_PATH_TOO_LONG");
        console.info("====>testArkWebErrFilePathTooLong result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrFilePathTooLong catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_1600
     * @tc.name    : testArkWebErrFileNoSpace
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_FILE_NO_SPACE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrFileNoSpace', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrFileNoSpace start====");
        let result: number = webCode.WebArkWebResponseGetError("ARKWEB_ERR_FILE_NO_SPACE");
        console.info("====>testArkWebErrFileNoSpace result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrFileNoSpace catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_1700
     * @tc.name    : testArkWebErrFileVirusInfected
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_FILE_VIRUS_INFECTED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrFileVirusInfected', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrFileVirusInfected start====");
        let result: number = webCode.WebArkWebResponseGetError("ARKWEB_ERR_FILE_VIRUS_INFECTED");
        console.info("====>testArkWebErrFileVirusInfected result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrFileVirusInfected catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_1800
     * @tc.name    : testArkWebErrBlockedByClient
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_BLOCKED_BY_CLIENT
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrBlockedByClient', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrBlockedByClient start====");
        let result: number = webCode.WebArkWebResponseGetError("ARKWEB_ERR_BLOCKED_BY_CLIENT");
        console.info("====>testArkWebErrBlockedByClient result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrBlockedByClient catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_1900
     * @tc.name    : testArkWebErrNetWorkChanged
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_NETWORK_CHANGED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrNetWorkChanged', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrNetWorkChanged start====");
        let result: number = webCode.WebArkWebResponseGetError("ARKWEB_ERR_NETWORK_CHANGED");
        console.info("====>testArkWebErrNetWorkChanged result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrNetWorkChanged catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_2000
     * @tc.name    : testArkWebErrBlockedByAdministarator
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_BLOCKED_BY_ADMINISTRATOR
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrBlockedByAdministarator', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkWebErrBlockedByAdministarator start====");
          let result: number = webCode.WebArkWebResponseGetError("ARKWEB_ERR_BLOCKED_BY_ADMINISTRATOR");
          console.info("====>testArkWebErrBlockedByAdministarator result====", result);
          expect(result).assertEqual(0);
          done();
        } catch (err) {
          console.error("====>testArkWebErrBlockedByAdministarator catch err: " + err);
        }
      })

    /**
    * @tc.number  : SUB_WEB_NET_ERROR_2100
    * @tc.name    : testArkWebErrSocketConnected
    * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_SOCKET_CONNECTED
    * @tc.level   : Level1
    * @tc.size    : MediumTest
    * @tc.type    : Function
    */
    it('testArkWebErrSocketConnected', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrBlockedByAdministarator start====");
        let result: number = webCode.WebArkWebResponseGetError("ARKWEB_ERR_SOCKET_CONNECTED");
        console.info("====>testArkWebErrBlockedByAdministarator result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrBlockedByAdministarator catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_2200
     * @tc.name    : testArkWebErrUploadStreamRewindNotSupported
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_UPLOAD_STREAM_REWIND_NOT_SUPPORTED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrUploadStreamRewindNotSupported', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: Function) => {
        try {
          console.info("====>testArkWebErrUploadStreamRewindNotSupported start====");
          let result: number = webCode.WebArkWebResponseGetError("ARKWEB_ERR_UPLOAD_STREAM_REWIND_NOT_SUPPORTED");
          console.info("====>testArkWebErrUploadStreamRewindNotSupported result====", result);
          expect(result).assertEqual(0);
          done();
        } catch (err) {
          console.error("====>testArkWebErrUploadStreamRewindNotSupported catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_2300
     * @tc.name    : testArkWebErrContextShutDown
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_CONTEXT_SHUT_DOWN
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrContextShutDown', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>testArkWebErrContextShutDown start====");
          let result: number = webCode.webNetErrorDidFailWithError("ARKWEB_ERR_CONTEXT_SHUT_DOWN");
          console.info("====>testArkWebErrContextShutDown result====", result);
          expect(result).assertEqual(0);
          done();
        } catch (err) {
          console.error("====>testArkWebErrContextShutDown catch err: " + err);
        }
      })




  })
}