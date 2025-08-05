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

import { describe, it, expect, TestType, Size, Level } from '@ohos/hypium'
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
        let result: number = webCode.webArkWebResponseGetError("ARKWEB_ERR_IO_PENDING");
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
        let result: number = webCode.webArkWebResponseGetError("ARKWEB_ERR_FAILED");
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
        let result: number = webCode.webArkWebResponseGetError("ARKWEB_ERR_ABORTED");
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
        let result: number = webCode.webArkWebResponseGetError("ARKWEB_ERR_INVALID_HANDLE");
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
        let result: number = webCode.webArkWebResponseGetError("ARKWEB_ERR_TIMED_OUT");
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
        let result: number = webCode.webArkWebResponseGetError("ARKWEB_ERR_FILE_TOO_LARGE");
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
        let result: number = webCode.webArkWebResponseGetError("ARKWEB_ERR_UNEXPECTED");
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
        let result: number = webCode.webArkWebResponseGetError("ARKWEB_ERR_ACCESS_DENIED");
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
        let result: number = webCode.webArkWebResponseGetError("ARKWEB_ERR_NOT_IMPLEMENTED");
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
        let result: number = webCode.webArkWebResponseGetError("ARKWEB_ERR_INSUFFICIENT_RESOURCES");
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
        let result: number = webCode.webArkWebResponseGetError("ARKWEB_ERR_OUT_OF_MEMORY");
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
        let result: number = webCode.webArkWebResponseGetError("ARKWEB_ERR_UPLOAD_FILE_CHANGED");
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
          let result: number = webCode.webArkWebResponseGetError("ARKWEB_ERR_SOCKET_NOT_CONNECTED");
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
        let result: number = webCode.webArkWebResponseGetError("ARKWEB_ERR_FILE_EXISTS");
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
        let result: number = webCode.webArkWebResponseGetError("ARKWEB_ERR_FILE_PATH_TOO_LONG");
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
        let result: number = webCode.webArkWebResponseGetError("ARKWEB_ERR_FILE_NO_SPACE");
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
        let result: number = webCode.webArkWebResponseGetError("ARKWEB_ERR_FILE_VIRUS_INFECTED");
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
        let result: number = webCode.webArkWebResponseGetError("ARKWEB_ERR_BLOCKED_BY_CLIENT");
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
        let result: number = webCode.webArkWebResponseGetError("ARKWEB_ERR_NETWORK_CHANGED");
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
          let result: number = webCode.webArkWebResponseGetError("ARKWEB_ERR_BLOCKED_BY_ADMINISTRATOR");
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
        let result: number = webCode.webArkWebResponseGetError("ARKWEB_ERR_SOCKET_CONNECTED");
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
          let result: number = webCode.webArkWebResponseGetError("ARKWEB_ERR_UPLOAD_STREAM_REWIND_NOT_SUPPORTED");
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
          let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_CONTEXT_SHUT_DOWN");
          console.info("====>testArkWebErrContextShutDown result====", result);
          expect(result).assertEqual(0);
          done();
        } catch (err) {
          console.error("====>testArkWebErrContextShutDown catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_2400
     * @tc.name    : testArkWebErrBlockByResponse
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_BLOCKED_BY_RESPONSE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrBlockByResponse', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrBlockByResponse start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_BLOCKED_BY_RESPONSE");
        console.info("====>testArkWebErrBlockByResponse result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrBlockByResponse catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_2500
     * @tc.name    : testArkWebErrCleartextNotPermitted
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_CLEARTEXT_NOT_PERMITTED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrCleartextNotPermitted', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrCleartextNotPermitted start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_CLEARTEXT_NOT_PERMITTED");
        console.info("====>testArkWebErrCleartextNotPermitted result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrCleartextNotPermitted catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_2600
     * @tc.name    : testArkWebErrBlockedByCSP
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_BLOCKED_BY_CSP
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrBlockedByCSP', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrBlockedByCSP start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_BLOCKED_BY_CSP");
        console.info("====>testArkWebErrBlockedByCSP result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrBlockedByCSP catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_2700
     * @tc.name    : testArkWebErrH2orQUICRequired
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_H2_OR_QUIC_REQUIRED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrH2orQUICRequired', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrH2orQUICRequired start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_H2_OR_QUIC_REQUIRED");
        console.info("====>testArkWebErrH2orQUICRequired result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrH2orQUICRequired catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_2800
     * @tc.name    : testArkWebErrBlockedByORB
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_BLOCKED_BY_ORB
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrBlockedByORB', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrBlockedByORB start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_BLOCKED_BY_ORB");
        console.info("====>testArkWebErrBlockedByORB result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrBlockedByORB catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_2900
     * @tc.name    : testArkWebErrConnectionClosed
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_CONNECTION_CLOSED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrConnectionClosed', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrConnectionClosed start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_CONNECTION_CLOSED");
        console.info("====>testArkWebErrConnectionClosed result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrConnectionClosed catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_3000
     * @tc.name    : testArkWebErrConnectionReset
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_CONNECTION_RESET
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrConnectionReset', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrConnectionReset start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_CONNECTION_RESET");
        console.info("====>testArkWebErrConnectionReset result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrConnectionReset catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_3100
     * @tc.name    : testArkWebErrConnectionRefused
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_CONNECTION_REFUSED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrConnectionRefused', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrConnectionRefused start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_CONNECTION_REFUSED");
        console.info("====>testArkWebErrConnectionRefused result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrConnectionRefused catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_3200
     * @tc.name    : testArkWebErrConnectionAborted
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_CONNECTION_ABORTED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrConnectionAborted', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrConnectionAborted start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_CONNECTION_ABORTED");
        console.info("====>testArkWebErrConnectionAborted result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrConnectionAborted catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_3300
     * @tc.name    : testArkWebErrConnectionFailed
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_CONNECTION_FAILED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrConnectionFailed', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrConnectionFailed start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_CONNECTION_FAILED");
        console.info("====>testArkWebErrConnectionFailed result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrConnectionFailed catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_3400
     * @tc.name    : testArkWebErrNameNotResolved
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_NAME_NOT_RESOLVED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrNameNotResolved', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrNameNotResolved start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_NAME_NOT_RESOLVED");
        console.info("====>testArkWebErrNameNotResolved result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrNameNotResolved catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_3500
     * @tc.name    : testArkWebErrInternetDisconnected
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_INTERNET_DISCONNECTED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrInternetDisconnected', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrInternetDisconnected start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_INTERNET_DISCONNECTED");
        console.info("====>testArkWebErrInternetDisconnected result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrInternetDisconnected catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_3600
     * @tc.name    : testArkWebErrSSLProtocolError
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_SSL_PROTOCOL_ERROR
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrSSLProtocolError', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrSSLProtocolError start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_SSL_PROTOCOL_ERROR");
        console.info("====>testArkWebErrSSLProtocolError result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrSSLProtocolError catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_3700
     * @tc.name    : testArkWebErrAddressInvalid
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_ADDRESS_INVALID
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrAddressInvalid', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrAddressInvalid start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_ADDRESS_INVALID");
        console.info("====>testArkWebErrAddressInvalid result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrAddressInvalid catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_3800
     * @tc.name    : testArkWebErrAddressUnreachable
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_ADDRESS_UNREACHABLE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrAddressUnreachable', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrAddressUnreachable start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_ADDRESS_UNREACHABLE");
        console.info("====>testArkWebErrAddressUnreachable result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrAddressUnreachable catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_3900
     * @tc.name    : testArkWebErrSSLClientAuthCertNeeded
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_SSL_CLIENT_AUTH_CERT_NEEDED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrSSLClientAuthCertNeeded', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrAddretestArkWebErrSSLClientAuthCertNeededssUnreachable start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_SSL_CLIENT_AUTH_CERT_NEEDED");
        console.info("====>testArkWebErrSSLClientAuthCertNeeded result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrSSLClientAuthCertNeeded catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_4000
     * @tc.name    : testArkWebErrTunnelConnectionFailed
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_TUNNEL_CONNECTION_FAILED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrTunnelConnectionFailed', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrTunnelConnectionFailed start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_TUNNEL_CONNECTION_FAILED");
        console.info("====>testArkWebErrTunnelConnectionFailed result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrTunnelConnectionFailed catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_4100
     * @tc.name    : testArkWebErrNoSSLVersionsEnabled
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_NO_SSL_VERSIONS_ENABLED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrNoSSLVersionsEnabled', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
        try {
          console.info("====>testArkWebErrNoSSLVersionsEnabled start====");
          let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_NO_SSL_VERSIONS_ENABLED");
          console.info("====>testArkWebErrNoSSLVersionsEnabled result====", result);
          expect(result).assertEqual(0);
          done();
        } catch (err) {
          console.error("====>testArkWebErrNoSSLVersionsEnabled catch err: " + err);
        }
      })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_4200
     * @tc.name    : testArkWebErrSSLVersionOrCIPHERMismatch
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_SSL_VERSION_OR_CIPHER_MISMATCH
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrSSLVersionOrCIPHERMismatch', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrSSLVersionOrCIPHERMismatch start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_SSL_VERSION_OR_CIPHER_MISMATCH");
        console.info("====>testArkWebErrSSLVersionOrCIPHERMismatch result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrSSLVersionOrCIPHERMismatch catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_4300
     * @tc.name    : testArkWebErrSSLRenegotiationRequested
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_SSL_RENEGOTIATION_REQUESTED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrSSLRenegotiationRequested', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrSSLRenegotiationRequested start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_SSL_RENEGOTIATION_REQUESTED");
        console.info("====>testArkWebErrSSLRenegotiationRequested result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrSSLRenegotiationRequested catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_4400
     * @tc.name    : testArkWebErrProxyAuthUnsupported
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_PROXY_AUTH_UNSUPPORTED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrProxyAuthUnsupported', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrProxyAuthUnsupported start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_PROXY_AUTH_UNSUPPORTED");
        console.info("====>testArkWebErrProxyAuthUnsupported result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrProxyAuthUnsupported catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_4500
     * @tc.name    : testArkWebErrBadSSLClientAuthCert
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_BAD_SSL_CLIENT_AUTH_CERT
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrBadSSLClientAuthCert', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrBadSSLClientAuthCert start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_BAD_SSL_CLIENT_AUTH_CERT");
        console.info("====>testArkWebErrBadSSLClientAuthCert result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrBadSSLClientAuthCert catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_4600
     * @tc.name    : testArkWebErrConnectionTimedOut
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_CONNECTION_TIMED_OUT
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrConnectionTimedOut', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrConnectionTimedOut start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_CONNECTION_TIMED_OUT");
        console.info("====>testArkWebErrConnectionTimedOut result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrConnectionTimedOut catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_4700
     * @tc.name    : testArkWebErrHostResolverQueueTooLarge
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_HOST_RESOLVER_QUEUE_TOO_LARGE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrHostResolverQueueTooLarge', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrHostResolverQueueTooLarge start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_HOST_RESOLVER_QUEUE_TOO_LARGE");
        console.info("====>testArkWebErrHostResolverQueueTooLarge result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrHostResolverQueueTooLarge catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_4800
     * @tc.name    : testArkWebErrSocksConnectionFailed
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_SOCKS_CONNECTION_FAILED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrSocksConnectionFailed', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrSocksConnectionFailed start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_SOCKS_CONNECTION_FAILED");
        console.info("====>testArkWebErrSocksConnectionFailed result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrSocksConnectionFailed catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_4900
     * @tc.name    : testArkWebErrSocksConnectionHostUnreachable
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_SOCKS_CONNECTION_HOST_UNREACHABLE
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrSocksConnectionHostUnreachable', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrSocksConnectionHostUnreachable start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_SOCKS_CONNECTION_HOST_UNREACHABLE");
        console.info("====>testArkWebErrSocksConnectionHostUnreachable result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrSocksConnectionHostUnreachable catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_5000
     * @tc.name    : testArkWebErrAlpnNegotiationFailed
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_ALPN_NEGOTIATION_FAILED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrAlpnNegotiationFailed', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrAlpnNegotiationFailed start====");
        let result: number = webCode.webArkWebConnectError("ARKWEB_ERR_ALPN_NEGOTIATION_FAILED");
        console.info("====>testArkWebErrAlpnNegotiationFailed result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrAlpnNegotiationFailed catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_5100
     * @tc.name    : testArkWebErrSSLNoRenegotiation
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_SSL_NO_RENEGOTIATION
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrSSLNoRenegotiation', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrSSLNoRenegotiation start====");
        let result: number = webCode.webArkWebSSLError("ARKWEB_ERR_SSL_NO_RENEGOTIATION");
        console.info("====>testArkWebErrSSLNoRenegotiation result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrSSLNoRenegotiation catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_5200
     * @tc.name    : testArkWebErrWinsockUnexpectedWrittenBytes
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_WINSOCK_UNEXPECTED_WRITTEN_BYTES
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrWinsockUnexpectedWrittenBytes', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrWinsockUnexpectedWrittenBytes start====");
        let result: number = webCode.webArkWebSSLError("ARKWEB_ERR_WINSOCK_UNEXPECTED_WRITTEN_BYTES");
        console.info("====>testArkWebErrWinsockUnexpectedWrittenBytes result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrWinsockUnexpectedWrittenBytes catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_5300
     * @tc.name    : testArkWebErrSSLDecompressionFailureAlert
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_SSL_DECOMPRESSION_FAILURE_ALERT
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrSSLDecompressionFailureAlert', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrSSLDecompressionFailureAlert start====");
        let result: number = webCode.webArkWebSSLError("ARKWEB_ERR_SSL_DECOMPRESSION_FAILURE_ALERT");
        console.info("====>testArkWebErrSSLDecompressionFailureAlert result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrSSLDecompressionFailureAlert catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_5400
     * @tc.name    : testArkWebErrSSLBadRecordMacAlert
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_SSL_BAD_RECORD_MAC_ALERT
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrSSLBadRecordMacAlert', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrSSLBadRecordMacAlert start====");
        let result: number = webCode.webArkWebSSLError("ARKWEB_ERR_SSL_BAD_RECORD_MAC_ALERT");
        console.info("====>testArkWebErrSSLBadRecordMacAlert result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrSSLBadRecordMacAlert catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_5500
     * @tc.name    : testArkWebErrProxyAuthRequested
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_PROXY_AUTH_REQUESTED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrProxyAuthRequested', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrProxyAuthRequested start====");
        let result: number = webCode.webArkWebSSLError("ARKWEB_ERR_PROXY_AUTH_REQUESTED");
        console.info("====>testArkWebErrProxyAuthRequested result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrProxyAuthRequested catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_5600
     * @tc.name    : testArkWebErrProxyConnectionFailed
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_PROXY_CONNECTION_FAILED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrProxyConnectionFailed', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrProxyConnectionFailed start====");
        let result: number = webCode.webArkWebSSLError("ARKWEB_ERR_PROXY_CONNECTION_FAILED");
        console.info("====>testArkWebErrProxyConnectionFailed result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrProxyConnectionFailed catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_5700
     * @tc.name    : testArkWebErrProxyConfigurationFailed
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_MANDATORY_PROXY_CONFIGURATION_FAILED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrProxyConfigurationFailed', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrProxyConfigurationFailed start====");
        let result: number = webCode.webArkWebSSLError("ARKWEB_ERR_MANDATORY_PROXY_CONFIGURATION_FAILED");
        console.info("====>testArkWebErrProxyConfigurationFailed result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrProxyConfigurationFailed catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_5800
     * @tc.name    : testArkWebErrPreconnectMaxSocketLimit
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_PRECONNECT_MAX_SOCKET_LIMIT
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrPreconnectMaxSocketLimit', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrPreconnectMaxSocketLimit start====");
        let result: number = webCode.webArkWebSSLError("ARKWEB_ERR_PRECONNECT_MAX_SOCKET_LIMIT");
        console.info("====>testArkWebErrProxyConfigurationFailed result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrPreconnectMaxSocketLimit catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_5900
     * @tc.name    : testArkWebErrSSLClientAuthPrivateKeyAccessDenied
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_SSL_CLIENT_AUTH_PRIVATE_KEY_ACCESS_DENIED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrSSLClientAuthPrivateKeyAccessDenied', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrSSLClientAuthPrivateKeyAccessDenied start====");
        let result: number = webCode.webArkWebSSLError("ARKWEB_ERR_SSL_CLIENT_AUTH_PRIVATE_KEY_ACCESS_DENIED");
        console.info("====>testArkWebErrSSLClientAuthPrivateKeyAccessDenied result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrSSLClientAuthPrivateKeyAccessDenied catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_6000
     * @tc.name    : testArkWebErrSSLClientAuthCertNoPrivateKey
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_SSL_CLIENT_AUTH_CERT_NO_PRIVATE_KEY
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrSSLClientAuthCertNoPrivateKey', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrSSLClientAuthCertNoPrivateKey start====");
        let result: number = webCode.webArkWebSSLError("ARKWEB_ERR_SSL_CLIENT_AUTH_CERT_NO_PRIVATE_KEY");
        console.info("====>testArkWebErrSSLClientAuthCertNoPrivateKey result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrSSLClientAuthCertNoPrivateKey catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_6100
     * @tc.name    : testArkWebErrProxyCertificateInvalid
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_PROXY_CERTIFICATE_INVALID
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrProxyCertificateInvalid', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrProxyCertificateInvalid start====");
        let result: number = webCode.webArkWebSSLError("ARKWEB_ERR_PROXY_CERTIFICATE_INVALID");
        console.info("====>testArkWebErrProxyCertificateInvalid result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrProxyCertificateInvalid catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_6200
     * @tc.name    : testArkWebErrNameResolutionFailed
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_NAME_RESOLUTION_FAILED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrNameResolutionFailed', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrNameResolutionFailed start====");
        let result: number = webCode.webArkWebSSLError("ARKWEB_ERR_NAME_RESOLUTION_FAILED");
        console.info("====>testArkWebErrNameResolutionFailed result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrNameResolutionFailed catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_6300
     * @tc.name    : testArkWebErrNetworkAccessDenied
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_NETWORK_ACCESS_DENIED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrNetworkAccessDenied', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrNetworkAccessDenied start====");
        let result: number = webCode.webArkWebSSLError("ARKWEB_ERR_NETWORK_ACCESS_DENIED");
        console.info("====>testArkWebErrNetworkAccessDenied result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrNetworkAccessDenied catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_6400
     * @tc.name    : testArkWebErrTemporarilyThrottled
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_TEMPORARILY_THROTTLED
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebErrTemporarilyThrottled', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebErrTemporarilyThrottled start====");
        let result: number = webCode.webNetErrorDidFailWithError("ARKWEB_ERR_TEMPORARILY_THROTTLED");
        console.info("====>testArkWebErrTemporarilyThrottled result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebErrTemporarilyThrottled catch err: " + err);
      }
    })

    /**
     * @tc.number  : SUB_WEB_NET_ERROR_6500
     * @tc.name    : testArkWebHttpsProxyTunnelResponseRedirect
     * @tc.desc    : test ArkWeb_NetError Test ARKWEB_ERR_HTTPS_PROXY_TUNNEL_RESPONSE_REDIRECT
     * @tc.level   : Level1
     * @tc.size    : MediumTest
     * @tc.type    : Function
     */
    it('testArkWebHttpsProxyTunnelResponseRedirect', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      try {
        console.info("====>testArkWebHttpsProxyTunnelResponseRedirect start====");
        let result: number = webCode.webNetErrorDidFailWithError("ARKWEB_ERR_HTTPS_PROXY_TUNNEL_RESPONSE_REDIRECT");
        console.info("====>testArkWebHttpsProxyTunnelResponseRedirect result====", result);
        expect(result).assertEqual(0);
        done();
      } catch (err) {
        console.error("====>testArkWebHttpsProxyTunnelResponseRedirect catch err: " + err);
      }
    })

  })
}